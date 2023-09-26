#pragma once

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
#include "HepMC3/GenRunInfo.h"

#include "NuHepMC/Exceptions.hxx"
#include "NuHepMC/Traits.hxx"

#include <memory>
#include <string>

namespace NuHepMC {

NEW_NuHepMC_EXCEPT(CannotAddAttribute);

template <typename T>
void add_attribute(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                   std::string const &name, T const &val) {
  run_info->add_attribute(
      name, std::make_shared<typename NuHepMC::attr_traits<T>::type>(val));
}

template <typename T>
void add_attribute(HepMC3::GenParticlePtr &part, std::string const &name,
                   T const &val) {
  if (!part->in_event()) {
    throw CannotAddAttribute()
        << "particle id: " << part->id()
        << " was not assigned to an event but "
           "NuHepMC::add_attribute was called. In HepMC3 this would result in "
           "a silent failure.";
  }
  part->add_attribute(
      name, std::make_shared<typename NuHepMC::attr_traits<T>::type>(val));
}

template <typename T>
void add_attribute(HepMC3::GenEvent &ge, std::string const &name,
                   T const &val) {
  ge.add_attribute(
      name, std::make_shared<typename NuHepMC::attr_traits<T>::type>(val));
}

} // namespace NuHepMC