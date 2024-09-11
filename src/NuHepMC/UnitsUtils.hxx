#pragma once

#include "NuHepMC/Exceptions.hxx"

#include "HepMC3/GenEvent.h"

#include <ostream>

namespace NuHepMC {

namespace CrossSection {

namespace Units {

NEW_NuHepMC_EXCEPT(NonStandardUnitsUsed);
NEW_NuHepMC_EXCEPT(InvalidUnitType);
NEW_NuHepMC_EXCEPT(InvalidUnits);

enum class Scale { CustomType, pb, nb, cm2, cm2_ten38, Automatic };

enum class TargetScale { CustomType, PerTarget, PerTargetNucleon, Automatic };

struct Unit {
  Scale scale;
  TargetScale tgtscale;
  bool operator==(Unit const &other) const {
    return (scale == other.scale) && (tgtscale == other.tgtscale);
  }

  bool operator!=(Unit const &other) const { return !(*this == other); }
};

const Unit pb_PerTarget{Scale::pb, TargetScale::PerTarget};
const Unit cm2ten38_PerTarget{Scale::cm2_ten38, TargetScale::PerTarget};
const Unit pb_PerNucleon{Scale::pb, TargetScale::PerTargetNucleon};
const Unit cm2ten38_PerNucleon{Scale::cm2_ten38, TargetScale::PerTargetNucleon};
const Unit automatic{Scale::Automatic, TargetScale::Automatic};

const double pb = 1;
const double nb = 1E3;
const double cm2 = 1E36;
const double cm2_ten38 = 1E-2;

inline int NuclearPDG(int Z, int A) {
  // ±10LZZZAAAI
  return 1000000000 + (A * 10) + (Z * 10000);
}

inline int NuclearPDGToZ(int pid) {
  // ±10LZZZAAAI
  return (pid / 10000) % 1000;
}

inline int NuclearPDGToA(int pid) {
  // ±10LZZZAAAI
  return (pid / 1) % 1000;
}

inline int NuclearPDGToN(int pid) {
  return NuclearPDGToA(pid) - NuclearPDGToZ(pid);
}

} // namespace Units

} // namespace CrossSection

} // namespace NuHepMC

std::ostream &operator<<(std::ostream &os,
                         NuHepMC::CrossSection::Units::Scale us);

std::ostream &operator<<(std::ostream &os,
                         NuHepMC::CrossSection::Units::TargetScale ts);

std::ostream &operator<<(std::ostream &os,
                         NuHepMC::CrossSection::Units::Unit const &u);

namespace NuHepMC {
std::string to_string(NuHepMC::CrossSection::Units::Scale us);
std::string to_string(NuHepMC::CrossSection::Units::TargetScale ts);
std::string to_string(NuHepMC::CrossSection::Units::Unit const &u);
} // namespace NuHepMC
