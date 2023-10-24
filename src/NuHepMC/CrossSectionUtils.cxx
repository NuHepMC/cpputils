// // Leave this at the top to enable features detected at build time in headers in 
// // HepMC3
#include "NuHepMC/HepMC3Features.hxx"

#include "NuHepMC/CrossSectionUtils.hxx"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/Exceptions.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "HepMC3/ReaderFactory.h"

#include <algorithm>

namespace NuHepMC {

namespace CrossSection {

struct SplineBuilder {

  std::vector<double> EnergyBinning_MeV;
  std::vector<std::vector<double>> XSSamples;

  Units::XSUnits units;
  Units::XSTargetScale targ_scale;

  SplineBuilder(std::vector<double> const &binning_MeV,
                Units::XSUnits ut = Units::XSUnits::pb,
                Units::XSTargetScale ts = Units::XSTargetScale::PerTargetAtom);

  void AddSample(double Enu_MeV, double XS);

  Spline ToSpline() const;
};

NEW_NuHepMC_EXCEPT(InvalidEnergyBinning);
NEW_NuHepMC_EXCEPT(IndeterminableEnergyBinning);
NEW_NuHepMC_EXCEPT(UnknownEnergyDistribution);
NEW_NuHepMC_EXCEPT(CouldNotOpenInputFile);
NEW_NuHepMC_EXCEPT(EC2NotSignalled);
NEW_NuHepMC_EXCEPT(EC4NotSignalled);
NEW_NuHepMC_EXCEPT(GC5NotSignalled);
NEW_NuHepMC_EXCEPT(GC7NotSignalled);
NEW_NuHepMC_EXCEPT(NoMethodToCalculateFATX);
NEW_NuHepMC_EXCEPT(NonStandardXSUnitsUsed);

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

struct KBAccumulator {
  double sum;
  double corr;
  KBAccumulator(double s = 0, double c = 0) : sum(s), corr(c) {}

  KBAccumulator &operator+=(double el) {
    double y = el - corr;
    double t = sum + y;

    corr = (t - sum) - y;
    sum = t;

    return *this;
  }
  KBAccumulator operator+(double el) {
    *this += el;
    return *this;
  }
};

Spline SplineBuilder::ToSpline() const {
  Spline spl;
  spl.units = units;
  spl.targ_scale = targ_scale;

  for (size_t i = 1; i < EnergyBinning_MeV.size(); ++i) {
    double bin_centre = (EnergyBinning_MeV[i] + EnergyBinning_MeV[i - 1]) / 2.0;

    KBAccumulator ka;

    for (size_t j = 0; j < XSSamples[i - 1].size(); ++j) {
      ka += XSSamples[i - 1][j];
    }
    double avg_xs = XSSamples[i - 1].size()
                        ? (ka.sum / double(XSSamples[i - 1].size()))
                        : 0;
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

std::map<int, std::map<int, double>>
CalculateFluxAveragedTotalCrossSectionFromSplinesAndFlux(
    std::string const &Filename) {

  auto rdr = HepMC3::deduce_reader(Filename);
  if (!rdr) {
    throw CouldNotOpenInputFile()
        << "Failed to instantiate HepMC3::Reader from " << Filename;
  }
  HepMC3::GenEvent evt;
  rdr->read_event(evt);
  if (!NuHepMC::GC1::SignalsConvention(evt.run_info(), "E.C.2")) {
    throw EC2NotSignalled()
        << " when running "
           "CalculateFluxAveragedTotalCrossSectionFromSplinesAndFlux("
        << Filename << ")";
  }
  if (!NuHepMC::GC1::SignalsConvention(evt.run_info(), "G.C.7")) {
    throw GC7NotSignalled()
        << " when running "
           "CalculateFluxAveragedTotalCrossSectionFromSplinesAndFlux("
        << Filename << ")";
  }

  auto splines = BuildSplines(Filename);
  auto fluxes = NuHepMC::GC7::ReadAllEnergyDistributions(evt.run_info());

  std::map<int, std::map<int, double>> fatxs;

  for (auto const &beam_map : splines) {
    if (!fluxes.count(beam_map.first)) {
      throw UnknownEnergyDistribution() << "CalculateFluxAveragedTotalCrossSect"
                                           "ionFromSplinesAndFlux for beam id: "
                                        << beam_map.first;
    }

    for (auto const &tgt_map : beam_map.second) {
      auto nu_bin_contents = fluxes[beam_map.first].GetContentCount();

      KBAccumulator numer, denom;

      for (size_t i = 0; i < nu_bin_contents.size(); ++i) {
        numer += tgt_map.second.at(0).points[i].second * nu_bin_contents[i];
        denom += nu_bin_contents[i];
      }

      fatxs[beam_map.first][tgt_map.first] = numer.sum / denom.sum;
    }
  }
  return fatxs;
}

std::map<int, std::map<int, double>>
CalculateFluxAveragedTotalCrossSection(std::string const &Filename,
                                       Units::XSUnits, Units::XSTargetScale) {

  auto rdr = HepMC3::deduce_reader(Filename);
  if (!rdr) {
    throw CouldNotOpenInputFile()
        << "Failed to instantiate HepMC3::Reader from " << Filename;
  }
  double to_MeV = 1;
  HepMC3::GenEvent evt;

  std::map<int, std::map<int, std::pair<KBAccumulator, KBAccumulator>>>
      accumulators;

  size_t NEvents = 0;
  while (!rdr->failed()) {
    rdr->read_event(evt);

    if (!NEvents) { // first event
      if (!NuHepMC::GC1::SignalsConvention(evt.run_info(), "E.C.2")) {
        throw EC2NotSignalled()
            << " when running CalculateFluxAveragedTotalCrossSection("
            << Filename << ")";
      }

      to_MeV = Event::ToMeVFactor(evt);
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

    double denom = evt.weights()[0];
    double numer = denom / totxs;

    accumulators[beam_part_id][tgt_part_id].first += numer;
    accumulators[beam_part_id][tgt_part_id].second += denom;

    accumulators[0][0].first += numer;
    accumulators[0][0].second += denom;
  }

  std::map<int, std::map<int, double>> FATXs;

  for (auto const &beam_map : accumulators) {
    for (auto const &tgt_map : beam_map.second) {
      FATXs[beam_map.first][tgt_map.first] =
          tgt_map.second.second.sum / tgt_map.second.first.sum;
    }
  }
  FATXs[0][0] = accumulators[0][0].second.sum / accumulators[0][0].first.sum;

  return FATXs;
}

std::tuple<double, std::string, std::string>
GetGC5FluxAveragedTotalCrossSection(std::string const &Filename) {
  auto rdr = HepMC3::deduce_reader(Filename);
  if (!rdr) {
    throw CouldNotOpenInputFile()
        << "Failed to instantiate HepMC3::Reader from " << Filename;
  }
  HepMC3::GenEvent evt;
  rdr->read_event(evt);
  if (!NuHepMC::GC1::SignalsConvention(evt.run_info(), "G.C.5")) {
    throw GC5NotSignalled()
        << " when running GetGC5FluxAveragedTotalCrossSection(" << Filename
        << ")";
  }

  auto gc4_units = NuHepMC::GC4::ReadCrossSectionUnits(evt.run_info());

  return std::tuple<double, std::string, std::string>{
      GC5::ReadFluxAveragedTotalXSec(evt.run_info()), gc4_units.first,
      gc4_units.second};
}

double
GetXSUnitsRescaleFactor(std::pair<Units::XSUnits, Units::XSTargetScale> from,
                        std::pair<Units::XSUnits, Units::XSTargetScale> to,
                        HepMC3::GenEvent &evt) {

  if ((from.first == Units::XSUnits::CustomType) ||
      (from.second == Units::XSTargetScale::CustomType)) {
    auto units_str = NuHepMC::GC4::ReadCrossSectionUnits(evt.run_info());
    throw NonStandardXSUnitsUsed()
        << units_str.first << " " << units_str.second;
  }
  if ((to.first == Units::XSUnits::CustomType) ||
      (to.second == Units::XSTargetScale::CustomType)) {
    auto units_str = NuHepMC::GC4::ReadCrossSectionUnits(evt.run_info());
    throw NonStandardXSUnitsUsed()
        << units_str.first << " " << units_str.second;
  }

  if ((from.second == Units::XSTargetScale::PerTargetMolecule) ||
      (to.second == Units::XSTargetScale::PerTargetMolecule)) {
    throw NonStandardXSUnitsUsed() << "Cannot automatically convert between "
                                      "units including PerTargetMolecule";
  }

  double sf = 1;

  if (from.first != to.first) {
    static std::map<Units::XSUnits, double> const xsunit_factors = {
        {Units::XSUnits::pb, Units::pb},
        {Units::XSUnits::cm2, Units::cm2},
        {Units::XSUnits::cm2_ten38, Units::cm2_ten38},
    };
    // pb -> cm2 : 1E-36
    // 1      1E36
    //   1 / 1E36 = 1E-36!
    sf *= xsunit_factors.at(from.first) / xsunit_factors.at(to.first);
  }

  if (from.second != to.second) {
    auto tgt_part_id = NuHepMC::Event::GetTargetParticle(evt)->pid();
    std::map<Units::XSTargetScale, double> tsunit_factors = {
        {Units::XSTargetScale::PerTargetAtom,
         1.0 / ((tgt_part_id / 10) % 1000)},
        {Units::XSTargetScale::PerTargetNucleon, 1.0},
        {Units::XSTargetScale::PerTargetMolecularNucleon, 1.0},
    };

    sf *= tsunit_factors.at(from.second) / tsunit_factors.at(to.second);
  }

  return sf;
}

std::tuple<double, std::string, std::string>
CalculateFluxAveragedTotalCrossSectionEC4(std::string const &Filename) {

  auto rdr = HepMC3::deduce_reader(Filename);
  if (!rdr) {
    throw CouldNotOpenInputFile()
        << "Failed to instantiate HepMC3::Reader from " << Filename;
  }

  double to_MeV = 1;
  HepMC3::GenEvent evt;

  double xsec, ntrials;

  KBAccumulator calc_xsec;

  std::pair<std::string, std::string> gc4_units;

  size_t NEvents = 0;
  while (!rdr->failed()) {
    rdr->read_event(evt);

    if (!NEvents) { // first event
      if (!NuHepMC::GC1::SignalsConvention(evt.run_info(), "E.C.4")) {
        throw EC4NotSignalled()
            << " when running CalculateFluxAveragedTotalCrossSection("
            << Filename << ")";
      }

      to_MeV = Event::ToMeVFactor(evt);
      gc4_units = NuHepMC::GC4::ReadCrossSectionUnits(evt.run_info());
    }

    if (!rdr->failed()) {
      NEvents++;
    } else {
      break;
    }

    calc_xsec += evt.weights()[0];

    auto tmp = evt.cross_section();
    xsec = tmp->xsec();
    ntrials = tmp->get_attempted_events();
  }

  if (std::fabs(xsec - (calc_xsec.sum / ntrials)) > 1e-8) {
    std::cerr << "[WARN] Calculated xsec different than stored\n";
  }

  return std::tuple<double, std::string, std::string>{xsec, gc4_units.first,
                                                      gc4_units.second};
}

double GetFATX(std::string const &Filename, Units::XSUnits ut,
               Units::XSTargetScale ts) {

  auto rdr = HepMC3::deduce_reader(Filename);
  if (!rdr) {
    throw CouldNotOpenInputFile()
        << "Failed to instantiate HepMC3::Reader from " << Filename;
  }
  HepMC3::GenEvent evt;
  rdr->read_event(evt);

  auto units = NuHepMC::GC4::ParseCrossSectionUnits(evt.run_info());

  if ((units.first == Units::XSUnits::CustomType) ||
      (units.second == Units::XSTargetScale::CustomType)) {
    auto units_str = NuHepMC::GC4::ReadCrossSectionUnits(evt.run_info());
    throw NonStandardXSUnitsUsed()
        << units_str.first << " " << units_str.second;
  }

  auto beam_part = NuHepMC::Event::GetBeamParticle(evt);
  auto beam_part_id = beam_part->pid();
  auto tgt_part_id = NuHepMC::Event::GetTargetParticle(evt)
                         ? NuHepMC::Event::GetTargetParticle(evt)->pid()
                         : 0;

  double units_rescale = GetXSUnitsRescaleFactor(units, {ut, ts}, evt);

  if (GC1::SignalsConvention(evt.run_info(), "G.C.5")) {
    auto xs = std::get<0>(GetGC5FluxAveragedTotalCrossSection(Filename)) *
              units_rescale;
    std::cout << "-- FATX from GC5 = " << xs << "(" << (xs / units_rescale)
              << "*" << units_rescale << ", u: [" << units.first << ","
              << units.second << "] -> [" << ut << ", " << ts << "])"
              << std::endl;
    return xs;
  }

  if (GC1::SignalsConventions(evt.run_info(), {
                                                  "E.C.2",
                                              })) {
    auto xs = CalculateFluxAveragedTotalCrossSection(
                  Filename)[beam_part_id][tgt_part_id] *
              units_rescale;
    std::cout << "-- FATX from EC2 = " << xs << "(" << (xs / units_rescale)
              << "*" << units_rescale << ", u: [" << units.first << ","
              << units.second << "] -> [" << ut << ", " << ts << "])"
              << std::endl;
    return xs;
  }

  if (GC1::SignalsConventions(evt.run_info(), {
                                                  "E.C.4",
                                              })) {

    auto xs = std::get<0>(CalculateFluxAveragedTotalCrossSectionEC4(Filename)) *
              units_rescale;
    std::cout << "-- FATX from EC4 = " << xs << "(" << (xs / units_rescale)
              << "*" << units_rescale << ", u: [" << units.first << ","
              << units.second << "] -> [" << ut << ", " << ts << "])"
              << std::endl;
    return xs;
  }

  throw NoMethodToCalculateFATX();
}

} // namespace CrossSection
} // namespace NuHepMC
