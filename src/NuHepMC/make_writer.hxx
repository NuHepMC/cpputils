#pragma once

#include "HepMC3/GenRunInfo.h"

#include "HepMC3/WriterAscii.h"
#ifdef HEPMC3_USE_COMPRESSION
#include "HepMC3/WriterGZ.h"
#endif
#ifdef HEPMC3_ProtobufIO_SUPPORT
#include "HepMC3/Writerprotobuf.h"
#endif

#include <string>

namespace NuHepMC {

namespace Writer {

HepMC3::Writer *make_writer(std::string const &name,
                            std::shared_ptr<HepMC3::GenRunInfo> &run_info);
} // namespace Writer
} // namespace NuHepMC