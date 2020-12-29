#include "d2r.h"
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

// SCALERの増分を数える。ch 1~4専用
int delta(const int l, const int m, const int L, const int M)
{
    bool cleared = (m > M);
    if (cleared)
    {
        return (L - l);
    }
    else
    {
        return (L - l) + 65536 * (M - m);
    }
}

// SCALERの増分を数える。ch 5(プラシン)専用
int deltaPS(const int l, const int m, const int L, const int M)
{
    bool cleared = (m > M); // 上位ビットが減ったときにclearとみなす（たぶんコレで大丈夫のはず）
    if (cleared)
    {
        return (L - l) + 65536;
    }
    else
    {
        return (L - l) + 65536 * (M - m);
    }
}

// SCALER増分の配列を返す
std::array<int, 5> delta(const std::array<int, 10> &prevSCALER, const std::array<int, 10> &currentSCALER)
{
    std::array<int, 5> ret;

    // SCALER No.1~4まで。
    for (int i = 0; i < 4; ++i)
    {
        int l = prevSCALER.at(2 * i);
        int m = prevSCALER.at(2 * i + 1);
        int L = currentSCALER.at(2 * i);
        int M = currentSCALER.at(2 * i + 1);
        ret.at(i) = delta(l, m, L, M);
    }
    {
        int l = prevSCALER.at(2 * 4);
        int m = prevSCALER.at(2 * 4 + 1);
        int L = currentSCALER.at(2 * 4);
        int M = currentSCALER.at(2 * 4 + 1);
        ret.at(4) = deltaPS(l, m, L, M);
    }
    return ret;
}

// hitしたかどうかのbool x 30
std::array<bool, 30> CR(const int CR1, const int CR2)
{
    /*
    input:
        CR1 = 下位16bit, 右から順にCsI No.1,2,3,...がhitしたかどうか。
        CR2 = 上位14bit+1bit
    output:
        std::vector<int> : 0 or 1 が30個詰まったstd::vector. 1ならそのCsIに放射線がhitしている。
        int : CoinReg2の15chにはclock信号が入っている。このintが1なら、clockでトリガーがかかったイベント。
            そのようなイベントのみでヒストグラムを描くとペデスタルのchが分かる。
    example:
        CR1について。CR2も同様に処理されるので省略。
        CR1 = 0001 0100 0000 0001 => CsI No.1,9,13に放射線がhit=>index=0,8,12がtrueになっている配列
                 ^    ^         ^
                 13   9         1
    */

    std::array<bool, 30> ret;

    for (int i = 0; i < 16; ++i)
    {
        ret.at(i) = (CR1 >> i & 1) == 1 ? true : false;
    }
    for (int i = 0; i < 14; ++i)
    {
        ret.at(i + 16) = (CR2 >> i & 1) == 1 ? true : false;
    }

    return ret;
}

// クロックジェネレータでトリガーがかかったイベントかどうかを表すbool
bool checkifClockEvent(const int CR2)
{
    bool ret = (CR2 >> 14 & 1) == 1 ? true : false;
    // if (ret)
    // {
    //     std::cout << "hoge!" << std::endl;
    // }
    return ret;
}

// 昔使ってたCR
std::tuple<std::vector<int>, bool> CR_legacy(int CR1, int CR2)
{
    /*
    input:
        CR1 = 下位16bit
        CR2 = 上位14bit+1bit
    output:
        std::vector<int> : 0 or 1 が30個詰まったstd::vector. 1ならそのCsIに放射線がhitしている。
        int : CoinReg2の15chにはclock信号が入っている。このintが1なら、clockでトリガーがかかったイベント。
            そのようなイベントのみでヒストグラムを描くとペデスタルのchが分かる。
    */

    std::vector<int> ret;
    bool isClockEvent;

    for (int i = 0; i < 16; ++i)
    {
        ret.push_back(CR1 & 1);
        CR1 = CR1 >> 1;
    }
    for (int i = 0; i < 14; ++i)
    {
        ret.push_back(CR2 & 1);
        CR2 = CR2 >> 1;
    }

    isClockEvent = ((CR2 & 1) == 1 ? true : false);

    return std::forward_as_tuple(ret, isClockEvent);
}

// テキストファイルの行数
long long nLine(std::string filename)
{
    std::ifstream file(filename);
    long long ret = std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n');
    return ret;
}

// コマンドライン引数が適切かをチェックする
bool properlyUsed(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "使い方： ./dat2root <datfile名>" << std::endl;
        return false;
    }
    std::ifstream f(argv[1]);
    bool exist = f.good();
    if (!exist)
    {
        std::cout << "ファイルが見当たりません！" << std::endl;
        return false;
    }
    return true;
}

// "datfile/hoge.dat" -> "hoge"
std::string netFileName(std::string s)
{
    auto begin = s.find_first_of("/") + 1;
    auto nChar = s.find_last_of(".") - s.find_last_of("/") - 1;
    return s.substr(begin, nChar); // sの最後の/と最後の.に挟まれた部分列
}