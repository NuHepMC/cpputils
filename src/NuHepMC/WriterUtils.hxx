#pragma once

#include "HepMC3/GenRunInfo.h"

#include "NuHepMC/AttributeUtils.hxx"
#include "NuHepMC/Constants.hxx"
#include "NuHepMC/Types.hxx"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace NuHepMC {

namespace GR2 {
void WriteVersion(std::shared_ptr<HepMC3::GenRunInfo> run_info);
} // namespace GR2

namespace GR4 {
void WriteProcessIDDefinitions(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                               StatusCodeDescriptors const &Definitions);
} // namespace GR4

namespace GR5 {
void WriteVertexStatusIDDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo> run_info,
    StatusCodeDescriptors const &Definitions);
} // namespace GR5

namespace GR6 {
void WriteParticleStatusIDDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo> run_info,
    StatusCodeDescriptors const &Definitions);
} // namespace GR6

namespace GR7 {
void SetWeightNames(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                    std::vector<std::string> const &names);
} // namespace GR7

namespace GR8 {
void WriteNonStandardParticleNumbers(
    std::shared_ptr<HepMC3::GenRunInfo> run_info,
    std::map<int, std::string> const &nonstandard_pdg_definitions);
void WriteNonStandardParticleNumbers(
    std::shared_ptr<HepMC3::GenRunInfo> run_info,
    ParticleNumberDescriptors const &nonstandard_pdg_definitions);
} // namespace GR8

namespace GC1 {
void SetConventions(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                    std::vector<std::string> const &conventions);
} // namespace GC1

namespace GC2 {
void SetExposureNEvents(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                        int NEvents);
} // namespace GC2

namespace GC3 {
void SetExposurePOT(std::shared_ptr<HepMC3::GenRunInfo> run_info, double POT);
void SetExposureLivetime(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                         double Livetime);
} // namespace GC3

namespace GC4 {
void SetCrossSectionUnits(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                          std::string const &xs_units,
                          std::string const &target_scale);
} // namespace GC4

namespace GC5 {
void SetFluxAveragedTotalXSec(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                              double fatx);
} // namespace GC5

namespace GC6 {
void AddCitationMetadata(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                         std::string const &component, std::string const &type,
                         std::vector<std::string> const &values);

void AddGeneratorCitation(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                          std::string const &type,
                          std::vector<std::string> const &values);

void AddProcessCitation(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                        int const &ProcID, std::string const &type,
                        std::vector<std::string> const &values);
} // namespace GC6

namespace GC7 {

void WriteBeamUnits(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                    std::string const &EnergyUnit,
                    std::string const &RateUnit = "");

void SetHistogramBeamType(std::shared_ptr<HepMC3::GenRunInfo> run_info);

void SetMonoEnergeticBeamType(std::shared_ptr<HepMC3::GenRunInfo> run_info);

void WriteBeamEnergyHistogram(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                              int BeamParticleNumber,
                              std::vector<double> const &bin_edges,
                              std::vector<double> const &bin_content,
                              bool ContentIsPerWidth = false);

void WriteBeamEnergyMonoenergetic(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                                  int BeamParticleNumber, double const &energy);

void WriteBeamEnergyDistribution(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                                 int BeamParticleNumber,
                                 EnergyDistribution distribution);

void WriteBeamEnergyDistributions(
    std::shared_ptr<HepMC3::GenRunInfo> run_info,
    std::map<int, EnergyDistribution> const &distributions);
} // namespace GC7

namespace ER3 {
void SetProcessID(HepMC3::GenEvent &evt, int ProcID);
} // namespace ER3

namespace ER5 {
void SetLabPosition(HepMC3::GenEvent &evt, std::vector<double> const &LabPos);
} // namespace ER5

namespace EC2 {
void SetTotalCrossSection(HepMC3::GenEvent &evt, double CrossSec);
} // namespace EC2

namespace EC3 {
void SetProcessCrossSection(HepMC3::GenEvent &evt, double CrossSec);
} // namespace EC3

namespace PC2 {
void SetRemnantParticleNumber(HepMC3::GenParticlePtr &ptr, int particle_number);
} // namespace PC2

} // namespace NuHepMC