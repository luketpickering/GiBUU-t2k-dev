#include "PureGenUtils.hxx"

#include "GiRooTracker.hxx"

GiRooTracker::GiRooTracker(){
  StdHepPdg = new Int_t[kGiStdHepNPmax];
  StdHepStatus = new Int_t[kGiStdHepNPmax];
  GiBHepHistory = new Long_t[kGiStdHepNPmax];
  Reset();
}

GiRooTracker::~GiRooTracker(){
  if(StdHepPdg != nullptr){ delete StdHepPdg; };
  if(StdHepPdg != nullptr){ delete GiBHepHistory; };
  if(StdHepStatus != nullptr){ delete StdHepStatus; };
}

void GiRooTracker::Reset(){
  GiBUU2NeutCode = 0;
  EvtNum = 0;
  StdHepN = 0;

  PGUtils::ClearPointer(StdHepPdg,kGiStdHepNPmax);
  PGUtils::ClearPointer(StdHepStatus,kGiStdHepNPmax);
  PGUtils::ClearPointer(GiBHepHistory,kGiStdHepNPmax);
  PGUtils::ClearArray2D(StdHepP4);
}

void GiRooTracker::AddBranches(TTree* &tree){

  tree->Branch("GiBUU2NeutCode", &GiBUU2NeutCode, "GiBUU2NeutCode/I");

  tree->Branch("EvtNum", &EvtNum,"EvtNum/I");

  tree->Branch("StdHepN", &StdHepN,"StdHepN/I");

  tree->Branch("StdHepPdg", StdHepPdg,"StdHepPdg[StdHepN]/I");

  tree->Branch("StdHepStatus", StdHepStatus,"StdHepStatus[StdHepN]/I");
  tree->Branch("GiBHepHistory", GiBHepHistory,"GiBHepHistory[StdHepN]/L");

  static std::string GiStdHepNPmaxstr = PGUtils::int2str(kGiStdHepNPmax);

  tree->Branch("StdHepP4", StdHepP4,
    ("StdHepP4["+GiStdHepNPmaxstr+"][4]/D").c_str());
}
