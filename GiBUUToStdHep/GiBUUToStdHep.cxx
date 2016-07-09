#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "TFile.h"
#include "TLorentzVector.h"
#include "TTree.h"
#include "TVector3.h"

#include "LHEF.hpp"

#include "LUtils/CLITools.hxx"
#include "LUtils/Utils.hxx"

#include "GiBUUToStdHep_Utils.hxx"

#include "GiRooTracker.hxx"

namespace {
template <typename T>
inline std::string NegSpacer(T const &num) {
  return (num >= 0) ? " " : "";
}
inline std::ostream &operator<<(std::ostream &os, TVector3 const &tl) {
  auto prec = os.precision();
  auto flags = os.flags();
  os.precision(2);
  os.flags(std::ios::scientific);
  os << " " << NegSpacer(tl[0]) << tl[0] << "," << NegSpacer(tl[1]) << tl[1]
     << "," << NegSpacer(tl[2]) << tl[2] << ")]";
  os.precision(prec);
  os.flags(flags);
  return os;
}
inline std::ostream &operator<<(std::ostream &os, TLorentzVector const &tlv) {
  auto prec = os.precision();
  auto flags = os.flags();
  os.precision(2);
  os.flags(std::ios::scientific);
  os << "[" << NegSpacer(tlv[0]) << tlv[0] << "," << NegSpacer(tlv[1]) << tlv[1]
     << "," << NegSpacer(tlv[2]) << tlv[2] << "," << NegSpacer(tlv[3]) << tlv[3]
     << ":M(" << tlv.M() << ")]";
  os.precision(prec);
  os.flags(flags);
  return os;
}
}

/// Options relevant to the GiBUUToStdHep.exe executable.
namespace GiBUUToStdHepOpts {

/// The location of the input file which was produced by GiBUU.
std::string InpFName;
/// The name of the output root file to write.
std::string OutFName;
/// Whether the input is a FinalEvents.dat type file.
bool InpIsFE;
/// Whether the input is a LesHouchesXXX.xml type files.
bool InpIsLH;
/// Whether the GiBUU output contains struck nucleon information.
bool HaveStruckNucleonInfo;

///\brief The neutrino species PDG.
///
///\note Set by
///  `GiBUUToStdHep.exe ... -u xx ...'
/// Required.
int nuType;
///\brief The target nuclei nucleon number, A.
///
///\note Set by
///  `GiBUUToStdHep.exe ... -a xx ...'
/// Required.
int TargetA;
///\brief The target nuclei proton number, Z.
///
///\note Set by
///  `GiBUUToStdHep.exe ... -z xx ...'
/// Required.
int TargetZ;
///\brief Whether input events are simulated NC interactions.
///
///\note Set by
///  `GiBUUToStdHep.exe ... -c ...'
int IsNC;
///\brief The maximum number of input entries to process.
///
///\note Set by
///  `GiBUUToStdHep.exe ... -n xx ...'
long MaxEntries;
///\brief The the debugging verbosity. From 0 (quiet) --- 4 (verbose).
///
///\note Set by
///  `GiBUUToStdHep.exe ... -v xx ...'
int Verbosity = 0;

///\brief Whether to produce NuWro flavor StdHep output.
bool EmulateNuWro = false;

///\brief Whether to remove events which have a zero weight.
bool SaveNoWeight = true;
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
        Enu(0) {}
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
};

namespace {
std::ostream &operator<<(std::ostream &os, GiBUUPartBlob const &part) {
  return os << "{ Run: " << part.Run << ", EvNum: " << part.EvNum
            << ", ID: " << part.ID << ", Charge: " << part.Charge
            << ", PerWeight: " << part.PerWeight << ", Pos: " << part.Position
            << ", 4Mom: " << part.FourMom << ", History: " << part.History
            << ", Prodid: " << part.Prodid << ", Enu: " << part.Enu << " }";
}
}

GiBUUPartBlob GetParticleLine(std::string const &line) {
  GiBUUPartBlob pblob;

  auto const &splitLine = Utils::SplitStringByDelim(line, " ");
  if (splitLine.size() != 15) {
    std::cout << "[WARN]: Event had malformed particle line: \"" << line << "\""
              << std::endl;
    return pblob;
  }

  try {
    pblob.Run = std::stoi(splitLine[0]);
    pblob.EvNum = std::stoi(splitLine[1]);
    pblob.ID = std::stoi(splitLine[2]);
    pblob.Charge = std::stoi(splitLine[3]);
    pblob.PerWeight = std::stod(splitLine[4]);
    pblob.Position[GiRooTracker::kStdHepIdxPx] = std::stod(splitLine[5]);
    pblob.Position[GiRooTracker::kStdHepIdxPy] = std::stod(splitLine[6]);
    pblob.Position[GiRooTracker::kStdHepIdxPz] = std::stod(splitLine[7]);
    pblob.FourMom[GiRooTracker::kStdHepIdxE] = std::stod(splitLine[8]);
    pblob.FourMom[GiRooTracker::kStdHepIdxPx] = std::stod(splitLine[9]);
    pblob.FourMom[GiRooTracker::kStdHepIdxPy] = std::stod(splitLine[10]);
    pblob.FourMom[GiRooTracker::kStdHepIdxPz] = std::stod(splitLine[11]);
    pblob.History = std::stol(splitLine[12]);
    pblob.Prodid = std::stoi(splitLine[13]);
    pblob.Enu = std::stod(splitLine[14]);
  } catch (const std::invalid_argument &ia) {
    std::cout << "[WARN]: Failed to parse one of the values: \"" << line << "\""
              << std::endl;
    throw;
  }

  if (GiBUUToStdHepOpts::Verbosity > 2) {
    std::cout << "[PARSED]: " << pblob << std::endl;
  }
  return pblob;
}

int ParseFinalEventsFile(TTree *OutputTree, GiRooTracker *giRooTracker) {
  std::ifstream ifs(GiBUUToStdHepOpts::InpFName);
  std::vector<std::vector<GiBUUPartBlob> > Events;
  // http://www2.research.att.com/~bs/bs_faq2.html
  // People sometimes worry about the cost of std::vector growing incrementally.
  // I used to worry about that and used reserve() to optimize the growth.
  // After measuring my code and repeatedly having trouble finding the
  // performance benefits of reserve() in real programs, I stopped using it
  // except where it is needed to avoid iterator invalidation (a rare case in my
  // code). Again: measure before you optimize.

  std::string line;
  std::vector<GiBUUPartBlob> CurrEv;
  int LastEvNum = 0;
  int LineNum = 0;
  int NumEvs = 0;
  while (std::getline(ifs, line)) {
    if (GiBUUToStdHepOpts::Verbosity > 3) {
      std::cout << "[LINE:" << LineNum << "]: " << line << std::endl;
    }
    if (!LastEvNum) {  // Skip header line
      if (!std::getline(ifs, line)) {
        throw 5;
      }
      LineNum++;
      // std::cout << "[LINE:" << LineNum << "]: " << line << std::endl;
    }
    auto const &part = GetParticleLine(line);
    if ((part.EvNum != LastEvNum) && LastEvNum) {
      Events.push_back(CurrEv);
      NumEvs++;
      CurrEv.clear();
      if (GiBUUToStdHepOpts::MaxEntries == NumEvs) {
        break;
      }
    }
    CurrEv.push_back(part);
    LastEvNum = part.EvNum;
    LineNum++;
  }
  if (CurrEv.size()) {
    Events.push_back(CurrEv);
    CurrEv.clear();
  }

  ifs.close();  // Read all the lines.
  std::cout << "Found " << Events.size() << " events in "
            << GiBUUToStdHepOpts::InpFName << "." << std::endl;

  NumEvs = 0;
  for (auto const &ev : Events) {
    giRooTracker->Reset();

    int const &EvNum = ev.front().EvNum;

    if (!EvNum) {  // Malformed line
      if (GiBUUToStdHepOpts::Verbosity) {
        std::cout << "Skipping event due to malformed line." << std::endl;
      }
      continue;
    }

    if (!(NumEvs % 1000) && GiBUUToStdHepOpts::Verbosity) {
      std::cout << "Read " << NumEvs << " events." << std::endl;
    }

    if (GiBUUToStdHepOpts::MaxEntries == NumEvs) {
      std::cout << "Finishing after " << NumEvs << " entries." << std::endl;
      break;
    }
    // Skip very low weight events.
    if (!GiBUUToStdHepOpts::SaveNoWeight && ev.front().PerWeight < 1E-12) {
      continue;
    }

    giRooTracker->EvtNum = EvNum;

    // neutrino
    giRooTracker->StdHepPdg[0] = GiBUUToStdHepOpts::nuType;
    giRooTracker->StdHepStatus[0] = 0;
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPx] = 0;
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPy] = 0;
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPz] = ev.front().Enu;
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxE] = ev.front().Enu;

    // target
    giRooTracker->StdHepPdg[1] = Utils::MakeNuclearPDG(
        GiBUUToStdHepOpts::TargetZ, GiBUUToStdHepOpts::TargetA);
    giRooTracker->StdHepStatus[1] = 0;
    if (!GiBUUToStdHepOpts::EmulateNuWro) {
      giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPx] = 0;
      giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPy] = 0;
      giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPz] = 0;
      giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxE] =
          GiBUUToStdHepOpts::TargetA;
    }

    giRooTracker->GiBUUReactionCode = ev.front().Prodid;
    giRooTracker->GiBUUPerWeight = ev.front().PerWeight;
    giRooTracker->StdHepN = 2;

    bool BadEv = false;
    for (auto const &part : ev) {
      if (!part.EvNum) {  // Malformed line
        if (GiBUUToStdHepOpts::Verbosity) {
          std::cout << "Skipping event due to malformed line." << std::endl;
        }
        BadEv = true;
        break;
      }

      // Struck nucleon handling depends on output format
      if (GiBUUToStdHepOpts::HaveStruckNucleonInfo &&
          (giRooTracker->StdHepN == 3) && (GiBUUToStdHepOpts::EmulateNuWro &&
                                           (!giRooTracker->StruckNucleonPDG))) {
        giRooTracker->StruckNucleonPDG =
            GiBUUUtils::GiBUUToPDG(part.ID, part.Charge);
        giRooTracker->StdHepStatus[1] = 0;
        giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPx] =
            part.FourMom.X();
        giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPy] =
            part.FourMom.Y();
        giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPz] =
            part.FourMom.Z();
        giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxE] = part.FourMom.E();
        continue;

      } else if (GiBUUToStdHepOpts::HaveStruckNucleonInfo &&
                 (giRooTracker->StdHepN ==
                  3)) {  // Struck nucleon status if not
        giRooTracker->StdHepStatus[giRooTracker->StdHepN] =
            11;  // in NuWro mode.
      } else {
        giRooTracker->StdHepStatus[giRooTracker->StdHepN] = 1;  // All other FS
      }  // should be good.

      giRooTracker->StdHepPdg[giRooTracker->StdHepN] =
          GiBUUUtils::GiBUUToPDG(part.ID, part.Charge);

      giRooTracker->StdHepP4[giRooTracker->StdHepN]
                            [GiRooTracker::kStdHepIdxPx] = part.FourMom.X();
      giRooTracker->StdHepP4[giRooTracker->StdHepN]
                            [GiRooTracker::kStdHepIdxPy] = part.FourMom.Y();
      giRooTracker->StdHepP4[giRooTracker->StdHepN]
                            [GiRooTracker::kStdHepIdxPz] = part.FourMom.Z();
      giRooTracker->StdHepP4[giRooTracker->StdHepN][GiRooTracker::kStdHepIdxE] =
          part.FourMom.E();

      giRooTracker->GiBHepHistory[giRooTracker->StdHepN] = part.History;
      auto const &hDec = GiBUUUtils::DecomposeGiBUUHistory(part.History);
      giRooTracker->GiBHepGeneration[giRooTracker->StdHepN] = std::get<0>(hDec);

      if (std::get<1>(hDec) == -1) {  // If this was produced by a 3 body
                                      // process
        giRooTracker->GiBHepMother[giRooTracker->StdHepN] = std::get<1>(hDec);
        giRooTracker->GiBHepFather[giRooTracker->StdHepN] = std::get<2>(hDec);
      } else {
        giRooTracker->GiBHepMother[giRooTracker->StdHepN] =
            GiBUUUtils::GiBUUToPDG(std::get<1>(hDec));
        giRooTracker->GiBHepFather[giRooTracker->StdHepN] =
            GiBUUUtils::GiBUUToPDG(std::get<2>(hDec));
      }
      giRooTracker->StdHepN++;
    }

    if (BadEv) {
      continue;
    }  // If we broke then don't bother continuing
       // processing.

    if (GiBUUToStdHepOpts::EmulateNuWro &&
        GiBUUToStdHepOpts::HaveStruckNucleonInfo &&
        (giRooTracker->GiBUUReactionCode == 2) &&
        (giRooTracker->StruckNucleonPDG == 2212)) {
      giRooTracker->GiBUU2NeutCode = 11;  // Special case that we know.
    } else {                              // Try the heuristics.
      giRooTracker->GiBUU2NeutCode = GiBUUUtils::GiBUU2NeutReacCode(
          giRooTracker->GiBUUReactionCode, giRooTracker->StdHepPdg,
          giRooTracker->GiBHepHistory, giRooTracker->StdHepN, true,
          GiBUUToStdHepOpts::HaveStruckNucleonInfo ? 3 : -1);
    }

    std::stringstream ss("");
    ss << giRooTracker->GiBUU2NeutCode;
    giRooTracker->EvtCode->SetString(ss.str().c_str());

    if (GiBUUToStdHepOpts::Verbosity > 1) {
      std::cout << "[INFO]: EvNo: " << EvNum << ", contained "
                << giRooTracker->StdHepN << " (" << ev.size()
                << ") particles. "
                   "Event Weight: "
                << std::setprecision(3) << giRooTracker->GiBUUPerWeight
                << "\n\tGiBUUReactionCode: " << giRooTracker->GiBUUReactionCode
                << ", NeutConventionReactionCode: "
                << giRooTracker->GiBUU2NeutCode << "\n\t[Lep In] : "
                << TLorentzVector(
                       giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPx],
                       giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPy],
                       giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPz],
                       giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxE])
                << std::endl;
      std::cout << "\t[Target] : " << giRooTracker->StdHepPdg[1] << std::endl;
      if (GiBUUToStdHepOpts::HaveStruckNucleonInfo) {
        if (GiBUUToStdHepOpts::EmulateNuWro) {
          std::cout
              << "\t[Nuc In] : "
              << TLorentzVector(
                     giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPx],
                     giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPy],
                     giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPz],
                     giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxE])
              << " (" << std::setw(4) << giRooTracker->StruckNucleonPDG << ")"
              << std::endl;
        } else {
          std::cout
              << "\t[Nuc In] : "
              << TLorentzVector(
                     giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxPx],
                     giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxPy],
                     giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxPz],
                     giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxE])
              << " (" << std::setw(4) << giRooTracker->StdHepPdg[3] << ")"
              << std::endl;
        }
      }

      // We have already printed the struck nucleon
      Int_t StartPoint = ((!GiBUUToStdHepOpts::HaveStruckNucleonInfo)
                              ? 3
                              : (GiBUUToStdHepOpts::EmulateNuWro ? 3 : 4));
      for (Int_t stdHepInd = StartPoint; stdHepInd < giRooTracker->StdHepN;
           ++stdHepInd) {
        std::cout << "\t[" << std::setw(2) << (stdHepInd - (StartPoint)) << "]("
                  << std::setw(5) << giRooTracker->StdHepPdg[stdHepInd] << ")"
                  << TLorentzVector(
                         giRooTracker->StdHepP4[stdHepInd]
                                               [GiRooTracker::kStdHepIdxPx],
                         giRooTracker->StdHepP4[stdHepInd]
                                               [GiRooTracker::kStdHepIdxPy],
                         giRooTracker->StdHepP4[stdHepInd]
                                               [GiRooTracker::kStdHepIdxPz],
                         giRooTracker->StdHepP4[stdHepInd]
                                               [GiRooTracker::kStdHepIdxE])
                  << " (H:" << giRooTracker->GiBHepHistory[stdHepInd] << ")"
                  << std::endl;

        std::cout << "\t\t" << GiBUUUtils::WriteGiBUUHistory(
                                   giRooTracker->GiBHepHistory[stdHepInd])
                  << std::endl;
      }
      std::cout << "\t[Lep Out]: "
                << TLorentzVector(
                       giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPx],
                       giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPy],
                       giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPz],
                       giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxE])
                << std::endl
                << std::endl;
    }
    OutputTree->Fill();
    NumEvs++;
  }
  std::cout << "[INFO]: Saved: " << NumEvs
            << " events, skipped: " << (Events.size() - NumEvs)
            << " because of low weight"
            << (((Events.size() - NumEvs) > 1) ? "s" : "") << "." << std::endl;
  return 0;
}

struct LHAdditionInfoLine {
  LHAdditionInfoLine()
      : Magic(0),
        EvId(0),
        EvWeight(0),
        Nu4Mom(0, 0, 0, 0),
        ChargedLepton4Mom(0, 0, 0, 0),
        StruckNuc4Mom(0, 0, 0, 0) {}
  Int_t Magic;
  Int_t EvId;
  Double_t EvWeight;
  TLorentzVector Nu4Mom;
  TLorentzVector ChargedLepton4Mom;
  TLorentzVector StruckNuc4Mom;
};

namespace {
std::ostream &operator<<(std::ostream &os, LHAdditionInfoLine const &info) {
  return os << "{ "
            << ", Magic: " << info.Magic << ", EvId: " << info.EvId
            << ", EvWeight: " << info.EvWeight << ", Nu4Mom: " << info.Nu4Mom
            << ", ChargedLepton4Mom: " << info.ChargedLepton4Mom
            << ", StruckNuc4Mom: " << info.StruckNuc4Mom << " }";
}
}

LHAdditionInfoLine ParseAdditionInfoLine(std::string const &optLine) {
  std::string scrubbedLine = optLine.substr(2);  // scrub off the '# ';
  LHAdditionInfoLine info;

  auto const &splitLine = Utils::SplitStringByDelim(scrubbedLine, " ");
  if (splitLine.size() !=
      (11 + (GiBUUToStdHepOpts::HaveStruckNucleonInfo ? 4 : 0))) {
    std::cout << "[WARN]: Event had malformed additional info line: \""
              << optLine << "\"" << std::endl;
    return info;
  }

  try {
    info.Magic = std::stoi(splitLine[0]);
    info.EvId = std::stoi(splitLine[1]);
    info.EvWeight = std::stof(splitLine[2]);
    info.Nu4Mom[GiRooTracker::kStdHepIdxE] = std::stof(splitLine[3]);
    info.Nu4Mom[GiRooTracker::kStdHepIdxPx] = std::stof(splitLine[4]);
    info.Nu4Mom[GiRooTracker::kStdHepIdxPy] = std::stof(splitLine[5]);
    info.Nu4Mom[GiRooTracker::kStdHepIdxPz] = std::stof(splitLine[6]);
    info.ChargedLepton4Mom[GiRooTracker::kStdHepIdxE] = std::stof(splitLine[7]);
    info.ChargedLepton4Mom[GiRooTracker::kStdHepIdxPx] =
        std::stof(splitLine[8]);
    info.ChargedLepton4Mom[GiRooTracker::kStdHepIdxPy] =
        std::stof(splitLine[9]);
    info.ChargedLepton4Mom[GiRooTracker::kStdHepIdxPz] =
        std::stof(splitLine[10]);
    if (GiBUUToStdHepOpts::HaveStruckNucleonInfo) {
      info.StruckNuc4Mom[GiRooTracker::kStdHepIdxE] = std::stof(splitLine[11]);
      info.StruckNuc4Mom[GiRooTracker::kStdHepIdxPx] = std::stof(splitLine[12]);
      info.StruckNuc4Mom[GiRooTracker::kStdHepIdxPy] = std::stof(splitLine[13]);
      info.StruckNuc4Mom[GiRooTracker::kStdHepIdxPz] = std::stof(splitLine[14]);
    }
  } catch (const std::invalid_argument &ia) {
    std::cout << "[WARN]: Failed to parse one of the values: \"" << optLine
              << "\"" << std::endl;
    throw;
  }

  if (GiBUUToStdHepOpts::Verbosity > 2) {
    std::cout << "[PARSED]: " << info << std::endl;
  }

  return info;
}

int ParseLesHouchesFile(TTree *OutputTree, GiRooTracker *giRooTracker) {
  int EvNum = 0;
  LHPC::LhefParser LHEFParser(GiBUUToStdHepOpts::InpFName, true);

  LHPC::LHEF::LhefEvent const &currentEvent = LHEFParser.getEvent();
  while (LHEFParser.readNextEvent()) {
    auto const &ExtraInfo =
        ParseAdditionInfoLine(currentEvent.getOptionalInformation());

    giRooTracker->EvtNum = currentEvent.getEventNumberInFile();

    // neutrino
    giRooTracker->StdHepPdg[0] = GiBUUToStdHepOpts::nuType;
    giRooTracker->StdHepStatus[0] = -1;
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPx] =
        ExtraInfo.Nu4Mom.X();
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPy] =
        ExtraInfo.Nu4Mom.Y();
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPz] =
        ExtraInfo.Nu4Mom.Z();
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxE] = ExtraInfo.Nu4Mom.E();

    giRooTracker->GiBUUPerWeight = ExtraInfo.EvWeight;

    // target
    giRooTracker->StdHepPdg[1] = Utils::MakeNuclearPDG(
        GiBUUToStdHepOpts::TargetZ, GiBUUToStdHepOpts::TargetA);
    giRooTracker->StdHepStatus[1] = 11;
    giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPx] =
        ExtraInfo.StruckNuc4Mom.X();
    giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPy] =
        ExtraInfo.StruckNuc4Mom.Y();
    giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPz] =
        ExtraInfo.StruckNuc4Mom.Z();
    giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxE] =
        GiBUUToStdHepOpts::TargetA;

    // lepout
    giRooTracker->StdHepPdg[2] = (GiBUUToStdHepOpts::nuType - 1);  // you hope.
    giRooTracker->StdHepStatus[2] = 1;
    giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPx] =
        ExtraInfo.ChargedLepton4Mom.X();
    giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPy] =
        ExtraInfo.ChargedLepton4Mom.Y();
    giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPz] =
        ExtraInfo.ChargedLepton4Mom.Z();
    giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxE] =
        ExtraInfo.ChargedLepton4Mom.E();

    giRooTracker->GiBUUReactionCode = ExtraInfo.EvId;

    giRooTracker->StdHepN = 3;

    if (GiBUUToStdHepOpts::Verbosity > 1) {
      std::cout << "\n[INFO]: EvNo: " << currentEvent.getEventNumberInFile()
                << ", contained " << (currentEvent.getNumberOfParticles() + 3)
                << " particles."
                << "\n\tGiBUUReactionCode: " << giRooTracker->GiBUUReactionCode
                << ", NeutConventionReactionCode: "
                << "0"  // Not dealing with LH
                        // conversions at the moment.
                << "\n\t[Lep In] : " << ExtraInfo.Nu4Mom << std::endl;
      std::cout << "\t[Target]  : " << giRooTracker->StdHepPdg[1] << std::endl;
      if (GiBUUToStdHepOpts::HaveStruckNucleonInfo) {
        std::cout << "\t[Nuc In] : " << ExtraInfo.StruckNuc4Mom << std::endl;
      }
    }

    for (int i = 0; i < currentEvent.getNumberOfParticles(); ++i) {
      LHPC::LHEF::ParticleLine const &p = currentEvent.getLine(i + 1);

      TLorentzVector fourmom(p.getXMomentum(), p.getYMomentum(),
                             p.getZMomentum(), p.getEnergy());

      if (GiBUUToStdHepOpts::Verbosity > 1) {
        std::cout << "\t[" << std::setw(2) << i << "](" << std::setw(5)
                  << p.getParticleCode() << ")" << fourmom << std::endl;
      }

      giRooTracker->StdHepPdg[giRooTracker->StdHepN] = p.getParticleCode();
      giRooTracker->StdHepStatus[giRooTracker->StdHepN] = 1;
      giRooTracker->StdHepP4[giRooTracker->StdHepN]
                            [GiRooTracker::kStdHepIdxPx] = p.getXMomentum();
      giRooTracker->StdHepP4[giRooTracker->StdHepN]
                            [GiRooTracker::kStdHepIdxPy] = p.getYMomentum();
      giRooTracker->StdHepP4[giRooTracker->StdHepN]
                            [GiRooTracker::kStdHepIdxPz] = p.getZMomentum();
      giRooTracker->StdHepP4[giRooTracker->StdHepN][GiRooTracker::kStdHepIdxE] =
          p.getEnergy();

      giRooTracker->GiBUU2NeutCode = 0;
      giRooTracker->StdHepN++;
    }

    if (GiBUUToStdHepOpts::Verbosity > 1) {
      std::cout << "\t[Lep Out]: " << ExtraInfo.ChargedLepton4Mom << std::endl;
    }

    EvNum++;
    OutputTree->Fill();
    giRooTracker->Reset();

    if (GiBUUToStdHepOpts::MaxEntries == EvNum) {
      std::cout << "Finishing after " << EvNum << " entries." << std::endl;
      break;
    }
  }
  std::cout << "Read " << EvNum << " events." << std::endl;
  return 0;
}

int GiBUUToStdHep() {
  TFile *outFile = new TFile(GiBUUToStdHepOpts::OutFName.c_str(), "CREATE");
  if (!outFile->IsOpen()) {
    std::cout << "Couldn't open output file." << std::endl;
    return 2;
  }

  TTree *rooTrackerTree = new TTree(
      GiBUUToStdHepOpts::EmulateNuWro ? "nRooTracker" : "giRooTracker",
      "GiBUU StdHepVariables");
  GiRooTracker *giRooTracker = new GiRooTracker();
  giRooTracker->AddBranches(rooTrackerTree, GiBUUToStdHepOpts::InpIsFE,
                            GiBUUToStdHepOpts::EmulateNuWro &&
                                GiBUUToStdHepOpts::HaveStruckNucleonInfo,
                            GiBUUToStdHepOpts::EmulateNuWro);

  int ParserRtnCode = 0;
  if (GiBUUToStdHepOpts::InpIsLH) {
    ParserRtnCode = ParseLesHouchesFile(rooTrackerTree, giRooTracker);
  } else if (GiBUUToStdHepOpts::InpIsFE) {
    ParserRtnCode = ParseFinalEventsFile(rooTrackerTree, giRooTracker);
  }

  rooTrackerTree->Write();
  outFile->Write();
  outFile->Close();
  delete giRooTracker;
  giRooTracker = nullptr;
  delete outFile;
  outFile = nullptr;
  return ParserRtnCode;
}

void SetOpts() {
  CLIArgs::AddOpt(
      "-f", "--FEinput-file", true,
      [&](std::string const &opt) -> bool {
        std::cout << "\t--Reading FinalEvents-style GiBUU file : " << opt
                  << std::endl;
        GiBUUToStdHepOpts::InpFName = opt;
        GiBUUToStdHepOpts::InpIsFE = true;
        if (GiBUUToStdHepOpts::InpIsLH) {
          std::cerr
              << "[ERROR] only one style of input allowed, -l already used."
              << std::endl;
          throw 6;
        }
        return true;
      },
      false, []() { GiBUUToStdHepOpts::InpIsFE = false; }, "<File Name>");

  CLIArgs::AddOpt(
      "-l", "--LHinput-file", true,
      [&](std::string const &opt) -> bool {
        std::cout << "\t--Reading LesHouches Event Format GiBUU file : " << opt
                  << std::endl;
        GiBUUToStdHepOpts::InpFName = opt;
        GiBUUToStdHepOpts::InpIsLH = true;
        if (GiBUUToStdHepOpts::InpIsFE) {
          std::cerr
              << "[ERROR] only one style of input allowed, -f already used."
              << std::endl;
          throw 6;
        }
        return true;
      },
      false, []() { GiBUUToStdHepOpts::InpIsLH = false; }, "<File Name>");

  CLIArgs::AddOpt(
      "-o", "--output-file", true,
      [&](std::string const &opt) -> bool {
        GiBUUToStdHepOpts::OutFName = opt;
        std::cout << "\t--Writing to file " << opt << std::endl;
        return true;
      },
      false, []() { GiBUUToStdHepOpts::OutFName = "GiBUURooTracker.root"; },
      "<File Name {default:GiBUURooTracker.root}>");

  CLIArgs::AddOpt("-u", "--nu-pdg", true,
                  [&](std::string const &opt) -> bool {
                    int ival = 0;
                    try {
                      ival = Utils::str2i(opt, true);
                    } catch (...) {
                      return false;
                    }

                    std::cout << "\t--Nu PDG: " << ival << std::endl;
                    GiBUUToStdHepOpts::nuType = ival;
                    return true;
                  },
                  true, []() {}, "<Neutrino PDG identifier>");

  CLIArgs::AddOpt("-c", "--is-NC", false,
                  [&](std::string const &opt) -> bool {
                    std::cout << "\t--Assuming NC events. " << std::endl;
                    GiBUUToStdHepOpts::IsNC = true;
                    return true;
                  },
                  true, []() {}, "<Input is NC events>");

  CLIArgs::AddOpt("-a", "--target-a", true,
                  [&](std::string const &opt) -> bool {
                    int ival = 0;
                    try {
                      ival = Utils::str2i(opt, true);
                    } catch (...) {
                      return false;
                    }

                    std::cout << "\t--Target A: " << ival << std::endl;
                    GiBUUToStdHepOpts::TargetA = ival;
                    return true;

                  },
                  true, []() {}, "<Target A>");

  CLIArgs::AddOpt("-z", "--target-z", true,
                  [&](std::string const &opt) -> bool {
                    int ival = 0;
                    try {
                      ival = Utils::str2i(opt, true);
                    } catch (...) {
                      return false;
                    }
                    std::cout << "\t--Target Z: " << ival << std::endl;
                    GiBUUToStdHepOpts::TargetZ = ival;
                    return true;

                  },
                  true, []() {}, "<Target Z>");

  CLIArgs::AddOpt(
      "-v", "--GiBUUToStdHepOpts::verbosity", true,
      [&](std::string const &opt) -> bool {
        int ival = 0;
        try {
          ival = Utils::str2i(opt, true);
        } catch (...) {
          return false;
        }
        std::cout << "\t--GiBUUToStdHepOpts::Verbosity: " << ival << std::endl;
        GiBUUToStdHepOpts::Verbosity = ival;
        return true;
      },
      false, [&]() { GiBUUToStdHepOpts::Verbosity = 0; }, "<0-4>{default==0}");

  CLIArgs::AddOpt("-n", "--nevs", true,
                  [&](std::string const &opt) -> bool {
                    int ival = 0;
                    try {
                      ival = Utils::str2i(opt, true);
                    } catch (...) {
                      return false;
                    }
                    std::cout << "\t--Processing " << ival << " events."
                              << std::endl;
                    GiBUUToStdHepOpts::MaxEntries = ival;
                    return true;
                  },
                  false, [&]() { GiBUUToStdHepOpts::MaxEntries = -1; },
                  "<Num Entries [<-1>: means all]> [default==-1]");

  CLIArgs::AddOpt(
      "-I", "--have-Initial-State", false,
      [&](std::string const &opt) -> bool {
        GiBUUToStdHepOpts::HaveStruckNucleonInfo = true;
        std::cout << "\t--Attempting to read Initial State Info." << std::endl;
        return true;
      },
      false, [&]() { GiBUUToStdHepOpts::HaveStruckNucleonInfo = false; },
      "Have struck nucleon information in GiBUU output.");

  CLIArgs::AddOpt("-E", "--Emulate-NuWro", false,
                  [&](std::string const &opt) -> bool {
                    GiBUUToStdHepOpts::EmulateNuWro = true;
                    return true;
                    std::cout << "\t--Outputting in NuWro flavor StdHep."
                              << std::endl;
                  },
                  false, [&]() { GiBUUToStdHepOpts::EmulateNuWro = false; },
                  "Emulate NuWro StdHep Flavor.");

  CLIArgs::AddOpt("-S", "--Save-No-Weight", false,
                  [&](std::string const &opt) -> bool {
                    GiBUUToStdHepOpts::SaveNoWeight = true;
                    return true;
                    std::cout << "\t--Not saving events with 0 weight."
                              << std::endl;
                  },
                  false, [&]() { GiBUUToStdHepOpts::SaveNoWeight = false; },
                  "Save events that have a weight of 0.");
}

int main(int argc, char const *argv[]) {
  try {
    SetOpts();
  } catch (std::exception const &e) {
    std::cerr << "[ERROR]: " << e.what() << std::endl;
    return 1;
  }

  CLIArgs::AddArguments(argc, argv);
  if (!CLIArgs::HandleArgs()) {
    CLIArgs::SayRunLike();
    return 1;
  }

  return GiBUUToStdHep();
}
