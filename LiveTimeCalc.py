from copy import copy
import datetime
import os


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


def DeltaCleared(l, m, L, M):
    """
    @input:
    l,m: 1つ前のイベントのscaler下位ビット, 上位ビット
    L,M: 現在のイベントのscaler下位ビット, 上位ビット
    @output:
    delta: scalerの増分値(int)
    cleared: scalerがclearされたかどうかを表すbool
    """
    cleared = (m > M)
    if not cleared:
        delta = (L - l) + 65536 * (M - m)
        return (delta, cleared)
    else:
        delta = (L - l) + 65536  # M = m+1 を仮定（たぶん妥当）
        return (delta, cleared)


filename = input("対象ファイル名を入力（拡張子含む）：")
filepath = os.getcwd() + "/datfile/" + filename
existing = os.path.exists(filepath)

if not existing:
    print(f"ファイル: {filepath} は存在しません")
    exit()


ms = 0.001
count = 1 * ms  # 今のところ、1~4chはクロック数=1kHz
counter = [0, 0, 0, 0, 0]


Scaler_old = [0]*10
Scaler_new = [0]*10


f = open(filepath, "r")
while True:
    try:
        Scaler_old = copy(Scaler_new)
        Scaler_new = reader(f)
        Delta = []  # int*5
        Cleared = []  # bool*5
        for i in range(5):
            l, m = Scaler_old[2 * i], Scaler_old[2 * i + 1]
            L, M = Scaler_new[2 * i], Scaler_new[2 * i + 1]
            delta, cleared = DeltaCleared(l, m, L, M)
            Delta.append(delta)
            Cleared.append(cleared)

        if any(Cleared):
            print("Cleared!!")

        for i in range(5):
            counter[i] += Delta[i]

    except ValueError:
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
