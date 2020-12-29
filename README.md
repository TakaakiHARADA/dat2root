# dat2root
datfileをrootfileに直すプログラム。

## 使い方
datfileっていうディレクトリにdatファイルを入れる。
コマンドライン引数でhogeを入れるとhoge.datからhoge.rootを作ってくれる。
できたrootfileはrootfileっていうディレクトリに入る。

ついでにSI.datから傾きと切片を読み込んでADC(ch)→エネルギー(keV)へ変換した値も詰めてくれる。

## scalerについて
2020年10月22日現在の設定では、scaler 1~4Chは 1 count = 1 ms, 5chは 1 count = 10 us

1ch : (0.001をかけると)Realtime(秒)

2ch : (0.001をかけると)Deadtime(秒)

3ch : Acquired

4ch : Required

5ch : 宇宙線μの反同時計測のためのプラシンカウント

## 追記

~~LiveTimeCalc.pyっていうLivetimeを出してくれるpythonスクリプトもあるよ！~~
→前イベントからのscaler増分のブランチ(DELTA)を追加したので無用
