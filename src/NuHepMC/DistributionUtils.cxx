#include "NuHepMC/DistributionUtils.hxx"

#include "NuHepMC/EventUtils.hxx"
#include "NuHepMC/Exceptions.hxx"
#include "NuHepMC/ReaderUtils.hxx"

#include "HepMC3/ReaderFactory.h"

#include "fmt/format.h"

namespace NuHepMC {

namespace Distribution {

NEW_NuHepMC_EXCEPT(CouldNotOpenInputFile);

/// Template class to act as a helper for generating ranges in linear space. This is done in
/// a way to mimic the linspace function found within the numpy python library.
template <typename T> class LinspaceGen {
  private:
    T curValue, step;

  public:
    /// Construct a helper object for generating equally spaced points in linear space
    ///@param first: The value to start the range at
    ///@param step: The value to add at each step
    LinspaceGen(T first, T step_) : curValue(first), step(step_) {}

    /// Get the next value in the linear space chain
    ///@return T: The next value in the chain
    T operator()() {
        T retval = curValue;
        curValue += step;
        return retval;
    }
};

/// Function to generate a range of numbers equally spaced in linear space from start to
/// stop. This function mimics the linspace function found within the numpy python library.
///@param start: The starting value to generate from
///@param stop: The ending value to generate to
///@param num: The number of points to generate within the range
///@return std::vector<double>: A vector containing equally spaced points in linear space
inline std::vector<double> Linspace(double start, double stop, std::size_t num = 50) {
    double step = (stop - start) / (static_cast<double>(num) - 1);

    std::vector<double> retval;
    retval.reserve(num);
    std::generate_n(std::back_inserter(retval), num, LinspaceGen<double>(start, step));
    return retval;
}

Histogram::Histogram(std::string const &name, size_t nb, double min, double max) 
  : hist_name(name), nbins(nb), bin_edges(Linspace(min, max, nb+1)), heights(nb), errors(nb) {}

Histogram::Histogram(std::string const &name, std::vector<double> const &edges)
  : hist_name(name), nbins(edges.size()), bin_edges(edges), heights(edges.size()-1), errors(edges.size()-1) {}

void Histogram::Fill(double x, double wgt) {
  // Underflow and Overflow check
  if(x < bin_edges.front() || x > bin_edges.back()) return;

  // Find bin
  auto it = std::lower_bound(bin_edges.begin(), bin_edges.end(), x);
  size_t bin = static_cast<size_t>(std::distance(bin_edges.begin(), it));

  heights[bin - 1] += wgt / (bin_edges[bin] - bin_edges[bin - 1]);
  errors[bin - 1] += pow(wgt / (bin_edges[bin] - bin_edges[bin - 1]), 2);
}

void Histogram::Scale(double factor) {
  for(auto &height : heights) height *= factor;
  for(auto &error : errors) error *= factor*factor;
}

void Histogram::Save(std::ostream *out) const {
    *out << hist_name << std::endl;
    *out << fmt::format("{:>15},{:>15},{:>15},{:>15}\n", "lower edge", "upper edge", "value",
                        "error");
    for(size_t i = 0; i < heights.size(); ++i) {
        *out << fmt::format("{:> 15.6e},{:> 15.6e},{:> 15.6e},{:> 15.6e}\n", bin_edges[i],
                            bin_edges[i + 1], heights[i], std::sqrt(errors[i]));
    }
}

void Histogram::Save(std::string const &filename) const {
    std::ofstream out(filename + ".txt");
    Save(&out);
}

void PlotFinalStateLeptonEnergy(std::string const &Filename) {
  auto rdr = HepMC3::deduce_reader(Filename);
  if (!rdr) {
    throw CouldNotOpenInputFile()
        << "Failed to instantiate HepMC3::Reader from " << Filename;
  }

  double to_MeV = 1;
  HepMC3::GenEvent evt;
  Histogram hist("E_mu", 100, 0, 10000);
  double weight_sum = 0;
  double xsec = 0;

  size_t NEvents = 0;
  while (!rdr->failed()) {
    rdr->read_event(evt);

    if (!NEvents) { // first event
      to_MeV = Event::ToMeVFactor(evt);
    }

    if (!rdr->failed()) {
      NEvents++;
    } else {
      break;
    }

    auto tmp = evt.cross_section();
    xsec = tmp -> xsec();
    double wgt = evt.weights()[0];
    auto muon = NuHepMC::Event::GetParticles_All(evt, 1, {13});
    hist.Fill(muon[0]->momentum().e(), wgt);
    weight_sum += wgt;
  }

  hist.Scale(xsec/weight_sum);
  hist.Save(&std::cout);
}

}

}
