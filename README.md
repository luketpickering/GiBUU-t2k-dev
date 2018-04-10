# GiBUUTools

  **GiBUUToStdHep** is a tool for converting the default GiBUU, text-based
  neutrino-mode event output into a format that is more commonly used in the
  neutrino generator community. It aims to preserve as much of the useful
  information provided by the generator for subsequent analysis.

  **GiBUUFluxTools** is a tool for converting neutrino flux histograms from
  bin-edge text histograms or root files containing a TH1 to a bin-center
  text histogram that GiBUU can use to throw neutrino events.

# Building GiBUUTools

  To build GiBUUTools:

  - Make a build directory: `mkdir build`
  - Configure the build: `cd build && cmake ../`
  - Optional -- If you want to download, patch, and build a local version of
  GiBUU2017 use `cmake /path/to/source -DUSE_GIBUU=1` instead.
  - Build! `make`.
  - Optional: Build the documentation -- `make docs`.
    - This release should come with pre-compiled documentation at
    `dox/GiBUUTools.pdf`
  - Install: `make install`.

  - N.B. Your compiler *must* have c++11 support. If you are trying to build
    with a non-system compiler, you must run cmake with
    `-DCMAKE_CXX_COMPILER=$(which g++) -DCMAKE_C_COMPILER=$(which gcc)`
    options manually specified, or cmake will use the system compiler.

  To set up a built GiBUUTools:

  - `source build/Linux/setup.sh`
    - `Linux` will change depending on your `CMAKE_SYSTEM_NAME`

  This will add GiBUUTools and GiBUU (if built) to the PATH.

# IMPORTANT

  Whenever you plot properties of GiBUU events, you **must** weight the events
  by the 'event weight' (`EvtWght` in the stdhep tree). GiBUU throws events
  according to the neutrino flux shape, if you do not weight by the xsec, then
  your distributions *will* be just **wrong**.
