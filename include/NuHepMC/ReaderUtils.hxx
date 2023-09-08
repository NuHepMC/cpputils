#pragma once

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
#include "NuHepMC/Traits.hxx"m
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

  if (!obj->template attribute<NuHepMC::attr_traits<AT>::type>(name)) {
    throw AttributeTypeException()
        << name << ": " << obj->attribute_as_string(name);
  }

  return obj->template attribute<NuHepMC::attr_traits<AT>::type>(name)->value();
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

  if (!obj->template attribute<NuHepMC::attr_traits<AT>::type>(name)) {
    throw AttributeTypeException()
        << name << ": " << obj->attribute_as_string(name);
  }

  return obj->template attribute<NuHepMC::attr_traits<AT>::type>(name)->value();
}

namespace GR2 {
inline std::tuple<int, int, int>
ReadVersion(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  return std::tuple<int, int, int>{CheckedAttributeValue<HepMC3::IntAttribute>(
                                       run_info, "NuHepMC.Version.Major"),
                                   CheckedAttributeValue<HepMC3::IntAttribute>(
                                       run_info, "NuHepMC.Version.Minor"),
                                   CheckedAttributeValue<HepMC3::IntAttribute>(
                                       run_info, "NuHepMC.Version.Patch")};
}
} // namespace GR2

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

namespace GR4 {
inline StatusCodeDescriptors
ReadProcessIdDefinitions(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  return ReadIdDefinitions(run_info, {"ProcessIDs", "ProcessInfo"});
}
} // namespace GR4

namespace GR5 {
inline StatusCodeDescriptors
ReadVertexStatusIdDefinitions(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  return ReadIdDefinitions(run_info, {"VertexStatusIDs", "VertexStatusInfo"});
}
} // namespace GR5

namespace GR6 {
inline StatusCodeDescriptors
ReadParticleStatusIdDefinitions(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  return ReadIdDefinitions(run_info,
                           {"ParticleStatusIDs", "ParticleStatusInfo"});
}
} // namespace GR6

namespace GC1 {
std::set<std::string>
ReadConventions(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  std::set<std::string> conventions;
  for (auto &c : CheckedAttributeValue<HepMC3::VectorStringAttribute>(
           run_info, "NuHepMC.Conventions", std::vector<std::string>{})) {
    conventions.insert(c);
  }
  return conventions;
}
bool SignalsConvention(std::shared_ptr<HepMC3::GenRunInfo> const &run_info,
                       std::string const &Convention) {
  auto conventions =
     ReadConventions(run_info);

  return conventions.count(Convention);
}
} // namespace GC1

namespace GC2 {
long ReadExposureNEvents(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  return CheckedAttributeValue<HepMC3::IntAttribute>(
      run_info, "NuHepMC.Exposure.NEvents");
}
} // namespace GC2

namespace GC4 {
std::pair<std::string, std::string> void
ReadCrossSectionUnits(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  return std::make_pair(
      CheckedAttributeValue<HepMC3::StringAttribute>(
          run_info, "NuHepMC.Units.CrossSection.Unit", "pb");
      CheckedAttributeValue<HepMC3::StringAttribute>(
          run_info, "NuHepMC.Units.CrossSection.TargetScale", "PerTargetAtom"));
}
} // namespace GC4

namespace GC5 {
long ReadFluxAveragedTotalXSec(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  return CheckedAttributeValue<HepMC3::DoubleAttribute>(
      run_info, "NuHepMC.FluxAveragedTotalCrossSection");
}
} // namespace GC5

namespace Reader {
template <bxz::Compression C>
HepMC3::Reader *make_readergz(std::string const &name,
                              std::shared_ptr<HepMC3::GenRunInfo> &run_info) {

  auto ext = ParseExtension(split_extension(name).first);

  if (ext == kHepMC3) {
    return new HepMC3::ReaderGZ<HepMC3::ReaderAscii, C>(name.c_str(), run_info);
  } else if (ext == kProtobuf) {
#if HEPMC3_ProtobufIO_SUPPORT != 1
    throw NuHepMC::UnsupportedFilenameExtension()
        << "HepMC3 built without ProtobufIO support but tried to instantiate a "
           "reader for output file: "
        << name;
#endif
    return new HepMC3::ReaderGZ<HepMC3::Readerprotobuf, C>(name.c_str(),
                                                           run_info);
  }
  throw NuHepMC::UnknownFilenameExtension()
      << "Parsed compressed extension: \""
      << split_extension(split_extension(name).first).second
      << "\" from filename: \"" << name
      << "\", could not automatically determine HepMC3::Reader concrete "
         "type";
}

HepMC3::Reader *make_reader(std::string const &name,
                            std::shared_ptr<HepMC3::GenRunInfo> &run_info) {

  int ext = ParseExtension(name);

  if (ext == kHepMC3) {
    return new HepMC3::ReaderAscii(name.c_str(), run_info);
  } else if (ext == kProtobuf) {
#if HEPMC3_ProtobufIO_SUPPORT != 1
    throw NuHepMC::UnsupportedFilenameExtension()
        << "HepMC3 built without ProtobufIO support but tried to instantiate a "
           "reader for output file: "
        << name;
#endif
    return new HepMC3::Readerprotobuf(name.c_str(), run_info);
  } else if (((ext / 10) * 10) == kZ) {
#if HEPMC3_Z_SUPPORT != 1
    throw NuHepMC::UnsupportedFilenameExtension()
        << "HepMC3 built without ZLib support but tried to instantiate a "
           "reader for output file: "
        << name;
#else
    return make_readergz<bxz::Compression::z>(name, run_info);
#endif

  } else if (((ext / 10) * 10) == kLZMA) {
#if HEPMC3_LZMA_SUPPORT != 1
    throw NuHepMC::UnsupportedFilenameExtension()
        << "HepMC3 built without LibLZMA support but tried to instantiate a "
           "reader for output file: "
        << name;
#else
    return make_readergz<bxz::Compression::lzma>(name, run_info);
#endif

  } else if (((ext / 10) * 10) == kBZip2) {
#if HEPMC3_BZ2_SUPPORT != 1
    throw NuHepMC::UnsupportedFilenameExtension()
        << "HepMC3 built without BZip2 support but tried to instantiate a "
           "reader for output file: "
        << name;
#else
    return make_readergz<bxz::Compression::bz2>(name, run_info);
#endif
  }
  throw NuHepMC::UnknownFilenameExtension()
      << "Parsed extension: \"" << split_extension(name).second
      << "\" from filename: \"" << name
      << "\", could not automatically determine HepMC3::Reader concrete "
         "type";
}
} // namespace Reader

} // namespace NuHepMC