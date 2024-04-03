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

  NEW_NuHepMC_EXCEPT(UnconvertibleEnergyUnit);

  void set_units(std::string to_unit) {
    if (to_unit == energy_unit) {
      return;
    }

    double to_fact = 1;

    if (to_unit == "MEV") {
      to_fact /= 1;
    } else if (to_unit == "GEV") {
      to_fact /= 1E3;
    } else {
      throw UnconvertibleEnergyUnit()
          << "Cannot call EnergyDistribution::set_units with " << to_unit;
    }

    double from_fact = 1;
    if (energy_unit == "MEV") {
      from_fact /= 1;
    } else if (energy_unit == "GEV") {
      from_fact /= 1E3;
    } else {
      throw UnconvertibleEnergyUnit()
          << "Cannot call EnergyDistribution::set_units on EnergyDistribution "
             "with energy_unit = "
          << energy_unit;
    }

    double sf = to_fact / from_fact;

    energy_unit = to_unit;

    bin_edges *= sf;
    bin_content *= sf;
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