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

namespace GiBUUUtils {

long MakeNuclearPDG(int Z, int A);

enum STR2INT_ERROR { STRINT_SUCCESS,
                     STRINT_OVERFLOW,
                     STRINT_UNDERFLOW,
                     STRINT_INCONVERTIBLE };

///Converts a string to a long, checking for errors.
///See STR2INT_ERROR for error codes.
STR2INT_ERROR str2int (long &i, char const *s, int base=10);

///Converts a string to a int, checking for errors.
///See STR2INT_ERROR for error codes.
STR2INT_ERROR str2int (int &i, char const *s, int base=10);

std::string int2str(int i);

template<typename T>
void ClearPointer(T * &arr, size_t N){
  for(size_t i = 0; i < N; ++i){
    arr[i] = 0;
  }
}

template<typename T, size_t N, size_t M>
void ClearArray2D(T (&arr)[N][M]){
  for(size_t i = 0; i < N; ++i){
    for(size_t j = 0; j < M; ++j){
      arr[i][j] = 0;
    }
  }
}

int GiBUUToPDG(int GiBUUCode, double GiBUUCharge);
int GiBUU2NeutReacCode(int GiBUUCode, int PDG);

}

namespace CLIUtils {

struct Option {
  std::string ShortName;
  std::string LongName;
  std::string ValueIdent;
  bool HasVal;
  bool Required;
  bool Used;
  std::function<bool(std::string const &opt)> CallBack;
  std::function<void()> Default;

  Option();
  Option(std::string shortname,std::string longname, bool hasval,
    std::function<bool(std::string const &opt)> callback,
    bool required=false,
    std::function<void()> def=[](){},
    std::string valString="Value");

  bool IsOpt(std::string const &optname) const;

  friend std::ostream& operator<<(std::ostream& os, Option const &opt);
};

void AddArguments(int argc, char const * argv[]);

bool GetOpts();

std::string GetArg(size_t i);
size_t GetNArg();

extern std::vector<Option> OptSpec;
//I feel that this should work, but it doesn't seem to.
//You can just do OptSpec.emplace_back manually.
template<class... arguments>
void AddToOptSpec(arguments&&... args){
    OptSpec.emplace_back(&args...);
}

void SayRunLike();

}

#endif
