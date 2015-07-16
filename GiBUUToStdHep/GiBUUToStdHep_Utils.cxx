#include "GiBUUToStdHep_Utils.hxx"

namespace GiBUUUtils {

long MakeNuclearPDG(int Z, int A){
  // 100%03d%03d0 % Z, A
  return 1000000000L + Z*10000L + A*10L;
}

///Converts a string to a long, checking for errors.
///See STR2INT_ERROR for error codes.
STR2INT_ERROR str2int (long &i, char const *s, int base) {
  char *end;
  long  l;
  (void)errno;
  l = strtol(s, &end, base);
  if ((errno == ERANGE && l == LONG_MAX) || l > LONG_MAX) {
      return STRINT_OVERFLOW;
  }
  if ((errno == ERANGE && l == LONG_MIN) || l < LONG_MIN) {
      return STRINT_UNDERFLOW;
  }
  if (*s == '\0' || *end != '\0') {
      return STRINT_INCONVERTIBLE;
  }
  i = l;
  return STRINT_SUCCESS;
}

///Converts a string to a int, checking for errors.
///See STR2INT_ERROR for error codes.
STR2INT_ERROR str2int (int &i, char const *s, int base) {
  long holder;
  STR2INT_ERROR retC = str2int(holder,s,base);
  if(retC != STRINT_SUCCESS){
    return retC;
  }
  if(holder > INT_MAX) {
    return STRINT_OVERFLOW;
  } else if (holder < INT_MIN){
    return STRINT_UNDERFLOW;
  }
  i = holder;
  return retC;
}

std::string int2str(int i){
  std::stringstream ss("");
  ss << i;
  return ss.str();
}

int GiBUUToPDG(int GiBUUCode, double GiBUUCharge){
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
    default:{
      return 0;
    }
  }
}

int GiBUU2NeutReacCode(int GiBUUCode){
  //1=QE, 2-31=res ID, 32,33=1pi, 34=DIS, 35,36=2p2h, 37=2pi
  //From https://gibuu.hepforge.org/trac/wiki/LesHouches
  if(GiBUUCode==1) { return 1; } // QES

  //How best to translate this to a specific NEUT code.
  if(GiBUUCode >= 2 && GiBUUCode <= 31){ return 11; } //CCResonance
  if(GiBUUCode == 34){ return 26; } //DIS
  if((GiBUUCode == 35) || (GiBUUCode == 36)){ return 2; } // MEC/2p-2h
  if(GiBUUCode == 37){ return 21; } //CCCoh?
  return 0;
}

}

namespace CLIUtils {
std::vector<Option> OptSpec;
std::vector<std::string> Args;

  Option::Option(){
    ShortName = "";
    LongName = "";
    HasVal = false;
    Required = false;
    Used = false;
    CallBack = [](std::string const &) -> bool { return false; };
    Default = [](){};
  }

  Option::Option(std::string shortname,std::string longname, bool hasval,
    std::function<bool(std::string const &opt)> callback,
    bool required,
    std::function<void()> def,
    std::string valString){
    ShortName = shortname;
    LongName = longname;
    HasVal = hasval;
    CallBack = callback;
    Required = required;
    Default = def;
    ValueIdent=valString;
    Used=false;
  }

  bool Option::IsOpt(std::string const &optname) const{
    return ((optname == ShortName) || (optname == LongName));
  }

  std::ostream& operator<<(std::ostream& os, Option const &opt){
    return os << "(" << opt.ShortName << "|" << opt.LongName << ")"
      << "  " << opt.ValueIdent
      << (opt.Required?" [Required]":"");
  }



void AddArguments(int argc, char const * argv[]){
  for(int i = 0; i < argc; ++i){
    Args.emplace_back(argv[i]);
  }
}

bool GetOpts(){
  bool skipNextArg = false;
  for(auto arg_it = std::next(Args.cbegin());
    arg_it < Args.cend(); std::advance(arg_it,1)){

    if(skipNextArg){ skipNextArg = false; continue; } //Allows args with values

    std::string const & arg = *arg_it;

    bool found = false;
    for(auto &opt : OptSpec){
      if(!opt.IsOpt(arg)) { continue; }
      found = true;
      if(opt.HasVal){
        skipNextArg = true;
        auto next_it = std::next(arg_it);
        if(next_it == Args.cend()){
          std::cout << "Didn't find expected value for Option " << opt.LongName
            << std::endl;
          return false;
        }

        std::string const & val = *next_it;

        if(!opt.CallBack(val)){
          std::cout << "Error Option: " << opt.LongName
            << ": doesn't understand value: \"" << val << "\"" << std::endl;
          return false;
        }

      } else {
        if(!opt.CallBack("")){ //C++11 lambdas cant have default arguments
          std::cout << "Invalid option: " << opt.LongName << std::endl;
          return false;
        }
      }
      opt.Used = true;
    }
    if(!found){
      std::cout << "Unknown Option: " << arg << std::endl;
      return false;
    }
  }
  bool AllGood = true;
  for(auto const &opt : OptSpec){
    if(opt.Required && !opt.Used){
      std::cout << "Didn't find and option for " << opt.LongName << std::endl;
      AllGood = false;
    }
    if(!opt.Used){
      opt.Default();
    }
  }
  return AllGood;
}

std::string GetArg(size_t i){ return Args[i]; }
size_t GetNArg(){ return Args.size(); }

void SayRunLike(){
  std::cout << "Run like:\n " << Args.front() << std::flush;
  for(auto const & opt: OptSpec){
    if(opt.Required){
      std::cout << " " << opt.ShortName
        << " " << (opt.HasVal?opt.ValueIdent:std::string("")) << std::flush;
    }
  }
  for(auto const & opt: OptSpec){
    if(!opt.Required){
      std::cout << " [" << opt.ShortName
        << " " << opt.ValueIdent << "]" << std::flush;
    }
  }
  std::cout << "\n\n-----------------------------------\n" << std::endl;
  for(auto const & opt: OptSpec){
    std::cout << "[Arg]: " << opt << std::endl;
  }
}

}
