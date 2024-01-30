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
/// Some subclasses will know the best estimate after one events and others will
/// continue to get better the more events you pass them
struct Accumulator {
  virtual void operator()(HepMC3::GenEvent const &) = 0;
  virtual double fatx() = 0;
  virtual double sumweights() = 0;
  virtual size_t events() = 0;
};

std::unique_ptr<Accumulator>
MakeAccumulator(std::shared_ptr<HepMC3::GenRunInfo> gri);

std::unique_ptr<Accumulator> MakeAccumulator(std::string const &Convention);

} // namespace FATX

} // namespace NuHepMC