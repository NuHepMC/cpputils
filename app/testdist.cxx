#include "NuHepMC/DistributionUtils.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include <iostream>

int main(int argc, char const *argv[]) {

  std::string filename = argv[1];
  NuHepMC::Distribution::PlotFinalStateLeptonEnergy(filename);

}
