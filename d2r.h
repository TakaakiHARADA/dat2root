#pragma once
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TTreeReader.h>
#include <TChain.h>

#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>

int delta(const int l, const int m, const int L, const int M);
int deltaPS(const int l, const int m, const int L, const int M);
std::array<int, 5> delta(const std::array<int, 10> &prevSCALER, const std::array<int, 10> &currentSCALER);
std::array<bool, 30> CR(const int CR1, const int CR2);
bool checkifClockEvent(const int CR2);
std::tuple<std::vector<int>, bool> CR_legacy(int CR1, int CR2);
long long nLine(std::string filename);
bool properlyUsed(int argc, char *argv[]);
std::string netFileName(std::string s);