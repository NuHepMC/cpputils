#pragma once

#include "NuHepMC/Exceptions.hxx"

#include <ostream>

namespace NuHepMC {

namespace CrossSection {
namespace Units {

NEW_NuHepMC_EXCEPT(InvalidUnitType);

enum class XSUnits { CustomType, pb, cm2, cm2_ten38 };

enum class XSTargetScale {
  CustomType,
  PerTargetMolecule,
  PerTargetAtom,
  PerTargetNucleon,
  PerTargetMolecularNucleon
};

const double pb = 1;
const double cm2 = 1E36;
const double cm2_ten38 = 1E-2;

} // namespace Units
} // namespace CrossSection

} // namespace NuHepMC

inline std::ostream &operator<<(std::ostream &os,
                                NuHepMC::CrossSection::Units::XSUnits us) {
  switch (us) {
  case NuHepMC::CrossSection::Units::XSUnits::CustomType: {
    return os << "CustomXSUnit";
  }
  case NuHepMC::CrossSection::Units::XSUnits::pb: {
    return os << "pb";
  }
  case NuHepMC::CrossSection::Units::XSUnits::cm2: {
    return os << "cm2";
  }
  case NuHepMC::CrossSection::Units::XSUnits::cm2_ten38: {
    return os << "1e-38 cm2";
  }
  default: {
    throw NuHepMC::CrossSection::Units::InvalidUnitType();
  }
  }
}

inline std::ostream &
operator<<(std::ostream &os, NuHepMC::CrossSection::Units::XSTargetScale ts) {
  switch (ts) {
  case NuHepMC::CrossSection::Units::XSTargetScale::CustomType: {
    return os << "CustomTargetScale";
  }
  case NuHepMC::CrossSection::Units::XSTargetScale::PerTargetMolecule: {
    return os << "PerTargetMolecule";
  }
  case NuHepMC::CrossSection::Units::XSTargetScale::PerTargetAtom: {
    return os << "PerTargetAtom";
  }
  case NuHepMC::CrossSection::Units::XSTargetScale::PerTargetNucleon: {
    return os << "PerTargetNucleon";
  }
  case NuHepMC::CrossSection::Units::XSTargetScale::PerTargetMolecularNucleon: {
    return os << "PerTargetMolecularNucleon";
  }
  default: {
    throw NuHepMC::CrossSection::Units::InvalidUnitType();
  }
  }
}
