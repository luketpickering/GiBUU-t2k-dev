#ifndef SEEN_GIBUUToStdHep_UTILS_HXX
#define SEEN_GIBUUToStdHep_UTILS_HXX

#include <cstdlib>
#include <cerrno>
#include <climits>

#include <sstream>
#include <string>
#include <functional>

#include <vector>
#include <iostream>

///Utilities which may be helpful for processing GiBUU specific output.
namespace GiBUUUtils {

///Returns the long-from PDG code for a nuclei with a given Z and A.
long MakeNuclearPDG(int Z, int A);

///Return codes for GiBUUUtils::str2int
enum STR2INT_ERROR { STRINT_SUCCESS,
                     STRINT_OVERFLOW,
                     STRINT_UNDERFLOW,
                     STRINT_INCONVERTIBLE };

///Converts a string to a long, checking for errors.
STR2INT_ERROR str2int (long &i, char const *s, int base=10);

///Converts a string to a int, checking for errors.
STR2INT_ERROR str2int (int &i, char const *s, int base=10);

///Converts an int to a std::string through std::stringstream.
std::string int2str(int i);

///Sets the first N values of a numeric array-like pointer to 0.
template<typename T>
void ClearPointer(T * &arr, size_t N){
  for(size_t i = 0; i < N; ++i){
    arr[i] = 0;
  }
}

///Clears a 2D C++ fixed size array.
template<typename T, size_t N, size_t M>
void ClearArray2D(T (&arr)[N][M]){
  for(size_t i = 0; i < N; ++i){
    for(size_t j = 0; j < M; ++j){
      arr[i][j] = 0;
    }
  }
}

///\brief Converts a GiBUU particle code, with associated particle EM charge
///information to a PDG code.
///
/// From https://gibuu.hepforge.org/trac/wiki/ParticleIDs
///\note Returns 0 when encountering an unknown particle.
///Current codes converted:
/// - GiBUU : PDG
/// - 1 : p=2212, n=2112
/// - 101 : pi+=211, pi0=111, pi-=-211
/// - 901 : 11
/// - 902 : 13
/// - 911 : 12
/// - 912 : 14
/// - 999 : 22
/// - 32 : 3122
/// - 33 : 3222, 3212, 3112
/// - 110 : K+=321, K0=311
int GiBUUToPDG(int GiBUUCode, double GiBUUCharge);
///\brief Converts a GiBUU interaction code to the corresponding NEUT code
///where possible.
///
///Sometimes the NEUT code is dependent on the particle produced in the intial
///interaction.
///\note Current codes converted:
/// - 1 = QE
/// - 2-31 = res (specific number represents GiBUU particle id for resonance)
/// - 32,33 = 1pi
/// - 34 = DIS
/// - 35,36 = 2p2h
/// - 37 = 2pi
///
///From https://gibuu.hepforge.org/trac/wiki/LesHouches
int GiBUU2NeutReacCode(int GiBUUCode, int PDG);

}

///Contains types and functions for adding CLI options.
namespace CLIUtils {

///\brief Used to describe a CLI option.
struct Option {
  ///Short name for the option.
  ///
  ///For example `-c'.
  std::string ShortName;
  ///Longer, alternative name for the option.
  ///
  ///For example `-config_file'
  std::string LongName;
  ///
  std::string ValueIdent;
  ///Whether an option takes a value or its presence denotes a switch.
  bool HasVal;
  ///Whether an option is required.
  ///
  ///\note In this case it must take a value.
  bool Required;
  ///Whether this option has been used.
  bool Used;
  ///The function to call when this option is encountered.
  ///
  ///The value, if present is pass in to the callback.
  ///\note As default arguments are not available for annonymous functions this
  /// version of the callback is used for options which do not take a value.
  /// In this case opt is the empty string.
  std::function<bool(std::string const &opt)> CallBack;
  ///The function to call if this option has not been 'Used'.
  std::function<void()> Default;

  ///Default constructor; will not produce a useful Option.
  Option();
  ///Constructor to make a useable Option.
  Option(std::string shortname,std::string longname, bool hasval,
    std::function<bool(std::string const &opt)> callback,
    bool required=false,
    std::function<void()> def=[](){},
    std::string valString="Value");

  ///Checks if the given string corresponds to this option.
  bool IsOpt(std::string const &optname) const;

  friend std::ostream& operator<<(std::ostream& os, Option const &opt);
};

///Interface to adding CLI args from the entry point declaration to this module.
void AddArguments(int argc, char const * argv[]);

///\brief Processes arguments added via AddArguments through options in
///CLIUtils::OptSpec.
bool GetOpts();

///Get the value of the i'th argument.
std::string GetArg(size_t i);
///Get the number of arguments.
size_t GetNArg();

///The vector of added CLI options.
///
///Use
///   CLIUtils::OptSpec.push_back(Option(...));
///to add options.
extern std::vector<Option> OptSpec;

///Writes descriptions for the Options from CLIUtils::OptSpec to std::cout.
void SayRunLike();

}

#endif
