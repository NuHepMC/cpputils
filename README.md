# cpputils

C++ Helper functions for NuHepMC reference implementation

## Building

```bash
cd /path/to/cpputils
mkdir build; cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/where/you/want/to/install
make install
```

We currently recommend using builtin HepMC3 to avoid bugs in the latest release (as of 2023/11/02).
Use the below instead.

```bash
cd /path/to/cpputils
mkdir build; cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/where/you/want/to/install -DBUILTIN_HEPMC3=ON
make install
```

## Setting up the environment

```
eval $(/path/to/cpputils/build/$(uname -s)/bin/NuHepMC-config --env)
```

This will also set up the HepMC3 autogenerated python bindings for you.

## MicroAnalysis Helper

Make a new skeleton analysis with `NuHepMC-config --skele myana.cxx`

Modify it to your hearts content.

Compile it with `NuHepMC-config --build myana.cxx`

And then run it like: `myana input.hepmc`.

There is nothing clever going on here, just a helper for compiler flags to
use as a starting point for a quick analysis.
