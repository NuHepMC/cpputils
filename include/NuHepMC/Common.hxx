#pragma once

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenRunInfo.h"

#include "NuHepMC/Traits.hxx"

#include <memory>
#include <string>

namespace NuHepMC {

template <typename T>
void add_attribute(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                   std::string const &name, T const &val) {
  run_info->add_attribute(
      name, std::make_shared<typename NuHepMC::attr_traits<T>::type>(val));
}

template <typename T>
void add_attribute(HepMC3::GenEvent &ge, std::string const &name,
                   T const &val) {
  ge.add_attribute(
      name, std::make_shared<typename NuHepMC::attr_traits<T>::type>(val));
}

} // namespace NuHepMC