#include <iomanip>
#include <algorithm>

#include "GiBUUToStdHep_Utils.hxx"

namespace GiBUUUtils {

int GiBUUToPDG(int GiBUUCode, int GiBUUCharge){
  //https://gibuu.hepforge.org/trac/wiki/ParticleIDs
  switch(GiBUUCode){
    case 1:{
      return (GiBUUCharge>0)?2212:2112;
    }
    case 2:{
      switch(GiBUUCharge){
        case 2:{
          return 2224;
        }
        case 1:{
          return 2214;
        }
        case 0:{
          return 2114;
        }
        case -1:{
          return 1114;
        }
        default:{
          std::cout << "[WARN]: Delta resonance had an odd charge: "
            << GiBUUCharge << std::endl;
          return 2114;
        }
      }
    }
    case 3:{
      return (GiBUUCharge>0)?202212:202112;
    }
    case 4:{
      return (GiBUUCharge>0)?102212:102112;
    }
    case 5:{
      return (GiBUUCharge>0)?122212:122112;
    }
    case 7:{
      return (GiBUUCharge>0)?102214:102114;
    }
    case 10:{
      return (GiBUUCharge>0)?102216:102116;
    }
    case 19:{
      switch(GiBUUCharge){
        case 2:{
          return 112222;
        }
        case 1:{
          return 112212;
        }
        case 0:{
          return 112112;
        }
        case -1:{
          return 111112;
        }
        default:{
          std::cout << "[WARN]: S31(1620) resonance had an odd charge: "
            << GiBUUCharge << std::endl;
          return 111112;
        }
      }
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
    case 101:{
      if(GiBUUCharge){ return(GiBUUCharge>0)?211:-211; }
      return 111;
    }
    case 104:{
      return 9000221;
    }
    case 110:{
      if(GiBUUCharge){return (GiBUUCharge>0)?321:-321;}
      return 311;
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
    default:{
      if(GiBUUCode){
        std::cout << "[WARN]: Missed a GiBUU PDG Code: " << GiBUUCode
          << std::endl;
      }
      return 0;
    }
  }
}

std::tuple<Int_t,Int_t,Int_t> DecomposeGiBUUHistory(Long_t HistCode){

  if(HistCode > -1){
    Int_t Generation = abs(HistCode)/1E6;
    Int_t P2 = ( HistCode - (1E6*Generation) )/1E3;
    Int_t P1 =  ( HistCode - (1E6*Generation) ) - (1E3*P2);

    Int_t PFather = std::min(P1,P2);
    Int_t PMother = std::max(P1,P2);

    return std::make_tuple(Generation,PMother,PFather);
  } else {
    HistCode = -1*HistCode;
    Int_t Generation = abs(HistCode)/1E6;
    Int_t ThreeBodyCode = ( HistCode - (1E6*Generation) );

    return std::make_tuple(Generation, -1, ThreeBodyCode);
  }
}

std::string WriteGiBUUHistory(Long_t HistCode){
  std::stringstream ss("");
  if(!HistCode){
    ss << "[Elementary interaction]";
  } else {
    auto const & hDec = GiBUUUtils::DecomposeGiBUUHistory(HistCode);

    if(std::get<1>(hDec) != -1){
      ss << "[Gen: " << std::get<0>(hDec)
         << ", Mother: " << GiBUUToPDG(std::get<1>(hDec));
      if(std::get<2>(hDec)){
        ss << ", Father: " << GiBUUToPDG(std::get<2>(hDec));
      }
      ss << "]";
    } else { // 3 body
      ss << "[Gen: " << std::get<0>(hDec) << ", 3Body Process: ";
      switch(std::get<2>(hDec)){
        case 1:{
          ss << "(N N N)]";
          break;
        }
        case 2:{
          ss << "(N N Delta)]";
          break;
        }
        case 3:{
          ss << "(N N Pion)]";
          break;
        }
        case 4:{
          ss << "Unknown]";
          break;
        }
        default:{
          ss << "Unknown]";
        }
      }
    }
  }

  ss << " -- " << "(" << HistCode << ")";
  return ss.str();
}

void PrintGiBUUStdHepArray(
  Int_t GiBUUCode,
  Int_t const * const StdHepPDGArray,
  Long_t const * const HistoryArray,
  Int_t StdHepN,
  std::string const &indent=""){

  std::cout << indent << "GiBUUCode: " << GiBUUCode << std::endl;
  for(Int_t i = 0; i < StdHepN; ++i){
    std::cout << indent << "\t(" << std::setw(4) << StdHepPDGArray[i] << "): "
      << WriteGiBUUHistory(HistoryArray[i]) << std::endl;
  }
}

//Returns vect of: Particle PDG, Parent1, Parent2
std::vector< std::tuple< Int_t, Int_t, Int_t> > GetGenNParticles(
  Int_t Gen,
  Int_t const * const StdHepPDGArray,
  Long_t const * const HistoryArray,
  Int_t StdHepN){

  std::vector< std::tuple<Int_t, Int_t, Int_t> > ret;

  for(Int_t i = 0; i < StdHepN; ++i){
    auto const & dHist = DecomposeGiBUUHistory(HistoryArray[i]);
    if(std::get<0>(dHist)==Gen){
      ret.push_back(std::make_tuple(StdHepPDGArray[i],std::get<1>(dHist),
        std::get<2>(dHist)));
    }
  }
  return ret;
}

std::vector<Int_t> GetTwoBodyFSParticles(
  Int_t const * const StdHepPDGArray,
  Long_t const * const HistoryArray,
  Int_t StdHepN){

  std::vector<Int_t> ret;

  for(Int_t i = 0; i < StdHepN; ++i){
    if(std::get<2>(DecomposeGiBUUHistory(HistoryArray[i]))!=0){
      ret.push_back(StdHepPDGArray[i]);
    }
  }
  return ret;
}

std::vector<Int_t> GetTwoBodyFSNucleons(
  Int_t const * const StdHepPDGArray,
  Long_t const * const HistoryArray,
  Int_t StdHepN){

  std::vector<Int_t> ret;

  for(Int_t i = 0; i < StdHepN; ++i){
    if( (std::get<2>(DecomposeGiBUUHistory(HistoryArray[i]))!=0) &&
       ( (StdHepPDGArray[i] == 2122) ||
         (StdHepPDGArray[i] == 2112) ) ){
      ret.push_back(StdHepPDGArray[i]);
    }
  }
  return ret;
}

//Returns Generation, Pion PDG, Decay Parent
std::vector< std::tuple<Int_t, Int_t,Int_t> > GetFSDecayPions(
  Int_t const * const StdHepPDGArray,
  Long_t const * const HistoryArray,
  Int_t StdHepN){

  std::vector< std::tuple<Int_t, Int_t,Int_t> > ret;

  for(Int_t i = 0; i < StdHepN; ++i){
    auto const & dHist = DecomposeGiBUUHistory(HistoryArray[i]);
    if( (std::get<1>(dHist)!=-1) &&
        (std::get<2>(dHist)==0) &&
        ( (StdHepPDGArray[i] == 211)  ||
          (StdHepPDGArray[i] == -211) ||
          (StdHepPDGArray[i] == 111) ) ){
      ret.push_back(std::make_tuple(std::get<0>(dHist),
                                    StdHepPDGArray[i],
                                    std::get<1>(dHist)) );
    }
  }
  std::sort(ret.begin(), ret.end());
  return ret;
}

//Returns Generation, PDG
std::vector< std::tuple<Int_t, Int_t> > GetDeltaDecayNucleons(
  Int_t const * const StdHepPDGArray,
  Long_t const * const HistoryArray,
  Int_t StdHepN){

  std::vector< std::tuple<Int_t, Int_t> > ret;

  for(Int_t i = 0; i < StdHepN; ++i){
    auto const & dHist = DecomposeGiBUUHistory(HistoryArray[i]);
    if( (std::get<1>(dHist)==2) &&
        (std::get<2>(dHist)==0) &&
        ( (StdHepPDGArray[i] == 2112)  ||
          (StdHepPDGArray[i] == 2212) ) ){
      ret.push_back(std::make_tuple(std::get<0>(dHist),
                                    StdHepPDGArray[i]) );
    }
  }
  std::sort(ret.begin(), ret.end());
  return ret;
}

std::vector<Int_t> GetFSPions(
  Int_t const * const StdHepPDGArray,
  Int_t StdHepN){

  std::vector<Int_t> ret;

  for(Int_t i = 0; i < StdHepN; ++i){
    if( (StdHepPDGArray[i] == 211) ||
        (StdHepPDGArray[i] == -211)||
        (StdHepPDGArray[i] == 111) ){
      ret.push_back(StdHepPDGArray[i]);
    }
  }
  return ret;
}

std::vector<Int_t> GetInHistoryResonances(
  Int_t const * const StdHepPDGArray,
  Long_t const * const HistoryArray,
  Int_t StdHepN){

  std::vector<Int_t> ret;

  for(Int_t i = 0; i < StdHepN; ++i){

    ret.push_back(StdHepPDGArray[i]);
  }
  return ret;
}

Int_t PionPDGToNeutResMode(Int_t pionPDG, Int_t NucleonPDG, bool &Warn){
  Warn = (!NucleonPDG);
  switch(pionPDG){
    case 211:{
      return (NucleonPDG==2112)?13:11;
    }
    case 111:{
      if(NucleonPDG==2112){
        Warn = true;
      }
      return 12;
    }
    default:{}
  }
  return 0;
}

int GiBUU2NeutReacCode(Int_t GiBUUCode,
  Int_t const * const StdHepPDGArray,
  Long_t const * const HistoryArray,
  Int_t StdHepN,
  bool IsCC,
  Int_t StruckNucleonPosition){

  //1=QE, 2-31=res ID, 32,33=1pi, 34=DIS, 35,36=2p2h, 37=2pi
  //From https://gibuu.hepforge.org/trac/wiki/LesHouches
  if(IsCC){
    switch(GiBUUCode){
      case 1: { return 1; } // QE
      case 2:{

        if(StdHepPDGArray[StruckNucleonPosition]==2212){return 11;}

        //RESONANCE HEURISTICS
        auto const &g1parts = GetGenNParticles(1,StdHepPDGArray,
          HistoryArray,StdHepN);

        Int_t NucleonPDG = 0;
        for(auto const & part : g1parts){
          if( (std::get<1>(part) == 2) && //first gen delta decay child
              ( (std::get<0>(part)==2212) || // a proton
                (std::get<0>(part)==2112) ) ){ // a neutron
            NucleonPDG = std::get<0>(part);
            break;
          }
        }

        //If we find a first generation pion
        for(auto const & part : g1parts){
          bool Warn = false;
          //Check the pions
          Int_t rMode = PionPDGToNeutResMode(std::get<0>(part),
            NucleonPDG, Warn);
          if(rMode) {
            if(Warn){ //If theres something fishy then shout about it
              // std::cout << "[WARN]: Returning potentially dodgey (mode:0) "
              //   << "NEUT Code: " << rMode << " (Found Nucleon: "
              //   << NucleonPDG << ")." << std::endl;
              // std::cout << "        It came from the event: " << std::endl;
              // PrintGiBUUStdHepArray(GiBUUCode,StdHepPDGArray,HistoryArray,
              //   StdHepN,"\t|");
            }
            return rMode;
          }
        }

        //If we're still here then we should check FS pions from decays
        auto const &FSDP =
          GetFSDecayPions(StdHepPDGArray, HistoryArray, StdHepN);
        for(auto const & decayPi : FSDP){ //Try first to make sure it is from
          if(std::get<2>(decayPi) != 2){  //a Delta
            // std::cout << "[WARN]: Had a GiBUU mode 2 which resulted in decay "
            //   << "pions. Their decay parent, " << std::get<2>(decayPi)
            //   << ", was not a Delta though."
            //   << std::endl;
              if(std::get<2>(decayPi) == 102 || std::get<2>(decayPi) == 104){
                return 21;
              }
            continue;
          }
          //Try and find a nucleon from the same generation.
          auto const &gxparts = GetGenNParticles(std::get<0>(decayPi),
            StdHepPDGArray, HistoryArray,StdHepN);
          Int_t SameGenNucleon = 0;
          for(auto const & part : gxparts){
            if( (std::get<1>(part) == 2) && //first gen delta decay child
                ( (std::get<0>(part)==2212) || // a proton
                  (std::get<0>(part)==2112) ) ){ // a neutron
              SameGenNucleon = std::get<0>(part);
              break;
            }
          }
          bool Warn = false;
          Int_t rMode = PionPDGToNeutResMode(std::get<1>(decayPi),
            SameGenNucleon, Warn);
          if(rMode) {
            if(Warn){ //If theres something fishy then shout about it
              // std::cout << "[WARN]: Returning potentially dodgey (mode:1) "
              //   << "NEUT Code: " << rMode << " (Found Same generation Nucleon: "
              //   << SameGenNucleon << ")." << std::endl;
              // std::cout << "        It came from the event: " << std::endl;
              // PrintGiBUUStdHepArray(GiBUUCode,StdHepPDGArray,HistoryArray,
              //   StdHepN,"\t|");
            }
            return rMode;
          }
        }

        for(auto const & decayPi : FSDP){
          //Try and find a nucleon from the same generation and same decay
          //parent PDG.
          auto const &gxparts = GetGenNParticles(std::get<0>(decayPi),
            StdHepPDGArray, HistoryArray,StdHepN);
          Int_t SameGenNucleon = 0;
          for(auto const & part : gxparts){
            if( (std::get<1>(part) == std::get<2>(decayPi)) && //same decay
                                                               //parent as pion
                ( (std::get<0>(part)==2212) || // a proton
                  (std::get<0>(part)==2112) ) ){ // a neutron
              SameGenNucleon = std::get<0>(part);
              break;
            }
          }
          bool Warn = false;
          Int_t rMode = PionPDGToNeutResMode(std::get<1>(decayPi),
            SameGenNucleon, Warn);
          if(rMode) {
            // std::cout << "[WARN]: Returning potentially dodgey (mode:2) "
            //   << "NEUT Code: " << rMode << " (Found Nucleon probably from same "
            //   << "decay: " << SameGenNucleon << ")." << std::endl;
            // std::cout << "        It came from the event: " << std::endl;
            // PrintGiBUUStdHepArray(GiBUUCode,StdHepPDGArray,HistoryArray,
            //   StdHepN,"\t|");
            return rMode;
          }
        }

        if(!NucleonPDG){ // If we havent found one yet.
          //Get the lowest gen one!
          auto const &gxparts = GetDeltaDecayNucleons(StdHepPDGArray,
            HistoryArray, StdHepN);
          NucleonPDG = gxparts.size()?std::get<1>(gxparts.front()):0;
        }

        // std::cout << "\nGiving up on this Delta resonance, returning: "
        //   << ((NucleonPDG==2212)?10:9) << " (Found Nucleon: "
        //   << NucleonPDG << ")." << std::endl;
        // PrintGiBUUStdHepArray(GiBUUCode,StdHepPDGArray,HistoryArray,
        //       StdHepN,"\t+");
        return (NucleonPDG==2212)?10:9;

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
      case 31:{  //CCResonance
        return 15;
      }
      case 32:
      case 33:{
        return 21;
        break;
      }
      case 34:{ return 26; } //DIS
      case 35:
      case 36:{ return 2; } // MEC/2p-2h
      case 37:{ return 21; }
      default: {}
    }
  } else {
    switch(GiBUUCode)
    case 1: { // QE
      if(StruckNucleonPosition >= 0){ //If we have the struck nucleon
        return (StdHepPDGArray[StruckNucleonPosition] == 2212)?51:52;
      }
      auto const &gen0Parts = GetGenNParticles(0,StdHepPDGArray,
        HistoryArray, StdHepN); //Do we have any gen0 particles
      if(gen0Parts.size()){
        return (std::get<0>(gen0Parts.front()) == 2212)?51:52;
      }
      auto const &FSNucs = GetTwoBodyFSNucleons(StdHepPDGArray,
        HistoryArray, StdHepN);
      if(FSNucs.size()){
        return (FSNucs.front() == 2212)?51:52;
      }
    }
  }
  std::cout << "[WARN]: Couldn't determine NEUT equivalent reaction code "
    "for the interaction:" << std::endl;
  PrintGiBUUStdHepArray(GiBUUCode,StdHepPDGArray,HistoryArray,StdHepN);
  return 0;
}

}
