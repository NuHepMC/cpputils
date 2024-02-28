#include "catch2/catch_test_macros.hpp"

#include "NuHepMC/AttributeUtils.hxx"

#include <cassert>

using namespace HepMC3;

#define TYPE_TEST_MACRO(tname, hmtype, tval)                                   \
  TEST_CASE(tname, "[AttributeUtils]") {                                       \
    auto gri = std::make_shared<HepMC3::GenRunInfo>();                         \
    NuHepMC::add_attribute(gri, "a", tval);                                    \
    REQUIRE(gri->attribute_names().size() == 1);                               \
    REQUIRE(gri->attribute_names()[0] == "a");                                 \
    auto a_hmtype = gri->attribute<hmtype>("a");                               \
    REQUIRE(bool(a_hmtype));                                                   \
    REQUIRE(a_hmtype->value() == tval);                                        \
  }

#define TYPE_TEST_MACRO_VECTOR(tname, hmtype, tval)                            \
  TEST_CASE(tname, "[AttributeUtils]") {                                       \
    auto gri = std::make_shared<HepMC3::GenRunInfo>();                         \
    NuHepMC::add_attribute(gri, "a", tval);                                    \
    REQUIRE(gri->attribute_names().size() == 1);                               \
    REQUIRE(gri->attribute_names()[0] == "a");                                 \
    auto a_hmtype = gri->attribute<hmtype>("a");                               \
    REQUIRE(bool(a_hmtype));                                                   \
    REQUIRE(a_hmtype->value()[1] == tval[1]);                                  \
  }

#define TYPE_TEST_MACRO_ARRAYXD(tname, hmtype, tval)                           \
  TEST_CASE(tname, "[AttributeUtils]") {                                       \
    auto gri = std::make_shared<HepMC3::GenRunInfo>();                         \
    NuHepMC::add_attribute(gri, "a", tval);                                    \
    REQUIRE(gri->attribute_names().size() == 1);                               \
    REQUIRE(gri->attribute_names()[0] == "a");                                 \
    auto a_hmtype = gri->attribute<hmtype>("a");                               \
    REQUIRE(bool(a_hmtype));                                                   \
    REQUIRE(a_hmtype->value()[1] == tval(1));                                  \
  }

bool mybool = true;
TYPE_TEST_MACRO("add_attribute<bool>", BoolAttribute, mybool);

int myint = std::numeric_limits<int>::max();
TYPE_TEST_MACRO("add_attribute<int>", IntAttribute, myint);

long mylong = std::numeric_limits<long>::max();
TYPE_TEST_MACRO("add_attribute<long>", LongAttribute, mylong);

long long mylonglong = std::numeric_limits<long long>::max();
TYPE_TEST_MACRO("add_attribute<long long>", LongAttribute, mylonglong);

size_t mysize_t = std::numeric_limits<size_t>::max();
TYPE_TEST_MACRO("add_attribute<size_t>", ULongAttribute, mysize_t);

std::vector<int> myvectorint = {
    std::numeric_limits<int>::max(),
    std::numeric_limits<int>::min(),
};
TYPE_TEST_MACRO_VECTOR("add_attribute<std::vector<int>>", VectorIntAttribute,
                       myvectorint);

double mydouble = std::numeric_limits<double>::max();
TYPE_TEST_MACRO("add_attribute<double>", DoubleAttribute, mydouble);

std::vector<double> myvectordouble = {
    std::numeric_limits<double>::max(),
    std::numeric_limits<double>::min(),
};
TYPE_TEST_MACRO_VECTOR("add_attribute<std::vector<double>>",
                       VectorDoubleAttribute, myvectordouble);

Eigen::ArrayXd myearr = (Eigen::ArrayXd(5) << 1, 2, 3, 4, 5).finished();
TYPE_TEST_MACRO_ARRAYXD("add_attribute<Eigen::ArrayXd>", VectorDoubleAttribute,
                        myearr);

std::string mystdstring = "abcdef";
TYPE_TEST_MACRO("add_attribute<std::string>", StringAttribute, mystdstring);

char mychararr[] = "abcdef";
TYPE_TEST_MACRO("add_attribute<char[N]>", StringAttribute, mychararr);

char const *mycstr = "abcdef";
TYPE_TEST_MACRO("add_attribute<char const *>", StringAttribute, mycstr);

std::vector<std::string> myvectorstring = {"abcde", "fghij"};
TYPE_TEST_MACRO_VECTOR("add_attribute<std::vector<std::string>>",
                       VectorStringAttribute, myvectorstring);

TEST_CASE("add_attribute_eventless_particle_throw", "[AttributeUtils]") {
  auto part = std::make_shared<HepMC3::GenParticle>();
  REQUIRE_THROWS_AS(NuHepMC::add_attribute(part, "a", bool(true)),
                    NuHepMC::CannotAddAttribute);
}

TEST_CASE("HasAttribute", "[AttributeUtils]") {
  auto gri = std::make_shared<HepMC3::GenRunInfo>();
  NuHepMC::add_attribute(gri, "a", bool(true));
  NuHepMC::add_attribute(gri, "b", 1);
  NuHepMC::add_attribute(gri, "c", std::vector<std::string>{"a", "b", "c"});
  NuHepMC::add_attribute(gri, "d", 1.2345);
  REQUIRE(NuHepMC::HasAttribute(gri, "a"));
  REQUIRE(NuHepMC::HasAttribute(gri, "b"));
  REQUIRE(NuHepMC::HasAttribute(gri, "c"));
  REQUIRE(NuHepMC::HasAttribute(gri, "d"));
  REQUIRE(!NuHepMC::HasAttribute(gri, "e"));
}

TEST_CASE("HasAttributeOfType", "[AttributeUtils]") {
  auto gri = std::make_shared<HepMC3::GenRunInfo>();
  NuHepMC::add_attribute(gri, "a", bool(true));
  NuHepMC::add_attribute(gri, "b", 1);
  NuHepMC::add_attribute(gri, "c", std::vector<std::string>{"a", "b", "c"});
  NuHepMC::add_attribute(gri, "d", 1.2345);
  REQUIRE(NuHepMC::HasAttributeOfType<bool>(gri, "a"));
  REQUIRE(NuHepMC::HasAttributeOfType<int>(gri, "b"));
  REQUIRE(NuHepMC::HasAttributeOfType<std::vector<std::string>>(gri, "c"));
  REQUIRE(NuHepMC::HasAttributeOfType<double>(gri, "d"));

  REQUIRE(!NuHepMC::HasAttributeOfType<int>(gri, "a"));
  REQUIRE(!NuHepMC::HasAttributeOfType<std::vector<std::string>>(gri, "a"));
  REQUIRE(!NuHepMC::HasAttributeOfType<double>(gri, "a"));

  REQUIRE(!NuHepMC::HasAttributeOfType<bool>(gri, "b"));
  REQUIRE(!NuHepMC::HasAttributeOfType<std::vector<std::string>>(gri, "b"));
  REQUIRE(!NuHepMC::HasAttributeOfType<double>(gri, "b"));

  REQUIRE(!NuHepMC::HasAttributeOfType<bool>(gri, "c"));
  REQUIRE(!NuHepMC::HasAttributeOfType<int>(gri, "c"));
  REQUIRE(!NuHepMC::HasAttributeOfType<double>(gri, "c"));

  REQUIRE(!NuHepMC::HasAttributeOfType<bool>(gri, "d"));
  REQUIRE(!NuHepMC::HasAttributeOfType<int>(gri, "d"));
  REQUIRE(!NuHepMC::HasAttributeOfType<std::vector<std::string>>(gri, "d"));
}
