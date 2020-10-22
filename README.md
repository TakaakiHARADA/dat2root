# dat2root
datfileをrootfileに直すプログラム。

## 使い方
datfileっていうディレクトリにdatファイルを入れる。
コマンドライン引数でhogeを入れるとhoge.datからhoge.rootを作ってくれる。
できたrootfileはrootfileっていうディレクトリに入る。

ついでにSI.datから傾きと切片を読み込んでADC(ch)→エネルギー(keV)へ変換した値も詰めてくれる。

## scalerについて
2020年10月22日現在の設定では、scaler 1~4Chは 1 count = 1 ms, 5chは 1 count = 10 us

1ch : Required

2ch : Acquired

3,4ch : 無用

5ch : 宇宙線μの反同時計測のためのプラシン

## 追記

~~LiveTimeCalc.pyっていうLivetimeを出してくれるpythonスクリプトもあるよ！~~
→dat2rootに前イベントからのscaler増分のブランチを追加したので無用
