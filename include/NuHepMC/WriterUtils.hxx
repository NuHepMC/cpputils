#pragma once

#include "NuHepMC/HepMC3Features.hxx"

#include "HepMC3/GenRunInfo.h"

#include "HepMC3/Attribute.h"

#include "HepMC3/WriterAscii.h"
#ifdef HEPMC3_USE_COMPRESSION
#include "HepMC3/WriterGZ.h"
#endif
#ifdef HEPMC3_ProtobufIO_SUPPORT
#include "HepMC3/Writerprotobuf.h"
#endif

#include "NuHepMC/Constants.hxx"
#include "NuHepMC/Exceptions.hxx"
#include "NuHepMC/NuHepMCVersion.hxx"
#include "NuHepMC/Types.hxx"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace NuHepMC {

NEW_NuHepMC_EXCEPT(UnsupportedFilenameExtension);
NEW_NuHepMC_EXCEPT(UnknownFilenameExtension);

// Lazy way of choosing the right attribute type via TMP
template <typename T> struct attr_traits {};

template <> struct attr_traits<int> {
  typedef HepMC3::IntAttribute type;
};

template <> struct attr_traits<std::vector<int>> {
  typedef HepMC3::VectorIntAttribute type;
};

template <> struct attr_traits<double> {
  typedef HepMC3::DoubleAttribute type;
};

template <> struct attr_traits<std::vector<double>> {
  typedef HepMC3::VectorDoubleAttribute type;
};

template <> struct attr_traits<std::string> {
  typedef HepMC3::StringAttribute type;
};

template <size_t N> struct attr_traits<char[N]> {
  typedef HepMC3::StringAttribute type;
};

template <> struct attr_traits<std::vector<std::string>> {
  typedef HepMC3::VectorStringAttribute type;
};

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

namespace GR2 {
inline void WriteVersion(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  add_attribute(run_info, "NuHepMC.Version.Major", NuHepMC_VERSION_MAJOR);
  add_attribute(run_info, "NuHepMC.Version.Minor", NuHepMC_VERSION_MINOR);
  add_attribute(run_info, "NuHepMC.Version.Patch", NuHepMC_VERSION_PATCH);
}
} // namespace GR2

inline void
WriteIDDefinitions(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                   StatusCodeDescriptors const &Definitions,
                   std::pair<std::string, std::string> const &AttributeStubs) {

  std::vector<int> IDs;
  for (auto const &p : Definitions) {
    IDs.push_back(p.first);
  }
  add_attribute(run_info, "NuHepMC." + AttributeStubs.first, IDs);

  for (auto const &p : Definitions) {
    add_attribute(run_info,
                  "NuHepMC." + AttributeStubs.second + "[" +
                      std::to_string(p.first) + "].Name",
                  p.second.first);

    add_attribute(run_info,
                  "NuHepMC." + AttributeStubs.second + "[" +
                      std::to_string(p.first) + "].Description",
                  p.second.second);
  }
}

namespace GR4 {
inline void
WriteProcessIDDefinitions(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                          StatusCodeDescriptors const &Definitions) {
  WriteIDDefinitions(run_info, Definitions, {"ProcessIDs", "ProcessInfo"});
}
} // namespace GR4

namespace GR5 {
inline void
WriteVertexStatusIDDefinitions(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                               StatusCodeDescriptors const &Definitions) {
  WriteIDDefinitions(run_info, Definitions,
                     {"VertexStatusIDs", "VertexStatusInfo"});
}
} // namespace GR5

namespace GR6 {
inline void
WriteParticleStatusIDDefinitions(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                                 StatusCodeDescriptors const &Definitions) {
  WriteIDDefinitions(run_info, Definitions,
                     {"ParticleStatusIDs", "ParticleStatusInfo"});
}
} // namespace GR6

namespace GR7 {
inline void SetWeightNames(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                           std::vector<std::string> const &names) {
  run_info->set_weight_names(names);
}
} // namespace GR7

namespace GC1 {
inline void SetConventions(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                           std::vector<std::string> const &conventions) {
  add_attribute(run_info, "NuHepMC.Conventions", conventions);
}
} // namespace GC1

namespace GC2 {
inline void SetExposureNEvents(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                               int NEvents) {
  add_attribute(run_info, "NuHepMC.Exposure.NEvents", NEvents);
}
} // namespace GC2

namespace GC4 {
inline void
SetFluxAveragedTotalXSec(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                         double fatx) {
  add_attribute(run_info, "NuHepMC.FluxAveragedTotalCrossSection", fatx);
}
} // namespace GC4

namespace Writer {

enum DiskFormat {
  kHepMC3 = 1,
  kProtobuf = 2,
};
enum CompressionFormat {
  kZ = 10,
  kLZMA = 20,
  kBZip2 = 30,
};

std::pair<std::string, std::string> split_extension(std::string const &name) {
  size_t fext = name.find_last_of('.');
  std::string ext = name.substr(fext + 1, std::string::npos);
  std::string name_woe = name.substr(0, fext);

  return std::make_pair(name_woe, ext);
}

int ParseExtension(std::string const &name) {

  auto split_name = split_extension(name);
  std::string name_woe = split_name.first, ext = split_name.second;

  if ((ext == "hepmc3") || (ext == "hepmc")) {
    return kHepMC3;
  } else if ((ext == "proto") || (ext == "pb")) {
    return kProtobuf;
  } else if (ext == "gz") {
    return kZ;
  } else if (ext == "lzma") {
    return kLZMA;
  } else if (ext == "bz2") {
    return kBZip2;
  }
  throw NuHepMC::UnknownFilenameExtension()
      << "Parsed extension: \"" << ext << "\" from filename: \"" << name
      << "\", could not automatically determine HepMC3::Writer concrete "
         "type";
}

template <bxz::Compression C>
HepMC3::Writer *make_writergz(std::string const &name,
                              std::shared_ptr<HepMC3::GenRunInfo> &run_info) {

  auto ext = ParseExtension(split_extension(name).first);

  if (ext == kHepMC3) {
    return new HepMC3::WriterGZ<HepMC3::WriterAscii, C>(name.c_str(), run_info);
  } else if (ext == kProtobuf) {
#if HEPMC3_ProtobufIO_SUPPORT != 1
    throw NuHepMC::UnsupportedFilenameExtension()
        << "HepMC3 built without ProtobufIO support but tried to instantiate a "
           "writer for output file: "
        << name;
#endif
    return new HepMC3::WriterGZ<HepMC3::Writerprotobuf, C>(name.c_str(),
                                                           run_info);
  }
  throw NuHepMC::UnknownFilenameExtension()
      << "Parsed compressed extension: \""
      << split_extension(split_extension(name).first).second
      << "\" from filename: \"" << name
      << "\", could not automatically determine HepMC3::Writer concrete "
         "type";
}

HepMC3::Writer *make_writer(std::string const &name,
                            std::shared_ptr<HepMC3::GenRunInfo> &run_info) {

  int ext = ParseExtension(name);

  if (ext == kHepMC3) {
    return new HepMC3::WriterAscii(name.c_str(), run_info);
  } else if (ext == kProtobuf) {
#if HEPMC3_ProtobufIO_SUPPORT != 1
    throw NuHepMC::UnsupportedFilenameExtension()
        << "HepMC3 built without ProtobufIO support but tried to instantiate a "
           "writer for output file: "
        << name;
#endif
    return new HepMC3::Writerprotobuf(name.c_str(), run_info);
  } else if (((ext / 10) * 10) == kZ) {
#if HEPMC3_Z_SUPPORT != 1
    throw NuHepMC::UnsupportedFilenameExtension()
        << "HepMC3 built without ZLib support but tried to instantiate a "
           "writer for output file: "
        << name;
#else
    return make_writergz<bxz::Compression::z>(name, run_info);
#endif

  } else if (((ext / 10) * 10) == kLZMA) {
#if HEPMC3_LZMA_SUPPORT != 1
    throw NuHepMC::UnsupportedFilenameExtension()
        << "HepMC3 built without LibLZMA support but tried to instantiate a "
           "writer for output file: "
        << name;
#else
    return make_writergz<bxz::Compression::lzma>(name, run_info);
#endif

  } else if (((ext / 10) * 10) == kBZip2) {
#if HEPMC3_BZ2_SUPPORT != 1
    throw NuHepMC::UnsupportedFilenameExtension()
        << "HepMC3 built without BZip2 support but tried to instantiate a "
           "writer for output file: "
        << name;
#else
    return make_writergz<bxz::Compression::bz2>(name, run_info);
#endif
  }
  throw NuHepMC::UnknownFilenameExtension()
      << "Parsed extension: \"" << split_extension(name).second
      << "\" from filename: \"" << name
      << "\", could not automatically determine HepMC3::Writer concrete "
         "type";
}
} // namespace Writer

} // namespace NuHepMC