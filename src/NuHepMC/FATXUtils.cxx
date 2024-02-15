#include "NuHepMC/FATXUtils.hxx"

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
  CrossSection::Units::Unit input_unit;
  size_t nevt;
  int cvweight_index;

  BaseAccumulator(int cvwi = -1)
      : sumw(), input_unit{CrossSection::Units::automatic}, nevt{0},
        cvweight_index{cvwi} {}

  double process(HepMC3::GenEvent const &ev) {
    double w = (cvweight_index == -1) ? 1 : ev.weights()[cvweight_index];
    sumw(w);
    nevt++;

    if (input_unit == CrossSection::Units::automatic) {
      input_unit = NuHepMC::GC4::ParseCrossSectionUnits(ev.run_info());
    }

    return w;
  }

  double sumweights() const { return sumw(); }
  size_t events() const { return nevt; }
};

struct GC5Accumulator : public BaseAccumulator {

  double GC5FATX;

  GC5Accumulator(int cvwi = -1) : BaseAccumulator(cvwi), GC5FATX{0xdeadbeef} {}

  double process(HepMC3::GenEvent const &ev) {
    double w = BaseAccumulator::process(ev);

    if (GC5FATX == 0xdeadbeef) {
      GC5FATX = GC5::ReadFluxAveragedTotalXSec(ev.run_info()) *
                CrossSection::Units::GetRescaleFactor(
                    ev, input_unit, CrossSection::Units::pb_PerAtom);
    }
    return w;
  }

  double fatx(CrossSection::Units::Unit const &units) const {
    if (units != CrossSection::Units::pb_PerAtom) {
      throw CrossSection::Units::InvalidUnits()
          << "GC5Accumulator can only provide FATX in pb /Atom. If you require "
             "other units you will have to convert them yourself.";
    }
    return GC5FATX;
  }
};

struct EC2Accumulator : public BaseAccumulator {

  KBAccumulator<double> ReciprocalTotXS;

  EC2Accumulator(int cvwi = -1) : BaseAccumulator(cvwi), ReciprocalTotXS() {}

  double process(HepMC3::GenEvent const &ev) {
    double w = BaseAccumulator::process(ev);

    ReciprocalTotXS(ev.weight("CV") /
                    (EC2::ReadTotalCrossSection(ev) *
                     CrossSection::Units::GetRescaleFactor(
                         ev, input_unit, CrossSection::Units::pb_PerAtom)));
    return w;
  }

  double fatx(CrossSection::Units::Unit const &units) const {
    if (units != CrossSection::Units::pb_PerAtom) {
      throw CrossSection::Units::InvalidUnits()
          << "EC2Accumulator can only provide FATX in pb /Atom. If you require "
             "other units you will have to convert them yourself.";
    }
    return sumw() / ReciprocalTotXS();
  }
};

struct EC4Accumulator : public BaseAccumulator {

  double EC4BestEstimate;

  EC4Accumulator(int cvwi = -1)
      : BaseAccumulator(cvwi), EC4BestEstimate(0xdeadbeef) {}

  double process(HepMC3::GenEvent const &ev) {
    double w = BaseAccumulator::process(ev);

    EC4BestEstimate = ev.cross_section()->xsec() *
                      CrossSection::Units::GetRescaleFactor(
                          ev, input_unit, CrossSection::Units::pb_PerAtom);
    return w;
  }

  double fatx(CrossSection::Units::Unit const &units) const {
    if (units != CrossSection::Units::pb_PerAtom) {
      throw CrossSection::Units::InvalidUnits()
          << "EC4Accumulator can only provide FATX in pb /Atom. If you require "
             "other units you will have to convert them yourself.";
    }
    return EC4BestEstimate;
  }
};

NEW_NuHepMC_EXCEPT(NoMethodToCalculateFATX);

std::unique_ptr<Accumulator>
MakeAccumulator(std::shared_ptr<HepMC3::GenRunInfo> gri) {
  if (GC1::SignalsConvention(gri, "G.C.5")) {
    return std::unique_ptr<Accumulator>(
        new GC5Accumulator(gri->weight_index("CV")));
  } else if (GC1::SignalsConvention(gri, "E.C.4")) {
    return std::unique_ptr<Accumulator>(
        new EC4Accumulator(gri->weight_index("CV")));
  } else if (GC1::SignalsConvention(gri, "E.C.2")) {
    return std::unique_ptr<Accumulator>(
        new EC2Accumulator(gri->weight_index("CV")));
  }

  throw NoMethodToCalculateFATX()
      << "GenRunInfo did not signal any of the possible FATX accumulator "
         "conventions. Can only use G.C.5, E.C.4.\nConventions signalled: "
      << CheckedAttributeValue<std::string>(gri, "NuHepMC.Conventions", "");
}

std::unique_ptr<Accumulator> MakeAccumulator(std::string const &Convention) {
  if (Convention == "G.C.5") {
    return std::unique_ptr<Accumulator>(new GC5Accumulator());
  } else if (Convention == "E.C.4") {
    return std::unique_ptr<Accumulator>(new EC4Accumulator());
  } else if (Convention == "E.C.2") {
    return std::unique_ptr<Accumulator>(new EC2Accumulator());
  }

  throw NoMethodToCalculateFATX() << "Convention: " << Convention
                                  << " passed. Can only use G.C.5, E.C.4, or "
                                     "E.C.2 to build a FATX accumulator.";
}

} // namespace FATX
} // namespace NuHepMC