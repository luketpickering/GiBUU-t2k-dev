#ifndef SEEN_GIBUUToStdHep_UTILS_HXX
#define SEEN_GIBUUToStdHep_UTILS_HXX

#include <cerrno>
#include <climits>
#include <cstdlib>

#include <functional>
#include <sstream>
#include <string>

#include <iostream>
#include <vector>

#include "Rtypes.h"
#include "TLorentzVector.h"
#include "TVector3.h"

#include "GiBUUToStdHep_CLIOpts.hxx"

namespace PhysConst {
  double const MeV = 1E-3;
  double const KeV = 1E-6;
}

/// Utilities which may be helpful for processing GiBUU specific output.
namespace GiBUUUtils {

///\brief Converts a GiBUU particle code, with associated particle EM charge
/// information to a PDG code.
///
/// From https://gibuu.hepforge.org/trac/wiki/ParticleIDs
///\note Returns 0 when encountering an unknown particle.
/// Current codes converted:
/// - GiBUU : PDG
/// - 1 : p=2212, n=2112
/// - 2 : Delta++=2224, Delta+=2214, Delta0=2114, Delta-=1114
/// - 3 : P11(1440) PDGs : 202212, 202112
/// - 4 : S11(1535) PDGs : 102212, 102112
/// - 5 : S11(1650) PDGs : 122212, 122112
/// - 6 : S11(2090) PDGs : n/a
/// - 7 : D13(1520) PDGs : 102214, 102114
/// - 8 : D13(1700) PDGs : 112214, 112114
/// - 9 : D13(2080) PDGs : n/a
/// - 10 : D15(1675) PDGs : 102216, 102116
/// - 11 : G17(2190) PDGs : n/a
/// - 12 : P11(1710) PDGs : 212212, 212112
/// - 13 : P11(2100) PDGs : n/a
/// - 14 : P13(1720) PDGs : 212214, 212114
/// - 15 : P13(1900) PDGs : n/a
/// - 16 : F15(1680) PDGs : 202216, 202116
/// - 17 : F15(2000) PDGs : n/a
/// - 18 : F17(1990) PDGs : n/a
/// - 19 : S31(1620) PDGs : 112222, 112212, 112112, 111112
/// - 20 : S31(1900) PDGs : n/a
/// - 21 : D33(1700) PDGs : 122224, 122214, 122114, 121114
/// - 22 : D33(1940) PDGs : n/a
/// - 23 : D35(1930 PDGs : n/a
/// - 24 : D35(2350) PDGs : n/a
/// - 25 : P31(1750) PDGs : n/a
/// - 26 : P31(1910) PDGs : 222222, 222212, 222112, 221112
/// - 27 : P33(1600) PDGs : 202224, 202214, 202114, 201114
/// - 28 : P33(1920) PDGs : 222224, 222214, 222114, 221114
/// - 29 : F35(1750) PDGs : n/a
/// - 30 : F35(1905) PDGs : 212226, 212216, 212116, 211116
/// - 31 : F35(1950) PDGs : 202228, 202218, 202118, 201118
/// - 32 : Lambda PDG: 3122
/// - 33 : Sigma PDGs : 3222, 3212, 3112
/// - 34 : Sigma(1385) PDGs : 3224, 3214, 3114
/// - 35 : Lambda(1405) PDGs : 102132
/// - 36 : Lambda(1520) PDG : 102134
/// - 37 : Lambda(1600) PDG : 203122
/// - 38 : Lambda(1670) PDG : 103122
/// - 39 : Lambda(1690) PDG : 103124
/// - 40 : Lambda(1810) PDG : 213122
/// - 41 : Lambda(1820) PDG : 203126
/// - 42 : Lambda(1830) PDG : 103126
/// - 43 : Sigma(1670) PDGs : 103224, 103214, 103114
/// - 44 : Sigma(1775) PDGs : 103226, 103216, 103116
/// - 45 : Sigma(2030) PDGs : 203228, 203218, 203118
/// - 46 : Lambda(1800) PDG : 123122
/// - 47 : Lambda(1890) PDG : 213124
/// - 48 : Lambda(2100) PDG : n/a
/// - 49 : Lambda(2110) PDG : n/a
/// - 50 : Sigma(1660) PDGs : 203222, 203212, 203112
/// - 51 : Sigma(1750) PDGs : 113222, 113212, 113112
/// - 52 : Sigma(1915) PDGs : 203226, 203216, 203116
/// - 53 : Xi PDG: 3322, 3312
/// - 54 : Xi^Star PDG: 3324, 3314
/// - 55 : Omega PDG: 3334
/// - 56 : Lambda_c PDG: 4122
/// - 57 : Sigma_c PDG: 4222, 4212, 4112
/// - 58 : Sigma_c^star PDG: 4224, 4214, 4114
/// - 59 : Xi_c PDG: 4232, 4132
/// - 60 : Xi_c^star PDG: 4324, 4314
/// - 61 : Omega_c PDG: 4332
/// - 101 : pi+=211, pi0=111, pi-=-211
/// - 102 : eta=221
/// - 103 : rho+=213, rho0=113, rho-=-213
/// - 104 : sigma=9000221
/// - 105 : omega=223
/// - 106 : eta prime=331
/// - 107 : phi=333
/// - 108 : eta_c=441
/// - 109 : j_psi=443
/// - 110 : K+=321, K0=311
/// - 111 : K-=-321, K0=-311
/// - 112 : K*-=323, K*0=313
/// - 113 : K*bar-=-323, K*0bar=-313
/// - 114 : D PDGs : 411, 421
/// - 115 : D bar PDGs : -411, -421
/// - 116 : D star PDGs : 413, 423
/// - 117 : D bar star PDGs : -413, -423
/// - 118 : D_s^plus PDGs : 431
/// - 119 : D_s_minus PDGs : -431
/// - 120 : D^star_s^plus PDGs : 433
/// - 121 : D^star_s^minus PDGs : -433
/// - 122 : f2(1270) PDGs : 225
/// - 901 : 11
/// - 902 : 13
/// - 911 : 12
/// - 912 : 14
/// - 913 : 16
/// - -911 : -12
/// - -912 : -14
/// - -913 : -16
/// - 999 : 22
int GiBUUToPDG(int GiBUUCode, int GiBUUCharge = 0);

#ifndef CPP03COMPAT
std::tuple<Int_t, Int_t, Int_t> DecomposeGiBUUHistory(Long_t HistCode);
std::string WriteGiBUUHistory(Long_t HistCode);
#endif

///\brief Converts a GiBUU interaction code to the corresponding NEUT code
/// where possible.
///
/// Sometimes the NEUT code is dependent on the particle produced in the intial
/// interaction.
/// CC:
/// * 1 : QE
/// * 2 : 2p2h
/// * 10 : Single pion background (non-resonant)
/// * 11 : Delta++ ( -11 : Delta- for nubar)
/// * 12 : Delta+ (-12 : Delta0 for nubar)
/// * 21 : Multi pion production
/// * 26 : DIS
/// * 4 : Higher resonance, charge: -1
/// * 5 : Higher resonance, charge: 0
/// * 6 : Higher resonance, charge: +1
/// * 7 : Higher resonance, charge: +2
///
/// NC:
/// * 30 : Single pion background (non-resonant)
/// * 31 : Delta0
/// * 32 : Delta+
/// * 41 : Multi pion production
/// * 42 : 2p2h
/// * 46 : DIS
/// * 47 : Higher resonance, charge: -1
/// * 48 : Higher resonance, charge: 0
/// * 49 : Higher resonance, charge: +1
/// * 50 : Higher resonance, charge: +2
/// * 51 : NCEL proton-target
/// * 52 : NCEL neutron-target
///
/// From https://gibuu.hepforge.org/trac/wiki/LesHouches
int GiBUU2NeutReacCode(Int_t GiBUUCode, Int_t const *const StdHepPDGArray,
#ifndef CPP03COMPAT
                       Long_t const *const HistoryArray,
#endif
                       Int_t StdHepN, bool IsCC = true,
                       Int_t StruckNucleonPosition = -1,
                       Int_t PrimaryProdCharge = -10);

///\brief Converts a GiBUU interaction code to a NEUT-like code for e-scattering
/// events
///
/// EM:
/// * 1 : QE
/// * 2 : 2p2h
/// * 10 : Single pion background (non-resonant)
/// * 11 : Delta
/// * 21 : Multi pion production
/// * 26 : DIS
/// * 4 : Higher resonance, charge
///
/// From https://gibuu.hepforge.org/trac/wiki/LesHouches
int GiBUU2NeutReacCode_escat(Int_t GiBUUCode,
                             Int_t const *const StdHepPDGArray);

}

template <typename T>
inline std::string NegSpacer(T const &num) {
  return (num >= 0) ? " " : "";
}

inline std::ostream &operator<<(std::ostream &os, TVector3 const &tl) {
  std::streamsize prec = os.precision();
  std::ios_base::fmtflags flags = os.flags();
  os.precision(2);
  os.flags(std::ios::scientific);
  os << " " << NegSpacer(tl[0]) << tl[0] << "," << NegSpacer(tl[1]) << tl[1]
     << "," << NegSpacer(tl[2]) << tl[2] << ")]";
  os.precision(prec);
  os.flags(flags);
  return os;
}

inline std::ostream &operator<<(std::ostream &os, TLorentzVector const &tlv) {
  std::streamsize prec = os.precision();
  std::ios_base::fmtflags flags = os.flags();
  os.precision(2);
  os.flags(std::ios::scientific);
  os << "[" << NegSpacer(tlv[0]) << tlv[0] << "," << NegSpacer(tlv[1]) << tlv[1]
     << "," << NegSpacer(tlv[2]) << tlv[2] << "," << NegSpacer(tlv[3]) << tlv[3]
     << ":M(" << tlv.M() << ")]";
  os.precision(prec);
  os.flags(flags);
  return os;
}

struct GiBUUPartBlob {
  GiBUUPartBlob()
      : Run(0),
        EvNum(0),
        ID(0),
        Charge(0),
        PerWeight(0),
        Position(0, 0, 0),
        FourMom(0, 0, 0, 0),
        History(0),
        Prodid(0),
        Enu(0),
        ProdCharge(0),
        ln(0) {}
  Int_t Run;
  Int_t EvNum;
  Int_t ID;
  Int_t Charge;
  Double_t PerWeight;
  TVector3 Position;
  TLorentzVector FourMom;
  Long_t History;
  Int_t Prodid;
  Double_t Enu;
  Int_t ProdCharge;
  Int_t ln;
};

namespace {
std::ostream &operator<<(std::ostream &os, GiBUUPartBlob const &part) {
  os << "{ Run: " << part.Run << ", EvNum: " << part.EvNum
     << ", ID: " << part.ID << ", Charge: " << part.Charge
     << ", PerWeight: " << part.PerWeight << ", Pos: " << part.Position
     << ", 4Mom: " << part.FourMom << ", History: " << part.History
     << ", Prodid: " << part.Prodid << ", Enu: " << part.Enu;
  if (GiBUUToStdHepOpts::HaveProdChargeInfo) {
    os << ", ProdCharge: " << part.ProdCharge;
  }
  os << ", LineNumber: " << part.ln;
  return os << " }";
}
}

#endif
