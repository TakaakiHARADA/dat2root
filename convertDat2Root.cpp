#include "d2r.h"
#include "Entry.h"
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
#include <TParameter.h>
#include <vector>
#include <array>
#include <string>
#include <random>
#include <iterator>
#include <queue>
#define NWORD 73

const double ms = 0.001;
const double count = 1.0 * ms;

int main(int argc, char *argv[])
{
    if (!properlyUsed(argc, argv))
    {
        return -1;
    }
    std::cout << "``` wc -l <ファイル名>|awk '{print $1/5}' ```であらかじめエントリー数を確認しておいてね！" << std::endl;

    std::string s = netFileName(argv[1]); // datfileから拡張子[.dat]を取った残り
    std::string rootFileName = "./rootfile/" + s + ".root";
    auto outfile = new TFile(rootFileName.c_str(), "recreate"); // TFileにはconst char*を食わせないといけない
    std::ifstream ifs("./datfile/" + s + ".dat");               // ifstreamはstringでもOK

    // TTree作成
    TTree *tree = new TTree("tree", "converted tree");
    Entry entry;
    tree->Branch("ADC", &entry.ADC_);
    tree->Branch("CR1", &entry.CR1_);
    tree->Branch("CR2", &entry.CR2_);
    tree->Branch("TDC", &entry.TDC_);
    tree->Branch("TDCHit", &entry.TDCHit_);
    tree->Branch("SCALER", &entry.Scaler_);
    tree->Branch("isHit", &entry.isHit_);
    tree->Branch("isClock", &entry.isClock_);
    tree->Branch("nHit", &entry.nHit_);
    tree->Branch("DELTA", &entry.delta_);
    tree->Branch("ADCkeV", &entry.ADCkeV_);
    tree->Branch("hasPedeShift", &entry.hasPedeShift_);
    tree->Branch("ACCUM", &entry.accum_);

    // *************************************************************
    // *************************************************************TTreeに値を詰める
    // ---------- temporal variables ----------
    int word, iWord = 0;
    long long iEntry = 0;
    // long long nEntry = nLine("datfile/" + s + ".dat");
    long long nEntry = 0;
    if (nEntry % 5 != 0)
    {
        std::cout << "datfileの最終行が改行文字で終わっていない可能性があります！" << std::endl;
        return -1;
    }
    nEntry = nEntry / 5;
    std::array<int, NWORD> buffer; // 73 wordsを一旦記憶しておくバッファ
    // ----------------------------------------

    while (ifs >> word)
    {
        buffer.at(iWord) = word;
        ++iWord;
        if (iWord < NWORD)
        {
            continue;
        }

        // 1 Event = 73 wordsの場合は以下を実行
        iWord = 0;
        iEntry += 1;
        entry.update_all(buffer);

        // 進捗表示
        if (iEntry % 100000 == 0)
        // if (entry.isClock_)
        {
            printf("Event: %lld/%lld\n", iEntry, nEntry);
        }
        tree->Fill();
    }

    // 終了処理
    TParameter<double> realtime, deadtime, livetime;
    TParameter<Long64_t> acquried, required;
    realtime.SetVal(entry.accum_.at(0) * count);
    deadtime.SetVal(entry.accum_.at(1) * count);
    realtime.SetVal((entry.accum_.at(0) - entry.accum_.at(1)) * count);
    acquried.SetVal(entry.accum_.at(2));
    required.SetVal(entry.accum_.at(3));
    realtime.Write("realtime");
    deadtime.Write("deadtime");
    realtime.Write("realtime");
    acquried.Write("acquired");
    required.Write("required");
    outfile->Write();
    outfile->Close();

    return 0;
}
