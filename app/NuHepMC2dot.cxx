#include "HepMC3/ReaderFactory.h"

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "fmt/os.h"

#include <fstream>
#include <iostream>

NuHepMC::StatusCodeDescriptors vtxstatus;
NuHepMC::StatusCodeDescriptors partstatus;
NuHepMC::StatusCodeDescriptors proc_ids;
double ToGeV = 1;

void WriteVertex(HepMC3::GenVertexPtr vtx, std::ostream &os) {

  // NuHepMC::VertexStatus::Primary NuHepMC::VertexStatus::FSISummary
  //     NuHepMC::VertexStatus::NucleonSeparation

  std::string color = "black";
  if (vtx->status() == NuHepMC::VertexStatus::Primary) {
    color = "crimson";
  } else if (vtx->status() == NuHepMC::VertexStatus::FSISummary) {
    color = "darkseagreen1";
  } else if (vtx->status() == NuHepMC::VertexStatus::NucleonSeparation) {
    color = "deepskyblue4";
  }

  std::string status =
      vtxstatus.count(vtx->status())
          ? vtxstatus.at(vtx->status()).first
          : std::string("status: ") + std::to_string(vtx->status());

  os << fmt::format(R"(  v{}[label="{}", shape=circle, color={}];)",
                    std::abs(vtx->id()), status, color)
     << std::endl;
}

std::map<int, std::string> pidmap = {
    {12, "nue"},       {-12, "anue"}, {14, "numu"},  {-14, "anumu"},
    {11, "e-"},        {-11, "e+"},   {13, "mu-"},   {-13, "mu+"},
    {111, "pi0"},      {211, "pi+"},  {-211, "pi-"}, {2212, "proton"},
    {2112, "neutron"},
};

void WriteEdge(HepMC3::GenParticlePtr pt, std::ostream &os, int &dummyvtxctr) {
  std::string ov, ev;

  std::string status =
      partstatus.count(pt->status())
          ? partstatus.at(pt->status()).first
          : std::string("status: ") + std::to_string(pt->status());

  std::string part_type =
      pidmap.count(pt->pid())
          ? pidmap.at(pt->pid())
          : std::string("pid: ") + std::to_string(pt->pid());

  auto mom = pt->momentum() * ToGeV;

  std::string mom_label = fmt::format(R"(p3 = ({:.3f}, {:.3f}, {:.3f}))", mom.x(),
                                      mom.y(), mom.z(), mom.e());

  std::string label = part_type + " [" + status + "]\\n" + mom_label;

  std::string color = "black";
  if (pt->status() == NuHepMC::ParticleStatus::UndecayedPhysical) {
    color = "chartreuse4";
  } else if ((pt->status() == NuHepMC::ParticleStatus::IncomingBeam) ||
             (pt->status() == NuHepMC::ParticleStatus::Target)) {
    color = "cornflowerblue";
  }

  if (pt->pid() == NuHepMC::ParticleNumber::NuclearRemnant) {
    color = "gray";
    label = "Nuclear Remnant";
  } else if (pt->pid() > 1E8) {
    int a = (pt->pid() / 10) % 1000;
    int z = (pt->pid() / 10000) % 1000;
    label = "A: " + std::to_string(a) + " Z: " + std::to_string(z) + "[" +
            status + "]";
  }

  if (pt->production_vertex() && (pt->production_vertex()->id() != 0)) {
    ov = "v" + std::to_string(std::abs(pt->production_vertex()->id()));
  } else {
    ov = "o" + std::to_string(dummyvtxctr++);
    os << fmt::format(R"(  {}[shape=point,color={}];)", ov, color) << std::endl;
  }
  if (pt->end_vertex() && (pt->end_vertex()->id() != 0)) {
    ev = "v" + std::to_string(std::abs(pt->end_vertex()->id()));
  } else {
    ev = "o" + std::to_string(dummyvtxctr++);
    os << fmt::format(R"(  {}[shape=point,color={}];)", ev, color) << std::endl;
  }

  os << fmt::format(R"(  {} -> {} [label="{}", color={}];)", ov, ev, label,
                    color)
     << std::endl;
}

int WriteEvent(HepMC3::GenEvent &evt, std::ostream &os) {

  os << "digraph nuhepmc_ev {" << std::endl;

  std::set<int> vids;
  for (auto &vtx : evt.vertices()) {
    WriteVertex(vtx, os);
    vids.insert(vtx->id());
  }

  int dummyvtxctr = 0;
  for (auto &pt : evt.particles()) {

    if (pt->production_vertex() && pt->production_vertex()->id() &&
        !vids.count(pt->production_vertex()->id())) {
      std::cout << " -- prodvtx id: " << pt->production_vertex()->id()
                << " not yet written." << std::endl;
    }
    if (pt->end_vertex() && pt->end_vertex()->id() &&
        !vids.count(pt->end_vertex()->id())) {
      std::cout << " -- endvtx id: " << pt->end_vertex()->id()
                << " not yet written." << std::endl;
    }

    WriteEdge(pt, os, dummyvtxctr);
  }

  try {
    std::string event_label =
        fmt::format(R"(  labelloc="t" label="{}";)",
                    proc_ids[NuHepMC::ER3::ReadProcessID(evt)].first);
    os << event_label << std::endl;
  } catch (...) {
  }

  os << "}" << std::endl;

  return 0;
}

int main(int argc, char const *argv[]) {

  if (argc < 2) {
    std::cout << "[RUNLIKE]: " << argv[0] << " <infile.hepmc3> <event_number>"
              << std::endl;
    return 1;
  }

  std::string inf = argv[1];
  long evid = std::stol(argv[2]);

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

    if (!NEvents) {

      proc_ids = NuHepMC::GR4::ReadProcessIdDefinitions(evt.run_info());
      vtxstatus = NuHepMC::GR5::ReadVertexStatusIdDefinitions(evt.run_info());

      // tidy up some known labels
      vtxstatus[NuHepMC::VertexStatus::Primary] =
          std::pair<std::string, std::string>{"Primary\\nVertex", ""};
      vtxstatus[NuHepMC::VertexStatus::FSISummary] =
          std::pair<std::string, std::string>{"FSI\\nVertex", ""};
      vtxstatus[NuHepMC::VertexStatus::NucleonSeparation] =
          std::pair<std::string, std::string>{"Nucleon\\nSeparation Vertex",
                                              ""};

      partstatus =
          NuHepMC::GR6::ReadParticleStatusIdDefinitions(evt.run_info());

      partstatus[NuHepMC::ParticleStatus::UndecayedPhysical] =
          std::pair<std::string, std::string>{"final state", ""};
      partstatus[NuHepMC::ParticleStatus::DecayedPhysical] =
          std::pair<std::string, std::string>{"decayed", ""};
      partstatus[NuHepMC::ParticleStatus::DocumentationLine] =
          std::pair<std::string, std::string>{"docline", ""};
      partstatus[NuHepMC::ParticleStatus::IncomingBeam] =
          std::pair<std::string, std::string>{"beam", ""};
      partstatus[NuHepMC::ParticleStatus::Target] =
          std::pair<std::string, std::string>{"target", ""};
      partstatus[NuHepMC::ParticleStatus::StruckNucleon] =
          std::pair<std::string, std::string>{"struck nucleon", ""};

      ToGeV = NuHepMC::Event::ToMeVFactor(evt) * 1E-3;
    }

    if (evt.event_number() == evid) {
      return WriteEvent(evt, std::cout);
    }

    if (!rdr->failed()) {
      NEvents++;
    } else {
      break;
    }
  }

  std::cout << "Failed to find event " << evid << std::endl;

  return 1;
}