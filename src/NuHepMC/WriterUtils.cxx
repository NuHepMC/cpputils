#include "NuHepMC/WriterUtils.hxx"

#include "NuHepMC/Exceptions.hxx"
#include "NuHepMC/NuHepMCVersion.hxx"
#include "NuHepMC/ReaderUtils.hxx"
#include "NuHepMC/Traits.hxx"

namespace NuHepMC {

NEW_NuHepMC_EXCEPT(InvalidBeamEnergyDescription);

namespace GR2 {
void WriteVersion(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  add_attribute(run_info, "NuHepMC.Version.Major", NuHepMC_VERSION_MAJOR);
  add_attribute(run_info, "NuHepMC.Version.Minor", NuHepMC_VERSION_MINOR);
  add_attribute(run_info, "NuHepMC.Version.Patch", NuHepMC_VERSION_PATCH);
}
} // namespace GR2

void WriteIDDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo> &run_info,
    StatusCodeDescriptors const &Definitions,
    std::pair<std::string, std::string> const &AttributeStubs) {

  std::vector<int> IDs;
  for (auto const &p : Definitions) {
    IDs.push_back(p.first);
  }
  add_attribute(run_info, "NuHepMC." + AttributeStubs.first, IDs);

  for (auto const &p : Definitions) {
    add_attribute(run_info,
                  "NuHepMC." + AttributeStubs.second + "[" +
                      std::to_string(p.first) + "].Name",
                  p.second.first);

    add_attribute(run_info,
                  "NuHepMC." + AttributeStubs.second + "[" +
                      std::to_string(p.first) + "].Description",
                  p.second.second);
  }
}

namespace GR4 {
void WriteProcessIDDefinitions(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                               StatusCodeDescriptors const &Definitions) {
  WriteIDDefinitions(run_info, Definitions, {"ProcessIDs", "ProcessInfo"});
}
} // namespace GR4

namespace GR5 {
void WriteVertexStatusIDDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo> &run_info,
    StatusCodeDescriptors const &Definitions) {
  WriteIDDefinitions(run_info, Definitions,
                     {"VertexStatusIDs", "VertexStatusInfo"});
}
} // namespace GR5

namespace GR6 {
void WriteParticleStatusIDDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo> &run_info,
    StatusCodeDescriptors const &Definitions) {
  WriteIDDefinitions(run_info, Definitions,
                     {"ParticleStatusIDs", "ParticleStatusInfo"});
}
} // namespace GR6

namespace GR7 {
void SetWeightNames(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                    std::vector<std::string> const &names) {
  run_info->set_weight_names(names);
}
} // namespace GR7

namespace GR8 {
template <typename T>
void WriteNonStandardParticleNumbers(
    std::shared_ptr<HepMC3::GenRunInfo> &run_info,
    T const &nonstandard_pdg_definitions) {
  static_assert(std::is_same_v<T, std::map<int, std::string>> ||
                    std::is_same_v<T, std::map<int, std::string>>,
                "WriteNonStandardParticleNumbers called with invalid type, "
                "should be std::map<int,std::string> or "
                "std::map<int,std::pair<std::string,std::string>>");

  if constexpr (std::is_same_v<T, std::map<int, std::string>>) {
    std::vector<int> Nums;
    for (auto const &p : nonstandard_pdg_definitions) {
      Nums.push_back(p.first);
    }
    add_attribute(run_info, "NuHepMC.AdditionalParticleNumbers", Nums);

    for (auto const &p : nonstandard_pdg_definitions) {
      add_attribute(run_info,
                    "NuHepMC.AdditionalParticleNumber[" +
                        std::to_string(p.first) + "].Name",
                    p.second.first);
    }

    if (std::is_same_v<T, std::map<int, std::string>>) {
      for (auto const &p : nonstandard_pdg_definitions) {
        add_attribute(run_info,
                      "NuHepMC.AdditionalParticleNumber[" +
                          std::to_string(p.first) + "].Description",
                      p.second.second);
      }
    }
  }
}
} // namespace GR8

namespace GC1 {
void SetConventions(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                    std::vector<std::string> const &conventions) {
  add_attribute(run_info, "NuHepMC.Conventions", conventions);
}
} // namespace GC1

namespace GC2 {
void SetExposureNEvents(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                        int NEvents) {
  add_attribute(run_info, "NuHepMC.Exposure.NEvents", NEvents);
}
} // namespace GC2

namespace GC3 {
void SetExposurePOT(std::shared_ptr<HepMC3::GenRunInfo> &run_info, double POT) {
  add_attribute(run_info, "NuHepMC.Exposure.POT", POT);
}
void SetExposureLivetime(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                         double Livetime) {
  add_attribute(run_info, "NuHepMC.Exposure.Livetime", Livetime);
}
} // namespace GC3

namespace GC4 {
void SetCrossSectionUnits(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                          std::string const &xs_units,
                          std::string const &target_scale) {
  add_attribute(run_info, "NuHepMC.Units.CrossSection.Unit", xs_units);
  add_attribute(run_info, "NuHepMC.Units.CrossSection.TargetScale",
                target_scale);
}
} // namespace GC4

namespace GC5 {
void SetFluxAveragedTotalXSec(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                              double fatx) {
  add_attribute(run_info, "NuHepMC.FluxAveragedTotalCrossSection", fatx);
}
} // namespace GC5

namespace GC6 {
void AddCitationMetadata(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                         std::string const &component, std::string const &type,
                         std::vector<std::string> const &values) {
  add_attribute(run_info, "NuHepMC.Citations." + component + "." + type,
                values);
}
void AddGeneratorCitation(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                          std::string const &type,
                          std::vector<std::string> const &values) {
  AddCitationMetadata(run_info, "Generator", type, values);
}
void AddProcessCitation(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                        int const &ProcID, std::string const &type,
                        std::vector<std::string> const &values) {
  AddCitationMetadata(run_info,
                      std::string("Process[") + std::to_string(ProcID) + "]",
                      type, values);
}
} // namespace GC6

namespace GC7 {

void WriteBeamEnergyUnits(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                          std::string const &EnergyUnit,
                          std::string const &RateUnit) {
  add_attribute(run_info, "NuHepMC.Beam.EnergyUnit", EnergyUnit);
  if (RateUnit.length()) {
    add_attribute(run_info, "NuHepMC.Beam.RateUnit", RateUnit);
  }
}

void SetHistogramBeamType(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  add_attribute(run_info, "NuHepMC.Beam.Type", "Histogram");
}

void SetMonoEnergeticBeamType(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  add_attribute(run_info, "NuHepMC.Beam.Type", "MonoEnergetic");
}

void WriteBeamEnergyHistogram(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                              int BeamParticleNumber,
                              std::vector<double> const &bin_edges,
                              std::vector<double> const &bin_content,
                              bool ContentIsPerWidth) {

  if (!bin_edges.size() || !((bin_content.size() + 1) == bin_edges.size())) {
    throw InvalidBeamEnergyDescription()
        << "WriteBeamEnergyHistogram pass " << bin_edges.size()
        << " bin edges and " << bin_content.size() << " bin contents.";
  }

  add_attribute(run_info,
                "NuHepMC.Beam[" + std::to_string(BeamParticleNumber) +
                    "].Histogram.BinEdges",
                bin_edges);
  add_attribute(run_info,
                "NuHepMC.Beam[" + std::to_string(BeamParticleNumber) +
                    "].Histogram.BinContent",
                bin_content);

  add_attribute(run_info,
                "NuHepMC.Beam[" + std::to_string(BeamParticleNumber) +
                    "].Histogram.ContentIsPerWidth",
                ContentIsPerWidth);
}
void WriteBeamEnergyMonoenergetic(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                                  int BeamParticleNumber,
                                  double const &energy) {

  add_attribute(run_info,
                "NuHepMC.Beam[" + std::to_string(BeamParticleNumber) +
                    "].MonoEnergetic.Energy",
                energy);
}

void WriteBeamEnergyDistribution(std::shared_ptr<HepMC3::GenRunInfo> &run_info,
                                 int BeamParticleNumber,
                                 EnergyDistribution distribution) {

  switch (distribution.dist_type) {
  case EDistType::kInvalid: {
    throw InvalidBeamEnergyType()
        << "WriteBeamEnergyDistribution with invalid dist_type";
  }
  case EDistType::kMonoEnergetic: {
    SetMonoEnergeticBeamType(run_info);
    WriteBeamEnergyUnits(run_info, distribution.energy_unit,
                         distribution.rate_unit);
    if (std::isnormal(distribution.MonoEnergeticEnergy)) {
      WriteBeamEnergyMonoenergetic(run_info, BeamParticleNumber,
                                   distribution.MonoEnergeticEnergy);
    }
  }
  case EDistType::kHistogram: {
    SetHistogramBeamType(run_info);
    WriteBeamEnergyUnits(run_info, distribution.energy_unit,
                         distribution.rate_unit);
    WriteBeamEnergyHistogram(run_info, BeamParticleNumber,
                             distribution.bin_edges, distribution.bin_content);
  }
  }
}

void WriteBeamEnergyDistributions(
    std::shared_ptr<HepMC3::GenRunInfo> &run_info,
    std::map<int, EnergyDistribution> const &distributions) {
  for (auto const &distribution : distributions) {
    WriteBeamEnergyDistribution(run_info, distribution.first,
                                distribution.second);
  }
}

} // namespace GC7

namespace ER3 {
void SetProcessID(HepMC3::GenEvent &evt, int ProcID) {
  add_attribute(evt, "signal_process_id", ProcID);
}
} // namespace ER3

namespace ER5 {
void SetLabPosition(HepMC3::GenEvent &evt, std::vector<double> const &LabPos) {
  add_attribute(evt, "LabPos", LabPos);
}
} // namespace ER5

namespace EC2 {
void SetTotalCrossSection(HepMC3::GenEvent &evt, double CrossSec) {
  add_attribute(evt, "TotXS", CrossSec);
}
} // namespace EC2

namespace EC3 {
void SetProcessCrossSection(HepMC3::GenEvent &evt, double CrossSec) {
  add_attribute(evt, "ProcXS", CrossSec);
}
} // namespace EC3

} // namespace NuHepMC