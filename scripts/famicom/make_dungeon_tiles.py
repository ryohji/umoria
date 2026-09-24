#!/usr/bin/env python3
"""ファミコン向けダンジョンタイルセットを作る。

参考画像（初代ゼルダの地上: 砂地と岩山）のやりかたをダンジョンに移した
16x16 のメタタイルと、全体マップ用の 8x8 罫線タイルを、NES の制約
（8x8 タイル 1 枚 4 色、16x16 ごとに BG パレット 1 本、BG パレット 4 本、
共通の背景色 1 色）の中で組み立て、次のファイルを assets/famicom/ に書き出す。

  dungeon_bg.chr        NES の 2bpp CHR（重複を除いた 8x8 タイル）
  dungeon_metatiles.inc ca65 の表（メタタイル名・パレット・4 枚のタイル番号）
  dungeon_tiles.png     タイル一覧（2 倍）
  dungeon_depths.png    深さごとの配色（2 倍）
  mock_screen.png       ダンジョン画面のモック（256x240 を 3 倍）
  minimap_tiles.png     全体マップ用 8x8 タイル一覧（3 倍）
  mock_minimap.png      全体マップ画面のモック（256x240 を 3 倍）

外部ライブラリは使わない（PNG は zlib で直接書く）。
"""

import os
import random
import struct
import sys
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
OUT = os.path.join(ROOT, "assets", "famicom")

# ---------------------------------------------------------------------------
# NES の色（2C02 の代表的な近似値）
# ---------------------------------------------------------------------------
NES_RGB = {
    0x00: 0x7C7C7C, 0x01: 0x0000FC, 0x02: 0x0000BC, 0x03: 0x4428BC, 0x04: 0x940084,
    0x05: 0xA80020, 0x06: 0xA81000, 0x07: 0x881400, 0x08: 0x503000, 0x09: 0x007800,
    0x0A: 0x006800, 0x0B: 0x005800, 0x0C: 0x004058, 0x0F: 0x000000,
    0x10: 0xBCBCBC, 0x11: 0x0078F8, 0x12: 0x0058F8, 0x13: 0x6844FC, 0x14: 0xD800CC,
    0x15: 0xE40058, 0x16: 0xF83800, 0x17: 0xE45C10, 0x18: 0xAC7C00, 0x19: 0x00B800,
    0x1A: 0x00A800, 0x1B: 0x00A844, 0x1C: 0x008888,
    0x20: 0xF8F8F8, 0x21: 0x3CBCFC, 0x22: 0x6888FC, 0x23: 0x9878F8, 0x24: 0xF878F8,
    0x25: 0xF85898, 0x26: 0xF87858, 0x27: 0xFCA044, 0x28: 0xF8B800, 0x29: 0xB8F818,
    0x2A: 0x58D854, 0x2B: 0x58F898, 0x2C: 0x00E8D8, 0x2D: 0x787878,
    0x30: 0xFCFCFC, 0x31: 0xA4E4FC, 0x32: 0xB8B8F8, 0x33: 0xD8B8F8, 0x34: 0xF8B8F8,
    0x35: 0xF8A4C0, 0x36: 0xF0D0B0, 0x37: 0xFCE0A8, 0x38: 0xF8D878,
    0x39: 0xD8F878, 0x3A: 0xB8F8B8, 0x3B: 0xB8F8D8, 0x3C: 0x00FCFC, 0x3D: 0xF8D8F8,
}



def blank(h=16, w=16):
    return [[0] * w for _ in range(h)]


def from_ascii(rows):
    assert len(rows) == 16, rows
    g = []
    for r in rows:
        assert len(r) == 16, repr(r)
        g.append([0 if ch in ". " else int(ch) for ch in r])
    return g


def transpose(g):
    return [list(row) for row in zip(*g)]



def ellipse(g, cy, cx, ry, rx, fill=2, edge=1, hi=3):
    """楕円の岩・塊を描く。縁 edge、中 fill、左上に明かり hi。"""
    for y in range(16):
        for x in range(16):
            d = ((y - cy) / ry) ** 2 + ((x - cx) / rx) ** 2
            if d <= 1.0:
                inner = ((y - cy) / max(ry - 1, 0.5)) ** 2 + ((x - cx) / max(rx - 1, 0.5)) ** 2
                if inner > 1.0:
                    g[y][x] = edge
                else:
                    hl = ((y - (cy - ry * 0.35)) / (ry * 0.45)) ** 2 + ((x - (cx - rx * 0.35)) / (rx * 0.45)) ** 2
                    g[y][x] = hi if (hi and hl <= 1.0) else fill
    return g




# ---------------------------------------------------------------------------
# パレット
#
# 共通の背景色（各パレットの 0 番）を「床の色」にする。黒ではない。
# こうすると壁のタイルが自分のパレットのまま床の色を含められるので、
# 岩の縁を床に食いこませたり、岩のすき間から床をのぞかせたりできる
# （参考画像の岩山と砂地の境目のやりかた）。
#
#   P0 床    : 床 / 影 / 明るい床（敷石）/ 黒
#   P1 岩    : 床 / 黒 / 岩（中）/ 岩（明）  … 花崗岩・溶岩・瓦礫・扉
#   P2 灰と白: 床 / 黒 / 灰 / 白            … 石英・金属の罠・薬瓶・文字
#   P3 生き物: 床 / 黒 / 緑 / 明るい緑      … モンスターの既定・酸
#
# 未探索の闇は P0 の 3 番（黒）で塗る。
# ---------------------------------------------------------------------------
SCHEMES = [
    # 名前, 床, 影, 明るい床, 岩（中）, 岩（明）
    ("B1-10", 0x36, 0x35, 0x38, 0x17, 0x27),   # 砂岩: 参考画像の配色
    ("B11-20", 0x10, 0x00, 0x37, 0x08, 0x18),  # 石灰岩: 灰の床に黄土色の岩、敷石は淡い黄
    ("B21-30", 0x3B, 0x2B, 0x3A, 0x0B, 0x1A),  # 苔: 薄緑の床に深緑の岩
    ("B31-40", 0x31, 0x21, 0x32, 0x02, 0x12),  # 氷: 水色の床に青い岩
    ("B41-", 0x06, 0x07, 0x08, 0x00, 0x10),    # 火山: 赤黒い床に灰の岩、敷石は玄武岩色
]


def palettes(scheme=0):
    _, bg, sh, lt, rm, rl = SCHEMES[scheme]
    return {
        "P0": [bg, sh, lt, 0x0F],
        "P1": [bg, 0x0F, rm, rl],
        "P2": [bg, 0x0F, 0x00, 0x30],
        "P3": [bg, 0x0F, 0x1A, 0x2A],
    }


PALETTES = palettes(0)

# スプライトパレット（プレイヤー）と、全体マップ画面のパレット（黒地）
SPRITE_PAL = [0x0F, 0x0F, 0x12, 0x37]
MINIMAP_PAL = [0x0F, 0x0C, 0x1C, 0x2C]
MM_TEXT_PAL = [0x0F, 0x0F, 0x00, 0x30]
STAIRS_PAL = [0x0F, 0x0F, 0x28, 0x28]


def noise(t, salt, lo, hi):
    """座標から決まる小さな揺らぎ（乱数ではなく毎回同じ値）。"""
    v = (t * 73 + salt * 151 + (t * t * 17) % 97) % 1009
    return lo + v % (hi - lo + 1)


def overlay(g, rows, mapping=None, only_on=None):
    for r, row in enumerate(rows):
        row = row.ljust(16, ".")
        for c, ch in enumerate(row[:16]):
            if ch in ". ":
                continue
            v = int(ch) if mapping is None else mapping[ch]
            if only_on is None or g[r][c] in only_on:
                g[r][c] = v
    return g


def from_art(rows, mapping=None):
    g = blank()
    return overlay(g, rows, mapping)


# ---------------------------------------------------------------------------
# 岩の壁
#
# 16x16 に岩の塊を 4 つ重ね、すき間は黒。床に面した辺（N E S W）だけ、
# ぎざぎざに削って床の色を出し、辺の近くのすき間も床の色にする。
# 削るのは辺から 7 ドット以内なので、8x8 の 1/4 ごとに影響する辺は 2 つ
# だけになり、16 通りの面しかたを 1 種類あたり 8x8 タイル 16 枚で作れる。
# ---------------------------------------------------------------------------
N, E, S, W = 1, 2, 4, 8

CLUMPS = [
    # 中心 y, x, 半径 y, x。奥（上）から順に描き、手前の岩が奥の岩に重なる
    (4.0, 4.5, 4.6, 4.2),
    (4.5, 12.0, 5.0, 3.6),
    (12.5, 0.5, 3.6, 2.2),
    (12.0, 15.5, 3.8, 2.4),
    (11.0, 7.5, 4.8, 4.6),
]


def rock_base(body=2, hi=3, edge=1, crack=None, crack_on=None, mirror=False):
    g = [[1] * 16 for _ in range(16)]
    rock = [[False] * 16 for _ in range(16)]
    for (cy, cx, ry, rx) in CLUMPS:
        if mirror:
            cx = 15 - cx
            cy = (cy + 7) % 16 if cy < 12 else cy - 9
        for y in range(16):
            for x in range(16):
                d = ((y - cy) / ry) ** 2 + ((x - cx) / rx) ** 2
                if d > 1.0:
                    continue
                rock[y][x] = True
                inner = ((y - cy) / (ry - 1)) ** 2 + ((x - cx) / (rx - 1)) ** 2
                if inner > 1.0:
                    g[y][x] = edge
                    continue
                # 左の縁から 1〜2 ドット内側に縦の明かり（参考画像の岩の筋）
                half = rx * (1 - ((y - cy) / ry) ** 2) ** 0.5
                from_left = x - (cx - half)
                if 1 <= from_left < 2.2 and y < cy + ry * 0.3:
                    g[y][x] = hi
                elif abs(x - (cx + rx * 0.3)) < 0.5 and y > cy + ry * 0.2:
                    # 岩の下半分を縦に割る暗い筋
                    g[y][x] = edge
                else:
                    g[y][x] = body
    if crack:
        paint_on_rock(g, rock, crack, crack_on)
    return g, rock


def paint_on_rock(g, rock, art, only_on=None):
    """岩の上だけに描く（すき間の黒には描かない）。"""
    for r, row in enumerate(art):
        for c, ch in enumerate(row.ljust(16, ".")[:16]):
            if ch not in ". " and rock[r][c] and (only_on is None or g[r][c] in only_on):
                g[r][c] = int(ch)


def erode(g, rock, mask, salt):
    """床に面した辺を削る。"""
    g = [row[:] for row in g]
    for y in range(16):
        for x in range(16):
            for side, dist, t in ((N, y, x), (S, 15 - y, x), (W, x, y), (E, 15 - x, y)):
                if not mask & side:
                    continue
                # 削る深さは 4 ドットごとに変える（細かく揺らすと毛羽立つ）
                cut = noise(t // 4, salt + side, 1, 3)
                if dist < cut or (not rock[y][x] and dist < 7):
                    g[y][x] = 0
    # 削った所に接する岩は黒い輪郭にする
    out = [row[:] for row in g]
    for y in range(16):
        for x in range(16):
            if g[y][x] in (0, 1):
                continue
            for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                ny, nx = y + dy, x + dx
                if 0 <= ny < 16 and 0 <= nx < 16 and g[ny][nx] == 0:
                    out[y][x] = 1
    return out


MAGMA_CRACK = [
    "................",
    "..........3.....",
    "...3......3.....",
    "....3....33.....",
    "....33....3.....",
    ".....3.......3..",
    "................",
    "...........33...",
    "..3.........3...",
    "...33.......3...",
    ".....3..........",
    "....33......3...",
    "....3......33...",
    "................",
    "................",
    "................",
]

QUARTZ_SHARD = [
    "................",
    "..........3.....",
    "...3.....333....",
    "..333....313....",
    "..313...3113....",
    "..3113..........",
    "................",
    "................",
    "...........3....",
    "....3.....313...",
    "...333....3113..",
    "...3113.........",
    "..31113.........",
    "................",
    "................",
    "................",
]

TREASURE = [
    "................",
    "................",
    "................",
    "......3.........",
    ".....333........",
    "......3.........",
    "................",
    "................",
    "..........33....",
    ".........3331...",
    ".........3311...",
    "..........11....",
    "................",
    "................",
    "................",
    "................",
]


def wall_kinds():
    granite = rock_base()
    granite2 = rock_base(mirror=True)
    magma = rock_base(body=1, hi=2, edge=1, crack=MAGMA_CRACK)
    quartz = rock_base(body=2, hi=3, edge=1, crack=QUARTZ_SHARD, crack_on=(2, 3))
    kinds = [("granite", "P1", granite, "花崗岩の壁。外周の壁と隠し扉も同じ絵"),
             ("granite2", "P1", granite2, "花崗岩の壁（模様 2。位置で交互に使う）"),
             ("magma", "P1", magma, "溶岩の鉱脈（黒い岩に光る筋）"),
             ("quartz", "P2", quartz, "石英の鉱脈（灰色の岩に結晶）")]
    for base, name in (("magma", "溶岩"), ("quartz", "石英")):
        g, rock = dict((k[0], k[2]) for k in kinds)[base]
        g = [row[:] for row in g]
        paint_on_rock(g, rock, TREASURE)
        pal = "P1" if base == "magma" else "P2"
        kinds.append((base + "_t", pal, (g, rock), name + "の鉱脈＋宝"))
    return kinds


# ---------------------------------------------------------------------------
# 床（P0: 0 床・1 影・2 明るい床・3 黒）
# ---------------------------------------------------------------------------
RIPPLE_A = [
    "................",
    "................",
    "..11............",
    "....111.........",
    "................",
    "..........11....",
    "............111.",
    "................",
    "................",
    "....11..........",
    "......111.......",
    "................",
    "...........11...",
    ".............11.",
    "................",
    "................",
]

RIPPLE_B = [
    "................",
    ".........11.....",
    "...........111..",
    "................",
    "................",
    "..11............",
    "....11..........",
    "................",
    "..........11....",
    "............11..",
    "................",
    "................",
    "...11...........",
    ".....111........",
    "................",
    "................",
]


def paved(squares):
    g = blank()
    for (sy, sx) in squares:
        for y in range(sy + 1, sy + 7):
            for x in range(sx + 1, sx + 7):
                g[y][x] = 2
    return g


def floor_variants():
    # 小石は岩のパレット（P1: 床・黒・岩・明）。参考画像の茶色い小石
    pebble = from_art([
        "................",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................",
        "......111.......",
        ".....13221......",
        ".....132221.....",
        ".....122221.....",
        "......11111.....",
        "................",
        "................",
        "................",
        "................",
    ])
    part = paved([(0, 0), (8, 8)])
    overlay(part, RIPPLE_A[8:] + ["." * 16] * 8, only_on=(0,))
    return [
        ("floor_a", from_art(RIPPLE_A), "床（さざ波 A）"),
        ("floor_b", from_art(RIPPLE_B), "床（さざ波 B）"),
        ("floor_paved", paved([(0, 0), (0, 8), (8, 0), (8, 8)]), "床（敷石）。部屋に多め"),
        ("floor_paved2", part, "床（敷石が欠けた所）"),
        ("floor_pebble", pebble, "床（小石）。P1"),
    ]


# 部屋と通路で使う床の組み合わせ（位置から決める）
ROOM_FLOORS = ["floor_a", "floor_paved", "floor_b", "floor_paved", "floor_paved2", "floor_a", "floor_pebble"]
CORR_FLOORS = ["floor_a", "floor_b", "floor_a", "floor_pebble"]


def floor_name(y, x, room):
    lst = ROOM_FLOORS if room else CORR_FLOORS
    return lst[noise(x * 31 + y * 17, 9, 0, len(lst) - 1)]


def shadowed(top, left, corner):
    """壁の右・下の床に落ちる影（光は左上から）。"""
    g = from_art(RIPPLE_A)
    for y in range(16):
        for x in range(16):
            if top and y < noise(x, 3, 2, 4):
                g[y][x] = 1
            if left and x < noise(y, 5, 2, 4):
                g[y][x] = 1
            if corner and x + y < 4:
                g[y][x] = 1
    return g


# ---------------------------------------------------------------------------
# 扉（P1: 0 床・1 黒・2 岩色の木・3 明かり）。H は左右が壁の扉
# ---------------------------------------------------------------------------
def door_closed_h():
    g = blank()
    for c in range(16):
        g[0][c] = 1
        g[1][c] = 2 if 0 < c < 15 else 1
    for r in range(2, 16):
        g[r][0] = 1
        g[r][1] = 2
        g[r][14] = 2
        g[r][15] = 1
        for c in range(2, 14):
            p = (c - 2) % 4
            g[r][c] = 3 if p == 0 else (1 if p == 3 else 2)
    for r in (4, 11):
        for c in range(2, 14):
            g[r][c] = 1
        for c in (3, 7, 11):
            g[r][c] = 3
    for (y, x) in [(7, 10), (8, 9), (8, 11), (9, 10)]:
        g[y][x] = 3
    g[8][10] = 1
    for c in range(16):
        g[15][c] = 1
    return g


def door_open_h():
    g = blank()
    for c in range(16):
        g[0][c] = 1
        g[1][c] = 2 if 0 < c < 15 else 1
    for r in range(2, 16):
        g[r][0] = 1
        g[r][1] = 2
        g[r][14] = 2
        g[r][15] = 1
        for c in range(5, 14):
            g[r][c] = 1  # 奥の暗がり
        g[r][2] = 3
        g[r][3] = 2
        g[r][4] = 1
    for r in (4, 11):
        g[r][3] = 1
    # 敷居の先に床がのぞく
    for c in range(5, 14):
        g[15][c] = 0
    return g


# ---------------------------------------------------------------------------
# 階段（P0）
# ---------------------------------------------------------------------------
def stairs(down):
    g = blank()
    for y in range(1, 15):
        for x in range(1, 15):
            g[y][x] = 3
    for x in range(1, 15):
        g[1][x] = 2
    for y in range(1, 15):
        g[y][1] = 2
    if down:
        for i in range(4):
            r = 3 + i * 3
            w = 12 - i * 3
            left = 2 + i
            for c in range(left, min(left + w, 15)):
                g[r][c] = 2
                g[r + 1][c] = 1
    else:
        for i in range(4):
            r = 3 + i * 3
            w = 3 + i * 3
            left = 14 - w
            for c in range(left, 14):
                g[r][c] = 2
                g[r + 1][c] = 1
                g[r + 2][c] = 0 if i == 3 else g[r + 2][c]
    # 右と下は床に落ちる影
    for y in range(2, 16):
        g[y][15] = 1
    for x in range(2, 16):
        g[15][x] = 1
    return g


# ---------------------------------------------------------------------------
# 瓦礫・落石（P1）
# ---------------------------------------------------------------------------
def boulders(spec):
    g = blank()
    for (cy, cx, ry, rx) in spec:
        ellipse(g, cy, cx, ry, rx, fill=2, edge=1, hi=3)
    return g


def rubble():
    return boulders([(6.5, 8, 3.2, 3.8), (10.5, 4.5, 3.2, 3.6), (11, 11, 3, 3.5), (12.5, 7.5, 2.4, 2.8)])


def trap_rock():
    g = boulders([(8, 8, 4.5, 5.5)])
    for (y, x) in [(7, 9), (8, 8), (9, 8), (10, 9), (6, 10)]:
        g[y][x] = 1
    for (y, x) in [(14, 2), (13, 3), (2, 13), (13, 13)]:
        g[y][x] = 1
    return g


# ---------------------------------------------------------------------------
# 罠（ほとんど P0。床の色の上に、影・明るい床・黒で描く）
# ---------------------------------------------------------------------------
def trap_pit():
    g = blank()
    cy = cx = 7.5
    for y in range(16):
        for x in range(16):
            d = ((y - cy) ** 2 + (x - cx) ** 2) ** 0.5
            if d <= 7.0:
                if d > 5.2:
                    g[y][x] = 2 if (x + y) < 14 else 1
                else:
                    d2 = ((y - (cy + 2.2)) ** 2 + (x - cx) ** 2) ** 0.5
                    g[y][x] = 1 if d2 > 5.0 else 3
    return g


def trap_door():
    g = blank()
    for r in range(2, 14):
        for c in range(2, 14):
            if r in (2, 13) or c in (2, 13):
                g[r][c] = 3
            elif (r - 2) % 3 == 0:
                g[r][c] = 3
            else:
                g[r][c] = 2 if (r - 2) % 3 == 1 else 1
    for r in (4, 5, 10, 11):
        g[r][3] = 3
    return g


def trap_dart():
    g = blank()
    for r in range(2, 14):
        for c in range(2, 14):
            g[r][c] = 1 if (r == 13 or c == 13) else 2
    for y in (4, 7, 10):
        for x in (4, 7, 10):
            g[y][x] = 3
            g[y][x + 1] = 3
            g[y + 1][x] = 3
            g[y + 1][x + 1] = 1
    return g


def trap_gas():
    g = blank()
    ellipse(g, 8.5, 7.5, 5.5, 5.5, fill=3, edge=2, hi=0)
    for r in (6, 8, 10):
        for c in range(4, 12):
            if g[r][c] == 3:
                g[r][c] = 1
    for (y, x) in [(1, 11), (0, 12), (1, 13), (2, 12), (3, 13)]:
        g[y][x] = 1
    return g


RUNE = [
    "................",
    ".....333333.....",
    "...33......33...",
    "..3....2.....3..",
    "..3...2.2....3..",
    ".3...2...2....3.",
    ".3..2.....2...3.",
    ".3.2222222222.3.",
    ".3..2.....2...3.",
    ".3...2...2....3.",
    "..3...2.2....3..",
    "..3....2.....3..",
    "...33......33...",
    ".....333333.....",
    "................",
    "................",
]

GLYPH = [
    "................",
    ".....333333.....",
    "...331....133...",
    "..31...33...13..",
    "..3...3223...3..",
    ".31..322223..13.",
    ".3..32211223..3.",
    ".3..32111123..3.",
    ".3..32211223..3.",
    ".31..322223..13.",
    "..3...3223...3..",
    "..31...33...13..",
    "...331....133...",
    ".....333333.....",
    "................",
    "................",
]

SCORCH = [
    "................",
    "................",
    "......1...1.....",
    "...1.11111111...",
    "....1111111111..",
    "..111112111111..",
    "..11112321111.1.",
    "...111232111....",
    "..1111121111....",
    "...11111113111..",
    "....1111111111..",
    "...1.111111.1...",
    ".....1...11.....",
    "................",
    "................",
    "................",
]

ACID = [
    "................",
    "................",
    "..........3.....",
    ".........3.3....",
    "..........3.....",
    "......1111......",
    "....11222211....",
    "...1223222221...",
    "..122222232221..",
    "..12322222221...",
    "...1222223221...",
    "....112222211...",
    "......11111.....",
    "................",
    "................",
    "................",
]


# ---------------------------------------------------------------------------
# モンスター・アイテムの見本（BG で描く。背景は床の色のまま）
# ---------------------------------------------------------------------------
def jelly():
    g = blank()
    ellipse(g, 9.5, 7.5, 5.5, 6.8)
    for (y, x) in [(8, 5), (8, 9)]:
        g[y][x] = 3
        g[y][x + 1] = 3
        g[y + 1][x] = 3
        g[y + 1][x + 1] = 1
    return g


def gold_pile():
    g = blank()
    for (cy, cx) in [(7, 7), (9, 4.5), (9, 10), (11, 7.5), (12, 3.5), (12.5, 11.5)]:
        for y in range(16):
            for x in range(16):
                side = ((y - cy - 1) / 1.7) ** 2 + ((x - cx) / 3.0) ** 2 <= 1.0
                top = ((y - cy) / 1.7) ** 2 + ((x - cx) / 3.0) ** 2
                if top <= 1.0:
                    g[y][x] = 1 if top > 0.55 else 3
                elif side:
                    g[y][x] = 2
    return g


POTION = [
    "................",
    "......1111......",
    "......1331......",
    "......1111......",
    ".......12.......",
    ".......12.......",
    ".....112211.....",
    "....12222221....",
    "...1232222221...",
    "...1322222221...",
    "...1322222221...",
    "...1222222221...",
    "....12222221....",
    ".....111111.....",
    "................",
    "................",
]

PLAYER = [
    "......1111......",
    ".....122221.....",
    "....12222221....",
    "....12111121....",
    "....13333331....",
    "....13133131....",
    ".....133331.....",
    "....11222211....",
    "...1222222221...",
    "..132122221231..",
    "..131222222131..",
    "...1122222211...",
    "....12211221....",
    "....12211221....",
    "....111..111....",
    "...1111..1111...",
]


# ---------------------------------------------------------------------------
# メタタイル一覧（名前, パレット, 絵, 説明）
# ---------------------------------------------------------------------------
METATILES = []


def mt(name, pal, g, note):
    METATILES.append((name, pal, g, note))


def mask_name(m):
    return "".join(ch for bit, ch in ((N, "n"), (E, "e"), (S, "s"), (W, "w")) if m & bit) or "0"


WALL_KINDS = wall_kinds()
for kind, pal, (g, rock), note in WALL_KINDS:
    for m in range(16):
        mt("%s_%s" % (kind, mask_name(m)), pal, erode(g, rock, m, salt=len(kind)),
           "%s（床に面した辺: %s）" % (note, mask_name(m)))

mt("void", "P0", [[3] * 16 for _ in range(16)], "未探索・闇（P0 の黒）")
for name, g, note in floor_variants():
    mt(name, "P1" if name == "floor_pebble" else "P0", g, note)
mt("shadow_n", "P0", shadowed(True, False, False), "床＋上の壁の影")
mt("shadow_w", "P0", shadowed(False, True, False), "床＋左の壁の影")
mt("shadow_nw", "P0", shadowed(True, True, False), "床＋上と左の壁の影")
mt("shadow_c", "P0", shadowed(False, False, True), "床＋左上の角の影")
mt("door_closed_h", "P1", door_closed_h(), "閉じた扉（横の壁）。鍵・つっかえも同じ")
mt("door_closed_v", "P1", transpose(door_closed_h()), "閉じた扉（縦の壁）")
mt("door_open_h", "P1", door_open_h(), "開いた扉・壊れた扉（横の壁）")
mt("door_open_v", "P1", transpose(door_open_h()), "開いた扉・壊れた扉（縦の壁）")
mt("stairs_up", "P0", stairs(False), "上り階段 <")
mt("stairs_down", "P0", stairs(True), "下り階段 >")
mt("rubble", "P1", rubble(), "瓦礫 :")
mt("trap_pit", "P0", trap_pit(), "落とし穴（開いた・覆われた）")
mt("trap_door", "P0", trap_door(), "落とし戸")
mt("trap_dart", "P0", trap_dart(), "矢・吹き矢の罠（穴の空いた板）")
mt("trap_gas", "P0", trap_gas(), "ガスの罠（通気口）")
mt("trap_rune", "P0", from_art(RUNE, {"3": 3, "2": 1}), "不思議なルーン（テレポート・召喚）")
mt("trap_fire", "P0", from_art(SCORCH, {"1": 3, "2": 1, "3": 2}), "焦げ跡（火炎の罠）")
mt("trap_rock", "P1", trap_rock(), "落石の罠（ゆるんだ岩）")
mt("trap_acid", "P3", from_art(ACID), "酸の罠（腐食した岩）")
mt("glyph", "P1", from_art(GLYPH), "守りのルーン（Scare Monster）")
mt("ex_jelly", "P3", jelly(), "見本: ゼリー J")
mt("ex_gold", "P1", gold_pile(), "見本: 金貨の山 $（這う硬貨と共用）")
mt("ex_potion", "P2", from_art(POTION), "見本: 薬瓶 !")

MT_INDEX = {m[0]: i for i, m in enumerate(METATILES)}


# ---------------------------------------------------------------------------
# 全体マップ用 8x8 タイル（4x4 マス = 1 タイル、正方形の区画）
# ---------------------------------------------------------------------------


def mm_corridor(mask):
    g = blank(8, 8)
    for r in range(3, 5):
        for c in range(3, 5):
            g[r][c] = 2
    if mask & N:
        for r in range(0, 4):
            g[r][3] = g[r][4] = 2
    if mask & S:
        for r in range(4, 8):
            g[r][3] = g[r][4] = 2
    if mask & W:
        for c in range(0, 4):
            g[3][c] = g[4][c] = 2
    if mask & E:
        for c in range(4, 8):
            g[3][c] = g[4][c] = 2
    return g


def mm_room(border):
    """border の立っている向きに枠線を描く（隣が部屋でない側）。"""
    g = [[1] * 8 for _ in range(8)]
    for i in range(8):
        if border & N:
            g[0][i] = 3
        if border & S:
            g[7][i] = 3
        if border & W:
            g[i][0] = 3
        if border & E:
            g[i][7] = 3
    return g


def mm_stairs(down):
    """全体マップの階段の目印。BG ではなくスプライト（0 番は透明）。"""
    art = [
        "...1....",
        "..131...",
        ".13331..",
        "1333331.",
        "1111111.",
    ]
    if down:
        art = [
            "1111111.",
            "1333331.",
            ".13331..",
            "..131...",
            "...1....",
        ]
    g = blank(8, 8)
    for r, row in enumerate(art):
        for c, ch in enumerate(row):
            if ch != ".":
                g[r + 1][c] = int(ch)
    return g


def mm_frame(kind):
    g = blank(8, 8)
    if kind == "h":
        for c in range(8):
            g[2][c] = 3
            g[5][c] = 3
    elif kind == "v":
        for r in range(8):
            g[r][2] = 3
            g[r][5] = 3
    else:
        # 角: tl tr bl br（二重線）
        top = kind[0] == "t"
        left = kind[1] == "l"
        outer = 2
        inner = 5
        for i in range(8):
            for j in range(8):
                rr = i if top else 7 - i
                cc = j if left else 7 - j
                on = False
                if rr == outer and cc >= outer:
                    on = True
                if cc == outer and rr >= outer:
                    on = True
                if rr == inner and cc >= inner:
                    on = True
                if cc == inner and rr >= inner:
                    on = True
                if on:
                    g[i][j] = 3
    return g


MINIMAP = []
MINIMAP.append(("mm_blank", blank(8, 8)))
for m in range(16):
    MINIMAP.append(("mm_corr_%02d" % m, mm_corridor(m)))
for m in range(16):
    MINIMAP.append(("mm_room_%02d" % m, mm_room(m)))
for k in ("tl", "tr", "bl", "br", "h", "v"):
    MINIMAP.append(("mm_frame_" + k, mm_frame(k)))
MM_INDEX = {m[0]: i for i, m in enumerate(MINIMAP)}

# 全体マップのスプライト（BG の 256 枚とは別のパターンテーブル）
MM_SPRITES = [("mm_up", mm_stairs(False)), ("mm_down", mm_stairs(True))]


# ---------------------------------------------------------------------------
# 8x8 の文字（モックに要る分だけ。5x7 字形）
# ---------------------------------------------------------------------------
FONT5 = {
    "0": [".###.", "#...#", "#..##", "#.#.#", "##..#", "#...#", ".###."],
    "1": ["..#..", ".##..", "..#..", "..#..", "..#..", "..#..", ".###."],
    "2": [".###.", "#...#", "....#", "...#.", "..#..", ".#...", "#####"],
    "3": ["#####", "...#.", "..#..", "...#.", "....#", "#...#", ".###."],
    "4": ["...#.", "..##.", ".#.#.", "#..#.", "#####", "...#.", "...#."],
    "5": ["#####", "#....", "####.", "....#", "....#", "#...#", ".###."],
    "6": ["..##.", ".#...", "#....", "####.", "#...#", "#...#", ".###."],
    "7": ["#####", "....#", "...#.", "..#..", ".#...", ".#...", ".#..."],
    "8": [".###.", "#...#", "#...#", ".###.", "#...#", "#...#", ".###."],
    "9": [".###.", "#...#", "#...#", ".####", "....#", "...#.", ".##.."],
    "A": [".###.", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"],
    "B": ["####.", "#...#", "#...#", "####.", "#...#", "#...#", "####."],
    "F": ["#####", "#....", "#....", "####.", "#....", "#....", "#...."],
    "H": ["#...#", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"],
    "L": ["#....", "#....", "#....", "#....", "#....", "#....", "#####"],
    "M": ["#...#", "##.##", "#.#.#", "#.#.#", "#...#", "#...#", "#...#"],
    "P": ["####.", "#...#", "#...#", "####.", "#....", "#....", "#...."],
    "U": ["#...#", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."],
    "V": ["#...#", "#...#", "#...#", "#...#", "#...#", ".#.#.", "..#.."],
    "/": ["....#", "...#.", "...#.", "..#..", ".#...", ".#...", "#...."],
    " ": ["....."] * 7,
}


def glyph8(ch):
    """黒地（1 番）に白（3 番）の文字。"""
    g = [[1] * 8 for _ in range(8)]
    for r, row in enumerate(FONT5[ch]):
        for c, px in enumerate(row):
            if px == "#":
                g[r][c + 1] = 3
    return g


# ---------------------------------------------------------------------------
# CHR（8x8 タイル）に分けて重複を除く
# ---------------------------------------------------------------------------
class Chr:
    def __init__(self):
        self.tiles = []
        self.index = {}

    def add(self, g8):
        key = tuple(tuple(r) for r in g8)
        if key not in self.index:
            self.index[key] = len(self.tiles)
            self.tiles.append(g8)
        return self.index[key]

    def bytes(self):
        out = bytearray()
        for t in self.tiles:
            lo = bytearray()
            hi = bytearray()
            for row in t:
                b0 = b1 = 0
                for x, v in enumerate(row):
                    b0 |= (v & 1) << (7 - x)
                    b1 |= ((v >> 1) & 1) << (7 - x)
                lo.append(b0)
                hi.append(b1)
            out += lo + hi
        return bytes(out)


def split16(g):
    return [[row[x:x + 8] for row in g[y:y + 8]] for y in (0, 8) for x in (0, 8)]


def check_colors(g, name):
    for row in g:
        for v in row:
            assert v in (0, 1, 2, 3), (name, v)


# ---------------------------------------------------------------------------
# PNG
# ---------------------------------------------------------------------------
class Canvas:
    def __init__(self, w, h, bg=0x000000):
        self.w, self.h = w, h
        self.px = [[bg] * w for _ in range(h)]

    def put(self, x, y, rgb):
        if 0 <= x < self.w and 0 <= y < self.h:
            self.px[y][x] = rgb

    def draw(self, g, x0, y0, pal, transparent0=False):
        for y, row in enumerate(g):
            for x, v in enumerate(row):
                if transparent0 and v == 0:
                    continue
                self.put(x0 + x, y0 + y, NES_RGB[pal[v]])

    def fill(self, x0, y0, w, h, rgb):
        for y in range(y0, y0 + h):
            for x in range(x0, x0 + w):
                self.put(x, y, rgb)

    def save(self, path, scale=3):
        raw = bytearray()
        for row in self.px:
            line = bytearray([0])
            for rgb in row:
                line += bytes([(rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255]) * scale
            raw += bytes(line) * scale
        w, h = self.w * scale, self.h * scale

        def chunk(t, d):
            c = struct.pack(">I", len(d)) + t + d
            return c + struct.pack(">I", zlib.crc32(t + d) & 0xFFFFFFFF)

        png = b"\x89PNG\r\n\x1a\n"
        png += chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
        png += chunk(b"IDAT", zlib.compress(bytes(raw), 9))
        png += chunk(b"IEND", b"")
        with open(path, "wb") as f:
            f.write(png)


def text(cv, s, x, y, pal=None):
    pal = pal or MM_TEXT_PAL
    for i, ch in enumerate(s):
        cv.draw(glyph8(ch), x + i * 8, y, pal)



# ---------------------------------------------------------------------------
# 全体マップのモック: 96x80 マスの見本ダンジョンを作り、4x4 マスずつ縮める
# ---------------------------------------------------------------------------
DW, DH, R = 96, 80, 4


def make_dungeon(seed=5):
    rnd = random.Random(seed)
    g = [["#"] * DW for _ in range(DH)]
    rooms = []
    slots = [(sy, sx) for sy in range(DH // 16) for sx in range(DW // 16)]
    rnd.shuffle(slots)
    for sy, sx in slots[:17]:
        cy = sy * 16 + 8 + rnd.randint(-2, 2)
        cx = sx * 16 + 8 + rnd.randint(-2, 2)
        y1, y2 = cy - rnd.randint(1, 4), cy + rnd.randint(1, 3)
        x1, x2 = cx - rnd.randint(1, 5), cx + rnd.randint(1, 5)
        y1, x1 = max(y1, 2), max(x1, 2)
        y2, x2 = min(y2, DH - 3), min(x2, DW - 3)
        rooms.append((y1, x1, y2, x2))
        for y in range(y1, y2 + 1):
            for x in range(x1, x2 + 1):
                g[y][x] = "."

    def in_room(y, x):
        return g[y][x] == "."

    def tunnel(a, b):
        (y, x), (ty, tx) = a, b
        dy = (ty > y) - (ty < y)
        dx = (tx > x) - (tx < x) if dy == 0 else 0
        for _ in range(2000):
            if (y, x) == (ty, tx):
                break
            if rnd.randint(1, 100) > 70 or (dy == 0 and dx == 0):
                dy = (ty > y) - (ty < y)
                dx = (tx > x) - (tx < x)
                if dy and dx:
                    if rnd.random() < 0.5:
                        dy = 0
                    else:
                        dx = 0
                if rnd.randint(1, 9) == 1:
                    dy, dx = rnd.choice([(0, 1), (0, -1), (1, 0), (-1, 0)])
            ny, nx = y + dy, x + dx
            if not (1 <= ny < DH - 1 and 1 <= nx < DW - 1):
                dy, dx = 0, 0
                continue
            y, x = ny, nx
            if g[y][x] == "#":
                g[y][x] = ","

    centers = [((a + c) // 2, (b + d) // 2) for a, b, c, d in rooms]
    for i in range(len(centers) - 1):
        tunnel(centers[i], centers[i + 1])
    # 扉: 部屋の縁のすぐ外にある通路
    for (y1, x1, y2, x2) in rooms:
        for y in range(y1 - 1, y2 + 2):
            for x in range(x1 - 1, x2 + 2):
                if (y in (y1 - 1, y2 + 1) or x in (x1 - 1, x2 + 1)) and g[y][x] == ",":
                    if rnd.randint(1, 100) <= 60:
                        g[y][x] = "+"
    # 階段
    for ch in "<>>":
        a, b, c, d = rnd.choice(rooms)
        g[rnd.randint(a, c)][rnd.randint(b, d)] = ch
    return g, rooms


def explore(g, rooms, start, limit=120):
    known = [[False] * DW for _ in range(DH)]
    passable = set(".,+<>")
    dist = {start: 0}
    q = [start]
    while q:
        y, x = q.pop(0)
        known[y][x] = True
        if dist[(y, x)] >= limit:
            continue
        for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            ny, nx = y + dy, x + dx
            if (ny, nx) not in dist and g[ny][nx] in passable:
                dist[(ny, nx)] = dist[(y, x)] + 1
                q.append((ny, nx))
    # 部屋は一歩入れば全体が見える（原典の明るい部屋）
    for (y1, x1, y2, x2) in rooms:
        if any(known[y][x] for y in range(y1, y2 + 1) for x in range(x1, x2 + 1)):
            for y in range(y1, y2 + 1):
                for x in range(x1, x2 + 1):
                    known[y][x] = True
    return known


def minimap_tiles(g, known):
    th, tw = DH // R, DW // R
    passable = set(".,+<>")
    kind = [[None] * tw for _ in range(th)]
    for ty in range(th):
        for tx in range(tw):
            cells = [(y, x) for y in range(ty * R, ty * R + R) for x in range(tx * R, tx * R + R)]
            seen = [(y, x) for (y, x) in cells if known[y][x] and g[y][x] in passable]
            chars = {g[y][x] for (y, x) in seen}
            if "." in chars or "<" in chars or ">" in chars:
                kind[ty][tx] = "room"
            elif seen:
                kind[ty][tx] = "corr"

    def is_room(ty, tx):
        return 0 <= ty < th and 0 <= tx < tw and kind[ty][tx] == "room"

    def ok(y, x):
        return 0 <= y < DH and 0 <= x < DW and known[y][x] and g[y][x] in passable

    out = [["mm_blank"] * tw for _ in range(th)]
    for ty in range(th):
        for tx in range(tw):
            k = kind[ty][tx]
            if k == "room":
                b = 0
                b |= 0 if is_room(ty - 1, tx) else N
                b |= 0 if is_room(ty + 1, tx) else S
                b |= 0 if is_room(ty, tx - 1) else W
                b |= 0 if is_room(ty, tx + 1) else E
                out[ty][tx] = "mm_room_%02d" % b
            elif k == "corr":
                m = 0
                y0, x0 = ty * R, tx * R
                for i in range(R):
                    if ok(y0, x0 + i) and ok(y0 - 1, x0 + i):
                        m |= N
                    if ok(y0 + R - 1, x0 + i) and ok(y0 + R, x0 + i):
                        m |= S
                    if ok(y0 + i, x0) and ok(y0 + i, x0 - 1):
                        m |= W
                    if ok(y0 + i, x0 + R - 1) and ok(y0 + i, x0 + R):
                        m |= E
                out[ty][tx] = "mm_corr_%02d" % m
    return out


def stairs_markers(g, known):
    """覚えている階段の位置（スプライトで出す）。"""
    return [(y, x, g[y][x]) for y in range(DH) for x in range(DW)
            if known[y][x] and g[y][x] in "<>"]


def render_minimap_mock():
    g, rooms = make_dungeon()
    a, b, c, d = rooms[0]
    start = ((a + c) // 2, (b + d) // 2)
    known = explore(g, rooms, start)
    tiles = minimap_tiles(g, known)
    cv = Canvas(256, 240)
    pal = MINIMAP_PAL
    ox, oy = 4, 5  # 地図の左上（タイル単位）
    th, tw = len(tiles), len(tiles[0])
    text(cv, "MAP  B5F", ox * 8, 2 * 8, MM_TEXT_PAL)
    fr = {k: MINIMAP[MM_INDEX["mm_frame_" + k]][1] for k in ("tl", "tr", "bl", "br", "h", "v")}
    cv.draw(fr["tl"], (ox - 1) * 8, (oy - 1) * 8, pal)
    cv.draw(fr["tr"], (ox + tw) * 8, (oy - 1) * 8, pal)
    cv.draw(fr["bl"], (ox - 1) * 8, (oy + th) * 8, pal)
    cv.draw(fr["br"], (ox + tw) * 8, (oy + th) * 8, pal)
    for i in range(tw):
        cv.draw(fr["h"], (ox + i) * 8, (oy - 1) * 8, pal)
        cv.draw(fr["h"], (ox + i) * 8, (oy + th) * 8, pal)
    for j in range(th):
        cv.draw(fr["v"], (ox - 1) * 8, (oy + j) * 8, pal)
        cv.draw(fr["v"], (ox + tw) * 8, (oy + j) * 8, pal)
    for ty in range(th):
        for tx in range(tw):
            cv.draw(MINIMAP[MM_INDEX[tiles[ty][tx]]][1], (ox + tx) * 8, (oy + ty) * 8, pal)
    # 階段（スプライト）: 1 マス = 2 ドット。三角の先が階段のマスを指す
    for (y, x, ch) in stairs_markers(g, known):
        spr = MM_SPRITES[0 if ch == "<" else 1][1]
        cv.draw(spr, ox * 8 + x * 2 - 3, oy * 8 + y * 2 - 3, STAIRS_PAL, transparent0=True)
    # 現在地（スプライト）
    py, px = start
    cv.fill(ox * 8 + px * 2 - 1, oy * 8 + py * 2 - 1, 4, 4, NES_RGB[0x30])
    return cv, tiles, g, known




# ---------------------------------------------------------------------------
# モック画面（16x11 マスの地図。上下の黒帯に文字）
# ---------------------------------------------------------------------------
MOCK = [
    "################",
    "###mmM####qqQ###",
    "##.....#.......#",
    "##.<...+...j...#",
    "##..@..#.^.....#",
    "##..*..#.....!.#",
    "###'####..W....#",
    "# #,#  ###'#####",
    "# #,,,T,,,,,,,>#",
    "# ########:#####",
    "#        #,#    ",
]
WALL_CH = {"#": "granite", "m": "magma", "M": "magma_t", "q": "quartz", "Q": "quartz_t"}
ROOM_CH = set(".<>@*!j^W")
SIMPLE = {"<": "stairs_up", ">": "stairs_down", ":": "rubble", "^": "trap_pit",
          "T": "trap_door", "W": "glyph", "j": "ex_jelly", "*": "ex_gold", "!": "ex_potion"}


def cell(rows, y, x):
    if 0 <= y < len(rows) and 0 <= x < len(rows[0]):
        return rows[y][x]
    return "#"


def is_wall(ch):
    return ch in WALL_CH


def is_open(ch):
    """床として扱うマス（壁でも未探索でもない）。"""
    return not is_wall(ch) and ch != " "


def mock_metatile(rows, y, x):
    ch = rows[y][x]
    if is_wall(ch):
        m = 0
        m |= N if is_open(cell(rows, y - 1, x)) else 0
        m |= S if is_open(cell(rows, y + 1, x)) else 0
        m |= W if is_open(cell(rows, y, x - 1)) else 0
        m |= E if is_open(cell(rows, y, x + 1)) else 0
        kind = WALL_CH[ch]
        if kind == "granite" and (x + y * 3) % 4 in (1, 2):
            kind = "granite2"
        return "%s_%s" % (kind, mask_name(m))
    if ch == " ":
        return "void"
    if ch in "+'":
        horiz = is_wall(cell(rows, y, x - 1))
        return ("door_closed" if ch == "+" else "door_open") + ("_h" if horiz else "_v")
    if ch in SIMPLE:
        return SIMPLE[ch]
    # 床: 上・左の壁の影を先に、なければ位置で決めた模様
    top = is_wall(cell(rows, y - 1, x))
    left = is_wall(cell(rows, y, x - 1))
    if top and left:
        return "shadow_nw"
    if top:
        return "shadow_n"
    if left:
        return "shadow_w"
    if is_wall(cell(rows, y - 1, x - 1)):
        return "shadow_c"
    return floor_name(y, x, ch in ROOM_CH)


def draw_map(cv, rows, pals, x0=0, y0=0, player=True):
    for y, row in enumerate(rows):
        assert len(row) == len(rows[0]), row
        for x in range(len(row)):
            name = mock_metatile(rows, y, x)
            _, pal, g, _ = METATILES[MT_INDEX[name]]
            cv.draw(g, x0 + x * 16, y0 + y * 16, pals[pal])
            if player and rows[y][x] == "@":
                cv.draw(from_ascii(PLAYER), x0 + x * 16, y0 + y * 16, SPRITE_PAL, transparent0=True)


def render_mock(scheme=0):
    pals = palettes(scheme)
    cv = Canvas(256, 240)
    # 上下の帯は黒で塗ったタイル（P2 の 1 番）で作る
    text(cv, "B5F", 16, 4, pals["P2"])
    draw_map(cv, MOCK, pals, 0, 16)
    text(cv, "LV12 HP 45/ 60 MP  8/12", 16, 200, pals["P2"])
    text(cv, "AU 1234", 16, 208, pals["P2"])
    return cv


def render_depths():
    rows = MOCK[:7]
    cv = Canvas(256, len(SCHEMES) * (len(rows) * 16 + 4))
    for j in range(len(SCHEMES)):
        draw_map(cv, rows, palettes(j), 0, j * (len(rows) * 16 + 4))
    return cv


# ---------------------------------------------------------------------------
# 一覧画像
# ---------------------------------------------------------------------------
def render_sheet():
    cols = 16
    cw, ch = 28, 28
    rows = (len(METATILES) + cols - 1) // cols
    cv = Canvas(cols * cw + 8, rows * ch + 8, bg=0x202020)
    pals = palettes(0)
    for i, (name, pal, g, _) in enumerate(METATILES):
        x = 8 + (i % cols) * cw
        y = 4 + (i // cols) * ch
        cv.draw(g, x, y, pals[pal])
        text(cv, "%03d" % i, x - 4, y + 17, MM_TEXT_PAL)
    return cv


def render_minimap_sheet():
    cols = 16
    cv = Canvas(cols * 12 + 4, ((len(MINIMAP) + 2 + cols - 1) // cols) * 12 + 4, bg=0x202020)
    for i, (_, g) in enumerate(MINIMAP):
        cv.draw(g, 4 + (i % cols) * 12, 4 + (i // cols) * 12, MINIMAP_PAL)
    base = len(MINIMAP) + 2
    for j, (_, g) in enumerate(MM_SPRITES):
        i = base + j
        cv.draw(g, 4 + (i % cols) * 12, 4 + (i // cols) * 12, STAIRS_PAL)
    return cv


def main():
    os.makedirs(OUT, exist_ok=True)
    chr_ = Chr()
    chr_.add(blank(8, 8))  # 0 番は床の色の無地
    table = []
    for name, pal, g, note in METATILES:
        check_colors(g, name)
        idx = [chr_.add(q) for q in split16(g)]
        table.append((name, pal, idx, note))
    n_terrain = len(chr_.tiles)
    mm_idx = {}
    for name, g in MINIMAP:
        check_colors(g, name)
        mm_idx[name] = chr_.add(g)
    n_mm = len(chr_.tiles) - n_terrain
    font_idx = {}
    for chn in sorted(FONT5):
        font_idx[chn] = chr_.add(glyph8(chn))
    n_font = len(chr_.tiles) - n_terrain - n_mm
    assert len(chr_.tiles) <= 256, len(chr_.tiles)

    with open(os.path.join(OUT, "dungeon_bg.chr"), "wb") as f:
        f.write(chr_.bytes())

    with open(os.path.join(OUT, "dungeon_metatiles.inc"), "w", encoding="utf-8") as f:
        f.write("; scripts/famicom/make_dungeon_tiles.py が生成。手で直さないこと\n")
        f.write("; メタタイル: 左上・右上・左下・右下のタイル番号, 属性（パレット番号）\n")
        f.write("; 壁の _nesw は床に面した辺（n 上・e 右・s 下・w 左、0 は面していない）\n")
        f.write("; パレット（深さ帯ごと。各行の先頭が共通の背景色＝床）:\n")
        for j, sch in enumerate(SCHEMES):
            p = palettes(j)
            f.write(";   %-7s " % sch[0] + "  ".join(
                "%s=%s" % (k, ",".join("$%02X" % c for c in v)) for k, v in p.items()) + "\n")
        f.write("\n")
        for i, (name, pal, idx, note) in enumerate(table):
            f.write("MT_%s = %d  ; %s\n" % (name.upper(), i, note))
        f.write("\nmetatile_tiles:\n")
        for name, pal, idx, note in table:
            f.write("    .byte $%02X,$%02X,$%02X,$%02X  ; %s\n" % (idx[0], idx[1], idx[2], idx[3], name))
        f.write("\nmetatile_attr:\n")
        for name, pal, idx, note in table:
            f.write("    .byte %d  ; %s\n" % (int(pal[1]), name))
        f.write("\n; 全体マップ用 8x8（全体マップ画面では黒地のパレットに入れかえる）\n")
        for name, _ in MINIMAP:
            f.write("MM_%s = $%02X\n" % (name[3:].upper(), mm_idx[name]))
        f.write("; 全体マップのスプライト（階段）は BG とは別に持つ\n")
        f.write("\n; 文字（黒地に白。パレット P2）\n")
        for chn in sorted(FONT5):
            label = {"/": "SLASH", " ": "SPACE"}.get(chn, chn)
            f.write("CH_%s = $%02X\n" % (label, font_idx[chn]))

    render_sheet().save(os.path.join(OUT, "dungeon_tiles.png"), scale=2)
    render_depths().save(os.path.join(OUT, "dungeon_depths.png"), scale=2)
    render_mock().save(os.path.join(OUT, "mock_screen.png"))
    render_minimap_sheet().save(os.path.join(OUT, "minimap_tiles.png"))
    cv, tiles, g, known = render_minimap_mock()
    cv.save(os.path.join(OUT, "mock_minimap.png"))

    print("metatiles: %d" % len(METATILES))
    print("8x8 tiles: %d (terrain %d, minimap %d, font %d)" % (len(chr_.tiles), n_terrain, n_mm, n_font))
    if "-v" in sys.argv:
        for y in range(DH):
            print("".join(g[y][x] if known[y][x] else " " for x in range(DW)))


if __name__ == "__main__":
    main()
