#include <iostream>
#include <fstream>
#include <sstream>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TString.h>
#include <TStyle.h>
#include <TF1.h>
#include <vector>
#include <string>
#include <random>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Error! Usage:" << std::endl;
        std::cout << "./dat2root<Target File Name(w / o extension)> " << std::endl;
        exit(0);
    }

    std::stringstream ssdat, ssroot;
    ssdat << "./datfile/" << argv[1] << ".dat";
    ssroot << "./rootfile" << argv[1] << ".root";

    
    return 1;
}