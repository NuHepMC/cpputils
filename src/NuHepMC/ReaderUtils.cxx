#include "NuHepMC/ReaderUtils.hxx"

#include "HepMC3/GenParticle.h"

#include <regex>

namespace NuHepMC {

namespace GR2 {
std::tuple<int, int, int>
ReadVersion(std::shared_ptr<HepMC3::GenRunInfo const> run_info) {
  return std::make_tuple(
      CheckedAttributeValue<int>(run_info, "NuHepMC.Version.Major"),
      CheckedAttributeValue<int>(run_info, "NuHepMC.Version.Minor"),
      CheckedAttributeValue<int>(run_info, "NuHepMC.Version.Patch"));
}
std::string
ReadVersionString(std::shared_ptr<HepMC3::GenRunInfo const> run_info) {
  auto vt = ReadVersion(run_info);
  return std::to_string(std::get<0>(vt)) + "." +
         std::to_string(std::get<1>(vt)) + "." +
         std::to_string(std::get<2>(vt));
}
} // namespace GR2

StatusCodeDescriptors
ReadIdDefinitions(std::shared_ptr<HepMC3::GenRunInfo const> run_info,
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
ReadProcessIdDefinitions(std::shared_ptr<HepMC3::GenRunInfo const> run_info) {
  return ReadIdDefinitions(run_info, {"ProcessIDs", "ProcessInfo"});
}
} // namespace GR4

namespace GR5 {
StatusCodeDescriptors ReadVertexStatusIdDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo const> run_info) {
  return ReadIdDefinitions(run_info, {"VertexStatusIDs", "VertexStatusInfo"});
}
} // namespace GR5

namespace GR6 {
StatusCodeDescriptors ReadParticleStatusIdDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo const> run_info) {
  return ReadIdDefinitions(run_info,
                           {"ParticleStatusIDs", "ParticleStatusInfo"});
}
} // namespace GR6

namespace GR8 {
StatusCodeDescriptors ReadNonStandardParticleNumbers(
    std::shared_ptr<HepMC3::GenRunInfo const> run_info) {
  return ReadIdDefinitions(
      run_info, {"AdditionalParticleNumbers", "AdditionalParticleInfo"});
}
} // namespace GR8

namespace GC1 {
std::set<std::string>
ReadConventions(std::shared_ptr<HepMC3::GenRunInfo const> run_info) {
  std::set<std::string> conventions;
  for (auto &c : CheckedAttributeValue<std::vector<std::string>>(
           run_info, "NuHepMC.Conventions", std::vector<std::string>{})) {
    conventions.insert(c);
  }
  return conventions;
}
bool SignalsConvention(std::shared_ptr<HepMC3::GenRunInfo const> run_info,
                       std::string const &Convention) {
  return ReadConventions(run_info).count(Convention);
}
bool SignalsConventions(std::shared_ptr<HepMC3::GenRunInfo const> run_info,
                        std::vector<std::string> Conventions) {
  bool all = true;
  auto convs = ReadConventions(run_info);
  for (auto const &c : Conventions) {
    all = all && convs.count(c);
  }
  return all;
}
} // namespace GC1

namespace GC2 {
long ReadExposureNEvents(std::shared_ptr<HepMC3::GenRunInfo const> run_info) {
  return CheckedAttributeValue<long>(run_info, "NuHepMC.Exposure.NEvents");
}
} // namespace GC2

namespace GC3 {
double ReadExposurePOT(std::shared_ptr<HepMC3::GenRunInfo const> run_info) {
  return CheckedAttributeValue<double>(run_info, "NuHepMC.Exposure.POT");
}
double
ReadExposureLivetime(std::shared_ptr<HepMC3::GenRunInfo const> run_info) {
  return CheckedAttributeValue<double>(run_info, "NuHepMC.Exposure.Livetime");
}
} // namespace GC3

namespace GC4 {
std::pair<std::string, std::string>
ReadCrossSectionUnits(std::shared_ptr<HepMC3::GenRunInfo const> run_info) {
  return std::make_pair(
      CheckedAttributeValue<std::string>(
          run_info, "NuHepMC.Units.CrossSection.Unit", "pb"),
      CheckedAttributeValue<std::string>(
          run_info, "NuHepMC.Units.CrossSection.TargetScale", "PerTargetAtom"));
}

CrossSection::Units::Unit
ParseCrossSectionUnits(std::pair<std::string, std::string> const &csu) {

  CrossSection::Units::Scale xs = CrossSection::Units::Scale::CustomType;

  if (csu.first == "pb") {
    xs = CrossSection::Units::Scale::pb;
  } else if (csu.first == "cm2") {
    xs = CrossSection::Units::Scale::cm2;
  } else if (csu.first == "1e-38 cm2") {
    xs = CrossSection::Units::Scale::cm2_ten38;
  }

  CrossSection::Units::TargetScale ts =
      CrossSection::Units::TargetScale::CustomType;

  if (csu.second == "PerTargetMolecule") {
    ts = CrossSection::Units::TargetScale::PerTargetMolecule;
  } else if (csu.second == "PerTargetAtom") {
    ts = CrossSection::Units::TargetScale::PerTargetAtom;
  } else if (csu.second == "PerTargetNucleon") {
    ts = CrossSection::Units::TargetScale::PerTargetNucleon;
  } else if (csu.second == "PerTargetMolecularNucleon") {
    ts = CrossSection::Units::TargetScale::PerTargetMolecularNucleon;
  }

  return {xs, ts};
}

CrossSection::Units::Unit
ParseCrossSectionUnits(std::shared_ptr<HepMC3::GenRunInfo const> run_info) {
  return ParseCrossSectionUnits(ReadCrossSectionUnits(run_info));
}
} // namespace GC4

namespace GC5 {
double
ReadFluxAveragedTotalXSec(std::shared_ptr<HepMC3::GenRunInfo const> run_info) {
  return CheckedAttributeValue<double>(run_info,
                                       "NuHepMC.FluxAveragedTotalCrossSection");
}
} // namespace GC5

namespace GC6 {
CitationData
ReadAllCitations(std::shared_ptr<HepMC3::GenRunInfo const> run_info) {

  CitationData all_citations;

  static std::regex const key_re(R"(NuHepMC\.Citations\.([^\.]+)\.([^\.\s]+))");

  for (auto attn : run_info->attribute_names()) {

    std::smatch matches;

    if (!std::regex_match(attn, matches, key_re)) {
      continue;
    }

    if (!matches.ready() || (matches.size() != 3)) {
      std::cerr << "[ERROR]: When parsing Citation metadata key: " << attn
                << " expected pattern to match "
                   "NuHepMC\\.Citations\\.([^\\.]+)\\.([^\\.\\s]+) with 2 "
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

EnergyDistribution ReadMonoEnergeticDistribution(
    std::shared_ptr<HepMC3::GenRunInfo const> run_info, int pdg_number) {
  EnergyDistribution dist;
  dist.MonoEnergeticEnergy = CheckedAttributeValue<double>(
      run_info, std::string("NuHepMC.Beam[") + std::to_string(pdg_number) +
                    "].MonoEnergetic.Energy");
  return dist;
}

std::vector<int> FindAllMonoEnergeticDistributionsPDGs(
    std::shared_ptr<HepMC3::GenRunInfo const> run_info) {

  std::vector<int> pdg_numbers;

  static std::regex const key_re(
      R"(NuHepMC\.Beam\[([0-9]+)\]\.MonoEnergetic\.Energy)");

  for (auto attn : run_info->attribute_names()) {

    std::smatch matches;

    if (!std::regex_match(attn, matches, key_re)) {
      continue;
    }

    pdg_numbers.push_back(std::stol(matches[1]));

    if (pdg_numbers.back() == 0) {
      throw InvalidBeamParticleNumber() << "\"" << matches[1] << "\"";
    }
  }

  return pdg_numbers;
}

EnergyDistribution
ReadHistogramDistribution(std::shared_ptr<HepMC3::GenRunInfo const> run_info,
                          int pdg_number) {

  EnergyDistribution dist;

  dist.bin_edges = CheckedAttributeValue<Eigen::ArrayXd>(
      run_info, std::string("NuHepMC.Beam[") + std::to_string(pdg_number) +
                    "].Histogram.BinEdges");

  dist.bin_content = CheckedAttributeValue<Eigen::ArrayXd>(
      run_info, std::string("NuHepMC.Beam[") + std::to_string(pdg_number) +
                    "].Histogram.BinContent");

  dist.ContentIsPerWidth = CheckedAttributeValue<bool>(
      run_info,
      std::string("NuHepMC.Beam[") + std::to_string(pdg_number) +
          "].Histogram.ContentIsPerWidth",
      false);

  return dist;
}

std::vector<int> FindAllHistogramDistributionsPDGs(
    std::shared_ptr<HepMC3::GenRunInfo const> run_info) {

  std::vector<int> pdg_numbers;

  static std::regex const key_re(
      R"(NuHepMC\.Beam\[([0-9]+)\]\.Histogram\.BinEdges)");

  for (auto attn : run_info->attribute_names()) {

    std::smatch matches;

    if (!std::regex_match(attn, matches, key_re)) {
      continue;
    }

    pdg_numbers.push_back(std::stol(matches[1]));

    if (pdg_numbers.back() == 0) {
      throw InvalidBeamParticleNumber() << "\"" << matches[1] << "\"";
    }
  }

  return pdg_numbers;
}

EnergyDistribution BuildEnergyDistributionTemplate(
    std::shared_ptr<HepMC3::GenRunInfo const> run_info) {
  EnergyDistribution dist;

  dist.energy_unit =
      CheckedAttributeValue<std::string>(run_info, "NuHepMC.Beam.EnergyUnit");
  dist.rate_unit = CheckedAttributeValue<std::string>(
      run_info, "NuHepMC.Beam.RateUnit", "Arbitrary");

  std::string dist_type_attr =
      CheckedAttributeValue<std::string>(run_info, "NuHepMC.Beam.Type");

  if (dist_type_attr == "MonoEnergetic") {
    dist.dist_type = EDistType::kMonoEnergetic;
  } else if (dist_type_attr == "Histogram") {
    dist.dist_type = EDistType::kHistogram;
  } else {
    throw InvalidBeamEnergyType()
        << "NuHepMC.Beam.Type = \"" << dist_type_attr << "\" is invalid.";
  }

  return dist;
}

std::map<int, EnergyDistribution>
ReadAllEnergyDistributions(std::shared_ptr<HepMC3::GenRunInfo const> run_info) {

  std::map<int, EnergyDistribution> energy_distributions;

  EnergyDistribution template_dist = BuildEnergyDistributionTemplate(run_info);

  switch (template_dist.dist_type) {
  case EDistType::kMonoEnergetic: {
    for (auto pdg : FindAllMonoEnergeticDistributionsPDGs(run_info)) {
      energy_distributions[pdg] = ReadMonoEnergeticDistribution(run_info, pdg);
      energy_distributions[pdg].dist_type = template_dist.dist_type;
      energy_distributions[pdg].energy_unit = template_dist.energy_unit;
      energy_distributions[pdg].rate_unit = template_dist.rate_unit;
    }
    break;
  }
  case EDistType::kHistogram: {
    for (auto pdg : FindAllHistogramDistributionsPDGs(run_info)) {
      energy_distributions[pdg] = ReadHistogramDistribution(run_info, pdg);
      energy_distributions[pdg].dist_type = template_dist.dist_type;
      energy_distributions[pdg].energy_unit = template_dist.energy_unit;
      energy_distributions[pdg].rate_unit = template_dist.rate_unit;
    }
    break;
  }
  case EDistType::kInvalid:
  default: {
    throw InvalidBeamEnergyType();
  }
  }

  return energy_distributions;
}

EnergyDistribution
ReadEnergyDistribution(std::shared_ptr<HepMC3::GenRunInfo const> run_info,
                       int pdg_number) {

  EnergyDistribution template_dist = BuildEnergyDistributionTemplate(run_info);
  EnergyDistribution dist = ReadHistogramDistribution(run_info, pdg_number);
  dist.dist_type = template_dist.dist_type;
  dist.energy_unit = template_dist.energy_unit;
  dist.rate_unit = template_dist.rate_unit;

  return dist;
}

bool HasEnergyDistribution(std::shared_ptr<HepMC3::GenRunInfo const> run_info,
                           int pdg_number) {
  if (!HasAttributeOfType<std::string>(run_info, "NuHepMC.Beam.EnergyUnit") ||
      !HasAttributeOfType<std::string>(run_info, "NuHepMC.Beam.Type")) {
    return false;
  }

  auto monoed = FindAllMonoEnergeticDistributionsPDGs(run_info);
  auto histed = FindAllHistogramDistributionsPDGs(run_info);

  if (pdg_number == 0) {
    return monoed.size() + histed.size();
  }

  if ((std::find(monoed.begin(), monoed.end(), pdg_number) == monoed.end()) &&
      (std::find(histed.begin(), histed.end(), pdg_number) == histed.end())) {
    return false;
  }

  return true;
}

} // namespace GC7

namespace ER3 {
int ReadProcessID(HepMC3::GenEvent const &evt) {

  // for compat
  if (HasAttribute(&evt, "ProcID")) {
    return CheckedAttributeValue<int>(&evt, "ProcID");
  }
  return CheckedAttributeValue<int>(&evt, "signal_process_id");
}
} // namespace ER3

namespace ER5 {
std::vector<double> ReadLabPosition(HepMC3::GenEvent const &evt) {
  return CheckedAttributeValue<std::vector<double>>(&evt, "LabPos");
}
} // namespace ER5

namespace EC2 {
double ReadTotalCrossSection(HepMC3::GenEvent const &evt) {
  return CheckedAttributeValue<double>(&evt, "TotXS");
}
} // namespace EC2

namespace EC3 {
double ReadProcessCrossSection(HepMC3::GenEvent const &evt) {
  return CheckedAttributeValue<double>(&evt, "ProcXS");
}
} // namespace EC3

namespace EC4 {
double ReadFluxAveragedTotalXSecCVBestEstimate(HepMC3::GenEvent const &evt) {
  return evt.cross_section()->xsecs()[evt.run_info()->weight_index("CV")];
}
} // namespace EC4

} // namespace NuHepMC
