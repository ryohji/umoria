#!/usr/bin/env python3
"""ファミコン向けダンジョンタイルセットを作る。

初代ゼルダのダンジョン（黒地・盛り上がった石組み・レンガの壁・扉のアーチ）を
手本にした 16x16 のメタタイルと、全体マップ用の 8x8 罫線タイルを、NES の
制約（8x8 タイル 1 枚 4 色、16x16 ごとに BG パレット 1 本、BG パレット 4 本）
の中で組み立て、次のファイルを assets/famicom/ に書き出す。

  dungeon_bg.chr        NES の 2bpp CHR（重複を除いた 8x8 タイル）
  dungeon_metatiles.inc ca65 の表（メタタイル名・パレット・4 枚のタイル番号）
  dungeon_tiles.png     タイル一覧（3 倍）
  dungeon_depths.png    深さごとの地形パレット（3 倍）
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
    0x2A: 0x58D854, 0x2B: 0x58F898, 0x2C: 0x00E8D8,
    0x30: 0xFCFCFC, 0x31: 0xA4E4FC, 0x32: 0xB8B8F8, 0x33: 0xD8B8F8, 0x34: 0xF8B8F8,
    0x35: 0xF8A4C0, 0x36: 0xF0D0B0, 0x37: 0xFCE0A8, 0x38: 0xF8D878,
}

# BG パレット 4 本。0 番は全パレット共通の背景（黒）。
# 1 番はどのパレットでも「暗い色」にする。床の目地をこの色で描くので、
# モンスターやアイテムのタイルにも同じ位置に目地を描ける（規則 R2）。
PALETTES = {
    "P0": [0x0F, 0x0C, 0x1C, 0x2C],  # 石組み（深さで入れかえる）: 暗い青緑・青緑・明るい水色
    "P1": [0x0F, 0x07, 0x17, 0x27],  # 暖色: 扉の木・溶岩の鉱脈・金・火・守りのルーン
    "P2": [0x0F, 0x00, 0x10, 0x30],  # 白と灰: 石英の鉱脈・金属の罠・薬瓶・文字
    "P3": [0x0F, 0x0A, 0x1A, 0x2A],  # 生き物（緑）: モンスターの既定・酸の罠
}

# 深さ帯ごとの P0（ゼルダの各レベルが色を変えるのにならう）
DEPTH_P0 = [
    ("B1-10", [0x0F, 0x0C, 0x1C, 0x2C]),
    ("B11-20", [0x0F, 0x02, 0x12, 0x22]),
    ("B21-30", [0x0F, 0x0B, 0x1B, 0x2B]),
    ("B31-40", [0x0F, 0x04, 0x14, 0x24]),
    ("B41-", [0x0F, 0x00, 0x10, 0x20]),
]

# スプライトパレット（プレイヤー・全体マップの現在地）
SPRITE_PAL = [0x0F, 0x0F, 0x12, 0x37]
MARKER_PAL = [0x0F, 0x30, 0x30, 0x30]


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


def grid_lines(g):
    """床の目地: 右端の列と下端の行を 1 番で描く（空いている所だけ）。"""
    for i in range(16):
        if g[15][i] == 0:
            g[15][i] = 1
        if g[i][15] == 0:
            g[i][15] = 1
    return g


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
# 壁
# ---------------------------------------------------------------------------
def block(face=2, top=3, shadow=1, recess=True):
    """ゼルダの押せるブロックのような、盛り上がった 16x16 の石。"""
    g = blank()
    for r in range(16):
        for c in range(16):
            if r < 2 and r <= 15 - c:
                v = top
            elif c < 2 and c <= 15 - r:
                v = top
            elif r > 13 or c > 13:
                v = shadow
            else:
                v = face
            g[r][c] = v
    if recess:
        # 中央のくぼみ（上・左が影、下・右が明かり）
        for i in range(5, 11):
            g[5][i] = shadow
            g[i][5] = shadow
            g[10][i] = top
            g[i][10] = top
        g[5][10] = shadow
        g[10][5] = shadow
    return g


def bricks():
    """床に面した壁の正面（レンガ積み）。下 2 行は足元の影。"""
    g = blank()
    for r in range(16):
        band = r // 4
        rr = r % 4
        for c in range(16):
            off = 0 if band % 2 == 0 else 4
            if rr == 3:
                v = 1
            elif (c + off) % 8 == 7:
                v = 1
            elif rr == 0:
                v = 3
            else:
                v = 2
            g[r][c] = v
    for c in range(16):
        g[14][c] = 1
        g[15][c] = 0
    return g


def boundary_block():
    """外周の壊せない壁: 暗い面に鋲を打った鉄張りの石。"""
    g = block(face=1, top=2, shadow=1, recess=False)
    for (y, x) in [(3, 3), (3, 11), (11, 3), (11, 11)]:
        g[y][x] = 3
        g[y][x + 1] = 2
        g[y + 1][x] = 2
    for i in range(4, 12):
        g[7][i] = 2 if i % 2 else 1
        g[i][7] = 2 if i % 2 else 1
    return g


def boundary_face():
    g = bricks()
    for r in range(14):
        for c in range(16):
            if g[r][c] == 2:
                g[r][c] = 1
            elif g[r][c] == 3:
                g[r][c] = 2
    for (y, x) in [(1, 3), (5, 11), (9, 3)]:
        g[y][x] = 3
    return g


MAGMA_CRACK = [
    "................",
    "................",
    "....3...........",
    ".....3......3...",
    ".....33....3....",
    "......3...33....",
    "......33.3......",
    ".......33.......",
    "........3.......",
    "........33......",
    ".........3...3..",
    "..3.......3.3...",
    "...3.......3....",
    "................",
    "................",
    "................",
]

QUARTZ_SHARD = [
    "................",
    "................",
    "................",
    "....3......3....",
    "...333....313...",
    "...3313...3113..",
    "..33113...33113.",
    "..3311.....311..",
    "...31..3.......",
    ".......33.......",
    "......3313......",
    "......33113.....",
    ".......311......",
    "................",
    "................",
    "................",
]


def overlay(g, rows, only_on=None):
    for r, row in enumerate(rows):
        row = row.ljust(16, ".")
        for c, ch in enumerate(row[:16]):
            if ch not in ". ":
                if only_on is None or g[r][c] in only_on:
                    g[r][c] = int(ch)
    return g


def magma_top():
    g = block(face=1, top=2, shadow=1, recess=False)
    return overlay(g, MAGMA_CRACK, only_on=(1,))


def quartz_top():
    g = block(face=2, top=3, shadow=1, recess=False)
    return overlay(g, QUARTZ_SHARD, only_on=(2,))


def vein_face(kind):
    g = bricks()
    if kind == "magma":
        # 暗いレンガにひびの光
        for r in range(14):
            for c in range(16):
                if g[r][c] == 2:
                    g[r][c] = 1
                elif g[r][c] == 3:
                    g[r][c] = 2
        overlay(g, MAGMA_CRACK[:14], only_on=(1, 2))
    else:
        overlay(g, QUARTZ_SHARD[:14], only_on=(2,))
    return g


NUGGETS = [
    "................",
    "................",
    "................",
    "................",
    "................",
    "................",
    "................",
    "...........33...",
    "..........3331..",
    "...33.....3311..",
    "..3331.....11...",
    "..3311..........",
    "...11...........",
    "................",
    "................",
    "................",
]

SPARKLE = [
    "................",
    "................",
    "..........3.....",
    "..........3.....",
    "........33333...",
    "..........3.....",
    "..........3.....",
    "................",
    "................",
    "...3............",
    "..333...........",
    "...3............",
    "................",
    "................",
    "................",
    "................",
]


def with_treasure(g):
    g = [row[:] for row in g]
    overlay(g, NUGGETS)
    overlay(g, SPARKLE)
    return g


# ---------------------------------------------------------------------------
# 床・階段・瓦礫
# ---------------------------------------------------------------------------
def floor_room():
    g = blank()
    for (y, x) in [(3, 3), (3, 11), (11, 3), (11, 11)]:
        g[y][x] = 1
    return grid_lines(g)


def floor_corridor():
    g = blank()
    for (y, x, v) in [(2, 4, 1), (3, 4, 1), (3, 5, 1), (9, 10, 1), (9, 11, 2), (10, 10, 1),
                      (12, 3, 1), (6, 12, 1), (13, 8, 1)]:
        g[y][x] = v
    return grid_lines(g)


def stairs(down):
    g = blank()
    widths = [14, 12, 10, 8] if down else [8, 10, 12, 14]
    for i, w in enumerate(widths):
        top = 1 + i * 3 + (1 if not down else 0)
        left = (16 - w) // 2
        for k, v in enumerate([3, 2, 1]):
            r = top + k
            for c in range(left, left + w):
                g[r][c] = v
            g[r][left] = 1
            g[r][left + w - 1] = 1
    if down:
        # いちばん下は闇へ落ちる
        for c in range(5, 11):
            g[13][c] = 0
    else:
        # 上り: てっぺんに光
        for c in range(6, 10):
            g[1][c] = 3
    return grid_lines(g)


def rubble():
    g = blank()
    for (cy, cx, ry, rx) in [(10.5, 4.5, 3.2, 3.6), (11, 11, 3, 3.5), (6.5, 8, 3.2, 3.8), (12.5, 7.5, 2.4, 2.8)]:
        ellipse(g, cy, cx, ry, rx)
    return grid_lines(g)


# ---------------------------------------------------------------------------
# 扉（P1）。H は左右が壁の扉。V は H を転置して作る
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
        # 開いた扉の板（左の枠に寄せて立つ）
        g[r][2] = 3
        g[r][3] = 2
        g[r][4] = 1
    for r in (4, 11):
        g[r][3] = 1
    return grid_lines(g)


# ---------------------------------------------------------------------------
# 罠
# ---------------------------------------------------------------------------
def trap_pit():
    """石で縁どった穴。奥（上側）の壁だけ暗く見える。"""
    g = blank()
    cy = cx = 7.5
    for y in range(16):
        for x in range(16):
            d = ((y - cy) ** 2 + (x - cx) ** 2) ** 0.5
            if d <= 7.0:
                if d > 5.0:
                    # 縁の石: 左上側は明るく、継ぎ目を少し入れる
                    v = 3 if (x + y) < 13 else 2
                    if (x * 3 + y * 5) % 11 == 0:
                        v = 1
                    g[y][x] = v
                else:
                    d2 = ((y - (cy + 2.2)) ** 2 + (x - cx) ** 2) ** 0.5
                    g[y][x] = 1 if d2 > 5.0 else 0
    return grid_lines(g)


def trap_door():
    g = blank()
    for r in range(2, 14):
        for c in range(2, 14):
            if r in (2, 13) or c in (2, 13):
                g[r][c] = 1
            else:
                g[r][c] = 3 if (r - 3) % 3 == 0 else 2
    for r in (4, 5, 10, 11):
        g[r][3] = 1
        g[r][4] = 1
    g[8][11] = 1
    return grid_lines(g)


def trap_plate(holes=True):
    g = blank()
    for r in range(2, 14):
        for c in range(2, 14):
            if r == 2 or c == 2:
                g[r][c] = 3
            elif r == 13 or c == 13:
                g[r][c] = 1
            else:
                g[r][c] = 2
    if holes:
        for y in (4, 7, 10):
            for x in (4, 7, 10):
                g[y][x] = 1
                g[y][x + 1] = 1
                g[y + 1][x] = 1
                g[y + 1][x + 1] = 0
    return grid_lines(g)


def trap_gas():
    g = blank()
    ellipse(g, 7.5, 7.5, 5.5, 5.5, fill=1, edge=2, hi=0)
    for r in (5, 7, 9):
        for c in range(4, 12):
            if g[r][c] == 1:
                g[r][c] = 0
    for (y, x) in [(2, 6), (2, 7), (3, 4)]:
        g[y][x] = 3
    # もれ出るガス
    for (y, x) in [(1, 11), (0, 12), (1, 13), (3, 13)]:
        g[y][x] = 3
    return grid_lines(g)


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


def from_art(rows):
    g = from_ascii([r.ljust(16, ".")[:16] for r in rows])
    return grid_lines(g)


def trap_rock():
    g = blank()
    ellipse(g, 8, 8, 4.5, 5.5)
    for (y, x) in [(8, 6), (9, 7), (9, 8), (10, 9), (7, 9), (6, 10)]:
        g[y][x] = 1
    for (y, x) in [(14, 2), (13, 3), (2, 13), (3, 12), (13, 13)]:
        g[y][x] = 2
    return grid_lines(g)


# ---------------------------------------------------------------------------
# モンスター・アイテムの見本（地形の上に BG で重ねる規則を確かめる）
# ---------------------------------------------------------------------------
def jelly():
    g = blank()
    ellipse(g, 9.5, 7.5, 5.5, 6.8)
    for (y, x) in [(8, 5), (8, 9)]:
        g[y][x] = 0
        g[y + 1][x] = 0
        g[y][x + 1] = 0
        g[y + 1][x + 1] = 0
    return grid_lines(g)


def gold_pile():
    """硬貨の山。奥から順に重ね、硬貨の面は 3、側面は 2。"""
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
    return grid_lines(g)


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


mt("void", "P0", blank(), "未探索・闇")
mt("floor_room", "P0", floor_room(), "部屋の床（目地つき）")
mt("floor_corr", "P0", floor_corridor(), "通路の床（小石）")
mt("wall_top", "P0", block(), "花崗岩の壁（上面・ブロック）。隠し扉も同じ絵")
mt("wall_face", "P0", bricks(), "花崗岩の壁（床に面した正面・レンガ）")
mt("perm_top", "P0", boundary_block(), "外周の壁（上面）")
mt("perm_face", "P0", boundary_face(), "外周の壁（正面）")
mt("magma_top", "P1", magma_top(), "溶岩の鉱脈（上面）")
mt("magma_face", "P1", vein_face("magma"), "溶岩の鉱脈（正面）")
mt("magma_top_t", "P1", with_treasure(magma_top()), "溶岩の鉱脈＋宝（上面）")
mt("magma_face_t", "P1", with_treasure(vein_face("magma")), "溶岩の鉱脈＋宝（正面）")
mt("quartz_top", "P2", quartz_top(), "石英の鉱脈（上面）")
mt("quartz_face", "P2", vein_face("quartz"), "石英の鉱脈（正面）")
mt("quartz_top_t", "P2", with_treasure(quartz_top()), "石英の鉱脈＋宝（上面）")
mt("quartz_face_t", "P2", with_treasure(vein_face("quartz")), "石英の鉱脈＋宝（正面）")
mt("door_closed_h", "P1", door_closed_h(), "閉じた扉（横の壁）。鍵・つっかえも同じ")
mt("door_closed_v", "P1", transpose(door_closed_h()), "閉じた扉（縦の壁）")
mt("door_open_h", "P1", door_open_h(), "開いた扉・壊れた扉（横の壁）")
mt("door_open_v", "P1", transpose(door_open_h()), "開いた扉・壊れた扉（縦の壁）")
mt("stairs_up", "P0", stairs(False), "上り階段 <")
mt("stairs_down", "P0", stairs(True), "下り階段 >")
mt("rubble", "P0", rubble(), "瓦礫 :")
mt("trap_pit", "P0", trap_pit(), "落とし穴（開いた・覆われた）")
mt("trap_door", "P1", trap_door(), "落とし戸")
mt("trap_dart", "P2", trap_plate(True), "矢・吹き矢の罠（穴の空いた板）")
mt("trap_gas", "P2", trap_gas(), "ガスの罠（通気口）")
mt("trap_rune", "P2", from_art(RUNE), "不思議なルーン（テレポート・召喚）")
mt("trap_fire", "P1", from_art(SCORCH), "焦げ跡（火炎の罠）")
mt("trap_rock", "P0", trap_rock(), "落石の罠（ゆるんだ岩）")
mt("trap_acid", "P3", from_art(ACID), "酸の罠（腐食した岩）")
mt("glyph", "P1", from_art(GLYPH), "守りのルーン（Scare Monster）")
mt("ex_jelly", "P3", jelly(), "見本: ゼリー J（モンスターも BG・目地つき）")
mt("ex_gold", "P1", gold_pile(), "見本: 金貨の山 $（這う硬貨と共用）")
mt("ex_potion", "P2", from_art(POTION), "見本: 薬瓶 !")

MT_INDEX = {m[0]: i for i, m in enumerate(METATILES)}

# ---------------------------------------------------------------------------
# 全体マップ用 8x8 タイル（4x4 マス = 1 タイル、正方形の区画）
# ---------------------------------------------------------------------------
N, E, S, W = 1, 2, 4, 8


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
STAIRS_PAL = [0x0F, 0x0F, 0x28, 0x28]

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
    g = blank(8, 8)
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
    pal = pal or PALETTES["P2"]
    for i, ch in enumerate(s):
        cv.draw(glyph8(ch), x + i * 8, y, pal)


# ---------------------------------------------------------------------------
# モック画面（16x11 マスの地図＋状態 2 行）
# ---------------------------------------------------------------------------
MOCK = [
    "XXXXXXXXXXXXXXXX",
    "X##mmM####qqQ###",
    "X#.....#.......#",
    "X#.<...+...j...#",
    "X#..@..#.^.....#",
    "X#..*..#.....!.#",
    "X##'####..W....#",
    "X #,#  ###'#####",
    "X #,,,T,,,,,,,>#",
    "X ########:#####",
    "X        #,#    ",
]
WALLS = {"#": ("wall_top", "wall_face"), "X": ("perm_top", "perm_face"),
         "m": ("magma_top", "magma_face"), "M": ("magma_top_t", "magma_face_t"),
         "q": ("quartz_top", "quartz_face"), "Q": ("quartz_top_t", "quartz_face_t")}
SIMPLE = {".": "floor_room", ",": "floor_corr", "<": "stairs_up", ">": "stairs_down",
          ":": "rubble", "^": "trap_pit", "T": "trap_door", "W": "glyph", "j": "ex_jelly",
          "*": "ex_gold", "!": "ex_potion", "@": "floor_room", " ": "void"}


def mock_metatile(rows, y, x):
    ch = rows[y][x]
    below = rows[y + 1][x] if y + 1 < len(rows) else " "
    if ch in WALLS:
        top, face = WALLS[ch]
        return face if (below not in WALLS and below != " ") else top
    if ch in "+'":
        left = rows[y][x - 1] if x > 0 else " "
        horiz = left in WALLS
        base = "door_closed" if ch == "+" else "door_open"
        return base + ("_h" if horiz else "_v")
    return SIMPLE[ch]


def render_mock():
    cv = Canvas(256, 240)
    text(cv, "B5F", 16, 8)
    for y, row in enumerate(MOCK):
        assert len(row) == 16, row
        for x in range(16):
            name = mock_metatile(MOCK, y, x)
            _, pal, g, _ = METATILES[MT_INDEX[name]]
            cv.draw(g, x * 16, 16 + y * 16, PALETTES[pal])
            if MOCK[y][x] == "@":
                cv.draw(from_ascii(PLAYER), x * 16, 16 + y * 16, SPRITE_PAL, transparent0=True)
    text(cv, "LV12 HP 45/ 60 MP  8/12", 16, 200)
    text(cv, "AU 1234", 16, 208)
    return cv


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
    pal = PALETTES["P0"]
    ox, oy = 4, 5  # 地図の左上（タイル単位）
    th, tw = len(tiles), len(tiles[0])
    text(cv, "MAP  B5F", ox * 8, 2 * 8, PALETTES["P2"])
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
# 一覧画像
# ---------------------------------------------------------------------------
def num_label(cv, n, x, y):
    text(cv, "%02d" % n, x, y, PALETTES["P2"])


def render_sheet():
    cols = 8
    cw, ch = 24, 28
    rows = (len(METATILES) + cols - 1) // cols
    cv = Canvas(cols * cw + 8, rows * ch + 8, bg=0x202020)
    for i, (name, pal, g, _) in enumerate(METATILES):
        x = 8 + (i % cols) * cw
        y = 4 + (i // cols) * ch
        cv.draw(g, x, y, PALETTES[pal])
        num_label(cv, i, x, y + 17)
    return cv


def render_depths():
    names = ["wall_top", "wall_face", "floor_room", "floor_corr", "stairs_down", "rubble"]
    cv = Canvas(len(names) * 16 + 8, len(DEPTH_P0) * 20 + 4, bg=0x202020)
    for j, (label, pal) in enumerate(DEPTH_P0):
        for i, n in enumerate(names):
            g = METATILES[MT_INDEX[n]][2]
            cv.draw(g, 4 + i * 16, 2 + j * 20, pal)
    return cv


def render_minimap_sheet():
    cols = 16
    cv = Canvas(cols * 12 + 4, ((len(MINIMAP) + cols - 1) // cols) * 12 + 4, bg=0x202020)
    for i, (_, g) in enumerate(MINIMAP):
        cv.draw(g, 4 + (i % cols) * 12, 4 + (i // cols) * 12, PALETTES["P0"])
    base = len(MINIMAP) + 2
    for j, (_, g) in enumerate(MM_SPRITES):
        i = base + j
        cv.draw(g, 4 + (i % cols) * 12, 4 + (i // cols) * 12, STAIRS_PAL)
    return cv


def main():
    os.makedirs(OUT, exist_ok=True)
    chr_ = Chr()
    chr_.add(blank(8, 8))  # 0 番は空白
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
        f.write("; パレット: " + "  ".join(
            "%s=%s" % (k, ",".join("$%02X" % c for c in v)) for k, v in PALETTES.items()) + "\n\n")
        for i, (name, pal, idx, note) in enumerate(table):
            f.write("MT_%s = %d  ; %s\n" % (name.upper(), i, note))
        f.write("\nmetatile_tiles:\n")
        for name, pal, idx, note in table:
            f.write("    .byte $%02X,$%02X,$%02X,$%02X  ; %s\n" % (idx[0], idx[1], idx[2], idx[3], name))
        f.write("\nmetatile_attr:\n")
        for name, pal, idx, note in table:
            f.write("    .byte %d  ; %s\n" % (int(pal[1]), name))
        f.write("\n; 全体マップ用 8x8（パレット P0）\n")
        for name, _ in MINIMAP:
            f.write("MM_%s = $%02X\n" % (name[3:].upper(), mm_idx[name]))
        f.write("; 全体マップのスプライト（階段）は BG とは別に持つ\n")
        f.write("\n; 文字（パレット P2）\n")
        for chn in sorted(FONT5):
            label = {"/": "SLASH", " ": "SPACE"}.get(chn, chn)
            f.write("CH_%s = $%02X\n" % (label, font_idx[chn]))

    render_sheet().save(os.path.join(OUT, "dungeon_tiles.png"))
    render_depths().save(os.path.join(OUT, "dungeon_depths.png"))
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
