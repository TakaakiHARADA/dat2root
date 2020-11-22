#include <string>
#include <iostream>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TH1.h>
using namespace std;

std::tuple<bool, std::string> initial_process(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    // しょーもない例外処理
    bool shouldExit;
    std::string rootfile;
    std::tie(shouldExit, rootfile) = initial_process(argc, argv);
    if (shouldExit)
    {
        exit(1);
    }

    // rootfileとtreeを取得
    auto f = new TFile(rootfile.c_str(), "UPDATE");
    auto tree = (TTree *)f->Get("tree");

    // ADCスペクトルを描画
    for (int i = 0; i < 30; i++)
    {
        auto h = new TH1D(Form("h%d", i + 1), Form("ADC No.%d;Ch;Counts/2ch", i + 1), 2048, 0, 4096);
        tree->Draw(Form("ADC[%d]>>h%d;", i, i + 1));
    }

    // ファイルに書き込み＆ファイルを閉じる
    f->Write();
    f->Close();

    return 0;
}

std::tuple<bool, std::string> initial_process(int argc, char *argv[])
{
    /*
    in: コマンドライン引数
    out: 1. 例外なのでexitすべきかどうかを表すbool
         2. rootfileの名前(=argv[1])
    */

    std::string rootfile{argv[1]};
    bool shouldExit = false;

    // 変な入力を除外
    if (argc != 2)
    {
        std::cout << "ERROR! Usage: ./makehist <rootfile name>" << std::endl;
        shouldExit = true;
    }

    // targetファイルの確認
    std::cout << "rootfileは" << rootfile << "でOK？？？" << std::endl;
    rootfile = "rootfile/" + rootfile;

    std::string response;
    std::cin >> response;
    if (response == "y" || response == "yes")
    {
    }
    else
    {
        shouldExit = true;
    }

    return std::forward_as_tuple(shouldExit, rootfile);
}