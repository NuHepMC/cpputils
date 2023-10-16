#include "NuHepMC/make_writer.hxx"

#include "NuHepMC/HepMC3Features.hxx"

#include "HepMC3/GenRunInfo.h"

#include "HepMC3/WriterAscii.h"
#ifdef HEPMC3_USE_COMPRESSION
#include "HepMC3/WriterGZ.h"
#endif
#ifdef HEPMC3_ProtobufIO_SUPPORT
#include "HepMC3/Writerprotobuf.h"
#endif

namespace NuHepMC {

namespace Writer {

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
                              std::shared_ptr<HepMC3::GenRunInfo> run_info) {

  auto ext = ParseExtension(split_extension(name).first);

  if (ext == kHepMC3) {
    return new HepMC3::WriterGZ<HepMC3::WriterAscii, C>(name.c_str(), run_info);
  } else if (ext == kProtobuf) {
#if HEPMC3_ProtobufIO_SUPPORT != 1
    throw NuHepMC::UnsupportedFilenameExtension()
        << "HepMC3 built without ProtobufIO support but tried to instantiate a "
           "writer for output file: "
        << name;
#else
    return new HepMC3::WriterGZ<HepMC3::Writerprotobuf, C>(name.c_str(),
                                                           run_info);
#endif
  }
  throw NuHepMC::UnknownFilenameExtension()
      << "Parsed compressed extension: \""
      << split_extension(split_extension(name).first).second
      << "\" from filename: \"" << name
      << "\", could not automatically determine HepMC3::Writer concrete "
         "type";
}

HepMC3::Writer *make_writer(std::string const &name,
                            std::shared_ptr<HepMC3::GenRunInfo> run_info) {

  int ext = ParseExtension(name);

  if (ext == kHepMC3) {
    return new HepMC3::WriterAscii(name.c_str(), run_info);
  } else if (ext == kProtobuf) {
#if HEPMC3_ProtobufIO_SUPPORT != 1
    throw NuHepMC::UnsupportedFilenameExtension()
        << "HepMC3 built without ProtobufIO support but tried to instantiate a "
           "writer for output file: "
        << name;
#else
    return new HepMC3::Writerprotobuf(name.c_str(), run_info);
#endif
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