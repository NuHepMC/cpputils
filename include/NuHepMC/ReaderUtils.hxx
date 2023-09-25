#pragma once

#include "NuHepMC/HepMC3Features.hxx"

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenRunInfo.h"
#include "HepMC3/GenVertex.h"

#include "HepMC3/Attribute.h"

#include "HepMC3/ReaderAscii.h"
#ifdef HEPMC3_USE_COMPRESSION
#include "HepMC3/ReaderGZ.h"
#endif
#ifdef HEPMC3_ProtobufIO_SUPPORT
#include "HepMC3/Readerprotobuf.h"
#endif

#include "NuHepMC/Constants.hxx"
#include "NuHepMC/Exceptions.hxx"
#include "NuHepMC/Traits.hxx"
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

  if (!obj->template attribute<typename NuHepMC::attr_traits<AT>::type>(name)) {
    throw AttributeTypeException()
        << name << ": " << obj->attribute_as_string(name);
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

namespace GR2 {
inline std::tuple<int, int, int>
ReadVersion(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  return std::make_tuple(
      CheckedAttributeValue<int>(run_info, "NuHepMC.Version.Major"),
      CheckedAttributeValue<int>(run_info, "NuHepMC.Version.Minor"),
      CheckedAttributeValue<int>(run_info, "NuHepMC.Version.Patch"));
}
} // namespace GR2

inline StatusCodeDescriptors
ReadIdDefinitions(std::shared_ptr<HepMC3::GenRunInfo> const &run_info,
                  std::pair<std::string, std::string> const &AttributeStubs) {

  auto IDs = CheckedAttributeValue<std::vector<int>>(
      run_info, "NuHepMC." + AttributeStubs.first);

  StatusCodeDescriptors status_codes;
  for (auto const &id : IDs) {
    status_codes[id] = std::pair<std::string, std::string>{
        CheckedAttributeValue<std::string>(
            run_info, "NuHepMC." + AttributeStubs.second + "[" +
                          std::to_string(id) + "].Name"),
        CheckedAttributeValue<std::string>(
            run_info, "NuHepMC." + AttributeStubs.second + "[" +
                          std::to_string(id) + "].Description")};
  }
  return status_codes;
}

namespace GR4 {
inline StatusCodeDescriptors
ReadProcessIdDefinitions(std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  return ReadIdDefinitions(run_info, {"ProcessIDs", "ProcessInfo"});
}
} // namespace GR4

namespace GR5 {
inline StatusCodeDescriptors ReadVertexStatusIdDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  return ReadIdDefinitions(run_info, {"VertexStatusIDs", "VertexStatusInfo"});
}
} // namespace GR5

namespace GR6 {
inline StatusCodeDescriptors ReadParticleStatusIdDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  return ReadIdDefinitions(run_info,
                           {"ParticleStatusIDs", "ParticleStatusInfo"});
}
} // namespace GR6

namespace GC1 {
std::set<std::string>
ReadConventions(std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  std::set<std::string> conventions;
  for (auto &c : CheckedAttributeValue<std::vector<std::string>>(
           run_info, "NuHepMC.Conventions", std::vector<std::string>{})) {
    conventions.insert(c);
  }
  return conventions;
}
bool SignalsConvention(std::shared_ptr<HepMC3::GenRunInfo> const &run_info,
                       std::string const &Convention) {
  auto conventions = ReadConventions(run_info);

  return conventions.count(Convention);
}
} // namespace GC1

namespace GC2 {
long ReadExposureNEvents(std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  return CheckedAttributeValue<int>(run_info, "NuHepMC.Exposure.NEvents");
}
} // namespace GC2

namespace GC4 {
std::pair<std::string, std::string>
ReadCrossSectionUnits(std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  return std::make_pair(
      CheckedAttributeValue<std::string>(
          run_info, "NuHepMC.Units.CrossSection.Unit", "pb"),
      CheckedAttributeValue<std::string>(
          run_info, "NuHepMC.Units.CrossSection.TargetScale", "PerTargetAtom"));
}
} // namespace GC4

namespace GC5 {
long ReadFluxAveragedTotalXSec(
    std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  return CheckedAttributeValue<double>(run_info,
                                       "NuHepMC.FluxAveragedTotalCrossSection");
}
} // namespace GC5

} // namespace NuHepMC