// Leave this at the top to enable features detected at build time in headers in
// HepMC3
#include "NuHepMC/HepMC3Features.hxx"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"
#include "NuHepMC/WriterUtils.hxx"
#include "NuHepMC/make_writer.hxx"

#include "HepMC3/ReaderFactory.h"

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"

int main(int argc, char const *argv[]) {

  auto rdr = HepMC3::deduce_reader(argv[1]);
  if (!rdr) {
    std::cout << "Failed to instantiate HepMC3::Reader from " << argv[1]
              << std::endl;
    return 1;
  }

  // for files that you know can be opened multiple times without failing (i.e.
  // not a stream) it is easiest to read one event, and grab the
  // HepMC3::GenRunInfo here.
  HepMC3::GenEvent evt;
  rdr->read_event(evt);
  if (rdr->failed()) {
    std::cout << "Failed to read first event from " << argv[1] << "."
              << std::endl;
    return 1;
  }

  auto in_gen_run_info = evt.run_info();
  auto vtx_statuses =
      NuHepMC::GR5::ReadVertexStatusIdDefinitions(in_gen_run_info);
  auto part_statuses =
      NuHepMC::GR6::ReadParticleStatusIdDefinitions(in_gen_run_info);

  // modify gen_run_info here to add to the file provenance that your code has
  // run on the file

  auto out_gen_run_info =
      std::make_shared<HepMC3::GenRunInfo>(*in_gen_run_info);

  constexpr int MyFSIVertexStatus = 123;
  vtx_statuses[MyFSIVertexStatus] = {"leptonfsi_dragq",
                                     "my process description"};
  part_statuses[MyFSIVertexStatus] = {"qdragged_lepton",
                                      "my particle status description"};

  out_gen_run_info->tools().push_back(HepMC3::GenRunInfo::ToolInfo{
      "MyToolName", "My.Tool.Version",
      "a longer form description of this event processor"});

  NuHepMC::GR5::WriteVertexStatusIDDefinitions(out_gen_run_info, vtx_statuses);
  NuHepMC::GR6::WriteParticleStatusIDDefinitions(out_gen_run_info,
                                                 part_statuses);

  // add link to your paper describing this model to the citation metadata
  NuHepMC::GC6::AddGeneratorCitation(out_gen_run_info, "arxiv",
                                     {
                                         "2404.12345v3",
                                     });

  auto wrtr = std::unique_ptr<HepMC3::Writer>(
      NuHepMC::Writer::make_writer(argv[2], out_gen_run_info));

  // re-open the file so that you start at the beginning
  rdr = HepMC3::deduce_reader(argv[1]);
  size_t nprocessed = 0;
  while (true) { // loop while there are events

    rdr->read_event(evt);
    if (rdr->failed()) {
      std::cout << "Reached the end of the file after " << nprocessed
                << " events." << std::endl;
      break;
    }
    // ensure units are in MeV will perform conversions if the previous
    // generation step used GeV
    evt.set_units(HepMC3::Units::MEV, HepMC3::Units::MM);

    auto beampt = NuHepMC::Event::GetBeamParticle(evt);
    auto tgtpt = NuHepMC::Event::GetTargetParticle(evt);

    if (!beampt || !tgtpt) {  // this event didn't have a beam particle or
                              // target, its an odd one
      wrtr->write_event(evt); // write out events that we don't modify
      continue;
    }

    auto primary_vtx = NuHepMC::Event::GetPrimaryVertex(evt);

    auto ccfslep_pid =
        (beampt->pid() > 0) ? (beampt->pid() - 1) : (beampt->pid() + 1);

    // grab all primary leptons that were considered 'final state' by the
    // previous simulation
    //  might have to adjust for simulations that include lepton FSI already
    auto primary_leptons = NuHepMC::Vertex::GetParticlesOut_All(
        primary_vtx, NuHepMC::ParticleStatus::UndecayedPhysical, {ccfslep_pid});

    if (!primary_leptons.size()) { // this event had no primary leptons.
      wrtr->write_event(evt);      // write out events that we don't modify
      continue;
    }

    if (primary_leptons.size() > 1) { // decide what to do here
      // maybe sort in non-descending order of momentum?
      std::sort(primary_leptons.begin(), primary_leptons.end(),
                [](auto const &evl, auto const &evr) {
                  return evl->momentum().e() < evr->momentum().e();
                });
    }

    auto fslep = primary_leptons.back();

    auto Elep = fslep->momentum().e();
    auto Q2 = -(beampt->momentum() - fslep->momentum()).m2();

    // calculate some modification to the lepton
    // as an example we drag the lepton by 10 MeV in the -q direction without
    // conserving 4mom
    auto qvec = (beampt->momentum() - fslep->momentum());
    qvec *= 10 / qvec.length();
    qvec.setE(0);

    // get a non-const handle to the FSLep
    auto fslep_preFSI = evt.particles()[fslep->id()];
    fslep_preFSI->set_status(
        MyFSIVertexStatus); // set a status code corresponding to underwent your
                            // FSI

    auto fslep_postFSI = std::make_shared<HepMC3::GenParticle>(
        fslep_preFSI->data()); // copy the preFSI particle
    // apply modifications to kinematics.
    fslep_postFSI->set_momentum(fslep_postFSI->momentum() - qvec);

    // make sure this postFSI lepton is set to be undecayed physical particle so
    // later simulation steps know how to handle it
    fslep_postFSI->set_status(NuHepMC::ParticleStatus::UndecayedPhysical);

    // make a new vertex to represent the FSI
    auto lepFSIvtx = std::make_shared<HepMC3::GenVertex>();
    lepFSIvtx->set_status(
        MyFSIVertexStatus); // set the vertex status for your FSI process.
    evt.add_vertex(
        lepFSIvtx); // add the vertex to the event before adding particles to it
    lepFSIvtx->add_particle_in(fslep_preFSI);
    lepFSIvtx->add_particle_out(fslep_postFSI);

    wrtr->write_event(evt); // write out events your modified event
  }
  wrtr->close();
}