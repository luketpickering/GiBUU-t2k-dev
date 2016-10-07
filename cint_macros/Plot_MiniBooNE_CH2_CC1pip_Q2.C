{
  TH1 *Q2 = new TH1D("Q2", ";Q^{2} (MeV^{2});d#sigma/dQ^{2} (cm^{2} MeV^{-2})",
                     20, 0, 2);
  Q2->Sumw2();
  CC1PipQ2->Draw("Q2 >> Q2", "EvtWght");
  Q2->Scale(1E-6);
  Q2->Scale(14.E-38, "width");
  Q2->GetYaxis()->SetRangeUser(0,60E-45);
  Q2->Draw("E1");
  TGraph comp("MiniBooNE_1DQ2_numu_CC1pip.dat");
  comp.Draw("L SAME");
}
