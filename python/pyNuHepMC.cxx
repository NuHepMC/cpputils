#include "NuHepMC/Constants.hxx"
#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/FATXUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"
#include "NuHepMC/Types.hxx"

#include "pybind11/eigen.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include <string>

namespace py = pybind11;
using namespace NuHepMC;

class pyFATXAccumulator {
  std::shared_ptr<FATX::Accumulator> acc;

public:
  pyFATXAccumulator(std::shared_ptr<FATX::Accumulator> a) : acc(a) {}
  double process(HepMC3::GenEvent const &ev) { return acc->process(ev); }
  double fatx(CrossSection::Units::Unit const &units =
                  CrossSection::Units::pb_PerTarget) const {
    return acc->fatx(units);
  }
  double sumweights() const { return acc->sumweights(); }
  size_t events() const { return acc->events(); }
  std::string to_string() const { return acc->to_string(); }
};

pyFATXAccumulator pyMakeAccumulator(std::shared_ptr<HepMC3::GenRunInfo> gri) {
  return pyFATXAccumulator(FATX::MakeAccumulator(gri));
}

PYBIND11_MODULE(pyNuHepMC, m) {
  m.doc() = "NuHepMC_CPPUtils implementation in python";

  auto constants = m.def_submodule("Constants", "");

  auto vertex_status = constants.def_submodule("VertexStatus", "");
  vertex_status.attr("Primary") = VertexStatus::Primary;
  vertex_status.attr("FSISummary") = VertexStatus::FSISummary;
  vertex_status.attr("NucleonSeparation") = VertexStatus::NucleonSeparation;

  auto particle_status = constants.def_submodule("ParticleStatus", "");
  particle_status.attr("UndecayedPhysical") = ParticleStatus::UndecayedPhysical;
  particle_status.attr("DecayedPhysical") = ParticleStatus::DecayedPhysical;
  particle_status.attr("DocumentationLine") = ParticleStatus::DocumentationLine;
  particle_status.attr("IncomingBeam") = ParticleStatus::IncomingBeam;
  particle_status.attr("Target") = ParticleStatus::Target;
  particle_status.attr("StruckNucleon") = ParticleStatus::StruckNucleon;

  auto particle_number = constants.def_submodule("ParticleNumber", "");
  particle_number.attr("NuclearRemnant") = ParticleNumber::NuclearRemnant;

  auto event_utils = m.def_submodule("EventUtils", "");
  event_utils.def("GetVertex_First", &Event::GetVertex_First, "");
  event_utils.def("GetPrimaryVertex", &Event::GetPrimaryVertex, "");
  event_utils.def("GetBeamParticle", &Event::GetBeamParticle, "");
  event_utils.def("GetTargetParticle", &Event::GetTargetParticle, "");
  event_utils.def("GetParticles_All", &Event::GetParticles_All, "");
  event_utils.def("GetParticles_AllRealFinalState",
                  &Event::GetParticles_AllRealFinalState, "");
  event_utils.def("GetParticle_First", &Event::GetParticle_First, "");
  event_utils.def("GetParticle_FirstRealFinalState",
                  &Event::GetParticle_FirstRealFinalState, "");
  event_utils.def("GetParticle_HighestMomentum",
                  &Event::GetParticle_HighestMomentum, "");
  event_utils.def("GetParticle_HighestMomentumRealFinalState",
                  &Event::GetParticle_HighestMomentumRealFinalState, "");
  event_utils.def("ToMeVFactor", &Event::ToMeVFactor, "");

  event_utils.def("GetParticlesIn_All", &Vertex::GetParticlesIn_All, "");
  event_utils.def("GetParticleIn_HighestMomentum",
                  &Vertex::GetParticleIn_HighestMomentum, "");
  event_utils.def("GetParticlesOut_All", &Vertex::GetParticlesOut_All, "");
  event_utils.def("GetParticleOut_HighestMomentum",
                  &Vertex::GetParticleOut_HighestMomentum, "");

  auto units_utils = m.def_submodule("UnitsUtils", "");

  py::enum_<CrossSection::Units::Scale>(units_utils, "Scale")
      .value("CustomType", CrossSection::Units::Scale::CustomType)
      .value("pb", CrossSection::Units::Scale::pb)
      .value("cm2", CrossSection::Units::Scale::cm2)
      .value("cm2_ten38", CrossSection::Units::Scale::cm2_ten38)
      .value("Automatic", CrossSection::Units::Scale::Automatic);

  py::enum_<CrossSection::Units::TargetScale>(units_utils, "TargetScale")
      .value("CustomType", CrossSection::Units::TargetScale::CustomType)
      .value("PerTarget", CrossSection::Units::TargetScale::PerTarget)
      .value("PerTargetNucleon",
             CrossSection::Units::TargetScale::PerTargetNucleon)
      .value("Automatic", CrossSection::Units::TargetScale::Automatic);

  py::class_<CrossSection::Units::Unit>(units_utils, "Unit")
      .def_readwrite("scale", &CrossSection::Units::Unit::scale)
      .def_readwrite("tgtscale", &CrossSection::Units::Unit::tgtscale)
      .def("__str__", [](CrossSection::Units::Unit const &u) {
        std::stringstream ss;
        ss << u;
        return ss.str();
      });

  auto fatx_utils = m.def_submodule("FATXUtils", "");
  py::class_<pyFATXAccumulator>(fatx_utils, "FATXAccumulator")
      .def("process", &pyFATXAccumulator::process)
      .def("fatx", &pyFATXAccumulator::fatx,
           py::arg("fatx") = NuHepMC::CrossSection::Units::pb_PerTarget)
      .def(
          "fatx",
          [](pyFATXAccumulator const &fatxacc, std::string const &units_scale,
             std::string const &tgt_scale) {
            return fatxacc.fatx(
                NuHepMC::GC4::ParseCrossSectionUnits({units_scale, tgt_scale}));
          },
          py::arg("units_scale") = "pb", py::arg("tgt_scale") = "PerTarget")
      .def("sumweights", &pyFATXAccumulator::sumweights)
      .def("events", &pyFATXAccumulator::events)
      .def("__str__", &pyFATXAccumulator::to_string);
  fatx_utils.def("make_accumulator", &pyMakeAccumulator, "");
  fatx_utils.attr("pb_PerTarget") = NuHepMC::CrossSection::Units::pb_PerTarget;
  fatx_utils.attr("cm2ten38_PerTarget") =
      NuHepMC::CrossSection::Units::cm2ten38_PerTarget;
  fatx_utils.attr("pb_PerNucleon") =
      NuHepMC::CrossSection::Units::pb_PerNucleon;
  fatx_utils.attr("cm2ten38_PerNucleon") =
      NuHepMC::CrossSection::Units::cm2ten38_PerNucleon;

  auto reader_utils = m.def_submodule("ReaderUtils", "");
  auto reader_utils_gc7 = reader_utils.def_submodule("GC7", "");

  py::enum_<GC7::EDistType>(reader_utils_gc7, "EDistType")
      .value("Invalid", GC7::EDistType::kInvalid)
      .value("MonoEnergetic", GC7::EDistType::kMonoEnergetic)
      .value("Histogram", GC7::EDistType::kHistogram);

  py::class_<GC7::EnergyDistribution>(reader_utils_gc7, "EnergyDistribution")
      .def_readwrite("dist_type", &GC7::EnergyDistribution::dist_type)
      .def_readwrite("MonoEnergeticEnergy",
                     &GC7::EnergyDistribution::MonoEnergeticEnergy)
      .def_readwrite("energy_unit", &GC7::EnergyDistribution::energy_unit)
      .def_readwrite("rate_unit", &GC7::EnergyDistribution::rate_unit)
      .def_readwrite("bin_edges", &GC7::EnergyDistribution::bin_edges)
      .def_readwrite("bin_content", &GC7::EnergyDistribution::bin_content)
      .def_readwrite("content_is_per_width",
                     &GC7::EnergyDistribution::ContentIsPerWidth)
      .def("get_bin_widths", &GC7::EnergyDistribution::get_bin_widths)
      .def("get_integral", &GC7::EnergyDistribution::get_integral)
      .def("get_flux_density", &GC7::EnergyDistribution::get_flux_density)
      .def("get_flux_shape_density",
           &GC7::EnergyDistribution::get_flux_shape_density)
      .def("get_flux_rate", &GC7::EnergyDistribution::get_flux_rate)
      .def("get_flux_shape_rate", &GC7::EnergyDistribution::get_flux_shape_rate)
      .def("set_units", &GC7::EnergyDistribution::set_units)
      .def("get_bin_centers", &GC7::EnergyDistribution::get_bin_centers)
      .def("is_in_GeV", &GC7::EnergyDistribution::is_in_GeV)
      .def("is_in_MeV", &GC7::EnergyDistribution::is_in_MeV);

  reader_utils_gc7.def("read_all_energy_distributions",
                       &GC7::ReadAllEnergyDistributions);
  reader_utils_gc7.def("read_energy_distribution",
                       &GC7::ReadEnergyDistribution);
  reader_utils_gc7.def("has_energy_distribution", &GC7::HasEnergyDistribution);
}