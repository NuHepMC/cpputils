#include "NuHepMC/ReaderUtils.hxx"
#include "NuHepMC/AttributeUtils.hxx"
#include "NuHepMC/Constants.hxx"

#include "HepMC3/GenParticle.h"

#include <regex>

namespace NuHepMC {

namespace GR2 {
std::tuple<int, int, int>
ReadVersion(std::shared_ptr<HepMC3::GenRunInfo> &run_info) {
  return std::make_tuple(
      CheckedAttributeValue<int>(run_info, "NuHepMC.Version.Major"),
      CheckedAttributeValue<int>(run_info, "NuHepMC.Version.Minor"),
      CheckedAttributeValue<int>(run_info, "NuHepMC.Version.Patch"));
}
} // namespace GR2

StatusCodeDescriptors
ReadIdDefinitions(std::shared_ptr<HepMC3::GenRunInfo> const &run_info,
                  std::pair<std::string, std::string> const &AttributeStubs,
                  bool DescriptionRequired = true) {

  auto IDs = CheckedAttributeValue<std::vector<int>>(
      run_info, "NuHepMC." + AttributeStubs.first);

  StatusCodeDescriptors status_codes;
  for (auto const &id : IDs) {

    std::string name = CheckedAttributeValue<std::string>(
        run_info, "NuHepMC." + AttributeStubs.second + "[" +
                      std::to_string(id) + "].Name");

    // if description is not required pass a default empty string
    std::string description =
        DescriptionRequired
            ? CheckedAttributeValue<std::string>(
                  run_info, "NuHepMC." + AttributeStubs.second + "[" +
                                std::to_string(id) + "].Description")
            : CheckedAttributeValue<std::string>(
                  run_info,
                  "NuHepMC." + AttributeStubs.second + "[" +
                      std::to_string(id) + "].Description",
                  "");

    status_codes[id] = std::pair<std::string, std::string>{name, description};
  }
  return status_codes;
}

namespace GR4 {
StatusCodeDescriptors
ReadProcessIdDefinitions(std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  return ReadIdDefinitions(run_info, {"ProcessIDs", "ProcessInfo"});
}
} // namespace GR4

namespace GR5 {
StatusCodeDescriptors ReadVertexStatusIdDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  return ReadIdDefinitions(run_info, {"VertexStatusIDs", "VertexStatusInfo"});
}
} // namespace GR5

namespace GR6 {
StatusCodeDescriptors ReadParticleStatusIdDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  return ReadIdDefinitions(run_info,
                           {"ParticleStatusIDs", "ParticleStatusInfo"});
}
} // namespace GR6

namespace GR8 {
StatusCodeDescriptors ReadNonStandardParticleNumbers(
    std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  return ReadIdDefinitions(
      run_info, {"AdditionalParticleNumbers", "AdditionalParticleInfo"});
}
} // namespace GR8

namespace GC1 {
std::set<std::string>
ReadConventions(std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  std::set<std::string> conventions;
  for (auto &c : CheckedAttributeValue<std::vector<std::string>>(
           run_info, "NuHepMC.Conventions", std::vector<std::string>{})) {
    conventions.insert(c);
  }
  return conventions;
}
bool SignalsConvention(std::shared_ptr<HepMC3::GenRunInfo> const &run_info,
                       std::string const &Convention) {
  return ReadConventions(run_info).count(Convention);
}
} // namespace GC1

namespace GC2 {
long ReadExposureNEvents(std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  return CheckedAttributeValue<int>(run_info, "NuHepMC.Exposure.NEvents");
}
} // namespace GC2

namespace GC3 {
long ReadExposurePOT(std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  return CheckedAttributeValue<int>(run_info, "NuHepMC.Exposure.POT");
}
long ReadExposureLivetime(std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  return CheckedAttributeValue<int>(run_info, "NuHepMC.Exposure.Livetime");
}
} // namespace GC3

namespace GC4 {
std::pair<std::string, std::string>
ReadCrossSectionUnits(std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  return std::make_pair(
      CheckedAttributeValue<std::string>(
          run_info, "NuHepMC.Units.CrossSection.Unit", "pb"),
      CheckedAttributeValue<std::string>(
          run_info, "NuHepMC.Units.CrossSection.TargetScale", "PerTargetAtom"));
}
} // namespace GC4

namespace GC5 {
long ReadFluxAveragedTotalXSec(
    std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {
  return CheckedAttributeValue<double>(run_info,
                                       "NuHepMC.FluxAveragedTotalCrossSection");
}
} // namespace GC5

namespace GC6 {
CitationData
ReadAllCitations(std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {

  CitationData all_citations;

  static std::regex const key_re(
      R"(NuHepMC.Citations.([:alnum:])\.([:alnum:]))");

  for (auto attn : run_info->attribute_names()) {

    std::smatch matches;

    if (!std::regex_match(attn, matches, key_re)) {
      continue;
    }

    if (!matches.ready() || (matches.size() != 3)) {
      std::cerr << "[ERROR]: When parsing Citation metadata key: " << attn
                << " expected pattern to match "
                   "NuHepMC.Citations.([:alnum:])\\.([:alnum:]) with 2 "
                   "sub_matches, but found: "
                << (matches.size() - 1) << std::endl;
    }

    all_citations[matches[1]][matches[2]] =
        CheckedAttributeValue<std::string>(run_info, attn);
  }

  return all_citations;
}
} // namespace GC6

namespace GC7 {

std::map<int, EnergyDistribution> ReadAllMonoEnergeticDistributions(
    std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {

  std::map<int, EnergyDistribution> mono_energetic_beams;

  static std::regex const key_re(
      R"(NuHepMC.Beam\[[:digit:]\].MonoEnergetic.Energy)");

  for (auto attn : run_info->attribute_names()) {

    std::smatch matches;

    if (!std::regex_match(attn, matches, key_re)) {
      continue;
    }

    int pdg_number = std::stol(matches[1]);

    if (pdg_number == 0) {
      throw InvalidBeamParticleNumber() << "\"" << matches[1] << "\"";
    }

    mono_energetic_beams[pdg_number].MonoEnergeticEnergy =
        CheckedAttributeValue<double>(run_info, matches[0]);
  }

  return mono_energetic_beams;
}

std::map<int, EnergyDistribution> ReadAllHistogramDistributions(
    std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {

  std::map<int, EnergyDistribution> hist_energetic_beams;

  static std::regex const key_re(
      R"(NuHepMC.Beam\[[:digit:]\].Histogram.BinEdges)");

  for (auto attn : run_info->attribute_names()) {

    std::smatch matches;

    if (!std::regex_match(attn, matches, key_re)) {
      continue;
    }

    int pdg_number = std::stol(matches[1]);

    if (pdg_number == 0) {
      throw InvalidBeamParticleNumber() << "\"" << matches[1] << "\"";
    }

    hist_energetic_beams[pdg_number].bin_edges =
        CheckedAttributeValue<std::vector<double>>(run_info, matches[0]);

    hist_energetic_beams[pdg_number].bin_content =
        CheckedAttributeValue<std::vector<double>>(
            run_info, std::string("NuHepMC.Beam[") +
                          std::to_string(pdg_number) +
                          "].Histogram.BinContent");

    hist_energetic_beams[pdg_number].ContentIsPerWidth =
        CheckedAttributeValue<bool>(run_info,
                                    std::string("NuHepMC.Beam[") +
                                        std::to_string(pdg_number) +
                                        "].Histogram.ContentIsPerWidth",
                                    false);
  }

  return hist_energetic_beams;
}

std::map<int, EnergyDistribution> ReadAllEnergyDistributions(
    std::shared_ptr<HepMC3::GenRunInfo> const &run_info) {

  std::map<int, EnergyDistribution> energy_distributions;

  std::string energy_unit =
      CheckedAttributeValue<std::string>(run_info, "NuHepMC.Beam.EnergyUnit");
  std::string rate_unit = CheckedAttributeValue<std::string>(
      run_info, "NuHepMC.Beam.RateUnit", "Arbitrary");

  std::string beam_type_attr =
      CheckedAttributeValue<std::string>(run_info, "NuHepMC.Beam.Type");

  EDistType beam_type = EDistType::kInvalid;

  if (beam_type_attr == "MonoEnergetic") {
    beam_type = EDistType::kMonoEnergetic;
    energy_distributions = ReadAllMonoEnergeticDistributions(run_info);
  } else if (beam_type_attr == "Histogram") {
    beam_type = EDistType::kHistogram;
    energy_distributions = ReadAllHistogramDistributions(run_info);
  } else {
    throw InvalidBeamEnergyType()
        << "NuHepMC.Beam.Type = \"" << beam_type_attr << "\" is invalid.";
  }

  for (auto &ed : energy_distributions) {
    ed.second.dist_type = beam_type;
    ed.second.energy_unit = energy_unit;
    ed.second.rate_unit = rate_unit;
  }

  return energy_distributions;
}
} // namespace GC7

namespace ER2 {
int ReadProcessID(HepMC3::GenEvent &evt) {
  return CheckedAttributeValue<int>(&evt, "signal_process_id");
}
} // namespace ER2

namespace ER4 {
std::vector<double> ReadLabPosition(HepMC3::GenEvent &evt) {
  return CheckedAttributeValue<std::vector<double>>(&evt, "LabPos");
}
} // namespace ER4

namespace EC2 {
double ReadTotalCrossSection(HepMC3::GenEvent &evt) {
  return CheckedAttributeValue<double>(&evt, "TotXS");
}
} // namespace EC2

namespace EC3 {
double ReadProcessCrossSection(HepMC3::GenEvent &evt) {
  return CheckedAttributeValue<double>(&evt, "ProcXS");
}
} // namespace EC3

namespace EC4 {
double ReadFluxAveragedTotalXSecCVBestEstimate(HepMC3::GenEvent &evt) {
  return evt.cross_section()->xsecs()[evt.run_info()->weight_index("CV")];
}
} // namespace EC4

} // namespace NuHepMC