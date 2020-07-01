#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
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
        return 1;
    }

    std::stringstream ssdat, ssroot;
    ssdat << "./datfile/" << argv[1] << ".dat";
    ssroot << "./rootfile/" << argv[1] << ".root";
    std::string datFileName = ssdat.str();
    auto rootFileName = ssroot.str().c_str();
    std::ifstream ifs(datFileName);
    auto outfile = new TFile(rootFileName, "recreate");

    // TTree作成
    TTree *tree = new TTree("tree", "converted tree");
    std::vector<int> ADC;
    std::vector<int> TDC;
    std::vector<int> TDCHit;
    std::vector<int> SCALER;
    std::vector<int> CoinReg;
    std::vector<double> ADCkeV;
    std::vector<std::string> HitList;
    tree->Branch("ADC", &ADC);
    tree->Branch("TDC", &TDC);
    tree->Branch("TDCHit", &TDCHit);
    tree->Branch("SCALER", &SCALER);
    tree->Branch("CoinReg", &CoinReg);
    tree->Branch("ADCkeV", &ADCkeV);
    tree->Branch("HitList", &HitList);

    // ch->keVの変換関数を用意する
    TF1 *ch2keV[30];
    std::ifstream ifsSI("SI.dat");
    double slope, intercept;
    for (int i = 0; i < 30; i++)
    {
        ifsSI >> slope >> intercept;
        ch2keV[i] = new TF1(Form("ch2keV%d", i + 1), "[0]*x+[1]", -4000.0, 4000.0);
        ch2keV[i]->SetParameters(slope, intercept);
        ch2keV[i]->Write();
    }

    // TTreeに値を詰める
    int word, wordCounter = 0, i = 0;
    while (ifs >> word)
    {
        wordCounter++;
        // 最初の30個はADC
        if (wordCounter >= 1 && wordCounter <= 30)
        {
            ADC.push_back(word);
        }
        // 31, 32はCoinReg
        if (wordCounter >= 31 && wordCounter <= 32)
        {
            CoinReg.push_back(word);
        }
        // 31, 32はCoinReg
        if (wordCounter >= 31 && wordCounter <= 32)
        {
            CoinReg.push_back(word);
        }
        // 次の24個はTDC
        if (wordCounter >= 33 && wordCounter <= 56)
        {
            TDC.push_back(word);
        }
        // なぜか途中にTDCHitというのが挟まっている
        if (wordCounter == 57)
        {
            TDCHit.push_back(word);
        }
        // 6個TDC
        if (wordCounter >= 58 && wordCounter <= 63)
        {
            TDC.push_back(word);
        }
        // 10個Scaler
        if (wordCounter >= 64 && wordCounter <= 73)
        {
            SCALER.push_back(word);
        }
        // 1 Event = 73 wordsでリセット
        if (wordCounter == 73)
        {
            // ch->keV. 乱数を使用。
            double ch, keV;
            std::random_device rd;  //Will be used to obtain a seed for the random number engine
            std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
            std::uniform_real_distribution<double> dis(0.0, 1.0);
            for (int j = 0; j < 30; j++)
            {
                ch = ADC.at(j);
                ch = ch + dis(gen); // 0<=x<1の数xをchに足す。離散を連続化するイメージ。
                keV = ch2keV[j]->Eval(ch);
                ADCkeV.push_back(keV);
            }

            wordCounter = 0;

            if (i % 100000 == 0)
            {
                printf("Event: %d\n", i);
            }
            i++;

            tree->Fill();
            ADC.clear();
            TDC.clear();
            TDCHit.clear();
            SCALER.clear();
            CoinReg.clear();
            ADCkeV.clear();
        }
    }

    // 書き出し
    tree->Write();
    outfile->Close();
    return 0;
}