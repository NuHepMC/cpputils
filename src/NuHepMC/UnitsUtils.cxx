#include "NuHepMC/UnitsUtils.hxx"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "HepMC3/Print.h"

namespace NuHepMC {

namespace CrossSection {

namespace Units {
std::ostream &operator<<(std::ostream &os,
                         NuHepMC::CrossSection::Units::Scale const &us) {
  switch (us) {
  case NuHepMC::CrossSection::Units::Scale::CustomType: {
    return os << "CustomXSUnit";
  }
  case NuHepMC::CrossSection::Units::Scale::pb: {
    return os << "pb";
  }
  case NuHepMC::CrossSection::Units::Scale::nb: {
    return os << "nb";
  }
  case NuHepMC::CrossSection::Units::Scale::cm2: {
    return os << "cm2";
  }
  case NuHepMC::CrossSection::Units::Scale::cm2_ten38: {
    return os << "1e-38 cm2";
  }
  default: {
    throw NuHepMC::CrossSection::Units::InvalidUnitType();
  }
  }
}

std::ostream &operator<<(std::ostream &os,
                         NuHepMC::CrossSection::Units::TargetScale const &ts) {
  switch (ts) {
  case NuHepMC::CrossSection::Units::TargetScale::CustomType: {
    return os << "CustomTargetScale";
  }
  case NuHepMC::CrossSection::Units::TargetScale::PerTarget: {
    return os << "PerTarget";
  }
  case NuHepMC::CrossSection::Units::TargetScale::PerTargetNucleon: {
    return os << "PerTargetNucleon";
  }
  default: {
    throw NuHepMC::CrossSection::Units::InvalidUnitType();
  }
  }
}

std::ostream &operator<<(std::ostream &os,
                         NuHepMC::CrossSection::Units::Unit const &u) {
  return os << "[" << u.scale << ", " << u.tgtscale << "]";
}

} // namespace Units
} // namespace CrossSection

std::string to_string(NuHepMC::CrossSection::Units::Scale us) {
  std::stringstream ss("");
  ss << us;
  return ss.str();
}
std::string to_string(NuHepMC::CrossSection::Units::TargetScale ts) {
  std::stringstream ss("");
  ss << ts;
  return ss.str();
}
std::string to_string(NuHepMC::CrossSection::Units::Unit const &u) {
  std::stringstream ss("");
  ss << u;
  return ss.str();
}
} // namespace NuHepMC