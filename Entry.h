#pragma once
// #include "d2r.h"
#include <array>
#include <random>
#include <queue>
#include <map>
#define NDATA 73
#define PERIOD 600.0 // 過去PERIOD秒間のclock eventのみがペデスタル変動の監視対象

namespace d2t
{
    // SCALERの増分を数える。ch 1~4専用
    int delta(const int l, const int m, const int L, const int M);
    // SCALERの増分を数える。ch 5(プラシン)専用
    int deltaPS(const int l, const int m, const int L, const int M);
} // namespace d2t

// ADC, TDC,...などの1エントリーごとに処理するオブジェクトの構造体. 1度だけ呼ぶこと!
struct Entry
{
    // ****************************************************************************************
    // ****************************************************************************************

    // メンバ変数
    // --------------------------------------------
    // 1. 生データ
    std::array<int, 30> ADC_;    // ADC ch
    int CR1_, CR2_;              // CoinReg下位ビット・上位ビット
    std::array<int, 30> TDC_;    // TDC ch
    std::array<int, 1> TDCHit_;  // 謎のデータ
    std::array<int, 10> Scaler_; // scaler
    // --------------------------------------------
    // 2. 生データから計算されるデータ
    std::array<bool, 30> isHit_;     // Hit判定
    bool isClock_;                   // クロックジェネレータでゲートがかかったイベントかどうか。
    int nHit_;                       // HitしたCsIの数
    std::array<int, 10> prevScaler_; // 1つ前のイベントでのSCALER
    std::array<int, 5> delta_;       // SCALERの増分, 5ch
    std::array<double, 30> ADCkeV_;  // ADC ch をエネルギー(keV)に変換したもの
    std::array<long long, 5> accum_; // DELTAの累積和
    bool hasPedeShift_;              // ペデスタル変動してるかどうか
    // --------------------------------------------
    // 3. クロックイベントの平均ADCを管理するキュー関連
    std::queue<double> qTime_;                            // entryの時刻のキュー
    std::array<std::map<double, int>, 30> dict_time2ADC_; // entry時刻->ADCch の辞書x30
    std::array<int, 30> sumADC_;                          // 過去のADCchの総和x30。平均を出すのに必要。
    // --------------------------------------------
    // 4. ADC -> ADCkeVへの変換に使うちょっとした変数たち
    std::array<double, 30> slope_;
    std::array<double, 30> intercept_;
    std::mt19937 gen;                           // メルセンヌ・ツイスタの生成子
    std::uniform_real_distribution<double> dis; // 実数一様分布。dis(gen)のように生成子を食わせると乱数を返す。

    // ****************************************************************************************
    // ****************************************************************************************

    // コンストラクタ
    Entry();
    // デストラクタ
    ~Entry();

    // ****************************************************************************************

    // 73wordひとかたまりのarrayを食って、個別のarrayに値を詰める
    void fill(const std::array<int, NDATA> &ar);
    void update_isHit_();
    void update_isClock_();
    void update_nHit_();
    void update_delta_();
    void update_ADCkeV_();
    void update_accum_();
    void update_prevScaler_();
    void update_queue();
    void update_all(const std::array<int, NDATA> &ar);

    // ****************************************************************************************
};
