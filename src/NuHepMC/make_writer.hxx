#pragma once

#include "NuHepMC/Exceptions.hxx"
#include "HepMC3/GenRunInfo.h"
#include "HepMC3/Writer.h"

#include <string>
#include <utility>

namespace NuHepMC {

NEW_NuHepMC_EXCEPT(UnsupportedFilenameExtension);
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

std::pair<std::string, std::string> split_extension(std::string const &name);
int ParseExtension(std::string const &name);

namespace Writer {

HepMC3::Writer *make_writer(std::string const &name,
                            std::shared_ptr<HepMC3::GenRunInfo> &run_info);
} // namespace Writer
} // namespace NuHepMC