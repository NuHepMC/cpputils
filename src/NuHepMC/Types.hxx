#pragma once

#include <map>
#include <string>

namespace NuHepMC {
using StatusCodeDescriptors =
    std::map<int, std::pair<std::string, std::string>>;
using ParticleNumberDescriptors = StatusCodeDescriptors;

using CitationData = std::map<std::string, std::map<std::string, std::string>>;

namespace GC7 {

enum class EDistType {
  kInvalid = 0,
  kMonoEnergetic = 1,
  kHistogram = 2,
};

struct EnergyDistribution {

  EDistType dist_type;

  double MonoEnergeticEnergy;

  std::string energy_unit;
  std::string rate_unit;

  std::vector<double> bin_edges;
  std::vector<double> bin_content;

  bool ContentIsPerWidth;

  std::vector<double> GetContentPerWidth() {
    if (!ContentIsPerWidth) {
      std::vector<double> contentperwidth;
      for (size_t bin_it = 0; bin_it < bin_content.size(); ++bin_it) {
        contentperwidth.push_back(bin_content[bin_it] /
                                  (bin_edges[bin_it + 1] - bin_edges[bin_it]));
      }
      return contentperwidth;
    }
    return bin_content;
  }

  std::vector<double> GetContentCount() {
    if (ContentIsPerWidth) {
      std::vector<double> contentcount;
      for (size_t bin_it = 0; bin_it < bin_content.size(); ++bin_it) {
        contentcount.push_back(bin_content[bin_it] *
                               (bin_edges[bin_it + 1] - bin_edges[bin_it]));
      }
      return contentcount;
    }
    return bin_content;
  }
};

} // namespace GC7
} // namespace NuHepMC