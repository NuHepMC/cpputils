#pragma once

#include "NuHepMC/AttributeUtils.hxx"
#include "NuHepMC/Constants.hxx"
#include "NuHepMC/Exceptions.hxx"
#include "NuHepMC/Traits.hxx"
#include "NuHepMC/Types.hxx"
#include "NuHepMC/UnitsUtils.hxx"

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenRunInfo.h"
#include "HepMC3/GenVertex.h"

#include "HepMC3/Attribute.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace NuHepMC {

namespace GR2 {
std::tuple<int, int, int>
ReadVersion(std::shared_ptr<HepMC3::GenRunInfo const> run_info);
std::string
ReadVersionString(std::shared_ptr<HepMC3::GenRunInfo const> run_info);
} // namespace GR2

namespace GR4 {
std::set<std::string>
ReadConventions(std::shared_ptr<HepMC3::GenRunInfo const> run_info);
bool SignalsConvention(std::shared_ptr<HepMC3::GenRunInfo const> run_info,
                       std::string const &Convention);
bool SignalsConventions(std::shared_ptr<HepMC3::GenRunInfo const> run_info,
                        std::vector<std::string> Conventions);
} // namespace GR4

namespace GR6 {

std::pair<std::string, std::string>
ReadCrossSectionUnits(std::shared_ptr<HepMC3::GenRunInfo const> run_info);

CrossSection::Units::Scale ParseCrossSectionScaleUnits(std::string const &su);
CrossSection::Units::TargetScale
ParseCrossSectionTargetScaleUnits(std::string const &tsu);
CrossSection::Units::Unit
ParseCrossSectionUnits(std::pair<std::string, std::string> const &csu);

CrossSection::Units::Unit
ParseCrossSectionUnits(std::shared_ptr<HepMC3::GenRunInfo const> run_info);

} // namespace GR6

namespace GR8 {
StatusCodeDescriptors
ReadProcessIdDefinitions(std::shared_ptr<HepMC3::GenRunInfo const> run_info);
} // namespace GR8

namespace GR9 {
StatusCodeDescriptors ReadVertexStatusIdDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo const> run_info);
} // namespace GR9

namespace GR10 {
StatusCodeDescriptors ReadParticleStatusIdDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo const> run_info);
} // namespace GR10

namespace GR11 {
StatusCodeDescriptors ReadNonStandardParticleNumbers(
    std::shared_ptr<HepMC3::GenRunInfo const> run_info);
} // namespace GR11

namespace GC1 {
double ReadExposurePOT(std::shared_ptr<HepMC3::GenRunInfo const> run_info);
double ReadExposureLivetime(std::shared_ptr<HepMC3::GenRunInfo const> run_info);
} // namespace GC1

namespace GC2 {
double
ReadFluxAveragedTotalXSec(std::shared_ptr<HepMC3::GenRunInfo const> run_info);
} // namespace GC2

namespace GC3 {
CitationData
ReadAllCitations(std::shared_ptr<HepMC3::GenRunInfo const> run_info);
} // namespace GC3

namespace GC4 {
NEW_NuHepMC_EXCEPT(InvalidBeamEnergyType);
NEW_NuHepMC_EXCEPT(InvalidBeamParticleNumber);

std::map<int, EnergyDistribution>
ReadAllEnergyDistributions(std::shared_ptr<HepMC3::GenRunInfo const> run_info);
EnergyDistribution
ReadEnergyDistribution(std::shared_ptr<HepMC3::GenRunInfo const> run_info,
                       int pdg_number);
bool HasEnergyDistribution(std::shared_ptr<HepMC3::GenRunInfo const> run_info,
                           int pdg_number = 0);

} // namespace GC4

namespace ER3 {
int ReadProcessID(HepMC3::GenEvent const &evt);
} // namespace ER3

namespace ER5 {
std::vector<double> ReadLabPosition(HepMC3::GenEvent const &evt);
} // namespace ER5

namespace EC2 {
double ReadTotalCrossSection(HepMC3::GenEvent const &evt);
} // namespace EC2

namespace EC3 {
double ReadProcessCrossSection(HepMC3::GenEvent const &evt);
} // namespace EC3

namespace EC4 {
double ReadFluxAveragedTotalXSecCVBestEstimate(HepMC3::GenEvent const &evt);
} // namespace EC4

} // namespace NuHepMC
