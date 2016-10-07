{
  giRooTracker->SetAlias("DeltaE", "StdHepP4[0][3]-StdHepP4[2][3]");
  giRooTracker->SetAlias("Delta3Mom0", "StdHepP4[0][0]-StdHepP4[2][0]");
  giRooTracker->SetAlias("Delta3Mom1", "StdHepP4[0][1]-StdHepP4[2][1]");
  giRooTracker->SetAlias("Delta3Mom2", "StdHepP4[0][2]-StdHepP4[2][2]");
  giRooTracker->SetAlias("nQ2",
                         "(Delta3Mom0*Delta3Mom0 + Delta3Mom1*Delta3Mom1 "
                         "+Delta3Mom2*Delta3Mom2) - DeltaE*DeltaE");
  TH1 *NQ2 = new TH1D(
      "NQ2", ";Q^{2} (GeV^{2});d#sigma/dQ^{2} (cm^{2} GeV^{-2})", 20, 0, 2);
  NQ2->Sumw2();
  giRooTracker->Draw("nQ2 >> NQ2", "EvtWght*(GiBUU2NeutCode==1)");
  NQ2->Scale(14.E-38 / 6., "width");
  NQ2->Draw("E1");
  TGraph comp("MiniBooNE_1DQ2_numu_CCQE.dat");
  comp.Draw("L SAME");
}
