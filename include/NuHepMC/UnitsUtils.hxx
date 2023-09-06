#pragma once

namespace NuHepMC {

namespace Units {
namespace CrossSection {
namespace Type {

const int CustomType = 1;

const int pb = 2;
const int cm2 = 3;
const int cm2_ten38 = 4;

const int PerTargetMolecule = 2;
const int PerTargetAtom = 3;
const int PerTargetNucleon = 4;
} // namespace Type

const double pb = 1;
const double cm2 = 1E36;
const double cm2_ten38 = 1E-2;

} // namespace CrossSection
} // namespace Units

} // namespace NuHepMC