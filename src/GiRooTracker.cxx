#include "LUtils/Utils.hxx"

#include "GiRooTracker.hxx"

GiRooTracker::GiRooTracker() {
  StdHepPdg = new Int_t[kGiStdHepNPmax];
  StdHepStatus = new Int_t[kGiStdHepNPmax];
  GiBHepHistory = new Long_t[kGiStdHepNPmax];
#ifndef CPP03COMPAT
  GiBHepFather = new Int_t[kGiStdHepNPmax];
  GiBHepMother = new Int_t[kGiStdHepNPmax];
  GiBHepGeneration = new Int_t[kGiStdHepNPmax];
#endif
  Reset();
}

GiRooTracker::~GiRooTracker() {
  delete StdHepPdg;
  delete StdHepStatus;
  delete GiBHepHistory;
#ifndef CPP03COMPAT
  delete GiBHepFather;
  delete GiBHepMother;
  delete GiBHepGeneration;
#endif
}

void GiRooTracker::Reset() {
  GiBUU2NeutCode = 0;
  GiBUUReactionCode = 0;
  GiBUUPrimaryParticleCharge = 0;
  EvtNum = 0;
  StdHepN = 0;
  GiBUUPerWeight = 1.0;

  SpeciesWght_numu = 0.0;
  SpeciesWght_nue = 0.0;
  SpeciesWght = 0.0;

  Utils::ClearPointer(StdHepPdg, kGiStdHepNPmax);
  Utils::ClearPointer(StdHepStatus, kGiStdHepNPmax);
  Utils::ClearPointer(GiBHepHistory, kGiStdHepNPmax);
#ifndef CPP03COMPAT
  Utils::ClearPointer(GiBHepFather, kGiStdHepNPmax);
  Utils::ClearPointer(GiBHepMother, kGiStdHepNPmax);
  Utils::ClearPointer(GiBHepGeneration, kGiStdHepNPmax);
#endif
  Utils::ClearArray2D(StdHepP4);
}

void GiRooTracker::AddBranches(TTree*& tree, bool AddHistory,
                               bool AddProdCharge, bool IsElectronScattering) {
  tree->Branch("GiBUU2NeutCode", &GiBUU2NeutCode, "GiBUU2NeutCode/I");
  tree->Branch("GiBUUReactionCode", &GiBUUReactionCode, "GiBUUReactionCode/I");

  tree->Branch("EvtNum", &EvtNum, "EvtNum/I");

  tree->Branch("StdHepN", &StdHepN, "StdHepN/I");

  tree->Branch("StdHepPdg", StdHepPdg, "StdHepPdg[StdHepN]/I");

  tree->Branch("StdHepStatus", StdHepStatus, "StdHepStatus[StdHepN]/I");
  tree->Branch("GiBUUPerWeight", &GiBUUPerWeight, "GiBUUPerWeight/D");
  tree->Branch("NumRunsWeight", &NumRunsWeight, "NumRunsWeight/D");
  tree->Branch("FileExtraWeight", &FileExtraWeight, "FileExtraWeight/D");
  tree->Branch("EvtWght", &EvtWght, "EvtWght/D");
  if (!IsElectronScattering) {
    tree->Branch("SpeciesWght_numu", &SpeciesWght_numu, "SpeciesWght_numu/D");
    tree->Branch("SpeciesWght_nue", &SpeciesWght_nue, "SpeciesWght_nue/D");
    tree->Branch("SpeciesWght", &SpeciesWght, "SpeciesWght/D");
  }

  if (AddHistory) {
    tree->Branch("GiBHepHistory", GiBHepHistory, "GiBHepHistory[StdHepN]/L");
#ifndef CPP03COMPAT
    tree->Branch("GiBHepFather", GiBHepFather, "GiBHepFather[StdHepN]/I");
    tree->Branch("GiBHepMother", GiBHepMother, "GiBHepMother[StdHepN]/I");
    tree->Branch("GiBHepGeneration", GiBHepGeneration,
                 "GiBHepGeneration[StdHepN]/I");
#endif
  }
  if (AddProdCharge) {
    tree->Branch("GiBUUPrimaryParticleCharge", &GiBUUPrimaryParticleCharge,
                 "GiBUUPrimaryParticleCharge/I");
  }
  static std::string GiStdHepNPmaxstr = Utils::int2str(kGiStdHepNPmax);

  tree->Branch("StdHepP4", StdHepP4,
               ("StdHepP4[" + GiStdHepNPmaxstr + "][4]/D").c_str());
}
