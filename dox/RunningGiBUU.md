# Running GiBUU

  Having sourced the installed setup script (`/<install path>/setup.sh`), check
  that you can see GiBUU by executing: `which GiBUU.x`. If you can't, you
  probably haven't built, configured with `-DUSE_GIBUU=1`, or sourced the setup
  script correctly.

  Some jobcard options are explained below. To run a simulation from
  a given jobcard, execute: `GiBUU.x < my.job`. This will output O(100) files,
  so it is probably worthwhile running it in a subdirectory of anywhere you
  care about. The main file that we will use is `FinalEvents.dat`, which is the
  ASCII event vector. If this was not produced when running, check that
  `neutrinoAnalysis:outputEvents=.true.` is set in the jobcard.

# Useful jobcard options

  Options specified throughout take the form:
  `NamelistName:ParameterName = ParameterValue`. In the jobcard these will look
  like:

      &input
        numEnsembles=4000 ! Number of nuclei to simultaneously model per run.
        eventtype=5       ! Neutrino induced event
        numTimeSteps=150  ! Number of hadronic transport steps to simulate
        !...
      /

  where "`input`" is the `NamelistName`, and `numEnsembles` is a parameter
  within that namelist.

  **Note:** You must set `input:buuinput` in each jobcard. You can make a
  symlink in the current working directory to the downloaded inputs
  folder by executing `MakeBUUInputSoftlink`.

  **Note:** The character string that `input:buuinput` is read into is quite
  limited in length (O(100) characters), if the local path is long, consider
  using a symlink to make shorter, or edit the code to make the variable longer.
  It can comprehend relative paths so `input:buuinput='./BUUInput'` is an
  acceptable value.

  GiBUU was not designed as a neutrino event generator and thus has a significant
  number of options that you are unlikely to want to play with from a neutrino
  interaction physics standpoint. A good base jobcard can be dumped to the
  current directory by using the command `GiBUUInitNeutrinoJobCard`. More
  default jobcards can be found in `<install path>/jobcards`.

## Interaction physics options:

### Target nuclei and Nucleon momentum distributions

  The nuclear momentum distribution model used is a local Thomas-Fermi gas and
  it's implementation is described in
  [Phys. Rev. C 79 034601](http://journals.aps.org/prc/pdf/10.1103/PhysRevC.79.034601) Section III.A.
  For some nuclei, explicit density parameterisation and experimental tunings from
  [Nuclear Physics A 554 4 509 (1993)](http://www.sciencedirect.com/science/article/pii/037594749390245S)
  and [Atomic Data and Nuclear Data Tables 14 5 479 (1974)](http://www.sciencedirect.com/science/article/pii/S0092640X74800021)
  can be used (`target:densitySwitch_Static=2`). However, for some nuclei the
  specific data is not available, and thus
  `target:densitySwitch_Static=3` should be used --- notably to simulate on
  Ar40 you must use the Wood-Saxon density.

      &target ! e.g. A Carbon-target set up
          target_Z=6
          target_A=12
          densitySwitch_Static=2 ! 0: density=0.0
                                 ! 1: Wood-Saxon by Lenske
                                 ! 2 : NPA 554
                                 ! 3: Wood-Saxon by Lenske, different neutron
                                 !    and proton radii
                                 ! 5: density distribution is a sphere with
                                 !    density according to the input value of
                                 !    "fermiMomentum_input".
          fermiMomentum_input=0.225 ! Only relevant for "densitySwitch_Static=5"
          fermiMotion=.false.
          ReAdjustForConstBinding=.false.
      /

  For much much more detail see
  [here](https://gibuu.hepforge.org/trac/wiki/physicsInput#GiBUUmodel),
  references therein and [here](http://www.sciencedirect.com/science/article/pii/S0370157311003619).

### QE

  Quasi-Elastic interactions are modelled similarly to other generators, and
  by default use form factors from BBBA07 and an MA of 1.03.

      &ff_QE
        parametrization=3        ! 1=BBBA03, 2=BBBA05, 3=BBBA07
        useNonStandardMA=.false. ! if true, use value of MA_in for axial mass MA,
                                 ! if false, use best fit
        ! MA_in=1.03
      /

  The implementation is described in detail in
  [Phys. Rev. C 73 065502 (2006)](http://journals.aps.org/prc/abstract/10.1103/PhysRevC.73.065502).

### 2p2h

  The 2p2h model used in GiBUU2016 is the Christy model () and is tuned only to
  electron scattering data. The implementation is described in detail in
  [Phys. Rev. C 94 035502](http://journals.aps.org/prc/abstract/10.1103/PhysRevC.94.035502)
  and is accompanied by numerous model comparisons to recent experimental data.

      &lepton2p2h
        ME_Version=4 ! This is the Christy model
        T=1
      /

  Note, in (Phys. Rev. C 94 035502), `T=1` (target isospin) is used and it is
  likely that it should be used for your comparisons.

### Pion production

  **Model**: Nuclear resonance-induced single pion production with tuned,
  non-resonant 1- and 2-pi background contributions. By default, the resonance
  matrix elements are calculated from the MAID model
  (Eur. Phys. J. A 4 1 69 (2007)). It is also possible to use Rein-Sehgal,
  or just a simple Delta resonance.

  The overall parameter for controlling the resonant pion production model is
  `neutrino_matrixelement:which_resonanceModel`. The options are shown below.

      &neutrino_matrixelement
        which_resonanceModel=0    !0=QE + matrixelements from MAID,
                                  !1=QE matrixelements + old Delta,
                                  !2=Rein-Sehgal
      /

  The model has been tuned to both ANL and BNL bubble chamber data.
  Both the resonant and non-resonant contributions should be altered depending
  on which tuning is in use.

  For ANL fit:


      &input_FF_ResProd
        FF_ResProd=0  ! 0=MAID in CM-frame
        MA=0.95       ! Axial mass in the Delta resonance form factor
      /
      &neutrino_MAIDlikeBG ! Non-resonant background contribution
        b_proton_pinull=3.0
        b_neutron_piplus=1.5
      /

  For BNL fit:

      &input_FF_ResProd
        FF_ResProd=0 ! 0=MAID in CM-frame
        MA=1.3       ! Axial mass in the Delta resonance form factor
      /
      &neutrino_MAIDlikeBG ! Non-resonant background contribution
        b_proton_pinull=6.0
        b_neutron_piplus=3.0
      /


  Extensive comparisons to MiniBooNE data can be found in
  [Phys. Rev. C 87 014602](http://journals.aps.org/prc/pdf/10.1103/PhysRevC.87.014602).
  The systematic uncertainty on the GiBUU model is
  generally taken as the difference between predictions using the ANL and BNL
  fit, as in (Phys. Rev. C 87 014602).

  The implementation is discussed in
  [Phys. Rev. D 82 093001](http://journals.aps.org/prd/pdf/10.1103/PhysRevD.82.093001).

## Got your own flux?

  GiBUU includes quite a number of pre-defined fluxes that can be enabled with
  the `neutrino_induced:nuExp` entry in the jobcard. The default jobcard is well
  documented with regard to which values correspond to which flux.

  However, sometimes you might want to use your own flux.
  By default, GiBUU does not expose simple options for reading arbitrary fluxes.
  The version that can be built here (using cmake option `-DUSE_GIBUU=1`) is
  patched so that arbitrary fluxes can be specified in the jobcard. To do so
  you will want to set `neutrino_induced:nuExp = -1` and then
  `neutrino_induced:InputFluxFileName='/path/to/flux/file.txt'`.

  **Note:** The character string that this is read into is quite limited in
  length (O(100) characters), if the local path is longer, consider using a
  symlink to make shorter, or edit the code to make the variable longer.

  **Note:** So as not to collider with the internal usage of the flux reading
  method you must specify the value of `neutrino_induced:InputFluxFileName` as
  either a fully qualified path, or a relative path beginning with `./`.

  The fluxes are simple two column, space delimited tables of flux shape.
  Note that the normalisation is entirely ignored. The first column is taken
  as bin centre values, and the second as the flux content in that bin. Consider
  using the packaged `GiBUUFluxTools` helper to convert from bin-edge text
  histograms or ROOT files containing histograms to GiBUU friendly flux inputs.

## Want to run without FSI

  The simplest way to 'run without FSI' is to set the number of hadronic
  transport steps to 0 (`input:numTimeSteps=0`). This will result in an
  'inclusive' analysis of the uncoupled neutrino interaction model. However, as
  the final state particles will not have been transported through the nuclear
  medium they will still feel the potential of of the nucleus and their
  kinematics will not be free. When examining the kinematics of such particles,
  be aware that they are likely still off-mass-shell. This can still be a useful
  way to produce plots with that caveat.

## Want to run on free nucleons?

  To generate events on hydrogen, either for comparisons to bubble chamber data,
  or for building composite targets such as CH, the following options should be
  used:


      &input
            numTimeSteps=0
      /
      &target
            target_Z=1
            target_A=1
            densitySwitch_Static=2
            fermiMotion=.false.
            ReAdjustForConstBinding=.false.
      /
      &width_Baryon
          mediumSwitch=.false.  ! if .false. vacuum widths will be used for all
                                ! resonances
      /


  As the majority of the time-consuming calculation is running the hadronic
  transport simulation, this will execute very quickly.

## How many events to generate

  Unlike for other generators, which do not have coupled initial and final
  states, before the simulation is run it is not possible to exactly determine
  all allowed phase spaces. This means that if you want 1E6 events, it is
  impossible to ask the generator to generate exactly this many events. The
  upper-limit of the number of events generated is given by:
  `target_A * numEnsembles * num_runs_SameEnergy`. To determine the number of
  events you should execute a short run with the proposed numbers for
  `target_A` and `numEnsembles`... and then scale up statistics by increasing
  the value `num_runs_SameEnergy`. This should give a predicatble number of
  events (though still not exact).

  Modifying `input:numEnsembles` sets the number of nuclei to simultaneously
  simulate. Increasing it, improves the efficiency of the simulation, but
  will increase the memory usage. `input:numEnsembles=4000` is a good start for
  a carbon target, but can be decreased for heavier nuclei.

## How long to run the simulation for

  The number of simulation steps is set by the jobcard. If particles are still
  inside the nucleus at the end of the simulation, their kinematics will show
  them as off-mass-shell. A good starting point for a carbon-target is to set
  `input:numTimeSteps=150`, larger nuclear targets will require more simulation
  steps. The suggested value is such that
  `input:numTimeSteps*input:delta_T (fm)` should significantly exceed the radius
  of the target nucleus. You can also check the number of particles still inside
  of the nucleus at the end of the simulation by looking at the output.

      ################### NEUTRINO ANALYSIS FINISHED #######################
      (FINAL) number of counted Events:
      Real         -- all, 1-body, 2-body, 3-body:        0        0        0        0
      Perturbative -- all, 1-body, 2-body, 3-body:    30529     4890    24867      772

  The numbers shown here are the number of interactions still taking place at
  the end of the simulation. These numbers should decrease when increasing
  `input:numTimeSteps`, for a given targer. Ideally these would all be 0.

## Output histograms

  It appears that often the GiBUU developers code their analyses into the
  simulation itself, as a result many in-built analyses can be run at the same
  time as generating the events. For a number of uses, these results are
  actually all the information you might need. The options are contained
  within the namelists `neutrinoAnalysis` and `nl_specificEvent`. Turning some
  of these analyses on will significantly increase the number of output files
  that GiBUU produces.

## FAQs

  **Why are lots of my final state particles off shell?** GiBUU will transport
  the final states of any neutrino interactions through the hadronic transport
  simulation for an exact number of steps. If some of these particles, after the
  specified number of steps, are still inside the nucleus, they will feel the
  nuclear potential and thus be off-mass-shell. By plotting the calculated mass
  of certain classes of final state particles you should be able to gauge how
  much of a problem this is for you events. If it appears to be a significant
  problem, then you should up the value of `input:numTimeSteps`. O(150) appears
  to be okay for Carbon-targets.

  **Why does GiBUU fail to start when given a flux file with more than 120
  bins?** Because FORTRAN. The flux histogram is read into a compile-time
  allocated array. You have three choices: 1 -- Chop off a few bins from the
  tail of your flux, 2 -- Rebin your flux, 3 -- Edit the size of the array in
  GiBUU and recompile.

## Where do I go for more info?

  The full GiBUU documentation is on hepforge:
  [GiBUU hepforge](https://gibuu.hepforge.org/trac/wiki).
  The GiBUU developer mailing list will often offer specifc help -- they
  want their wonderful code to be used for good physics! Please do not spam
  this address before searching for the answer in papers and documentation.
