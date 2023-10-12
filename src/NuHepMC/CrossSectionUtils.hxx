#pragma once

#include "NuHepMC/UnitsUtils.hxx"

#include "HepMC3/GenEvent.h"

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

std::map<int, std::map<int, double>>
CalculateFluxAveragedTotalCrossSectionFromSplinesAndFlux(
    std::string const &Filename);

std::map<int, std::map<int, double>> CalculateFluxAveragedTotalCrossSection(
    std::string const &Filename, Units::XSUnits ut = Units::XSUnits::pb,
    Units::XSTargetScale ts = Units::XSTargetScale::PerTargetAtom);

std::tuple<double, std::string, std::string>
GetGC5FluxAveragedTotalCrossSection(std::string const &Filename);

double
GetXSUnitsRescaleFactor(std::pair<Units::XSUnits, Units::XSTargetScale> from,
                        std::pair<Units::XSUnits, Units::XSTargetScale> to,
                        HepMC3::GenEvent &evt);

double GetFATX(std::string const &Filename,
               Units::XSUnits ut = Units::XSUnits::pb,
               Units::XSTargetScale ts = Units::XSTargetScale::PerTargetAtom);

} // namespace CrossSection
} // namespace NuHepMC