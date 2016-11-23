#ifndef SEEN_GIROOTRACKER_HXX
#define SEEN_GIROOTRACKER_HXX

#include <string>

#include "TTree.h"

///\brief Struct describing TTree structure for the GiBUU rooTracker-like output
///.
struct GiRooTracker {
#ifndef CPP03COMPAT
  constexpr
#else
  const
#endif
      static int kStdHepIdxPx = 0;
#ifndef CPP03COMPAT
  constexpr
#else
  const
#endif
      static int kStdHepIdxPy = 1;
#ifndef CPP03COMPAT
  constexpr
#else
  const
#endif
      static int kStdHepIdxPz = 2;
#ifndef CPP03COMPAT
  constexpr
#else
  const
#endif
      static int kStdHepIdxE = 3;
#ifndef CPP03COMPAT
  constexpr
#else
  const
#endif
      static int kGiStdHepNPmax = 100;

  ///\brief Costructs a GiRooTracker with default values provided by
  /// GiRooTracker::Reset.
  ///
  /// Allocates GiRooTracker::StdHepPdg and GiRooTracker::StdHepStatus.
  GiRooTracker();
  ///\brief Free's owned heap space.
  ~GiRooTracker();

  ///\brief The NEUT interaction mode equivalent of the GiBUU interaction type.
  ///
  /// This is determined from the GiBUU interaction type by
  /// GiBUUUtils::GiBUU2NeutReacCode.
  ///\warning This is not a one-to-one mapping.
  Int_t GiBUU2NeutCode;

  ///\brief The GiBUU interaction type.
  Int_t GiBUUReactionCode;
  ///\brief The charge of the first particle produced in the neutrino
  ///interaction.
  ///
  /// Useful for determinining the charge of the resonance in resonant pion
  /// production.
  Int_t GiBUUPrimaryParticleCharge;

  ///\brief The event number from the input event vector.
  ///
  ///\note This will not be unique between GiBUU runs but should be in a
  /// single output file.
  Int_t EvtNum;

  ///\brief The number of StdHep particles in this event.
  Int_t StdHepN;

  ///\brief The PDG codes of particles in this event.
  ///
  /// This is determined from the GiBUU particle number by
  /// GiBUUUtils::GiBUUToPDG.
  ///\warning This is not a one-to-one mapping, e.g. resonances are not uniquely
  /// determined by the GiBUU scheme.
  Int_t* StdHepPdg;  //[StdHepN]

  ///\brief The StdHep Status of particles in this event.
  ///
  /// Status Codes in use:
  /// - 0: Initial state real particle.
  /// - 1: Final state real particle.
  Int_t* StdHepStatus;  //[StdHepN]

  ///\brief Four momentum for particles in this event.
  Double_t StdHepP4[kGiStdHepNPmax][4];

  ///\brief GiBUU history array, indices correspond to the StdHep arrays.
  Long_t* GiBHepHistory;  //[StdHepN]
#ifndef CPP03COMPAT
  Int_t* GiBHepFather;      //[StdHepN]
  Int_t* GiBHepMother;      //[StdHepN]
  Int_t* GiBHepGeneration;  //[StdHepN]
#endif

  ///\brief GiBUU reported event weight.
  /// Directly related to the cross section assuming this run is in isolation.
  Double_t GiBUUPerWeight;

  ///\brief An extra weight that needs to be applied due using multiple runs
  /// to enhance event statistics.
  Double_t NumRunsWeight;

  ///\brief an arbitrary weight that is passed in at parse time. Useful for
  /// combining molecular target constituents.
  Double_t FileExtraWeight;

  ///\brief The total XSec weighting that should be applied to this event.
  Double_t EvtWght;

  ///\brief Weighting which takes account of multiple input numu species.
  ///
  /// Defined such that W_numu + W_numubar = 1
  Double_t SpeciesWght_numu;
  ///\brief Weighting which takes account of multiple input nue species.
  ///
  /// Defined such that W_nue + W_nuebar = 1
  Double_t SpeciesWght_nue;
  ///\brief Weighting which takes account of multiple input neutrino species.
  ///
  /// Defined such that \Sum_species W_species = 1
  Double_t SpeciesWght;

  ///\brief Function to reset an instance of this class to its default state.
  ///
  /// Used between fillings to result any values to default.
  void Reset();

  ///\brief Will add the relevant output branches to a given TTree.
  void AddBranches(TTree*& tree, bool AddHistory = false,
                   bool AddProdCharge = false);
};
#endif
