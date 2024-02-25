#pragma once

#include "HepMC3/Attribute.h"

namespace NuHepMC {

// Lazy way of choosing the right attribute type via TMP
template <typename T> struct attr_traits {};

template <> struct attr_traits<bool> {
  typedef HepMC3::BoolAttribute type;
};

template <> struct attr_traits<int> {
  typedef HepMC3::IntAttribute type;
};

template <> struct attr_traits<long> {
  typedef HepMC3::LongAttribute type;
};

template <> struct attr_traits<long long> {
  typedef HepMC3::LongAttribute type;
};

template <> struct attr_traits<size_t> {
  typedef HepMC3::ULongAttribute type;
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

template <> struct attr_traits<char const *> {
  typedef HepMC3::StringAttribute type;
};

template <> struct attr_traits<std::vector<std::string>> {
  typedef HepMC3::VectorStringAttribute type;
};

} // namespace NuHepMC