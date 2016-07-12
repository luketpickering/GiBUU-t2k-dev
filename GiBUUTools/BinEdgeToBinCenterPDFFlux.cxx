#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "TFile.h"
#include "TH1.h"

#include "LUtils/CLITools.hxx"
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

int WriteFile(std::unique_ptr<float[]> &BinCenters,
              std::unique_ptr<float[]> &BinWidths,
              std::unique_ptr<float[]> &BinValues, size_t NBins) {
  float Integral = 0;
  float WidthIntegral = 0;
  for (size_t i = 0; Opts::DoPDF && (i < NBins); ++i) {
    Integral += BinValues[i];
    WidthIntegral += BinWidths[i] * BinValues[i];
  }

  std::ofstream of(Opts::OutputFName);
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

  std::unique_ptr<float[]> BinCenters(new float[inph->GetXaxis()->GetNbins()]);
  std::unique_ptr<float[]> BinWidths(new float[inph->GetXaxis()->GetNbins()]);
  std::unique_ptr<float[]> BinValues(new float[inph->GetXaxis()->GetNbins()]);
  for (Int_t i = 0; i < inph->GetXaxis()->GetNbins(); ++i) {
    BinCenters[i] = inph->GetXaxis()->GetBinCenter(i + 1);
    BinWidths[i] = (inph->GetXaxis()->GetBinLowEdge(i + 2) -
                    inph->GetXaxis()->GetBinLowEdge(i + 1));
    BinValues[i] = inph->GetBinContent(i + i);
    std::cout << "[ROOT] Bin: " << (i + 1) << ", center: " << BinCenters[i]
              << ", width: " << BinWidths[i] << ", value: " << BinValues[i]
              << std::endl;
  }
  return WriteFile(BinCenters, BinWidths, BinValues,
                   inph->GetXaxis()->GetNbins());
}

int Text_BinEdgeToBinCenterPDFFlux_Text() {
  std::ifstream ifs(Opts::InputFName);
  if (!ifs.good()) {
    std::cerr << "[ERROR]: File \"" << Opts::InputFName
              << " could not be opened for reading." << std::endl;
    return 1;
  }
  std::string line;

  size_t ln = 0;
  std::vector<std::tuple<float, float, float> > BinEdgeVals;
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
    if (Opts::UpBinEdgeColumn != -1) {  // If we have both Bin Edge columns
      BinEdgeVals.push_back(std::make_tuple(splitLine[Opts::LowBinEdgeColumn],
                                            splitLine[Opts::UpBinEdgeColumn],
                                            splitLine[Opts::ValueColumn]));
    } else {  // Only have low bin edges
      BinEdgeVals.push_back(std::make_tuple(splitLine[Opts::LowBinEdgeColumn],
                                            0xdeadbeef,
                                            splitLine[Opts::ValueColumn]));
    }
    ln++;
  }
  ifs.close();

  if (!BinEdgeVals.size()) {
    std::cerr << "[ERROR]: Found no lines " << ln
              << " is badly formed: " << line << std::endl;
    return 2;
  }

  std::unique_ptr<float[]> BinCenters(new float[BinEdgeVals.size()]);
  std::unique_ptr<float[]> BinWidths(new float[BinEdgeVals.size()]);
  std::unique_ptr<float[]> BinValues(new float[BinEdgeVals.size()]);
  for (size_t i = 0; i < BinEdgeVals.size(); ++i) {
    if (Opts::UpBinEdgeColumn != -1) {  // If we have both Bin Edge columns
      BinWidths[i] = std::get<1>(BinEdgeVals[i]) - std::get<0>(BinEdgeVals[i]);
    } else {  // Only have low bin edges
      BinWidths[i] =
          ((i + 1) != BinEdgeVals.size())
              ? (std::get<0>(BinEdgeVals[i + 1]) - std::get<0>(BinEdgeVals[i]))
              : (std::get<0>(BinEdgeVals[i]) -
                 std::get<0>(BinEdgeVals[i - 1]));  // Assume the last bin is
                                                    // the same width as the
                                                    // previous bin
    }
    BinCenters[i] = std::get<0>(BinEdgeVals[i]) + BinWidths[i] / 2.0;
    BinValues[i] = std::get<2>(BinEdgeVals[i]);
  }
  return WriteFile(BinCenters, BinWidths, BinValues, BinEdgeVals.size());
}

void SetOpts() {
  CLIArgs::AddOpt("-l", "--low-bin-edge", true,
                  [&](std::string const &opt) -> bool {
                    try {
                      Opts::LowBinEdgeColumn = Utils::str2i(opt, true);
                    } catch (...) {
                      return false;
                    }

                    std::cout << "\t--Lower bin edge column : "
                              << Opts::LowBinEdgeColumn << std::endl;
                    return true;
                  },
                  false, []() {}, "<Low bin edge column number>");

  CLIArgs::AddOpt("-u", "--upper-bin-edge", true,
                  [&](std::string const &opt) -> bool {
                    try {
                      Opts::UpBinEdgeColumn = Utils::str2i(opt, true);
                    } catch (...) {
                      return false;
                    }

                    std::cout << "\t--Upper bin edge column : "
                              << Opts::UpBinEdgeColumn << std::endl;
                    return true;

                  },
                  false, []() {}, "<Upper bin edge column number>");

  CLIArgs::AddOpt("-v", "--value", true,
                  [&](std::string const &opt) -> bool {
                    try {
                      Opts::ValueColumn = Utils::str2i(opt, true);
                    } catch (...) {
                      return false;
                    }

                    std::cout << "\t--Value column : " << Opts::ValueColumn
                              << std::endl;
                    return true;

                  },
                  false, []() {}, "Value column number>");

  CLIArgs::AddOpt("-t", "--input-file-text", true,
                  [&](std::string const &opt) -> bool {
                    Opts::InputFName = opt;
                    Opts::TextInput = true;
                    std::cout << "\t--Reading from text file "
                              << Opts::InputFName << std::endl;
                    return true;
                  },
                  false, []() {}, "<Input text file name>");

  CLIArgs::AddOpt("-r", "--input-file-ROOT", true,
                  [&](std::string const &opt) -> bool {
                    Opts::InputFName = opt;
                    Opts::TextInput = false;
                    std::cout << "\t--Reading from ROOT file "
                              << Opts::InputFName << std::endl;
                    return true;
                  },
                  false, []() {}, "<Input ROOT file name>");

  CLIArgs::AddOpt("-H", "--input-ROOT-histogram", true,
                  [&](std::string const &opt) -> bool {
                    Opts::InputTHName = opt;
                    std::cout << "\t--Reading " << Opts::InputTHName
                              << " from ROOT file." << std::endl;
                    return true;
                  },
                  false, []() {}, "<Input ROOT histogram name>");

  CLIArgs::AddOpt("-o", "--output-file", true,
                  [&](std::string const &opt) -> bool {
                    Opts::OutputFName = opt;
                    std::cout << "\t--Writing to file " << Opts::OutputFName
                              << std::endl;
                    return true;
                  },
                  true, []() {}, "<Output file name>");

  CLIArgs::AddOpt("-k", "--keep-norm", false,
                  [&](std::string const &opt) -> bool {
                    Opts::DoPDF = false;
                    std::cout << "\t--Keeping original normalisation "
                              << std::endl;
                    return true;
                  },
                  false, []() {}, "<Keep original normalisation>");
}

int main(int argc, char const *argv[]) {
  try {
    SetOpts();
  } catch (std::exception const &e) {
    std::cerr << "[ERROR]: " << e.what() << std::endl;
    return 1;
  }

  CLIArgs::AddArguments(argc, argv);
  if (!CLIArgs::HandleArgs()) {
    CLIArgs::SayRunLike();
    return 1;
  }

  return Opts::TextInput ? Text_BinEdgeToBinCenterPDFFlux_Text()
                         : ROOTTH_ToBinCenterPDFFlux_Text();
}
