#include "Entry.h"
// #include <iostream>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <vector>
#define PERIOD 600.0 // 過去PERIOD秒間のclock eventのみがペデスタル変動の監視対象

const double ms = 0.001;
const double count = 1.0 * ms;
const int ch = 1;
const int thresh = 3 * ch; // 許容できるペデスタル変動

template <typename T>
std::ostream &operator<<(std::ostream &o, const std::vector<T> &v)
{
    o << "{";
    for (int i = 0; i < (int)v.size(); i++)
    {
        o << (i > 0 ? ", " : "") << v.at(i);
    }
    o << "}";
    return o;
}

// コンストラクタ
// 初期化子リストを使ってprevScaler_とaccum_とsumADC_を0埋め、分布の範囲を0.0~1.0に設定、genにデバイス乱数のシードを食わせる。
Entry::Entry()
    : ADC_(30, 0), TDC_(30, 0), TDCHit_(1, 0), Scaler_(10, 0), isHit_(30, false), prevScaler_(10, 0), delta_(5, 0), ADCkeV_(30, 0.0), accum_(5, 0), hasPedeShift_(false), sumADC_(), gen(std::random_device{}()), dis(0.0, 1.0)
{
    std::ifstream ifsSI("SI.dat");
    double slp, intcp;
    for (int i = 0; i < 30; ++i)
    {
        ifsSI >> intcp >> slp;
        this->slope_.at(i) = slp;
        this->intercept_.at(i) = intcp;
    }
    ifsSI.close();
}

// デストラクタ
Entry::~Entry(){}; // 何もしてないので、書かなくてもコンパイラが勝手につけてくれると思う

// std::array<int, 73>のバッファから、各配列に値を詰めている
void Entry::fill(const std::array<int, NDATA> &ar)
{
    // 生データを詰める
    // --------------------------------------------
    for (int iData = 0; iData <= 29; ++iData)
    {
        // std::cout << "vec len=" << ADC_.size() << std::endl;
        this->ADC_.at(iData - 0) = ar.at(iData); // 最初の30個, #0~29はADC
        // std::cout << "Pushing back ar[" << iData << "] complete!!" << std::endl;
    }
    // --------------------------------------------
    this->CR1_ = ar.at(30); // #30, 31はCoinReg
    this->CR2_ = ar.at(31);
    // --------------------------------------------
    for (int iData = 32; iData <= 55; ++iData)
    {
        // std::cout << "vec len=" << TDC_.size() << std::endl;
        this->TDC_.at(iData - 32) = ar.at(iData); // 次の24個, #32~55はTDC
    }
    // --------------------------------------------
    this->TDCHit_.at(0) = ar.at(56); // なぜか途中でTDC Hitというのが1個だけ挟まっている
    // --------------------------------------------
    for (int iData = 57; iData <= 62; ++iData)
    {
        this->TDC_.at(iData - 33) = ar.at(iData);
        // TDCの残りの6個 #57~62
        // data #57 <-> TDC #24 なので差は33
    }
    // --------------------------------------------
    for (int iData = 63; iData <= 72; ++iData)
    {
        this->Scaler_.at(iData - 63) = ar.at(iData); // SCALER10個 #63~72 ここまでで合計73data
    }
}

void Entry::update_isHit_()
{
    /*
    example:
        CR1_について。同様に処理されるのでCR2_は省略。
        CR1_ = 0001 0001 0000 0001 => CsI No.1,9,13に放射線がhit
                  ^    ^         ^
                  13   9         1
        よってisHit_の[0],[8],[12]をtrueに、残りをfalseにする。
    */

    for (int i = 0; i < 16; ++i)
    {
        this->isHit_.at(i) = (this->CR1_ >> i & 1) == 1 ? true : false;
    }
    for (int i = 0; i < 14; ++i)
    {
        this->isHit_.at(i + 16) = (this->CR2_ >> i & 1) == 1 ? true : false;
    }
}

void Entry::update_isClock_()
{
    this->isClock_ = (this->CR2_ >> 14 & 1) == 1 ? true : false;
}

void Entry::update_nHit_()
{
    this->nHit_ = std::count(this->isHit_.begin(), this->isHit_.end(), true);
}

// SCALERの増分を数える。ch 1~4専用
int d2t::delta(const int l, const int m, const int L, const int M)
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
int d2t::deltaPS(const int l, const int m, const int L, const int M)
{
    bool cleared = (m > M); // 上位ビットが減ったときにclearとみなす（たぶんコレで大丈夫のはず）
    if (cleared)
    {
        // クリアと同時に下位ビットも変わる現象がプラシンでのみ発生。
        // そのままだとL-l(<0)を返すので、M=m+1を勘案して65536を足してから返す
        // return (L-l)+65536*(m+1-m) = (L-l)+65536
        return (L - l) + 65536;
    }
    else
    {
        return (L - l) + 65536 * (M - m);
    }
}

void Entry::update_delta_()
{
    for (int i = 0; i < 4; ++i)
    {
        const int l = this->prevScaler_.at(2 * i);
        const int m = this->prevScaler_.at(2 * i + 1);
        const int L = this->Scaler_.at(2 * i);
        const int M = this->Scaler_.at(2 * i + 1);
        this->delta_.at(i) = d2t::delta(l, m, L, M);
    }
    // プラシン
    {
        const int l = this->prevScaler_.at(2 * 4);
        const int m = this->prevScaler_.at(2 * 4 + 1);
        const int L = this->Scaler_.at(2 * 4);
        const int M = this->Scaler_.at(2 * 4 + 1);
        this->delta_.at(4) = d2t::deltaPS(l, m, L, M);
    }
}

void Entry::update_ADCkeV_()
{
    for (int i = 0; i < 30; ++i)
    {
        auto A = this->slope_.at(i);
        auto x = this->ADC_.at(i) + this->dis(this->gen);
        auto B = this->intercept_.at(i);
        this->ADCkeV_.at(i) = A * x + B;
    }
}

void Entry::update_accum_()
{
    for (int i = 0; i < (int)this->delta_.size(); ++i)
    {
        this->accum_.at(i) += this->delta_.at(i);
    }
}

void Entry::update_prevScaler_()
{
    for (int i = 0; i < (int)this->Scaler_.size(); ++i)
    {
        this->prevScaler_.at(i) = this->Scaler_.at(i); // Scaler_から値をコピーしてくる。
    }
}

void Entry::update_queue()
{
    double cur_time = this->accum_.at(0) * count;
    /*
    以下の3つを管理する:
    [A] 時刻のキュー
    [B] 時刻->ADCchの辞書 x30
    [C] キューで管理しているentryにわたる、ADCの累積 x30
    */

    // 新しいエントリーを追加
    this->qTime_.push(cur_time); // キューに新しいエントリーの時刻を追加 ... [A]
    for (int i = 0; i < (int)this->dict_time2ADC_.size(); ++i)
    {
        this->dict_time2ADC_.at(i)[cur_time] = this->ADC_.at(i); // 辞書にキーと対応する値を追加 ... [B]
        this->sumADC_.at(i) += this->ADC_.at(i);                 // sumを加算 ... [C]
    }

    // 古すぎるエントリーを削除
    while (cur_time - this->qTime_.front() > PERIOD) // キューの先頭要素が過去dt秒間のエントリーになるまで、
    {
        double key = this->qTime_.front();
        this->qTime_.pop(); // キューの先頭要素を削除して ... [A]

        for (int i = 0; i < (int)this->dict_time2ADC_.size(); ++i)
        {
            auto &dict = this->dict_time2ADC_.at(i);
            int val = dict.at(key);
            dict.erase(key);            // 辞書から該当entryを削除して ... [B]
            this->sumADC_.at(i) -= val; // sumADC_から該当のADCchを引き算しつづける ... [C]
        }
    }

    // 今後、ここにhasPedeShiftの処理を書く予定
    // 現時点では、time vs (クロックイベント時の)ADCch のグラフをROOTで描くことで視覚的にぺデスタル変動を監視する

    // 雛形

    bool b = false; // 30個のboolのor
    for (int i = 0; i < (int)this->dict_time2ADC_.size(); ++i)
    {
        double average = (double)this->sumADC_.at(i) / this->qTime_.size();
        double diff = cur_time - average;
        diff = (diff < 0) ? -diff : diff; // 絶対値をとってる。cmathのabsは挙動がおかしいので使っていない。
        b = b || (diff > thresh);
    }
    this->hasPedeShift_ = b;
}

void Entry::clear_all()
{
    // vectorの中身を全部消す。
    // vectorは可変長配列なので、これをしないとどんどん長くなっていってしまう。
    this->ADC_.clear();
}

// 1エントリーごとにやらないといけない作業を全部やる関数。つまり1エントリーごとにこれを呼ぶ。
void Entry::update_all(const std::array<int, NDATA> &ar)
{
    fill(ar);
    update_isHit_();
    update_isClock_();
    update_nHit_();
    update_delta_();
    update_ADCkeV_();
    update_accum_();
    update_prevScaler_();
}