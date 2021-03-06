#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "TFile.h"
#include "TH1D.h"
#include "TLorentzVector.h"
#include "TTree.h"
#include "TVector3.h"

#include "LUtils/Debugging.hxx"
#include "LUtils/Utils.hxx"

#include "GiBUUToStdHep_CLIOpts.hxx"
#include "GiBUUToStdHep_Utils.hxx"

#include "GiRooTracker.hxx"

std::map<int, double> FluxComponentIntegrals;
std::map<int, TH1D *> FluxHists;
std::map<int, TH1D *> SigmaHists;
std::map<int, TH1D *> EvHists;

double DomFCI = std::numeric_limits<double>::min();
int DomPDG = 0;
TH1D *DomFlux = NULL;
TH1D *DomEvt = NULL;

GiBUUPartBlob GetParticleLine(std::string const &line) {
  GiBUUPartBlob pblob;

  std::vector<std::string> splitLine = Utils::SplitStringByDelim(line, " ");
  if (splitLine.size() !=
      (15 +
       size_t(GiBUUToStdHepOpts::HaveProdChargeInfo))) { // try to fix known
                                                         // parsing error
    std::string ln =
        Utils::Replace(line, "E-", "XXXXX"); // Guard any exponential notation
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
    pblob.Run = Utils::str2i(splitLine[0]);
    pblob.EvNum = Utils::str2i(splitLine[1]);
    pblob.ID = Utils::str2i(splitLine[2]);
    pblob.Charge = Utils::str2i(splitLine[3]);
    pblob.PerWeight = Utils::str2d(splitLine[4]);
    pblob.Position[GiRooTracker::kStdHepIdxPx] = Utils::str2d(splitLine[5]);
    pblob.Position[GiRooTracker::kStdHepIdxPy] = Utils::str2d(splitLine[6]);
    pblob.Position[GiRooTracker::kStdHepIdxPz] = Utils::str2d(splitLine[7]);
    pblob.FourMom[GiRooTracker::kStdHepIdxE] = Utils::str2d(splitLine[8]);
    pblob.FourMom[GiRooTracker::kStdHepIdxPx] = Utils::str2d(splitLine[9]);
    pblob.FourMom[GiRooTracker::kStdHepIdxPy] = Utils::str2d(splitLine[10]);
    pblob.FourMom[GiRooTracker::kStdHepIdxPz] = Utils::str2d(splitLine[11]);
    pblob.History = Utils::str2l(splitLine[12]);
    pblob.Prodid = Utils::str2i(splitLine[13]);
    pblob.EProbe = Utils::str2d(splitLine[14]);
    if (GiBUUToStdHepOpts::HaveProdChargeInfo) {
      pblob.ProdCharge = Utils::str2i(splitLine[15]);
    }
  } catch (const std::invalid_argument &ia) {
    UDBError("Failed to parse one of the values: \"" << line << "\"");
    throw;
  }

  UDBVerbose("Parsed particle: " << pblob);

  return pblob;
}

size_t FlushEventsToDisk(TTree *OutputTree, GiRooTracker *giRooTracker,
                         size_t fileNumber, size_t NRunsInFile,
                         std::vector<std::vector<GiBUUPartBlob>> &Events) {
  size_t NumEvs = 0;

  double NRunsScaleFactor =
      GiBUUToStdHepOpts::NFilesAddedWeights[fileNumber] / double(NRunsInFile);
  bool FileIsCC = GiBUUToStdHepOpts::CCFiles[fileNumber];
  int FileNuType = GiBUUToStdHepOpts::ProbeTypes[fileNumber];
  int FileTargetA = GiBUUToStdHepOpts::TargetAs[fileNumber];
  int FileTargetZ = GiBUUToStdHepOpts::TargetZs[fileNumber];
  double FileExtraWeight = GiBUUToStdHepOpts::FileExtraWeights[fileNumber];
  double TotalEventReweight =
      NRunsScaleFactor * FileExtraWeight * GiBUUToStdHepOpts::OverallWeight;

  size_t NEvents = Events.size();
  for (size_t ev_it = 0; ev_it < NEvents; ++ev_it) {
    std::vector<GiBUUPartBlob> const &ev = Events[ev_it];
    giRooTracker->Reset();

    int const &EvNum = ev.front().EvNum;

    if (!EvNum) { // Malformed line
      UDBWarn("Skipping event due to malformed line.");
      continue;
    }

    if (!(NumEvs % 10000)) {
      UDBInfo("Read " << NumEvs << " events.");
    }

    giRooTracker->EvtNum = EvNum;

    if (!GiBUUToStdHepOpts::IsNDK) {
      // neutrino
      giRooTracker->StdHepPdg[0] = FileNuType;

      giRooTracker->StdHepStatus[0] = 0;
      giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPx] = 0;
      giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPy] = 0;
      giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPz] =
          GiBUUToStdHepOpts::IsElectronScattering
              ? sqrt(ev.front().EProbe * ev.front().EProbe -
                     511 * PhysConst::KeV * 511 * PhysConst::KeV)
              : ev.front().EProbe;
      giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxE] = ev.front().EProbe;

      if (GiBUUToStdHepOpts::EScatteringInputEnergy = 0xdeadbeef) {
        GiBUUToStdHepOpts::EScatteringInputEnergy = ev.front().EProbe;
      } else if (fabs(GiBUUToStdHepOpts::EScatteringInputEnergy -
                      ev.front().EProbe) > 1E-5) {
        UDBError("Read a differing input energy: First event: "
                 << GiBUUToStdHepOpts::EScatteringInputEnergy << ", event "
                 << EvNum << " in file \""
                 << GiBUUToStdHepOpts::InpFNames[fileNumber] << "\" had "
                 << ev.front().EProbe);
        throw;
      }
    }
    size_t targetIdx = GiBUUToStdHepOpts::IsNDK ? 0 : 1;

    // target
    giRooTracker->StdHepPdg[targetIdx] =
        Utils::MakeNuclearPDG(FileTargetZ, FileTargetA);
    giRooTracker->StdHepStatus[targetIdx] = 0;
    giRooTracker->StdHepP4[targetIdx][GiRooTracker::kStdHepIdxPx] = 0;
    giRooTracker->StdHepP4[targetIdx][GiRooTracker::kStdHepIdxPy] = 0;
    giRooTracker->StdHepP4[targetIdx][GiRooTracker::kStdHepIdxPz] = 0;
    giRooTracker->StdHepP4[targetIdx][GiRooTracker::kStdHepIdxE] = FileTargetA;

    // event meta-data
    giRooTracker->GiBUUReactionCode = ev.front().Prodid;
    if (GiBUUToStdHepOpts::HaveProdChargeInfo) {
      giRooTracker->GiBUUPrimaryParticleCharge = ev.front().ProdCharge;
    }
    giRooTracker->GiBUUPerWeight = ev.front().PerWeight;
    giRooTracker->NumRunsWeight = NRunsScaleFactor;
    giRooTracker->FileExtraWeight = FileExtraWeight;
    giRooTracker->EvtWght = giRooTracker->GiBUUPerWeight * TotalEventReweight *
                            (GiBUUToStdHepOpts::IsElectronScattering ? 1E5 : 1);

    if (FluxHists.count(FileNuType)) {
      SigmaHists[FileNuType]->Fill(ev.front().EProbe, giRooTracker->EvtWght);
      EvHists[FileNuType]->Fill(ev.front().EProbe,
                                giRooTracker->EvtWght *
                                    FluxComponentIntegrals[FileNuType]);
    }
    if (FileNuType == DomPDG) {
      DomEvt->Fill(ev.front().EProbe, giRooTracker->EvtWght * DomFCI);
    }

    giRooTracker->StdHepN = GiBUUToStdHepOpts::IsNDK ? 1 : 2;

    bool BadEv = false;
    for (size_t p_it = 0; p_it < ev.size(); ++p_it) {
      GiBUUPartBlob const &part = ev[p_it];
      if (!part.EvNum) { // Malformed line
        UDBWarn("Skipping event due to malformed line.");
        BadEv = true;
        break;
      }

      if (GiBUUToStdHepOpts::IsNDK) {
        if (p_it == 0) { // Pre-FSI Kaon information
          giRooTracker->StdHepStatus[giRooTracker->StdHepN] =
              14; // GENIE hadron in the nucleus convention.
        } else {
          giRooTracker->StdHepStatus[giRooTracker->StdHepN] = 1; // All other FS
        }
      } else {
        if (GiBUUToStdHepOpts::HaveStruckNucleonInfo &&
            (giRooTracker->StdHepN == 3)) {
          giRooTracker->StdHepStatus[giRooTracker->StdHepN] = 11;
        } else {
          giRooTracker->StdHepStatus[giRooTracker->StdHepN] = 1; // All other FS
        } // should be good.
      }

      // Particles read from LH files are already in PDG format
      giRooTracker->StdHepPdg[giRooTracker->StdHepN] =
          part.IDIsPDG ? part.ID : GiBUUUtils::GiBUUToPDG(part.ID, part.Charge);

      if (!giRooTracker->StdHepPdg[giRooTracker->StdHepN]) {
        UDBWarn("Parsed part: " << part << " from file "
                                << GiBUUToStdHepOpts::InpFNames[fileNumber]
                                << " to have a PDG of 0.");
      }
      // A known unknown
      if (giRooTracker->StdHepPdg[giRooTracker->StdHepN] == -1) {
        giRooTracker->StdHepPdg[giRooTracker->StdHepN] = 0;
      }

      giRooTracker
          ->StdHepP4[giRooTracker->StdHepN][GiRooTracker::kStdHepIdxPx] =
          part.FourMom.X();
      giRooTracker
          ->StdHepP4[giRooTracker->StdHepN][GiRooTracker::kStdHepIdxPy] =
          part.FourMom.Y();
      giRooTracker
          ->StdHepP4[giRooTracker->StdHepN][GiRooTracker::kStdHepIdxPz] =
          part.FourMom.Z();
      giRooTracker->StdHepP4[giRooTracker->StdHepN][GiRooTracker::kStdHepIdxE] =
          part.FourMom.E();

      giRooTracker->GiBHepHistory[giRooTracker->StdHepN] = part.History;
#ifndef CPP03COMPAT
      auto const &hDec = GiBUUUtils::DecomposeGiBUUHistory(part.History);
      giRooTracker->GiBHepGeneration[giRooTracker->StdHepN] = std::get<0>(hDec);

      if (std::get<1>(hDec) == -1) { // If this was produced by a 3 body
                                     // process
        giRooTracker->GiBHepMother[giRooTracker->StdHepN] = std::get<1>(hDec);
        giRooTracker->GiBHepFather[giRooTracker->StdHepN] = std::get<2>(hDec);
      } else {
        giRooTracker->GiBHepMother[giRooTracker->StdHepN] =
            GiBUUUtils::GiBUUToPDG(std::get<1>(hDec));
        giRooTracker->GiBHepFather[giRooTracker->StdHepN] =
            GiBUUUtils::GiBUUToPDG(std::get<2>(hDec));
      }
#endif

      giRooTracker->StdHepN++;
      if (giRooTracker->StdHepN == GiRooTracker::kGiStdHepNPmax) {
        UDBWarn("In file " << GiBUUToStdHepOpts::InpFNames[fileNumber]
                           << ", event " << EvNum
                           << " contained to many final state particles "
                           << ev.size() << ". Ignoring the last: "
                           << (ev.size() - GiRooTracker::kGiStdHepNPmax));
        break;
      }
    }

    if (BadEv) {
      continue;
    } // If we broke then don't bother continuing
      // processing.

    if (GiBUUToStdHepOpts::IsElectronScattering) {
      giRooTracker->GiBUU2NeutCode = GiBUUUtils::GiBUU2NeutReacCode_escat(
          giRooTracker->GiBUUReactionCode, giRooTracker->StdHepPdg);
    } else if (!GiBUUToStdHepOpts::IsNDK) {
      try {
        giRooTracker->GiBUU2NeutCode = GiBUUUtils::GiBUU2NeutReacCode(
            giRooTracker->GiBUUReactionCode, giRooTracker->StdHepPdg,

#ifndef CPP03COMPAT
            giRooTracker->GiBHepHistory,
#endif
            giRooTracker->StdHepN, FileIsCC,
            (GiBUUToStdHepOpts::HaveStruckNucleonInfo) ? 3 : -1,
            GiBUUToStdHepOpts::HaveProdChargeInfo
                ? giRooTracker->GiBUUPrimaryParticleCharge
                : -10);
      } catch (...) {
        UDBLog("Caught error in " << GiBUUToStdHepOpts::InpFNames[fileNumber]
                                  << ":" << ev.front().ln);
        if (GiBUUToStdHepOpts::StrictMode) {
          return 1;
        } else {
          continue;
        }
      }
    }

    if (UDBDebugging::GetInfoLevel() > 2) {
      if (!GiBUUToStdHepOpts::IsNDK) {
        UDBInfo("EvNo: "
                << EvNum << ", contained " << giRooTracker->StdHepN << " ("
                << ev.size()
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
                << " (" << giRooTracker->StdHepPdg[0] << ")");
        UDBInfo("\t[Target] : " << giRooTracker->StdHepPdg[1]);
        if (GiBUUToStdHepOpts::HaveStruckNucleonInfo) {
          UDBInfo("\t[Nuc In] : "
                  << TLorentzVector(
                         giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxPx],
                         giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxPy],
                         giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxPz],
                         giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxE])
                  << " (" << std::setw(4) << giRooTracker->StdHepPdg[3] << ")");
        }
      } else {
        UDBInfo("EvNo: "
                << EvNum << ", contained " << giRooTracker->StdHepN << " ("
                << ev.size() << ") particles. "
                << "\n\t[Init NDK particle] : "
                << TLorentzVector(
                       giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPx],
                       giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPy],
                       giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPz],
                       giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxE])
                << " (" << giRooTracker->StdHepPdg[1] << ")");
        UDBInfo("\t[Target] : " << giRooTracker->StdHepPdg[0]);
      }

      // We have already printed the struck nucleon
      Int_t StartPoint =
          GiBUUToStdHepOpts::IsNDK
              ? 2
              : ((!GiBUUToStdHepOpts::HaveStruckNucleonInfo) ? 3 : 4);
      for (Int_t stdHepInd = StartPoint; stdHepInd < giRooTracker->StdHepN;
           ++stdHepInd) {
        UDBInfo(
            "\t["
            << std::setw(2) << (stdHepInd - (StartPoint)) << "]("
            << std::setw(5) << giRooTracker->StdHepPdg[stdHepInd] << ")"
            << TLorentzVector(
                   giRooTracker
                       ->StdHepP4[stdHepInd][GiRooTracker::kStdHepIdxPx],
                   giRooTracker
                       ->StdHepP4[stdHepInd][GiRooTracker::kStdHepIdxPy],
                   giRooTracker
                       ->StdHepP4[stdHepInd][GiRooTracker::kStdHepIdxPz],
                   giRooTracker->StdHepP4[stdHepInd][GiRooTracker::kStdHepIdxE])
#ifndef CPP03COMPAT
            << " (H:" << giRooTracker->GiBHepHistory[stdHepInd] << ")"
#endif

        );
#ifndef CPP03COMPAT
        UDBInfo("\t\t" << GiBUUUtils::WriteGiBUUHistory(
                    giRooTracker->GiBHepHistory[stdHepInd]));
#endif
      }
      if (!GiBUUToStdHepOpts::IsNDK) {
        UDBInfo("\t[Lep Out]: "
                << TLorentzVector(
                       giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPx],
                       giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPy],
                       giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPz],
                       giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxE])
                << " (" << giRooTracker->StdHepPdg[2] << ")" << std::endl);
      }
    }
    OutputTree->Fill();
    NumEvs++;
  }
  UDBInfo("Wrote " << NumEvs << " events to disk.");
  Events.clear();
  return NumEvs;
}

std::istream &Ignoreline(std::ifstream &in, std::ifstream::pos_type &pos) {
  pos = in.tellg();
  return in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string GetLastLine(std::ifstream &in) {
  std::ifstream::pos_type pos = in.tellg();

  std::ifstream::pos_type lastPos;
  while (in >> std::ws && Ignoreline(in, lastPos)) {
    pos = lastPos;
  }

  in.clear();
  in.seekg(pos);

  std::string line;
  std::getline(in, line);
  return line;
}

int ParseACSIIEventVectors(TTree *OutputTree, GiRooTracker *giRooTracker) {
  std::vector<std::vector<GiBUUPartBlob>> FileEvents;
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

  for (size_t fname_it = 0; fname_it < GiBUUToStdHepOpts::InpFNames.size();
       ++fname_it) {
    std::string const &fname = GiBUUToStdHepOpts::InpFNames[fname_it];

    size_t NEvsInFile = 0;

    std::string format = Utils::SplitStringByDelim(fname, ".").back();
    if (format == "lhe") {
      bool holder_SNI = GiBUUToStdHepOpts::HaveStruckNucleonInfo;
      bool holder_PCI = GiBUUToStdHepOpts::HaveProdChargeInfo;
      GiBUUToStdHepOpts::HaveStruckNucleonInfo = false;
      GiBUUToStdHepOpts::HaveProdChargeInfo = false;

      LHVectorReader lhevr(fname);

      UDBVerbose("Les Houches File with " << lhevr.CountEvents()
                                          << " events found. ");

      size_t NParts = 0;
      do {
        std::vector<GiBUUPartBlob> ev = lhevr.ReadEvent();
        if ((NParts = ev.size())) {
          if (!GiBUUToStdHepOpts::IsNDK) {
            // Have to force known FSLepton information
            int FSLeptonPDG = 0;
            if (GiBUUToStdHepOpts::IsElectronScattering) {
              FSLeptonPDG = 11;
            } else {
              bool FileIsCC = GiBUUToStdHepOpts::CCFiles[fileNumber];
              int FileNuType = GiBUUToStdHepOpts::ProbeTypes[fileNumber];
              if (FileIsCC) {
                FSLeptonPDG = FileNuType + ((FileNuType < 0) ? +1 : -1);
              } else { // NC event
                FSLeptonPDG = FileNuType;
              }
            }
            ev.front().ID = FSLeptonPDG;
          } else {
            int FileNuType = 0;
            ev.front().ID = 321;
          }
          FileEvents.push_back(ev);
        }

        if (FileEvents.size() == 5E4) {
          NEvsInFile += FlushEventsToDisk(OutputTree, giRooTracker, fileNumber,
                                          1, FileEvents);
        }

      } while (NParts);

      if (FileEvents.size()) {
        NEvsInFile += FlushEventsToDisk(OutputTree, giRooTracker, fileNumber, 1,
                                        FileEvents);
      }

      GiBUUToStdHepOpts::HaveStruckNucleonInfo = holder_SNI;
      GiBUUToStdHepOpts::HaveProdChargeInfo = holder_PCI;

    } else { // FinalEvents.dat
      if (GiBUUToStdHepOpts::IsNDK) {
        std::cout << "[ERROR]: Can currently only read NDK events from Les "
                     "Houches file format."
                  << std::endl;
        return 1;
      }
      std::ifstream ifs(fname.c_str());

      if (!ifs.good()) {
        UDBError("Failed to open " << fname << " for reading.");
        return 1;
      }

      /// Get NRuns
      size_t NRunsInFile = 0;
      std::string line = GetLastLine(ifs);
      std::vector<std::string> splitLine = Utils::SplitStringByDelim(line, " ");
      NRunsInFile = Utils::str2i(splitLine[0]);
      UDBLog("Found " << NRunsInFile << " runs in " << fname << ".");

      /// Rewind
      ifs.clear();
      ifs.seekg(0);

      std::vector<GiBUUPartBlob> CurrEv;
      size_t LastEvNum = 0;
      size_t LineNum = 0;
      while (std::getline(ifs, line)) {
        UDBVerbose("[LINE:" << LineNum << "]: " << line);

        if (line[0] == '#') { // Skip comments
          continue;
          LineNum++;
        }
        GiBUUPartBlob const &part = GetParticleLine(line);

        if ((part.PerWeight == 0) &&
            (!GiBUUToStdHepOpts::HaveStruckNucleonInfo)) {
          UDBWarn("Found particle with 0 weight, but do not have "
                  "initial state information enabled (-v -1 to silence this "
                  "message).");
        }

        if ((part.EvNum != int(LastEvNum)) && LastEvNum) {
          FileEvents.push_back(CurrEv);
          ParsedEvs++;
          CurrEv.clear();

          // Flush events every 50k events
          if (FileEvents.size() == 5E4) {
            NEvsInFile += FlushEventsToDisk(
                OutputTree, giRooTracker, fileNumber, NRunsInFile, FileEvents);
          }
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

      // Flush any remaining events
      if (FileEvents.size()) {
        NEvsInFile += FlushEventsToDisk(OutputTree, giRooTracker, fileNumber,
                                        NRunsInFile, FileEvents);
      }

      ifs.close(); // Read all the lines.
    }
    UDBLog("Found " << NEvsInFile << " events in " << fname << ".");

    if (!NEvsInFile) {
      continue;
    }

    fileNumber++;
    NumEvs += NEvsInFile;
  }

  UDBInfo("Saved " << NumEvs << " events.");

  if (!GiBUUToStdHepOpts::IsNDK) {
    for (std::map<int, TH1D *>::iterator h_it = SigmaHists.begin();
         h_it != SigmaHists.end(); ++h_it) {
      for (int bi_it = 1; bi_it < h_it->second->GetXaxis()->GetNbins() + 1;
           ++bi_it) {
        double ENuBWidth =
            FluxHists[h_it->first]->GetXaxis()->GetBinWidth(bi_it);
        double NNu = FluxHists[h_it->first]->GetBinContent(bi_it) * ENuBWidth;
        if (NNu < std::numeric_limits<double>::min()) {
          continue;
        }
        h_it->second->SetBinContent(bi_it,
                                    h_it->second->GetBinContent(bi_it) / NNu);
        h_it->second->SetBinError(bi_it,
                                  h_it->second->GetBinError(bi_it) / NNu);
      }

      EvHists[h_it->first]->Scale(1, "width");
      EvHists[h_it->first]->Scale(NumEvs);
    }
    if (DomEvt) {
      DomEvt->Scale(1, "width");
      DomEvt->Write("evt_per_NEvents");
      std::string name = DomEvt->GetName();
      std::string title = DomEvt->GetTitle();
      DomEvt = static_cast<TH1D *>(FluxHists[DomPDG]->Clone());
      DomEvt->SetNameTitle(name.c_str(), title.c_str());
      DomEvt->Scale(NumEvs);
    }
  }

  return 0;
}

int GetPDGFromHistName(std::string const &histname) {
  if ("numu_flux" == histname) {
    return 14;
  } else if ("numub_flux" == histname) {
    return -14;
  } else if ("nue_flux" == histname) {
    return 12;
  } else if ("nueb_flux" == histname) {
    return -12;
  } else {
    return 0;
  }
}

std::pair<double, double> HandleFluxIntegralLine(std::string const &fln,
                                                 std::string const &histname,
                                                 int pdgfromhistname) {
  if (FluxComponentIntegrals.count(pdgfromhistname)) {
    UDBWarn("Already read a flux file for hist: " << histname
                                                  << ", overwriting.");
  } else {
    FluxComponentIntegrals[pdgfromhistname] = 0;
  }

  if ("# input flux integral:" != fln.substr(0, 22)) {
    UDBWarn(
        "Input flux file didn't contain integral comment, the flux species "
        "associated with \""
        << histname
        << "\" will not be correctly normalisable in a multi-species sample.");
    UDBWarn("From flux line: " << fln);

    return std::pair<double, double>(0, 0);
  }
  size_t wi_it = fln.find("(width integral: ");

  if (wi_it == std::string::npos) {
    UDBWarn(
        "Input flux file didn't contain integral comment, the flux species "
        "associated with \""
        << histname
        << "\" will not be correctly normalisable in a multi-species sample.");
    UDBWarn("From flux line: " << fln);
    return std::pair<double, double>(0, 0);
  }

  size_t end_bracket = fln.find_last_of(")");

  double fi;
  std::string IntegString = fln.substr(22, wi_it - (22));
  try {
    IntegString.erase(
        remove_if(IntegString.begin(), IntegString.end(), isspace),
        IntegString.end());
    fi = Utils::str2d(IntegString, true);
  } catch (std::exception const &e) {
    UDBWarn("Input flux file integral comment (\""
            << IntegString
            << "\") could not be parsed for the "
               "flux species "
               "associated with \""
            << histname << "\".");
    UDBWarn("From flux line: " << fln);
    UDBWarn("Parsing exception: " << e.what());
    fi = 0;
  }

  UDBLog("Parsed flux integral as: " << fi << " from line: " << fln);

  double fci;
  IntegString = fln.substr(wi_it + 17, end_bracket - (wi_it + 17));
  try {
    IntegString.erase(
        remove_if(IntegString.begin(), IntegString.end(), isspace),
        IntegString.end());
    fci = Utils::str2d(IntegString, true);
  } catch (std::exception const &e) {
    UDBWarn("Input flux file width integral comment (\""
            << IntegString
            << "\") could not be parsed for the "
               "flux species "
               "associated with \""
            << histname
            << "\", it will not be correctly normalisable in a "
               "multi-species sample.");
    UDBWarn("From flux line: " << fln);
    UDBWarn("Parsing exception: " << e.what());
    return std::pair<double, double>(0, 0);
  }

  UDBLog("Parsed flux width integral as: " << fci << " from line: " << fln);

  FluxComponentIntegrals[pdgfromhistname] = fci;

  if (fci > DomFCI) {
    DomFCI = fci;
    DomPDG = pdgfromhistname;
  }

  return std::pair<double, double>(fi, fci);
}

void SaveFluxFile(std::string const &fileloc, std::string const &histname) {
  std::ifstream ifs(fileloc.c_str());
  if (!ifs.good()) {
    UDBError("File \"" << fileloc << " could not be opened for reading.");
    return;
  }
  std::string line;

  int pdgfromhistname = GetPDGFromHistName(histname);
  if (!pdgfromhistname) {
    UDBError("Failed to parse the correct species from histname:\"" << histname
                                                                    << "\"");
    throw;
  }

  size_t ln = 0;
  std::vector<std::pair<double, double>> FluxValues;
  std::pair<double, double> FluxIntegrals(0, 0);
  while (std::getline(ifs, line)) {
    if (line[0] == '#') { // ignore comments
      if (ln == 0) {
        FluxIntegrals = HandleFluxIntegralLine(line, histname, pdgfromhistname);
      }
      ln++;
      continue;
    }
    UDBVerbose("Flux file line[" << ln << "]: " << line);
    std::vector<double> splitLine =
        Utils::StringVToDoubleV(Utils::SplitStringByDelim(line, " \t,"));
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

  double *BinLowEdges = new double[FluxValues.size() + 1];
  for (size_t bin_it = 1; bin_it < FluxValues.size(); ++bin_it) {
    BinLowEdges[bin_it] =
        FluxValues[bin_it - 1].first +
        (FluxValues[bin_it].first - FluxValues[bin_it - 1].first) / 2.0;
  }
  BinLowEdges[0] = FluxValues[0].first - (BinLowEdges[1] - FluxValues[0].first);
  BinLowEdges[FluxValues.size()] = FluxValues[FluxValues.size() - 1].first +
                                   (FluxValues[FluxValues.size() - 1].first -
                                    BinLowEdges[FluxValues.size() - 1]);

  FluxHists[pdgfromhistname] =
      new TH1D(histname.c_str(),
               (histname + ";#it{E}_{#nu} (GeV);#Phi (A.U.) per GeV").c_str(),
               FluxValues.size(), BinLowEdges);

  SigmaHists[pdgfromhistname] = new TH1D(
      (histname + "_xsec").c_str(),
      (histname + ";#it{E}_{#nu} (GeV);#sigma(E_{#nu}) (cm^{2} nucleon^{-1})")
          .c_str(),
      FluxValues.size(), BinLowEdges);
  EvHists[pdgfromhistname] =
      new TH1D((histname + "_evrate").c_str(),
               (histname + ";#it{E}_{#nu} (GeV);Events (A.U.) per GeV").c_str(),
               FluxValues.size(), BinLowEdges);
  delete BinLowEdges;

  for (Int_t bin_it = 1; bin_it < FluxHists[pdgfromhistname]->GetNbinsX() + 1;
       bin_it++) {
    FluxHists[pdgfromhistname]->SetBinContent(bin_it,
                                              FluxValues[bin_it - 1].second);
  }

  if (FluxIntegrals.first > std::numeric_limits<double>::min()) {
    FluxHists[pdgfromhistname]->Scale(1. /
                                      FluxHists[pdgfromhistname]->Integral());
    FluxHists[pdgfromhistname]->Scale(FluxIntegrals.first);
  }

  std::cout << "Input integrals: " << FluxIntegrals.first << ", "
            << FluxIntegrals.second
            << ". Histo integrals: " << FluxHists[pdgfromhistname]->Integral()
            << ", " << FluxHists[pdgfromhistname]->Integral("width")
            << std::endl;
}

int GiBUUToStdHep() {
  TFile *outFile = new TFile(GiBUUToStdHepOpts::OutFName.c_str(), "RECREATE");
  if (!outFile->IsOpen()) {
    UDBError("Couldn't open output file.");
    return 2;
  }

  TTree *rooTrackerTree = new TTree("giRooTracker", "GiBUU StdHepVariables");
  GiRooTracker *giRooTracker = new GiRooTracker();
  int EventMode = 0;
  if(GiBUUToStdHepOpts::IsElectronScattering){
    EventMode = 1;
  } else if(GiBUUToStdHepOpts::IsNDK){
    EventMode = 2;
  }
  giRooTracker->AddBranches(rooTrackerTree, true,
                            GiBUUToStdHepOpts::HaveProdChargeInfo,
                            EventMode);

  // Handle the fluxes first so that we know the relative normalisations
  for (size_t ff_it = 0; ff_it < GiBUUToStdHepOpts::FluxFilesToAdd.size();
       ++ff_it) {
    SaveFluxFile(GiBUUToStdHepOpts::FluxFilesToAdd[ff_it].second,
                 GiBUUToStdHepOpts::FluxFilesToAdd[ff_it].first);
  }
  if (GiBUUToStdHepOpts::IsElectronScattering) {
    DomPDG = 11;

    FluxHists[DomPDG] = new TH1D("e_flux", "e_flux", 100, 0,
                                 2 * GiBUUToStdHepOpts::EScatteringInputEnergy);
    FluxHists[DomPDG]->Fill(GiBUUToStdHepOpts::EScatteringInputEnergy);

    EvHists[DomPDG] = static_cast<TH1D *>(FluxHists[DomPDG]->Clone("e_evt"));
    SigmaHists[DomPDG] =
        static_cast<TH1D *>(FluxHists[DomPDG]->Clone("e_xsec"));

    EvHists[DomPDG]->Reset();
    SigmaHists[DomPDG]->Reset();
  }

  if (DomPDG) {
    DomFlux = static_cast<TH1D *>(FluxHists[DomPDG]->Clone("flux"));
    DomEvt = static_cast<TH1D *>(EvHists[DomPDG]->Clone("evt"));
  }

  int ParserRtnCode = 0;
  ParserRtnCode = ParseACSIIEventVectors(rooTrackerTree, giRooTracker);

  rooTrackerTree->Write();

  outFile->Write();
  outFile->Close();
  delete giRooTracker;
  giRooTracker = nullptr;
  delete outFile;
  outFile = nullptr;
  return ParserRtnCode;
}

int main(int argc, char const *argv[]) {
  if (!GiBUUToStdHep_CLIOpts::HandleArgs(argc, argv)) {
    GiBUUToStdHep_CLIOpts::SayRunLike(argv);
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
