# cpputils

C++ Helper functions for NuHepMC reference implementation

## Building

```bash
cd /path/to/cpputils
mkdir build; cd build
cmake ..
make install
```

## Setting up the environment

```
eval /path/to/cpputils/build/$(uname -s)/bin/NuHepMC-config --env
```

## MicroAnalysis Helper

Make a new skeleton analysis with `NuHepMC-config --skele myana.cxx`

Modify it to your hearts content.

Compile it with `NuHepMC-config --build myana.cxx`

And then run it like: `myana input.hepmc`.

There is nothing clever going on here, just a helper for compiler flags to
use as a starting point for a quick analysis.