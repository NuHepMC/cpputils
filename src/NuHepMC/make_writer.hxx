#pragma once

#include "NuHepMC/HepMC3Features.hxx"

#include "NuHepMC/Exceptions.hxx"

#include "HepMC3/GenRunInfo.h"
#include "HepMC3/Writer.h"

#include <string>
#include <utility>

namespace NuHepMC {

namespace Writer {

HepMC3::Writer *
make_writer(std::string const &name,
            std::shared_ptr<HepMC3::GenRunInfo> run_info = nullptr);
} // namespace Writer
} // namespace NuHepMC