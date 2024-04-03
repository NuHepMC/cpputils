#pragma once

#include "Eigen/Dense"

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

  Eigen::ArrayXd bin_edges;
  Eigen::ArrayXd bin_content;

  bool ContentIsPerWidth;

  Eigen::ArrayXd GetContentPerWidth(bool shape_only = false) {
    double num_probes = 1;
    if (shape_only) {
      num_probes = 1.0 / GetContentCount(false).sum();
    }

    if (!ContentIsPerWidth) {
      return (bin_content / (bin_edges.bottomRows(bin_edges.rows() - 1) -
                             bin_edges.topRows(bin_edges.rows() - 1))) /
             num_probes;
    }
    return bin_content / num_probes;
  }

  Eigen::ArrayXd GetContentCount(bool shape_only = false) {
    double num_probes = 1;
    if (shape_only) {
      num_probes = 1.0 / GetContentCount(false).sum();
    }

    if (ContentIsPerWidth) {
      return (bin_content * (bin_edges.bottomRows(bin_edges.rows() - 1) -
                             bin_edges.topRows(bin_edges.rows() - 1))) /
             num_probes;
    }
    return bin_content / num_probes;
  }

  Eigen::ArrayXd GetBinCenters() {
    return (bin_edges.bottomRows(bin_edges.rows() - 1) +
            bin_edges.topRows(bin_edges.rows() - 1)) /
           2.0;
  }

  bool IsInGeV() { return energy_unit == "GEV"; }
  bool IsInMeV() { return energy_unit == "MEV"; }
};

} // namespace GC7
} // namespace NuHepMC