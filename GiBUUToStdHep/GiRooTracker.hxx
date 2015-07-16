#ifndef SEEN_GIROOTRACKER_HXX
#define SEEN_GIROOTRACKER_HXX

#include <string>

#include "TTree.h"

struct GiRooTracker {
  constexpr static int kStdHepIdxPx = 0;
  constexpr static int kStdHepIdxPy = 1;
  constexpr static int kStdHepIdxPz = 2;
  constexpr static int kStdHepIdxE = 3;
  constexpr static int kGiStdHepNPmax = 100;
  std::string GiStdHepNPmaxstr;

  GiRooTracker();

  Int_t GiBUU2NeutCode;

  Int_t EvtNum;

  Int_t StdHepN;

  Int_t* StdHepPdg; //[StdHepN]

  Int_t* StdHepStatus; //[StdHepN]

  Double_t StdHepP4 [kGiStdHepNPmax][4];

  void Reset();

  void AddBranches(TTree* &tree);

};
#endif
