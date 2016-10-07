#include <iostream>

#include "TFile.h"
#include "TLorentzVector.h"
#include "TTree.h"

void Select_CC1pip(char const *InpFileName, char const *OupFileName) {
  TFile *inpF = new TFile(InpFileName);

  if (!inpF || !inpF->IsOpen()) {
    std::cerr << "[ERROR]: Could not open input file: " << InpFileName
              << std::endl;
    exit(1);
  }

  TTree *stdhep = static_cast<TTree *>(inpF->Get("giRooTracker"));

  if (!stdhep) {
    std::cerr
        << "[ERROR]: Could not read TTree (\"giRooTracker\") from input file: "
        << InpFileName << std::endl;
    exit(1);
  }

  Double_t EvtWght;
  Int_t GiStdHepN;
  Int_t GiStdHepPdg[100];
  Int_t GiStdHepStatus[100];
  Double_t GiStdHepP4[100][4];

  stdhep->SetBranchAddress("EvtWght", &EvtWght);
  stdhep->SetBranchAddress("StdHepN", &GiStdHepN);
  stdhep->SetBranchAddress("StdHepPdg", GiStdHepPdg);
  stdhep->SetBranchAddress("StdHepP4", GiStdHepP4);
  stdhep->SetBranchAddress("StdHepStatus", GiStdHepStatus);

  TFile *oupF = new TFile(OupFileName, "RECREATE");
  TTree *oupT = new TTree("CC1PipQ2","");
  Float_t Q2;
  oupT->Branch("Q2",&Q2);
  oupT->Branch("EvtWght",&EvtWght);


  Long64_t nentries = stdhep->GetEntries();
  for (Long64_t evt = 0; evt < nentries; ++evt) {
    stdhep->GetEntry(evt);

    TLorentzVector pnu(0, 0, 0, 0);
    TLorentzVector pmu(0, 0, 0, 0);

    Int_t NPiPlus = 0;
    Int_t NOtherPi = 0;

    // Loop through particle stack
    for (Int_t prt = 0; prt < GiStdHepN; ++prt) {
      //Inital numu
      if ((GiStdHepStatus[prt] == 0) && (GiStdHepPdg[prt] == 14)) {
        pnu = TLorentzVector(GiStdHepP4[prt][0], GiStdHepP4[prt][1],
                             GiStdHepP4[prt][2], GiStdHepP4[prt][3]);
      }
      //Final mu
      if ((GiStdHepStatus[prt] == 1) && (GiStdHepPdg[prt] == 13)) {
        pmu = TLorentzVector(GiStdHepP4[prt][0], GiStdHepP4[prt][1],
                             GiStdHepP4[prt][2], GiStdHepP4[prt][3]);
      }
      //Final pi+
      if ((GiStdHepStatus[prt] == 1) && (GiStdHepPdg[prt] == 211)) {
        NPiPlus++;
      }
      //Final other pi
      if ((GiStdHepStatus[prt] == 1) &&
          ((GiStdHepPdg[prt] == 111) || (GiStdHepPdg[prt] == -211))) {
        NOtherPi++;
      }
    }
    //cc1pip selection
    if((NPiPlus == 1) && (NOtherPi == 0) && (pnu.Vect().Mag2() > 0)&& (pmu.Vect().Mag2() > 0)){
      Q2 = -1.*(pnu - pmu).Mag2();
      oupT->Fill();
    }
  }
  oupT->Write();
  oupF->Write();
  oupF->Save();
}
