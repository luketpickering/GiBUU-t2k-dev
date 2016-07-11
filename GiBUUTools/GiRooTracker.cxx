#include "LUtils/Utils.hxx"

#include "GiRooTracker.hxx"

GiRooTracker::GiRooTracker() {
  EvtCode = new TObjString("");
  StdHepPdg = new Int_t[kGiStdHepNPmax];
  StdHepStatus = new Int_t[kGiStdHepNPmax];
  GiBHepHistory = new Long_t[kGiStdHepNPmax];
  GiBHepFather = new Int_t[kGiStdHepNPmax];
  GiBHepMother = new Int_t[kGiStdHepNPmax];
  GiBHepGeneration = new Int_t[kGiStdHepNPmax];

  Reset();
}

GiRooTracker::~GiRooTracker() {
  if (EvtCode) {
    delete EvtCode;
  }
  if (StdHepPdg != nullptr) {
    delete StdHepPdg;
  };
  if (StdHepPdg != nullptr) {
    delete GiBHepHistory;
  };
  if (StdHepStatus != nullptr) {
    delete StdHepStatus;
  };
  if (GiBHepFather != nullptr) {
    delete GiBHepFather;
  };
  if (GiBHepMother != nullptr) {
    delete GiBHepMother;
  };
  if (GiBHepGeneration != nullptr) {
    delete GiBHepGeneration;
  };
}

void GiRooTracker::Reset() {
  (*EvtCode) = "";
  GiBUU2NeutCode = 0;
  GiBUUReactionCode = 0;
  GiBUUPrimaryParticleCharge = 0;
  EvtNum = 0;
  StdHepN = 0;
  GiBUUPerWeight = 1.0;
  StruckNucleonPDG = 0;

  Utils::ClearPointer(StdHepPdg, kGiStdHepNPmax);
  Utils::ClearPointer(StdHepStatus, kGiStdHepNPmax);
  Utils::ClearPointer(GiBHepHistory, kGiStdHepNPmax);
  Utils::ClearPointer(GiBHepFather, kGiStdHepNPmax);
  Utils::ClearPointer(GiBHepMother, kGiStdHepNPmax);
  Utils::ClearPointer(GiBHepGeneration, kGiStdHepNPmax);
  Utils::ClearArray2D(StdHepP4);
}

void GiRooTracker::AddBranches(TTree*& tree, bool AddHistory,
                               bool AddStruckNucleonPDG, bool EmulateNuWro,
                               bool AddProdCharge) {
  tree->Branch("GiBUU2NeutCode", &GiBUU2NeutCode, "GiBUU2NeutCode/I");
  tree->Branch("GiBUUReactionCode", &GiBUUReactionCode, "GiBUUReactionCode/I");

  if (EmulateNuWro) {
    tree->Branch("EvtCode", &EvtCode);
  }
  tree->Branch("EvtNum", &EvtNum, "EvtNum/I");

  tree->Branch("StdHepN", &StdHepN, "StdHepN/I");

  tree->Branch("StdHepPdg", StdHepPdg, "StdHepPdg[StdHepN]/I");

  tree->Branch("StdHepStatus", StdHepStatus, "StdHepStatus[StdHepN]/I");
  tree->Branch("GiBUUPerWeight", &GiBUUPerWeight, "GiBUUPerWeight/D");
  tree->Branch("NumRunsWeight", &NumRunsWeight, "NumRunsWeight/D");
  tree->Branch("ExtraWeight", &ExtraWeight, "ExtraWeight/D");
  tree->Branch("EvtWght", &EvtWght, "EvtWght/D");

  if (AddHistory) {
    tree->Branch("GiBHepHistory", GiBHepHistory, "GiBHepHistory[StdHepN]/L");
    tree->Branch("GiBHepFather", GiBHepFather, "GiBHepFather[StdHepN]/I");
    tree->Branch("GiBHepMother", GiBHepMother, "GiBHepMother[StdHepN]/I");
    tree->Branch("GiBHepGeneration", GiBHepGeneration,
                 "GiBHepGeneration[StdHepN]/I");
  }
  if (AddStruckNucleonPDG) {
    tree->Branch("StruckNucleonPDG", &StruckNucleonPDG, "StruckNucleonPDG/I");
  }
  if (AddProdCharge) {
    tree->Branch("GiBUUPrimaryParticleCharge", &GiBUUPrimaryParticleCharge,
                 "GiBUUPrimaryParticleCharge/I");
  }
  static std::string GiStdHepNPmaxstr = Utils::int2str(kGiStdHepNPmax);

  tree->Branch("StdHepP4", StdHepP4,
               ("StdHepP4[" + GiStdHepNPmaxstr + "][4]/D").c_str());
}