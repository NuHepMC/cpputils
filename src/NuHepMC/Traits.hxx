#pragma once

#include "HepMC3/Attribute.h"

#include "Eigen/Dense"

namespace NuHepMC {

// Lazy way of choosing the right attribute type via TMP
template <typename T> struct attr_traits {};

template <> struct attr_traits<bool> {
  typedef HepMC3::BoolAttribute type;
  constexpr static char const typestr[] = "HepMC3::BoolAttribute";
};

template <> struct attr_traits<int> {
  typedef HepMC3::IntAttribute type;
  constexpr static char const typestr[] = "HepMC3::IntAttribute";
};

template <> struct attr_traits<long> {
  typedef HepMC3::LongAttribute type;
  constexpr static char const typestr[] = "HepMC3::LongAttribute";
};

template <> struct attr_traits<long long> {
  typedef HepMC3::LongAttribute type;
  constexpr static char const typestr[] = "HepMC3::LongAttribute";
};

template <> struct attr_traits<size_t> {
  typedef HepMC3::ULongAttribute type;
  constexpr static char const typestr[] = "HepMC3::ULongAttribute";
};

template <> struct attr_traits<std::vector<int>> {
  typedef HepMC3::VectorIntAttribute type;
  constexpr static char const typestr[] = "HepMC3::VectorIntAttribute";
};

template <> struct attr_traits<double> {
  typedef HepMC3::DoubleAttribute type;
  constexpr static char const typestr[] = "HepMC3::DoubleAttribute";
};

template <> struct attr_traits<std::vector<double>> {
  typedef HepMC3::VectorDoubleAttribute type;
  constexpr static char const typestr[] = "HepMC3::VectorDoubleAttribute";
};

template <> struct attr_traits<Eigen::ArrayXd> {
  typedef HepMC3::VectorDoubleAttribute type;
  constexpr static char const typestr[] = "HepMC3::VectorDoubleAttribute";
};

template <> struct attr_traits<std::string> {
  typedef HepMC3::StringAttribute type;
  constexpr static char const typestr[] = "HepMC3::StringAttribute";
};

template <size_t N> struct attr_traits<char[N]> {
  typedef HepMC3::StringAttribute type;
  constexpr static char const typestr[] = "HepMC3::StringAttribute";
};

template <> struct attr_traits<char const *> {
  typedef HepMC3::StringAttribute type;
  constexpr static char const typestr[] = "HepMC3::StringAttribute";
};

template <> struct attr_traits<std::vector<std::string>> {
  typedef HepMC3::VectorStringAttribute type;
  constexpr static char const typestr[] = "HepMC3::VectorStringAttribute";
};

} // namespace NuHepMC