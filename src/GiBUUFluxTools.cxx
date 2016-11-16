#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include "TFile.h"
#include "TH1.h"

#include "LUtils/Debugging.hxx"
#include "LUtils/Utils.hxx"

namespace Opts {
int LowBinEdgeColumn = 0;
int UpBinEdgeColumn = -1;
int ValueColumn = 1;
std::string InputFName = "";
std::string OutputFName = "";
bool TextInput = true;
bool DoPDF = true;
std::string InputTHName = "";
}

int WriteFile(float *BinCenters, float *BinWidths, float *BinValues,
              size_t NBins) {
  float Integral = 0;
  float WidthIntegral = 0;
  for (size_t i = 0; Opts::DoPDF && (i < NBins); ++i) {
    Integral += BinValues[i];
    WidthIntegral += BinWidths[i] * BinValues[i];
  }

  std::ofstream of(Opts::OutputFName.c_str());
  of << "# input flux integral: " << Integral
     << " (width integral: " << WidthIntegral << ")" << std::endl;
  if (!of.good()) {
    std::cerr << "[ERROR]: File \"" << Opts::OutputFName
              << " could not be opened for writing." << std::endl;
    return 4;
  }

  for (size_t i = 0; i < NBins; ++i) {
    of << BinCenters[i] << " "
       << (BinValues[i] / (Opts::DoPDF ? Integral : 1.0)) << std::endl;
  }
  of.close();
  return 0;
}

int ROOTTH_ToBinCenterPDFFlux_Text() {
  TFile *inpf = TFile::Open(Opts::InputFName.c_str(), "READ");
  if (!inpf || !inpf->IsOpen()) {
    std::cerr << "[ERROR]: Could not open " << Opts::InputFName
              << " ROOT file for reading." << std::endl;
    return 1;
  }
  TH1 *inph = dynamic_cast<TH1 *>(inpf->Get(Opts::InputTHName.c_str()));
  if (!inph) {
    std::cerr << "[ERROR]: ROOT file " << Opts::InputFName
              << " does not appear to contain a TH1 named: "
              << Opts::InputTHName << std::endl;
    return 2;
  }

  float *BinCenters = new float[inph->GetXaxis()->GetNbins()];
  float *BinWidths = new float[inph->GetXaxis()->GetNbins()];
  float *BinValues = new float[inph->GetXaxis()->GetNbins()];
  for (Int_t i = 0; i < inph->GetXaxis()->GetNbins(); ++i) {
    BinCenters[i] = inph->GetXaxis()->GetBinCenter(i + 1);
    BinWidths[i] = (inph->GetXaxis()->GetBinLowEdge(i + 2) -
                    inph->GetXaxis()->GetBinLowEdge(i + 1));
    BinValues[i] = inph->GetBinContent(i + 1);
    std::cout << "[ROOT] Bin: " << (i + 1) << ", center: " << BinCenters[i]
              << ", width: " << BinWidths[i] << ", value: " << BinValues[i]
              << std::endl;
  }
  bool res =
      WriteFile(BinCenters, BinWidths, BinValues, inph->GetXaxis()->GetNbins());
  delete BinCenters;
  delete BinWidths;
  delete BinValues;
  return res;
}

/// I hate TFL
struct tfl {
  float lbe;
  float ube;
  float v;
};

int Text_BinEdgeToBinCenterPDFFlux_Text() {
  std::ifstream ifs(Opts::InputFName.c_str());
  if (!ifs.good()) {
    std::cerr << "[ERROR]: File \"" << Opts::InputFName
              << " could not be opened for reading." << std::endl;
    return 1;
  }
  std::string line;

  size_t ln = 0;
  std::vector<tfl> BinEdgeVals;
  while (std::getline(ifs, line)) {
    if (line[0] == '#') {  // ignore comments
      ln++;
      continue;
    }

    std::vector<float> splitLine =
        Utils::StringVToFloatV(Utils::SplitStringByDelim(line, " "));
    if ((int(splitLine.size()) < Opts::ValueColumn) ||
        (int(splitLine.size()) < Opts::LowBinEdgeColumn) ||
        (int(splitLine.size()) < Opts::UpBinEdgeColumn)) {
      std::cerr << "[ERROR]: line[#" << ln << "]: " << line << std::endl
                << "\tNeed a minimum of: "
                << std::max(Opts::ValueColumn, std::max(Opts::LowBinEdgeColumn,
                                                        Opts::UpBinEdgeColumn))
                << " columns for current options, found " << splitLine.size()
                << "." << std::endl;
      return 2;
    }
    tfl t;
    if (Opts::UpBinEdgeColumn != -1) {  // If we have both Bin Edge columns
      t.lbe = splitLine[Opts::LowBinEdgeColumn];
      t.ube = splitLine[Opts::UpBinEdgeColumn];
      t.v = splitLine[Opts::ValueColumn];
    } else {  // Only have low bin edges
      t.lbe = splitLine[Opts::LowBinEdgeColumn];
      t.ube = 0xdead;
      t.v = splitLine[Opts::ValueColumn];
    }
    BinEdgeVals.push_back(t);
    ln++;
  }
  ifs.close();

  if (!BinEdgeVals.size()) {
    std::cerr << "[ERROR]: Found no lines " << ln
              << " is badly formed: " << line << std::endl;
    return 2;
  }

  float *BinCenters = new float[BinEdgeVals.size()];
  float *BinWidths = new float[BinEdgeVals.size()];
  float *BinValues = new float[BinEdgeVals.size()];
  for (size_t i = 0; i < BinEdgeVals.size(); ++i) {
    if (Opts::UpBinEdgeColumn != -1) {  // If we have both Bin Edge columns
      BinWidths[i] = BinEdgeVals[i].ube - BinEdgeVals[i].lbe;
    } else {  // Only have low bin edges
      BinWidths[i] = ((i + 1) != BinEdgeVals.size())
                         ? (BinEdgeVals[i + 1].lbe - BinEdgeVals[i].lbe)
                         : (BinEdgeVals[i].lbe -
                            BinEdgeVals[i - 1].lbe);  // Assume the last bin is
                                                      // the same width as the
                                                      // previous bin
    }
    BinCenters[i] = BinEdgeVals[i].lbe + BinWidths[i] / 2.0;
    BinValues[i] = BinEdgeVals[i].v;
  }

  bool res = WriteFile(BinCenters, BinWidths, BinValues, BinEdgeVals.size());
  delete BinCenters;
  delete BinWidths;
  delete BinValues;
  return res;
}

bool Handle_LowBinEdge(std::string const &opt) {
  try {
    Opts::LowBinEdgeColumn = Utils::str2i(opt, true);
  } catch (...) {
    return false;
  }

  std::cout << "\t--Lower bin edge column : " << Opts::LowBinEdgeColumn
            << std::endl;
  return true;
}

bool Handle_UpBinEdge(std::string const &opt) {
  try {
    Opts::UpBinEdgeColumn = Utils::str2i(opt, true);
  } catch (...) {
    return false;
  }

  std::cout << "\t--Upper bin edge column : " << Opts::UpBinEdgeColumn
            << std::endl;
  return true;
}
bool Handle_ValueBin(std::string const &opt) {
  try {
    Opts::ValueColumn = Utils::str2i(opt, true);
  } catch (...) {
    return false;
  }

  std::cout << "\t--Value column : " << Opts::ValueColumn << std::endl;
  return true;
}
bool Handle_InputTexFile(std::string const &opt) {
  Opts::InputFName = opt;
  Opts::TextInput = true;
  std::cout << "\t--Reading from text file " << Opts::InputFName << std::endl;
  return true;
}
bool Handle_InputROOTFile(std::string const &opt) {
  Opts::InputFName = opt;
  Opts::TextInput = false;
  std::cout << "\t--Reading from ROOT file " << Opts::InputFName << std::endl;
  return true;
}
bool Handle_InputROOTHistogram(std::string const &opt) {
  Opts::InputTHName = opt;
  std::cout << "\t--Reading " << Opts::InputTHName << " from ROOT file."
            << std::endl;
  return true;
}
bool Handle_OutputFile(std::string const &opt) {
  Opts::OutputFName = opt;
  std::cout << "\t--Writing to file " << Opts::OutputFName << std::endl;
  return true;
}
bool Handle_KeepNorm(std::string const &opt) {
  Opts::DoPDF = false;
  std::cout << "\t--Keeping original normalisation " << std::endl;
  return true;
}

void SayRunLike(char const *argv[]) {
  std::cout
      << "[RUNLIKE]: " << argv[0]
      << " -o <Output file name> [-h] [-l "
         "<Low bin edge column number>] [-u <Upper bin edge column number>] "
         "[-v Value column number>] [-t <Input text file name>] [-r <Input "
         "ROOT file name>] [-H <Input ROOT histogram name>] [-k]"

      << "\n-----------------------------------\n"

      << "\n\t[Arg]: (-h|--help)"
      << "\n\t[Arg]: (-l|--low-bin-edge) <Low bin edge column number>"
      << "\n\t[Arg]: (-u|--upper-bin-edge) <Upper bin edge column number>"
      << "\n\t[Arg]: (-v|--value) Value column number>"
      << "\n\t[Arg]: (-t|--input-file-text) <Input text file name>"
      << "\n\t[Arg]: (-r|--input-file-ROOT) <Input ROOT file name>"
      << "\n\t[Arg]: (-H|--input-ROOT-histogram) <Input ROOT histogram name>"
      << "\n\t[Arg]: (-o|--output-file) <Output file name> [Required]"
      << "\n\t[Arg]: (-k|--keep-norm)" << std::endl;
}

bool HandleArgs(int argc, char const *argv[]) {
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
    if (("-l" == arg) || ("--low-bin-edge" == arg)) {
      if (opt_it == ArgArray.size()) {
        UDBError("Parameter -l expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_LowBinEdge(opt);
      continue;
    }
    if (("-u" == arg) || ("--upper-bin-edge" == arg)) {
      if (opt_it == ArgArray.size()) {
        UDBError("Parameter -u expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_UpBinEdge(opt);
      continue;
    }
    if (("-v" == arg) || ("--value" == arg)) {
      if (opt_it == ArgArray.size()) {
        UDBError("Parameter -v expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_ValueBin(opt);
      continue;
    }
    if (("-t" == arg) || ("--input-file-text" == arg)) {
      if (opt_it == ArgArray.size()) {
        UDBError("Parameter -t expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_InputTexFile(opt);
      continue;
    }
    if (("-r" == arg) || ("--input-file-ROOT" == arg)) {
      if (opt_it == ArgArray.size()) {
        UDBError("Parameter -r expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_InputROOTFile(opt);
      continue;
    }
    if (("-H" == arg) || ("--input-ROOT-histogram" == arg)) {
      if (opt_it == ArgArray.size()) {
        UDBError("Parameter -H expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_InputROOTHistogram(opt);
      continue;
    }
    if (("-o" == arg) || ("--output-file" == arg)) {
      if (opt_it == ArgArray.size()) {
        UDBError("Parameter -o expected an option.");
        SayRunLike(argv);
        exit(1);
      }
      opt = ArgArray[opt_it++];
      LastArgOkay = Handle_OutputFile(opt);
      continue;
    }
    if (("-k" == arg) || ("--keep-norm" == arg)) {
      LastArgOkay = Handle_KeepNorm(opt);
      continue;
    }
    if (("-?" == arg) || ("-h" == arg) || ("--help" == arg)) {
      SayRunLike(argv);
      exit(0);
    }
    std::cout << "[ERROR]: Unexpected argument: " << arg << std::endl;
    SayRunLike(argv);
    exit(1);
  }
  if (!Opts::InputFName.length()) {
    std::cout << "[ERROR]: Expected -t or -r argument to specify input file."
              << std::endl;
    return false;
  }
  if (!Opts::OutputFName.length()) {
    std::cout << "[ERROR]: Expected -o argument to specify output file."
              << std::endl;
    return false;
  }
  if (!Opts::TextInput && !Opts::InputTHName.length()) {
    std::cout
        << "[ERROR]: Got -r, but no -H argument to specify ROOT histogram name."
        << std::endl;
    return false;
  }
  return LastArgOkay;
}

int main(int argc, char const *argv[]) {
  if (!HandleArgs(argc, argv)) {
    SayRunLike(argv);
    return 1;
  }

  return Opts::TextInput ? Text_BinEdgeToBinCenterPDFFlux_Text()
                         : ROOTTH_ToBinCenterPDFFlux_Text();
}
