# Further analysis of stdhep events

## Plotting cross sections

  As shown in the Quickstart guide, histograms of event properties must be
  weighted by each event's `EvtWght`. This weighting does most of the leg-work
  in scaling the event-rate-like distribution to a flux-averaged cross section.
  Unlike many other generators, the flux averaging has already taken place as
  part of the simulation and subsequent processing, there is no need to scale
  by the number of generated events. For a more complete description of how the
  GiBUU event weights are calculated and used, see
  [here](https://gibuu.hepforge.org/trac/wiki/perWeight).

  The final step in producing a flux-averaged differential cross section from
  the event vector is to divide each bin by it's width. This will result in a
  binned cross section prediction in units of
  `10^-38 cm^2 nucleon^-1 <Property Units>^-1`.

  An example of this process is shown below:

  ![example_weighted_histo.pdf](example_weighted_histo.pdf)
  @image latex example_weighted_histo.pdf "Example effect of correctly weighting histogram fills with the EvtWght, and then dividing by binwidth to produce a differential cross section prediction in squared four momentum transfer." width=0.4\textwidth

## Running event selections

  Most analyses will want to post-process the particle stack, or make
  selections, or build composite properties. The reader is directed to the
  [NUISANCE](nuisance.hepforge.org) project and the 'GenericFluxTester' tool
  therein which makes writing such an analysis simple.

  As a rudimentary example of such post-processing, execute
  `$ Select_CC1pip_GiBUUStdHep <input_GiBUU.stdhep.root>` passing a produced
  GiBUU stdhep file. Herein, one produced by executing
  `$ Generate_MiniBooNE_numuCC_CH2_Events_GIBUU 5` will be used. This script
  will select CC numu events that contain a single positively charged pion and
  write out a TTree of the squared four momentum transfer to a file name
  `CC1pip_Q2_<input_GiBUU.root>`. To check that it looks sensible, we can
  compare to the MiniBooNE data (which is dumped to the current
  directory by the script if using a MiniBooNE event vector):

      root -l CC1pip_Q2_MiniBooNE_CH2_numuCC.root
      [root] TH1 *Q2 = new TH1D("Q2",";Q^{2} (MeV^{2});d#sigma/dQ^{2} (cm^{2} MeV^{-2})",20,0,2);
      [root] Q2->Sumw2();
      [root] CC1PipQ2->Draw("Q2 >> Q2","EvtWght");
      // Scale to MeV
      [root] Q2->Scale(1E-6);
      // Scale to a differential xsec per CH2
      [root] Q2->Scale(14.E-38,"width");
      [root] Q2->GetYaxis()->SetRangeUser(0,60E-45);
      [root] Q2->Draw("E1");
      [root] TGraph comp("MiniBooNE_1DQ2_numu_CC1pip.dat");
      [root] comp.Draw("L SAME");

  This is contained within a a `cint` macro and can be executed as
  `root -l CC1pip_Q2_MiniBooNE_CH2_numuCC.root ${GIBUUTOOLSROOT}/cint_macros/Plot_MiniBooNE_CH2_CC1pip_Q2.C`

  The macro used to run the selection and write the analysis tree will be dumped
  to the current directory when `Select_CC1pip_GiBUUStdHep` is run, but also
  lives in `${GIBUUTOOLSROOT}/cint_macros/Select_CC1pip.C`.

  **Note:** Before you get worried, most generators underestimate this dataset
  quite severly. By using the results of the BNL fit parameters for resonant and
  non-resonant pion production the difference in this dataset can be reduced.

  ![MiniBooNE_furtheranalysis_cc1pip.png](MiniBooNE_furtheranalysis_cc1pip.png)
  @image latex MiniBooNE_furtheranalysis_cc1pip.png "Example comparison of generated and selected GiBUU CC1pi+ events to the MiniBooNE data: Solid line is interpolated data, points are the generated prediction" width=0.6\textwidth
