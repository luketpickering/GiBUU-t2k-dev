#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include "TFile.h"
#include "TTree.h"

#include "GiBUUToStdHep_Utils.hxx"

#include "GiRooTracker.hxx"

namespace {

std::string InpFName;

int nuType;
int TargetA;
int TargetZ;
long MaxEntries;

int Verbosity = 0;

}

int GiBUUToStdHep(){
  std::ifstream ifs(InpFName);
  std::string line;
  long ctr = 0;

  TFile* outFile = new TFile("GiBUURooTracker.root","CREATE");
  if(!outFile->IsOpen()){
    std::cout << "Couldn't open output file." << std::endl;
    return 2;
  }

  TTree* rooTrackerTree = new TTree("giRooTracker","GiBUU StdHepVariables");
  GiRooTracker* outRooTracker = new GiRooTracker();
  outRooTracker->AddBranches(rooTrackerTree);

  int LastEvNum = 0;

  bool isHOREv = false;

  while(std::getline(ifs,line)){
    if(!ctr){ctr++;continue;} //skip the table header
    std::istringstream iss(line);

    int Run, EvNum, ID, History, Prodid;
    float Charge,PerWeight,Pos1,Pos2,Pos3,Mom0,Mom1,Mom2,Mom3,Enu;

    iss >> Run >> EvNum >> ID >> Charge >> PerWeight >> Pos1 >>
      Pos2 >> Pos3 >> Mom0 >> Mom1 >> Mom2 >> Mom3 >> History >> Prodid >> Enu;

    if(EvNum != LastEvNum){
      if(LastEvNum){
        if(isHOREv && !outRooTracker->GiBUU2NeutCode){
          std::cout << "[WARN]: Missed a GiBUU reaction code: " << Prodid
            << std::endl;
        }
        rooTrackerTree->Fill();
        outRooTracker->Reset();
        isHOREv = false;
      }
      if(!(EvNum%1000)){ std::cout << "On Ev: " << EvNum << std::endl; }
      if(MaxEntries == EvNum){
        std::cout << "Finishing after " << EvNum << " entries." << std::endl;
        break;
      }

      outRooTracker->EvtNum = EvNum;

      //neutrino
      outRooTracker->StdHepPdg[0] = nuType;
      outRooTracker->StdHepStatus[0] = -1;
      outRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPx] = 0;
      outRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPy] = 0;
      outRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxPz] = Enu;
      outRooTracker->StdHepP4[0][GiRooTracker::kStdHepIdxE] = Enu;

      //target
      outRooTracker->StdHepPdg[1] = GiBUUUtils::MakeNuclearPDG(TargetZ,TargetA);
      outRooTracker->StdHepStatus[1] = -1;
      outRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPx] = 0;
      outRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPy] = 0;
      outRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxPz] = 0;
      outRooTracker->StdHepP4[1][GiRooTracker::kStdHepIdxE] = TargetA;

      outRooTracker->StdHepN = 2;
      LastEvNum = EvNum;
    }

    outRooTracker->StdHepPdg[outRooTracker->StdHepN] =
      GiBUUUtils::GiBUUToPDG(ID,Charge);
    outRooTracker->StdHepStatus[outRooTracker->StdHepN] = 1;
    outRooTracker->StdHepP4[outRooTracker->StdHepN]\
      [GiRooTracker::kStdHepIdxPx] = Mom1;
    outRooTracker->StdHepP4[outRooTracker->StdHepN]\
      [GiRooTracker::kStdHepIdxPy] = Mom2;
    outRooTracker->StdHepP4[outRooTracker->StdHepN]\
      [GiRooTracker::kStdHepIdxPz] = Mom3;
    outRooTracker->StdHepP4[outRooTracker->StdHepN]\
      [GiRooTracker::kStdHepIdxE] = Mom0;

    outRooTracker->GiBUU2NeutCode = GiBUUUtils::GiBUU2NeutReacCode(Prodid,
      outRooTracker->StdHepPdg[outRooTracker->StdHepN]);

    if((Prodid == 32) || (Prodid == 33)){
      // std::cout << "[INFO]: Is HOR Event." << std::endl;
      isHOREv = true;
    }

    if(isHOREv && outRooTracker->GiBUU2NeutCode != 0){
      // std::cout << "[INFO]: Found the correct NEUT Code: "
      //   << outRooTracker->GiBUU2NeutCode << std::endl;
    }

    if(Verbosity>=1 && !outRooTracker->GiBUU2NeutCode){
      // std::cout << "[WARN]: Missed a GiBUU reaction code: " << Prodid
      //   << std::endl;
    }

    outRooTracker->StdHepN++;

    if(Verbosity && (outRooTracker->StdHepPdg[outRooTracker->StdHepN-1]==0)){
      std::cout << "[WARN] PDG == 0\n\t" << line << std::endl;
      std::cout << "Copy: " << Run << " " <<  EvNum<< " " <<  ID << " "
        << Charge << " " << PerWeight << " " << Pos1 << " " <<
        Pos2 << " " << Pos3 << " " << Mom0 << " " << Mom1 << " " << Mom2
        << " " << Mom3 << " " <<  History << " " <<  Prodid << " " <<  Enu
        << std::endl;
    }

    if(Verbosity>2){
      std::cout << "Line: " << line << std::endl;
      std::cout << "Copy: " << Run << " " <<  EvNum<< " " <<  ID << " "
        << Charge << " " << PerWeight << " " << Pos1 << " " <<
        Pos2 << " " << Pos3 << " " << Mom0 << " " << Mom1 << " " << Mom2
        << " " << Mom3 << " " <<  History << " " <<  Prodid << " " <<  Enu
        << std::endl;
    }
    ctr++;
  }
  rooTrackerTree->Fill();
  rooTrackerTree->Write();
  outFile->Write();
  outFile->Close();
  delete outRooTracker;
  ifs.close();
  return 0;
}

void SetOpts(){
  CLIUtils::OptSpec.emplace_back("-h","--help", false,
    [&] (std::string const &opt) -> bool {
      CLIUtils::SayRunLike();
      exit(0);
    });

  CLIUtils::OptSpec.emplace_back("-i", "--input-file", true,
    [&] (std::string const &opt) -> bool {
      std::cout << "\tReading GiBUU file : " << opt << std::endl;
      InpFName = opt;
      return true;
    }, true,[](){},"<File Name>");

  CLIUtils::OptSpec.emplace_back("-u", "--nu-pdg", true,
    [&] (std::string const &opt) -> bool {
      int vbhold;
      if(GiBUUUtils::str2int(vbhold,opt.c_str()) == GiBUUUtils::STRINT_SUCCESS){
        std::cout << "Nu PDG: " << vbhold << std::endl;
        nuType = vbhold;
        return true;
      }
      return false;
    }, true,[](){},"<Neutrino PDG identifier>");

  CLIUtils::OptSpec.emplace_back("-a", "--target-a", true,
    [&] (std::string const &opt) -> bool {
      int vbhold;
      if(GiBUUUtils::str2int(vbhold,opt.c_str()) == GiBUUUtils::STRINT_SUCCESS){
        std::cout << "Target A: " << vbhold << std::endl;
        TargetA = vbhold;
        return true;
      }
      return false;
    }, true,[](){},"<Target A>");

  CLIUtils::OptSpec.emplace_back("-z", "--target-z", true,
    [&] (std::string const &opt) -> bool {
      int vbhold;
      if(GiBUUUtils::str2int(vbhold,opt.c_str()) == GiBUUUtils::STRINT_SUCCESS){
        std::cout << "Target Z: " << vbhold << std::endl;
        TargetZ = vbhold;
        return true;
      }
      return false;
    }, true,[](){},"<Target Z>");

  CLIUtils::OptSpec.emplace_back("-v", "--verbosity", true,
    [&] (std::string const &opt) -> bool {
      int vbhold;
      if(GiBUUUtils::str2int(vbhold,opt.c_str()) == GiBUUUtils::STRINT_SUCCESS){
        std::cout << "Verbosity: " << vbhold << std::endl;
        Verbosity = vbhold;
        return true;
      }
      return false;
    }, false,
    [&](){Verbosity = 0;}, "<0-4>{default==0}");

  CLIUtils::OptSpec.emplace_back("-n", "--nevs", true,
    [&] (std::string const &opt) -> bool {
      int vbhold;
      if(GiBUUUtils::str2int(vbhold,opt.c_str()) == GiBUUUtils::STRINT_SUCCESS){
        std::cout << "Number of events: " << vbhold << std::endl;
        MaxEntries = vbhold;
        return true;
      }
      return false;
    }, false,
    [&](){MaxEntries = -1;}, "<Num Entries [<-1>: means all]> [default==-1]");
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
