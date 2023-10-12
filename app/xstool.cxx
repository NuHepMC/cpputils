#include "NuHepMC/CrossSectionUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include <iostream>

int main(int argc, char const *argv[]) {

  std::string filename = argv[1];

  std::cout << "GetFATX(pb/mnuc): "
            << NuHepMC::CrossSection::GetFATX(
                   argv[1], NuHepMC::CrossSection::Units::XSUnits::pb,
                   NuHepMC::CrossSection::Units::XSTargetScale::
                       PerTargetMolecularNucleon)
            << std::endl;
  std::cout
      << "GetFATX(pb/nuc): "
      << NuHepMC::CrossSection::GetFATX(
             argv[1], NuHepMC::CrossSection::Units::XSUnits::pb,
             NuHepMC::CrossSection::Units::XSTargetScale::PerTargetNucleon)
      << std::endl;
  std::cout << "GetFATX(pb/atom): "
            << NuHepMC::CrossSection::GetFATX(
                   argv[1], NuHepMC::CrossSection::Units::XSUnits::pb,
                   NuHepMC::CrossSection::Units::XSTargetScale::PerTargetAtom)
            << std::endl;
  std::cout << "GetFATX(cm/atom): "
            << NuHepMC::CrossSection::GetFATX(
                   argv[1], NuHepMC::CrossSection::Units::XSUnits::cm2,
                   NuHepMC::CrossSection::Units::XSTargetScale::PerTargetAtom)
            << std::endl;

  auto gc5_fatx =
      NuHepMC::CrossSection::GetGC5FluxAveragedTotalCrossSection(argv[1]);

  std::cout << "G.C.5 FATX: " << std::get<0>(gc5_fatx) << " "
            << std::get<1>(gc5_fatx) << " " << std::get<2>(gc5_fatx)
            << std::endl;

  auto fatx_spline_rebuild = NuHepMC::CrossSection::
      CalculateFluxAveragedTotalCrossSectionFromSplinesAndFlux(argv[1]);

  std::cout << "Recalculated Flux+Spline Method FATX {" << std::endl;

  for (auto beam_fatx : fatx_spline_rebuild) {
    std::cout << "\t" << beam_fatx.first << ": {" << std::endl;
    for (auto tgt_fatx : beam_fatx.second) {
      std::cout << "\t\t" << tgt_fatx.first << ": " << tgt_fatx.second
                << std::endl;
    }
    std::cout << "\t}" << std::endl;
  }

  auto fatx =
      NuHepMC::CrossSection::CalculateFluxAveragedTotalCrossSection(argv[1]);

  std::cout << "Recalculated Stephen Method FATX {" << std::endl;

  for (auto beam_fatx : fatx) {
    std::cout << "\t" << beam_fatx.first << ": {" << std::endl;
    for (auto tgt_fatx : beam_fatx.second) {
      std::cout << "\t\t" << tgt_fatx.first << ": " << tgt_fatx.second
                << std::endl;
    }
    std::cout << "\t}" << std::endl;
  }
}