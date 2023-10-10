#pragma once

namespace NuHepMC {

namespace CrossSection {
namespace Units {

enum class XSUnits { CustomType, pb, cm2, cm2_ten38 };

enum class XSTargetScale {
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