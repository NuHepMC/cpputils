# NuHepMC::CPPUtils

C++ Helper functions for working with 
[NuHepMC](https://arxiv.org/pdf/2310.13211.pdf) standard 
[HepMC3](https://gitlab.cern.ch/hepmc/HepMC3) objects.

## Table of Contents

* [Quick Start](#quick-start)
* [Worked Analysis Example](#worked-analysis-example)
* [API Reference](#api-reference)

## Quick Start

### Using this package in a CMake Project

This is the recommended way to use NuHepMC::CPPUtils.

Firstly, use [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) in your 
project.

Then include the following your CMakeLists.txt
```cmake
CPMAddPackage(
  NAME NuHepMC_CPPUtils
  GIT_TAG main
  GIT_REPOSITORY "https://github.com/NuHepMC/cpputils.git"
  OPTIONS "BUILTIN_HEPMC3 ON"
)
```

If you already have a compatible copy of `HepMC3` that is visible to CMake, 
then you can omit the `OPTIONS` argument.

Then link your targets like so:
```cmake
target_link_libraries(<mytarget> PUBLIC NuHepMC::CPPUtils)
```

this will add include directories and linker flags for this library and its 
dependencies to your target.

### Building Standalone

```bash
cd /path/to/cpputils
mkdir build; cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/where/you/want/to/install \
         -DBUILTIN_HEPMC3=ON
make install
```

#### Setting up the environment

```bash
eval $(/path/to/cpputils/build/$(uname -s)/bin/NuHepMC-config --env)
```

This will also set up the HepMC3 autogenerated python bindings for you.

## Worked Analysis Example

This section walks through the details of 
[example/ana_skeleton.cxx](example/ana_skeleton.cxx).

### Analysis Quick Start

Make a new skeleton analysis implementation, `myana.cxx` by running 
`NuHepMC-config --skele myana.cxx` with the 
[environment](#setting-up-the-environment) set up. Modify it to your hearts 
content. Compile it with `NuHepMC-config --build myana.cxx <additional flags>`.
The produced binary can then run it like: `myana input.hepmc`.

There is nothing particularly clever going on here, `NuHepMC-config --build` 
just wraps a call to the compiler with the relevant NuHepMC::CPPUtils and HepMC3
compiler and linker flags. It hopefully provides a quick way of prototyping C++ 
NuHepMC analyses.

### Opening a file

We use the standard HepMC3::deduce_reader to open input files from file names.

```c++
#include "HepMC3/ReaderFactory.h"

#include <iostream>

int main(int argc, char const *argv[]){
  auto rdr = HepMC3::deduce_reader(argv[1]);
  if (!rdr) {
    std::cout << "Failed to instantiate HepMC3::Reader from " << inf
              << std::endl;
    return 1;
  }
}
```

Its good practice to check that you can read the first event from the input 
file. This also triggers HepMC3 to read the GenRunInfo, which is useful for
determining some input details that you might need before processing the event
stream.

```c++
#include "HepMC3/ReaderFactory.h"

#include <iostream>

int main(int argc, char const *argv[]){
  auto rdr = HepMC3::deduce_reader(argv[1]);
  if (!rdr) {
    std::cout << "Failed to instantiate HepMC3::Reader from " << inf
              << std::endl;
    return 1;
  }

  HepMC3::GenEvent evt;
  rdr->read_event(evt);
  if (rdr->failed()) {
    std::cout << "Failed to read the first event from " << inf << std::endl;
    return 1;
  }

  auto run_info = rdr->run_info();
  if(!run_info){
    std::cout << "Warning, after reading the first event, we have no "
    "GenRunInfo." << std::endl;
    // decide if this is a failure for your analysis.
    // return 1;
  }
```

### Important GenRunInfo Metadata

It is worth grabbing a few important bits of metadata from the run_info before
processing the event stream. Some common examples are the meanings of various
status codes (interaction process ids and vertex and particle status codes).

```c++
  // read the various status code definitions
  auto proc_ids = GR4::ReadProcessIdDefinitions(run_info);
  auto vtxstatus = GR5::ReadVertexStatusIdDefinitions(run_info);
  auto partstatus = GR6::ReadParticleStatusIdDefinitions(run_info);
```

Each of these is a map of a process or status identifier to a pair of strings,
the first is a short name for the identifier and the latter is space for a
longer form description of the identifer.

HepMC events signal the metric prefix used for their energy and momentum units,
it is useful to grab this once at the start and use it as a global scale factor
to your analyses desired units. This information is stored on the event and not
on the run_info, so you can use the first event that you read to check the
reader to grab it and store it.

```c++
  double ToGeV = Event::ToMeVFactor(evt) * 1E-3;
```

### Event Processing Loops

Once we have a HepMC3::Reader instance and have checked that it can read events,
we can loop over events and process each one like so:

```c++
  // re-open the file to start from the first event
  rdr = HepMC3::deduce_reader(inf);
  while (true) {

    // read an event and check that you haven't finished the file.
    // A file only knows it has read the last event after trying to read the 
    // next one 
    rdr->read_event(evt);
    if (rdr->failed()) {
      break;
    }

    // .. do something with evt
    ProcessEvent(evt);
  }
```

### Scaling To A Cross Section

For many applications we will need to calculate the flux-averaged total cross
section (FATX). See [NuHepMC](https://arxiv.org/pdf/2310.13211.pdf) section 3 
for more details. NuHepMC::CPPUtils provides a accumulator-like helper class for 
calculating the best-estimate for an event stream. There are multiple ways to
estimate the FATX from the metadata specified by NuHepMC, we provide a helper
factory function to instantiate the best accumulator subclass given the current
event stream. Its usage is demoed below:


```c++
  auto FATXAcc = FATX::MakeAccumulator(run_info);

  // re-open the file to start from the first event
  rdr = HepMC3::deduce_reader(inf);
  while (true) {

    // read an event and check that you haven't finished the file.
    // A file only knows it has read the last event after trying to read the 
    // next one 
    rdr->read_event(evt);
    if (rdr->failed()) {
      break;
    }

    // G.R.7 CV weight
    double evw = FATXAcc->process(evt);

    // .. do something with evt
    ProcessEvent(evt);
  }

  double fatx = FATXAcc->fatx(); // in pb/Atom
  double sumw = FATXAcc->sumweights();
  size_t nevents = FATXAcc->events();
```

When tracking event properties, the CV event weight must always be taken into
account according to G.R.7. For many simulations, this weight will be 1 or a
constant for every event, but it still must be accounted for explicity when
writing generic NuHepMC analyses. 

To correctly scale some summary property of an event stream to a flux-averaged 
cross section measurement prediction, an overall scale of `fatx/sumw` will also
need to be applied, where `sumw` is the sum of weights for all *read* events
not just all selected events. The `fatx` and `sumw` are important properties of
the generator run and not your analysis. For the simplifying case where the 
G.R.7 CV weight is 1 for all events, then `sumw == nevents`, but this must not 
be assumed.


### Event Processing

We can write a simple `ProcessEvent` function that dumps out some assorted
interesting information about every event that it is passed.

First lets write a helper function that pretty-prints interesting particle
information. `HepMC3::Print` does provide pretty print functions, but they're a
bit verbose.

```c++
std::string PartToStr(HepMC3::ConstGenParticlePtr pt) {
  if (!pt) {
    return "PARTICLE-NOTFOUND";
  }
  std::stringstream ss;

  std::string status = partstatus.count(pt->status())
                           ? partstatus.at(pt->status()).first
                           : std::to_string(pt->status());

  ss << "{ id: " << pt->id() << ", pid: " << pt->pid() << ", status: " << status
     << ", p: ( " << pt->momentum().x() << ", " << pt->momentum().y() << ", "
     << pt->momentum().z() << ", E: " << pt->momentum().e() << ") GeV }";

  return ss.str();
}
```

Then we can then implement a `ProcessEvent` function that writes out event
information for every event it is passed.

```c++
void ProcessEvent(HepMC3::GenEvent &evt) {

  auto beampt = Event::GetBeamParticle(evt);
  auto tgtpt = Event::GetTargetParticle(evt);

  auto primary_vtx = Event::GetPrimaryVertex(evt);

  auto process_id = ER3::ReadProcessID(evt);

  std::cout << "Evt: " << evt.event_number()
            << ", channel name: " << proc_ids[process_id].first << std::endl;
  std::cout << "  NVertices = " << evt.vertices().size()
            << ", NParticles = " << evt.particles().size() << std::endl;
  std::cout << "  Beam particle = " << PartToStr(beampt) << std::endl;
  std::cout << "  Target particle = " << PartToStr(tgtpt) << std::endl;
  std::cout << "  Primary Vertex:" << std::endl;
  size_t ctr = 0;
  for (auto &pt : primary_vtx->particles_in()) {
    std::cout << "    in[" << ctr++ << "]: " << PartToStr(pt) << std::endl;
  }
  ctr = 0;
  for (auto &pt : primary_vtx->particles_out()) {
    std::cout << "    out[" << ctr++ << "]: " << PartToStr(pt) << std::endl;
  }
  std::cout << "  Final state particles:" << std::endl;
  ctr = 0;
  for (auto &pt : Event::GetParticles_AllRealFinalState(evt)) {
    std::cout << "    fs[" << ctr++ << "]: " << PartToStr(pt) << std::endl;
  }
}
```

#### Example outputs

An ACHILLES event:

```
Evt: 25945, channel name: QESpectralCC0p0pi
  NVertices = 4, NParticles = 10
  Beam particle = { id: 10, pid: 14, status: Incoming beam Particle, p: ( 0, 0, 80, E: 80) GeV }
  Target particle = { id: 9, pid: 1000180400, status: Target particle, p: ( 0, 0, 0, E: 37215.8) GeV }
  Primary Vertex:
    in[0]: { id: 1, pid: 2112, status: Decayed physical particle, p: ( -85.1989, 296.03, -46.4709, E: 918.539) GeV }
    in[1]: { id: 2, pid: 14, status: Decayed physical particle, p: ( 0, 0, 1410.99, E: 1410.99) GeV }
    out[0]: { id: 3, pid: 2212, status: Propagating, p: ( 392.819, -95.4914, 251.554, E: 1052.17) GeV }
  Final state particles:
    fs[0]: { id: 4, pid: 13, status: Undecayed physical particle, p: ( -478.018, 391.522, 1112.96, E: 1277.36) GeV }
    fs[1]: { id: 5, pid: 2212, status: Undecayed physical particle, p: ( 141.409, -93.4786, 47.9892, E: 954.667) GeV }
    fs[2]: { id: 6, pid: 2112, status: Undecayed physical particle, p: ( 142.775, 149.573, -108.146, E: 966.852) GeV }
    fs[3]: { id: 7, pid: 2212, status: Undecayed physical particle, p: ( 14.5119, 78.5001, 143.176, E: 952.483) GeV }
    fs[4]: { id: 8, pid: 2212, status: Undecayed physical particle, p: ( 216.949, -183.061, 202.668, E: 1001) GeV }
```

A NEUT event:

```
Evt: 6073, channel name: CC_multi_pi_nu
  NVertices = 3, NParticles = 15
  Beam particle = { id: 4, pid: 14, status: IncomingBeam, p: ( 0, 0, 2832.71, E: 2832.71) GeV }
  Target particle = { id: 1, pid: 1000180400, status: TargetParticle, p: ( 0, 0, 0, E: 0) GeV }
  Primary Vertex:
    in[0]: { id: 4, pid: 14, status: IncomingBeam, p: ( 0, 0, 2832.71, E: 2832.71) GeV }
    in[1]: { id: 3, pid: 2112, status: StruckNucleon, p: ( 175.006, -136.761, -30.3932, E: 965.939) GeV }
    out[0]: { id: 5, pid: 13, status: DocumentationLine, p: ( -308.677, -477.374, 257.145, E: 632.815) GeV }
    out[1]: { id: 6, pid: 2212, status: UnderwentFSI, p: ( 553.409, 328.314, 1040.24, E: 1541.59) GeV }
    out[2]: { id: 7, pid: -211, status: DocumentationLine, p: ( -303.444, 27.1702, 745.302, E: 817.173) GeV }
    out[3]: { id: 8, pid: 211, status: DocumentationLine, p: ( 233.718, -14.8713, 759.63, E: 807.07) GeV }
  Final state particles:
    fs[0]: { id: 9, pid: 2009900000, status: UndecayedPhysical, p: ( 0, 0, 0, E: 0) GeV }
    fs[1]: { id: 10, pid: 13, status: UndecayedPhysical, p: ( -308.677, -477.374, 257.145, E: 632.815) GeV }
    fs[2]: { id: 11, pid: -211, status: UndecayedPhysical, p: ( -303.444, 27.1702, 745.302, E: 817.173) GeV }
    fs[3]: { id: 12, pid: 211, status: UndecayedPhysical, p: ( 233.718, -14.8713, 759.63, E: 807.07) GeV }
    fs[4]: { id: 13, pid: 111, status: UndecayedPhysical, p: ( -2.17097, 56.577, 28.5757, E: 149.131) GeV }
    fs[5]: { id: 14, pid: 2212, status: UndecayedPhysical, p: ( 497.899, 228.388, 223.754, E: 1109.27) GeV }
    fs[6]: { id: 15, pid: 2112, status: UndecayedPhysical, p: ( 75.2598, -34.7639, 744.115, E: 1201.4) GeV }
```

A NuWro event:

```
Evt: 3895, channel name: NC qel
  NVertices = 3, NParticles = 10
  Beam particle = { id: 4, pid: 14, status: IncomingBeamParticle, p: ( 0, 0, 2934.81, E: 2934.81) GeV }
  Target particle = { id: 1, pid: 1000180400, status: TargetParticle, p: ( 0, 0, 0, E: 0) GeV }
  Primary Vertex:
    in[0]: { id: 4, pid: 14, status: IncomingBeamParticle, p: ( 0, 0, 2934.81, E: 2934.81) GeV }
    in[1]: { id: 3, pid: 2112, status: StruckNucleon, p: ( 63.9722, 13.0031, 71.9294, E: 922.76) GeV }
    out[0]: { id: 5, pid: 14, status: DocumentationLine, p: ( 230.165, 425.32, 2742.87, E: 2785.17) GeV }
    out[1]: { id: 6, pid: 2112, status: DocumentationLine, p: ( -166.193, -412.317, 263.875, E: 1072.4) GeV }
  Final state particles:
    fs[0]: { id: 7, pid: 2009900000, status: UndecayedPhysicalParticle, p: ( 0, 0, 0, E: 0) GeV }
    fs[1]: { id: 8, pid: 14, status: UndecayedPhysicalParticle, p: ( 230.165, 425.32, 2742.87, E: 2785.17) GeV }
    fs[2]: { id: 9, pid: 2112, status: UndecayedPhysicalParticle, p: ( 127.216, -294.953, 76.403, E: 995.892) GeV }
    fs[3]: { id: 10, pid: 2212, status: UndecayedPhysicalParticle, p: ( -304.577, -42.1201, 180.771, E: 1003.78) GeV }
```

A GENIE event:

```
Evt: 2481, channel name: QES-Weak[CC]
  NVertices = 4, NParticles = 9
  Beam particle = { id: 1, pid: 14, status: Beam, p: ( 0, 0, 2.59026, E: 2.59026) GeV }
  Target particle = { id: 2, pid: 1000180400, status: Target, p: ( 0, 0, 0, E: 37.2155) GeV }
  Primary Vertex:
    in[0]: { id: 1, pid: 14, status: Beam, p: ( 0, 0, 2.59026, E: 2.59026) GeV }
    in[1]: { id: 3, pid: 2112, status: Target nucleon, p: ( -0.138524, 0.0474722, 0.00161607, E: 0.912119) GeV }
    out[0]: { id: 6, pid: 13, status: Final state, p: ( 0.507448, 0.580357, 1.93058, E: 2.0815) GeV }
    out[1]: { id: 7, pid: 2212, status: Hadron in the nucleus, p: ( -0.645972, -0.532885, 0.661297, E: 1.42089) GeV }
  Final state particles:
    fs[0]: { id: 5, pid: 22, status: Final state, p: ( 2.5586e-05, -4.77668e-05, 0.00221057, E: 0.00221124) GeV }
    fs[1]: { id: 6, pid: 13, status: Final state, p: ( 0.507448, 0.580357, 1.93058, E: 2.0815) GeV }
    fs[2]: { id: 8, pid: 2212, status: Final state, p: ( -0.645972, -0.532885, 0.661297, E: 1.42089) GeV }
```

## API Reference

This package contains helper functions for reading, analysing, and writing 
NuHepMC files.

The hope is that they can be used to abstract away implementation details of the 
NuHepMC standard and make for more declarative code that works with NuHepMC 
events.

* Reading: [`ReaderUtils`](#readerutils)
* Analysing: [`EventUtils`](#eventutils), [`FATXUtils`](#fatxutils),
* Writing: [`WriterUtils`](#writerutils), [`make_writer`](#make-writer)
* Miscellaneous: [`AttributUtils`](#attributeutils), [`Constants`](#constants), 
  [`UnitsUtils`](#unitsutils)

### ReaderUtils

Helper functions that abstract the querying of `HepMC3::GenRunInfo` and 
`HepMC3::GenEvent` objects for NuHepMC metadata. Each are namespaced according 
to the specific Requirement, Convention, or Suggestion that they correspond to.

```c++
#include "NuHepMC/ReaderUtils.hxx"
```

#### GenRunInfo

```c++

std::tuple<int, int, int> NuHepMC::GR2::ReadVersion(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info);
std::string NuHepMC::GR2::ReadVersionString(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info);

//StatusCodeDescriptors are defined in NuHepMC/Types.hxx as 
// using StatusCodeDescriptors =
//  std::map<int, std::pair<std::string, std::string>>;
// StatusCodeDescriptors[<id>].first is the human readoable name of the <id>
// StatusCodeDescriptors[<id>].second is a longer form description

StatusCodeDescriptors NuHepMC::GR4::ReadProcessIdDefinitions(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info);

StatusCodeDescriptors NuHepMC::GR5::ReadVertexStatusIdDefinitions(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info);

StatusCodeDescriptors NuHepMC::GR6::ReadParticleStatusIdDefinitions(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info);

StatusCodeDescriptors NuHepMC::GR8::ReadNonStandardParticleNumbers(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info);

std::set<std::string> NuHepMC::GC1::ReadConventions(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info);
bool NuHepMC::GC1::SignalsConvention(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info, 
  std::string const &Convention);
bool NuHepMC::GC1::SignalsConventions(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info,
  std::vector<std::string> Conventions);

long NuHepMC::GC2::ReadExposureNEvents(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info);

double NuHepMC::GC3::ReadExposurePOT(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info);
double NuHepMC::GC3::ReadExposureLivetime(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info);


std::pair<std::string, std::string> NuHepMC::GC4::ReadCrossSectionUnits(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info);

// See NuHepMC/UnitsUtils.hxx for defined unit enums
CrossSection::Units::Unit NuHepMC::GC4::ParseCrossSectionUnits(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info);


double NuHepMC::GC5::ReadFluxAveragedTotalXSec(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info);

CitationData NuHepMC::GC6::ReadAllCitations(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info);

// See NuHepMC/Types.hxx for the definition of the EnergyDistribution type
// The integer key corresponds to the pid/PDG numbers for a beam particle 
// species with a known energy distribution
std::map<int, EnergyDistribution> NuHepMC::GC7::ReadAllEnergyDistributions(
  std::shared_ptr<HepMC3::GenRunInfo const> run_info);
```

#### GenEvent

```c++
int NuHepMC::ER3::ReadProcessID(HepMC3::GenEvent const &evt);

std::vector<double> NuHepMC::ER5::ReadLabPosition(HepMC3::GenEvent const &evt);

double NuHepMC::EC2::ReadTotalCrossSection(HepMC3::GenEvent const &evt);

double NuHepMC::EC3::ReadProcessCrossSection(HepMC3::GenEvent const &evt);

double NuHepMC::EC4::ReadFluxAveragedTotalXSecCVBestEstimate(
  HepMC3::GenEvent const &evt);
```

### EventUtils

Helper functions for working with `HepMC3::GenEvent`s and `HepMC3::GenVertex`s.

```c++
#include "NuHepMC/EventUtils.hxx"
```

#### GenEvent

```c++

// For NuHepMC-defined vertex status codes, you can use definitions from 
//   NuHepMC/Constants
HepMC3::ConstGenVertexPtr NuHepMC::Event::GetVertex_First(
  HepMC3::GenEvent const &evt, int vtx_status);
HepMC3::ConstGenVertexPtr NuHepMC::Event::GetPrimaryVertex(
  HepMC3::GenEvent const &evt);

HepMC3::ConstGenParticlePtr NuHepMC::Event::GetBeamParticle(
  HepMC3::GenEvent const &evt);
HepMC3::ConstGenParticlePtr NuHepMC::Event::GetTargetParticle(
  HepMC3::GenEvent const &evt);

// For NuHepMC-defined particle status codes, you can use definitions from 
//   NuHepMC/Constants
std::vector<HepMC3::ConstGenParticlePtr> NuHepMC::Event::GetParticles_All(
  HepMC3::GenEvent const &evt, int part_status, std::vector<int> PDGs = {});

std::vector<HepMC3::ConstGenParticlePtr> 
NuHepMC::Event::GetParticles_AllRealFinalState(HepMC3::GenEvent const &evt, 
  std::vector<int> PDGs = {});
HepMC3::ConstGenParticlePtr NuHepMC::Event::GetParticle_First(
  HepMC3::GenEvent const &evt, int part_status, std::vector<int> PDGs = {});
HepMC3::ConstGenParticlePtr NuHepMC::Event::GetParticle_HighestMomentum(
  HepMC3::GenEvent const &evt, int part_status, std::vector<int> PDGs = {});

// Reads the energy/momentum units of a given event and calculates the scale 
//   factor required to express particle quantities in MeV-scale units
double NuHepMC::Event::ToMeVFactor(HepMC3::GenEvent const &evt);
```

#### GenVertex

```c++

// For NuHepMC-defined particle status codes, you can use definitions from 
//   NuHepMC/Constants
std::vector<HepMC3::ConstGenParticlePtr>
NuHepMC::Vertex::GetParticlesIn_All(HepMC3::ConstGenVertexPtr &evt, 
  int part_status, std::vector<int> PDGs = {});

HepMC3::ConstGenParticlePtr
NuHepMC::Vertex::GetParticleIn_HighestMomentum(HepMC3::ConstGenVertexPtr &evt, 
  int part_status, std::vector<int> PDGs = {});

std::vector<HepMC3::ConstGenParticlePtr>
NuHepMC::Vertex::GetParticlesOut_All(HepMC3::ConstGenVertexPtr &vtx, 
  int part_status, std::vector<int> PDGs = {});

HepMC3::ConstGenParticlePtr
NuHepMC::Vertex::GetParticleOut_HighestMomentum(HepMC3::ConstGenVertexPtr &evt, 
  int part_status, std::vector<int> PDGs = {});
```

### FATXUtils

A helper class for estimating the flux-averaged total cross section from a 
stream of NuHepMC events.

```c++
#include "NuHepMC/FATXUtils.hxx"
```

```c++
namespace NuHepMC {
namespace FATX {

// ABC for FATX accumulators that can give their best estimate of the FATX after
//   being passed N events
//   Some subclasses will know the best estimate after one events and others will
//   continue to get better the more events you pass them
struct Accumulator {
  // Returns the cv weight for the proferred event and accumulates FATX 
  //   information in an input-dependent way
  virtual double process(HepMC3::GenEvent const &) = 0;
  // Get the current best estimate of the FATX from processed events
  virtual double fatx(CrossSection::Units::Unit const &units = 
    CrossSection::Units::pb_PerAtom) = 0;
  // Get the sum of event weights from all processed events
  virtual double sumweights() = 0;
  // Get the count of all processed events
  virtual size_t events() = 0;
};

// Accumulator factory function which picks the best FATX estimation technique
//   from conventions signalled in the GenRunInfo
std::unique_ptr<Accumulator>
MakeAccumulator(std::shared_ptr<HepMC3::GenRunInfo> gri);

// Accumulator factory function that selects the accumulator from NuhepMC
//   convention identifier. Valid options: G.C.5, E.C.2, E.C.4
std::unique_ptr<Accumulator> MakeAccumulator(std::string const &Convention);

}
}
```

### WriterUtils

Helper functions that abstract the writing of NuHepMC metadata on 
`HepMC3::GenRunInfo` and `HepMC3::GenEvent` objects. 
Each are namespaced according to the specific Requirement, Convention, or 
Suggestion that they correspond to.

```c++
#include "NuHepMC/WriterUtils.hxx"
```

#### GenRunInfo

```c++

void NuHepMC::GR2::WriteVersion(std::shared_ptr<HepMC3::GenRunInfo> run_info);

void NuHepMC::GR4::WriteProcessIDDefinitions(
  std::shared_ptr<HepMC3::GenRunInfo> run_info,
  StatusCodeDescriptors const &Definitions);

void NuHepMC::GR5::WriteVertexStatusIDDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo> run_info,
    StatusCodeDescriptors const &Definitions);

void NuHepMC::GR6::WriteParticleStatusIDDefinitions(
    std::shared_ptr<HepMC3::GenRunInfo> run_info,
    StatusCodeDescriptors const &Definitions);

void NuHepMC::GR7::SetWeightNames(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                    std::vector<std::string> const &names);

void NuHepMC::GR8::WriteNonStandardParticleNumbers(
    std::shared_ptr<HepMC3::GenRunInfo> run_info,
    std::map<int, std::string> const &nonstandard_pdg_definitions);
void NuHepMC::GR8::WriteNonStandardParticleNumbers(
    std::shared_ptr<HepMC3::GenRunInfo> run_info,
    ParticleNumberDescriptors const &nonstandard_pdg_definitions);


void NuHepMC::GC1::SetConventions(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                    std::vector<std::string> const &conventions);

void NuHepMC::GC2::SetExposureNEvents(
  std::shared_ptr<HepMC3::GenRunInfo> run_info, int NEvents);

void NuHepMC::GC3::SetExposurePOT(std::shared_ptr<HepMC3::GenRunInfo> run_info,
 double POT);
void NuHepMC::GC3::SetExposureLivetime(
  std::shared_ptr<HepMC3::GenRunInfo> run_info, double Livetime);

void NuHepMC::GC4::SetCrossSectionUnits(
  std::shared_ptr<HepMC3::GenRunInfo> run_info,std::string const &xs_units,
  std::string const &target_scale);

void NuHepMC::GC5::SetFluxAveragedTotalXSec(
  std::shared_ptr<HepMC3::GenRunInfo> run_info, double fatx);

void NuHepMC::GC6::AddCitationMetadata(
  std::shared_ptr<HepMC3::GenRunInfo> run_info, std::string const &component, 
  std::string const &type, std::vector<std::string> const &values);

void NuHepMC::GC6::AddGeneratorCitation(
  std::shared_ptr<HepMC3::GenRunInfo> run_info, std::string const &type,
  std::vector<std::string> const &values);

void NuHepMC::GC6::AddProcessCitation(
  std::shared_ptr<HepMC3::GenRunInfo> run_info, int const &ProcID, 
  std::string const &type, std::vector<std::string> const &values);

void NuHepMC::GC7::WriteBeamUnits(std::shared_ptr<HepMC3::GenRunInfo> run_info,
  std::string const &EnergyUnit, std::string const &RateUnit = "");
void NuHepMC::GC7::SetHistogramBeamType(
  std::shared_ptr<HepMC3::GenRunInfo> run_info);
void NuHepMC::GC7::SetMonoEnergeticBeamType(
  std::shared_ptr<HepMC3::GenRunInfo> run_info);
void NuHepMC::GC7::WriteBeamEnergyHistogram(
  std::shared_ptr<HepMC3::GenRunInfo> run_info, int BeamParticleNumber,
  std::vector<double> const &bin_edges, std::vector<double> const &bin_content,
  bool ContentIsPerWidth = false);
void NuHepMC::GC7::WriteBeamEnergyMonoenergetic(
  std::shared_ptr<HepMC3::GenRunInfo> run_info, int BeamParticleNumber, 
  double const &energy);
void NuHepMC::GC7::WriteBeamEnergyDistribution(
  std::shared_ptr<HepMC3::GenRunInfo> run_info, int BeamParticleNumber,
  EnergyDistribution distribution);
void NuHepMC::GC7::WriteBeamEnergyDistributions(
  std::shared_ptr<HepMC3::GenRunInfo> run_info,
  std::map<int, EnergyDistribution> const &distributions);

```

#### GenEvent

```c++
void NuHepMC::ER3::SetProcessID(HepMC3::GenEvent &evt, int ProcID);

void NuHepMC::ER5::SetLabPosition(HepMC3::GenEvent &evt, 
  std::vector<double> const &LabPos);

void NuHepMC::EC2::SetTotalCrossSection(HepMC3::GenEvent &evt, double CrossSec);

void NuHepMC::EC3::SetProcessCrossSection(HepMC3::GenEvent &evt, 
  double CrossSec);
```

#### GenParticle

```c++
void NuHepMC::PC2::SetRemnantParticleNumber(HepMC3::GenParticlePtr &ptr, 
  int particle_number);
```

### make_writer

A helper function for picking the most appropriate `HepMC3::Writer` subclass for
an output filename; _e.g._ a HepMC3::WriterGZ<HepMC3::protobufIOWriter> would be
instantiated for the filename `myevents.pb.gz`.

```c++
#include "NuHepMC/make_writer.hxx"
```

```c++

HepMC3::Writer * NuHepMC::Writer::make_writer(std::string const &name,
            std::shared_ptr<HepMC3::GenRunInfo> run_info = nullptr);
```

### AttributeUtils

Helper template functions picking the correct `HepMC3::Attribute` subclass to 
read and write common C++ types. See 
[src/NuHepMC/Traits.hxx](src/NuHepMC/Traits.hxx) for the mapping between C++ 
types and `HepMC3::Attribute` types.

```c++
#include "NuHepMC/AttributeUtils.hxx"
```

#### Writing Attributes

```c++
template <typename T>
void NuHepMC::add_attribute(std::shared_ptr<HepMC3::GenRunInfo> run_info,
                   std::string const &name, T const &val);

template <typename T>
void NuHepMC::add_attribute(HepMC3::GenParticlePtr &part, std::string const &name,
                   T const &val);

template <typename T>
void NuHepMC::add_attribute(HepMC3::GenEvent &ge, std::string const &name,
                   T const &val);
```

#### Reading Attributes

```c++
template <typename T> bool HasAttribute(T const &obj, std::string const &name);

template <typename AT, typename T>
auto CheckedAttributeValue(T const &obj, std::string const &name);

template <typename AT, typename T>
auto CheckedAttributeValue(T const &obj, std::string const &name,
                           AT const &defval);
```

### Constants

Some useful enum-like definitions corresponding to NuHepMC-defined status 
identifiers.

```c++
#include "NuHepMC/Constants.hxx"
```

```c++

// V.R.1
NuHepMC::VertexStatus::Primary;
NuHepMC::VertexStatus::FSISummary;
// V.C.1
NuHepMC::VertexStatus::NucleonSeparation;

// P.R.1
NuHepMC::ParticleStatus::UndecayedPhysical;
NuHepMC::ParticleStatus::DecayedPhysical;
NuHepMC::ParticleStatus::DocumentationLine;
NuHepMC::ParticleStatus::IncomingBeam;
NuHepMC::ParticleStatus::Target;
// P.C.1
NuHepMC::ParticleStatus::StruckNucleon;

// P.C.2
NuHepMC::ParticleNumber::NuclearRemnant;
```

### UnitsUtils

Some useful enums for NuHepMC-defined cross section units and a helper function
for calculating scale factors between different unit definitions.

```c++
#include "NuHepMC/UnitsUtils.hxx"
```

```c++
namespace NuHepMC {
namespace CrossSection {
namespace Units {

enum class Scale { CustomType, pb, cm2, cm2_ten38, Automatic };

enum class TargetScale {
  CustomType,
  PerTargetMolecule,
  PerTargetAtom,
  PerTargetNucleon,
  PerTargetMolecularNucleon,
  Automatic
};

struct Unit {
  Scale scale;
  TargetScale tgtscale;
  bool operator==(Unit const &other) const {
    return (scale == other.scale) && (tgtscale == other.tgtscale);
  }

  bool operator!=(Unit const &other) const { return !(*this == other); }
};

const Unit pb_PerAtom{Scale::pb, TargetScale::PerTargetAtom};
// automatic is used to signal that the input scale should be read from the 
//   evt.run_info() according to G.C.4
const Unit automatic{Scale::Automatic, TargetScale::Automatic};

double GetRescaleFactor(HepMC3::GenEvent const &evt,
                        Unit from = automatic,
                        Unit const &to = pb_PerAtom);
}
}
}
```
