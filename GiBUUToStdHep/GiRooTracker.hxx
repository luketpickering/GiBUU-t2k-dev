#ifndef SEEN_GIROOTRACKER_HXX
#define SEEN_GIROOTRACKER_HXX

#include <string>

#include "TTree.h"

///\brief Struct describing TTree structure for the GiBUU rooTracker-like output
///.
struct GiRooTracker {
  constexpr static int kStdHepIdxPx = 0;
  constexpr static int kStdHepIdxPy = 1;
  constexpr static int kStdHepIdxPz = 2;
  constexpr static int kStdHepIdxE = 3;
  constexpr static int kGiStdHepNPmax = 100;

  ///\brief Costructs a GiRooTracker with default values provided by
  ///GiRooTracker::Reset.
  ///
  ///Allocates GiRooTracker::StdHepPdg and GiRooTracker::StdHepStatus.
  GiRooTracker();
  ///\brief Free's owned heap space.
  ~GiRooTracker();

  ///\brief The NEUT interaction mode equivalent of the GiBUU interaction type.
  ///
  ///This is determined from the GiBUU interaction type by
  ///GiBUUUtils::GiBUU2NeutReacCode.
  ///\warning This is not a one-to-one mapping.
  Int_t GiBUU2NeutCode;

  ///\brief The event number from the input event vector.
  ///
  ///\note This will not be unique between GiBUU runs but should be in a
  ///single output file.
  Int_t EvtNum;

  ///\brief The number of StdHep particles in this event.
  Int_t StdHepN;

  ///\brief The PDG codes of particles in this event.
  ///
  ///This is determined from the GiBUU particle number by
  ///GiBUUUtils::GiBUUToPDG.
  ///\warning This is not a one-to-one mapping, e.g. resonances are not uniquely
  ///determined by the GiBUU scheme.
  Int_t* StdHepPdg; //[StdHepN]

  ///\brief The StdHep Status of particles in this event.
  ///
  /// Status Codes in use:
  /// - -1: Initial state real particle.
  /// - 1: Final state real particle.
  Int_t* StdHepStatus; //[StdHepN]

  ///\brief Four momentum for particles in this event.
  Double_t StdHepP4 [kGiStdHepNPmax][4];

  ///\brief GiBUU history array, indices correspond to the StdHep arrays.
  Long_t* GiBHepHistory; //[StdHepN]

  ///\brief Function to reset an instance of this class to its default state.
  ///
  ///Used between fillings to result any values to default.
  void Reset();

  ///\brief Will add the relevant output branches to a given TTree.
  void AddBranches(TTree* &tree);

};
#endif
