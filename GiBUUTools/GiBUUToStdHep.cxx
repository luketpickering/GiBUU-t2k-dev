#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

// Unix
#include <dirent.h>
#include <unistd.h>

#include "TFile.h"
#include "TH1D.h"
#include "TLorentzVector.h"
#include "TRegexp.h"
#include "TTree.h"
#include "TVector3.h"

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
float OverallWeight = 1;
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

std::vector<std::pair<std::string, std::string> > FluxFilesToAdd;
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
  std::vector<std::vector<GiBUUPartBlob> > FileEvents;
  // http://www2.research.att.com/~bs/bs_faq2.html
  // People sometimes worry about the cost of std::vector growing incrementally.
  // I used to worry about that and used reserve() to optimize the growth.
  // After measuring my code and repeatedly having trouble finding the
  // performance benefits of reserve() in real programs, I stopped using it
  // except where it is needed to avoid iterator invalidation (a rare case in my
  // code). Again: measure before you optimize.

  size_t ParsedEvs = 0;
  size_t fileNumber = 0;
  size_t NumEvs = 0;

  for (auto &fname : GiBUUToStdHepOpts::InpFNames) {
    std::ifstream ifs(fname);

    if (!ifs.good()) {
      std::cerr << "[ERROR]: Failed to open " << fname << " for reading."
                << std::endl;
      return 1;
    }

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

    double NRunsScaleFactor = FileEvents.back().back().Run;
    bool FileIsCC = GiBUUToStdHepOpts::CCFiles[fileNumber];
    int FileNuType = GiBUUToStdHepOpts::nuTypes[fileNumber];
    int FileTargetA = GiBUUToStdHepOpts::TargetAs[fileNumber];
    int FileTargetZ = GiBUUToStdHepOpts::TargetZs[fileNumber];
    double FileExtraWeight = GiBUUToStdHepOpts::FileExtraWeights[fileNumber];

    for (auto const &ev : FileEvents) {
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
          giRooTracker->ExtraWeight * GiBUUToStdHepOpts::OverallWeight;

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
        if (giRooTracker->StdHepN == (GiRooTracker::kGiStdHepNPmax - 1)) {
          std::cerr << "[ERROR]: In file " << fname << ", event " << EvNum
                    << " contained to many final state particles " << ev.size()
                    << ". Ignoring the last: "
                    << (ev.size() - GiRooTracker::kGiStdHepNPmax) << std::endl;
          break;
        }
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
    FileEvents.clear();
    fileNumber++;
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

void SaveFluxFile(std::string const &fileloc, std::string const &histname) {
  std::ifstream ifs(fileloc);
  if (!ifs.good()) {
    std::cerr << "[ERROR]: File \"" << fileloc
              << " could not be opened for reading." << std::endl;
    return;
  }
  std::string line;

  size_t ln = 0;
  std::vector<std::pair<float, float> > FluxValues;
  while (std::getline(ifs, line)) {
    if (line[0] == '#') {  // ignore comments
      ln++;
      continue;
    }
    std::vector<float> splitLine =
        Utils::StringVToFloatV(Utils::SplitStringByDelim(line, " "));
    if (splitLine.size() != 2) {
      std::cout << "[WARN]: ingoring line: \"" << line
                << "\" in input flux file." << std::endl;
      continue;
    }
    FluxValues.push_back(std::make_pair(splitLine.front(), splitLine.back()));
  }
  ifs.close();

  std::unique_ptr<float[]> BinLowEdges(new float[FluxValues.size() + 1]);
  for (size_t bin_it = 1; bin_it < FluxValues.size(); ++bin_it) {
    BinLowEdges[bin_it] =
        FluxValues[bin_it - 1].first +
        (FluxValues[bin_it].first - FluxValues[bin_it - 1].first) / 2.0;
  }
  BinLowEdges[0] = FluxValues[0].first - (BinLowEdges[1] - FluxValues[0].first);
  BinLowEdges[FluxValues.size()] = FluxValues[FluxValues.size() - 1].first +
                                   (FluxValues[FluxValues.size() - 1].first -
                                    BinLowEdges[FluxValues.size() - 1]);

  TH1D *fluxHist = new TH1D(
      histname.c_str(), (histname + ";#it{E}_{#nu} (GeV);#Phi (A.U.)").c_str(),
      FluxValues.size(), BinLowEdges.get());

  for (Int_t bin_it = 1; bin_it < fluxHist->GetNbinsX() + 1; bin_it++) {
    fluxHist->SetBinContent(bin_it, FluxValues[bin_it - 1].second);
  }

  fluxHist->Write();
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
  giRooTracker->AddBranches(
      rooTrackerTree, true, GiBUUToStdHepOpts::EmulateNuWro &&
                                GiBUUToStdHepOpts::HaveStruckNucleonInfo,
      GiBUUToStdHepOpts::EmulateNuWro, GiBUUToStdHepOpts::HaveProdChargeInfo);

  int ParserRtnCode = 0;
  ParserRtnCode = ParseFinalEventsFile(rooTrackerTree, giRooTracker);

  rooTrackerTree->Write();

  for (auto const &ff : GiBUUToStdHepOpts::FluxFilesToAdd) {
    SaveFluxFile(ff.second, ff.first);
  }

  outFile->Write();
  outFile->Close();
  delete giRooTracker;
  giRooTracker = nullptr;
  delete outFile;
  outFile = nullptr;
  return ParserRtnCode;
}

bool AddFiles(std::string const &OptVal, bool IsCC, int NuType, int TargetA,
              int TargetZ, float FileExtraWeight) {
  size_t AsteriskPos = OptVal.find_last_of('*');
  if (AsteriskPos == std::string::npos) {
    std::cout << "\t--Adding file: " << OptVal << std::endl;
    GiBUUToStdHepOpts::InpFNames.push_back(OptVal);
    GiBUUToStdHepOpts::nuTypes.push_back(NuType);
    GiBUUToStdHepOpts::TargetAs.push_back(TargetA);
    GiBUUToStdHepOpts::TargetZs.push_back(TargetZ);
    GiBUUToStdHepOpts::CCFiles.push_back(IsCC);
    GiBUUToStdHepOpts::FileExtraWeights.push_back(FileExtraWeight);
    return true;
  }

  DIR *dir;
  struct dirent *ent;
  size_t lastFSlash = OptVal.find_last_of('/');
  std::string matchPat = OptVal.substr(lastFSlash + 1);
  std::string dirpath = "";
  if (lastFSlash == std::string::npos) {
    std::unique_ptr<char[]> cwd(new char[1000]);
    getcwd(cwd.get(), sizeof(char) * 1000);
    std::cout << "\t--Looking in current directory (" << cwd.get()
              << ") for matching (\"" << matchPat << "\") files." << std::endl;
    dirpath = "./";
  } else {
    if (AsteriskPos < lastFSlash) {
      std::cerr << "[ERROR]: Currently cannot handle a wildcard in the "
                   "directory structure. Please put input files in the same "
                   "directory or use separate -f arguments. Expected -f "
                   "\"../some/rel/path/*.dat\""
                << std::endl;
      return false;
    }
    dirpath = OptVal.substr(0, lastFSlash + 1);
    std::cout << "\t--Looking in directory (" << dirpath
              << ") for matching files." << std::endl;
  }
  dir = opendir(dirpath.c_str());

  if (dir != NULL) {
    TRegexp matchExp(matchPat.c_str(), true);
    /* print all the files and directories within directory */
    Ssiz_t len = 0;
    size_t NFilesAdded = 0;
    while ((ent = readdir(dir)) != NULL) {
      if (matchExp.Index(TString(ent->d_name), &len) != Ssiz_t(-1)) {
        std::cout << "\t\t\tAdding matching file: " << (dirpath + ent->d_name)
                  << "(nu: " << NuType << ", A: " << TargetA
                  << ", Z: " << TargetZ << ", W: " << FileExtraWeight
                  << ", IsCC: " << IsCC << ")" << std::endl;
        GiBUUToStdHepOpts::InpFNames.push_back((dirpath + ent->d_name));
        GiBUUToStdHepOpts::nuTypes.push_back(NuType);
        GiBUUToStdHepOpts::TargetAs.push_back(TargetA);
        GiBUUToStdHepOpts::TargetZs.push_back(TargetZ);
        GiBUUToStdHepOpts::CCFiles.push_back(IsCC);
        NFilesAdded++;
      }
    }
    closedir(dir);

    for (size_t file_it = 0; file_it < NFilesAdded; ++file_it) {
      GiBUUToStdHepOpts::FileExtraWeights.push_back(FileExtraWeight /
                                                    double(NFilesAdded));
    }
    std::cout << "[INFO]: Added " << NFilesAdded << " overall file weight: "
              << (FileExtraWeight / double(NFilesAdded)) << std::endl;
  } else {
    /* could not open directory */
    perror("");
    return false;
  }
  return true;
}

void SetOpts() {
  CLIArgs::AddOpt(
      "-c", "--CompositeExample", false,
      [&](std::string const &opt) -> bool {
        std::cout
            << "[RUNLIKE]: To combine C runs and H runs for a correctly "
               "weighted (per nucleon) CH2 target with CC and [NC] "
               "events and "
               "neutrino and {antineutrino} beam components"
               ":\n[RUNLIKE]:\tGiBUUToStdHep.exe -u 14 -a 12 -z 6 -W "
               "12 -f \"FinalEvents_nu_C_CC_*.dat\" [-N -W 12 -f "
               "\"FinalEvents_nu_C_NC_*.dat\"] -a 1 -z 1 -W 2 -f "
               "\"FinalEvents_nu_H_CC_*.dat\" [-N -W 2 -f "
               "\"FinalEvents_nu_H_NC_*.dat\"] {-u -14 -a 12 -z 6 -W "
               "12 -f \"FinalEvents_nub_C_CC_*.dat\" [-N -W 12 -f "
               "\"FinalEvents_nub_C_NC_*.dat\"] -a 1 -z 1 -W 2 -f "
               "\"FinalEvents_nub_H_CC_*.dat\" [-N -W 2 -f "
               "\"FinalEvents_nub_H_NC_*.dat\"]} -R i14\n[RUNLIKE]: This "
               "will weight carbon events up by 12 and hydrogen events up by "
               "2, before weighting all events down by 14 to rescale the "
               "produced weights to cross section per nucleon."
            << std::endl;
        exit(0);
      },
      false, []() {},
      "<Write example of how to combine GiBUU runs for a composite target>");

  CLIArgs::AddOpt(
      "-f", "--FEinput-file", true,
      [&](std::string const &opt) -> bool {

        bool IsCC = true;
        // If specified as NC take that, otherwise, assume CC
        if (GiBUUToStdHepOpts::CCFiles.size() >
            GiBUUToStdHepOpts::InpFNames.size()) {
          IsCC = GiBUUToStdHepOpts::CCFiles.back();
          GiBUUToStdHepOpts::CCFiles.pop_back();
        }

        if ((!GiBUUToStdHepOpts::nuTypes.size()) ||
            (!GiBUUToStdHepOpts::TargetAs.size()) ||
            (!GiBUUToStdHepOpts::TargetZs.size())) {
          std::cerr << "[ERROR]: -u X -a Y -z Z must be specified before the "
                       "first input file."
                    << std::endl;
          return false;
        }

        std::cout
            << "\t--Reading FinalEvents-style GiBUU file(s) from descriptor: \""
            << opt << "\"" << std::endl;

        // Assume previous or latest specified otherwise.
        int NuType = GiBUUToStdHepOpts::nuTypes.back();
        int TargetA = GiBUUToStdHepOpts::TargetAs.back();
        int TargetZ = GiBUUToStdHepOpts::TargetZs.back();

        // These get added back by AddFiles so that the number of files and
        // options are synched
        if (GiBUUToStdHepOpts::nuTypes.size() >
            GiBUUToStdHepOpts::InpFNames.size()) {
          GiBUUToStdHepOpts::nuTypes.pop_back();
        }
        if (GiBUUToStdHepOpts::TargetAs.size() >
            GiBUUToStdHepOpts::InpFNames.size()) {
          GiBUUToStdHepOpts::TargetAs.pop_back();
        }
        if (GiBUUToStdHepOpts::TargetZs.size() >
            GiBUUToStdHepOpts::InpFNames.size()) {
          GiBUUToStdHepOpts::TargetZs.pop_back();
        }

        float FileExtraWeight = 1.0;
        if (GiBUUToStdHepOpts::FileExtraWeights.size() >
            GiBUUToStdHepOpts::InpFNames.size()) {
          FileExtraWeight = GiBUUToStdHepOpts::FileExtraWeights.back();
          GiBUUToStdHepOpts::FileExtraWeights.pop_back();
        }

        size_t NFilesAdded =
            AddFiles(opt, IsCC, NuType, TargetA, TargetZ, FileExtraWeight);

        return NFilesAdded;
      },
      true, []() {}, "<File Name>");

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

                    if (GiBUUToStdHepOpts::nuTypes.size() >
                        GiBUUToStdHepOpts::InpFNames.size()) {
                      std::cerr << "[ERROR]: Found another -u option before "
                                   "the next file has been specified."
                                << std::endl;
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

                    if (GiBUUToStdHepOpts::CCFiles.size() >
                        GiBUUToStdHepOpts::InpFNames.size()) {
                      std::cerr << "[ERROR]: Found another -N option before "
                                   "the next file has been specified."
                                << std::endl;
                      return false;
                    }

                    std::cout << "\t--Assuming next files contains NC event."
                              << std::endl;
                    GiBUUToStdHepOpts::CCFiles.push_back(false);
                    return true;
                  },
                  false, []() {}, "<Next file is NC events>");

  CLIArgs::AddOpt("-a", "--target-a", true,
                  [&](std::string const &opt) -> bool {

                    if (GiBUUToStdHepOpts::TargetAs.size() >
                        GiBUUToStdHepOpts::InpFNames.size()) {
                      std::cerr << "[ERROR]: Found another -a option before "
                                   "the next file has been specified."
                                << std::endl;
                      return false;
                    }

                    int ival = 0;
                    try {
                      ival = Utils::str2i(opt, true);
                    } catch (...) {
                      return false;
                    }

                    std::cout
                        << "\t--Assuming next files are target A: " << ival
                        << std::endl;
                    GiBUUToStdHepOpts::TargetAs.push_back(ival);
                    return true;

                  },
                  true, []() {}, "<Next file target nucleus 'A'>");

  CLIArgs::AddOpt("-z", "--target-z", true,
                  [&](std::string const &opt) -> bool {

                    if (GiBUUToStdHepOpts::TargetZs.size() >
                        GiBUUToStdHepOpts::InpFNames.size()) {
                      std::cerr << "[ERROR]: Found another -z option before "
                                   "the next file has been specified."
                                << std::endl;
                      return false;
                    }

                    int ival = 0;
                    try {
                      ival = Utils::str2i(opt, true);
                    } catch (...) {
                      return false;
                    }
                    std::cout
                        << "\t--Assuming next files are target Z: " << ival
                        << std::endl;
                    GiBUUToStdHepOpts::TargetZs.push_back(ival);
                    return true;

                  },
                  true, []() {}, "<Next file target nucleus 'Z'>");

  CLIArgs::AddOpt("-W", "--file-weight", true,
                  [&](std::string const &opt) -> bool {

                    if (GiBUUToStdHepOpts::FileExtraWeights.size() >
                        GiBUUToStdHepOpts::InpFNames.size()) {
                      std::cerr << "[ERROR]: Found another -W option before "
                                   "the next file has been specified."
                                << std::endl;
                      return false;
                    }

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
                  false, []() {},
                  "[i]<Next file extra weight [1.0/]'W' -- You do not need to "
                  "account for averaging over multiple files included by a "
                  "wildstar, this is done automagically.>");

  CLIArgs::AddOpt(
      "-R", "--Total-ReWeight", true,
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
        std::cout << "\t--Assigning overall weight: " << ival << std::endl;
        GiBUUToStdHepOpts::OverallWeight = ival;
        return true;
      },
      false, []() { GiBUUToStdHepOpts::OverallWeight = 1; },
      "[i]<Overall extra weight [1.0/]'W' -- This is most useful for weighting "
      "composite targets back to a weight per nucleon>");

  CLIArgs::AddOpt(
      "-v", "--Verbosity", true,
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

  CLIArgs::AddOpt("-NI", "--No-Initial-State", false,
                  [&](std::string const &opt) -> bool {
                    GiBUUToStdHepOpts::HaveStruckNucleonInfo = false;
                    std::cout << "\t--Not expecting FinalEvents.dat to contain "
                                 "initial state info."
                              << std::endl;
                    return true;
                  },
                  false,
                  [&]() { GiBUUToStdHepOpts::HaveStruckNucleonInfo = true; },
                  "Have struck nucleon information in GiBUU output.");

  CLIArgs::AddOpt("-NP", "--No-Prod-Charge", false,
                  [&](std::string const &opt) -> bool {
                    GiBUUToStdHepOpts::HaveProdChargeInfo = false;
                    std::cout << "\t--Not expecting FinalEvents.dat to contain "
                                 "neutrino induced resonance charge info."
                              << std::endl;
                    return true;
                  },
                  false,
                  [&]() { GiBUUToStdHepOpts::HaveProdChargeInfo = true; },
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

  CLIArgs::AddOpt("-F", "--Save-Flux-File", true,
                  [&](std::string const &opt) -> bool {
                    auto const &split = Utils::SplitStringByDelim(opt, ",");
                    if (split.size() != 2) {
                      std::cout << "[ERROR]: Expected -F argument to look like "
                                   "`histname,inputfilename.txt`."
                                << std::endl;
                      return false;
                    }

                    std::cout << "\t--Saving Flux histogram: " << split.front()
                              << ", from input: " << split.back() << std::endl;
                    GiBUUToStdHepOpts::FluxFilesToAdd.push_back(
                        std::make_pair(split.front(), split.back()));
                    return true;
                  },
                  false, [&]() {},
                  "[output_hist_name,input_text_flux_file.txt]");
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
