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
/// - 101 : pi+=211, pi0=111, pi-=-211
/// - 901 : 11
/// - 902 : 13
/// - 911 : 12
/// - 912 : 14
/// - 999 : 22
/// - 32 : 3122
/// - 33 : 3222, 3212, 3112
/// - 110 : K+=321, K0=311
int GiBUUToPDG(int GiBUUCode, int GiBUUCharge);
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
int GiBUU2NeutReacCode(int GiBUUCode, int PDG);

}

#endif
