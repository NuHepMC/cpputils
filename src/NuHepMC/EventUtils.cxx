#include "NuHepMC/EventUtils.hxx"

#include "NuHepMC/Exceptions.hxx"

#include "HepMC3/GenParticle.h"

#include <vector>

namespace NuHepMC {

namespace Event {

HepMC3::ConstGenVertexPtr GetVertex_First(HepMC3::GenEvent const &evt,
                                          int vtx_status) {
  for (auto const &vtx : evt.vertices()) {
    if (vtx->status() == vtx_status) {
      return vtx;
    }
  }
  return nullptr;
}

HepMC3::ConstGenVertexPtr GetPrimaryVertex(HepMC3::GenEvent const &evt) {
  return GetVertex_First(evt, NuHepMC::VertexStatus::Primary);
}

HepMC3::ConstGenParticlePtr GetBeamParticle(HepMC3::GenEvent const &evt) {
  return GetParticle_First(evt, ParticleStatus::IncomingBeam);
}

HepMC3::ConstGenParticlePtr GetTargetParticle(HepMC3::GenEvent const &evt) {
  auto pt_nhmctgt = GetParticle_First(evt, ParticleStatus::Target);
  if (pt_nhmctgt) {
    return pt_nhmctgt;
  }
  // for legacy apps
  return GetParticle_First(evt, 11);
}

std::vector<HepMC3::ConstGenParticlePtr>
GetParticles_All(HepMC3::GenEvent const &evt, int part_status,
                 std::vector<int> PDGs) {

  std::vector<HepMC3::ConstGenParticlePtr> parts;

  for (auto const &part : evt.particles()) {
    if (part->status() == part_status) {
      if (PDGs.size()) {
        if (std::find(PDGs.begin(), PDGs.end(), part->pid()) != PDGs.end()) {
          parts.push_back(part);
        }
      } else {
        parts.push_back(part);
      }
    }
  }
  return parts;
}

std::vector<HepMC3::ConstGenParticlePtr>
GetParticles_AllRealFinalState(HepMC3::GenEvent const &evt,
                               std::vector<int> PDGs) {
  return GetParticles_All(evt, ParticleStatus::UndecayedPhysical, PDGs);
}

HepMC3::ConstGenParticlePtr GetParticle_First(HepMC3::GenEvent const &evt,
                                              int part_status,
                                              std::vector<int> PDGs) {
  for (auto const &part : evt.particles()) {
    if (part->status() == part_status) {
      if (PDGs.size()) {
        if (std::find(PDGs.begin(), PDGs.end(), part->pid()) != PDGs.end()) {
          return part;
        }
      } else {
        return part;
      }
    }
  }
  return nullptr;
}

HepMC3::ConstGenParticlePtr
GetParticle_HighestMomentum(HepMC3::GenEvent const &evt, int part_status,
                            std::vector<int> PDGs) {

  auto parts = GetParticles_All(evt, part_status, PDGs);
  if (!parts.size()) {
    return nullptr;
  }

  std::sort(parts.begin(), parts.end(),
            [](HepMC3::ConstGenParticlePtr &l,
               HepMC3::ConstGenParticlePtr &r) -> bool {
              return l->momentum().p3mod2() < r->momentum().p3mod2();
            });

  return parts.back();
}

double ToMeVFactor(HepMC3::GenEvent const &evt) {
  return (evt.momentum_unit() == HepMC3::Units::MEV) ? 1 : 1E3;
}

} // namespace Event

namespace Vertex {

std::vector<HepMC3::ConstGenParticlePtr>
GetParticlesIn_All(HepMC3::ConstGenVertexPtr &evt, int part_status,
                   std::vector<int> PDGs) {

  std::vector<HepMC3::ConstGenParticlePtr> parts;

  for (auto const &part : evt->particles_in()) {
    if (part->status() == part_status) {
      if (PDGs.size()) {
        if (std::find(PDGs.begin(), PDGs.end(), part->pid()) != PDGs.end()) {
          parts.push_back(part);
        }
      } else {
        parts.push_back(part);
      }
    }
  }
  return parts;
}

HepMC3::ConstGenParticlePtr
GetParticleIn_HighestMomentum(HepMC3::ConstGenVertexPtr &evt, int part_status,
                              std::vector<int> PDGs) {

  auto parts = GetParticlesIn_All(evt, part_status, PDGs);
  if (!parts.size()) {
    return nullptr;
  }

  std::sort(parts.begin(), parts.end(),
            [](HepMC3::ConstGenParticlePtr &l,
               HepMC3::ConstGenParticlePtr &r) -> bool {
              return l->momentum().p3mod2() < r->momentum().p3mod2();
            });

  return parts.back();
}

std::vector<HepMC3::ConstGenParticlePtr>
GetParticlesOut_All(HepMC3::ConstGenVertexPtr &vtx, int part_status,
                    std::vector<int> PDGs) {

  std::vector<HepMC3::ConstGenParticlePtr> parts;

  for (auto const &part : vtx->particles_out()) {
    if (part->status() == part_status) {
      if (PDGs.size()) {
        if (std::find(PDGs.begin(), PDGs.end(), part->pid()) != PDGs.end()) {
          parts.push_back(part);
        }
      } else {
        parts.push_back(part);
      }
    }
  }
  return parts;
}

HepMC3::ConstGenParticlePtr
GetParticleOut_HighestMomentum(HepMC3::ConstGenVertexPtr &evt, int part_status,
                               std::vector<int> PDGs) {

  auto parts = GetParticlesOut_All(evt, part_status, PDGs);
  if (!parts.size()) {
    return nullptr;
  }

  std::sort(parts.begin(), parts.end(),
            [](HepMC3::ConstGenParticlePtr &l,
               HepMC3::ConstGenParticlePtr &r) -> bool {
              return l->momentum().p3mod2() < r->momentum().p3mod2();
            });

  return parts.back();
}

} // namespace Vertex

} // namespace NuHepMC