{
  size_t NFluxes = 9;
  char const * Fluxes[] = { "MINERvA_numubar_fhc_2015",
    "MINERvA_numubar_rhc_2015",
    "MINERvA_numu_fhc_2015",
    "MINERvA_numu_rhc_2015",
    "MiniBooNE_NP_numub",
    "MiniBooNE_NP_numu",
    "MiniBooNE_PP_numu",
    "T2K_ND5_FHC_numub",
    "T2K_ND5_FHC_numu" };

  int const Wheel[] = {kBlack, kRed, kBlue, kGreen, kMagenta, kYellow-5, kRed-3, kBlue+3, kGray};
  int const Style[] = {2,1,1,2,1,2,1,2,1};

  for(size_t f = 0; f < NFluxes; ++f){
    TGraph *g = new TGraph((std::string(Fluxes[f]) + ".txt").c_str());
    
    double max = 0;
    int pi = 0;
    for(size_t i = 0; i < g->GetN(); ++i){ double x, y; g->GetPoint(i,x,y); if (y > max){ max = y; } if ((x > 15.) && (pi == 0)){ pi = x; } }
    for(size_t i = 0; i < g->GetN(); ++i){ double x, y; g->GetPoint(i,x,y); g->SetPoint(i,x,y/max); }

    if(pi){ g->Set(pi); }
    g->SetTitle(Fluxes[f]);
    g->SetLineColor(Wheel[f]);
    g->SetLineWidth(2);
    g->SetLineStyle(Style[f]);
    g->SetFillColor(0);
    g->Draw((f==0)?"AL":"L SAME");
  }

  c1->BuildLegend();

}
