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
std::vector<std::string> InpFNames;
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
std::vector<int> nuTypes;
///\brief The target nuclei nucleon number, A.
///
///\note Set by
///  `GiBUUToStdHep.exe ... -a xx ...'
/// Required.
std::vector<int> TargetAs;
///\brief The target nuclei proton number, Z.
///
///\note Set by
///  `GiBUUToStdHep.exe ... -z xx ...'
/// Required.
std::vector<int> TargetZs;
///\brief Whether input events are simulated NC interactions.
///
///\note Set by
///  `GiBUUToStdHep.exe ... -c ...'
std::vector<bool> CCFiles;
std::vector<float> FileExtraWeights;
float OverallWeight;
///\brief The maximum number of input entries to process.
///
///\note Set by
///  `GiBUUToStdHep.exe ... -n xx ...'
size_t MaxEntries;
///\brief The the debugging verbosity. From 0 (quiet) --- 4 (verbose).
///
///\note Set by
///  `GiBUUToStdHep.exe ... -v xx ...'
int Verbosity = 0;

///\brief Whether to produce NuWro flavor StdHep output.
bool EmulateNuWro = false;

bool HaveProdChargeInfo = false;

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
        Enu(0),
        ProdCharge(0) {}
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
  return os << " }";
}
}

GiBUUPartBlob GetParticleLine(std::string const &line) {
  GiBUUPartBlob pblob;

  std::vector<std::string> splitLine = Utils::SplitStringByDelim(line, " ");
  if (splitLine.size() !=
      (15 +
       size_t(GiBUUToStdHepOpts::HaveProdChargeInfo))) {  // try to fix known
                                                          // parsing error
    std::string ln =
        Utils::Replace(line, "E-", "XXXXX");  // Guard any exponential notation
    ln = Utils::Replace(ln, "-", " -");
    ln = Utils::Replace(ln, "XXXXX", "E-");

    splitLine = Utils::SplitStringByDelim(ln, " ");
    if (splitLine.size() !=
        (15 + size_t(GiBUUToStdHepOpts::HaveProdChargeInfo))) {
      std::cout << "[WARN]: Event had malformed particle line: \"" << line
                << "\"" << std::endl;
      return pblob;
    }
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
    if (GiBUUToStdHepOpts::HaveProdChargeInfo) {
      pblob.ProdCharge = std::stoi(splitLine[15]);
    }
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
  std::map<std::string, std::vector<std::vector<GiBUUPartBlob> > > Events;
  // http://www2.research.att.com/~bs/bs_faq2.html
  // People sometimes worry about the cost of std::vector growing incrementally.
  // I used to worry about that and used reserve() to optimize the growth.
  // After measuring my code and repeatedly having trouble finding the
  // performance benefits of reserve() in real programs, I stopped using it
  // except where it is needed to avoid iterator invalidation (a rare case in my
  // code). Again: measure before you optimize.

  size_t ParsedEvs = 0;
  for (auto &fname : GiBUUToStdHepOpts::InpFNames) {
    std::ifstream ifs(fname);

    if (!ifs.good()) {
      std::cerr << "[ERROR]: Failed to open " << fname << " for reading."
                << std::endl;
      return 1;
    }

    auto &FileEvents = Events[fname];

    std::string line;
    std::vector<GiBUUPartBlob> CurrEv;
    size_t LastEvNum = 0;
    size_t LineNum = 0;
    while (std::getline(ifs, line)) {
      if (GiBUUToStdHepOpts::Verbosity > 3) {
        std::cout << "[LINE:" << LineNum << "]: " << line << std::endl;
      }
      if (line[0] == '#') {  // Skip comments
        continue;
        LineNum++;
      }
      auto const &part = GetParticleLine(line);

      if ((part.PerWeight == 0) &&
          (!GiBUUToStdHepOpts::HaveStruckNucleonInfo) &&
          (GiBUUToStdHepOpts::Verbosity > -1)) {
        std::cout << "[WARN]: Found particle with 0 weight, but do not have "
                     "initial state information enabled (-v -1 to silence this "
                     "message)."
                  << std::endl;
      }

      if ((part.EvNum != int(LastEvNum)) && LastEvNum) {
        FileEvents.push_back(CurrEv);
        ParsedEvs++;
        CurrEv.clear();
        if (GiBUUToStdHepOpts::MaxEntries == ParsedEvs) {
          break;
        }
      }
      CurrEv.push_back(part);
      LastEvNum = part.EvNum;
      LineNum++;
    }
    if (CurrEv.size()) {
      FileEvents.push_back(CurrEv);
      ParsedEvs++;
      CurrEv.clear();
    }

    ifs.close();  // Read all the lines.
    std::cout << "Found " << FileEvents.size() << " events in " << fname << "."
              << std::endl;
  }

  size_t NumEvs = 0;
  size_t fn = 0;
  for (auto const &FileEvents : Events) {
    double NRunsScaleFactor = FileEvents.second.back().back().Run;
    bool FileIsCC = GiBUUToStdHepOpts::CCFiles[fn];
    int FileNuType = GiBUUToStdHepOpts::nuTypes[fn];
    int FileTargetA = GiBUUToStdHepOpts::TargetAs[fn];
    int FileTargetZ = GiBUUToStdHepOpts::TargetZs[fn];
    double FileExtraWeight = GiBUUToStdHepOpts::FileExtraWeights[fn];
    for (auto const &ev : FileEvents.second) {
      giRooTracker->Reset();

      int const &EvNum = ev.front().EvNum;

      if (!EvNum) {  // Malformed line
        if (GiBUUToStdHepOpts::Verbosity > 0) {
          std::cout << "Skipping event due to malformed line." << std::endl;
        }
        continue;
      }

      if (!(NumEvs % 10000) && (GiBUUToStdHepOpts::Verbosity > 0)) {
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
      giRooTracker->StdHepPdg[0] = FileNuType;
      giRooTracker->StdHepStatus[0] = 0;
      giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPx] = 0;
      giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPy] = 0;
      giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPz] = ev.front().Enu;
      giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxE] = ev.front().Enu;

      // target
      giRooTracker->StdHepPdg[1] =
          Utils::MakeNuclearPDG(FileTargetZ, FileTargetA);
      giRooTracker->StdHepStatus[1] = 0;
      if (!GiBUUToStdHepOpts::EmulateNuWro) {
        giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPx] = 0;
        giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPy] = 0;
        giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPz] = 0;
        giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxE] = FileTargetA;
      }

      giRooTracker->GiBUUReactionCode = ev.front().Prodid;
      if (GiBUUToStdHepOpts::HaveProdChargeInfo) {
        giRooTracker->GiBUUPrimaryParticleCharge = ev.front().ProdCharge;
      }
      giRooTracker->GiBUUPerWeight = ev.front().PerWeight;
      giRooTracker->NumRunsWeight = 1.0 / NRunsScaleFactor;
      giRooTracker->ExtraWeight = FileExtraWeight;
      giRooTracker->EvtWght =
          giRooTracker->GiBUUPerWeight * giRooTracker->NumRunsWeight *
          giRooTracker->ExtraWeight * *GiBUUToStdHepOpts::OverallWeight;

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
            (giRooTracker->StdHepN == 3) &&
            (GiBUUToStdHepOpts::EmulateNuWro &&
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
          giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxE] =
              part.FourMom.E();
          continue;

        } else if (GiBUUToStdHepOpts::HaveStruckNucleonInfo &&
                   (giRooTracker->StdHepN ==
                    3)) {  // Struck nucleon status if not
          giRooTracker->StdHepStatus[giRooTracker->StdHepN] =
              11;  // in NuWro mode.
        } else {
          giRooTracker->StdHepStatus[giRooTracker->StdHepN] =
              1;  // All other FS
        }         // should be good.

        giRooTracker->StdHepPdg[giRooTracker->StdHepN] =
            GiBUUUtils::GiBUUToPDG(part.ID, part.Charge);

        giRooTracker->StdHepP4[giRooTracker->StdHepN]
                              [GiRooTracker::kStdHepIdxPx] = part.FourMom.X();
        giRooTracker->StdHepP4[giRooTracker->StdHepN]
                              [GiRooTracker::kStdHepIdxPy] = part.FourMom.Y();
        giRooTracker->StdHepP4[giRooTracker->StdHepN]
                              [GiRooTracker::kStdHepIdxPz] = part.FourMom.Z();
        giRooTracker->StdHepP4[giRooTracker->StdHepN]
                              [GiRooTracker::kStdHepIdxE] = part.FourMom.E();

        giRooTracker->GiBHepHistory[giRooTracker->StdHepN] = part.History;
        auto const &hDec = GiBUUUtils::DecomposeGiBUUHistory(part.History);
        giRooTracker->GiBHepGeneration[giRooTracker->StdHepN] =
            std::get<0>(hDec);

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
          (giRooTracker->StruckNucleonPDG == (FileNuType > 0 ? 2212 : 2112))) {
        giRooTracker->GiBUU2NeutCode = 11;  // Special case that we know.
      } else {                              // Try the heuristics.
        giRooTracker->GiBUU2NeutCode = GiBUUUtils::GiBUU2NeutReacCode(
            giRooTracker->GiBUUReactionCode, giRooTracker->StdHepPdg,
            giRooTracker->GiBHepHistory, giRooTracker->StdHepN, FileIsCC,
            (GiBUUToStdHepOpts::HaveStruckNucleonInfo &&
             (!GiBUUToStdHepOpts::EmulateNuWro))
                ? 3
                : -1,
            GiBUUToStdHepOpts::HaveProdChargeInfo
                ? giRooTracker->GiBUUPrimaryParticleCharge
                : -10);
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
                  << "\n\tGiBUUReactionCode: "
                  << giRooTracker->GiBUUReactionCode
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
          std::cout << "\t[" << std::setw(2) << (stdHepInd - (StartPoint))
                    << "](" << std::setw(5)
                    << giRooTracker->StdHepPdg[stdHepInd] << ")"
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
    fn++;
  }
  std::cout << "[INFO]: Saved: " << NumEvs
            << " events, skipped: " << (ParsedEvs - NumEvs)
            << " because of low weight"
            << (((ParsedEvs - NumEvs) > 1) ? "s" : "") << "." << std::endl;
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
  return 1;
  // size_t EvNum = 0;
  // LHPC::LhefParser LHEFParser(GiBUUToStdHepOpts::InpFNames.front(), true);

  // LHPC::LHEF::LhefEvent const &currentEvent = LHEFParser.getEvent();
  // while (LHEFParser.readNextEvent()) {
  //   auto const &ExtraInfo =
  //       ParseAdditionInfoLine(currentEvent.getOptionalInformation());

  //   giRooTracker->EvtNum = currentEvent.getEventNumberInFile();

  //   // neutrino
  //   giRooTracker->StdHepPdg[0] = GiBUUToStdHepOpts::nuType;
  //   giRooTracker->StdHepStatus[0] = -1;
  //   giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPx] =
  //       ExtraInfo.Nu4Mom.X();
  //   giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPy] =
  //       ExtraInfo.Nu4Mom.Y();
  //   giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPz] =
  //       ExtraInfo.Nu4Mom.Z();
  //   giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxE] =
  //   ExtraInfo.Nu4Mom.E();

  //   giRooTracker->GiBUUPerWeight = ExtraInfo.EvWeight;

  //   // target
  //   giRooTracker->StdHepPdg[1] = Utils::MakeNuclearPDG(
  //       GiBUUToStdHepOpts::TargetZ, GiBUUToStdHepOpts::TargetA);
  //   giRooTracker->StdHepStatus[1] = 11;
  //   giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPx] =
  //       ExtraInfo.StruckNuc4Mom.X();
  //   giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPy] =
  //       ExtraInfo.StruckNuc4Mom.Y();
  //   giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPz] =
  //       ExtraInfo.StruckNuc4Mom.Z();
  //   giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxE] =
  //       GiBUUToStdHepOpts::TargetA;

  //   // lepout
  //   giRooTracker->StdHepPdg[2] = (GiBUUToStdHepOpts::nuTypes[0] - 1);  // you
  //   hope.
  //   giRooTracker->StdHepStatus[2] = 1;
  //   giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPx] =
  //       ExtraInfo.ChargedLepton4Mom.X();
  //   giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPy] =
  //       ExtraInfo.ChargedLepton4Mom.Y();
  //   giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPz] =
  //       ExtraInfo.ChargedLepton4Mom.Z();
  //   giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxE] =
  //       ExtraInfo.ChargedLepton4Mom.E();

  //   giRooTracker->GiBUUReactionCode = ExtraInfo.EvId;

  //   giRooTracker->StdHepN = 3;

  //   if (GiBUUToStdHepOpts::Verbosity > 1) {
  //     std::cout << "\n[INFO]: EvNo: " << currentEvent.getEventNumberInFile()
  //               << ", contained " << (currentEvent.getNumberOfParticles() +
  //               3)
  //               << " particles."
  //               << "\n\tGiBUUReactionCode: " <<
  //               giRooTracker->GiBUUReactionCode
  //               << ", NeutConventionReactionCode: "
  //               << "0"  // Not dealing with LH
  //                       // conversions at the moment.
  //               << "\n\t[Lep In] : " << ExtraInfo.Nu4Mom << std::endl;
  //     std::cout << "\t[Target]  : " << giRooTracker->StdHepPdg[1] <<
  //     std::endl;
  //     if (GiBUUToStdHepOpts::HaveStruckNucleonInfo) {
  //       std::cout << "\t[Nuc In] : " << ExtraInfo.StruckNuc4Mom << std::endl;
  //     }
  //   }

  //   for (size_t i = 0; i < size_t(currentEvent.getNumberOfParticles()); ++i)
  //   {
  //     LHPC::LHEF::ParticleLine const &p = currentEvent.getLine(i + 1);

  //     TLorentzVector fourmom(p.getXMomentum(), p.getYMomentum(),
  //                            p.getZMomentum(), p.getEnergy());

  //     if (GiBUUToStdHepOpts::Verbosity > 1) {
  //       std::cout << "\t[" << std::setw(2) << i << "](" << std::setw(5)
  //                 << p.getParticleCode() << ")" << fourmom << std::endl;
  //     }

  //     giRooTracker->StdHepPdg[giRooTracker->StdHepN] = p.getParticleCode();
  //     giRooTracker->StdHepStatus[giRooTracker->StdHepN] = 1;
  //     giRooTracker->StdHepP4[giRooTracker->StdHepN]
  //                           [GiRooTracker::kStdHepIdxPx] = p.getXMomentum();
  //     giRooTracker->StdHepP4[giRooTracker->StdHepN]
  //                           [GiRooTracker::kStdHepIdxPy] = p.getYMomentum();
  //     giRooTracker->StdHepP4[giRooTracker->StdHepN]
  //                           [GiRooTracker::kStdHepIdxPz] = p.getZMomentum();
  //     giRooTracker->StdHepP4[giRooTracker->StdHepN][GiRooTracker::kStdHepIdxE]
  //     =
  //         p.getEnergy();

  //     giRooTracker->GiBUU2NeutCode = 0;
  //     giRooTracker->StdHepN++;
  //   }

  //   if (GiBUUToStdHepOpts::Verbosity > 1) {
  //     std::cout << "\t[Lep Out]: " << ExtraInfo.ChargedLepton4Mom <<
  //     std::endl;
  //   }

  //   EvNum++;
  //   OutputTree->Fill();
  //   giRooTracker->Reset();

  //   if (GiBUUToStdHepOpts::MaxEntries == EvNum) {
  //     std::cout << "Finishing after " << EvNum << " entries." << std::endl;
  //     break;
  //   }
  // }
  // std::cout << "Read " << EvNum << " events." << std::endl;
  // return 0;
}

int GiBUUToStdHep() {
  TFile *outFile = new TFile(GiBUUToStdHepOpts::OutFName.c_str(), "RECREATE");
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
                            GiBUUToStdHepOpts::EmulateNuWro,
                            GiBUUToStdHepOpts::HaveProdChargeInfo);

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
        GiBUUToStdHepOpts::InpFNames.push_back(opt);
        GiBUUToStdHepOpts::InpIsFE = true;
        if (GiBUUToStdHepOpts::InpIsLH) {
          std::cerr
              << "[ERROR] only one style of input allowed, -l already used."
              << std::endl;
          return false;
        }
        if (GiBUUToStdHepOpts::CCFiles.size() <
            GiBUUToStdHepOpts::InpFNames.size()) {
          GiBUUToStdHepOpts::CCFiles.push_back(true);
          std::cout << "\t\t--File assumed CC events." << std::endl;
        }

        if ((!GiBUUToStdHepOpts::nuTypes.size()) ||
            (!GiBUUToStdHepOpts::TargetAs.size()) ||
            (!GiBUUToStdHepOpts::TargetZs.size())) {
          std::cerr << "[ERROR]: -u X -a Y -z Z must be specified before the "
                       "first input file."
                    << std::endl;
          return false;
        }

        if (GiBUUToStdHepOpts::nuTypes.size() <
            GiBUUToStdHepOpts::InpFNames.size()) {
          GiBUUToStdHepOpts::nuTypes.push_back(
              GiBUUToStdHepOpts::nuTypes.back());
          std::cout << "\t\t--File assumed Nu PDG: "
                    << GiBUUToStdHepOpts::nuTypes.back() << std::endl;
        }
        if (GiBUUToStdHepOpts::TargetAs.size() <
            GiBUUToStdHepOpts::InpFNames.size()) {
          GiBUUToStdHepOpts::TargetAs.push_back(
              GiBUUToStdHepOpts::TargetAs.back());
          std::cout << "\t\t--File assumed target a: "
                    << GiBUUToStdHepOpts::TargetAs.back() << std::endl;
        }
        if (GiBUUToStdHepOpts::TargetZs.size() <
            GiBUUToStdHepOpts::InpFNames.size()) {
          GiBUUToStdHepOpts::TargetZs.push_back(
              GiBUUToStdHepOpts::TargetZs.back());
          std::cout << "\t\t--File assumed target z: "
                    << GiBUUToStdHepOpts::TargetZs.back() << std::endl;
        }
        if (GiBUUToStdHepOpts::FileExtraWeights.size() <
            GiBUUToStdHepOpts::InpFNames.size()) {
          GiBUUToStdHepOpts::FileExtraWeights.push_back(1.0);
          std::cout << "\t\t--File assumed weight is: 1.0" << std::endl;
        }
        return true;
      },
      false, []() { GiBUUToStdHepOpts::InpIsFE = false; }, "<File Name>");

  CLIArgs::AddOpt(
      "-l", "--LHinput-file", true,
      [&](std::string const &opt) -> bool {
        std::cerr << "LesHouches format is currently disabled." << std::endl;
        return false;
        std::cout << "\t--Reading LesHouches Event Format GiBUU file : " << opt
                  << std::endl;
        GiBUUToStdHepOpts::InpFNames.push_back(opt);
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

                    std::cout << "\t--Assuming next file has Nu PDG: " << ival
                              << std::endl;
                    GiBUUToStdHepOpts::nuTypes.push_back(ival);
                    return true;
                  },
                  true, []() {}, "<Next file neutrino PDG code>");

  CLIArgs::AddOpt("-N", "--is-NC", false,
                  [&](std::string const &opt) -> bool {
                    std::cout << "\t--Assuming next file contains NC events. "
                              << std::endl;
                    GiBUUToStdHepOpts::CCFiles.push_back(false);
                    return true;
                  },
                  false, []() {}, "<Next file is NC events>");

  CLIArgs::AddOpt("-a", "--target-a", true,
                  [&](std::string const &opt) -> bool {
                    int ival = 0;
                    try {
                      ival = Utils::str2i(opt, true);
                    } catch (...) {
                      return false;
                    }

                    std::cout << "\t--Assuming next file is target A: " << ival
                              << std::endl;
                    GiBUUToStdHepOpts::TargetAs.push_back(ival);
                    return true;

                  },
                  true, []() {}, "<Next file target nucleus 'A'>");

  CLIArgs::AddOpt("-z", "--target-z", true,
                  [&](std::string const &opt) -> bool {
                    int ival = 0;
                    try {
                      ival = Utils::str2i(opt, true);
                    } catch (...) {
                      return false;
                    }
                    std::cout << "\t--Assuming next file is target Z: " << ival
                              << std::endl;
                    GiBUUToStdHepOpts::TargetZs.push_back(ival);
                    return true;

                  },
                  true, []() {}, "<Next file target nucleus 'Z'>");

  CLIArgs::AddOpt("-W", "--file-weight", true,
                  [&](std::string const &opt) -> bool {
                    double ival = 0;
                    bool IsReciprocal = false;
                    std::string inp = opt;
                    if (inp[0] == 'i') {
                      IsReciprocal = true;
                      inp = Utils::Replace(inp, "i", "");
                    }

                    try {
                      ival = Utils::str2d(inp, true);
                      if (IsReciprocal) {
                        ival = (1.0 / ival);
                      }
                    } catch (...) {
                      return false;
                    }
                    std::cout << "\t--Assigning next file weight: " << ival
                              << std::endl;
                    GiBUUToStdHepOpts::FileExtraWeights.push_back(ival);
                    return true;
                  },
                  false, []() {}, "[i]<Next file extra weight [1.0/]'W'>");

  CLIArgs::AddOpt("-R", "--Total-ReWeight", true,
                  [&](std::string const &opt) -> bool {
                    double ival = 0;
                    bool IsReciprocal = false;
                    std::string inp = opt;
                    if (inp[0] == 'i') {
                      IsReciprocal = true;
                      inp = Utils::Replace(inp, "i", "");
                    }

                    try {
                      ival = Utils::str2d(inp, true);
                      if (IsReciprocal) {
                        ival = (1.0 / ival);
                      }
                    } catch (...) {
                      return false;
                    }
                    std::cout << "\t--Assigning overall weight: " << ival
                              << std::endl;
                    GiBUUToStdHepOpts::OverallWeight = ival;
                    return true;
                  },
                  false, []() {}, "[i]<Next file extra weight [1.0/]'W'>");

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
        std::cout << "\t--Attempting to read initial state Info." << std::endl;
        return true;
      },
      false, [&]() { GiBUUToStdHepOpts::HaveStruckNucleonInfo = false; },
      "Have struck nucleon information in GiBUU output.");

  CLIArgs::AddOpt(
      "-P", "--have-Prod-Charge", false,
      [&](std::string const &opt) -> bool {
        GiBUUToStdHepOpts::HaveProdChargeInfo = true;
        std::cout << "\t--Attempting to read prod charge Info." << std::endl;
        return true;
      },
      false, [&]() { GiBUUToStdHepOpts::HaveProdChargeInfo = false; },
      "Have primary particle charge information in GiBUU output.");

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

  if (GiBUUToStdHepOpts::CCFiles.size() !=
      GiBUUToStdHepOpts::InpFNames.size()) {
    std::cerr << "[ERROR]: found " << GiBUUToStdHepOpts::CCFiles.size()
              << " CC/NC event markers and "
              << GiBUUToStdHepOpts::InpFNames.size() << " input file names."
              << std::endl
              << "N.B. the -N argument specifies that the value of the next "
                 "-f/-l argument is an NC file---argument position is "
                 "important."
              << std::endl;
    return 1;
  }

  return GiBUUToStdHep();
}
