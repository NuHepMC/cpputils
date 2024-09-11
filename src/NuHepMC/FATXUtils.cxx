#include "NuHepMC/FATXUtils.hxx"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenRunInfo.h"

namespace NuHepMC {

namespace FATX {

template <typename T = double> class KBAccumulator {
  T sum;
  T corr;

public:
  KBAccumulator() : sum(0), corr(0) {}

  void operator()(T el) {
    T y = el - corr;
    T t = sum + y;

    corr = (t - sum) - y;
    sum = t;
  }
  T operator()() const { return sum; }
};

struct BaseAccumulator : public Accumulator {

  KBAccumulator<double> sumw;
  std::map<int, KBAccumulator<double>> targets_sumw;

  CrossSection::Units::Unit input_unit;
  size_t nevt;
  int cvweight_index;

  BaseAccumulator(int cvwi = -1)
      : sumw(), input_unit{CrossSection::Units::automatic}, nevt{0},
        cvweight_index{cvwi} {}

  double process(HepMC3::GenEvent const &ev) {
    double w = (cvweight_index == -1) ? 1 : ev.weights()[cvweight_index];
    sumw(w);
    targets_sumw[Event::GetTargetPDG(ev)](w);
    nevt++;

    if (input_unit == CrossSection::Units::automatic) {
      input_unit = NuHepMC::GC4::ParseCrossSectionUnits(ev.run_info());

      if ((input_unit.scale == CrossSection::Units::Scale::CustomType) ||
          (input_unit.tgtscale ==
           CrossSection::Units::TargetScale::CustomType)) {
        auto units_str = NuHepMC::GC4::ReadCrossSectionUnits(ev.run_info());
        throw CrossSection::Units::NonStandardUnitsUsed()
            << units_str.first << " " << units_str.second;
      }
    }

    return w;
  }

  double units_scale_factor(CrossSection::Units::Unit const &to) const {
    double sf = 1;

    if (input_unit.scale != to.scale) {
      static std::map<CrossSection::Units::Scale, double> const xsunit_factors =
          {
              {CrossSection::Units::Scale::pb, CrossSection::Units::pb},
              {CrossSection::Units::Scale::cm2, CrossSection::Units::cm2},
              {CrossSection::Units::Scale::cm2_ten38,
               CrossSection::Units::cm2_ten38},
          };
      // pb -> cm2 : 1E-36
      // 1      1E36
      //   1 / 1E36 = 1E-36!
      sf *= xsunit_factors.at(input_unit.scale) / xsunit_factors.at(to.scale);
    }
    return sf;
  }

  std::string to_string() const {
    std::stringstream ss;
    ss << "nevt: " << nevt << std::endl;
    ss << "sumw: " << sumw() << std::endl;
    if (targets_sumw.size() > 1) {
      ss << "targets_sumw: " << std::endl;
      ss << "  Target PDG  sumw" << std::endl;
      ss << "  ----------------" << std::endl;
      for (auto const &[tgt_pid, tgt_sumw] : targets_sumw) {
        ss << "  " << tgt_pid << "  " << tgt_sumw() << std::endl;
      }
    }
    ss << "input_unit: " << input_unit << std::endl;
    ss << "fatx: " << fatx(CrossSection::Units::pb_PerAtom)
       << " pb/TargetAtom, " << fatx(CrossSection::Units::cm2ten38_PerNucleon)
       << " cm^2 * 10^-38/Nucleon" << std::endl;

    return ss.str();
  }

  double sumweights() const { return sumw(); }
  size_t events() const { return nevt; }
};

struct DummyAccumulator : public Accumulator {
  size_t nevt;
  DummyAccumulator() : nevt{0} {}
  double process(HepMC3::GenEvent const &) {
    nevt++;
    return 1;
  }
  double fatx(CrossSection::Units::Unit const &) const { return 1; }

  double sumweights() const { return nevt; }
  size_t events() const { return nevt; }
  std::string to_string() const { return "DummyAccumulator"; }
};

// This just reads the FATX from the run_info
// Need to accumulate target fractions as we go to allow unit conversion later
struct GC5Accumulator : public BaseAccumulator {

  double GC5FATX;

  GC5Accumulator(int cvwi = -1) : BaseAccumulator(cvwi), GC5FATX{0xdeadbeef} {}

  double process(HepMC3::GenEvent const &ev) {
    double w = BaseAccumulator::process(ev);

    if (GC5FATX == 0xdeadbeef) {
      GC5FATX = GC5::ReadFluxAveragedTotalXSec(ev.run_info());
    }
    return w;
  }

  double fatx(CrossSection::Units::Unit const &units) const {
    if (units == input_unit) {
      return GC5FATX;
    }

    double sf = units_scale_factor(units);

    // for GC5 /Nucleon or /MolecularNucleon are equivalent
    if ((units.tgtscale == input_unit.tgtscale) ||
        ((input_unit.tgtscale ==
          CrossSection::Units::TargetScale::PerTargetMolecularNucleon) &&
         (units.tgtscale ==
          CrossSection::Units::TargetScale::PerTargetNucleon)) ||
        ((units.tgtscale ==
          CrossSection::Units::TargetScale::PerTargetMolecularNucleon) &&
         (input_unit.tgtscale ==
          CrossSection::Units::TargetScale::PerTargetNucleon))) {
      return GC5FATX * sf;
    }

    if ((input_unit.tgtscale ==
         CrossSection::Units::TargetScale::PerTargetAtom) &&
        (units.tgtscale ==
         CrossSection::Units::TargetScale::PerTargetNucleon)) {

      double RescaledFATX = 0;

      // For some sub-component of the cross section, A:
      //   Know that FATXA/FATX = SumWeightsA/SumWeightsAll
      // as events are generated according to their cross section

      for (auto const &[tgt_pid, tgt_sumw] : targets_sumw) {
        RescaledFATX += (GC5FATX * sf) * (tgt_sumw() / sumw()) /
                        CrossSection::Units::NuclearPDGToA(tgt_pid);
      }

      return RescaledFATX;

      // for GC5 /Nucleon or /MolecularNucleon are equivalent
    } else if (((input_unit.tgtscale ==
                 CrossSection::Units::TargetScale::PerTargetNucleon) ||
                (input_unit.tgtscale == CrossSection::Units::TargetScale::
                                            PerTargetMolecularNucleon)) &&
               (units.tgtscale ==
                CrossSection::Units::TargetScale::PerTargetAtom)) {

      double RescaledFATX = 0;

      // For some sub-component of the cross section, A:
      //   Know that FATXA * NucA / (FATX * NucTotal) =
      //   SumWeightsA/SumWeightsAll
      // as events are generated according to their cross section

      int NTot = 0;
      for (auto const &[tgt_pid, tgt_sumw] : targets_sumw) {
        NTot += CrossSection::Units::NuclearPDGToA(tgt_pid);
      }
      for (auto const &[tgt_pid, tgt_sumw] : targets_sumw) {
        RescaledFATX += (GC5FATX * sf * NTot) * (tgt_sumw() / sumw());
      }

      return RescaledFATX;
    }

    throw CrossSection::Units::InvalidUnits()
        << "GC5Accumulator can only provide FATX in /TargetAtom or "
           "/TargetNucleon currently for inputs in the same two. If you "
           "require other units you may have to convert them yourself. The "
           "input unit for this file was: "
        << input_unit << ", and the requested output unit was: " << units;
  }

  std::string to_string() const {
    std::stringstream ss;
    ss << BaseAccumulator::to_string();
    ss << "GC5FATX: " << GC5FATX << std::endl;
    return ss.str();
  }
};

// This constructs the FATX from the total cross section for each event
// keep track per target and combine upon request to allow unit conversions
struct EC2Accumulator : public BaseAccumulator {

  KBAccumulator<double> ReciprocalTotXS;
  std::map<int, KBAccumulator<double>> targets_ReciprocalTotXS;

  EC2Accumulator(int cvwi = -1) : BaseAccumulator(cvwi), ReciprocalTotXS() {}

  double process(HepMC3::GenEvent const &ev) {
    double w = BaseAccumulator::process(ev);

    auto tgt_pdg = Event::GetTargetPDG(ev);

    auto totxs_recip = w / EC2::ReadTotalCrossSection(ev);

    ReciprocalTotXS(totxs_recip);
    targets_ReciprocalTotXS[tgt_pdg](totxs_recip);

    return w;
  }

  double fatx(CrossSection::Units::Unit const &units) const {

    if (units == input_unit) {
      return sumw() / ReciprocalTotXS();
    }

    double sf = units_scale_factor(units);

    if (units.tgtscale == input_unit.tgtscale) {
      return (sumw() / ReciprocalTotXS()) * sf;
    }

    if ((input_unit.tgtscale ==
         CrossSection::Units::TargetScale::PerTargetAtom) &&
        (units.tgtscale ==
         CrossSection::Units::TargetScale::PerTargetNucleon)) {

      double RescaledFATX = 0;

      // For some sub-component of the cross section, A:
      //   Know that FATXA/FATX = SumWeightsA/SumWeightsAll
      // as events are generated according to their cross section

      for (auto const &[tgt_pid, tgt_rtxs] : targets_ReciprocalTotXS) {
        RescaledFATX += ((targets_sumw.at(tgt_pid)() / tgt_rtxs()) * sf) *
                        (targets_sumw.at(tgt_pid)() / sumw()) /
                        CrossSection::Units::NuclearPDGToA(tgt_pid);
      }

      return RescaledFATX;

    } else if ((input_unit.tgtscale ==
                CrossSection::Units::TargetScale::PerTargetMolecularNucleon) &&
               (units.tgtscale ==
                CrossSection::Units::TargetScale::PerTargetAtom)) {

      double RescaledFATX = 0;

      // For some sub-component of the cross section, A:
      //   Know that FATXA * NucA / (FATX * NucTotal) =
      //   SumWeightsA/SumWeightsAll
      // as events are generated according to their cross section

      int NTot = 0;
      for (auto const &[tgt_pid, tgt_rtxs] : targets_ReciprocalTotXS) {
        NTot += CrossSection::Units::NuclearPDGToA(tgt_pid);
      }
      for (auto const &[tgt_pid, tgt_rtxs] : targets_ReciprocalTotXS) {
        RescaledFATX +=
            ((targets_sumw.at(tgt_pid)() / tgt_rtxs()) * sf * NTot) *
            (targets_sumw.at(tgt_pid)() / sumw());
      }

      return RescaledFATX;
    }

    throw CrossSection::Units::InvalidUnits()
        << "GC5Accumulator can only provide FATX in /TargetAtom or "
           "/TargetNucleon currently for inputs in the same two. If you "
           "require other units you may have to convert them yourself. The "
           "input unit for this file was: "
        << input_unit << ", and the requested output unit was: " << units;
  }

  std::string to_string() const {
    std::stringstream ss;
    ss << BaseAccumulator::to_string();
    if (targets_ReciprocalTotXS.size() > 1) {
      ss << "targets_sumw / targets_ReciprocalTotXS: " << std::endl;
      ss << "  Target PDG  sumw/ReciprocalTotXS" << std::endl;
      ss << "  --------------------------------" << std::endl;
      for (auto const &[tgt_pid, tgt_rtxs] : targets_ReciprocalTotXS) {
        ss << "  " << tgt_pid << "  "
           << (targets_sumw.at(tgt_pid)() / tgt_rtxs()) << std::endl;
      }
    }
    return ss.str();
  }
};

// This reads the FATX from the last event
// Need to accumulate target fractions as we go to allow unit conversion later
struct EC4Accumulator : public BaseAccumulator {

  double EC4BestEstimate;

  EC4Accumulator(int cvwi = -1)
      : BaseAccumulator(cvwi), EC4BestEstimate(0xdeadbeef) {}

  double process(HepMC3::GenEvent const &ev) {
    double w = BaseAccumulator::process(ev);

    EC4BestEstimate = ev.cross_section()->xsec();
    return w;
  }

  double fatx(CrossSection::Units::Unit const &units) const {
    if (units == input_unit) {
      return EC4BestEstimate;
    }

    double sf = units_scale_factor(units);

    // for EC4 /Nucleon or /MolecularNucleon are equivalent
    if ((units.tgtscale == input_unit.tgtscale) ||
        ((input_unit.tgtscale ==
          CrossSection::Units::TargetScale::PerTargetMolecularNucleon) &&
         (units.tgtscale ==
          CrossSection::Units::TargetScale::PerTargetNucleon)) ||
        ((units.tgtscale ==
          CrossSection::Units::TargetScale::PerTargetMolecularNucleon) &&
         (input_unit.tgtscale ==
          CrossSection::Units::TargetScale::PerTargetNucleon))) {
      return EC4BestEstimate * sf;
    }

    if ((input_unit.tgtscale ==
         CrossSection::Units::TargetScale::PerTargetAtom) &&
        (units.tgtscale ==
         CrossSection::Units::TargetScale::PerTargetNucleon)) {

      double RescaledFATX = 0;

      // For some sub-component of the cross section, A:
      //   Know that FATXA/FATX = SumWeightsA/SumWeightsAll
      // as events are generated according to their cross section

      for (auto const &[tgt_pid, tgt_sumw] : targets_sumw) {
        RescaledFATX += (EC4BestEstimate * sf) * (tgt_sumw() / sumw()) /
                        CrossSection::Units::NuclearPDGToA(tgt_pid);
      }

      return RescaledFATX;

      // for EC4 /Nucleon or /MolecularNucleon are equivalent
    } else if (((input_unit.tgtscale ==
                 CrossSection::Units::TargetScale::PerTargetNucleon) ||
                (input_unit.tgtscale == CrossSection::Units::TargetScale::
                                            PerTargetMolecularNucleon)) &&
               (units.tgtscale ==
                CrossSection::Units::TargetScale::PerTargetAtom)) {

      double RescaledFATX = 0;

      // For some sub-component of the cross section, A:
      //   Know that FATXA * NucA / (FATX * NucTotal) =
      //   SumWeightsA/SumWeightsAll
      // as events are generated according to their cross section

      int NTot = 0;
      for (auto const &[tgt_pid, tgt_sumw] : targets_sumw) {
        NTot += CrossSection::Units::NuclearPDGToA(tgt_pid);
      }
      for (auto const &[tgt_pid, tgt_sumw] : targets_sumw) {
        RescaledFATX += (EC4BestEstimate * sf * NTot) * (tgt_sumw() / sumw());
      }

      return RescaledFATX;
    }

    throw CrossSection::Units::InvalidUnits()
        << "GC5Accumulator can only provide FATX in /TargetAtom or "
           "/TargetNucleon currently for inputs in the same two. If you "
           "require other units you may have to convert them yourself. The "
           "input unit for this file was: "
        << input_unit << ", and the requested output unit was: " << units;
  }

  std::string to_string() const {
    std::stringstream ss;
    ss << BaseAccumulator::to_string();
    ss << "EC4BestEstimate: " << EC4BestEstimate << std::endl;
    return ss.str();
  }
};

NEW_NuHepMC_EXCEPT(NoMethodToCalculateFATX);

std::shared_ptr<Accumulator>
MakeAccumulator(std::shared_ptr<HepMC3::GenRunInfo> gri) {
  if (GC1::SignalsConvention(gri, "G.C.5")) {
    return std::shared_ptr<Accumulator>(
        new GC5Accumulator(gri->weight_index("CV")));
  } else if (GC1::SignalsConvention(gri, "E.C.4")) {
    return std::shared_ptr<Accumulator>(
        new EC4Accumulator(gri->weight_index("CV")));
  } else if (GC1::SignalsConvention(gri, "E.C.2")) {
    return std::shared_ptr<Accumulator>(
        new EC2Accumulator(gri->weight_index("CV")));
  }

  throw NoMethodToCalculateFATX()
      << "GenRunInfo did not signal any of the possible FATX accumulator "
         "conventions. Can only use G.C.5, E.C.4.\nConventions signalled: "
      << CheckedAttributeValue<std::string>(gri, "NuHepMC.Conventions", "");
}

std::shared_ptr<Accumulator> MakeAccumulator(std::string const &Convention) {
  if (Convention == "G.C.5") {
    return std::shared_ptr<Accumulator>(new GC5Accumulator());
  } else if (Convention == "E.C.4") {
    return std::shared_ptr<Accumulator>(new EC4Accumulator());
  } else if (Convention == "E.C.2") {
    return std::shared_ptr<Accumulator>(new EC2Accumulator());
  } else if (Convention == "Dummy") {
    return std::shared_ptr<Accumulator>(new DummyAccumulator());
  }

  throw NoMethodToCalculateFATX() << "Convention: " << Convention
                                  << " passed. Can only use G.C.5, E.C.4, or "
                                     "E.C.2 to build a FATX accumulator.";
}

} // namespace FATX
} // namespace NuHepMC