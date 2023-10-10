#pragma once

#include "HepMC3/GenEvent.h"
#include "HepMC3/GenRunInfo.h"
#include "HepMC3/GenVertex.h"

#include "HepMC3/Attribute.h"

#include "NuHepMC/Exceptions.hxx"
#include "NuHepMC/Traits.hxx"
#include "NuHepMC/Types.hxx"
#include "NuHepMC/AttributeUtils.hxx"
#include "NuHepMC/Constants.hxx"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace NuHepMC {

namespace GR2 {
std::tuple<int, int, int>
ReadVersion(std::shared_ptr<HepMC3::GenRunInfo> &run_info);
} // namespace GR2

namespace GR4 {
StatusCodeDescriptors
ReadProcessIdDefinitions(std::shared_ptr<HepMC3::GenRunInfo> const &run_info);
} // namespace GR4

namespace GR5 {
StatusCodeDescriptors ReadVertexStatusIdDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo> const &run_info);
} // namespace GR5

namespace GR6 {
StatusCodeDescriptors ReadParticleStatusIdDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo> const &run_info);
} // namespace GR6

namespace GR8 {
StatusCodeDescriptors ReadNonStandardParticleNumbers(
    std::shared_ptr<HepMC3::GenRunInfo> const &run_info);
} // namespace GR8

namespace GC1 {
std::set<std::string>
ReadConventions(std::shared_ptr<HepMC3::GenRunInfo> const &run_info);
bool SignalsConvention(std::shared_ptr<HepMC3::GenRunInfo> const &run_info,
                       std::string const &Convention);
} // namespace GC1

namespace GC2 {
long ReadExposureNEvents(std::shared_ptr<HepMC3::GenRunInfo> const &run_info);
} // namespace GC2

namespace GC3 {
double ReadExposurePOT(std::shared_ptr<HepMC3::GenRunInfo> const &run_info);
double ReadExposureLivetime(std::shared_ptr<HepMC3::GenRunInfo> const &run_info);
} // namespace GC3

namespace GC4 {
std::pair<std::string, std::string>
ReadCrossSectionUnits(std::shared_ptr<HepMC3::GenRunInfo> const &run_info);
} // namespace GC4

namespace GC5 {
double ReadFluxAveragedTotalXSec(
    std::shared_ptr<HepMC3::GenRunInfo> const &run_info);
} // namespace GC5

namespace GC6 {
CitationData
ReadAllCitations(std::shared_ptr<HepMC3::GenRunInfo> const &run_info);
} // namespace GC6

namespace GC7 {
NEW_NuHepMC_EXCEPT(InvalidBeamEnergyType);
NEW_NuHepMC_EXCEPT(InvalidBeamParticleNumber);

std::map<int, EnergyDistribution>
ReadAllEnergyDistributions(std::shared_ptr<HepMC3::GenRunInfo> const &run_info);
} // namespace GC7

namespace ER3 {
int ReadProcessID(HepMC3::GenEvent &evt);
} // namespace ER3

namespace ER5 {
std::vector<double> ReadLabPosition(HepMC3::GenEvent &evt);
} // namespace ER5

namespace EC2 {
double ReadTotalCrossSection(HepMC3::GenEvent &evt);
} // namespace EC2

namespace EC3 {
double ReadProcessCrossSection(HepMC3::GenEvent &evt);
} // namespace EC3

namespace EC4 {
double ReadFluxAveragedTotalXSecCVBestEstimate(HepMC3::GenEvent &evt);
} // namespace EC4

} // namespace NuHepMC