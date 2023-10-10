#include "NuHepMC/CrossSectionUtils.hxx"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/Exceptions.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "HepMC3/ReaderFactory.h"

#include <algorithm>

namespace NuHepMC {

namespace CrossSection {

NEW_NuHepMC_EXCEPT(InvalidEnergyBinning);
NEW_NuHepMC_EXCEPT(IndeterminableEnergyBinning);
NEW_NuHepMC_EXCEPT(CouldNotOpenInputFile);
NEW_NuHepMC_EXCEPT(EC2NotSignalled);

SplineBuilder::SplineBuilder(std::vector<double> const &binning_MeV,
                             Units::XSUnits ut, Units::XSTargetScale ts)
    : EnergyBinning_MeV(binning_MeV), units(ut), targ_scale(ts) {

  if (EnergyBinning_MeV.size() < 2) {
    throw InvalidEnergyBinning()
        << "SplineBuilder constructor recieved empty binning descriptor.";
  }

  for (size_t i = 1; i < EnergyBinning_MeV.size(); ++i) {
    if (EnergyBinning_MeV[i] <= EnergyBinning_MeV[i - 1]) {
      throw InvalidEnergyBinning()
          << "SplineBuilder constructor recieved invalid binning defintion. "
             "bin: "
          << i << " edge: " << EnergyBinning_MeV[i] << " <= "
          << "bin: " << (i - 1) << " edge: " << EnergyBinning_MeV[i - 1];
    }
    XSSamples.emplace_back();
  }
}

void SplineBuilder::AddSample(double Enu_MeV, double XS) {

  // enu outside of binned range
  if ((Enu_MeV < EnergyBinning_MeV.front()) ||
      (Enu_MeV >= EnergyBinning_MeV.back())) {
    return;
  }

  auto bin_it =
      std::distance(EnergyBinning_MeV.begin(),
                    std::upper_bound(EnergyBinning_MeV.begin(),
                                     EnergyBinning_MeV.end(), Enu_MeV)) -
      1;
  XSSamples[bin_it].push_back(XS);
}

Spline SplineBuilder::ToSpline() const {
  Spline spl;
  spl.units = units;
  spl.targ_scale = targ_scale;

  for (size_t i = 1; i < EnergyBinning_MeV.size(); ++i) {
    double bin_centre = (EnergyBinning_MeV[i] + EnergyBinning_MeV[i - 1]) / 2.0;

    // KahanSum from https://en.wikipedia.org/wiki/Kahan_summation_algorithm
    double sum = 0.0; // Prepare the accumulator.
    double c = 0.0;   // A running compensation for lost low-order bits.

    for (size_t j = 0; j < XSSamples[i - 1].size();
         ++j) { // The array input has elements indexed input[1] to
                // input[input.length].
      double y = XSSamples[i - 1][j] - c; // c is zero the first time around.
      double t = sum + y; // Alas, sum is big, y small, so low-order digits
                          // of y are lost.
      c = (t - sum) - y;  // (t - sum) cancels the high-order part of y;
                          // subtracting y recovers negative (low part of y)
      sum = t;            // Algebraically, c should always be zero. Beware
                          // overly-aggressive optimizing compilers!
    } // Next time around, the lost low part will be added to y in a fresh
      // attempt.
    double avg_xs =
        XSSamples[i - 1].size() ? (sum / double(XSSamples[i - 1].size())) : 0;
    spl.points.emplace_back(bin_centre, avg_xs);
  }
  return spl;
}

NuTgtProcSplineMap BuildSplines(std::string const &Filename,
                                std::vector<double> EnergyBinning_MeV) {

  auto rdr = HepMC3::deduce_reader(Filename);
  if (!rdr) {
    throw CouldNotOpenInputFile()
        << "Failed to instantiate HepMC3::Reader from " << Filename;
  }

  std::map<int, std::map<int, std::map<int, SplineBuilder>>> spl_builders;

  std::map<int, NuHepMC::GC7::EnergyDistribution> energy_distributions;

  bool have_EC3 = false;
  double to_MeV = 1;

  HepMC3::GenEvent evt;

  size_t NEvents = 0;
  while (!rdr->failed()) {
    rdr->read_event(evt);

    if (!NEvents) { // first event
      if (!NuHepMC::GC1::SignalsConvention(evt.run_info(), "E.C.2")) {
        throw EC2NotSignalled()
            << " when running BuildSplines(" << Filename << ")";
      }

      to_MeV = Event::ToMeVFactor(evt);

      have_EC3 = NuHepMC::GC1::SignalsConvention(evt.run_info(), "E.C.3");

      if (EnergyBinning_MeV.size() < 2) {
        if (NuHepMC::GC1::SignalsConvention(evt.run_info(), "G.C.7")) {
          energy_distributions =
              NuHepMC::GC7::ReadAllEnergyDistributions(evt.run_info());

        } else {
          throw IndeterminableEnergyBinning()
              << "Input file " << Filename
              << " didn't signal G.C.7 and not energy binning was passed";
        }
      }
    }

    if (!rdr->failed()) {
      NEvents++;
    } else {
      break;
    }

    auto beam_part = NuHepMC::Event::GetBeamParticle(evt);
    auto beam_part_id = beam_part->pid();
    auto tgt_part_id = NuHepMC::Event::GetTargetParticle(evt)->pid();

    double totxs = NuHepMC::EC2::ReadTotalCrossSection(evt);

    // use procid 0 for the total xs
    if (!spl_builders[beam_part_id][tgt_part_id].count(0)) {
      try {
        spl_builders[beam_part_id][tgt_part_id].emplace(
            0, energy_distributions.count(beam_part_id)
                   ? energy_distributions[beam_part_id].bin_edges
                   : EnergyBinning_MeV);
      } catch (NuHepMC::except ex) {
        throw ex << " for beam pid: " << beam_part_id;
      }
    }

    spl_builders[beam_part_id][tgt_part_id].at(0).AddSample(
        beam_part->momentum().e() * to_MeV, totxs);

    if (have_EC3) {
      int procid = NuHepMC::ER3::ReadProcessID(evt);
      double procxs = NuHepMC::EC3::ReadProcessCrossSection(evt);

      if (!spl_builders[beam_part_id][tgt_part_id].count(procid)) {
        try {
          spl_builders[beam_part_id][tgt_part_id].emplace(
              procid, energy_distributions.count(beam_part_id)
                          ? energy_distributions[beam_part_id].bin_edges
                          : EnergyBinning_MeV);
        } catch (NuHepMC::except ex) {
          throw ex << " for beam pid: " << beam_part_id;
        }
      }

      spl_builders[beam_part_id][tgt_part_id].at(procid).AddSample(
          beam_part->momentum().e() * to_MeV, procxs);
    }
  }

  NuTgtProcSplineMap spl_map;
  for (auto const &beam_map : spl_builders) {
    for (auto const &tgt_map : beam_map.second) {
      for (auto const &proc_splbuilders : tgt_map.second) {
        spl_map[beam_map.first][tgt_map.first].emplace(
            proc_splbuilders.first, proc_splbuilders.second.ToSpline());
      }
    }
  }

  return spl_map;
}

double CalculateFluxAveragedTotalCrossSection(std::string const &Filename,
                                              Units::XSUnits,
                                              Units::XSTargetScale) {}

} // namespace CrossSection
} // namespace NuHepMC