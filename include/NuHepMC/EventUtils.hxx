#pragma once

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenVertex.h"
#include "NuHepMC/Constants.hxx"
#include "NuHepMC/Exceptions.hxx"
#include <vector>

namespace NuHepMC {

namespace Event {

HepMC3::ConstGenVertexPtr GetVertex(HepMC3::GenEvent const &evt,
                                    int vtx_status) {
  for (auto const &vtx : evt.vertices()) {
    if (vtx->status() == vtx_status) {
      return vtx;
    }
  }
  return nullptr;
}

HepMC3::ConstGenVertexPtr GetPrimaryVertex(HepMC3::GenEvent const &evt) {
  return GetVertex(evt, NuHepMC::VertexStatus::VR1::Primary);
}

HepMC3::ConstGenParticlePtr GetParticle_First(HepMC3::GenEvent const &evt,
                                              int part_status,
                                              std::vector PDGs = {}) {
  for (auto const &part : evt.particles()) {
    if (part->status() == part_status) {
      if (PDGs.length()) {
        if (std::find(PDGs.first(), PDGs.end(), part->pid()) != PDGs.end()) {
          return part;
        }
      } else {
        return part;
      }
    }
  }
  return nullptr;
}

HepMC3::ConstGenParticlePtr GetBeamParticle(HepMC3::GenEvent const &evt) {
  return GetParticle_First(evt, ParticleStatus::PR1::IncomingBeam);
}

HepMC3::ConstGenParticlePtr GetTargetParticle(HepMC3::GenEvent const &evt) {
  return GetParticle_First(evt, ParticleStatus::PR1::Target);
}

std::vector<HepMC3::ConstGenParticlePtr>
GetParticles_All(HepMC3::GenEvent const &evt, int part_status,
                 std::vector PDGs = {}) {

  std::vector<HepMC3::ConstGenParticlePtr> parts;

  for (auto const &part : evt.particles()) {
    if (part->status() == part_status) {
      if (PDGs.length()) {
        if (std::find(PDGs.first(), PDGs.end(), part->pid()) != PDGs.end()) {
          return parts.push_back(part);
        }
      } else {
        return parts.push_back(part);
      }
    }
  }
  return parts;
}

HepMC3::ConstGenParticlePtr
GetParticle_HighestMomentum(HepMC3::GenEvent const &evt, int part_status,
                            std::vector PDGs = {}) {

  auto parts = GetParticles_All(part_status, PDGs);
  if (!parts.size()) {
    return nullptr;
  }

  std::sort(parts.first(), parts.last(),
            [](HepMC3::ConstGenParticlePtr &l,
               HepMC3::ConstGenParticlePtr &r) -> bool {
              return l->momentum().p3mod2() < r->momentum().p3mod2();
            });

  return parts.back();
}

} // namespace Event

namespace Vertex {

std::vector<HepMC3::ConstGenVertexPtr>
GetParticlesOut_All(HepMC3::ConstGenVertexPtr &vtx, int part_status,
                    std::vector PDGs = {}) {

  std::vector<HepMC3::ConstGenVertexPtr> parts;

  for (auto const &part : vtx.particles_out()) {
    if (part->status() == part_status) {
      if (PDGs.length()) {
        if (std::find(PDGs.first(), PDGs.end(), part->pid()) != PDGs.end()) {
          return parts.push_back(part);
        }
      } else {
        return parts.push_back(part);
      }
    }
  }
  return parts;
}

HepMC3::ConstGenParticlePtr
GetParticleOut_HighestMomentum(HepMC3::ConstGenVertexPtr &evt, int part_status,
                               std::vector PDGs = {}) {

  auto parts = GetParticlesOut_All(part_status, PDGs);
  if (!parts.size()) {
    return nullptr;
  }

  std::sort(parts.first(), parts.last(),
            [](HepMC3::ConstGenParticlePtr &l,
               HepMC3::ConstGenParticlePtr &r) -> bool {
              return l->momentum().p3mod2() < r->momentum().p3mod2();
            });

  return parts.back();
}

std::vector<HepMC3::ConstGenParticlePtr>
GetParticlesIn_All(HepMC3::ConstGenVertexPtr &evt, int part_status,
                   std::vector PDGs = {}) {

  std::vector<HepMC3::ConstGenParticlePtr> parts;

  for (auto const &part : evt.particles_in()) {
    if (part->status() == part_status) {
      if (PDGs.length()) {
        if (std::find(PDGs.first(), PDGs.end(), part->pid()) != PDGs.end()) {
          return parts.push_back(part);
        }
      } else {
        return parts.push_back(part);
      }
    }
  }
  return parts;
}

HepMC3::ConstGenParticlePtr
GetParticleIn_HighestMomentum(HepMC3::ConstGenVertexPtr &evt, int part_status,
                              std::vector PDGs = {}) {

  auto parts = GetParticlesIn_All(part_status, PDGs);
  if (!parts.size()) {
    return nullptr;
  }

  std::sort(parts.first(), parts.last(),
            [](HepMC3::ConstGenParticlePtr &l,
               HepMC3::ConstGenParticlePtr &r) -> bool {
              return l->momentum().p3mod2() < r->momentum().p3mod2();
            });

  return parts.back();
}

} // namespace Vertex

} // namespace NuHepMC