# GiBUUToStdHep output file format.

The output format is based around a ROOT tree where each event contains a stdhep
particle stack of incoming and nuclear-leaving particles. Some extra generation
information is also included, as well as a best-guess at determining the
NEUT-equivalent neutrino interaction mode.

The individual output branches are well documented in the doxygen autodocs
below: GiRooTracker.

The correspondence between GiBUU's internal 'prodid' and the converted Neut-equivalent code
can be seen in the documentation for: GiBUUUtils::GiBUU2NeutReacCode.

The correspondence between GiBUU's internal particle numbering scheme and the pdg codes
can be seen in the documentation for: GiBUUUtils::GiBUUToPDG.

**Note:** Following the GENIE convention, the struck nucleon is given a
`StdHepStatus == 11`.

**Note:** The target nucleus information is saved at index `1` in the StdHep
arrays. The `StdHepPdg` is a nuclear PDG code (100ZZZAAA0) and can be decomposed
into target A, and Z like:

    TargetZ = ((StdHepPdg[1] / 10000) % 1000);
    TargetA = ((StdHepPdg[1] / 10) % 1000);
