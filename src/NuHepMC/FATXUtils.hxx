#pragma once

#include "NuHepMC/UnitsUtils.hxx"

namespace HepMC3 {
class GenEvent;
class GenRunInfo;
} // namespace HepMC3

namespace NuHepMC {

namespace FATX {

// ABC for FATX accumulators that can give their best estimate of the FATX after
// being passed N events
// Some subclasses will know the best estimate after one event and others will
// continue to get better the more events you pass them
struct Accumulator {
  // returns the cv weight for the proferred event and accumulates FATX
  // information in an input-dependent way
  virtual double process(HepMC3::GenEvent const &) = 0;
  // retrieve the best estimate of the fatx in the desired units
  virtual double fatx(CrossSection::Units::Unit const &units =
                          CrossSection::Units::pb_PerAtom) const = 0;
  virtual double sumweights() const = 0;
  virtual size_t events() const = 0;

  virtual std::string to_string() const = 0;

  virtual int TargetTotalNucleons() const = 0;
  virtual int TargetTotalProtons() const = 0;
  virtual int TargetTotalNeutrons() const = 0;

  virtual double TargetAverageA() const = 0;
  virtual double TargetAverageZ() const = 0;
  virtual double TargetAverageN() const = 0;

  virtual ~Accumulator() {}
};

std::shared_ptr<Accumulator>
MakeAccumulator(std::shared_ptr<HepMC3::GenRunInfo> gri);

// Can pass Convention = "Dummy" to instantiate an accumulator that
// just counts events.
std::shared_ptr<Accumulator> MakeAccumulator(std::string const &Convention);

} // namespace FATX

} // namespace NuHepMC