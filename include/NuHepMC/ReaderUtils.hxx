#pragma once

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenRunInfo.h"
#include "HepMC3/GenVertex.h"

#include "HepMC3/Attribute.h"

#include "NuHepMC/Constants.hxx"
#include "NuHepMC/Exceptions.hxx"
#include "NuHepMC/Types.hxx"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace NuHepMC {

NEW_NuHepMC_EXCEPT(NullObjectException);
NEW_NuHepMC_EXCEPT(MissingAttributeException);
NEW_NuHepMC_EXCEPT(AttributeTypeException);

template <typename T> bool HasAttribute(T const &obj, std::string const &name) {
  auto attr_names = obj->attribute_names();
  return std::find(attr_names.begin(), attr_names.end(), name) !=
         attr_names.end();
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

  if (!obj->template attribute<AT>(name)) {
    throw AttributeTypeException()
        << name << ": " << obj->attribute_as_string(name);
  }

  return obj->template attribute<AT>(name)->value();
}

template <typename AT, typename T>
auto CheckedAttributeValue(T const &obj, std::string const &name,
                           decltype(obj->template attribute<AT>(name)->value())
                               const defval) {
  if (!obj) {
    throw NullObjectException();
  }

  if (!HasAttribute(obj, name)) {
    return defval;
  }

  if (!obj->template attribute<AT>(name)) {
    throw AttributeTypeException()
        << name << ": " << obj->attribute_as_string(name);
  }

  return obj->template attribute<AT>(name)->value();
}

namespace RunInfo {

// G.R.2
inline std::tuple<int, int, int>
ReadNuHepMCVersion(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  return std::tuple<int, int, int>{CheckedAttributeValue<HepMC3::IntAttribute>(
                                       run_info, "NuHepMC.Version.Major"),
                                   CheckedAttributeValue<HepMC3::IntAttribute>(
                                       run_info, "NuHepMC.Version.Minor"),
                                   CheckedAttributeValue<HepMC3::IntAttribute>(
                                       run_info, "NuHepMC.Version.Patch")};
}

inline StatusCodeDescriptors
ReadIdDefinitions(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                  std::pair<std::string, std::string> const &AttributeStubs) {

  auto IDs = CheckedAttributeValue<HepMC3::VectorIntAttribute>(
      run_info, "NuHepMC." + AttributeStubs.first);

  StatusCodeDescriptors status_codes;
  for (auto const &id : IDs) {
    status_codes[id] = std::pair<std::string, std::string>{
        CheckedAttributeValue<HepMC3::StringAttribute>(
            run_info, "NuHepMC." + AttributeStubs.second + "[" +
                          std::to_string(id) + "].Name"),
        CheckedAttributeValue<HepMC3::StringAttribute>(
            run_info, "NuHepMC." + AttributeStubs.second + "[" +
                          std::to_string(id) + "].Description")};
  }
  return status_codes;
}

// G.R.4
inline StatusCodeDescriptors
ReadProcIdDefinitions(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  return ReadIdDefinitions(run_info, {"ProcessIds", "ProcessInfo"});
}

// G.R.5
inline StatusCodeDescriptors
ReadVertexStatusIdDefinitions(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  return ReadIdDefinitions(run_info, {"VertexStatusIds", "VertexStatusInfo"});
}

// G.R.6
inline StatusCodeDescriptors
ReadParticleStatusIdDefinitions(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  return ReadIdDefinitions(run_info,
                           {"ParticleStatusIDs", "ParticleStatusInfo"});
}

std::set<std::string>
ReadConventions(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  std::set<std::string> conventions;
  for (auto &c : CheckedAttributeValue<HepMC3::VectorStringAttribute>(
           run_info, "NuHepMC.Conventions", std::vector<std::string>{})) {
    conventions.insert(c);
  }
  return conventions;
}

} // namespace RunInfo

} // namespace NuHepMC