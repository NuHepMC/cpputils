#include "catch2/catch_test_macros.hpp"

#include "NuHepMC/ReaderUtils.hxx"
#include "NuHepMC/AttributeUtils.hxx"

#include <cassert>

TEST_CASE("GR2::ReadVersion", "[ReaderUtils]") {
  auto gri = std::make_shared<HepMC3::GenRunInfo>();

  NuHepMC::add_attribute(gri,"NuHepMC.Version.Major",1);
  NuHepMC::add_attribute(gri,"NuHepMC.Version.Minor",2);
  NuHepMC::add_attribute(gri,"NuHepMC.Version.Patch",3);

  auto [maj,min,pat] = NuHepMC::GR2::ReadVersion(gri);

  REQUIRE(maj == 1);
  REQUIRE(min == 2);
  REQUIRE(pat == 3);

}

// ReadVersion
// ReadVersionString