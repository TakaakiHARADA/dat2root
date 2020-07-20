# 2/5に新しいscalerに替えたあとの問題に対応済み

from copy import copy
import datetime


def reader(myfile):
    ret = []
    line = myfile.readline()
    line = myfile.readline()
    line = myfile.readline()
    line = myfile.readline()
    a, b, c, d, e = map(int, line.split()[12:17])
    line = myfile.readline()
    f, g, h, i, j = map(int, line.split())
    ret.append(a)
    ret.append(b)
    ret.append(c)
    ret.append(d)
    ret.append(e)
    ret.append(f)
    ret.append(g)
    ret.append(h)
    ret.append(i)
    ret.append(j)
    return ret


def isCleared(oldnum: int, newnum: int):
    return (oldnum > newnum)


ms = 0.001
count = 1 * ms
counter = [0, 0, 0, 0]


Scaler_old = [0]*10
Scaler_new = [0]*10

filename = input("対象ファイル名を入力（拡張子含む）：")
f = open("/Users/harada/Desktop/dat2root/datfile/" +
         filename, "r")  # ホームディレクトリは適宜書き換えてください
while True:
    try:
        Scaler_old = copy(Scaler_new)
        Scaler_new = reader(f)
        # clearがかかるのは3chだけかと思ったがそんなことはなかった
        bools = [isCleared(Scaler_old[i]+Scaler_old[i+1]*(2 ** 16),
                           Scaler_new[i]+Scaler_new[i+1]*(2 ** 16)) for i in [0, 2, 4, 6, 8]]
        if any(bools):
            print("Cleared!!")
            # print(Scaler_old)
            # print(Scaler_new)
            # print("\n")
            counter[0] += Scaler_old[0] + Scaler_old[1] * (2 ** 16)
            counter[1] += Scaler_old[2] + Scaler_old[3] * (2 ** 16)
            counter[2] += Scaler_old[4] + Scaler_old[5] * (2 ** 16)
            counter[3] += Scaler_old[6] + Scaler_old[7] * (2 ** 16)

            counter[0] -= Scaler_new[0] + Scaler_new[1] * (2 ** 16)
            counter[1] -= Scaler_new[2] + Scaler_new[3] * (2 ** 16)
            counter[2] -= Scaler_new[4] + Scaler_new[5] * (2 ** 16)
            counter[3] -= Scaler_new[6] + Scaler_new[7] * (2 ** 16)
    except ValueError:
        counter[0] += Scaler_old[0] + Scaler_old[1] * (2 ** 16)
        counter[1] += Scaler_old[2] + Scaler_old[3] * (2 ** 16)
        counter[2] += Scaler_old[4] + Scaler_old[5] * (2 ** 16)
        counter[3] += Scaler_old[6] + Scaler_old[7] * (2 ** 16)
        break


if filename == 'BG20200205.dat':
    start = datetime.datetime(year=2020, month=2, day=5,
                              hour=22, minute=47, second=54)
    stop = datetime.datetime(year=2020, month=2, day=6,
                             hour=13, minute=28, second=24)

if filename == 'shieldedBGwoGN2_20200207.dat':
    start = datetime.datetime(year=2020, month=2, day=7,
                              hour=18, minute=26, second=5)
    stop = datetime.datetime(year=2020, month=2, day=9,
                             hour=23, minute=33, second=52)

# seconds = (stop - start).total_seconds()

RealTime = counter[0] * count
DeadTime = counter[1] * count
print(f'''解析結果
------------------------------------------------
ファイル名：{filename}
0ch:{counter[0]}カウント
1ch:{counter[1]}カウント
2ch:{counter[2]}カウント
3ch:{counter[3]}カウント
------------------------------------------------
Real Time={RealTime:.3f}秒
Dead Time={DeadTime:.3f}秒
Live Time={RealTime-DeadTime:.3f}秒
------------------------------------------------
Live Ratio  = {1-DeadTime/RealTime}
------------------------------------------------
''')
