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
  T operator()() { return sum; }
};

struct BaseAccumulator : public Accumulator {

  KBAccumulator<double> sumw;
  size_t nevt;

  BaseAccumulator() : sumw(), nevt{0} {}

  void process(HepMC3::GenEvent const &ev) {
    sumw(ev.weight("CV"));
    nevt++;
  }

  double sumweights() { return sumw(); }
  size_t events() { return nevt; }
};

struct GC5Accumulator : public BaseAccumulator {

  double GC5FATX;

  GC5Accumulator() : BaseAccumulator(), GC5FATX{0xdeadbeef} {}

  void process(HepMC3::GenEvent const &ev) {
    if (GC5FATX == 0xdeadbeef) {
      GC5FATX = GC5::ReadFluxAveragedTotalXSec(ev.run_info());
    }
    BaseAccumulator::process(ev);
  }

  double fatx() { return GC5FATX; }
};

struct EC2Accumulator : public BaseAccumulator {

  KBAccumulator<double> ReciprocalTotXS;

  EC2Accumulator() : BaseAccumulator(), ReciprocalTotXS() {}

  void process(HepMC3::GenEvent const &ev) {

    ReciprocalTotXS(ev.weight("CV") / EC2::ReadTotalCrossSection(ev));
    BaseAccumulator::process(ev);
  }

  double fatx() { return sumw() / ReciprocalTotXS(); }
};

struct EC4Accumulator : public BaseAccumulator {

  double EC4BestEstimate;

  EC4Accumulator() : BaseAccumulator(), EC4BestEstimate(0xdeadbeef) {}

  void process(HepMC3::GenEvent const &ev) {
    EC4BestEstimate = ev.cross_section()->xsec();
    BaseAccumulator::process(ev);
  }

  double fatx() { return EC4BestEstimate; }
};

NEW_NuHepMC_EXCEPT(NoMethodToCalculateFATX);

std::unique_ptr<Accumulator>
MakeAccumulator(std::shared_ptr<HepMC3::GenRunInfo> gri) {
  if (GC1::SignalsConvention(gri, "G.C.5")) {
    return std::unique_ptr<Accumulator>(new GC5Accumulator());
  } else if (GC1::SignalsConvention(gri, "E.C.4")) {
    return std::unique_ptr<Accumulator>(new EC4Accumulator());
  } else if (GC1::SignalsConvention(gri, "E.C.2")) {
    return std::unique_ptr<Accumulator>(new EC2Accumulator());
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