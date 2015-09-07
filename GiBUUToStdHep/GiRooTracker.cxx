#include "GiBUUToStdHep_Utils.hxx"

#include "GiRooTracker.hxx"

GiRooTracker::GiRooTracker(){
  StdHepPdg = new Int_t[kGiStdHepNPmax];
  StdHepStatus = new Int_t[kGiStdHepNPmax];
  Reset();
}

GiRooTracker::~GiRooTracker(){
  if(StdHepPdg != nullptr){ delete StdHepPdg; };
  if(StdHepStatus != nullptr){ delete StdHepStatus; };
}

void GiRooTracker::Reset(){
  GiBUU2NeutCode = 0;
  EvtNum = 0;
  StdHepN = 0;

  GiBUUUtils::ClearPointer(StdHepPdg,kGiStdHepNPmax);
  GiBUUUtils::ClearPointer(StdHepStatus,kGiStdHepNPmax);
  GiBUUUtils::ClearArray2D(StdHepP4);
}

void GiRooTracker::AddBranches(TTree* &tree){

  tree->Branch("GiBUU2NeutCode", &GiBUU2NeutCode, "GiBUU2NeutCode/I");

  tree->Branch("EvtNum", &EvtNum,"EvtNum/I");

  tree->Branch("StdHepN", &StdHepN,"StdHepN/I");

  tree->Branch("StdHepPdg", StdHepPdg,"StdHepPdg[StdHepN]/I");

  tree->Branch("StdHepStatus", StdHepStatus,"StdHepStatus[StdHepN]/I");

  static std::string GiStdHepNPmaxstr = GiBUUUtils::int2str(kGiStdHepNPmax);

  tree->Branch("StdHepP4", StdHepP4,
    ("StdHepP4["+GiStdHepNPmaxstr+"][4]/D").c_str());
}
