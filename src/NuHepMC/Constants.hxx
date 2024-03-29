#pragma once

namespace NuHepMC {

namespace VR1 {
const int Primary = 1;
const int FSISummary = 2;
} // namespace VR1

namespace VC1 {
const int NucleonSeparation = 21;
}

namespace VertexStatus {
const int Any = 0;
const int Primary = VR1::Primary;
const int FSISummary = VR1::FSISummary;
const int NucleonSeparation = VC1::NucleonSeparation;
} // namespace VertexStatus

namespace PR1 {
const int UndecayedPhysical = 1;
const int DecayedPhysical = 2;
const int DocumentationLine = 3;
const int IncomingBeam = 4;
const int Target = 20;
} // namespace PR1

namespace PC1 {
const int StruckNucleon = 21;
}

namespace PC2 {
const int NuclearRemnant = 2009900000;
}

namespace ParticleStatus {
const int Any = 0;
const int UndecayedPhysical = PR1::UndecayedPhysical;
const int DecayedPhysical = PR1::DecayedPhysical;
const int DocumentationLine = PR1::DocumentationLine;
const int IncomingBeam = PR1::IncomingBeam;
const int Target = PR1::Target;
const int StruckNucleon = PC1::StruckNucleon;

}; // namespace ParticleStatus

namespace ParticleNumber {
const int NuclearRemnant = PC2::NuclearRemnant;
}

} // namespace NuHepMC