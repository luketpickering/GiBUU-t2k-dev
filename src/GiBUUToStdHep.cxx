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
#include "LUtils/Debugging.hxx"
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

/// The location of the input files which was produced by GiBUU.
std::vector<std::string> InpFNames;
/// The name of the output root file to write.
std::string OutFName;

/// Whether the GiBUU output contains struck nucleon information.
///
///\note Assumed true.
bool HaveStruckNucleonInfo;

///\brief The neutrino species PDG.
///
///\note Set by
///  `GiBUUToStdHep.exe ... -u xx ...'
/// Required.
std::vector<int> nuTypes;
///\brief The target nuclei nucleon number, A, for the next input file(s).
///
///\note Set by
///  `GiBUUToStdHep.exe ... -a xx ...'
/// Required.
std::vector<int> TargetAs;
///\brief The target nuclei proton number, Z, for the next input file(s).
///
///\note Set by
///  `GiBUUToStdHep.exe ... -z xx ...'
/// Required.
std::vector<int> TargetZs;
///\brief Whether input events for the next file(s) are simulated NC
/// interactions.
///
///\note Set by
///  `GiBUUToStdHep.exe ... -N ...'
std::vector<bool> CCFiles;
///\brief An extra weight to apply to the events of the next file(s).
///
/// Useful for building composite targets.
std::vector<float> FileExtraWeights;
///\brief An extra weight to applied which averages over the number of files
/// added.
std::vector<float> NFilesAddedWeights;
///\brief An extra weight to apply to all parsed events.
///
/// Useful for building composite targets.
float OverallWeight = 1;

///\brief Whether the GiBUU output contains the neutrino-induced hadronic
/// particles charge.
///
///\note Assumed true.
bool HaveProdChargeInfo = false;

std::vector<std::pair<std::string, std::string> > FluxFilesToAdd;

///\brief Whether to exit on suspicious input file contents.
bool StrictMode = true;
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
      UDBWarn("Event had malformed particle line: \"" << line << "\"");
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
    UDBError("Failed to parse one of the values: \"" << line << "\"");
    throw;
  }

  UDBVerbose("Parsed particle: " << pblob);

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
      UDBError("Failed to open " << fname << " for reading.");
      return 1;
    }

    std::string line;
    std::vector<GiBUUPartBlob> CurrEv;
    size_t LastEvNum = 0;
    size_t LineNum = 0;
    while (std::getline(ifs, line)) {
      UDBVerbose("[LINE:" << LineNum << "]: " << line);

      if (line[0] == '#') {  // Skip comments
        continue;
        LineNum++;
      }
      auto const &part = GetParticleLine(line);

      if ((part.PerWeight == 0) &&
          (!GiBUUToStdHepOpts::HaveStruckNucleonInfo)) {
        UDBWarn(
            "Found particle with 0 weight, but do not have "
            "initial state information enabled (-v -1 to silence this "
            "message).");
      }

      if ((part.EvNum != int(LastEvNum)) && LastEvNum) {
        FileEvents.push_back(CurrEv);
        ParsedEvs++;
        CurrEv.clear();
      }
      CurrEv.push_back(part);
      CurrEv.back().ln = LineNum;
      LastEvNum = part.EvNum;
      LineNum++;
    }
    if (CurrEv.size()) {
      FileEvents.push_back(CurrEv);
      ParsedEvs++;
      CurrEv.clear();
    }

    ifs.close();  // Read all the lines.
    UDBLog("Found " << FileEvents.size() << " events in " << fname << ".");

    double NRunsScaleFactor =
        GiBUUToStdHepOpts::NFilesAddedWeights[fileNumber] /
        double(FileEvents.back().back().Run);
    bool FileIsCC = GiBUUToStdHepOpts::CCFiles[fileNumber];
    int FileNuType = GiBUUToStdHepOpts::nuTypes[fileNumber];
    int FileTargetA = GiBUUToStdHepOpts::TargetAs[fileNumber];
    int FileTargetZ = GiBUUToStdHepOpts::TargetZs[fileNumber];
    double FileExtraWeight = GiBUUToStdHepOpts::FileExtraWeights[fileNumber];
    double TotalEventReweight =
        NRunsScaleFactor * FileExtraWeight * GiBUUToStdHepOpts::OverallWeight;

    for (auto const &ev : FileEvents) {
      giRooTracker->Reset();

      int const &EvNum = ev.front().EvNum;

      if (!EvNum) {  // Malformed line
        UDBWarn("Skipping event due to malformed line.");
        continue;
      }

      if (!(NumEvs % 10000)) {
        UDBInfo("Read " << NumEvs << " events.");
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
      giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPx] = 0;
      giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPy] = 0;
      giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPz] = 0;
      giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxE] = FileTargetA;

      giRooTracker->GiBUUReactionCode = ev.front().Prodid;
      if (GiBUUToStdHepOpts::HaveProdChargeInfo) {
        giRooTracker->GiBUUPrimaryParticleCharge = ev.front().ProdCharge;
      }
      giRooTracker->GiBUUPerWeight = ev.front().PerWeight;
      giRooTracker->NumRunsWeight = NRunsScaleFactor;
      giRooTracker->FileExtraWeight = FileExtraWeight;
      giRooTracker->EvtWght = giRooTracker->GiBUUPerWeight * TotalEventReweight;

      giRooTracker->StdHepN = 2;

      bool BadEv = false;
      for (auto const &part : ev) {
        if (!part.EvNum) {  // Malformed line
          UDBWarn("Skipping event due to malformed line.");
          BadEv = true;
          break;
        }

        if (GiBUUToStdHepOpts::HaveStruckNucleonInfo &&
            (giRooTracker->StdHepN == 3)) {
          giRooTracker->StdHepStatus[giRooTracker->StdHepN] = 11;
        } else {
          giRooTracker->StdHepStatus[giRooTracker->StdHepN] =
              1;  // All other FS
        }         // should be good.

        giRooTracker->StdHepPdg[giRooTracker->StdHepN] =
            GiBUUUtils::GiBUUToPDG(part.ID, part.Charge);

        if (!giRooTracker->StdHepPdg[giRooTracker->StdHepN]) {
          UDBWarn("Parsed part: " << part << " from file " << fname
                                  << " to have a PDG of 0.");
        }

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
        if (giRooTracker->StdHepN == GiRooTracker::kGiStdHepNPmax) {
          UDBWarn("In file " << fname << ", event " << EvNum
                             << " contained to many final state particles "
                             << ev.size() << ". Ignoring the last: "
                             << (ev.size() - GiRooTracker::kGiStdHepNPmax));
          break;
        }
      }

      if (BadEv) {
        continue;
      }  // If we broke then don't bother continuing
         // processing.

      try {
        giRooTracker->GiBUU2NeutCode = GiBUUUtils::GiBUU2NeutReacCode(
            giRooTracker->GiBUUReactionCode, giRooTracker->StdHepPdg,
            giRooTracker->GiBHepHistory, giRooTracker->StdHepN, FileIsCC,
            (GiBUUToStdHepOpts::HaveStruckNucleonInfo) ? 3 : -1,
            GiBUUToStdHepOpts::HaveProdChargeInfo
                ? giRooTracker->GiBUUPrimaryParticleCharge
                : -10);
      } catch (...) {
        UDBLog("Caught error in " << fname << ":" << ev.front().ln);
        if (GiBUUToStdHepOpts::StrictMode) {
          return 1;
        } else {
          continue;
        }
      }

      if (UDBDebugging::GetInfoLevel() > 3) {
        UDBVerbose(
            "[INFO]: EvNo: "
            << EvNum << ", contained " << giRooTracker->StdHepN << " ("
            << ev.size() << ") particles. "
                            "Event Weight: "
            << std::setprecision(3) << giRooTracker->GiBUUPerWeight
            << "\n\tGiBUUReactionCode: " << giRooTracker->GiBUUReactionCode
            << ", NeutConventionReactionCode: " << giRooTracker->GiBUU2NeutCode
            << "\n\t[Lep In] : "
            << TLorentzVector(
                   giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPx],
                   giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPy],
                   giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPz],
                   giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxE]));
        UDBVerbose("\t[Target] : " << giRooTracker->StdHepPdg[1]);
        if (GiBUUToStdHepOpts::HaveStruckNucleonInfo) {
          UDBVerbose(
              "\t[Nuc In] : "
              << TLorentzVector(
                     giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxPx],
                     giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxPy],
                     giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxPz],
                     giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxE])
              << " (" << std::setw(4) << giRooTracker->StdHepPdg[3] << ")");
        }

        // We have already printed the struck nucleon
        Int_t StartPoint =
            ((!GiBUUToStdHepOpts::HaveStruckNucleonInfo) ? 3 : 4);
        for (Int_t stdHepInd = StartPoint; stdHepInd < giRooTracker->StdHepN;
             ++stdHepInd) {
          UDBVerbose(
              "\t[" << std::setw(2) << (stdHepInd - (StartPoint)) << "]("
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
                    << " (H:" << giRooTracker->GiBHepHistory[stdHepInd] << ")");

          UDBVerbose("\t\t" << GiBUUUtils::WriteGiBUUHistory(
                         giRooTracker->GiBHepHistory[stdHepInd]));
        }
        UDBVerbose("\t[Lep Out]: "
                   << TLorentzVector(
                          giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPx],
                          giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPy],
                          giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPz],
                          giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxE])
                   << std::endl);
      }
      OutputTree->Fill();
      NumEvs++;
    }
    FileEvents.clear();
    fileNumber++;
  }

  UDBInfo("Saved " << NumEvs << " events.");
  return 0;
}

void SaveFluxFile(std::string const &fileloc, std::string const &histname) {
  std::ifstream ifs(fileloc);
  if (!ifs.good()) {
    UDBError("File \"" << fileloc << " could not be opened for reading.");
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
    UDBVerbose("Flux file line[" << ln << "]: " << line);
    std::vector<float> splitLine =
        Utils::StringVToFloatV(Utils::SplitStringByDelim(line, " \t,"));
    if (splitLine.size() != 2) {
      UDBWarn("ingoring line: \"" << line << "\" in input flux file.");
      continue;
    }
    FluxValues.push_back(std::make_pair(splitLine.front(), splitLine.back()));
    ln++;
  }
  ifs.close();

  if (FluxValues.size() == 0) {
    UDBError("Found no input lines in" << fileloc);
    throw;
  }

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
    UDBError("Couldn't open output file.");
    return 2;
  }

  TTree *rooTrackerTree = new TTree("giRooTracker", "GiBUU StdHepVariables");
  GiRooTracker *giRooTracker = new GiRooTracker();
  giRooTracker->AddBranches(rooTrackerTree, true,
                            GiBUUToStdHepOpts::HaveProdChargeInfo);

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
    UDBLog("\t--Adding file: " << OptVal);
    GiBUUToStdHepOpts::InpFNames.push_back(OptVal);
    GiBUUToStdHepOpts::nuTypes.push_back(NuType);
    GiBUUToStdHepOpts::TargetAs.push_back(TargetA);
    GiBUUToStdHepOpts::TargetZs.push_back(TargetZ);
    GiBUUToStdHepOpts::CCFiles.push_back(IsCC);
    GiBUUToStdHepOpts::FileExtraWeights.push_back(FileExtraWeight);
    GiBUUToStdHepOpts::NFilesAddedWeights.push_back(1);
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
    UDBLog("\t--Looking in current directory ("
           << cwd.get() << ") for matching (\"" << matchPat << "\") files.");
    dirpath = "./";
  } else {
    if (AsteriskPos < lastFSlash) {
      UDBError(
          "Currently cannot handle a wildcard in the "
          "directory structure. Please put input files in the same "
          "directory or use separate -f arguments (N.B. you will have "
          "to manually weight each separate file by 1/NFiles). "
          "Expected -f \"../some/rel/path/*.dat\"");
      return false;
    }
    dirpath = OptVal.substr(0, lastFSlash + 1);
    UDBLog("\t--Looking in directory (" << dirpath << ") for matching files.");
  }
  dir = opendir(dirpath.c_str());

  if (dir != NULL) {
    TRegexp matchExp(matchPat.c_str(), true);
    /* print all the files and directories within directory */
    Ssiz_t len = 0;
    size_t NFilesAdded = 0;
    while ((ent = readdir(dir)) != NULL) {
      if (matchExp.Index(TString(ent->d_name), &len) != Ssiz_t(-1)) {
        UDBLog("\t\t\tAdding matching file: "
               << (dirpath + ent->d_name) << "(nu: " << NuType
               << ", A: " << TargetA << ", Z: " << TargetZ
               << ", TW: " << FileExtraWeight << ", IsCC: " << IsCC << ")");
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
      GiBUUToStdHepOpts::FileExtraWeights.push_back(FileExtraWeight);
      GiBUUToStdHepOpts::NFilesAddedWeights.push_back(1.0 /
                                                      double(NFilesAdded));
    }
    UDBLog("Added " << NFilesAdded << " overall weight: "
                    << (FileExtraWeight / double(NFilesAdded)));
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
          UDBError(
              "-u X -a Y -z Z must be specified before the "
              "first input file.");
          return false;
        }

        UDBLog("\t--Reading FinalEvents-style GiBUU file(s) from descriptor: \""
               << opt << "\"");

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
        UDBLog("\t--Writing to file " << opt);
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
                      UDBError(
                          "Found another -u option before "
                          "the next file has been specified.");
                      return false;
                    }

                    UDBLog("\t--Assuming next file has Nu PDG: " << ival);
                    GiBUUToStdHepOpts::nuTypes.push_back(ival);
                    return true;
                  },
                  true, []() {}, "<Next file neutrino PDG code>");

  CLIArgs::AddOpt("-N", "--is-NC", false,
                  [&](std::string const &opt) -> bool {

                    if (GiBUUToStdHepOpts::CCFiles.size() >
                        GiBUUToStdHepOpts::InpFNames.size()) {
                      UDBError(
                          "Found another -N option before "
                          "the next file has been specified.");
                      return false;
                    }

                    UDBLog("\t--Assuming next files contains NC event.");
                    GiBUUToStdHepOpts::CCFiles.push_back(false);
                    return true;
                  },
                  false, []() {}, "<Next file is NC events>");

  CLIArgs::AddOpt("-a", "--target-a", true,
                  [&](std::string const &opt) -> bool {

                    if (GiBUUToStdHepOpts::TargetAs.size() >
                        GiBUUToStdHepOpts::InpFNames.size()) {
                      UDBError(
                          "Found another -a option before "
                          "the next file has been specified.");
                      return false;
                    }

                    int ival = 0;
                    try {
                      ival = Utils::str2i(opt, true);
                    } catch (...) {
                      return false;
                    }

                    UDBLog("\t--Assuming next files are target A: " << ival);
                    GiBUUToStdHepOpts::TargetAs.push_back(ival);
                    return true;

                  },
                  true, []() {}, "<Next file target nucleus 'A'>");

  CLIArgs::AddOpt("-z", "--target-z", true,
                  [&](std::string const &opt) -> bool {

                    if (GiBUUToStdHepOpts::TargetZs.size() >
                        GiBUUToStdHepOpts::InpFNames.size()) {
                      UDBError(
                          "Found another -z option before "
                          "the next file has been specified.");
                      return false;
                    }

                    int ival = 0;
                    try {
                      ival = Utils::str2i(opt, true);
                    } catch (...) {
                      return false;
                    }
                    UDBLog("\t--Assuming next files are target Z: " << ival);
                    GiBUUToStdHepOpts::TargetZs.push_back(ival);
                    return true;

                  },
                  true, []() {}, "<Next file target nucleus 'Z'>");

  CLIArgs::AddOpt("-W", "--file-weight", true,
                  [&](std::string const &opt) -> bool {

                    if (GiBUUToStdHepOpts::FileExtraWeights.size() >
                        GiBUUToStdHepOpts::InpFNames.size()) {
                      UDBError(
                          "Found another -W option before "
                          "the next file has been specified.");
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
                    UDBLog("\t--Assigning next file target weight: " << ival);
                    GiBUUToStdHepOpts::FileExtraWeights.push_back(ival);
                    return true;
                  },
                  false, []() {}, "[i]<Next file target weight [1.0/]'W'>");

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
        UDBLog("\t--Assigning overall weight: " << ival);
        GiBUUToStdHepOpts::OverallWeight = ival;
        return true;
      },
      false, []() { GiBUUToStdHepOpts::OverallWeight = 1; },
      "[i]<Overall extra weight [1.0/]'W' -- This is most useful for weighting "
      "composite targets back to a weight per nucleon>");

  CLIArgs::AddOpt("-v", "--Verbosity", true,
                  [&](std::string const &opt) -> bool {
                    int ival = 0;
                    try {
                      ival = Utils::str2i(opt, true);
                    } catch (...) {
                      return false;
                    }
                    UDBLog("\t--Verbosity: " << ival);
                    UDBDebugging::SetDebugLevel(ival);
                    UDBDebugging::SetInfoLevel(ival);
                    return true;
                  },
                  false,
                  [&]() {
                    UDBDebugging::SetDebugLevel(2);
                    UDBDebugging::SetInfoLevel(2);
                  },
                  "<0-4>{default==0}");

  CLIArgs::AddOpt("-NI", "--No-Initial-State", false,
                  [&](std::string const &opt) -> bool {
                    GiBUUToStdHepOpts::HaveStruckNucleonInfo = false;
                    UDBLog(
                        "\t--Not expecting FinalEvents.dat to contain "
                        "initial state info.");
                    return true;
                  },
                  false,
                  [&]() { GiBUUToStdHepOpts::HaveStruckNucleonInfo = true; },
                  "Have struck nucleon information in GiBUU output.");

  CLIArgs::AddOpt("-NP", "--No-Prod-Charge", false,
                  [&](std::string const &opt) -> bool {
                    GiBUUToStdHepOpts::HaveProdChargeInfo = false;
                    UDBLog(
                        "\t--Not expecting FinalEvents.dat to contain "
                        "neutrino induced resonance charge info.");
                    return true;
                  },
                  false,
                  [&]() { GiBUUToStdHepOpts::HaveProdChargeInfo = true; },
                  "Have primary particle charge information in GiBUU output.");

  CLIArgs::AddOpt(
      "-F", "--Save-Flux-File", true,
      [&](std::string const &opt) -> bool {
        auto const &split = Utils::SplitStringByDelim(opt, ",");
        if (split.size() != 2) {
          UDBLog(
              "[ERROR]: Expected -F argument to look like "
              "`histname,inputfilename.txt`.");
          return false;
        }

        UDBLog("\t--Saving Flux histogram: "
               << split.front() << ", from input: " << split.back());
        GiBUUToStdHepOpts::FluxFilesToAdd.push_back(
            std::make_pair(split.front(), split.back()));
        return true;
      },
      false, [&]() {}, "[output_hist_name,input_text_flux_file.txt]");
}

int main(int argc, char const *argv[]) {
  try {
    SetOpts();
  } catch (std::exception const &e) {
    UDBError(e.what());
    return 1;
  }

  CLIArgs::AddArguments(argc, argv);
  if (!CLIArgs::HandleArgs()) {
    CLIArgs::SayRunLike();
    return 1;
  }

  if (GiBUUToStdHepOpts::CCFiles.size() !=
      GiBUUToStdHepOpts::InpFNames.size()) {
    UDBError(
        "found " << GiBUUToStdHepOpts::CCFiles.size()
                 << " CC/NC event markers and "
                 << GiBUUToStdHepOpts::InpFNames.size() << " input file names."
                 << std::endl
                 << "N.B. the -N argument specifies that the value of the next "
                    "-f/-l argument is an NC file---argument position is "
                    "important.");
    return 1;
  }

  return GiBUUToStdHep();
}
