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
#include <string>
#include <random>
#include <iterator>

int delta(const int l, const int m, const int L, const int M);
int deltaPS(const int l, const int m, const int L, const int M);
std::vector<int> delta(const std::vector<int> &oldSCALER, const std::vector<int> &currentSCALER);
std::vector<int> CR(int CR1, int CR2);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Error! Usage:" << std::endl;
        std::cout << "./dat2root<Target File Name(w / o extension)> " << std::endl;
        return 1;
    }

    std::stringstream datFileName, rootFileName;
    datFileName << "./datfile/" << argv[1] << ".dat";
    rootFileName << "./rootfile/" << argv[1] << ".root";
    auto outfile = new TFile(rootFileName.str().c_str(), "recreate"); // TFileにはconst char*を食わせないといけない
    std::ifstream ifs(datFileName.str());                             // ifstreamはstringでもOK

    // TTree作成
    TTree *tree = new TTree("tree", "converted tree");
    std::vector<int> ADC;
    std::vector<int> TDC;
    std::vector<int> TDCHit;
    std::vector<int> SCALER;
    std::vector<int> memoSCALER(10, 0); // [0]*10
    std::vector<int> CoinReg;
    std::vector<double> ADCkeV;        // キャリブレーションで得られたエネルギー(keV)
    std::vector<int> DELTA;            // scalerの増分。int5個。
    int nHit;                          // HitしたCsIの数
    std::vector<Long64_t> ACCUM(4, 0); // DELTAの積分
    tree->Branch("ADC", &ADC);
    tree->Branch("TDC", &TDC);
    tree->Branch("TDCHit", &TDCHit);
    tree->Branch("SCALER", &SCALER);
    tree->Branch("CoinReg", &CoinReg);
    tree->Branch("ADCkeV", &ADCkeV);
    tree->Branch("DELTA", &DELTA);
    tree->Branch("nHit", &nHit);

    // count
    double ms = 0.001;
    auto count = 1.0 * ms;

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
    int word, wordCounter = 0, entryCounter = 0;
    while (ifs >> word)
    {
        wordCounter++;
        int CR1 = 0, CR2 = 0;

        // 最初の30個はADC
        if (wordCounter >= 1 && wordCounter <= 30)
        {
            ADC.push_back(word);
        }

        // 31, 32はCoinReg
        if (wordCounter == 31)
        {
            CR1 = word;
        }
        if (wordCounter == 32)
        {
            CR2 = word;
        }
        CoinReg = CR(CR1, CR2);

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

            // 空のDELTAに中身を入れる
            DELTA = delta(memoSCALER, SCALER);
            for (int i = 0; i < 4; i++)
            {
                ACCUM.at(i) += DELTA.at(i);
            }

            // nHit
            nHit = std::count(CoinReg.begin(), CoinReg.end(), 1);

            // ワードカウンタのリセット
            wordCounter = 0;

            // 進捗表示
            if (entryCounter % 100000 == 0)
            {
                printf("Event: %d\n", entryCounter);
            }
            entryCounter++;

            // 各ループの終了処理
            tree->Fill();
            ADC.clear();
            TDC.clear();
            TDCHit.clear();
            CoinReg.clear();
            ADCkeV.clear();
            memoSCALER.clear();                                                      // memoSCALERの全要素を削除して、
            std::copy(SCALER.begin(), SCALER.end(), std::back_inserter(memoSCALER)); // SCALERから値をコピーしてくる。
            SCALER.clear();                                                          // memoSCALERにコピーしてからclearすること！
            DELTA.clear();
        }
    }

    // 書き出し
    TParameter<double> realtime, livetime;
    TParameter<Long64_t> acquried, required;
    for (int i = 0; i < 4; i++)
    {
        std::cout << "ACCUM[" << i << "] = " << ACCUM.at(i) << std::endl;
    }
    realtime.SetVal(ACCUM.at(0) * count);
    livetime.SetVal(ACCUM.at(1) * count);
    acquried.SetVal(ACCUM.at(2));
    required.SetVal(ACCUM.at(3));
    realtime.Write("realtime");
    livetime.Write("livetime");
    acquried.Write("acquired");
    required.Write("required");
    outfile->Write();
    outfile->Close();

    return 0;
}

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

std::vector<int> delta(const std::vector<int> &oldSCALER, const std::vector<int> &currentSCALER)
{
    // if (currentSCALER.size() % 2 == 1)
    // {
    //     std::cout << "エラー：SCALERの長さが奇数です！" << std::endl;
    // }

    std::vector<int> ret;

    // SCALER No.1~4まで。
    //
    for (int i = 0; i < 4; ++i)
    {
        int l = oldSCALER.at(2 * i);
        int m = oldSCALER.at(2 * i + 1);
        int L = currentSCALER.at(2 * i);
        int M = currentSCALER.at(2 * i + 1);
        ret.push_back(delta(l, m, L, M));
    }
    {
        int l = oldSCALER.at(2 * 4);
        int m = oldSCALER.at(2 * 4 + 1);
        int L = currentSCALER.at(2 * 4);
        int M = currentSCALER.at(2 * 4 + 1);
        ret.push_back(deltaPS(l, m, L, M));
    }
    return ret;
}

std::vector<int> CR(int CR1, int CR2)
{
    std::vector<int> ret;
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
    return ret;
}