#pragma once

#include "NuHepMC/Constants.hxx"

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenVertex.h"

#include <vector>

namespace NuHepMC {

namespace Event {

HepMC3::ConstGenVertexPtr GetVertex_First(HepMC3::GenEvent const &evt,
                                    int vtx_status);

HepMC3::ConstGenVertexPtr GetPrimaryVertex(HepMC3::GenEvent const &evt);

HepMC3::ConstGenParticlePtr GetBeamParticle(HepMC3::GenEvent const &evt);

HepMC3::ConstGenParticlePtr GetTargetParticle(HepMC3::GenEvent const &evt);

std::vector<HepMC3::ConstGenParticlePtr>
GetParticles_All(HepMC3::GenEvent const &evt, int part_status,
                 std::vector<int> PDGs = {});

std::vector<HepMC3::ConstGenParticlePtr>
GetParticles_AllRealFinalState(HepMC3::GenEvent const &evt,
                               std::vector<int> PDGs = {});

HepMC3::ConstGenParticlePtr GetParticle_First(HepMC3::GenEvent const &evt,
                                              int part_status,
                                              std::vector<int> PDGs = {});
HepMC3::ConstGenParticlePtr GetParticle_FirstRealFinalState(HepMC3::GenEvent const &evt,
                                              std::vector<int> PDGs = {});

HepMC3::ConstGenParticlePtr
GetParticle_HighestMomentum(HepMC3::GenEvent const &evt, int part_status,
                            std::vector<int> PDGs = {});
HepMC3::ConstGenParticlePtr
GetParticle_HighestMomentumRealFinalState(HepMC3::GenEvent const &evt,
                            std::vector<int> PDGs = {});

double ToMeVFactor(HepMC3::GenEvent const &evt);
} // namespace Event

namespace Vertex {

std::vector<HepMC3::ConstGenParticlePtr>
GetParticlesIn_All(HepMC3::ConstGenVertexPtr &evt, int part_status,
                   std::vector<int> PDGs = {});

HepMC3::ConstGenParticlePtr
GetParticleIn_HighestMomentum(HepMC3::ConstGenVertexPtr &evt, int part_status,
                              std::vector<int> PDGs = {});

std::vector<HepMC3::ConstGenParticlePtr>
GetParticlesOut_All(HepMC3::ConstGenVertexPtr &vtx, int part_status,
                    std::vector<int> PDGs = {});

HepMC3::ConstGenParticlePtr
GetParticleOut_HighestMomentum(HepMC3::ConstGenVertexPtr &evt, int part_status,
                               std::vector<int> PDGs = {});

} // namespace Vertex

} // namespace NuHepMC