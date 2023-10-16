#pragma once

#include "NuHepMC/UnitsUtils.hxx"

#include "HepMC3/GenEvent.h"

#include <string>
#include <vector>

namespace NuHepMC {

namespace Distribution {

class Histogram {
  public:
    Histogram(std::string const &name, size_t nb, double min, double max);
    Histogram(std::string const &name, std::vector<double> const &edges);
    void Fill(double x, double wgt=1);
    void Scale(double factor);
    void Save(std::ostream *out) const;
    void Save(std::string const &filename) const;

  private:
    std::string hist_name;
    size_t nbins;
    std::vector<double> bin_edges, heights, errors;
};

void PlotFinalStateLeptonEnergy(std::string const &filename);

}

}
