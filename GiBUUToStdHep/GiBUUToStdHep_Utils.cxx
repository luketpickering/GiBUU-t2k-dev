#include "GiBUUToStdHep_Utils.hxx"

namespace GiBUUUtils {

int GiBUUToPDG(int GiBUUCode, int GiBUUCharge){
  //https://gibuu.hepforge.org/trac/wiki/ParticleIDs
  switch(GiBUUCode){
    case 1:{
      return (GiBUUCharge>0)?2212:2112;
    }
    case 101:{
      if(GiBUUCharge){ return(GiBUUCharge>0)?211:-211; }
      return 111;
    }
    case 901:{
      return (GiBUUCharge<0)?11:-11;
    }
    case 902:{
      return (GiBUUCharge<0)?13:-13;
    }
    case 911:{
      return 12;
    }
    case 912:{
      return 14;
    }
    case 999:{
      return 22;
    }
    case 32:{
      std::cout << "[INFO]: Final State Lambda(" << GiBUUCharge << ")."
        << std::endl;
      return 3122;
    }
    case 33:{
      std::cout << "[INFO]: Final State Eta(" << GiBUUCharge << ")."
        << std::endl;
      return 3222;
    }
    case 110:{
      if(GiBUUCharge){return (GiBUUCharge>0)?321:-321;}
      return 311;
    }
    default:{
      std::cout << "[WARN]: Missed a GiBUU PDG Code: " << GiBUUCode << std::endl;
      return 0;
    }
  }
}

int GiBUU2NeutReacCode(int GiBUUCode, int PDG){
  //1=QE, 2-31=res ID, 32,33=1pi, 34=DIS, 35,36=2p2h, 37=2pi
  //From https://gibuu.hepforge.org/trac/wiki/LesHouches
  if(GiBUUCode==1) { return 1; } // QES

  //How best to translate this to a specific NEUT code.
  if(GiBUUCode >= 2 && GiBUUCode <= 31){ return 11; } //CCResonance
  if(GiBUUCode == 34){ return 26; } //DIS
  if((GiBUUCode == 35) || (GiBUUCode == 36)){ return 2; } // MEC/2p-2h
  if(GiBUUCode == 37){ return 21; }
  if((GiBUUCode == 32) || (GiBUUCode == 33)){
    if(PDG == 3222){ return 22; }
    if((PDG == 3122) || (PDG == 321) || (PDG == -321) || (PDG == 311)){ return 23; }
    std::cout << "[WARN]: Couldn't find RES mode for GiBUU Code: "
      << GiBUUCode << ", PDG: " << PDG << std::endl;
    return 24;
  }//CCCoh?
  std::cout << "[WARN]: Missed a GiBUU Reac Code: " << GiBUUCode << ", PDG: "
    << PDG << std::endl;
  return 0;
}

}
