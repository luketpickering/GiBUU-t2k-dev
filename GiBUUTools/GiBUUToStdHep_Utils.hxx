#ifndef SEEN_GIBUUToStdHep_UTILS_HXX
#define SEEN_GIBUUToStdHep_UTILS_HXX

#include <cstdlib>
#include <cerrno>
#include <climits>

#include <sstream>
#include <string>
#include <functional>

#include <vector>
#include <iostream>

#include "Rtypes.h"

///Utilities which may be helpful for processing GiBUU specific output.
namespace GiBUUUtils {

///\brief Converts a GiBUU particle code, with associated particle EM charge
///information to a PDG code.
///
/// From https://gibuu.hepforge.org/trac/wiki/ParticleIDs
///\note Returns 0 when encountering an unknown particle.
///Current codes converted:
/// - GiBUU : PDG
/// - 1 : p=2212, n=2112
/// - 2 : Delta++=2224, Delta+=2214, Delta0=2114, Delta−=1114
/// - 3 : P11(1440) PDGs: 202212, 202112
/// - 4 : S11(1535) PDGs : 102212, 102112
/// - 5 : S11(1650) PDGs : 122212, 122112
/// - 7 : D13(1520) PDGs: 102214, 102114
/// - 10 : D15(1675) PDGs : 102216, 102116
/// - 15: P13(1900) P13(1900) PDGs: n/a
/// - 16: F15(1680) PDGs: 202216, 202116
/// - 19 : S31(1620) PDGs: 112222, 112212, 112112, 111112
/// - 20 : S31(1900) PDGs: n/a
/// - 21 : D33(1700) PDGs: 122224, 122214, 122114, 121114
/// - 26 : P31(1910) PDGs: 222222, 222212, 222112, 221112
/// - 27 : P33(1600) PDGs: 202224, 202214, 202114, 201114
/// - 32 : Lambda PDG: 3122
/// - 33 : Sigma PDGs: 3222, 3212, 3112
/// - 101 : pi+=211, pi0=111, pi-=-211
/// - 103 : rho+=213, rho0=113, rho-=-213
/// - 104 : sigma=9000221
/// - 105 : omega=223
/// - 110 : K+=321, K0=311
/// - 111 : K-=-321, K0=-311
/// - 901 : 11
/// - 902 : 13
/// - 911 : 12
/// - 912 : 14
/// - 999 : 22
int GiBUUToPDG(int GiBUUCode, int GiBUUCharge=0);

std::tuple<Int_t,Int_t,Int_t> DecomposeGiBUUHistory(Long_t HistCode);
std::string WriteGiBUUHistory(Long_t HistCode);
///\brief Converts a GiBUU interaction code to the corresponding NEUT code
///where possible.
///
///Sometimes the NEUT code is dependent on the particle produced in the intial
///interaction.
///\note Current codes converted:
/// - 1 = QE
/// - 2-31 = res (specific number represents GiBUU particle id for resonance)
/// - 32,33 = 1pi
/// - 34 = DIS
/// - 35,36 = 2p2h
/// - 37 = 2pi
///
///From https://gibuu.hepforge.org/trac/wiki/LesHouches
int GiBUU2NeutReacCode(Int_t GiBUUCode,
  Int_t const * const StdHepPDGArray,
  Long_t const * const HistoryArray,
  Int_t StdHepN,
  bool IsCC=true,
  Int_t StruckNucleonPosition=-1,
  Int_t PrimaryProdCharge=-10);

}

#endif
