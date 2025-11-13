// Leave this at the top to enable features detected at build time in headers in
// HepMC3
#include "NuHepMC/HepMC3Features.hxx"

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"
#include "NuHepMC/Reader.hxx"

#include <iostream>
#include <sstream>

double ToGeV = 1;
double fatx = 1;

using namespace NuHepMC;

StatusCodeDescriptors vtxstatus;
StatusCodeDescriptors partstatus;
StatusCodeDescriptors proc_ids;

std::string PartToStr(HepMC3::ConstGenParticlePtr pt) {
  if (!pt) {
    return "PARTICLE-NOTFOUND";
  }
  std::stringstream ss;

  std::string status = partstatus.count(pt->status())
                           ? partstatus.at(pt->status()).first
                           : std::to_string(pt->status());

  auto mom = pt->momentum() * ToGeV;

  ss << "{ id: " << pt->id() << ", pid: " << pt->pid() << ", status: " << status
     << ", p: ( " << mom.x() << ", " << mom.y() << ", "
     << mom.z() << ", E: " << mom.e() << ") GeV }";

  return ss.str();
}

void ProcessEvent(HepMC3::GenEvent &evt) {

  auto beampt = Event::GetBeamParticle(evt);
  auto tgtpt = Event::GetTargetParticle(evt);

  auto primary_vtx = Event::GetPrimaryVertex(evt);

  auto process_id = ER3::ReadProcessID(evt);

  std::cout << "Evt: " << evt.event_number()
            << ", channel name: " << proc_ids[process_id].first << std::endl;
  std::cout << "  NVertices = " << evt.vertices().size()
            << ", NParticles = " << evt.particles().size() << std::endl;
  std::cout << "  Beam particle = " << PartToStr(beampt) << std::endl;
  std::cout << "  Target particle = " << PartToStr(tgtpt) << std::endl;
  std::cout << "  Primary Vertex:" << std::endl;
  size_t ctr = 0;
  for (auto &pt : primary_vtx->particles_in()) {
    std::cout << "    in[" << ctr++ << "]: " << PartToStr(pt) << std::endl;
  }
  ctr = 0;
  for (auto &pt : primary_vtx->particles_out()) {
    std::cout << "    out[" << ctr++ << "]: " << PartToStr(pt) << std::endl;
  }
  std::cout << "  Final state particles:" << std::endl;
  ctr = 0;
  for (auto &pt : Event::GetParticles_AllRealFinalState(evt)) {
    std::cout << "    fs[" << ctr++ << "]: " << PartToStr(pt) << std::endl;
  }
}

int main(int argc, char const *argv[]) {

  if (argc < 2) {
    std::cout << "[RUNLIKE]: " << argv[0] << " <infile.hepmc3>" << std::endl;
    return 1;
  }

  std::string inf = argv[1];

  auto rdr = std::make_unique<NuHepMC::Reader>(inf);
  if (!rdr) {
    std::cout << "Failed to instantiate HepMC3::Reader from " << inf
              << std::endl;
    return 1;
  }

  HepMC3::GenEvent evt;
  rdr->read_event(evt);
  if (rdr->failed()) {
    std::cout << "Failed to read the first event from " << inf << std::endl;
    return 1;
  }

  // grab a copy of the GenRunInfo
  auto run_info = rdr->run_info();
  if (!run_info) {
    std::cout << "Warning, after reading the first event, we have no "
                 "GenRunInfo."
              << std::endl;
    // decide if this is a failure for your analysis.
    // return 1;
  }

  auto FATXAcc = FATX::MakeAccumulator(run_info);
  // read the various status code definitions
  proc_ids = GR8::ReadProcessIdDefinitions(run_info);
  vtxstatus = GR9::ReadVertexStatusIdDefinitions(run_info);
  partstatus = GR10::ReadParticleStatusIdDefinitions(run_info);

  // determine the units scale
  ToGeV = Event::ToMeVFactor(evt) * 1E-3;

  // re-open the file to start from the first event
  rdr = std::make_unique<NuHepMC::Reader>(inf);
  while (true) {

    // read an event and check that you haven't finished the file.
    // A file only knows it has read the last event after trying to read the
    // next one
    rdr->read_event(evt);
    if (rdr->failed()) {
      break;
    }

    // G.R.7 CV weight
    double evw = FATXAcc->process(evt);

    // .. do something with evt
    ProcessEvent(evt);
  }

  double fatx = FATXAcc->fatx(); // in pb/Atom
  double sumw = FATXAcc->sumweights();
  size_t nevents = FATXAcc->events();
}
