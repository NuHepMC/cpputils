#pragma once

#include "NuHepMC/Exceptions.hxx"

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

  std::vector<double> get_bin_widths() const {

    std::vector<double> widths = bin_content;
    for (size_t i = 0; i < widths.size(); ++i) {
      widths[i] = bin_edges[i + 1] - bin_edges[i];
    }

    return widths;
  }

  double get_integral() const {
    auto const &widths = get_bin_widths();
    double integral = 0;
    for (size_t i = 0; i < bin_content.size(); ++i) {
      if (ContentIsPerWidth) {
        integral += bin_content[i] * widths[i];
      } else {
        integral += bin_content[i];
      }
    }
    return integral;
  }

  std::vector<double> get_flux_density() {
    std::vector<double> flux_density = bin_content;
    auto const &widths = get_bin_widths();
    if (!ContentIsPerWidth) {
      for (size_t i = 0; i < bin_content.size(); ++i) {
        flux_density[i] /= widths[i];
      }
    }
    return flux_density;
  }

  std::vector<double> get_flux_shape_density() {
    auto flux_density = get_flux_density();
    auto const &integral = get_integral();
    for (size_t i = 0; i < flux_density.size(); ++i) {
      flux_density[i] /= integral;
    }
    return flux_density;
  }

  std::vector<double> get_flux_rate() {
    std::vector<double> flux_rate = bin_content;
    auto const &widths = get_bin_widths();
    if (ContentIsPerWidth) {
      for (size_t i = 0; i < bin_content.size(); ++i) {
        flux_rate[i] *= widths[i];
      }
    }
    return flux_rate;
  }

  std::vector<double> get_flux_shape_rate() {
    auto flux_rate = get_flux_rate();
    auto const &integral = get_integral();
    for (size_t i = 0; i < flux_rate.size(); ++i) {
      flux_rate[i] /= integral;
    }
    return flux_rate;
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
      for (size_t i = 0; i < bin_content.size(); ++i) {
        bin_content[i] *= sf;
      }
    }
    for (size_t i = 0; i < bin_edges.size(); ++i) {
      bin_edges[i] *= sf;
    }
  }

  std::vector<double> get_bin_centers() {
    std::vector<double> centers = bin_content;

    for (size_t i = 0; i < centers.size(); ++i) {
      centers[i] = (bin_edges[i + 1] + bin_edges[i]) / 2.0;
    }

    return centers;
  }

  bool is_in_GeV() { return energy_unit == "GEV"; }
  bool is_in_MeV() { return energy_unit == "MEV"; }
};

} // namespace GC7
} // namespace NuHepMC