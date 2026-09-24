#!/usr/bin/env python3
"""ファミコン向けダンジョンタイルセットを作る。

参考画像（初代ゼルダの地上: 砂地と岩山）のやりかたをダンジョンに移す。
地形は 8x8 のチップを共通化し、並べかたを変えて 16x16 のバリエーションを
作る。NES の制約（8x8 タイル 1 枚 4 色、16x16 ごとに BG パレット 1 本、
BG パレット 4 本、共通の背景色 1 色）の中で組み立て、assets/famicom/ に
次のファイルを書き出す。

  dungeon_bg.chr        NES の 2bpp CHR（地形のチップ・見本・全体マップ・文字）
  dungeon_metatiles.inc ca65 の表（チップ番号、壁の 1/4 ごとの選びかた、床の模様、
                        そのほかのメタタイル）
  dungeon_chips.png     地形のチップ一覧（番号つき・3 倍）
  dungeon_tiles.png     チップを組んだ 16x16 の例（2 倍）
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
# こうすると壁のチップが自分のパレットのまま床の色を含められるので、
# 岩の縁を床に食いこませたり、岩のすき間から床をのぞかせたりできる。
# 黒は各パレットに 1 色ずつ持ち、壁の奥と未探索の闇をそれで塗る
# （壁の外が闇に溶けこむ）。
#
#   P0 影側  : 床 / 黒 / 岩 / 影（ピンク）
#   P1 光側  : 床 / 黒 / 岩 / 明かり（敷石の色）
#   P2 淡色  : 床 / 黒 / 岩 / 淡いクリーム … レンガの壁・石英・文字
#   P3 生き物: 床 / 黒 / 緑 / 明るい緑 … モンスターの既定・酸
#
# P0・P1・P2 は 1 番（黒）と 2 番（岩）が同じで、3 番だけが違う。
# 3 番に描いた点は、P0 のマスでは床に落ちた影（ピンク）に、P1 のマスでは
# 光の当たった縁や敷石（黄）に、P2 のマスでは淡い光（参考画像の薄いピンクに
# あたる色）に見える。同じチップを属性（パレット）の振りかたで描き分ける
# （参考画像の岩も、黒と茶は共通で 3 色目だけが砂・ピンク・薄いピンクと
# 入れかわっている）。
# ---------------------------------------------------------------------------
SCHEMES = [
    # 名前, 床, 岩, 影, 明かり
    ("B1-10", 0x36, 0x17, 0x25, 0x38),   # 砂岩: 参考画像の配色
    ("B11-20", 0x10, 0x18, 0x00, 0x37),  # 石灰岩: 灰の床に黄土色の岩
    ("B21-30", 0x3B, 0x1A, 0x2B, 0x3A),  # 苔: 薄緑の床に緑の岩
    ("B31-40", 0x31, 0x12, 0x21, 0x30),  # 氷: 水色の床に青い岩
    ("B41-", 0x06, 0x10, 0x07, 0x17),    # 火山: 赤黒い床に灰の岩
]


def palettes(scheme=0):
    _, bg, rock, sh, lt = SCHEMES[scheme]
    return {
        "P0": [bg, 0x0F, rock, sh],
        "P1": [bg, 0x0F, rock, lt],
        "P2": [bg, 0x0F, rock, 0x37],
        "P3": [bg, 0x0F, 0x1A, 0x2A],
    }

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
# 陰影の描きかた（全体の約束）
#
# 光は左上から。輪郭線は描かない。
#   - 左と上の縁: 輪郭を描かず、明るい色（3 番）の筋を入れて床に直接つなぐ
#   - 右と下の縁: 黒（または暗い色）の影を 1 ドット入れて強める
# ---------------------------------------------------------------------------
def shade_blobs(g, blobs, body=2, lit=3, dark=1):
    """楕円の塊を奥から順に描く。左上は明るく、右下に黒い影。"""
    h, w = len(g), len(g[0])
    for (cy, cx, ry, rx) in blobs:
        def inside(y, x):
            return ((y - cy) / ry) ** 2 + ((x - cx) / rx) ** 2 <= 1.0
        for y in range(h):
            for x in range(w):
                if not inside(y, x):
                    continue
                k = 1 - ((y - cy) / ry) ** 2
                left = cx - rx * max(k, 0) ** 0.5
                kk = 1 - ((x - cx) / rx) ** 2
                top = cy - ry * max(kk, 0) ** 0.5
                if not inside(y + 1, x) or not inside(y, x + 1) or not inside(y + 1, x + 1):
                    g[y][x] = dark
                elif x - left < 1.6 or (y - top < 1.0 and x < cx):
                    g[y][x] = lit
                else:
                    g[y][x] = body
    return g


def chip(fill=0):
    return [[fill] * 8 for _ in range(8)]


def chip_art(rows):
    assert len(rows) == 8, rows
    g = chip()
    for r, row in enumerate(rows):
        assert len(row) == 8, repr(row)
        for c, ch in enumerate(row):
            g[r][c] = 0 if ch == "." else int(ch)
    return g


def area_chip(floor_fn, blobs, **kw):
    """床の部分（floor_fn が真）は床の色、残りは黒。その上に岩を描く。"""
    g = [[0 if floor_fn(y, x) else 1 for x in range(8)] for y in range(8)]
    return shade_blobs(g, blobs, **kw)


# ---------------------------------------------------------------------------
# 8x8 のチップ
#
# 地形はすべて、このチップを 4 枚並べて作る（16x16 のメタタイル）。
# 同じチップをいろいろな場所・組み合わせで使いまわして、枚数を抑えつつ
# 見た目の変化を出す。チップごとに、どのパレットで使うかを添えておく
# （一覧画像の色づけにだけ使う。実際の色はメタタイルのパレットで決まる）。
# ---------------------------------------------------------------------------
CHIPS = {}


def add_chip(name, pal, g):
    assert name not in CHIPS, name
    for row in g:
        for v in row:
            assert v in (0, 1, 2, 3), (name, v)
    CHIPS[name] = (pal, g)


add_chip("blank", "P0", chip(0))  # 床の色だけ（0 番。どのパレットでも使える）
add_chip("black", "P1", chip(1))  # 黒だけ（P1・P2 の 1 番）。壁の奥・未探索の闇


# --- 岩の壁（P0/P1。石英の鉱脈は同じチップを P2 で使う） -------------------
# 0 床・1 黒（右下の影）・2 岩・3 床の側の点（P0 で影、P1 で明かり）。
# 左上は輪郭を描かず岩の色のまま床に接し、右下だけ黒で締める。
def _cast_shadow(g, right=False, bottom=False):
    """岩（黒を含む）の右・下に接する床の 1 ドットを 3 番にする（床に落ちる影）。"""
    out = [row[:] for row in g]
    for y in range(8):
        for x in range(8):
            if g[y][x] != 0:
                continue
            if (bottom and y > 0 and g[y - 1][x] != 0) or (right and x > 0 and g[y][x - 1] != 0):
                out[y][x] = 3
    return out


def rock_chip(floor_fn, blobs, lit, right=False, bottom=False):
    """岩のチップ。lit=3 なら左上の縁に 3 番の明かり、2 なら明かり無し。"""
    g = area_chip(floor_fn, blobs, body=2, lit=lit, dark=1)
    return _cast_shadow(g, right, bottom)


def _no_floor(y, x):
    return False


def _floor_top(y, x):
    return y < 3


def _floor_bottom(y, x):
    return y > 4


def _floor_left(y, x):
    return x < 3


def _floor_right(y, x):
    return x > 4


ROCK_SPECS = {
    # 名前: (床の側, 岩の塊, 明かり, 右に影, 下に影)
    # 奥（床に面していない 1/4）: 黒の中に岩の塊。3 番は岩の頭の小さな明かり
    "rock_in_a": (_no_floor, [(3.0, 2.8, 3.2, 2.9), (6.2, 6.4, 2.6, 2.4)], 3, 0, 0),
    "rock_in_b": (_no_floor, [(2.4, 5.6, 2.8, 2.6), (5.8, 2.4, 2.8, 2.6)], 3, 0, 0),
    "rock_in_c": (_no_floor, [(3.6, 3.6, 3.6, 3.4)], 3, 0, 0),
    "rock_in_d": (_no_floor, [(1.6, 1.6, 2.2, 2.2), (4.6, 5.0, 3.2, 3.0)], 3, 0, 0),
    "rock_in_e": (_no_floor, [(2.2, 3.8, 2.4, 3.6), (6.0, 1.8, 2.2, 2.0), (6.2, 6.0, 2.0, 2.2)], 3, 0, 0),
    "rock_in_f": (_no_floor, [(4.0, 2.2, 3.8, 2.2), (3.2, 6.2, 3.0, 1.8)], 3, 0, 0),
    # 上・左が床: 光の当たる側。左上の縁に明かり
    "rock_top_a": (_floor_top, [(3.3, 3.6, 3.4, 3.9)], 3, 0, 0),
    "rock_top_b": (_floor_top, [(3.0, 1.9, 3.1, 2.5), (3.9, 5.6, 3.1, 2.4)], 3, 0, 0),
    "rock_left_a": (_floor_left, [(3.6, 3.4, 3.9, 3.2)], 3, 0, 0),
    "rock_left_b": (_floor_left, [(1.9, 3.0, 2.4, 3.1), (5.6, 3.6, 2.4, 3.0)], 3, 0, 0),
    # 下・右が床: 影の側。明かりは入れず、外の床に影を落とす
    "rock_bottom_a": (_floor_bottom, [(3.6, 3.6, 3.1, 3.9)], 2, 0, 1),
    "rock_bottom_b": (_floor_bottom, [(4.0, 2.0, 2.6, 2.5), (3.4, 5.6, 3.2, 2.4)], 2, 0, 1),
    "rock_right_a": (_floor_right, [(3.6, 3.4, 3.9, 3.2)], 2, 1, 0),
    "rock_right_b": (_floor_right, [(2.0, 3.8, 2.4, 2.9), (5.6, 3.2, 2.4, 3.1)], 2, 1, 0),
    # 2 辺が床の角
    "rock_ctl": (lambda y, x: y < 3 or x < 3, [(4.0, 4.0, 3.4, 3.4)], 3, 0, 0),
    "rock_ctr": (lambda y, x: y < 3 or x > 4, [(4.0, 3.4, 3.4, 3.4)], 3, 1, 0),
    "rock_cbl": (lambda y, x: y > 4 or x < 3, [(3.4, 4.0, 3.4, 3.4)], 3, 0, 1),
    "rock_cbr": (lambda y, x: y > 4 or x > 4, [(3.2, 3.2, 3.1, 3.1)], 2, 1, 1),
}
for _name, (_fn, _blobs, _lit, _r, _b) in ROCK_SPECS.items():
    add_chip(_name, "P1", rock_chip(_fn, _blobs, _lit, _r, _b))

# 宝を含む鉱脈: 上・下の縁の岩に 3 番の粒（P1 で金、P2 で白い宝石）
_GEM = [(2, 5), (3, 5), (3, 6), (4, 2), (5, 2), (5, 3), (2, 2)]


def _gem(g):
    g = [row[:] for row in g]
    for (y, x) in _GEM:
        if g[y][x] == 2:
            g[y][x] = 3
    return g


add_chip("gem_top", "P1", _gem(CHIPS["rock_top_a"][1]))
add_chip("gem_bottom", "P1", _gem(CHIPS["rock_bottom_a"][1]))
ROCK_INSIDE = ["rock_in_a", "rock_in_b", "rock_in_c", "rock_in_d", "rock_in_e", "rock_in_f",
               "rock_in_a", "rock_in_c"]  # 3 ビットで引く（a と c は出やすい）


# --- レンガの壁（P2: 床・黒・灰・白）。部屋のまわりに使う ------------------
def brick_face(joints, crack=False, lit_left=False, dark_right=False):
    """正面のレンガ 2 段。上の縁と左の縁が明るく、目地（右と下）が黒。"""
    g = chip(2)
    for band, j in enumerate(joints):
        y0 = band * 4
        for x in range(8):
            g[y0][x] = 3
            g[y0 + 3][x] = 1
        for y in range(y0, y0 + 3):
            g[y][j] = 1
            g[y][(j + 1) % 8] = 3
    if crack:
        for (y, x) in [(1, 2), (2, 3), (5, 5), (6, 5)]:
            g[y][x] = 1
    if lit_left:
        for y in range(8):
            if g[y][0] != 1:
                g[y][0] = 3
    if dark_right:
        for y in range(8):
            g[y][7] = 1
    return g


add_chip("brick_face_a", "P2", brick_face((7, 3)))
add_chip("brick_face_b", "P2", brick_face((3, 7)))
add_chip("brick_face_crack", "P2", brick_face((7, 3), crack=True))
add_chip("brick_cbl", "P2", brick_face((7, 3), lit_left=True))
add_chip("brick_cbr", "P2", brick_face((3, 7), dark_right=True))


def brick_cap(top=False, left=False, right=False):
    """床に面した壁の上端・横の縁（4 ドット幅のレンガ）。奥は黒。"""
    g = chip(1)
    if top:
        for x in range(8):
            g[0][x] = 3
            g[1][x] = 2
            g[2][x] = 2
            g[3][x] = 1
        g[1][3] = g[2][3] = 1
        g[0][4] = 3
    if left:
        for y in range(8):
            g[y][0] = 3
            g[y][1] = 2
            g[y][2] = 2
            g[y][3] = 1
            if y % 4 == 3:
                g[y][0] = g[y][1] = g[y][2] = 1
    if right:
        for y in range(8):
            g[y][4] = 3
            g[y][5] = 2
            g[y][6] = 2
            g[y][7] = 1
            if y % 4 == 3 and not (top and y < 4):
                g[y][4] = g[y][5] = g[y][6] = 1
        if top:
            for x in range(4, 8):
                g[0][x] = 3
    return g


add_chip("brick_cap_top", "P2", brick_cap(top=True))
add_chip("brick_cap_left", "P2", brick_cap(left=True))
add_chip("brick_cap_right", "P2", brick_cap(right=True))
add_chip("brick_ctl", "P2", brick_cap(top=True, left=True))
add_chip("brick_ctr", "P2", brick_cap(top=True, right=True))


# --- 床（P0: 床・影・明るい床・黒） -----------------------------------------
add_chip("ripple_a", "P0", chip_art([
    "........",
    ".11.....",
    "...111..",
    "........",
    "........",
    "....11..",
    "......11",
    "........",
]))
add_chip("ripple_b", "P0", chip_art([
    "........",
    ".....11.",
    "........",
    "........",
    "11......",
    "..111...",
    "........",
    "........",
]))
# 敷石: 明るい面、右と下に影、すき間は床の色
add_chip("tile_big", "P0", chip_art([
    "2222222.",
    "2222222.",
    "2222222.",
    "2222222.",
    "2222222.",
    "2222222.",
    "2222222.",
    "........",
]))
add_chip("tile_small", "P0", chip_art([
    "222.222.",
    "222.222.",
    "222.222.",
    "........",
    "222.222.",
    "222.222.",
    "222.222.",
    "........",
]))
add_chip("tile_crack", "P0", chip_art([
    "2222222.",
    "2212222.",
    "2221222.",
    "2222122.",
    "2222212.",
    "2222222.",
    "2222222.",
    "........",
]))
add_chip("tile_worn", "P0", chip_art([
    "2222222.",
    "222222..",
    "22222...",
    "2222....",
    "222.....",
    "22....11",
    "........",
    "........",
]))
add_chip("tile_brick", "P0", chip_art([
    "222.2222",
    "222.2222",
    "........",
    "2.222222",
    "2.222222",
    "........",
    "222.2222",
    "222.2222",
]))
# 壁の右・下に落ちる影（ピンク）
add_chip("shadow_top", "P0", chip_art([
    "11111111",
    "11111111",
    "1.11.111",
    "........",
    "........",
    "........",
    "........",
    "........",
]))
add_chip("shadow_left", "P0", chip_art([
    "11......",
    "111.....",
    "11......",
    "1.......",
    "11......",
    "111.....",
    "11......",
    "1.......",
]))
add_chip("shadow_tl", "P0", chip_art([
    "11111111",
    "11111111",
    "111.1111",
    "11......",
    "1.......",
    "11......",
    "111.....",
    "11......",
]))
add_chip("shadow_dot", "P0", chip_art([
    "111.....",
    "11......",
    "1.......",
    "........",
    "........",
    "........",
    "........",
    "........",
]))


# --- 岩のかけら（P1）: 瓦礫・小石・落石 --------------------------------------
add_chip("boulder_a", "P1", shade_blobs(chip(), [(4.0, 3.6, 3.2, 3.3)]))
add_chip("boulder_b", "P1", shade_blobs(chip(), [(4.6, 4.0, 2.6, 3.4), (2.6, 2.6, 1.9, 2.0)]))
add_chip("pebble", "P1", shade_blobs(chip(), [(5.0, 4.0, 1.6, 2.1)]))


# --- 扉（P1）。縦の壁も横の壁も同じ絵 ---------------------------------------
add_chip("door_l", "P1", chip_art([
    "32322321",
    "32322321",
    "32322321",
    "11111111",
    "32322321",
    "32322321",
    "32322321",
    "32322321",
]))
add_chip("door_r", "P1", chip_art([
    "32232321",
    "32232321",
    "32232321",
    "11111111",
    "32232321",
    "32232321",
    "32232321",
    "32232321",
]))
add_chip("door_open_l", "P1", chip_art([
    "32321111",
    "32321111",
    "32321111",
    "32311111",
    "32321111",
    "32321111",
    "32321111",
    "32321111",
]))
add_chip("door_open_r", "P1", chip_art([
    "11111321",
    "11111321",
    "11111321",
    "11111321",
    "11111321",
    "11111321",
    "11111321",
    "11111321",
]))


# --- 階段（P0）: 上半分は共通、下り階段は下半分が闇に沈む ---------------------
add_chip("steps_l", "P0", chip_art([
    "22222222",
    "22222222",
    "11111111",
    "33333333",
    "22222222",
    "22222222",
    "11111111",
    "33333333",
]))
add_chip("steps_r", "P0", chip_art([
    "22222223",
    "22222223",
    "11111113",
    "33333333",
    "22222223",
    "22222223",
    "11111113",
    "33333333",
]))
add_chip("steps_dark_l", "P0", chip_art([
    "11111111",
    "11111111",
    "33333333",
    "33333333",
    "11111111",
    "33333333",
    "33333333",
    "33333333",
]))
add_chip("steps_dark_r", "P0", chip_art([
    "11111113",
    "11111113",
    "33333333",
    "33333333",
    "11111333",
    "33333333",
    "33333333",
    "33333333",
]))


# --- 16x16 の円を 4 枚に割るもの（落とし穴・ルーン） -------------------------
def split4(g16, prefix, pal):
    for i, q in enumerate(("tl", "tr", "bl", "br")):
        y0, x0 = (i // 2) * 8, (i % 2) * 8
        add_chip("%s_%s" % (prefix, q), pal, [row[x0:x0 + 8] for row in g16[y0:y0 + 8]])


def pit16():
    """穴。中は黒、光の当たる右下の内壁だけ明るい。縁に輪郭は描かない。"""
    g = blank()
    for y in range(16):
        for x in range(16):
            d = ((y - 7.5) ** 2 + (x - 7.5) ** 2) ** 0.5
            if d <= 6.6:
                d2 = ((y - 5.6) ** 2 + (x - 5.6) ** 2) ** 0.5
                g[y][x] = 2 if d2 > 6.4 else (1 if d2 > 5.4 else 3)
    return g


def ring16():
    """魔法陣。輪は 2 番、中の星形は 1 番（P0 ではピンク、P1 では黒）。"""
    g = blank()
    for y in range(16):
        for x in range(16):
            d = ((y - 7.5) ** 2 + (x - 7.5) ** 2) ** 0.5
            if 5.6 <= d <= 6.7:
                g[y][x] = 2
    for i in range(3, 13):
        g[7][i] = 1
        g[i][7] = 1
    for i in range(-3, 4):
        g[7 + i][7 + i] = 1 if abs(i) < 3 else g[7 + i][7 + i]
        g[7 + i][7 - i] = 1 if abs(i) < 3 else g[7 + i][7 - i]
    return g


split4(pit16(), "pit", "P0")
split4(ring16(), "ring", "P0")


# --- 1 枚を 4 回並べて使う罠 ---------------------------------------------------
add_chip("planks", "P1", chip_art([
    "33333331",
    "22222221",
    "22222221",
    "11111111",
    "33333331",
    "22222221",
    "22222221",
    "11111111",
]))
add_chip("grate", "P0", chip_art([
    "22222221",
    "23333331",
    "22222221",
    "23333331",
    "22222221",
    "23333331",
    "22222221",
    "11111111",
]))
add_chip("plate", "P0", chip_art([
    "2222222.",
    "2222222.",
    "2233222.",
    "2231222.",
    "2222222.",
    "2222222.",
    "1111111.",
    "........",
]))
add_chip("scorch", "P0", chip_art([
    "........",
    "..3.3...",
    ".33331..",
    "3333331.",
    ".333311.",
    "..3.11..",
    "........",
    "........",
]))
add_chip("acid", "P3", shade_blobs(chip(), [(4.5, 4.0, 2.5, 3.4)]))
CHIPS["acid"][1][3][3] = 3


# ---------------------------------------------------------------------------
# メタタイル（16x16）の組みたて
#
# 壁は「どの辺が床に面しているか」（4 ビット）から、1/4 ごとにチップを
# 選ぶ。1/4 に効くのはその角をはさむ 2 辺だけなので、表は
# 「1/4 の位置 × 2 辺の状態（4 通り）」で済む。
# ---------------------------------------------------------------------------
# --- 第 3 版のチップを新しいパレットの並びに合わせて塗りかえる -------------
# 旧 P0（床・影・明かり・黒）で描いたもの、旧 P1 と同じ並びのものを、
# 新しい P0（床・黒・岩・影）/ P1（床・黒・岩・明かり）に合わせる。
_REMAP = {
    # 名前: (パレット, {旧の番号: 新しい番号})
    "ripple_a": ("P1", {1: 3}), "ripple_b": ("P1", {1: 3}),
    "tile_big": ("P1", {2: 3}), "tile_small": ("P1", {2: 3}),
    "tile_crack": ("P1", {1: 2, 2: 3}), "tile_worn": ("P1", {1: 2, 2: 3}),
    "tile_brick": ("P1", {2: 3}),
    "shadow_top": ("P0", {1: 3}), "shadow_left": ("P0", {1: 3}),
    "shadow_tl": ("P0", {1: 3}), "shadow_dot": ("P0", {1: 3}),
    "steps_l": ("P1", {2: 3, 1: 2, 3: 1}), "steps_r": ("P1", {2: 3, 1: 2, 3: 1}),
    "steps_dark_l": ("P1", {1: 2, 3: 1}), "steps_dark_r": ("P1", {1: 2, 3: 1}),
    "grate": ("P1", {2: 3, 3: 1, 1: 2}), "plate": ("P1", {2: 3, 3: 1, 1: 2}),
    "scorch": ("P0", {3: 1, 1: 3}),
}
for _q in ("tl", "tr", "bl", "br"):
    _REMAP["pit_" + _q] = ("P0", {3: 1, 1: 3})
    _REMAP["ring_" + _q] = ("P0", {1: 3})  # 輪は岩の色、中の星が 3 番
for _name, (_pal, _table) in _REMAP.items():
    CHIPS[_name] = (_pal, [[_table.get(v, v) for v in row] for row in CHIPS[_name][1]])


N, E, S, W = 1, 2, 4, 8
QUADS = ("tl", "tr", "bl", "br")
# 1/4 ごとに見る 2 辺（縦の辺, 横の辺）
QUAD_SIDES = {"tl": (N, W), "tr": (N, E), "bl": (S, W), "br": (S, E)}
QUAD_V = {"tl": "top", "tr": "top", "bl": "bottom", "br": "bottom"}
QUAD_H = {"tl": "left", "tr": "right", "bl": "left", "br": "right"}

WALL_FAMILIES = {
    # 名前: 説明（パレットは wall_palette() で決める）
    "rock": "岩の壁（花崗岩・溶岩）。通路のまわり。外周の壁と隠し扉も同じ",
    "brick": "レンガの壁。部屋のまわり（P2）",
    "quartz": "石英の鉱脈（岩のチップを P2 で）",
    "gem": "宝を含む鉱脈（上下の縁を宝のチップに）",
}


def wall_quad(family, quad, mask, var, inner=False):
    """壁の 1/4 に使うチップの名前。

    var は位置から決めた値。岩では 1/4 ごとに 3 ビット（左上が下位）を取り出し、
    奥の岩の 6 種と縁の模様 A/B を 1/4 ごとに別々に選ぶ。レンガでは 0/1 が
    目地の模様、3 がひび。
    inner は、辺では床に面していないが斜めの隣が床のマス（岩山の奥の段）。
    """
    sv, sh = QUAD_SIDES[quad]
    v, h = bool(mask & sv), bool(mask & sh)
    if family == "brick":
        ab = "ab"[(var + (quad in ("tr", "bl"))) % 2]
        if not v and not h:
            return "black"
        if v and h:
            return "brick_c" + quad
        if v and QUAD_V[quad] == "top":
            return "brick_cap_top"
        if v:
            return ("brick_face_crack" if var == 3 else "brick_face_" + ab)
        return "brick_cap_" + QUAD_H[quad]
    sub = (var >> (3 * QUADS.index(quad))) & 7
    ab = "ab"[sub & 1]
    if not v and not h:
        if mask == 0 and not inner:
            return "black"
        return ROCK_INSIDE[sub]
    if v and h:
        return "rock_c" + quad
    if v:
        side = QUAD_V[quad]
        if family == "gem" and quad in ("tl", "br"):
            return "gem_" + side
        return "rock_%s_%s" % (side, ab)
    return "rock_%s_%s" % (QUAD_H[quad], ab)


def wall_palette(family, mask, var):
    """壁のマスのパレット（属性）。

    岩は、右か下が床なら影側の P0（床に落ちる影がピンクになる）。そうでなければ
    位置（var の上位ビット）で P0・P1・P2 を振り、同じチップの明かりの色を
    ピンク・黄・淡いクリームに描き分ける。レンガと石英は P2。
    """
    if family in ("brick", "quartz"):
        return "P2"
    if mask & (S | E):
        return "P0"
    return ("P0", "P1", "P2")[(var >> 12) % 3]


def wall_metatile(family, mask, var=0, inner=False):
    return [wall_quad(family, q, mask, var, inner) for q in QUADS]

SAND = [
    ["blank", "blank", "blank", "blank"],
    ["ripple_a", "blank", "blank", "ripple_b"],
    ["blank", "ripple_b", "ripple_a", "blank"],
    ["ripple_b", "blank", "blank", "blank"],
    ["blank", "blank", "ripple_a", "blank"],
]
PAVED = [
    ["tile_big", "tile_big", "tile_big", "tile_big"],
    ["tile_small", "tile_small", "tile_small", "tile_small"],
    ["tile_big", "tile_crack", "tile_big", "tile_big"],
    ["tile_big", "tile_big", "tile_worn", "tile_big"],
    ["tile_big", "tile_small", "tile_small", "tile_big"],
    ["tile_brick", "tile_brick", "tile_brick", "tile_brick"],
    ["tile_worn", "tile_big", "tile_big", "tile_small"],
]
PEBBLE = ["blank", "blank", "blank", "pebble"]


def floor_metatile(y, x, room, wall_n, wall_w, wall_nw):
    """床のマス: 位置で模様を選び、上・左の壁の影を重ねる。"""
    shadow = wall_n or wall_w or wall_nw
    if room:
        base = PAVED[noise(x * 31 + y * 17, 9, 0, len(PAVED) - 1)]
    elif not shadow and noise(x * 13 + y * 29, 4, 0, 5) == 0:
        return ("P0" if (x + y) % 2 else "P1"), PEBBLE
    else:
        base = SAND[noise(x * 31 + y * 17, 9, 0, len(SAND) - 1)]
    if shadow:
        # 影のマスは P0 になり、3 番（さざ波・敷石）がピンクに化けるので無地にする。
        # 部屋では壁ぎわに敷石の無い帯ができる
        base = SAND[0]
    q = list(base)
    if wall_n and wall_w:
        q[0] = "shadow_tl"
    elif wall_n:
        q[0] = "shadow_top"
    elif wall_w:
        q[0] = "shadow_left"
    elif wall_nw:
        q[0] = "shadow_dot"
    if wall_n:
        q[1] = "shadow_top"
    if wall_w:
        q[2] = "shadow_left"
    # 影の落ちたマスは P0（3 番がピンクの影）。敷石は影の色に沈む。
    # 影の無いマスは P1（3 番が明かり）で、敷石とさざ波が明るい色になる
    return ("P0" if shadow else "P1"), q


FIXED = {
    # 名前: パレット, 4 枚のチップ, 説明
    "void": ("P1", ["black"] * 4, "未探索の闇（壁の奥と同じ黒）"),
    "door_closed": ("P1", ["door_l", "door_r", "door_l", "door_r"], "閉じた扉（縦横どちらの壁でも同じ）"),
    "door_open": ("P1", ["door_open_l", "door_open_r", "door_open_l", "door_open_r"], "開いた扉・壊れた扉"),
    "stairs_up": ("P1", ["steps_l", "steps_r", "steps_l", "steps_r"], "上り階段 <"),
    "stairs_down": ("P1", ["steps_l", "steps_r", "steps_dark_l", "steps_dark_r"], "下り階段 >（下半分が闇）"),
    "rubble": ("P1", ["boulder_b", "boulder_a", "boulder_a", "boulder_b"], "瓦礫 :"),
    "trap_pit": ("P0", ["pit_tl", "pit_tr", "pit_bl", "pit_br"], "落とし穴"),
    "trap_door": ("P1", ["planks"] * 4, "落とし戸"),
    "trap_dart": ("P1", ["plate"] * 4, "矢・吹き矢の罠"),
    "trap_gas": ("P1", ["grate"] * 4, "ガスの罠"),
    "trap_rune": ("P0", ["ring_tl", "ring_tr", "ring_bl", "ring_br"], "不思議なルーン（岩の色の輪にピンクの星）"),
    "glyph": ("P1", ["ring_tl", "ring_tr", "ring_bl", "ring_br"], "守りのルーン（同じ絵で星が黄）"),
    "trap_fire": ("P0", ["scorch", "blank", "blank", "scorch"], "焦げ跡（火炎の罠）"),
    "trap_rock": ("P1", ["pebble", "boulder_a", "blank", "pebble"], "落石の罠"),
    "trap_acid": ("P3", ["blank", "acid", "acid", "blank"], "酸の罠"),
}


def compose(quads):
    g = blank()
    for i, name in enumerate(quads):
        y0, x0 = (i // 2) * 8, (i % 2) * 8
        c = CHIPS[name][1]
        for y in range(8):
            for x in range(8):
                g[y0 + y][x0 + x] = c[y][x]
    return g


# ---------------------------------------------------------------------------
# モンスター・アイテムの見本（地形のチップ枠の外。BG で描き、背景は床の色）
# ---------------------------------------------------------------------------
def jelly():
    g = shade_blobs(blank(), [(9.5, 7.5, 5.5, 6.8)])
    for (y, x) in [(8, 5), (8, 9)]:
        g[y][x] = 3
        g[y][x + 1] = 3
        g[y + 1][x] = 3
        g[y + 1][x + 1] = 1
    return g


def gold_pile():
    g = blank()
    for (cy, cx) in [(7, 7), (9, 4.5), (9, 10), (11, 7.5), (12, 3.5), (12.5, 11.5)]:
        shade_blobs(g, [(cy + 0.6, cx, 2.0, 3.0)], body=2, lit=2, dark=1)
        shade_blobs(g, [(cy, cx, 1.5, 2.6)], body=3, lit=3, dark=2)
    return g


def potion():
    g = blank()
    shade_blobs(g, [(10.0, 7.5, 4.5, 4.8)], body=2, lit=3, dark=1)
    for y in range(3, 7):
        g[y][7] = 3
        g[y][8] = 2
        g[y][9] = 1
    for x in range(6, 10):
        g[2][x] = 2
    g[2][9] = 1
    return g


SAMPLES = {
    "ex_jelly": ("P3", jelly(), "見本: ゼリー J"),
    "ex_gold": ("P1", gold_pile(), "見本: 金貨の山 $（這う硬貨と共用）"),
    "ex_potion": ("P2", potion(), "見本: 薬瓶 !"),
}


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
# 溶岩の鉱脈は原典どおり花崗岩と同じ見た目（highlight_seams を切った既定の表示）
VEIN_CH = {"m": "rock", "M": "gem", "q": "quartz", "Q": "gem"}
ROOM_CH = set(".<>@*!j^W")
FIXED_CH = {"<": "stairs_up", ">": "stairs_down", ":": "rubble", "^": "trap_pit",
            "T": "trap_door", "W": "glyph", "+": "door_closed", "'": "door_open"}
SAMPLE_CH = {"j": "ex_jelly", "*": "ex_gold", "!": "ex_potion"}


def cell(rows, y, x):
    if 0 <= y < len(rows) and 0 <= x < len(rows[0]):
        return rows[y][x]
    return "#"


def is_wall(ch):
    return ch == "#" or ch in VEIN_CH


def is_open(ch):
    """床として扱うマス（壁でも未探索でもない）。"""
    return not is_wall(ch) and ch != " "


def cell_metatile(rows, y, x):
    """マスに置くもの: (パレット, 16x16 の絵)。"""
    ch = rows[y][x]
    if is_wall(ch):
        m = 0
        m |= N if is_open(cell(rows, y - 1, x)) else 0
        m |= S if is_open(cell(rows, y + 1, x)) else 0
        m |= W if is_open(cell(rows, y, x - 1)) else 0
        m |= E if is_open(cell(rows, y, x + 1)) else 0
        if ch in VEIN_CH:
            family = VEIN_CH[ch]
        else:
            near_room = any(cell(rows, y + dy, x + dx) in ROOM_CH
                            for dy in (-1, 0, 1) for dx in (-1, 0, 1))
            family = "brick" if near_room else "rock"
        inner = any(is_open(cell(rows, y + dy, x + dx)) for dy in (-1, 1) for dx in (-1, 1))
        var = rock_var(y, x)
        if family == "brick":
            var = (x + y) % 2
            if noise(x * 7 + y * 5, 2, 0, 4) == 0:
                var = 3
        pal = wall_palette(family, m, var)
        if ch == "Q":
            pal = "P2"
        return pal, compose(wall_metatile(family, m, var, inner))
    if ch == " ":
        pal, quads, _ = FIXED["void"]
        return pal, compose(quads)
    if ch in FIXED_CH:
        pal, quads, _ = FIXED[FIXED_CH[ch]]
        return pal, compose(quads)
    if ch in SAMPLE_CH:
        pal, g, _ = SAMPLES[SAMPLE_CH[ch]]
        return pal, g
    pal, quads = floor_metatile(y, x, ch in ROOM_CH, is_wall(cell(rows, y - 1, x)),
                                is_wall(cell(rows, y, x - 1)), is_wall(cell(rows, y - 1, x - 1)))
    return pal, compose(quads)


def draw_map(cv, rows, pals, x0=0, y0=0, player=True):
    for y, row in enumerate(rows):
        assert len(row) == len(rows[0]), row
        for x in range(len(row)):
            pal, g = cell_metatile(rows, y, x)
            cv.draw(g, x0 + x * 16, y0 + y * 16, pals[pal])
            if player and rows[y][x] == "@":
                cv.draw(from_ascii(PLAYER), x0 + x * 16, y0 + y * 16, SPRITE_PAL, transparent0=True)


def render_mock(scheme=0):
    pals = palettes(scheme)
    cv = Canvas(256, 240)
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


# 参考画像の岩山の配置（16x16 のマス単位に読みとったもの。, は砂地）
ROCK_DEMO = [
    "#######qq#,#####",
    "###,,,,,,,,,,###",
    "##,,,,,,,,,,,,##",
    "#,,,,,,,,,,,,,##",
    "#,,,,,,,,,,,,,##",
    "#,,,,,,,,,,,,,#,",
    "#,,,,,,,,,,,,,,#",
    "##,,,,,,,,,,,,,,",
    "#q###,,,,,,,,,,,",
    "#qq###,,,,,,,,,,",
]


def rock_var(y, x):
    """岩のマスの模様と属性を決める値（位置から計算。1/4 ごとに 3 ビット＋属性）。"""
    return noise(x * 37 + y * 101, 7, 0, 1008) * 47 % 49152


def rock_stats(rows):
    """岩のマスに使ったチップ・色つきの 8x8・16x16 の種類を数える。"""
    chips, colored, blocks, cells = set(), set(), set(), 0
    for y, row in enumerate(rows):
        for x, ch in enumerate(row):
            if ch not in "#q":
                continue
            m = 0
            m |= N if is_open(cell(rows, y - 1, x)) else 0
            m |= S if is_open(cell(rows, y + 1, x)) else 0
            m |= W if is_open(cell(rows, y, x - 1)) else 0
            m |= E if is_open(cell(rows, y, x + 1)) else 0
            inner = any(is_open(cell(rows, y + dy, x + dx)) for dy in (-1, 1) for dx in (-1, 1))
            var = rock_var(y, x)
            family = "quartz" if ch == "q" else "rock"
            quads = wall_metatile(family, m, var, inner)
            pal = wall_palette(family, m, var)
            cells += 1
            chips.update(quads)
            # 黒だけのチップは、どのパレットでも同じ色になる
            colored.update((q, "-" if q == "black" else pal) for q in quads)
            blocks.add((tuple(quads), pal))
    return cells, chips, colored, blocks


def render_rock_demo():
    cv = Canvas(256, len(ROCK_DEMO) * 16)
    draw_map(cv, ROCK_DEMO, palettes(0))
    return cv


# ---------------------------------------------------------------------------
# 一覧画像
# ---------------------------------------------------------------------------
SHEET_BG = 0x404040


def render_chips():
    """チップ一覧。番号は CHR のタイル番号。"""
    cols = 12
    cw, ch = 24, 20
    names = list(CHIPS)
    cv = Canvas(cols * cw + 4, ((len(names) + cols - 1) // cols) * ch + 4, bg=SHEET_BG)
    pals = palettes(0)
    for i, name in enumerate(names):
        pal, g = CHIPS[name]
        x = 4 + (i % cols) * cw
        y = 2 + (i // cols) * ch
        cv.draw(g, x + 4, y, pals[pal])
        text(cv, "%02d" % i, x, y + 9, MM_TEXT_PAL)
    return cv


def sheet_rows():
    """組みたて例の行: [(パレット, 16x16), ...]。"""
    rows = []
    for family in ("rock", "brick"):
        rows.append([(wall_palette(family, m, 0), compose(wall_metatile(family, m, 0, True)))
                     for m in range(16)])
    # 同じチップの組みを P0（影側）と P1（光側）で描き分けた例
    rows.append([(pal, compose(wall_metatile("rock", m, v, True)))
                 for m, v in ((0, 0), (0, 0o1234), (0, 0o4321), (0, 0o5501), (N, 0o1111), (W, 0o1111))
                 for pal in ("P0", "P1", "P2")])
    veins = []
    for family, pal in (("quartz", "P2"), ("gem", "P1"), ("gem", "P2")):
        for m in (S, N, S | W, N | E):
            veins.append((pal, compose(wall_metatile(family, m, 0, True))))
    rows.append(veins)
    rows.append([("P0", compose(q)) for q in SAND] + [("P0", compose(PEBBLE)), ("P1", compose(PEBBLE))] +
                [("P1", compose(q)) for q in PAVED])
    shadows = []
    for (n, w, nw) in ((1, 0, 0), (0, 1, 0), (1, 1, 0), (0, 0, 1)):
        for room in (False, True):
            pal, q = floor_metatile(3, 4, room, n, w, nw)
            shadows.append((pal, compose(q)))
    rows.append(shadows + [(SAMPLES[k][0], SAMPLES[k][1]) for k in SAMPLES])
    rows.append([(FIXED[k][0], compose(FIXED[k][1])) for k in FIXED])
    return rows


def render_sheet():
    rows = sheet_rows()
    cols = max(len(r) for r in rows)
    cv = Canvas(cols * 18 + 4, len(rows) * 20 + 4, bg=SHEET_BG)
    pals = palettes(0)
    for j, row in enumerate(rows):
        for i, (pal, g) in enumerate(row):
            cv.draw(g, 3 + i * 18, 3 + j * 20, pals[pal])
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
    chip_idx = {}
    for name, (pal, g) in CHIPS.items():
        chip_idx[name] = chr_.add(g)
    n_terrain = len(chr_.tiles)
    assert n_terrain == len(CHIPS), "チップに同じ絵が 2 つある"
    sample_idx = {}
    for name, (pal, g, note) in SAMPLES.items():
        check_colors(g, name)
        sample_idx[name] = [chr_.add(q) for q in split16(g)]
    n_samples = len(chr_.tiles) - n_terrain
    mm_idx = {}
    for name, g in MINIMAP:
        check_colors(g, name)
        mm_idx[name] = chr_.add(g)
    n_mm = len(chr_.tiles) - n_terrain - n_samples
    font_idx = {}
    for chn in sorted(FONT5):
        font_idx[chn] = chr_.add(glyph8(chn))
    n_font = len(chr_.tiles) - n_terrain - n_samples - n_mm
    assert len(chr_.tiles) <= 256, len(chr_.tiles)

    with open(os.path.join(OUT, "dungeon_bg.chr"), "wb") as f:
        f.write(chr_.bytes())

    def q4(quads):
        return ",".join("CHIP_%s" % q.upper() for q in quads)

    with open(os.path.join(OUT, "dungeon_metatiles.inc"), "w", encoding="utf-8") as f:
        f.write("; scripts/famicom/make_dungeon_tiles.py が生成。手で直さないこと\n")
        f.write("; パレット（深さ帯ごと。各行の先頭が共通の背景色＝床）:\n")
        for j, sch in enumerate(SCHEMES):
            p = palettes(j)
            f.write(";   %-7s " % sch[0] + "  ".join(
                "%s=%s" % (k, ",".join("$%02X" % c for c in v)) for k, v in p.items()) + "\n")
        f.write("\n; --- 地形のチップ（CHR のタイル番号） ---\n")
        for name, i in chip_idx.items():
            f.write("CHIP_%s = $%02X\n" % (name.upper(), i))

        f.write("\n; --- 壁: 1/4 ごとのチップ ---\n")
        f.write("; wall_<種類>: 左上・右上・左下・右下の順に 4 組。各組は 2 辺の状態\n")
        f.write(";   0 どちらも床に面していない / 1 左右の辺だけ（左上なら左）/ 2 上下の辺だけ（左上なら上）/ 3 両方\n")
        f.write("; を模様 8 通り（計 32 バイト）。模様は位置から 1/4 ごとに 3 ビットずつ決める\n")
        f.write("; 状態 0 は、マスが斜めにだけ床に接していれば奥の岩（ROCK_IN_A〜D を位置で）、\n")
        f.write("; どこにも接していなければ黒。表には斜めに接している場合を載せる\n")
        f.write("; パレット: 岩は右か下が床なら P0（影側）、それ以外は位置で P0/P1/P2 を振る。\n")
        f.write(";           レンガと石英は P2\n")
        for family, note in WALL_FAMILIES.items():
            f.write("\n; %s\n" % note)
            f.write("wall_%s:\n" % family)
            for q in QUADS:
                sv, sh = QUAD_SIDES[q]
                vals = []
                for state in range(4):
                    mask = (sh if state & 1 else 0) | (sv if state & 2 else 0)
                    if family == "brick":
                        vars_ = (0, 1, 0, 1)
                    else:
                        vars_ = [sub << (3 * QUADS.index(q)) for sub in range(8)]
                    for var in vars_:
                        vals.append("CHIP_" + wall_quad(family, q, mask, var, True).upper())
                f.write("    .byte %s  ; %s\n" % (",".join(vals), q))
        f.write("brick_face_crack = CHIP_BRICK_FACE_CRACK  ; レンガの正面は時々ひびに差しかえる\n")

        f.write("\n; --- 床: 模様（位置で選ぶ）。影は左上・右上・左下の 1/4 を差しかえる ---\n")
        f.write("; 上が壁: 左上と右上を SHADOW_TOP / 左が壁: 左上と左下を SHADOW_LEFT\n")
        f.write("; 上と左が壁: 左上を SHADOW_TL / 左上だけ壁: 左上を SHADOW_DOT\n")
        f.write("floor_sand:   ; P1（影のマスは P0 で無地）\n")
        for q in SAND:
            f.write("    .byte %s\n" % q4(q))
        f.write("floor_paved:  ; P1（部屋。影の落ちたマスは P0 で敷石ごと影の色に）\n")
        for q in PAVED:
            f.write("    .byte %s\n" % q4(q))
        f.write("floor_pebble: ; P0/P1 を位置で（影の無い通路の床に時々）\n    .byte %s\n" % q4(PEBBLE))

        f.write("\n; --- そのほかのメタタイル（左上・右上・左下・右下, パレット） ---\n")
        for name, (pal, quads, note) in FIXED.items():
            f.write("mt_%s: .byte %s, %d  ; %s\n" % (name, q4(quads), int(pal[1]), note))

        f.write("\n; --- 見本（モンスター・アイテム。地形とは別に数える） ---\n")
        for name, (pal, g, note) in SAMPLES.items():
            f.write("mt_%s: .byte %s, %d  ; %s\n" % (
                name, ",".join("$%02X" % i for i in sample_idx[name]), int(pal[1]), note))

        f.write("\n; --- 全体マップ用 8x8（全体マップ画面では黒地のパレットに入れかえる） ---\n")
        for name, _ in MINIMAP:
            f.write("MM_%s = $%02X\n" % (name[3:].upper(), mm_idx[name]))
        f.write("; 全体マップのスプライト（階段）は BG とは別に持つ\n")
        f.write("\n; --- 文字（黒地に白。パレット P2） ---\n")
        for chn in sorted(FONT5):
            label = {"/": "SLASH", " ": "SPACE"}.get(chn, chn)
            f.write("CH_%s = $%02X\n" % (label, font_idx[chn]))

    render_chips().save(os.path.join(OUT, "dungeon_chips.png"), scale=3)
    render_sheet().save(os.path.join(OUT, "dungeon_tiles.png"), scale=2)
    render_depths().save(os.path.join(OUT, "dungeon_depths.png"), scale=2)
    render_mock().save(os.path.join(OUT, "mock_screen.png"))
    render_minimap_sheet().save(os.path.join(OUT, "minimap_tiles.png"))
    render_rock_demo().save(os.path.join(OUT, "rock_demo.png"))
    cv, tiles, g, known = render_minimap_mock()
    cv.save(os.path.join(OUT, "mock_minimap.png"))

    print("8x8 tiles: %d (terrain %d, samples %d, minimap %d, font %d)" % (
        len(chr_.tiles), n_terrain, n_samples, n_mm, n_font))
    rock_chips = [n for n in CHIPS if n.startswith(("rock_", "gem_"))] + ["black"]
    print("rock chips in CHR: %d" % len(rock_chips))
    for label, rows in (("rock_demo", ROCK_DEMO), ("mock", [r.replace("m", "#") for r in MOCK])):
        cells, chips, colored, blocks = rock_stats(rows)
        print("%s: rock cells %d, 8x8 positions %d, chips used %d, colored 8x8 variants %d, 16x16 variants %d" % (
            label, cells, cells * 4, len(chips), len(colored), len(blocks)))
    if "-v" in sys.argv:
        for y in range(DH):
            print("".join(g[y][x] if known[y][x] else " " for x in range(DW)))


if __name__ == "__main__":
    main()
