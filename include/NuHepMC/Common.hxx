#pragma once

#include <string>

#include "NuHepMC/Exceptions.hxx"

namespace NuHepMC {

NEW_NuHepMC_EXCEPT(UnknownFilenameExtension);

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

} // namespace NuHepMC