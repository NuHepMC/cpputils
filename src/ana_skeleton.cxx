#include "HepMC3/ReaderFactory.h"

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"

#include "NuHepMC/CrossSectionUtils.hxx"
#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include <iostream>
#include <sstream>

double ToGeV = 1;
double fatx = 1;

NuHepMC::StatusCodeDescriptors vtxstatus;
NuHepMC::StatusCodeDescriptors partstatus;
NuHepMC::StatusCodeDescriptors proc_ids;

std::string PartToStr(HepMC3::ConstGenParticlePtr pt) {
  if (!pt) {
    return "PARTICLE-NOTFOUND";
  }
  std::stringstream ss;

  std::string status = partstatus.count(pt->status())
                           ? partstatus.at(pt->status()).first
                           : std::to_string(pt->status());

  ss << "{ pid: " << pt->pid() << ", status: " << status << ", p: ( "
     << pt->momentum().x() << ", " << pt->momentum().y() << ", "
     << pt->momentum().z() << ", E: " << pt->momentum().e() << ") GeV }";

  return ss.str();
}

void ProcessEvent(HepMC3::GenEvent &evt) {

  std::cout << "Evt: " << evt.event_number() << std::endl;
  std::cout << "  NVertices = " << evt.vertices().size()
            << ", NParticles = " << evt.particles().size() << std::endl;
  std::cout << "  Beam particle = "
            << PartToStr(NuHepMC::Event::GetBeamParticle(evt)) << std::endl;
  std::cout << "  Target particle = "
            << PartToStr(NuHepMC::Event::GetTargetParticle(evt)) << std::endl;
  std::cout << "  Primary Vertex:" << std::endl;
  size_t ctr = 0;
  for (auto &pt : NuHepMC::Event::GetPrimaryVertex(evt)->particles_in()) {
    std::cout << "    in[" << ctr++ << "]: " << PartToStr(pt) << std::endl;
  }
  ctr = 0;
  for (auto &pt : NuHepMC::Event::GetPrimaryVertex(evt)->particles_out()) {
    std::cout << "    out[" << ctr++ << "]: " << PartToStr(pt) << std::endl;
  }
}

int main(int argc, char const *argv[]) {

  if (argc < 2) {
    std::cout << "[RUNLIKE]: " << argv[0] << " <infile.hepmc3>" << std::endl;
    return 1;
  }

  std::string inf = argv[1];

  fatx = NuHepMC::CrossSection::GetFATX(inf);
  std::cout << "FATX = " << fatx << " pb/atom" << std::endl;

  auto rdr = HepMC3::deduce_reader(inf);
  if (!rdr) {
    std::cout << "Failed to instantiate HepMC3::Reader from " << inf
              << std::endl;
    return 1;
  }

  HepMC3::GenEvent evt;

  size_t NEvents = 0;
  while (!rdr->failed()) {
    rdr->read_event(evt);

    if (!NEvents) { // can only reliably read run_info after reading an event,
                    // so do it on the first one
      ToGeV = NuHepMC::Event::ToMeVFactor(evt) * 1E-3;

      proc_ids = NuHepMC::GR4::ReadProcessIdDefinitions(evt.run_info());
      vtxstatus = NuHepMC::GR5::ReadVertexStatusIdDefinitions(evt.run_info());
      partstatus =
          NuHepMC::GR6::ReadParticleStatusIdDefinitions(evt.run_info());
    }

    if (!rdr->failed()) {
      NEvents++;
    } else {
      break;
    }

    ProcessEvent(evt);
  }
}