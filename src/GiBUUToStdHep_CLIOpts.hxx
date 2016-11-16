#ifndef GiBUUToStdHepCLIOpts_HXX_SEEN
#define GiBUUToStdHepCLIOpts_HXX_SEEN

#include <string>
#include <vector>
#include <map>

/// Options relevant to the GiBUUToStdHep.exe executable.
namespace GiBUUToStdHepOpts {

/// The location of the input files which was produced by GiBUU.
extern std::vector<std::string> InpFNames;
/// The name of the output root file to write.
extern std::string OutFName;

/// Whether the GiBUU output contains struck nucleon information.
///
///\note Assumed true.
extern bool HaveStruckNucleonInfo;

///\brief The neutrino species PDG.
///
///\note Set by
///  `GiBUUToStdHep.exe ... -u xx ...'
/// Required.
extern std::vector<int> nuTypes;
///\brief The target nuclei nucleon number, A, for the next input file(s).
///
///\note Set by
///  `GiBUUToStdHep.exe ... -a xx ...'
/// Required.
extern std::vector<int> TargetAs;
///\brief The target nuclei proton number, Z, for the next input file(s).
///
///\note Set by
///  `GiBUUToStdHep.exe ... -z xx ...'
/// Required.
extern std::vector<int> TargetZs;
///\brief Whether input events for the next file(s) are simulated NC
/// interactions.
///
///\note Set by
///  `GiBUUToStdHep.exe ... -N ...'
extern std::vector<bool> CCFiles;
///\brief An extra weight to apply to the events of the next file(s).
///
/// Useful for building composite targets.
extern std::vector<double> FileExtraWeights;
///\brief An extra weight to applied which averages over the number of files
/// added.
extern std::vector<double> NFilesAddedWeights;
///\brief An extra weight to apply to all parsed events.
///
/// Useful for building composite targets.
extern double OverallWeight;
///\brief An extra weight to apply to all event induced by neutrinos of a given
/// species.
///
/// This can be used to correctly combine multiple neutrino species into a
/// single measurement. e.g. mu- mu+ measurement in an FHC flux with a
/// wrong-sign background component.
extern std::map<int, double> CompositeFluxWeight;

///\brief Whether the GiBUU output contains the neutrino-induced hadronic
/// particles charge.
///
///\note Assumed true.
extern bool HaveProdChargeInfo;

extern std::vector<std::pair<std::string, std::string> > FluxFilesToAdd;

///\brief Whether to exit on suspicious input file contents.
extern bool StrictMode;
}

namespace GiBUUToStdHep_CLIOpts {
  bool HandleArgs(int argc, char const *argv[]);
  void SayRunLike(char const *argv[]);
}

#endif
