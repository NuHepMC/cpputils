#pragma once

#include "NuHepMC/UnitsUtils.hxx"

#include <map>
#include <string>
#include <vector>

namespace NuHepMC {

namespace CrossSection {

struct Spline {
  std::vector<std::pair<double, double>> points;
  Units::XSUnits units;
  Units::XSTargetScale targ_scale;
};

struct SplineBuilder {

  std::vector<double> EnergyBinning_MeV;
  std::vector<std::vector<double>> XSSamples;

  Units::XSUnits units;
  Units::XSTargetScale targ_scale;

  SplineBuilder(std::vector<double> const &binning_MeV,
                Units::XSUnits ut = Units::XSUnits::pb,
                Units::XSTargetScale ts = Units::XSTargetScale::PerTargetAtom);

  void AddSample(double Enu_MeV, double XS);

  Spline ToSpline() const;
};

// splines[nupid][targetpid][procid]
using NuTgtProcSplineMap = std::map<int, std::map<int, std::map<int, Spline>>>;

NuTgtProcSplineMap BuildSplines(std::string const &Filename,
                                std::vector<double> EnergyBinning_MeV = {});

double CalculateFluxAveragedTotalCrossSection(
    std::string const &Filename, Units::XSUnits ut = Units::XSUnits::pb,
    Units::XSTargetScale ts = Units::XSTargetScale::PerTargetAtom);
} // namespace CrossSection
} // namespace NuHepMC