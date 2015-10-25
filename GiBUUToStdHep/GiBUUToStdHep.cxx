#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TLorentzVector.h"

#include "LHEF.hpp"

#include "GiBUUToStdHep_Utils.hxx"

#include "GiRooTracker.hxx"

namespace {
inline std::ostream & operator<<(std::ostream & os, TLorentzVector const & tlv){
  return os << "[ " << tlv[0] << ", " << tlv[1] << ", " << tlv[2] << ", "
    << tlv[3] << " : M(" << tlv.M() << ") ]";
}
}

///Options relevant to the GiBUUToStdHep.exe executable.
namespace GiBUUToStdHepOpts {

///The location of the input file which was produced by GiBUU.
std::string InpFName;
///The name of the output root file to write.
std::string OutFName;
///Whether the input is a FinalEvents.dat type file.
bool InpIsFE;
///Whether the input is a LesHouchesXXX.xml type files.
bool InpIsLH;
///Whether the GiBUU output contains struck nucleon information.
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

}

std::vector<std::string> SplitStringByDelim(std::string const &inp,
  char const *delim, bool PushEmpty=false){

  size_t nextOccurence = 0;
  size_t prevOccurence = 0;
  std::vector<std::string> outV;
  bool AtEnd = false;
  while(!AtEnd){
    nextOccurence = inp.find_first_of(delim,prevOccurence);
    if(nextOccurence == std::string::npos){
      if(prevOccurence == inp.length()){
        break;
      }
      AtEnd = true;
    }
    if(PushEmpty || (nextOccurence != prevOccurence)){
      outV.push_back(inp.substr(prevOccurence,(nextOccurence-prevOccurence)));
    }
    prevOccurence = nextOccurence+1;
  }
  return outV;
}

std::tuple<int,int,int,int,float,TVector3,TLorentzVector,long, int,float>
GetParticleLine(std::string const &line){

  int Run, EvNum, ID, Charge, Prodid;
  long History;
  double PerWeight,Pos1,Pos2,Pos3,MomE,Mom1,Mom2,Mom3,Enu;

  auto const &splitLine = SplitStringByDelim(line, " ");
  if(splitLine.size() != 15){
    std::cout << "[WARN]: Event had malformed particle line: \""
      << line << "\"" << std::endl;
    return std::make_tuple(0,0,0,0,0, TVector3(0,0,0),
      TLorentzVector(0,0,0,0),0,0,0);
  }

  try{

    Run = std::stoi(splitLine[0]);
    EvNum = std::stoi(splitLine[1]);
    ID = std::stoi(splitLine[2]);
    Charge = std::stoi(splitLine[3]);
    PerWeight = std::stod(splitLine[4]);
    Pos1 = std::stod(splitLine[5]);
    Pos2 = std::stod(splitLine[6]);
    Pos3 = std::stod(splitLine[7]);
    MomE = std::stod(splitLine[8]);
    Mom1 = std::stod(splitLine[9]);
    Mom2 = std::stod(splitLine[10]);
    Mom3 = std::stod(splitLine[11]);
    History = std::stol(splitLine[12]);
    Prodid = std::stoi(splitLine[13]);
    Enu = std::stod(splitLine[14]);

  } catch (const std::invalid_argument& ia) {
  std::cout << "[WARN]: Failed to parse one of the values: \""
    << line << "\"" << std::endl;
    throw;
  }
  // std::cout << "[PARSED]: Run: " << Run << std::endl;
  // std::cout << "[PARSED]: EvNum: " << EvNum << std::endl;
  // std::cout << "[PARSED]: ID: " << ID << std::endl;
  // std::cout << "[PARSED]: Charge: " << Charge << std::endl;
  // std::cout << "[PARSED]: PerWeight: " << PerWeight << std::endl;
  // std::cout << "[PARSED]: Pos1: " << Pos1 << std::endl;
  // std::cout << "[PARSED]: Pos2: " << Pos2 << std::endl;
  // std::cout << "[PARSED]: Pos3: " << Pos3 << std::endl;
  // std::cout << "[PARSED]: MomE: " << MomE << std::endl;
  // std::cout << "[PARSED]: Mom1: " << Mom1 << std::endl;
  // std::cout << "[PARSED]: Mom2: " << Mom2 << std::endl;
  // std::cout << "[PARSED]: Mom3: " << Mom3 << std::endl;
  // std::cout << "[PARSED]: History: " << History << std::endl;
  // std::cout << "[PARSED]: Prodid: " << Prodid << std::endl;
  // std::cout << "[PARSED]: Enu: " << Enu << std::endl;

  return std::make_tuple(Run, EvNum, ID, Charge, PerWeight, TVector3(Pos1,Pos2,Pos3),
    TLorentzVector(Mom1,Mom2,Mom3,MomE), History, Prodid, Enu);

}

int ParseFinalEventsFile(TTree *OutputTree, GiRooTracker *giRooTracker){

  std::ifstream ifs(GiBUUToStdHepOpts::InpFName);
  std::string line;
  long ctr = 0;

  int LastEvNum = 0;

  bool isHOREv = false;

  while(std::getline(ifs,line)){
    if(!ctr){ctr++;continue;} //skip the table header

    int Run, EvNum, Charge, ID, Prodid;
    long History;
    double PerWeight,Enu;
    TVector3 pos(0,0,0);
    TLorentzVector mom4(0,0,0,0);

    auto const & part = GetParticleLine(line);

    std::tie(Run, EvNum, ID, Charge, PerWeight, pos, mom4, History, Prodid,
      Enu) = part;

    if(EvNum != LastEvNum){
      if(LastEvNum){
        if(isHOREv && !giRooTracker->GiBUU2NeutCode){
          std::cout << "[WARN]: Missed a GiBUU reaction code: " << Prodid
            << std::endl;
        }
        std::cout << "====" << std::endl;
        OutputTree->Fill();
        giRooTracker->Reset();
        isHOREv = false;
      }
      if(!(EvNum%1000)){ std::cout << "On Ev: " << EvNum << std::endl; }
      if(GiBUUToStdHepOpts::MaxEntries == EvNum){
        std::cout << "Finishing after " << EvNum << " entries." << std::endl;
        break;
      }

      giRooTracker->EvtNum = EvNum;

      //neutrino
      giRooTracker->StdHepPdg[0] = GiBUUToStdHepOpts::nuType;
      giRooTracker->StdHepStatus[0] = -1;
      giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPx] = 0;
      giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPy] = 0;
      giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPz] = Enu;
      giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxE] = Enu;

      //target
      giRooTracker->StdHepPdg[1] =
        GiBUUUtils::MakeNuclearPDG(GiBUUToStdHepOpts::TargetZ,
                                   GiBUUToStdHepOpts::TargetA);
      giRooTracker->StdHepStatus[1] = -1;
      giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPx] = 0;
      giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPy] = 0;
      giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPz] = 0;
      giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxE] =
        GiBUUToStdHepOpts::TargetA;

      giRooTracker->StdHepN = 2;
      LastEvNum = EvNum;
    }

    if(GiBUUToStdHepOpts::HaveStruckNucleonInfo && giRooTracker->StdHepN == 3){
      giRooTracker->StdHepPdg[giRooTracker->StdHepN] = 0;
      giRooTracker->StdHepStatus[giRooTracker->StdHepN] = 11;
    } else {
      giRooTracker->StdHepPdg[giRooTracker->StdHepN] =
        GiBUUUtils::GiBUUToPDG(ID,Charge);
      giRooTracker->StdHepStatus[giRooTracker->StdHepN] = 1;
    }

    giRooTracker->StdHepP4[giRooTracker->StdHepN]\
      [GiRooTracker::kStdHepIdxPx] = mom4.X();
    giRooTracker->StdHepP4[giRooTracker->StdHepN]\
      [GiRooTracker::kStdHepIdxPy] = mom4.Y();
    giRooTracker->StdHepP4[giRooTracker->StdHepN]\
      [GiRooTracker::kStdHepIdxPz] = mom4.Z();
    giRooTracker->StdHepP4[giRooTracker->StdHepN]\
      [GiRooTracker::kStdHepIdxE] = mom4.E();

    std::cout << "\t[" << giRooTracker->StdHepN << "] ("
      << giRooTracker->StdHepPdg[giRooTracker->StdHepN] << " | " << ID << ": "
      << Charge << ") "
      << mom4 << " | " << History << std::endl;

    giRooTracker->GiBUU2NeutCode = GiBUUUtils::GiBUU2NeutReacCode(Prodid,
      giRooTracker->StdHepPdg[giRooTracker->StdHepN]);

    if((Prodid == 32) || (Prodid == 33)){
      isHOREv = true;
    }

    giRooTracker->StdHepN++;

    ctr++;
  }
  OutputTree->Fill();
  ifs.close();
  return 0;
}

std::tuple<int, TLorentzVector,TLorentzVector, TLorentzVector>
  ParseAdditionInfoLine(std::string const &optLine){

  std::string scrubbedLine = optLine.substr(2); // scrub off the '# ';

  int Magic, EvId;
  float weight, nuE, nuPX, nuPY, nuPZ, clepE, clepPX, clepPY, clepPZ,
    bosE = 0, bosPX = 0, bosPY = 0, bosPZ = 0;

  auto const &splitLine = SplitStringByDelim(scrubbedLine, " ");
  if(splitLine.size() != (11 + (GiBUUToStdHepOpts::HaveStruckNucleonInfo?4:0))){
    std::cout << "[WARN]: Event had malformed additional info line: \""
      << optLine << "\"" << std::endl;
    return std::make_tuple(0, TLorentzVector(0,0,0,0),
      TLorentzVector(0,0,0,0),
      TLorentzVector(0,0,0,0));
  }

  try{
    Magic = std::stoi(splitLine[0]);(void)Magic;
    EvId = std::stoi(splitLine[1]);
    weight = std::stof(splitLine[2]);(void)weight;
    nuE = std::stof(splitLine[3]);
    nuPX = std::stof(splitLine[4]);
    nuPY = std::stof(splitLine[5]);
    nuPZ = std::stof(splitLine[6]);
    clepE = std::stof(splitLine[7]);
    clepPX = std::stof(splitLine[8]);
    clepPY = std::stof(splitLine[9]);
    clepPZ = std::stof(splitLine[10]);
    if(GiBUUToStdHepOpts::HaveStruckNucleonInfo){
      bosE = std::stof(splitLine[11]);
      bosPX = std::stof(splitLine[12]);
      bosPY = std::stof(splitLine[13]);
      bosPZ = std::stof(splitLine[14]);
    }
  } catch (const std::invalid_argument& ia) {
  std::cout << "[WARN]: Failed to parse one of the values: \""
    << optLine << "\"" << std::endl;
    throw;
  }

  // std::cout << "[PARSED] : Magic = " << Magic << " from " << splitLine[0] << std::endl;
  // std::cout << "[PARSED] : EvId = " << EvId << " from " << splitLine[1] << std::endl;
  // std::cout << "[PARSED] : weight = " << weight << " from " << splitLine[2] << std::endl;
  // std::cout << "[PARSED] : nuE = " << nuE << " from " << splitLine[3] << std::endl;
  // std::cout << "[PARSED] : nuPX = " << nuPX << " from " << splitLine[4] << std::endl;
  // std::cout << "[PARSED] : nuPY = " << nuPY << " from " << splitLine[5] << std::endl;
  // std::cout << "[PARSED] : nuPZ = " << nuPZ << " from " << splitLine[6] << std::endl;
  // std::cout << "[PARSED] : clepE = " << clepE << " from " << splitLine[7] << std::endl;
  // std::cout << "[PARSED] : clepPX = " << clepPX << " from " << splitLine[8] << std::endl;
  // std::cout << "[PARSED] : clepPY = " << clepPY << " from " << splitLine[9] << std::endl;
  // std::cout << "[PARSED] : clepPZ = " << clepPZ << " from " << splitLine[10] << std::endl;
  // std::cout << "[PARSED] : bosE = " << bosE << " from " << splitLine[11] << std::endl;
  // std::cout << "[PARSED] : bosPX = " << bosPX << " from " << splitLine[12] << std::endl;
  // std::cout << "[PARSED] : bosPY = " << bosPY << " from " << splitLine[13] << std::endl;
  // std::cout << "[PARSED] : bosPZ = " << bosPZ << " from " << splitLine[14] << std::endl;

  return std::make_tuple(EvId, TLorentzVector(nuPX,nuPY,nuPZ,nuE),
    TLorentzVector(clepPX,clepPY,clepPZ,clepE),
    TLorentzVector(bosPX,bosPY,bosPZ,bosE));
}



int ParseLesHouchesFile(TTree *OutputTree, GiRooTracker *giRooTracker){
  int EvNum = 0;
  LHPC::LhefParser LHEFParser(GiBUUToStdHepOpts::InpFName,true);

  LHPC::LHEF::LhefEvent const& currentEvent = LHEFParser.getEvent();
  while(LHEFParser.readNextEvent()){
    std::cout << "[INFO] : EvNo: " << currentEvent.getEventNumberInFile()
    << ", contained "
    << currentEvent.getNumberOfParticles() << " particles:" << std::endl;
    std::tuple<int, TLorentzVector,TLorentzVector, TLorentzVector> const &
      ExtraInfo = ParseAdditionInfoLine(currentEvent.getOptionalInformation());

    std::cout << "NeutEquivMode: " <<
      GiBUUUtils::GiBUU2NeutReacCode(std::get<0>(ExtraInfo),0) << std::endl;
    std::cout << "\t[Lep In]: " << std::get<1>(ExtraInfo) << std::endl;

    if(GiBUUToStdHepOpts::HaveStruckNucleonInfo){
      std::cout << "\t[Bos In]: " << std::get<3>(ExtraInfo) << std::endl;
    }
    giRooTracker->EvtNum = currentEvent.getEventNumberInFile();

    //neutrino
    giRooTracker->StdHepPdg[0] = GiBUUToStdHepOpts::nuType;
    giRooTracker->StdHepStatus[0] = -1;
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPx] =
      std::get<1>(ExtraInfo).X();
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPy] =
      std::get<1>(ExtraInfo).Y();
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPz] =
      std::get<1>(ExtraInfo).Z();
    giRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxE] =
      std::get<1>(ExtraInfo).E();

    //target
    giRooTracker->StdHepPdg[1] =
      GiBUUUtils::MakeNuclearPDG(GiBUUToStdHepOpts::TargetZ,
                                 GiBUUToStdHepOpts::TargetA);
    giRooTracker->StdHepStatus[1] = 11;
    giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPx] =
      std::get<3>(ExtraInfo).X();
    giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPy] =
      std::get<3>(ExtraInfo).Y();
    giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPz] =
      std::get<3>(ExtraInfo).Z();
    giRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxE] =
      GiBUUToStdHepOpts::TargetA;

    //lepout
    giRooTracker->StdHepPdg[2] = (GiBUUToStdHepOpts::nuType-1); // you hope.
    giRooTracker->StdHepStatus[2] = 1;
    giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPx] =
      std::get<2>(ExtraInfo).X();
    giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPy] =
      std::get<2>(ExtraInfo).Y();
    giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxPz] =
      std::get<2>(ExtraInfo).Z();
    giRooTracker->StdHepP4[2][GiRooTracker::kStdHepIdxE] =
      std::get<2>(ExtraInfo).E();

    giRooTracker->StdHepN = 3;

    for(int i = 0; i < currentEvent.getNumberOfParticles(); ++i){
      LHPC::LHEF::ParticleLine const &p = currentEvent.getLine(i+1);

      TLorentzVector fourmom(p.getXMomentum(),p.getYMomentum(),
        p.getZMomentum(),p.getEnergy());

      std::cout << "\t[" << i << "] (" << p.getParticleCode() << ") "
        << fourmom << std::endl;

      giRooTracker->StdHepPdg[giRooTracker->StdHepN] =
        p.getParticleCode();
      giRooTracker->StdHepStatus[giRooTracker->StdHepN] = 1;
      giRooTracker->StdHepP4[giRooTracker->StdHepN]\
        [GiRooTracker::kStdHepIdxPx] = p.getXMomentum();
      giRooTracker->StdHepP4[giRooTracker->StdHepN]\
        [GiRooTracker::kStdHepIdxPy] = p.getYMomentum();
      giRooTracker->StdHepP4[giRooTracker->StdHepN]\
        [GiRooTracker::kStdHepIdxPz] = p.getZMomentum();
      giRooTracker->StdHepP4[giRooTracker->StdHepN]\
        [GiRooTracker::kStdHepIdxE] = p.getEnergy();

      giRooTracker->GiBUU2NeutCode = GiBUUUtils::GiBUU2NeutReacCode(
        std::get<0>(ExtraInfo),
        giRooTracker->StdHepPdg[giRooTracker->StdHepN]);
      giRooTracker->StdHepN++;
    }

    std::cout << "\t[Lep Out]: " << std::get<2>(ExtraInfo) << std::endl;

    EvNum++;
    OutputTree->Fill();
    giRooTracker->Reset();

    if(GiBUUToStdHepOpts::MaxEntries == EvNum){
      std::cout << "Finishing after " << EvNum << " entries." << std::endl;
      break;
    }
  }
  std::cout << "Read " << EvNum << " events." << std::endl;
  return 0;
}

int GiBUUToStdHep(){

  TFile* outFile = new TFile(GiBUUToStdHepOpts::OutFName.c_str(),"CREATE");
  if(!outFile->IsOpen()){
    std::cout << "Couldn't open output file." << std::endl;
    return 2;
  }

  TTree* rooTrackerTree = new TTree("giRooTracker","GiBUU StdHepVariables");
  GiRooTracker* giRooTracker = new GiRooTracker();
  giRooTracker->AddBranches(rooTrackerTree);

  int ParserRtnCode = 0;
  if(GiBUUToStdHepOpts::InpIsLH){
    ParserRtnCode = ParseLesHouchesFile(rooTrackerTree, giRooTracker);
  } else if (GiBUUToStdHepOpts::InpIsFE){
    ParserRtnCode = ParseFinalEventsFile(rooTrackerTree, giRooTracker);
  }

  rooTrackerTree->Write();
  outFile->Write();
  outFile->Close();
  delete giRooTracker; giRooTracker = nullptr;
  delete outFile; outFile = nullptr;
  return ParserRtnCode;
}

void SetOpts(){
  CLIUtils::OptSpec.emplace_back("-h","--help", false,
    [&] (std::string const &opt) -> bool {
      CLIUtils::SayRunLike();
      exit(0);
    });

  CLIUtils::OptSpec.emplace_back("-f", "--FEinput-file", true,
    [&] (std::string const &opt) -> bool {
      std::cout << "\t--Reading FinalEvents-style GiBUU file : "
      << opt << std::endl;
      GiBUUToStdHepOpts::InpFName = opt;
      GiBUUToStdHepOpts::InpIsFE = true;
      if(GiBUUToStdHepOpts::InpIsLH){
        std::cerr << "[ERROR] only one style of input allowed, -l already used."
          << std::endl;
          throw 6;
      }
      return true;
    }, false,[](){GiBUUToStdHepOpts::InpIsFE = false;},"<File Name>");

  CLIUtils::OptSpec.emplace_back("-l", "--LHinput-file", true,
    [&] (std::string const &opt) -> bool {
      std::cout << "\t--Reading LesHouches Event Format GiBUU file : "
      << opt << std::endl;
      GiBUUToStdHepOpts::InpFName = opt;
      GiBUUToStdHepOpts::InpIsLH = true;
      if(GiBUUToStdHepOpts::InpIsFE){
        std::cerr << "[ERROR] only one style of input allowed, -f already used."
          << std::endl;
          throw 6;
      }
      return true;
    }, false,[](){GiBUUToStdHepOpts::InpIsLH = false;},"<File Name>");

  CLIUtils::OptSpec.emplace_back("-o", "--output-file", true,
    [&] (std::string const &opt) -> bool {
      GiBUUToStdHepOpts::OutFName = opt;
      std::cout << "\t--Writing to file "
      << opt << std::endl;
      return true;
    }, false,[](){GiBUUToStdHepOpts::OutFName = "GiBUURooTracker.root";},
      "<File Name {default:GiBUURooTracker.root}>");


  CLIUtils::OptSpec.emplace_back("-u", "--nu-pdg", true,
    [&] (std::string const &opt) -> bool {
      int vbhold;
      if(GiBUUUtils::str2int(vbhold,opt.c_str()) == GiBUUUtils::STRINT_SUCCESS){
        std::cout << "\t--Nu PDG: " << vbhold << std::endl;
        GiBUUToStdHepOpts::nuType = vbhold;
        return true;
      }
      return false;
    }, true,[](){},"<Neutrino PDG identifier>");

  CLIUtils::OptSpec.emplace_back("-a", "--target-a", true,
    [&] (std::string const &opt) -> bool {
      int vbhold;
      if(GiBUUUtils::str2int(vbhold,opt.c_str()) == GiBUUUtils::STRINT_SUCCESS){
        std::cout << "\t--Target A: " << vbhold << std::endl;
        GiBUUToStdHepOpts::TargetA = vbhold;
        return true;
      }
      return false;
    }, true,[](){},"<Target A>");

  CLIUtils::OptSpec.emplace_back("-z", "--target-z", true,
    [&] (std::string const &opt) -> bool {
      int vbhold;
      if(GiBUUUtils::str2int(vbhold,opt.c_str()) == GiBUUUtils::STRINT_SUCCESS){
        std::cout << "\t--Target Z: " << vbhold << std::endl;
        GiBUUToStdHepOpts::TargetZ = vbhold;
        return true;
      }
      return false;
    }, true,[](){},"<Target Z>");

  CLIUtils::OptSpec.emplace_back("-v", "--GiBUUToStdHepOpts::verbosity", true,
    [&] (std::string const &opt) -> bool {
      int vbhold;
      if(GiBUUUtils::str2int(vbhold,opt.c_str()) == GiBUUUtils::STRINT_SUCCESS){
        std::cout << "\t--GiBUUToStdHepOpts::Verbosity: " << vbhold << std::endl;
        GiBUUToStdHepOpts::Verbosity = vbhold;
        return true;
      }
      return false;
    }, false,
    [&](){GiBUUToStdHepOpts::Verbosity = 0;}, "<0-4>{default==0}");

  CLIUtils::OptSpec.emplace_back("-n", "--nevs", true,
    [&] (std::string const &opt) -> bool {
      int vbhold;
      if(GiBUUUtils::str2int(vbhold,opt.c_str()) == GiBUUUtils::STRINT_SUCCESS){
        std::cout << "\t--Processing " << vbhold << " events." << std::endl;
        GiBUUToStdHepOpts::MaxEntries = vbhold;
        return true;
      }
      return false;
    }, false,
    [&](){GiBUUToStdHepOpts::MaxEntries = -1;},
      "<Num Entries [<-1>: means all]> [default==-1]");

  CLIUtils::OptSpec.emplace_back("-b", "--have-bosonic-info", false,
    [&] (std::string const &opt) -> bool {
      GiBUUToStdHepOpts::HaveStruckNucleonInfo = true;
      return true;
    }, false,
    [&](){GiBUUToStdHepOpts::HaveStruckNucleonInfo = false;},
      "Have struck nucleon information in GiBUU output.");
}

int main(int argc, char const *argv[]){

  SetOpts();
  CLIUtils::AddArguments(argc,argv);
  if(!CLIUtils::GetOpts()){
    CLIUtils::SayRunLike();
    return 1;
  }

  return GiBUUToStdHep();
}
