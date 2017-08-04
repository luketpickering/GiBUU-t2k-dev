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
    pblob.Run = Utils::str2i(splitLine[0]);
    pblob.EvNum = Utils::str2i(splitLine[1]);
    pblob.ID = Utils::str2i(splitLine[2]);
    pblob.Charge = Utils::str2i(splitLine[3]);
    pblob.PerWeight = Utils::str2f(splitLine[4]);
    pblob.Position[GiRooTracker::kStdHepIdxPx] = Utils::str2f(splitLine[5]);
    pblob.Position[GiRooTracker::kStdHepIdxPy] = Utils::str2f(splitLine[6]);
    pblob.Position[GiRooTracker::kStdHepIdxPz] = Utils::str2f(splitLine[7]);
    pblob.FourMom[GiRooTracker::kStdHepIdxE] = Utils::str2f(splitLine[8]);
    pblob.FourMom[GiRooTracker::kStdHepIdxPx] = Utils::str2f(splitLine[9]);
    pblob.FourMom[GiRooTracker::kStdHepIdxPy] = Utils::str2f(splitLine[10]);
    pblob.FourMom[GiRooTracker::kStdHepIdxPz] = Utils::str2f(splitLine[11]);
    pblob.History = Utils::str2l(splitLine[12]);
    pblob.Prodid = Utils::str2i(splitLine[13]);
    pblob.Enu = Utils::str2f(splitLine[14]);
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
                         std::vector<std::vector<GiBUUPartBlob> > &Events) {
  size_t NumEvs = 0;

  double NRunsScaleFactor =
      GiBUUToStdHepOpts::NFilesAddedWeights[fileNumber] / double(NRunsInFile);
  bool FileIsCC = GiBUUToStdHepOpts::CCFiles[fileNumber];
  int FileNuType = GiBUUToStdHepOpts::nuTypes[fileNumber];
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

    giRooTracker->SpeciesWght_numu =
        (abs(FileNuType) == 14)
            ? GiBUUToStdHepOpts::CompositeFluxWeight[FileNuType + 100]
            : 0;
    giRooTracker->SpeciesWght_nue =
        (abs(FileNuType) == 12)
            ? GiBUUToStdHepOpts::CompositeFluxWeight[FileNuType + 100]
            : 0;
    giRooTracker->SpeciesWght =
        GiBUUToStdHepOpts::CompositeFluxWeight[FileNuType];

    giRooTracker->StdHepStatus[0] = 0;
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPx] = 0;
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPy] = 0;
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPz] = ev.front().Enu;
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxE] = ev.front().Enu;

    if (GiBUUToStdHepOpts::EScatteringInputEnergy = 0xdeadbeef) {
      GiBUUToStdHepOpts::EScatteringInputEnergy = ev.front().Enu;
    } else if (fabs(GiBUUToStdHepOpts::EScatteringInputEnergy -
                    ev.front().Enu) > 1E-5) {
      UDBError("Read a differing input energy: First event: "
               << GiBUUToStdHepOpts::EScatteringInputEnergy << ", event "
               << EvNum << " in file \""
               << GiBUUToStdHepOpts::InpFNames[fileNumber] << "\" had "
               << ev.front().Enu);
      throw;
    }

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
    giRooTracker->EvtWght = giRooTracker->GiBUUPerWeight * TotalEventReweight *
                            (GiBUUToStdHepOpts::IsElectronScattering ? 1E5 : 1);

    giRooTracker->StdHepN = 2;

    bool BadEv = false;
    for (size_t p_it = 0; p_it < ev.size(); ++p_it) {
      GiBUUPartBlob const &part = ev[p_it];
      if (!part.EvNum) {  // Malformed line
        UDBWarn("Skipping event due to malformed line.");
        BadEv = true;
        break;
      }

      if (GiBUUToStdHepOpts::HaveStruckNucleonInfo &&
          (giRooTracker->StdHepN == 3)) {
        giRooTracker->StdHepStatus[giRooTracker->StdHepN] = 11;
      } else {
        giRooTracker->StdHepStatus[giRooTracker->StdHepN] = 1;  // All other FS
      }  // should be good.

      giRooTracker->StdHepPdg[giRooTracker->StdHepN] =
          GiBUUUtils::GiBUUToPDG(part.ID, part.Charge);

      if (!giRooTracker->StdHepPdg[giRooTracker->StdHepN]) {
        UDBWarn("Parsed part: " << part << " from file "
                                << GiBUUToStdHepOpts::InpFNames[fileNumber]
                                << " to have a PDG of 0.");
      }
      // A known unknown
      if (giRooTracker->StdHepPdg[giRooTracker->StdHepN] == -1) {
        giRooTracker->StdHepPdg[giRooTracker->StdHepN] = 0;
      }

      giRooTracker->StdHepP4[giRooTracker->StdHepN]
                            [GiRooTracker::kStdHepIdxPx] = part.FourMom.X();
      giRooTracker->StdHepP4[giRooTracker->StdHepN]
                            [GiRooTracker::kStdHepIdxPy] = part.FourMom.Y();
      giRooTracker->StdHepP4[giRooTracker->StdHepN]
                            [GiRooTracker::kStdHepIdxPz] = part.FourMom.Z();
      giRooTracker->StdHepP4[giRooTracker->StdHepN][GiRooTracker::kStdHepIdxE] =
          part.FourMom.E();

      giRooTracker->GiBHepHistory[giRooTracker->StdHepN] = part.History;
#ifndef CPP03COMPAT
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
    }  // If we broke then don't bother continuing
       // processing.

    if (GiBUUToStdHepOpts::IsElectronScattering) {
      giRooTracker->GiBUU2NeutCode = GiBUUUtils::GiBUU2NeutReacCode_escat(
          giRooTracker->GiBUUReactionCode, giRooTracker->StdHepPdg);
    } else {
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

    if (UDBDebugging::GetInfoLevel() > 3) {
      UDBVerbose("[INFO]: EvNo: "
                 << EvNum << ", contained " << giRooTracker->StdHepN << " ("
                 << ev.size() << ") particles. "
                                 "Event Weight: "
                 << std::setprecision(3) << giRooTracker->GiBUUPerWeight
                 << "\n\tGiBUUReactionCode: " << giRooTracker->GiBUUReactionCode
                 << ", NeutConventionReactionCode: "
                 << giRooTracker->GiBUU2NeutCode << "\n\t[Lep In] : "
                 << TLorentzVector(
                        giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPx],
                        giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPy],
                        giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPz],
                        giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxE]));
      UDBVerbose("\t[Target] : " << giRooTracker->StdHepPdg[1]);
      if (GiBUUToStdHepOpts::HaveStruckNucleonInfo) {
        UDBVerbose("\t[Nuc In] : "
                   << TLorentzVector(
                          giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxPx],
                          giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxPy],
                          giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxPz],
                          giRooTracker->StdHepP4[3][GiRooTracker::kStdHepIdxE])
                   << " (" << std::setw(4) << giRooTracker->StdHepPdg[3]
                   << ")");
      }

      // We have already printed the struck nucleon
      Int_t StartPoint = ((!GiBUUToStdHepOpts::HaveStruckNucleonInfo) ? 3 : 4);
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
#ifndef CPP03COMPAT
                  << " (H:" << giRooTracker->GiBHepHistory[stdHepInd] << ")"
#endif

            );
#ifndef CPP03COMPAT
        UDBVerbose("\t\t" << GiBUUUtils::WriteGiBUUHistory(
                       giRooTracker->GiBHepHistory[stdHepInd]));
#endif
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

  for (size_t fname_it = 0; fname_it < GiBUUToStdHepOpts::InpFNames.size();
       ++fname_it) {
    std::string const &fname = GiBUUToStdHepOpts::InpFNames[fname_it];
    std::ifstream ifs(fname.c_str());

    if (!ifs.good()) {
      UDBError("Failed to open " << fname << " for reading.");
      return 1;
    }

    /// Get NRuns
    size_t NRunsInFile = 0;
    size_t NEvsInFile = 0;
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

      if (line[0] == '#') {  // Skip comments
        continue;
        LineNum++;
      }
      GiBUUPartBlob const &part = GetParticleLine(line);

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

        // Flush events every 50k events
        if (FileEvents.size() == 5E4) {
          NEvsInFile += FlushEventsToDisk(OutputTree, giRooTracker, fileNumber,
                                      NRunsInFile, FileEvents);
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

    // Flush events every 10k events
    if (FileEvents.size()) {
      NEvsInFile += FlushEventsToDisk(OutputTree, giRooTracker, fileNumber,
                                  NRunsInFile, FileEvents);
    }

    ifs.close();  // Read all the lines.

    UDBLog("Found " << NEvsInFile << " events in " << fname << ".");

    if (!NEvsInFile) {
      continue;
    }

    fileNumber++;
    NumEvs += NEvsInFile;
  }

  UDBInfo("Saved " << NumEvs << " events.");
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
                                                 std::string const &histname) {
  int pdgfromhistname = GetPDGFromHistName(histname);
  if (!pdgfromhistname) {
    UDBWarn("Failed to parse the correct species from histname:\"" << histname
                                                                   << "\"");
    return std::pair<double, double>(0, 0);
  }
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
    fi = Utils::str2d(IntegString, true);
  } catch (std::exception const &e) {
    UDBWarn("Input flux file integral comment (\""
            << IntegString << "\") could not be parsed for the "
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
    fci = Utils::str2d(IntegString, true);
  } catch (std::exception const &e) {
    UDBWarn("Input flux file width integral comment (\""
            << IntegString << "\") could not be parsed for the "
                              "flux species "
                              "associated with \""
            << histname << "\", it will not be correctly normalisable in a "
                           "multi-species sample.");
    UDBWarn("From flux line: " << fln);
    UDBWarn("Parsing exception: " << e.what());
    return std::pair<double, double>(0, 0);
  }

  UDBLog("Parsed flux width integral as: " << fci << " from line: " << fln);

  FluxComponentIntegrals[pdgfromhistname] = fci;

  return std::pair<double, double>(fi, fci);
}

void SaveFluxFile(std::string const &fileloc, std::string const &histname) {
  std::ifstream ifs(fileloc.c_str());
  if (!ifs.good()) {
    UDBError("File \"" << fileloc << " could not be opened for reading.");
    return;
  }
  std::string line;

  size_t ln = 0;
  std::vector<std::pair<double, double> > FluxValues;
  std::pair<double, double> FluxIntegrals(0, 0);
  while (std::getline(ifs, line)) {
    if (line[0] == '#') {  // ignore comments
      if (ln == 0) {
        FluxIntegrals = HandleFluxIntegralLine(line, histname);
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

  TH1D *fluxHist = new TH1D(
      histname.c_str(), (histname + ";#it{E}_{#nu} (GeV);#Phi (A.U.)").c_str(),
      FluxValues.size(), BinLowEdges);
  delete BinLowEdges;

  for (Int_t bin_it = 1; bin_it < fluxHist->GetNbinsX() + 1; bin_it++) {
    fluxHist->SetBinContent(bin_it, FluxValues[bin_it - 1].second);
  }

  if (FluxIntegrals.first > 1E-8) {
    fluxHist->Scale(1. / fluxHist->Integral());
    fluxHist->Scale(FluxIntegrals.first);
  }

  fluxHist->Write();
}

void SetFluxSpeciesWeights() {
  if (!FluxComponentIntegrals.count(14)) {
    FluxComponentIntegrals[14] = 0;
  }
  if (!FluxComponentIntegrals.count(-14)) {
    FluxComponentIntegrals[-14] = 0;
  }
  if (!FluxComponentIntegrals.count(12)) {
    FluxComponentIntegrals[12] = 0;
  }
  if (!FluxComponentIntegrals.count(-12)) {
    FluxComponentIntegrals[-12] = 0;
  }

  if (FluxComponentIntegrals[14]) {
    GiBUUToStdHepOpts::CompositeFluxWeight[14] =
        FluxComponentIntegrals[14] /
        (FluxComponentIntegrals[14] + FluxComponentIntegrals[-14] +
         FluxComponentIntegrals[12] + FluxComponentIntegrals[-12]);
  } else {
    GiBUUToStdHepOpts::CompositeFluxWeight[14] = 0;
  }
  if (FluxComponentIntegrals[-14]) {
    GiBUUToStdHepOpts::CompositeFluxWeight[-14] =
        FluxComponentIntegrals[-14] /
        (FluxComponentIntegrals[14] + FluxComponentIntegrals[-14] +
         FluxComponentIntegrals[12] + FluxComponentIntegrals[-12]);
  } else {
    GiBUUToStdHepOpts::CompositeFluxWeight[-14] = 0;
  }
  if (FluxComponentIntegrals[14]) {
    GiBUUToStdHepOpts::CompositeFluxWeight[14 + 100] =
        FluxComponentIntegrals[14] /
        (FluxComponentIntegrals[14] + FluxComponentIntegrals[-14]);
  } else {
    GiBUUToStdHepOpts::CompositeFluxWeight[14 + 100] = 0;
  }
  if (FluxComponentIntegrals[-14]) {
    GiBUUToStdHepOpts::CompositeFluxWeight[-14 + 100] =
        FluxComponentIntegrals[-14] /
        (FluxComponentIntegrals[14] + FluxComponentIntegrals[-14]);
  } else {
    GiBUUToStdHepOpts::CompositeFluxWeight[-14 + 100] = 0;
  }

  if (FluxComponentIntegrals[12]) {
    GiBUUToStdHepOpts::CompositeFluxWeight[12] =
        FluxComponentIntegrals[12] /
        (FluxComponentIntegrals[14] + FluxComponentIntegrals[-14] +
         FluxComponentIntegrals[12] + FluxComponentIntegrals[-12]);
  } else {
    GiBUUToStdHepOpts::CompositeFluxWeight[12] = 0;
  }
  if (FluxComponentIntegrals[-12]) {
    GiBUUToStdHepOpts::CompositeFluxWeight[-12] =
        FluxComponentIntegrals[-12] /
        (FluxComponentIntegrals[14] + FluxComponentIntegrals[-14] +
         FluxComponentIntegrals[12] + FluxComponentIntegrals[-12]);
  } else {
    GiBUUToStdHepOpts::CompositeFluxWeight[-12] = 0;
  }
  if (FluxComponentIntegrals[12]) {
    GiBUUToStdHepOpts::CompositeFluxWeight[12 + 100] =
        FluxComponentIntegrals[12] /
        (FluxComponentIntegrals[12] + FluxComponentIntegrals[-12]);
  } else {
    GiBUUToStdHepOpts::CompositeFluxWeight[12 + 100] = 0;
  }
  if (FluxComponentIntegrals[-12]) {
    GiBUUToStdHepOpts::CompositeFluxWeight[-12 + 100] =
        FluxComponentIntegrals[-12] /
        (FluxComponentIntegrals[12] + FluxComponentIntegrals[-12]);
  } else {
    GiBUUToStdHepOpts::CompositeFluxWeight[-12 + 100] = 0;
  }
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
                            GiBUUToStdHepOpts::HaveProdChargeInfo,
                            GiBUUToStdHepOpts::IsElectronScattering);

  // Handle the fluxes first so that we know the relative normalisations
  for (size_t ff_it = 0; ff_it < GiBUUToStdHepOpts::FluxFilesToAdd.size();
       ++ff_it) {
    SaveFluxFile(GiBUUToStdHepOpts::FluxFilesToAdd[ff_it].second,
                 GiBUUToStdHepOpts::FluxFilesToAdd[ff_it].first);
  }
  SetFluxSpeciesWeights();

  int ParserRtnCode = 0;
  ParserRtnCode = ParseFinalEventsFile(rooTrackerTree, giRooTracker);

  if (GiBUUToStdHepOpts::IsElectronScattering) {
    TH1D *eFlux = new TH1D("e_flux", "e_flux", 100, 0,
                           2 * GiBUUToStdHepOpts::EScatteringInputEnergy);
    eFlux->Fill(GiBUUToStdHepOpts::EScatteringInputEnergy);
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
