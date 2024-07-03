#pragma once

#include "NuHepMC/Exceptions.hxx"

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

  Eigen::ArrayXd get_bin_widths() const {
    return (bin_edges.bottomRows(bin_edges.rows() - 1) -
            bin_edges.topRows(bin_edges.rows() - 1));
  }

  double get_integral() const {
    if (ContentIsPerWidth) {
      return (bin_content * get_bin_widths()).sum();
    }
    return bin_content.sum();
  }

  Eigen::ArrayXd get_flux_density() {
    if (!ContentIsPerWidth) {
      return (bin_content / get_bin_widths());
    }
    return bin_content;
  }

  Eigen::ArrayXd get_flux_shape_density() {
    if (!ContentIsPerWidth) {
      return (bin_content / get_bin_widths()) / get_integral();
    }
    return bin_content / get_integral();
  }

  Eigen::ArrayXd get_flux_rate() {

    if (ContentIsPerWidth) {
      return (bin_content * get_bin_widths());
    }
    return bin_content;
  }

  Eigen::ArrayXd get_flux_shape_rate() {

    if (ContentIsPerWidth) {
      return (bin_content * get_bin_widths()) / get_integral();
    }
    return bin_content / get_integral();
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

    if (ContentIsPerWidth) {
      bin_content *= sf;
    }

    bin_edges *= sf;
  }

  Eigen::ArrayXd get_bin_centers() {
    return (bin_edges.bottomRows(bin_edges.rows() - 1) +
            bin_edges.topRows(bin_edges.rows() - 1)) /
           2.0;
  }

  bool is_in_GeV() { return energy_unit == "GEV"; }
  bool is_in_MeV() { return energy_unit == "MEV"; }
};

} // namespace GC7
} // namespace NuHepMC