#include <algorithm>
#include <iomanip>

#include "LUtils/Debugging.hxx"

#include "GiBUUToStdHep_Utils.hxx"

namespace GiBUUUtils {

int GiBUUToPDG(int GiBUUCode, int GiBUUCharge) {
  // https://gibuu.hepforge.org/trac/wiki/ParticleIDs
  switch (GiBUUCode) {
    case 1: {
      return (GiBUUCharge > 0) ? 2212 : 2112;
    }
    case 2: {
      switch (GiBUUCharge) {
        case 2: {
          return 2224;
        }
        case 1: {
          return 2214;
        }
        case 0: {
          return 2114;
        }
        case -1: {
          return 1114;
        }
        default: {
          UDBWarn("Delta resonance had an odd charge: " << GiBUUCharge);
          return 2114;
        }
      }
    }
    case 3: {
      return (GiBUUCharge > 0) ? 202212 : 202112;
    }
    case 4: {
      return (GiBUUCharge > 0) ? 102212 : 102112;
    }
    case 5: {
      return (GiBUUCharge > 0) ? 122212 : 122112;
    }
    case 7: {
      return (GiBUUCharge > 0) ? 102214 : 102114;
    }
    case 8: {
      return (GiBUUCharge > 0) ? 112214 : 112114;
    }
    case 10: {
      return (GiBUUCharge > 0) ? 102216 : 102116;
    }
    case 12: {
      return (GiBUUCharge > 0) ? 212212 : 212112;
    }
    case 14: {
      return (GiBUUCharge > 0) ? 212214 : 212114;
    }
    case 15: {
      return 0;
    }
    case 16: {
      return (GiBUUCharge > 0) ? 202216 : 202116;
    }
    case 19: {
      switch (GiBUUCharge) {
        case 2: {
          return 112222;
        }
        case 1: {
          return 112212;
        }
        case 0: {
          return 112112;
        }
        case -1: {
          return 111112;
        }
        default: {
          UDBWarn("S31(1620) resonance had an odd charge: " << GiBUUCharge);
          return 112112;
        }
      }
    }
    case 21: {
      switch (GiBUUCharge) {
        case 2: {
          return 122224;
        }
        case 1: {
          return 122214;
        }
        case 0: {
          return 122114;
        }
        case -1: {
          return 121114;
        }
        default: {
          UDBWarn("D33(1700) resonance had an odd charge: " << GiBUUCharge);
          return 122114;
        }
      }
    }
    case 26: {
      switch (GiBUUCharge) {
        case 2: {
          return 222222;
        }
        case 1: {
          return 222212;
        }
        case 0: {
          return 222112;
        }
        case -1: {
          return 221112;
        }
        default: {
          UDBWarn("P31(1910) resonance had an odd charge: " << GiBUUCharge);
          return 222112;
        }
      }
    }
    case 27: {
      switch (GiBUUCharge) {
        case 2: {
          return 202224;
        }
        case 1: {
          return 202214;
        }
        case 0: {
          return 202114;
        }
        case -1: {
          return 201114;
        }
        default: {
          UDBWarn("P33(1600) resonance had an odd charge: " << GiBUUCharge);
          return 202114;
        }
      }
    }
    case 28: {
      switch (GiBUUCharge) {
        case 2: {
          return 222224;
        }
        case 1: {
          return 222214;
        }
        case 0: {
          return 222114;
        }
        case -1: {
          return 221114;
        }
        default: {
          UDBWarn("P33(1920) resonance had an odd charge: " << GiBUUCharge);
          return 222114;
        }
      }
    }
    case 30: {
      switch (GiBUUCharge) {
        case 2: {
          return 212226;
        }
        case 1: {
          return 212216;
        }
        case 0: {
          return 212116;
        }
        case -1: {
          return 211116;
        }
        default: {
          UDBWarn("F35(1905) resonance had an odd charge: " << GiBUUCharge);
          return 212116;
        }
      }
    }
        case 31: {
      switch (GiBUUCharge) {
        case 2: {
          return 202228;
        }
        case 1: {
          return 202218;
        }
        case 0: {
          return 202118;
        }
        case -1: {
          return 201118;
        }
        default: {
          UDBWarn("F37(1950) resonance had an odd charge: " << GiBUUCharge);
          return 202118;
        }
      }
    }
    case 32: {
      return 3122;
    }
    case 33: {
      if (GiBUUCharge) {
        return (GiBUUCharge > 0) ? 3222 : 3112;
      }
      return 3212;
    }
    case 34: {
      if (GiBUUCharge) {
        return (GiBUUCharge > 0) ? 3224 : 3114;
      }
      return 3214;
    }
    case 53: {
      return (GiBUUCharge > 0) ? 3322 : 3312;
    }
    case 56: {
      return 4122;
    }
    case 57: {
      if (GiBUUCharge) {
        return (GiBUUCharge > 0) ? 4222 : 4112;
      }
      return 4212;
    }
    case 101: {
      if (GiBUUCharge) {
        return (GiBUUCharge > 0) ? 211 : -211;
      }
      return 111;
    }
    case 102: {
      return 221;
    }
    case 103: {
      if (GiBUUCharge) {
        return (GiBUUCharge > 0) ? 213 : -213;
      }
      return 113;
    }
    case 104: {
      return 9000221;
    }
    case 105: {
      return 223;
    }
    case 110: {
      return (GiBUUCharge) ? 321 : 311;
    }
    case 111: {
      return (GiBUUCharge) ? -321 : -311;
    }
    case 112: {
      return (GiBUUCharge) ? 323 : 313;
    }
    case 114: {
      return (GiBUUCharge) ? 411 : 421;
    }
    case 115: {
      return (GiBUUCharge) ? -411 : -421;
    }
    case 116: {
      return (GiBUUCharge) ? 413 : 423;
    }
    case 117: {
      return (GiBUUCharge) ? -413 : -423;
    }
    case 901: {
      return (GiBUUCharge < 0) ? 11 : -11;
    }
    case 902: {
      return (GiBUUCharge < 0) ? 13 : -13;
    }
    case 911: {
      return 12;
    }
    case 912: {
      return 14;
    }
    case 999: {
      return 22;
    }
    case 6:   // S11(2090)
    case 20:  // S31(1900)
    case 22:  // D33(1940)
    case 23:  // D35(1930)
    case 25:
    {         // No PDG for these particles
      return -1;
    }

    default: {
      if (GiBUUCode) {
        UDBWarn("Missed a GiBUU PDG Code: " << GiBUUCode);
      }
      return 0;
    }
  }
}

int PDGToGiBUU(int PDG) {
  // https://gibuu.hepforge.org/trac/wiki/ParticleIDs
  switch (PDG) {
    case 2212:
    case 2112: {
      return 1;
    }

    case 2224:
    case 2214:
    case 2114:
    case 1114: {
      return 2;
    }

    case 202212:
    case 202112: {
      return 3;
    }

    case 102212:
    case 102112: {
      return 4;
    }

    case 122212:
    case 122112: {
      return 5;
    }

    case 102214:
    case 102114: {
      return 7;
    }

    case 112214:
    case 112114: {
      return 8;
    }

    case 102216:
    case 102116: {
      return 10;
    }

    case 212212:
    case 212112: {
      return 12;
    }

    case 202216:
    case 202116: {
      return 16;
    }

    case 112222:
    case 112212:
    case 112112:
    case 111112: {
      return 19;
    }

    case 122224:
    case 122214:
    case 122114:
    case 121114: {
      return 21;
    }

    case 222222:
    case 222212:
    case 222112:
    case 221112: {
      return 26;
    }

    case 202224:
    case 202214:
    case 202114:
    case 201114: {
      return 27;
    }

    case 222224:
    case 222214:
    case 222114:
    case 221114: {
      return 28;
    }

    case 212226:
    case 212216:
    case 212116:
    case 211116: {
      return 30;
    }
    case 202228:
    case 202218:
    case 202118:
    case 201118: {
      return 31;
    }

    case 3122: {
      return 32;
    }

    case 3222:
    case 3112:
    case 3212: {
      return 33;
    }

    case 3224:
    case 3114:
    case 3214: {
      return 34;
    }

    case 211:
    case -211:
    case 111: {
      return 101;
    }

    case 221: {
      return 102;
    }

    case 213:
    case -213:
    case 113: {
      return 103;
    }

    case 9000221: {
      return 104;
    }

    case 223: {
      return 105;
    }

    case 321:
    case 311: {
      return 110;
    }

    case -321:
    case -311: {
      return 111;
    }

    case 323:
    case 313: {
      return 112;
    }

    case 11:
    case -11: {
      return 901;
    }

    case 13:
    case -13: {
      return 902;
    }

    case 12:
    case -12: {
      return 911;
    }

    case 14:
    case -14: {
      return 912;
    }

    case 22: {
      return 999;
    }

    default: {
      if (PDG) {
        UDBWarn("Missed a PDG Code: " << PDG);
      }
      return 0;
    }
  }
}

std::tuple<Int_t, Int_t, Int_t> DecomposeGiBUUHistory(Long_t HistCode) {
  if (HistCode > -1) {
    Int_t Generation = abs(HistCode) / 1E6;
    Int_t P2 = (HistCode - (1E6 * Generation)) / 1E3;
    Int_t P1 = (HistCode - (1E6 * Generation)) - (1E3 * P2);

    Int_t PFather = std::min(P1, P2);
    Int_t PMother = std::max(P1, P2);

    return std::make_tuple(Generation, PMother, PFather);
  } else {
    HistCode = -1 * HistCode;
    Int_t Generation = abs(HistCode) / 1E6;
    Int_t ThreeBodyCode = (HistCode - (1E6 * Generation));

    return std::make_tuple(Generation, -1, ThreeBodyCode);
  }
}

std::string WriteGiBUUHistory(Long_t HistCode) {
  std::stringstream ss("");
  if (!HistCode) {
    ss << "[Elementary interaction]";
  } else {
    auto const &hDec = GiBUUUtils::DecomposeGiBUUHistory(HistCode);

    if (std::get<1>(hDec) != -1) {
      ss << "[Gen: " << std::get<0>(hDec)
         << ", Mother: " << GiBUUToPDG(std::get<1>(hDec));
      if (std::get<2>(hDec)) {
        ss << ", Father: " << GiBUUToPDG(std::get<2>(hDec));
      }
      ss << "]";
    } else {  // 3 body
      ss << "[Gen: " << std::get<0>(hDec) << ", 3Body Process: ";
      switch (std::get<2>(hDec)) {
        case 1: {
          ss << "(N N N)]";
          break;
        }
        case 2: {
          ss << "(N N Delta)]";
          break;
        }
        case 3: {
          ss << "(N N Pion)]";
          break;
        }
        case 4: {
          ss << "Unknown]";
          break;
        }
        default: { ss << "Unknown]"; }
      }
    }
  }

  ss << " -- "
     << "(" << HistCode << ")";
  return ss.str();
}

std::string PrintGiBUUStdHepArray(Int_t GiBUUCode,
                                  Int_t const *const StdHepPDGArray,
                                  Long_t const *const HistoryArray,
                                  Int_t StdHepN,
                                  std::string const &indent = "") {
  std::stringstream ss("");
  ss << indent << "GiBUUCode: " << GiBUUCode << std::endl;
  for (Int_t i = 0; i < StdHepN; ++i) {
    ss << indent << "\t(" << std::setw(4) << StdHepPDGArray[i]
       << "): " << WriteGiBUUHistory(HistoryArray[i]) << std::endl;
  }
  return ss.str();
}

// Returns vect of: Particle PDG, Parent1, Parent2
std::vector<std::tuple<Int_t, Int_t, Int_t> > GetGenNParticles(
    Int_t Gen, Int_t const *const StdHepPDGArray,
    Long_t const *const HistoryArray, Int_t StdHepN) {
  std::vector<std::tuple<Int_t, Int_t, Int_t> > ret;

  for (Int_t i = 0; i < StdHepN; ++i) {
    auto const &dHist = DecomposeGiBUUHistory(HistoryArray[i]);
    if (std::get<0>(dHist) == Gen) {
      ret.push_back(std::make_tuple(StdHepPDGArray[i], std::get<1>(dHist),
                                    std::get<2>(dHist)));
    }
  }
  return ret;
}

size_t GetNParticleGiBUUCode(Int_t GiBUUCode, Int_t const *const StdHepPDGArray,
                             Long_t const *const HistoryArray, Int_t StdHepN) {
  size_t ctr = 0;

  for (Int_t i = 0; i < StdHepN; ++i) {
    auto const &dHist = DecomposeGiBUUHistory(HistoryArray[i]);
    ctr += (GiBUUCode == std::get<1>(dHist));
    ctr += (GiBUUCode == std::get<2>(dHist));
    ctr += (GiBUUCode == PDGToGiBUU(StdHepPDGArray[i]));
  }
  return ctr;
}

size_t GetNFSParticleGiBUUCode(Int_t GiBUUCode,
                               Int_t const *const StdHepPDGArray,
                               Int_t StdHepN) {
  size_t ctr = 0;

  for (Int_t i = 0; i < StdHepN; ++i) {
    ctr += (GiBUUCode == PDGToGiBUU(StdHepPDGArray[i]));
  }
  return ctr;
}

std::vector<Int_t> GetTwoBodyFSParticles(Int_t const *const StdHepPDGArray,
                                         Long_t const *const HistoryArray,
                                         Int_t StdHepN) {
  std::vector<Int_t> ret;

  for (Int_t i = 0; i < StdHepN; ++i) {
    if (std::get<2>(DecomposeGiBUUHistory(HistoryArray[i])) != 0) {
      ret.push_back(StdHepPDGArray[i]);
    }
  }
  return ret;
}

std::vector<Int_t> GetTwoBodyFSNucleons(Int_t const *const StdHepPDGArray,
                                        Long_t const *const HistoryArray,
                                        Int_t StdHepN) {
  std::vector<Int_t> ret;

  for (Int_t i = 0; i < StdHepN; ++i) {
    if ((std::get<2>(DecomposeGiBUUHistory(HistoryArray[i])) != 0) &&
        ((StdHepPDGArray[i] == 2122) || (StdHepPDGArray[i] == 2112))) {
      ret.push_back(StdHepPDGArray[i]);
    }
  }
  return ret;
}

// Returns Generation, Pion PDG, Decay Parent
std::vector<std::tuple<Int_t, Int_t, Int_t> > GetFSDecayPions(
    Int_t const *const StdHepPDGArray, Long_t const *const HistoryArray,
    Int_t StdHepN) {
  std::vector<std::tuple<Int_t, Int_t, Int_t> > ret;

  for (Int_t i = 0; i < StdHepN; ++i) {
    auto const &dHist = DecomposeGiBUUHistory(HistoryArray[i]);
    if ((std::get<1>(dHist) != -1) && (std::get<2>(dHist) == 0) &&
        ((StdHepPDGArray[i] == 211) || (StdHepPDGArray[i] == -211) ||
         (StdHepPDGArray[i] == 111))) {
      ret.push_back(std::make_tuple(std::get<0>(dHist), StdHepPDGArray[i],
                                    std::get<1>(dHist)));
    }
  }
  std::sort(ret.begin(), ret.end());
  return ret;
}

// Returns Generation, PDG
std::vector<std::tuple<Int_t, Int_t> > GetDeltaDecayNucleons(
    Int_t const *const StdHepPDGArray, Long_t const *const HistoryArray,
    Int_t StdHepN) {
  std::vector<std::tuple<Int_t, Int_t> > ret;

  for (Int_t i = 0; i < StdHepN; ++i) {
    auto const &dHist = DecomposeGiBUUHistory(HistoryArray[i]);
    if ((std::get<1>(dHist) == 2) && (std::get<2>(dHist) == 0) &&
        ((StdHepPDGArray[i] == 2112) || (StdHepPDGArray[i] == 2212))) {
      ret.push_back(std::make_tuple(std::get<0>(dHist), StdHepPDGArray[i]));
    }
  }
  std::sort(ret.begin(), ret.end());
  return ret;
}

std::vector<Int_t> GetFSPions(Int_t const *const StdHepPDGArray,
                              Int_t StdHepN) {
  std::vector<Int_t> ret;

  for (Int_t i = 0; i < StdHepN; ++i) {
    if ((StdHepPDGArray[i] == 211) || (StdHepPDGArray[i] == -211) ||
        (StdHepPDGArray[i] == 111)) {
      ret.push_back(StdHepPDGArray[i]);
    }
  }
  return ret;
}

std::vector<Int_t> GetInHistoryResonances(Int_t const *const StdHepPDGArray,
                                          Long_t const *const HistoryArray,
                                          Int_t StdHepN) {
  std::vector<Int_t> ret;

  for (Int_t i = 0; i < StdHepN; ++i) {
    ret.push_back(StdHepPDGArray[i]);
  }
  return ret;
}

Int_t PionPDGToNeutResMode(Int_t pionPDG, Int_t NucleonPDG, bool &Warn) {
  Warn = (!NucleonPDG);
  switch (pionPDG) {
    case 211: {
      return (NucleonPDG == 2112) ? 13 : 11;
    }
    case 111: {
      if (NucleonPDG == 2112) {
        Warn = true;
      }
      return 12;
    }
    default: {}
  }
  return 0;
}

int ResonanceHeuristics(Int_t const *const StdHepPDGArray,
                        Long_t const *const HistoryArray, Int_t StdHepN) {
  // RESONANCE HEURISTICS
  auto const &g1parts =
      GetGenNParticles(1, StdHepPDGArray, HistoryArray, StdHepN);

  Int_t NucleonPDG = 0;
  for (auto const &part : g1parts) {
    if ((std::get<1>(part) == 2) &&       // first gen delta decay child
        ((std::get<0>(part) == 2212) ||   // a proton
         (std::get<0>(part) == 2112))) {  // a neutron
      NucleonPDG = std::get<0>(part);
      break;
    }
  }

  // If we find a first generation pion
  for (auto const &part : g1parts) {
    bool Warn = false;
    // Check the pions
    Int_t rMode = PionPDGToNeutResMode(std::get<0>(part), NucleonPDG, Warn);
    if (rMode) {
      if (Warn) {  // If theres something fishy then shout about it
        UDBWarn("Returning potentially dodgey (mode:0) "
                << "NEUT Code: " << rMode << " (Found Nucleon: " << NucleonPDG
                << ").");
        UDBWarn("        It came from the event: " << PrintGiBUUStdHepArray(
                    2, StdHepPDGArray, HistoryArray, StdHepN, "\t|"));
      }
      return rMode;
    }
  }

  // If we're still here then we should check FS pions from decays
  auto const &FSDP = GetFSDecayPions(StdHepPDGArray, HistoryArray, StdHepN);
  for (auto const &decayPi : FSDP) {  // Try first to make sure it is from
    if (std::get<2>(decayPi) != 2) {  // a Delta
      UDBWarn("Had a GiBUU mode 2 which resulted in decay "
              << "pions. Their decay parent, " << std::get<2>(decayPi)
              << ", was not a Delta though.");
      if (std::get<2>(decayPi) == 102 || std::get<2>(decayPi) == 104) {
        return 21;
      }
      continue;
    }
    // Try and find a nucleon from the same generation.
    auto const &gxparts = GetGenNParticles(std::get<0>(decayPi), StdHepPDGArray,
                                           HistoryArray, StdHepN);
    Int_t SameGenNucleon = 0;
    for (auto const &part : gxparts) {
      if ((std::get<1>(part) == 2) &&       // first gen delta decay child
          ((std::get<0>(part) == 2212) ||   // a proton
           (std::get<0>(part) == 2112))) {  // a neutron
        SameGenNucleon = std::get<0>(part);
        break;
      }
    }
    bool Warn = false;
    Int_t rMode =
        PionPDGToNeutResMode(std::get<1>(decayPi), SameGenNucleon, Warn);
    if (rMode) {
      if (Warn) {  // If theres something fishy then shout about it
        UDBWarn("Returning potentially dodgey (mode:1) "
                << "NEUT Code: " << rMode << " (Found Same generation Nucleon: "
                << SameGenNucleon << ").");
        UDBWarn("        It came from the event: " << PrintGiBUUStdHepArray(
                    2, StdHepPDGArray, HistoryArray, StdHepN, "\t|"));
      }
      return rMode;
    }
  }

  for (auto const &decayPi : FSDP) {
    // Try and find a nucleon from the same generation and same decay
    // parent PDG.
    auto const &gxparts = GetGenNParticles(std::get<0>(decayPi), StdHepPDGArray,
                                           HistoryArray, StdHepN);
    Int_t SameGenNucleon = 0;
    for (auto const &part : gxparts) {
      if ((std::get<1>(part) == std::get<2>(decayPi)) &&  // same decay
          // parent as pion
          ((std::get<0>(part) == 2212) ||   // a proton
           (std::get<0>(part) == 2112))) {  // a neutron
        SameGenNucleon = std::get<0>(part);
        break;
      }
    }
    bool Warn = false;
    Int_t rMode =
        PionPDGToNeutResMode(std::get<1>(decayPi), SameGenNucleon, Warn);
    if (rMode) {
      UDBWarn("Returning potentially dodgey (mode:2) "
              << "NEUT Code: " << rMode << " (Found Nucleon probably from same "
              << "decay: " << SameGenNucleon << ").");
      UDBWarn("        It came from the event: " << PrintGiBUUStdHepArray(
                  2, StdHepPDGArray, HistoryArray, StdHepN, "\t|"));
      return rMode;
    }
  }

  if (!NucleonPDG) {  // If we havent found one yet.
    // Get the lowest gen one!
    auto const &gxparts =
        GetDeltaDecayNucleons(StdHepPDGArray, HistoryArray, StdHepN);
    NucleonPDG = gxparts.size() ? std::get<1>(gxparts.front()) : 0;
  }

  UDBWarn("Giving up on this Delta resonance, returning: "
          << ((NucleonPDG == 2212) ? 10 : 9)
          << " (Found Nucleon: " << NucleonPDG << ")."
          << PrintGiBUUStdHepArray(2, StdHepPDGArray, HistoryArray, StdHepN,
                                   "\t+"));
  return (NucleonPDG == 2212) ? 11 : 12;
}

int GiBUU2NeutReacCode(Int_t GiBUUCode, Int_t const *const StdHepPDGArray,
                       Long_t const *const HistoryArray, Int_t StdHepN,
                       bool IsCC, Int_t StruckNucleonPosition,
                       Int_t PrimaryProdCharge) {
  // 1=QE, 2-31=res ID, 32,33=1pi, 34=DIS, 35,36=2p2h, 37=2pi
  // From https://gibuu.hepforge.org/trac/wiki/LesHouches

  Int_t StruckNucleonPDG =
      (StruckNucleonPosition > 0) ? StdHepPDGArray[StruckNucleonPosition] : 0;
  bool IsNu = (StdHepPDGArray[0] > 0);

  switch (GiBUUCode) {
    case 1: {
      if (IsCC) {
        return (IsNu ? 1 : -1);
      } else {
        return (IsNu ? 1 : -1) * ((StruckNucleonPDG == 2212) ? 51 : 52);
      }
    }          // QE
    case 2: {  // delta
      if (PrimaryProdCharge == -10) {
        return (IsNu ? 1 : -1) *
               (ResonanceHeuristics(StdHepPDGArray, HistoryArray, StdHepN) +
                ((!IsCC) * 20));
      }

      if (IsCC) {
        if (IsNu) {
          switch (PrimaryProdCharge) {
            case 2:
              return 11;
            case 1:
              return 12;
            default: {
              UDBError("Unexpected delta state: CC "
                       << IsCC << ", neutrino " << IsNu
                       << ", with delta charge: " << PrimaryProdCharge);
              throw;
            }
          }
        } else {
          switch (PrimaryProdCharge) {
            case 0:
              return -12;
            case -1:
              return -11;
            default: {
              UDBError("Unexpected delta state: CC "
                       << IsCC << ", neutrino " << IsNu
                       << ", with delta charge: " << PrimaryProdCharge);
              throw;
            }
          }
        }
      } else {
        if (IsNu) {
          switch (PrimaryProdCharge) {
            case 0:
              return 31;
            case 1:
              return 32;
            default: {
              UDBError("Unexpected delta state: CC "
                       << IsCC << ", neutrino " << IsNu
                       << ", with delta charge: " << PrimaryProdCharge);
              throw;
            }
          }
        } else {
          switch (PrimaryProdCharge) {
            case 0:
              return -31;
            case 1:
              return -32;
            default: {
              UDBError("Unexpected delta state: CC "
                       << IsCC << ", neutrino " << IsNu
                       << ", with delta charge: " << PrimaryProdCharge);
              throw;
            }
          }
        }
      }
    }
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31: {  // CCResonance
      return (IsNu ? 1 : -1) *
             (5 + ((PrimaryProdCharge == -10) ? 0 : PrimaryProdCharge) +
              43 * (!IsCC));
    }
    case 32:
    case 33: {  // 1Pi Bkg
      return (IsNu ? 1 : -1) * (10 + 20 * (!IsCC));
    }
    case 34: {  // DIS
      return (IsNu ? 1 : -1) * (26 + 20 * (!IsCC));
    }
    case 35:
    case 36: {  // MEC/2p-2h
      return (IsNu ? 1 : -1) * 2;
    }
    case 37: {  // MultiPi
      return (IsNu ? 1 : -1) * (21 + 20 * (!IsCC));
    }
    default: {}
  }

  UDBWarn(
      "Couldn't determine NEUT equivalent reaction code "
      "for the interaction:"
      << PrintGiBUUStdHepArray(GiBUUCode, StdHepPDGArray, HistoryArray,
                               StdHepN));
  return 0;
}
}
