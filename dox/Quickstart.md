# My First GiBUU Prediction

Here should be the fastest way to make a GiBUU prediction.

## Build

  - Make a build directory: `mkdir build && cd build`
  - Configure the build: `cmake ../ -DUSE_GIBUU=1`
  - Build! `make`.
  - Install: `make install`.

## Source the environment:

  - `source build/Linux/setup.sh`
    - `Linux` will change depending on your `CMAKE_SYSTEM_NAME`

## Generate some events

  - `$ Generate_MiniBooNE_numuCC_CH2_Events_GIBUU`
  - Make a coffee, or three.

  If you want to up the stats, pass an integer as an argument to the
  `Generate_MiniBooNE_numuCC_CH2_Events_GIBUU`,
  this will set `input:num_runs_SameEnergy`. This will increase the time to
  generate linearly with the number passed. Passing `10` will generate O(1.6E5)
  events and take about an hour on any semi-current CPU.

## Plot the muon momentum

    root -l MiniBooNE_CH2_numuCC.stdhep.root
    [root] giRooTracker->SetAlias("DeltaE","StdHepP4[0][3]-StdHepP4[2][3]");
    [root] giRooTracker->SetAlias("Delta3Mom0","StdHepP4[0][0]-StdHepP4[2][0]");
    [root] giRooTracker->SetAlias("Delta3Mom1","StdHepP4[0][1]-StdHepP4[2][1]");
    [root] giRooTracker->SetAlias("Delta3Mom2","StdHepP4[0][2]-StdHepP4[2][2]");
    [root] giRooTracker->SetAlias("nQ2","(Delta3Mom0*Delta3Mom0 + Delta3Mom1*Delta3Mom1 + Delta3Mom2*Delta3Mom2) - DeltaE*DeltaE");
    [root] TH1 *NQ2 = new TH1D("NQ2",";Q^{2} (GeV^{2});d#sigma/dQ^{2} (cm^{2} GeV^{-2})",20,0,2);
    [root] NQ2->Sumw2();
    // Weight histogram fills by the event weight and select only CCQE events.
    [root] giRooTracker->Draw("nQ2 >> NQ2","EvtWght*(GiBUU2NeutCode==1)");
    // Scale to a differential xsec per neutron
    [root] NQ2->Scale(14.E-38/6.,"width");
    [root] NQ2->Draw("E1");
    // Compare to data
    [root] TGraph comp("MiniBooNE_1DQ2_numu_CCQE.dat");
    [root] comp.Draw("L SAME");

  This is contained within a `cint` macro and can be executed as
  `root -l MiniBooNE_CH2_numuCC.stdhep.root ${GIBUUTOOLSROOT}/cint_macros/Plot_MiniBooNE_CH2_CCQE_Q2.C`

  ![MiniBooNE_quickstart_qe.png](MiniBooNE_quickstart_qe.png)
  @image latex MiniBooNE_quickstart_qe.png "Example comparison of generated GiBUU CCQE events to the MiniBooNE data: Solid line is interpolated data, points are the generated prediction" width=0.6\textwidth

  If the normalisation looks similar to the above plot, then you're doing
  something right, if it looks awful, or you can't see both series, then
  something has gone wrong somewhere. Please report this as an issue on the
  GiBUUTools repo!

  **For all your comparison needs, consider using
  [NUISANCE](nuisance.hepforge.org), a generic global xsec generator tuner and
  comparison synthesiser.**

## Look at other fluxes/target

  Other helper scripts that are provided:

  * `Generate_ND280_numuCC_CH_Events_GIBUU`
  * `Generate_MINERvA_numuCC_CH_Events_GIBUU`
  * `Generate_DUNE_numuCC_Ar_Events_GIBUU`
  * `Generate_NuMi_numuCC_Ar_Events_GIBUU`

## Cluster jobs

  To generate a serious amounts of events, using a batch system is advised.
  **Important:** Remeber to change the `SEED` for each job!

  There is a set of example scripts that live in ${GIBUUTOOLSROOT}/batchjobs.
  These are written to work with SGE qsub. Executing GenerateAll.sh will
  send off about 500 jobs, generating O(2 million) events for 5 fluxes, and at
  two settings for each flux. It will also hold queue the conversion to StdHep
  process, which will automatically run after the events have been generated.

  The Generate_Batch.sh script is a more generic script for submitting jobs to
  create a single event vector which contains interactions on some target,
  optionally with <N> hydrogen atoms (e.g. CH2-target at MiniBooNE), optionally
  also generating NC interactions, and optionally you can also include the
  wrong-sign background flux (numubar in numu beam and vice versa). Execute
  `${GIBUUTOOLS}/bacthjobs/Generate_Batch.sh -?` to see a list of options.

  If you environment for executing GiBUU requires any additional setup other
  than `${GIBUUTOOLS}/setup.sh`, then you will need to add your environment
  setup to ${GIBUUTOOLS}/batchjobs/RunGiBUUBatch.sh, this is the script that
  will be run on the compute nodes.
