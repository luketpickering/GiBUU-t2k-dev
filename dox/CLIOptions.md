# GiBUUToStdHep Command Line Interface

  The options that you need to pass to correctly parse and combine the GiBUU
  output can be a bit fiddly at times. As GiBUU cannot generate events on
  composite targets, or by multiple neutrino species, or both CC and NC events
  in the same run, multiple, separate GiBUU runs must be consistently combined.

  **Note:** even when consistently combined, the raw events will not be `correctly'
  distributed, however, the event weights will be correct. **You must always
  weight event properties by the `EvtWght` when building histograms**.

  **Note:** It is almost always easiest to increase the statistics of a single
  run mode by running multiple identical jobs. Correctly merging the cross
  section predictions from two simulations, run on the same target with the same
  neutrino species and CC/NC mode, but with differing `input:numEnsembles` or
  `input:numTimeSteps` is infeasibly fiddly. If you want to generate a large
  number of events, spread multiple identical jobs over multiple CPUs.
  **IMPORTANT**: Remember to change `initRandom:SEED` for each run, or your
  separate runs will be *too* identical.

  Command line options are split into two types, ones that affect the whole
  parsing, and ones that affect only the next file or group of files.

  Any of these arguments can be placed inside a space and newline separated
  ascii file which can then be passed to `GiBUUToStdHep` by the `-@` argument.

## Whole parsing options

  * `(-s|--long-form) <Option Type {default value}> [Required or not]`: Example of an option described by this documentation.


  * `(-h|--help)`: Print a help message with a usage example.
  * `(-c|--CompositeExample)`: Print the example usage for building a full CH-target vector including neutrino and anti-neutrino flux components
  * `(-@) <FileName>` : Replace this CLI argument with the contents of the referenced file. Useful for organising the large number of required arguments for a composite target vector.
  * `(-o|--output-file) <FileName {default:GiBUURooTracker.root}>`: The output file name.
  * `(-R|--Total-ReWeight) [i]<[1.0/]float>`: The overall weight to apply to the output vector, useful for outputting a composite-target vector with xsec weights in units of `/nucleon`. If the value is prepended with an `i` then the inverse of the numerical part of the option is used, e.g. if `-T i12` is passed, then the file weight will be `1/12`.
  * `(-NI|--No-Initial-State)`: If you are using an old version of GiBUU which does not output initial state/target nucleon information this will not look for it. GiBUU2016 has initial state information in the output FinalEvents.dat
  * `(-NP|--No-Prod-Charge)`: If you are using a default version of GiBUU, as opposed to the patched version that can be built by this package, if enabled, this will not expect that information. This makes guessing the NEUT-equivalent mode more tricky as you do not know the charge of the neutrino-induced resonance state.
  * `(-v|--Verbosity) <0-4>`: Raises the verbosity of the parsing.

## Options which affect the next input file(s)

  As making a complete event vector with GiBUU always requires multiple runs,
  a number of CLI options will have to be specified for each file. e.g. Whether
  the next file contains NC or CC events, whether the next file contains
  Carbon-target, or Hydrogen-target. The options will be described before a few
  examples are given, which should hopefully make their use more clear.

  * `(-u|--nu-pdg) <int> [required at least once]`: Specifies the neutrino species PDG of the next file(s). **Note:** This option is assumed for subsequent `-f` options until overriden.
  * `(-N|--is-NC)`: Specifies whether the next file(s) are simulated NC events. **Note:** This option is assumed for subsequent `-f` options until overriden.
  * `(-a|--target-a) <int> [required at least once]`: Specifies the nucleon number of the target used in the next file(s). **Note:** This option is assumed for subsequent `-f` options until overriden.
  * `(-z|--target-z) <int> [required at least once]`: Specifies the nucleon number of the target used in the next file(s). **Note:** This option is assumed for subsequent `-f` options until overriden.
  * `(-W|--file-weight) [i]<[1.0/]float>`: Specifies the overall file target weight for the next file(s). If the value is prepended with an `i` then the inverse of the numerical part of the option is used, e.g. if `-T i12` is passed, then the file weight will be `1/12`. **Note:** This option is reset to `1.0` for subsequent `-f` options, the next file(s) weight *must* be specified for each set of files to be parsed.
  * `(-f|--FEinput-file) <File Name>  [required at least once]`: Specifies the next file(s) to parse, which all previous 'per file' options will apply to. Wildcards are allowed at the file level of the specifier, but not at a directory level: e.g. `-f "some/subdir/FinalEvents*.dat"` is allowed but `-f "some/sub*dir/FinalEvents.dat"` is not. Averaging over multiple runs is handled automatically, so the file weight specified by `-W` does not need to account for multiple files being parsed due to the wildcard expansion. **Note:** Be careful not to let the calling shell expand the wildcard, when using a wildcard in the file specifier, wrap the path in double quotes, e.g.: `-f "path/to/some/files*.dat"`.
  * `(-F|--Save-Flux-File) <output_hist_name,input_text_flux_file.txt>`: This option is used to save the GiBUU-style bin-centered flux histogram stored in `'input_text_flux_file.txt'` as a ROOT `TH1` named `'output_hist_name'` in the output file. This can be useful for some downstream code.

## Notes on event weight combinations

  As part of the generation, GiBUU averages cross section weights over the
  ensemble of generated targets (`input:numEnsembles`). *You never need to
  account for this averaging*. The averaging over weights for values of
  `input:num_runs_SameEnergy` larger than `1` is handled automatically by
  `GiBUUToStdHep` upon parsing each `FinalEvents.dat` file. *You never need
  to account for this averaging*.

  *You do need to account for averaging over the number of nucleons of a
  composite target*. The cross section weights in the final event vector are
  often expected to be in units of per nucleon. While it might be expected that
  this weight handling could be taken care of by using the `-a` option, for
  clarity when building vectors of targets such as CH2 it was decided that this
  weighting should be explicit. Therefore, when combining runs for a composite
  target, the per nucleon weights needs to be manually applied: e.g. for a CH2
  target, the overall output weight should be `-R i14`, the input carbon-target
  run weights should be `-W 12` and the input hydrogen-target run weights should
  be `-W 2`. This weights each contribution up correctly for the target that was
  simualted in a given run (C or H), and then weights the combined results back
  down to per composite-target-nucleon. The resulting weights in the event
  vector should correctly give cross sections in units of
  `10^-38 cm^2 /nucleon`.

### Composite target examples

  To build an event vector of correctly weighted CH2 muon neutrino mode events
  where the Carbon-target events are stored in some subdirectory `numu_C_CC`,
  and the hydrogen-target in `numu_H_CC`, in files named
  `FinalEvents_<RunNum>.dat` the command might be:


      GiBUUToStdHep\
        -u 14 -a 12 -z 6 -W 12 -f "numu_C_CC/FinalEvents_*.dat" \
          -a 1 -z 1 -W 2 -f "numu_H_CC/FinalEvents_*.dat"\
        -R i14 -o numu_CH2_CC.stdhep.root


### Composite beam examples

  To build an event vector that from multiple GiBUU runs that used different
  neutrino components of a realistic flux, where the Carbon-target anti-nue
  mode events are stored in some subdirectory `FHC_nuebar_C_CC` (FHC stands
  for Forward Horn Current, often synonymous with a 'mostly muon neutrino
  beam'), and other components follow a similar naming convention, the command
  might be:

      GiBUUToStdHep \
        -u 14 -a 12 -z 6 -W 12 -f "FHC_numu_C_CC/FinalEvents_*.dat" \
          -a 1 -z 1 -W 2 -f "FHC_numu_H_CC/FinalEvents_*.dat" \
        -u -14 -a 12 -z 6 -W 12 -f "FHC_numubar_C_CC/FinalEvents_*.dat" \
          -a 1 -z 1 -W 2 -f "FHC_numubar_H_CC/FinalEvents_*.dat"\
        -u 12 -a 12 -z 6 -W 12 -f "FHC_nue_C_CC/FinalEvents_*.dat" \
          -a 1 -z 1 -W 2 -f "FHC_nue_H_CC/FinalEvents_*.dat"\
        -u -12 -a 12 -z 6 -W 12 -f "FHC_nuebar_C_CC/FinalEvents_*.dat" \
          -a 1 -z 1 -W 2 -f "FHC_nuebar_H_CC/FinalEvents_*.dat"\
        -R i14 -o numu_CH2_CC.stdhep.root


  You can see that it is starting to build up...

### Full example

  And now to add in some NC events...


      GiBUUToStdHep \
        -u 14 -a 12 -z 6 -W 12 -f "FHC_numu_C_CC/FinalEvents_*.dat" \
          -a 1 -z 1 -W 2 -f "FHC_numu_H_CC/FinalEvents_*.dat" \
        -u -14 -a 12 -z 6 -W 12 -f "FHC_numubar_C_CC/FinalEvents_*.dat" \
          -a 1 -z 1 -W 2 -f "FHC_numubar_H_CC/FinalEvents_*.dat"\
        -u 12 -a 12 -z 6 -W 12 -f "FHC_nue_C_CC/FinalEvents_*.dat" \
          -a 1 -z 1 -W 2 -f "FHC_nue_H_CC/FinalEvents_*.dat"\
        -u -12 -a 12 -z 6 -W 12 -f "FHC_nuebar_C_CC/FinalEvents_*.dat" \
          -a 1 -z 1 -W 2 -f "FHC_nuebar_H_CC/FinalEvents_*.dat"\
       \
        -N -u 14 -a 12 -z 6 -W 12 -f "FHC_numu_C_NC/FinalEvents_*.dat" \
          -a 1 -z 1 -W 2 -f "FHC_numu_H_NC/FinalEvents_*.dat" \
        -N -u -14 -a 12 -z 6 -W 12 -f "FHC_numubar_C_NC/FinalEvents_*.dat" \
          -a 1 -z 1 -W 2 -f "FHC_numubar_H_NC/FinalEvents_*.dat"\
        -N -u 12 -a 12 -z 6 -W 12 -f "FHC_nue_C_NC/FinalEvents_*.dat" \
          -a 1 -z 1 -W 2 -f "FHC_nue_H_NC/FinalEvents_*.dat"\
        -N -u -12 -a 12 -z 6 -W 12 -f "FHC_nuebar_C_NC/FinalEvents_*.dat" \
          -a 1 -z 1 -W 2 -f "FHC_nuebar_H_NC/FinalEvents_*.dat" \
        -R i14 -o numu_CH2_CC.stdhep.root


  For many applications this full example would be entirely uneccessary.

# GiBUUFluxTools Command Line Interface

  This tool prepares input flux files in either text or ROOT format for use by
  GiBUU. GiBUU expects to read two column text histograms laid out as:
  `<bin center> <bin content>`.

    * `(-h|--help)`
    * `(-l|--low-bin-edge) <int> [required for -t]`: Low bin edge column number, zero indexed.
    * `(-u|--upper-bin-edge) <int> [optional for -t]`: Upper bin edge column number, zero indexed.
    * `(-v|--value) <int>`: Value column number, zero indexed.
    * `(-t|--input-file-text) <file path>`: Input text file name.
    * `(-r|--input-file-ROOT) <file path>`: Input ROOT file name.
    * `(-H|--input-ROOT-histogram) <string> [required for -r]`: Input ROOT histogram name.
    * `(-o|--output-file) <file path> [required]`: Output file name.
    * `(-k|--keep-norm)`: Whether to keep the input normalisation, GiBUU ignores the normalisation but can be useful to remember the normalisation.

  When reading from an input bin-edge defined text histogram, `-l` must be
  specified, however `-u` is optional as the upper edge will default to the low
  edge of the following bin. For the upper edge of the final bin, if `-u` is
  unspecified, the bin width is assumed to be the same as the previous bin.
