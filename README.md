# dat2root
datfileをrootfileに直すプログラム。

## 使い方
datfileっていうディレクトリにdatファイルを入れる。
コマンドライン引数でhogeを入れるとhoge.datからhoge.rootを作ってくれる。
できたrootfileはrootfileっていうディレクトリに入る。

ついでにSI.datから傾きと切片を読み込んでADC(ch)→エネルギー(keV)へ変換した値も詰めてくれる。

## 追記
LiveTimeCalc.pyっていうLivetimeを出してくれるpythonスクリプトもあるよ！
→dat2rootに前イベントからのscaler増分のブランチを追加したので無用
