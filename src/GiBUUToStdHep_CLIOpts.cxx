#include <fstream>

// Unix
#include <dirent.h>
#include <unistd.h>

#include "TRegexp.h"
#include "TString.h"

#include "LUtils/Debugging.hxx"
#include "LUtils/Utils.hxx"

#include "GiBUUToStdHep_CLIOpts.hxx"

/// Options relevant to the GiBUUToStdHep.exe executable.
namespace GiBUUToStdHepOpts {
  std::vector<std::string> InpFNames;
  std::string OutFName;
  bool HaveStruckNucleonInfo;
  std::vector<int> nuTypes;
  std::vector<int> TargetAs;
  std::vector<int> TargetZs;
  std::vector<bool> CCFiles;
  std::vector<double> FileExtraWeights;
  std::vector<double> NFilesAddedWeights;
  double OverallWeight = 1;
  std::map<int, double> CompositeFluxWeight;
  bool HaveProdChargeInfo = false;
  std::vector<std::pair<std::string, std::string> > FluxFilesToAdd;
  bool StrictMode = true;
}

std::vector<std::string> CLIFileArgs;

bool AddFiles(std::string const &OptVal, bool IsCC, int NuType, int TargetA,
              int TargetZ, double FileExtraWeight) {
  size_t AsteriskPos = OptVal.find_last_of('*');
  if (AsteriskPos == std::string::npos) {
    UDBLog("\t--Adding file: " << OptVal);
    GiBUUToStdHepOpts::InpFNames.push_back(OptVal);
    GiBUUToStdHepOpts::nuTypes.push_back(NuType);
    GiBUUToStdHepOpts::TargetAs.push_back(TargetA);
    GiBUUToStdHepOpts::TargetZs.push_back(TargetZ);
    GiBUUToStdHepOpts::CCFiles.push_back(IsCC);
    GiBUUToStdHepOpts::FileExtraWeights.push_back(FileExtraWeight);
    GiBUUToStdHepOpts::NFilesAddedWeights.push_back(1);
    return true;
  }

  DIR *dir;
  struct dirent *ent;
  size_t lastFSlash = OptVal.find_last_of('/');
  std::string matchPat = OptVal.substr(lastFSlash + 1);
  std::string dirpath = "";
  if (lastFSlash == std::string::npos) {
    char *cwd = new char[1000];
    getcwd(cwd, sizeof(char) * 1000);
    UDBLog("\t--Looking in current directory (" << cwd << ") for matching (\""
                                                << matchPat << "\") files.");
    dirpath = "./";
    delete cwd;
  } else {
    if (AsteriskPos < lastFSlash) {
      UDBError(
          "Currently cannot handle a wildcard in the "
          "directory structure. Please put input files in the same "
          "directory or use separate -f arguments (N.B. you will have "
          "to manually weight each separate file by 1/NFiles). "
          "Expected -f \"../some/rel/path/*.dat\"");
      return false;
    }
    dirpath = OptVal.substr(0, lastFSlash + 1);
    UDBLog("\t--Looking in directory (" << dirpath << ") for matching files.");
  }
  dir = opendir(dirpath.c_str());

  if (dir != NULL) {
    TRegexp matchExp(matchPat.c_str(), true);
    /* print all the files and directories within directory */
    Ssiz_t len = 0;
    size_t NFilesAdded = 0;
    while ((ent = readdir(dir)) != NULL) {
      if (matchExp.Index(TString(ent->d_name), &len) != Ssiz_t(-1)) {
        UDBLog("\t\t\tAdding matching file: "
               << (dirpath + ent->d_name) << "(nu: " << NuType
               << ", A: " << TargetA << ", Z: " << TargetZ
               << ", TW: " << FileExtraWeight << ", IsCC: " << IsCC << ")");
        GiBUUToStdHepOpts::InpFNames.push_back((dirpath + ent->d_name));
        GiBUUToStdHepOpts::nuTypes.push_back(NuType);
        GiBUUToStdHepOpts::TargetAs.push_back(TargetA);
        GiBUUToStdHepOpts::TargetZs.push_back(TargetZ);
        GiBUUToStdHepOpts::CCFiles.push_back(IsCC);
        NFilesAdded++;
      }
    }
    closedir(dir);

    for (size_t file_it = 0; file_it < NFilesAdded; ++file_it) {
      GiBUUToStdHepOpts::FileExtraWeights.push_back(FileExtraWeight);
      GiBUUToStdHepOpts::NFilesAddedWeights.push_back(1.0 /
                                                      double(NFilesAdded));
    }
    if (NFilesAdded) {
      UDBLog("Added " << NFilesAdded << " overall weight: "
                      << (FileExtraWeight / double(NFilesAdded)));
    } else {
      UDBLog("Failed to find any matching files.");
    }
  } else {
    /* could not open directory */
    perror("");
    return false;
  }
  return true;
}

bool Handle_CompositeExample(std::string const &opt) {
  std::cout << "[RUNLIKE]: To combine C runs and H runs for a correctly "
               "weighted (per nucleon) CH2 target with CC and [NC] "
               "events and "
               "neutrino and {antineutrino} beam components"
               ":\n[RUNLIKE]:\tGiBUUToStdHep.exe -u 14 -a 12 -z 6 -W "
               "12 -f \"FinalEvents_nu_C_CC_*.dat\" [-N -W 12 -f "
               "\"FinalEvents_nu_C_NC_*.dat\"] -a 1 -z 1 -W 2 -f "
               "\"FinalEvents_nu_H_CC_*.dat\" [-N -W 2 -f "
               "\"FinalEvents_nu_H_NC_*.dat\"] {-u -14 -a 12 -z 6 -W "
               "12 -f \"FinalEvents_nub_C_CC_*.dat\" [-N -W 12 -f "
               "\"FinalEvents_nub_C_NC_*.dat\"] -a 1 -z 1 -W 2 -f "
               "\"FinalEvents_nub_H_CC_*.dat\" [-N -W 2 -f "
               "\"FinalEvents_nub_H_NC_*.dat\"]} -R i14\n[RUNLIKE]: This "
               "will weight carbon events up by 12 and hydrogen events up by "
               "2, before weighting all events down by 14 to rescale the "
               "produced weights to cross section per nucleon."
            << std::endl;
  exit(0);
}

bool Handle_FEFile(std::string const &opt) {
  bool IsCC = true;
  // If specified as NC take that, otherwise, assume CC
  if (GiBUUToStdHepOpts::CCFiles.size() > GiBUUToStdHepOpts::InpFNames.size()) {
    IsCC = GiBUUToStdHepOpts::CCFiles.back();
    GiBUUToStdHepOpts::CCFiles.pop_back();
  }

  if ((!GiBUUToStdHepOpts::nuTypes.size()) ||
      (!GiBUUToStdHepOpts::TargetAs.size()) ||
      (!GiBUUToStdHepOpts::TargetZs.size())) {
    UDBError(
        "-u X -a Y -z Z must be specified before the "
        "first input file.");
    return false;
  }

  UDBLog("\t--Reading FinalEvents-style GiBUU file(s) from descriptor: \""
         << opt << "\"");

  // Assume previous or latest specified otherwise.
  int NuType = GiBUUToStdHepOpts::nuTypes.back();
  int TargetA = GiBUUToStdHepOpts::TargetAs.back();
  int TargetZ = GiBUUToStdHepOpts::TargetZs.back();

  // These get added back by AddFiles so that the number of files and
  // options are synched
  if (GiBUUToStdHepOpts::nuTypes.size() > GiBUUToStdHepOpts::InpFNames.size()) {
    GiBUUToStdHepOpts::nuTypes.pop_back();
  }
  if (GiBUUToStdHepOpts::TargetAs.size() >
      GiBUUToStdHepOpts::InpFNames.size()) {
    GiBUUToStdHepOpts::TargetAs.pop_back();
  }
  if (GiBUUToStdHepOpts::TargetZs.size() >
      GiBUUToStdHepOpts::InpFNames.size()) {
    GiBUUToStdHepOpts::TargetZs.pop_back();
  }

  double FileExtraWeight = 1.0;
  if (GiBUUToStdHepOpts::FileExtraWeights.size() >
      GiBUUToStdHepOpts::InpFNames.size()) {
    FileExtraWeight = GiBUUToStdHepOpts::FileExtraWeights.back();
    GiBUUToStdHepOpts::FileExtraWeights.pop_back();
  }

  return AddFiles(opt, IsCC, NuType, TargetA, TargetZ, FileExtraWeight);
}

bool Handle_OutputFile(std::string const &opt) {
  GiBUUToStdHepOpts::OutFName = opt;
  UDBLog("\t--Writing to file " << opt);
  return true;
}

bool Handle_nuPDG(std::string const &opt) {
  int ival = 0;
  try {
    ival = Utils::str2i(opt, true);
  } catch (...) {
    return false;
  }

  if (GiBUUToStdHepOpts::nuTypes.size() > GiBUUToStdHepOpts::InpFNames.size()) {
    UDBError(
        "Found another -u option before "
        "the next file has been specified.");
    return false;
  }

  UDBLog("\t--Assuming next file has Nu PDG: " << ival);
  GiBUUToStdHepOpts::nuTypes.push_back(ival);

  // Set default flux weights, these will be filled in if
  // input fluxes are specified for these species.
  if (!GiBUUToStdHepOpts::CompositeFluxWeight.count(ival)) {
    GiBUUToStdHepOpts::CompositeFluxWeight[ival] = 0;
    GiBUUToStdHepOpts::CompositeFluxWeight[ival + 100] = 0;
  }

  return true;
}

bool Handle_IsNC(std::string const &opt) {
  if (GiBUUToStdHepOpts::CCFiles.size() > GiBUUToStdHepOpts::InpFNames.size()) {
    UDBError(
        "Found another -N option before "
        "the next file has been specified.");
    return false;
  }

  UDBLog("\t--Assuming next files contains NC event.");
  GiBUUToStdHepOpts::CCFiles.push_back(false);
  return true;
}

bool Handle_TargetA(std::string const &opt) {
  if (GiBUUToStdHepOpts::TargetAs.size() >
      GiBUUToStdHepOpts::InpFNames.size()) {
    UDBError(
        "Found another -a option before "
        "the next file has been specified.");
    return false;
  }

  int ival = 0;
  try {
    ival = Utils::str2i(opt, true);
  } catch (...) {
    return false;
  }

  UDBLog("\t--Assuming next files are target A: " << ival);
  GiBUUToStdHepOpts::TargetAs.push_back(ival);
  return true;
}

bool Handle_TargetZ(std::string const &opt) {
  if (GiBUUToStdHepOpts::TargetZs.size() >
      GiBUUToStdHepOpts::InpFNames.size()) {
    UDBError(
        "Found another -z option before "
        "the next file has been specified.");
    return false;
  }

  int ival = 0;
  try {
    ival = Utils::str2i(opt, true);
  } catch (...) {
    return false;
  }
  UDBLog("\t--Assuming next files are target Z: " << ival);
  GiBUUToStdHepOpts::TargetZs.push_back(ival);
  return true;
}

bool Handle_FileWeight(std::string const &opt) {
  if (GiBUUToStdHepOpts::FileExtraWeights.size() >
      GiBUUToStdHepOpts::InpFNames.size()) {
    UDBError(
        "Found another -W option before "
        "the next file has been specified.");
    return false;
  }

  double ival = 0;
  bool IsReciprocal = false;
  std::string inp = opt;
  if (inp[0] == 'i') {
    IsReciprocal = true;
    inp = Utils::Replace(inp, "i", "");
  }

  try {
    ival = Utils::str2d(inp, true);
    if (IsReciprocal) {
      ival = (1.0 / ival);
    }
  } catch (...) {
    return false;
  }
  UDBLog("\t--Assigning next file target weight: " << ival);
  GiBUUToStdHepOpts::FileExtraWeights.push_back(ival);
  return true;
}

bool Handle_TotalReWeight(std::string const &opt) {
  double ival = 0;
  bool IsReciprocal = false;
  std::string inp = opt;
  if (inp[0] == 'i') {
    IsReciprocal = true;
    inp = Utils::Replace(inp, "i", "");
  }

  try {
    ival = Utils::str2d(inp, true);
    if (IsReciprocal) {
      ival = (1.0 / ival);
    }
  } catch (...) {
    return false;
  }
  UDBLog("\t--Assigning overall weight: " << ival);
  GiBUUToStdHepOpts::OverallWeight = ival;
  return true;
}

bool Handle_Verbosity(std::string const &opt) {
  int ival = 0;
  try {
    ival = Utils::str2i(opt, true);
  } catch (...) {
    return false;
  }
  UDBLog("\t--Verbosity: " << ival);
  UDBDebugging::SetDebugLevel(ival);
  UDBDebugging::SetInfoLevel(ival);
  return true;
}

bool Handle_NoInitialState(std::string const &opt) {
  GiBUUToStdHepOpts::HaveStruckNucleonInfo = false;
  UDBLog(
      "\t--Not expecting FinalEvents.dat to contain "
      "initial state info.");
  return true;
}

bool Handle_NoProdCharge(std::string const &opt) {
  GiBUUToStdHepOpts::HaveProdChargeInfo = false;
  UDBLog(
      "\t--Not expecting FinalEvents.dat to contain "
      "neutrino induced resonance charge info.");
  return true;
}

bool Handle_SaveFluxFile(std::string const &opt) {
  auto const &split = Utils::SplitStringByDelim(opt, ",");
  if (split.size() != 2) {
    UDBLog(
        "[ERROR]: Expected -F argument to look like "
        "`histname,inputfilename.txt`.");
    return false;
  }

  UDBLog("\t--Saving Flux histogram: " << split.front()
                                       << ", from input: " << split.back());
  GiBUUToStdHepOpts::FluxFilesToAdd.push_back(
      std::make_pair(split.front(), split.back()));
  return true;
}

bool Handle_CLIInputFile(std::string const &opt) {
  std::ifstream ifs(opt);

  if (!ifs.good()) {
    std::cerr << "Failed to open " << opt << " for reading CLI args from."
              << std::endl;
    return false;
  }
  std::string line;
  size_t NAdded = 0;
  while (std::getline(ifs, line)) {
    std::vector<std::string> const &split =
        Utils::SplitStringByDelim(line, " \t\n");
    for (size_t s_it = 0; s_it < split.size(); ++s_it) {
      NAdded++;
      CLIFileArgs.push_back(split[s_it]);
    }
  }
  ifs.close();
  std::cout << "[CLI]: Added " << NAdded << " CLI arguments from " << opt
            << std::endl;
  return true;
}

namespace GiBUUToStdHep_CLIOpts {
void SetDefaults() {
  GiBUUToStdHepOpts::OutFName = "GiBUURooTracker.root";
  GiBUUToStdHepOpts::OverallWeight = 1;
  GiBUUToStdHepOpts::HaveStruckNucleonInfo = true;
  GiBUUToStdHepOpts::HaveProdChargeInfo = true;
  UDBDebugging::SetDebugLevel(2);
  UDBDebugging::SetInfoLevel(2);
}
bool HandleArgs(int const argc, char const *argv[]) {
  SetDefaults();

  size_t requiredArguments = 0;

  std::vector<std::string> ArgArray;
  for (int opt_it = 1; opt_it < argc; ++opt_it) {
    ArgArray.push_back(argv[opt_it]);
  }

  bool LastArgOkay = true;
  std::string arg, opt;
  for (size_t opt_it = 0; opt_it < ArgArray.size();) {
    if (!LastArgOkay) {
      UDBError("Argument: \"" << arg << (arg.length() ? std::string(" ") + arg
                                                      : std::string(""))
                              << "\" was not correctly understood.");
      return false;
    }
    arg = ArgArray[opt_it++];
    opt = "";
    if (("-c" == arg) || ("--CompositeExample" == arg)) {
      LastArgOkay = Handle_CompositeExample(opt);
      continue;
    }

    if (("-f" == arg) || ("--FEinput-file" == arg)) {
      if(opt_it == ArgArray.size()){
        UDBError("Parameter  expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_FEFile(opt);
      requiredArguments |= 1;
      continue;
    }

    if (("-o" == arg) || ("--output-file" == arg)) {
      if(opt_it == ArgArray.size()){
        UDBError("Parameter -o expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_OutputFile(opt);
      continue;
    }

    if (("-u" == arg) || ("--nu-pdg" == arg)) {
      if(opt_it == ArgArray.size()){
        UDBError("Parameter -u expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_nuPDG(opt);
      requiredArguments |= 2;
      continue;
    }
    if (("-N" == arg) || ("--is-NC" == arg)) {
      LastArgOkay = Handle_IsNC(opt);
      continue;
    }
    if (("-a" == arg) || ("--target-a" == arg)) {
      if(opt_it == ArgArray.size()){
        UDBError("Parameter -a expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_TargetA(opt);
      requiredArguments |= 4;
      continue;
    }
    if (("-z" == arg) || ("--target-z" == arg)) {
      if(opt_it == ArgArray.size()){
        UDBError("Parameter -z expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_TargetZ(opt);
      requiredArguments |= 8;
      continue;
    }

    if (("-W" == arg) || ("--file-weight" == arg)) {
      if(opt_it == ArgArray.size()){
        UDBError("Parameter -W expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_FileWeight(opt);
      continue;
    }
    if (("-R" == arg) || ("--Total-ReWeight" == arg)) {
      if(opt_it == ArgArray.size()){
        UDBError("Parameter -R expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_TotalReWeight(opt);
      continue;
    }
    if (("-v" == arg) || ("--Verbosity" == arg)) {
      if(opt_it == ArgArray.size()){
        UDBError("Parameter -v expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_Verbosity(opt);
      continue;
    }
    if (("-NI" == arg) || ("--No-Initial-State" == arg)) {
      LastArgOkay = Handle_NoInitialState(opt);
      continue;
    }
    if (("-NP" == arg) || ("--No-Prod-Charge" == arg)) {
      LastArgOkay = Handle_NoProdCharge(opt);
      continue;
    }
    if (("-F" == arg) || ("--Save-Flux-File" == arg)) {
      if(opt_it == ArgArray.size()){
        UDBError("Parameter -F expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_SaveFluxFile(opt);
      continue;
    }
    if (("-h" == arg) || ("-?" == arg) || ("--help" == arg)) {
      SayRunLike(argv);
      exit(0);
    }
    if (("-@" == arg)) {
      if(opt_it == ArgArray.size()){
        UDBError("Parameter -@ expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_CLIInputFile(opt);
      if (LastArgOkay) {
        ArgArray.insert(ArgArray.begin() + opt_it, CLIFileArgs.begin(),
                        CLIFileArgs.end());
        CLIFileArgs.clear();
      }
      continue;
    }
    std::cout << "[ERROR]: Unexpected argument: " << arg << std::endl;
    SayRunLike(argv);
    exit(1);
  }

  if (requiredArguments != 15) {
    std::cout << "[ERROR]: Not all required arguments were found: -u, -a, -z, "
                 "-f"
              << std::endl;
    return false;
  }
  return LastArgOkay;
}
void SayRunLike(char const *argv[]) {
  std::cout
      << "[RUNLIKE]: " << argv[0]
      << "-u <Next file neutrino PDG code> -a <Next file "
         "target nucleus 'A'> -z <Next file target nucleus 'Z'> -f <File Name> "
         "[-h] [-@ Read "
         "CLI from specified file] [-c] [-o <File Name "
         "{default:GiBUURooTracker.root}>] [-N] [-W [i]<Next file target "
         "weight [1.0/]'W'>] [-R [i]<Overall extra weight [1.0/]'W' -- This is "
         "most useful for weighting composite targets back to a weight per "
         "nucleon>] [-v <0-4>{default==0}] [-NI] [-NP] [-F "
         "[output_hist_name,input_text_flux_file.txt]]"

      << "\n-----------------------------------\n"

      << "\n\t[Arg]: (-h|-?|--help)"
      << "\n\t[Arg]: (-@) Read CLI from specified file"
      << "\n\t[Arg]: (-c|--CompositeExample)"
      << "\n\t[Arg]: (-f|--FEinput-file) <File Name> [Required]"
      << "\n\t[Arg]: (-o|--output-file) <File Name {default:GiBUURooTracker.root}>"
      << "\n\t[Arg]: (-u|--nu-pdg) <Next file neutrino PDG code> [Required]"
      << "\n\t[Arg]: (-N|--is-NC)"
      << "\n\t[Arg]: (-a|--target-a) <Next file target nucleus 'A'> [Required]"
      << "\n\t[Arg]: (-z|--target-z) <Next file target nucleus 'Z'> [Required]"
      << "\n\t[Arg]: (-W|--file-weight) [i]<Next file target weight [1.0/]'W'>"
      << "\n\t[Arg]: (-R|--Total-ReWeight) [i]<Overall extra weight [1.0/]'W' -- "
         "This is most useful for weighting composite targets back to a weight "
         "per nucleon>"
      << "\n\t[Arg]: (-v|--Verbosity) <0-4>{default==0}"
      << "\n\t[Arg]: (-NI|--No-Initial-State)"
      << "\n\t[Arg]: (-NP|--No-Prod-Charge)"
      << "\n\t[Arg]: (-F|--Save-Flux-File) "
         "[output_hist_name,input_text_flux_file.txt]"
      << std::endl;
}
}
