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
void add_attribute(std::shared_ptr<HepMC3::GenRunInfo> run_info,
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

NEW_NuHepMC_EXCEPT(NullObjectException);
NEW_NuHepMC_EXCEPT(MissingAttributeException);
NEW_NuHepMC_EXCEPT(AttributeTypeException);

template <typename T> bool HasAttribute(T const &obj, std::string const &name) {
  auto const &attr_names = obj->attribute_names();
  return std::find(attr_names.begin(), attr_names.end(), name) !=
         attr_names.end();
}

template <typename AT, typename T>
bool HasAttributeOfType(T const &obj, std::string const &name) {
  if (!obj) {
    throw NullObjectException();
  }

  if (!HasAttribute(obj, name)) {
    return false;
  }

  if (!obj->template attribute<typename NuHepMC::attr_traits<AT>::type>(name)) {
    return false;
  }
  return true;
}

template <typename AT, typename T>
auto CheckedAttributeValue(T const &obj, std::string const &name) {
  if (!obj) {
    throw NullObjectException();
  }

  if (!HasAttribute(obj, name)) {
    MissingAttributeException mae;
    mae << "Failed to find attribute: " << name;
    mae << "\n\tKnown attributes: \n";
    for (auto const &a : obj->attribute_names()) {
      mae << "\t\t" << a << "\n";
    }
    throw mae;
  }

  if (!obj->template attribute<typename NuHepMC::attr_traits<AT>::type>(name)) {
    throw AttributeTypeException()
        << name << ": \"" << obj->attribute_as_string(name)
        << "\" could not be parsed as requested type: "
        << NuHepMC::attr_traits<AT>::typestr;
  }

  return obj->template attribute<typename NuHepMC::attr_traits<AT>::type>(name)
      ->value();
}

template <typename AT, typename T>
auto CheckedAttributeValue(T const &obj, std::string const &name,
                           AT const &defval) {
  if (!obj) {
    throw NullObjectException();
  }

  if (!HasAttribute(obj, name)) {
    return defval;
  }

  if (!obj->template attribute<typename NuHepMC::attr_traits<AT>::type>(name)) {
    throw AttributeTypeException()
        << name << ": " << obj->attribute_as_string(name);
  }

  return obj->template attribute<typename NuHepMC::attr_traits<AT>::type>(name)
      ->value();
}

} // namespace NuHepMC