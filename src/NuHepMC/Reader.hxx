#pragma once

#include "HepMC3/ReaderFactory.h"

#include "NuHepMC/Exceptions.hxx"

namespace NuHepMC {

// A reader implementation that can automatically update the NuHepMC spec of
// a read file so that users of cpputils can just target the latest spec.
class Reader : public HepMC3::Reader {

  std::shared_ptr<HepMC3::Reader> rdr;

  int in_version;

public:
  NEW_NuHepMC_EXCEPT(NullReader);

  Reader(std::shared_ptr<HepMC3::Reader> other) : rdr(other), in_version(0) {
    if (!rdr) {
      throw NullReader() << "NuHepMC::Reader instantiated with a nullptr.";
    }
  }

  Reader(std::string const &filename)
      : Reader(HepMC3::deduce_reader(filename)) {}

  bool skip(const int n) { return rdr->skip(n); }
  bool read_event(HepMC3::GenEvent &evt);
  bool failed() { return rdr->failed(); }
  void close() { return rdr->close(); }
  void set_options(const std::map<std::string, std::string> &options) {
    rdr->set_options(options);
  }
  std::map<std::string, std::string> get_options() {
    return rdr->get_options();
  }
};

} // namespace NuHepMC