# ファミコン級コンソール移植のための絵の洗い出し

作成日: 2026-09-24

このリポジトリの Umoria 5.x（`src/`）を、ファミコン（NES）程度の機械へ移す
ときに要る絵を数えた資料。数はすべてソースから数えた（付録 A・B は
`src/monsters.c` の `c_list` と `src/treasure.c` の `object_list` から
機械的に作った表）。

前提は次の 3 つ。

- ダンジョンは **16×16 ドット**を 1 マスとして描く（8×8 タイル 4 枚の
  メタタイル）。
- 文字は 8×8 ドット。英数字・ひらがな・カタカナを持つ。
- 全体マップ（原典の `M` コマンド、`screen_map()` `src/io.c:395`）は
  **罫線のようなタイル**で描く。
- 1 マスは**縦横同じ大きさ**にする（原典の端末は 1 マスが縦長だった）。
  それに合わせて、ダンジョン全体の大きさも見直す（第 8.4 節）。

ダンジョンのタイルの試作は別の資料 `docs/famicom_dungeon_tileset.md` に
まとめた。

---

## 0. 全体の数（まとめ）

| 区分 | 原典の数 | 描き分けの提案 | 16×16 の絵 | 8×8 タイル換算 |
|---|---:|---|---:|---:|
| 地形・ダンジョン | 地形 9 種＋扉・階段・罠など | 第 2 節 | 約 30 | 約 120 |
| 町（店・建物） | 店 6 軒 | 第 2.6 節 | 約 12 | 約 48 |
| プレイヤー | 8 種族 × 6 職業 | 職業別 6 体（最小 1 体） | 6（×向き・歩き） | 24〜96 |
| モンスター | **279 種・49 記号** | 基本の形 **約 85** ＋パレット替え | 約 85（×2 コマなら 170） | 340〜680 |
| アイテム | **420 件**（地形扱いと金を除くと 369） | 形 **約 65**＋パレット替え | 約 65 | 約 260 |
| エフェクト | ボルト・ボール・ブレス | 第 5 節 | 約 12 | 約 48 |
| 文字 | 英数 95 字＋かな | 第 6 節 | — | 約 200〜250 |
| 窓枠・UI 部品 | — | 第 7 節 | — | 約 40 |
| 全体マップ用罫線 | — | 第 8 節 | — | BG 39 ＋スプライト 2（窓枠と共用あり） |
| 1 枚絵 | 墓石・タイトルなど | 第 9 節 | — | 画面ごと |

8×8 タイルに直すと合計で 1,000〜1,500 枚ほどになる。1 画面で同時に使える
BG タイルは 256 枚しかないので、**地図の領域と文字の領域で CHR バンクを
切りかえる**か、**CHR-RAM に「いま見えるものだけ」を載せる**必要がある
（第 10 節）。

---

## 1. 画面の組みかたの前提

原典は 80×24 文字の端末で、地図の窓は 66×22 マス（`SCREEN_WIDTH`
`SCREEN_HEIGHT`、`src/constant.h:43`）、ダンジョン全体は 198×66 マス
（`MAX_WIDTH` `MAX_HEIGHT`）。

ファミコンは 256×240 ドット（8×8 タイルで 32×30、ブラウン管の見切れを
除くと安全なのは 28×26 ほど）。16×16 マスにすると画面いっぱいでも
**16×15 マス**しか見えない。案としては:

```
+--------------------------------+  y=0
| メッセージ 2 行 (8x8 文字)       |  16 px
+--------------------------------+
|                                |
|  地図 16 x 11 マス (16x16)       |  176 px
|  256 x 176 ドット               |
|                                |
+--------------------------------+
| 状態 3 行: HP/MP/階/金/状態異常  |  24 px
+--------------------------------+  (残り 24 px は見切れ余白)
```

- 見える範囲が原典（66×22）よりずっと狭いので、**全体マップの価値が
  原典より高い**。第 8 節の罫線マップはほぼ必須と考えてよい。
- 原典のパネル（画面単位の飛びスクロール、`src/panel.c`）はそのままでも
  動くが、16×11 だとすぐ端に着くので、プレイヤー中心の 1 マス単位
  スクロールのほうが遊びやすい。
- 16×16 マスはファミコンの属性テーブル（BG パレットを 16×16 ドット単位で
  選ぶ）とちょうど一致する。**1 マス 1 パレット**で素直に塗れる。

---

## 2. ダンジョンを組み立てるタイル（16×16）

原典の地形は `cave[y][x].fval`（`src/constant.h:239-256`）と、床に置かれる
「物」（扉・階段・罠・瓦礫も `t_list` の物として置かれる）の 2 層で
できている。表示の決め方は `loc_symbol()`（`src/misc1.c:408`）。

### 2.1 床と壁

| 絵 | 原典 | 記号 | 備考 |
|---|---|---|---|
| 未探索・闇 | 見えていない、覚えていない | 空白 | 真っ黒。タイル 0 番 |
| 部屋の床（明るい） | `LIGHT_FLOOR` かつ照らされている | `.` | |
| 部屋の床（記憶だけ） | `DARK_FLOOR` など、覚えているが光が無い | `.` | 同じ絵で暗いパレット |
| 通路の床 | `CORR_FLOOR` `BLOCKED_FLOOR` | `.` | 部屋と描き分けると全体マップとも話が合う（任意） |
| 花崗岩の壁 | `GRANITE_WALL` | `#` | 基本の壁 |
| 外周の壁（壊せない） | `BOUNDARY_WALL` | `#` | 原典は花崗岩と同じ見た目。描き分けてもよい |
| 溶岩の鉱脈 | `MAGMA_WALL` | `%` | 原典はオプション `highlight_seams` のときだけ `%` |
| 石英の鉱脈 | `QUARTZ_WALL` | `%` | 同上 |
| 鉱脈の中の宝 | 鉱脈のマスに金・宝石（`$` `*`）が置かれる | `*` | 「鉱脈の壁＋宝のきらめき」の合成が要る。**壁の上に物が乗る**唯一の組み合わせ |

壁の見栄え:

- **最小**: 壁 1 枚（＋鉱脈 2 枚）。
- **推奨（試作で採用）**: 岩の塊の壁で、上下左右のどの辺が床に面するかの
  16 通りを描き分ける。床に面した辺だけ岩の縁を削って床の色を出す。
  削るのは辺から 7 ドット以内なので、8×8 の 1/4 ごとに効く辺は 2 つ
  だけ。1 種類あたり **8×8 タイル 16 枚**で 16 通りが揃う
  （`docs/famicom_dungeon_tileset.md`）。
- 共通の背景色（パレットの 0 番）を**床の色**にしておくのが前提。
  そうすると壁のタイルも床の色を使えるので、1 マス 1 パレットのまま
  岩と床の境目をぎざぎざにできる。

### 2.2 扉・階段・瓦礫

| 絵 | 原典 | 記号 | 備考 |
|---|---|---|---|
| 開いた扉 | `TV_OPEN_DOOR` | `'` | 壊れた扉（開いたまま閉まらない）も原典では同じ `'` |
| 閉じた扉 | `TV_CLOSED_DOOR` | `+` | 鍵のかかった扉・つっかえた扉も原典では見分けがつかない。**見分けがつかないことが遊びの一部** |
| 隠し扉 | `TV_SECRET_DOOR` | `#` | **花崗岩の壁と完全に同じ絵にする**（別の絵にすると探索の意味が無くなる） |
| 上り階段 | `TV_UP_STAIR` | `<` | |
| 下り階段 | `TV_DOWN_STAIR` | `>` | |
| 瓦礫 | `TV_RUBBLE` | `:` | 通れない。掘れる |

### 2.3 罠

原典は見つけた罠をすべて `^` で描き、名前だけ変える（落とし穴は例外で
空白）。罠は 18 種＋守りのルーン 1 種（`src/moria3.c:37-160` の効果、
`src/treasure.c` の名前）。

| 絵の案 | 含まれる罠（効果） | 原典の名前 |
|---|---|---|
| 開いた落とし穴 | 1 Open pit | an open pit |
| 覆われた落とし穴 | 3 Covered pit | a covered pit |
| 落とし戸 | 4 Trap door（1 階下へ） | a trap door |
| 矢の罠 | 2 Arrow trap | an arrow trap |
| 吹き矢の罠 | 7 STR Dart / 17 Slow Dart / 18 CON Dart | a dart trap |
| ガスの罠 | 5 Sleep / 14 Poison / 15 Blind / 16 Confuse gas | a gas trap |
| 腐食のガス | 10 Corrode gas | some corroded rock |
| 不思議なルーン | 8 Teleport / 11 Summon monster | a strange rune |
| 落石 | 9 Rockfall | some loose rock / a loose rock |
| 焦げ跡 | 12 Fire trap | a blackened spot |
| 酸の罠 | 13 Acid trap | some corroded rock |
| 隠し物（罠ではなく物が出る） | 6 Hid Obj | — |
| 守りのルーン | 99 Scare monster（巻物・呪文で置く） | a strange rune |

- **最小**: 汎用の罠 1 枚（原典どおり `^` 相当）＋守りのルーン 1 枚。
- **推奨**: 上表の 9〜11 枚。ガスと吹き矢は 1 枚ずつにして色違いにできる。

### 2.4 状態による見えかた

| 状態 | 原典のふるまい | 必要な絵 |
|---|---|---|
| 盲目 | `@` 以外すべて空白 | 追加の絵は要らない（黒で塗る） |
| 幻覚 | 12 回に 1 回、ランダムな記号を描く（`randint(95)+31`） | 既存のモンスター・アイテムの絵を**でたらめに引いて描く**仕組みで足りる |
| 走る・探す | 原典は `@` を消して走る | なし |
| 検知の呪文 | 見えないモンスターも一瞬描く | なし（通常の絵で描く） |

### 2.5 プレイヤー

- 原典は種族 8（Human, Half-Elf, Elf, Halfling, Gnome, Dwarf, Half-Orc,
  Half-Troll）× 職業 6（Warrior, Mage, Priest, Rogue, Ranger, Paladin）×
  性別 2。全部は描けない。
- **推奨: 職業ごとに 6 体**。向き 4 方向 × 2 コマで 1 体 8 枚の 16×16、
  ただし左右は反転で済むので実質 3 方向 × 2 コマ = 6 枚。
- **最小: 1 体**（向きなし）。原典は向きの概念が無いので、向きを出すなら
  「最後に動いた方向」を覚えておく。
- プレイヤーだけはスプライトにして点滅・歩行アニメを付けると、
  BG で描くモンスター（第 10 節）とはっきり区別できる。

### 2.6 町（第 0 階）

原典の町は 1 画面分（66×22）で、店 6 軒は外周と同じ壊せない壁の四角に
店の扉（`TV_STORE_DOOR`、記号 `1`〜`6`）が 1 つ付いたもの
（`town_gen()` `src/generate.c:1209`）。昼は全体が明るく、夜は暗い。

| 絵 | 数 | 備考 |
|---|---:|---|
| 町の地面（昼・夜） | 1 | パレットで昼夜を切りかえる |
| 建物の壁・屋根 | 3〜6 | 屋根の縁・屋根の中・正面の壁など |
| 店の扉 | 6 | 看板の絵で描き分け: 1 雑貨屋（袋）、2 防具屋（盾）、3 武器屋（剣）、4 神殿（聖印）、5 錬金術師（フラスコ）、6 魔法屋（杖・星） |
| 町の外周 | 1 | 柵や石垣にしてもよい（原典は `#`） |

町の人（浮浪児、物乞い、酔っ払いなど 8 種）はモンスター `p` の一部
（付録 A）。

---

## 3. 敵キャラクター（モンスター）

原典は **279 種**、表示記号は **49 種**。同じ記号の中は「色違い」が多い
（White/Yellow/Blue/Green Jelly など）。そこで:

- **形（デザイン）は記号ごと**に作り、同じ記号でも見た目が大きく違う
  ものだけ形を分ける。
- **色違いはパレット替え**で出す。

原典の記号 1 つにつき形 1 つなら 49 枚。下の表の「形の数」は、それを
見た目の違いで分けた提案で、**合計約 85**。全種類の名前は付録 A。

| 記号 | 種類 | 数 | 階層 | 形の数 | 分けかたの提案 |
|---|---|---:|---|---:|---|
| `p` | 人間 | 29 | 0–50 | 10 | 町の人（浮浪児・物乞い・病人・白痴・酔っ払い）2、ならず者・盗賊・山賊 1、傭兵・古参兵・戦士・剣士 1、狂戦士 1、黒騎士 1、忍者 1、僧侶 1、魔術師・死霊術師・妖術師 2（杖と頭巾で差）、ホビット・ノーム 1。**Evil Iggy は固有の絵**を足すなら +1 |
| `o` | オーク・オーガ | 7 | 6–31 | 2 | オーク系（Orc, Shaman, Warrior, Black, Uruk-Hai）とオーガ系（Ogre, Ogre Mage） |
| `k` | コボルド | 1 | 1 | 1 | |
| `H` | ホブゴブリン | 1 | 11 | 1 | |
| `T` | トロル | 4 | 17–32 | 2 | 普通のトロルと双頭トロル |
| `P` | 巨人 | 5 | 14–20 | 1 | 丘・霜・火・石・雲の巨人はパレット替え |
| `h` | ハーピー | 4 | 2–9 | 1 | |
| `n` | ナーガ | 4 | 3–15 | 1 | |
| `y` | イーク | 5 | 2–12 | 1 | |
| `j` | ジャッカル | 1 | 4 | 1 | 群れで出る |
| `r` | ネズミ類 | 8 | 1–29 | 2 | ネズミと Vorpal Bunny（ウサギ） |
| `R` | ヘビ | 9 | 1–12 | 2 | 普通のヘビとコブラ（King Cobra） |
| `f` | カエル | 4 | 2–13 | 1 | |
| `c` | ムカデ | 9 | 1–15 | 1 | |
| `a` | アリ | 8 | 2–32 | 1 | |
| `A` | アリジゴク | 5 | 26–36 | 1 | |
| `K` | 甲虫 | 11 | 13–37 | 2 | 普通の甲虫とクワガタ（Stag, Slicer） |
| `F` | 羽虫・トンボ | 10 | 5–27 | 2 | ハエ・ブヨ・ノミとトンボ（Dragon Fly） |
| `b` | コウモリ | 16 | 3–30 | 2 | コウモリとドラゴンバット |
| `l` | シラミ | 2 | 3–14 | 1 | 大群で増える |
| `t` | ダニ | 4 | 10–26 | 1 | |
| `S` | サソリ | 4 | 17–34 | 1 | |
| `w` | ワーム | 7 | 1–40 | 2 | 群れのワーム（mass）と巨大ワーム（Purple, Disenchanter） |
| `i` | アイキー・シング | 8 | 1–9 | 1 | |
| `e` | 目玉 | 4 | 1–7 | 1 | |
| `m` | カビ | 11 | 3–27 | 1 | 動かない |
| `,` | キノコ | 8 | 1–10 | 1 | **食料のキノコ（アイテム `,`）と同じ絵にする**。原典は床のキノコと区別がつかないのが仕掛け |
| `$` | 這う硬貨 | 3 | 4–10 | 0 | **金貨の山（アイテム `$`）と同じ絵を使う**。擬態が仕掛け |
| `J` | ゼリー | 8 | 2–12 | 1 | 動かない |
| `O` | ウーズ | 6 | 19–40 | 1 | |
| `C` | ゼラチナス・キューブ | 1 | 16 | 1 | |
| `E` | 精霊・エレメンタル | 8 | 12–34 | 3 | 精霊（Air/Water/Earth/Fire Spirit）、エレメンタル（Water/Fire/Earth）、Invisible Stalker（見えないのが普通。輪郭だけ） |
| `G` | 幽霊 | 7 | 3–34 | 2 | 幽霊類と Spirit Troll |
| `s` | 骸骨 | 6 | 5–33 | 2 | 人型の骸骨とトロルの骸骨 |
| `z` | ゾンビ | 3 | 7–12 | 1 | |
| `M` | ミイラ | 4 | 19–35 | 1 | |
| `W` | ワイト・レイス | 8 | 24–39 | 2 | ワイトとレイス |
| `V` | 吸血鬼 | 3 | 27–37 | 1 | |
| `L` | リッチ | 3 | 34–40 | 1 | |
| `g` | ゴーレム | 4 | 14–22 | 1 | 肉・粘土・石・鉄はパレット替え |
| `q` | クアジット | 2 | 16–40 | 1 | |
| `Q` | クイルスルグ | 2 | 20–40 | 1 | 召喚だけする肉塊 |
| `U` | アンバーハルク | 1 | 16 | 1 | |
| `X` | ゾーン | 1 | 36 | 1 | 壁の中を通る |
| `Y` | イエティ | 1 | 9 | 1 | |
| `d` | 若い・成熟した竜 | 12 | 29–38 | 1 | 色（白・青・緑・黒・赤・虹）はパレット替え |
| `D` | 古代竜 | 6 | 38–40 | 1 | 同上。`d` より大きく見える絵に（1 マスのまま。第 3.2 節） |
| `B` | バルログ | 1 | 100 | 1 | **最後の敵**。地図では 1 マス（16×16）のまま描き、大きさは第 3.2 節の方法で出す |

合計すると形は 85 前後。数を詰めるときは、1 種類しかいない記号（`k` `H`
`j` `C` `U` `X` `Y`）や虫類から削るのではなく、`p` の分けかたを減らすのが
いちばん効く。

### 3.1 モンスターの描きかたの注意

- **色違いの数**: 虹色（Multi-Hued）・透明（Clear）・銀（Silver）など、
  パレット 1 本（3 色＋背景）で表しにくい色がある。透明は「輪郭だけ」の
  別パレット、虹色は点滅（パレット回し）で出せる。
- **見えないモンスター**: Clear 系・Ghost・Invisible Stalker などは
  `See Invisible` が無いと描かれない。絵は要るが、見える機会は少ない。
- **アニメ**: 2 コマの足踏みを付けると絵の枚数が倍になる。原典は
  ターン制なので 1 コマでも遊びは成り立つ。まずは 1 コマで作り、
  パレット回しで「息づかい」を出すのが安い。
- **モンスター思い出し画面**（`/` や `recall.c`）: 大きな肖像は要らない。
  地図と同じ 16×16 の絵を 1 つ出せば足りる。

### 3.2 大きなモンスターを大きく描かない理由

最初の版ではバルログを 32×32（4 マス分）で描く案を出したが、**取り下げる**。
原典ではバルログも竜も `@` や `r` と同じ 1 文字で、それは見た目の都合では
なく**ルールそのもの**だから。

- **どのモンスターも 1 マスだけを占める。** 居場所は `cave[y][x].cptr`
  1 つで、移動・攻撃・ボルトの当たり・視線はすべてマス単位で決まる。
  絵だけ 4 マスに広げると、「4 マスのどこを叩いても当たる」「横を
  すり抜けられない」と誤解させる。
- **まわりの 3 マスを隠す。** 隣のモンスター、床のアイテム、扉、罠、
  階段が絵の下に消える。この情報で逃げ道を決めるゲームなので致命的。
- **通路からはみ出す。** 通路は幅 1 マスなので、大きな絵は壁に重なる。
- **竜は珍しくない。** `d` `D` は 18 種いて、深い階では群れや他の敵と
  一緒に出る。バルログだけ特別扱いにしても、竜で同じ問題が起きる。

大きさは 1 マスの中で出す。

1. **体格で余白を変える**: 虫・ネズミ・目玉など小さいものは 8〜10 ドットで
   マスの中に余白を残し、人型は 12〜14 ドット、竜・巨人・バルログは
   **16×16 を端まで使う**（輪郭がマスの縁に触れるくらい）。並ぶと差がわかる。
2. **輪郭と色**: 大物は輪郭を太く、パレット回しで炎や鱗をゆらめかせる
   （バルログの炎、虹色の竜）。
3. **登場の演出**: 初めて視界に入ったときに画面のフラッシュや揺れと
   メッセージを出す。地図の上のマスは増やさない。
4. **地図の外なら大きくしてよい**: 思い出し画面（`/`）や初めて出会った
   ときの紹介窓なら 32×32 や 48×48 の肖像を出してもルールは壊れない。
   枚数に余裕があれば、バルログと古代竜にだけ用意する価値がある。

---

## 4. アイテム

`object_list` は 420 件。うち 51 件は扉 3・階段 2・店 6・罠 19・瓦礫 1・
金と宝石 18・空き 2（地形として第 2 節で扱うものと、第 4.3 節の金・宝石）で、
残りの 369 件が持てる物。
全件は付録 B。

### 4.1 形の提案（約 65）

| 原典の区分（tval） | 記号 | 件数 | 形の提案 | 数 |
|---|---|---:|---|---:|
| 食料（`TV_FOOD`） | `,` | 34 | キノコ（未鑑定の 21 種はすべてキノコ）、携帯食、スライムモールド、エルフの行糧、堅パン、干し肉、エール（ジョッキ）、ワイン（瓶） | 8 |
| 剣（`TV_SWORD`） | `\|` | 24 | 短剣、片手剣、両手剣、曲刀（カトラス・サーベル・刀）、細剣（レイピア・フォイル）、折れた刃 | 6 |
| 鈍器（`TV_HAFTED`） | `\` | 9 | 棍棒、メイス、モーニングスター、フレイル、ウォーハンマー、九尾の鞭、鉄球と鎖 | 5〜7 |
| 長柄（`TV_POLEARM`） | `/` | 13 | 槍（スピア・パイク・ランス・ジャベリン）、斧（戦斧・大斧・嘴斧）、ハルバード類（グレイブ・フォシャール・ルツェルンハンマー） | 3 |
| 飛び道具（`TV_BOW`） | `}` | 6 | 弓、クロスボウ、スリング | 3 |
| 矢玉（`TV_ARROW` `TV_BOLT` `TV_SLING_AMMO`） | `{` | 6 | 矢、ボルト、石・鉄弾 | 3 |
| 楔（`TV_SPIKE`） | `~` | 1 | 鉄の楔 | 1 |
| 明かり（`TV_LIGHT`） | `~` | 4 | 松明、ランタン | 2 |
| 掘る道具（`TV_DIGGING`） | `\` | 6 | つるはし、シャベル | 2 |
| 靴（`TV_BOOTS`） | `]` | 3 | 靴 | 1 |
| 手袋（`TV_GLOVES`） | `]` | 2 | 革手袋、籠手 | 2 |
| 兜（`TV_HELM`） | `]` | 8 | 帽子・兜、冠 | 2 |
| 外套（`TV_CLOAK`） | `(` | 1 | 外套 | 1 |
| 軽い鎧（`TV_SOFT_ARMOR`） | `(` | 10 | ローブ、革鎧、ぼろ布（Filthy Rags） | 3 |
| 重い鎧（`TV_HARD_ARMOR`） | `[` | 12 | 鎖かたびら、板金鎧 | 2 |
| 盾（`TV_SHIELD`） | `)` | 6 | 革の盾、金属の盾（大きさは 3 段だがパレット・同形で可） | 2 |
| 指輪（`TV_RING`） | `=` | 30 | 指輪 | 1 |
| 首飾り（`TV_AMULET`） | `"` | 9 | 首飾り | 1 |
| 巻物（`TV_SCROLL1` `TV_SCROLL2`） | `?` | 58 | 巻物 | 1 |
| 薬（`TV_POTION1` `TV_POTION2`） | `!` | 50 | 薬瓶 | 1 |
| 油壺（`TV_FLASK`） | `!` | 2 | 油壺（投げると火炎） | 1 |
| 魔法棒（`TV_WAND`） | `-` | 24 | ワンド | 1 |
| 杖（`TV_STAFF`） | `_` | 25 | スタッフ | 1 |
| 魔法書（`TV_MAGIC_BOOK`） | `?` | 4 | 魔法書（4 冊はパレット替え） | 1 |
| 祈祷書（`TV_PRAYER_BOOK`） | `?` | 4 | 祈祷書（同上） | 1 |
| 宝箱（`TV_CHEST`） | `&` | 7 | 小さい箱、大きい箱（木・鉄・鋼はパレット）、壊れた箱 | 3 |
| がらくた（`TV_MISC` ほか） | `s` `!` `~` | 11 | 骸骨、骨、歯、陶器のかけら、折れた棒、空き瓶 | 5〜6 |

### 4.2 未鑑定の「見た目の名前」とパレット

原典は未鑑定品に、プレイごとにでたらめな見た目を割りふる
（`src/constant.h:154-161`）。

| 区分 | 見た目の候補数 | 例 |
|---|---:|---|
| 薬 | 49 色 | Azure, Crimson … |
| キノコ | 22 | Blue, Spotted … |
| スタッフ | 25 種の木 | Oak, Ebony … |
| ワンド | 25 種の金属 | Iron, Silver … |
| 指輪 | 32 種の石 | Ruby, Opal … |
| 首飾り | 11 | Amber, Coral … |
| 巻物 | 45 の題名（音節 153 から組む） | "abra ka dabra" … |

ファミコンのパレットで 49 色の薬を描き分けるのは無理なので:

- **推奨**: 形は区分ごとに 1 つ。色は「その見た目の名前にいちばん近い
  色」を 10〜12 色ほどの代表色から選んでパレット替えする。同じ色に
  見える薬が出るが、名前は文字で出るので遊びは壊れない。
- 巻物は題名が文字なので絵は 1 つでよい。

### 4.3 金と宝石

| 絵 | 原典 | 記号 | 備考 |
|---|---|---|---|
| 硬貨の山 | copper / silver / gold / mithril（11 件） | `$` | 1 枚でパレット替え 4 色。**這う硬貨（モンスター）と共用** |
| 宝石 | garnets / opals / sapphires / rubies / diamonds / emeralds（7 件） | `*` | 1 枚でパレット替え。鉱脈の宝（第 2.1 節）にも使う |

---

## 5. エフェクト

原典はボルトもボールも `*` を描いて消すだけ（`src/spells.c:665` `752`
`792` `829`）。ファミコンでは属性で色を分けたい。

| 絵 | 用途 | 数 |
|---|---|---:|
| ボルト（飛ぶ弾） | 魔法の矢、雷、冷気、火炎、酸、生命吸収 など | 形 2（縦横・斜め）× パレット |
| ボール（爆発） | 火・冷気・雷・酸・毒ガス（Stinking Cloud）、ブレス | 形 2 コマ × パレット |
| 光の線 | Wand/Staff of Light、Light Line | 1 |
| 壁が溶ける | Stone to Mud | 1〜2 |
| 投げた物・射た矢 | 飛ぶ間はその物の絵を使う（原典も物の記号を描く） | 0（アイテムの絵） |
| 命中・被弾の閃光 | 画面のフラッシュ（パレット替え）で足りる | 0 |
| 地震・破壊 | 画面揺れ（スクロール揺らし）で足りる | 0 |

矢印の向きを 8 方向描くなら、縦横 1 枚と斜め 1 枚を反転で使いまわす。

---

## 6. 文字セット（8×8）

### 6.1 英数字・記号

原典の画面・メッセージで使う ASCII 可視文字は 95 字。英語版を残すなら
全部要る。

| 群 | 字数 | 備考 |
|---|---:|---|
| 数字 `0-9` | 10 | HP・金・階など。日本語版でも必須 |
| 英大文字 `A-Z` | 26 | `HP` `MP` `AC` `LV` `EXP` や英語の略称。日本語版でも必須 |
| 英小文字 `a-z` | 26 | 英語版のみ。日本語版では省ける |
| 記号 | 33 | `! " # $ % & ' ( ) * + , - . / : ; < = > ? @ [ \ ] ^ _ { \| } ~` と空白。日本語版では `( ) + - / : % ? !` など 10 数個に減らせる |

装備の表記（`(+3,+5)` `[12,+2]` `(10 charges)`）で `( ) [ ] , +` が要る。

### 6.2 ひらがな・カタカナ

モンスター名・アイテム名はカタカナが、文のほとんどはひらがなが要る。
どちらか片方だけでは済まない。

| 群 | 字数 | 内訳 |
|---|---:|---|
| ひらがな 清音 | 46 | あ〜ん（を を含む） |
| ひらがな 小書き | 9 | ぁぃぅぇぉっゃゅょ |
| ひらがな 濁音・半濁音 | 25 | が行 ざ行 だ行 ば行 各 5、ぱ行 5 |
| カタカナ 清音 | 46 | ア〜ン |
| カタカナ 小書き | 9 | ァィゥェォッャュョ |
| カタカナ 濁音・半濁音 | 26 | ヴ を含む |
| 約物 | 10 前後 | ー 、 。 「 」 ・ … ゛ ゜ ？ ！ |

**濁点の扱いで枚数が大きく変わる。**

- **案 A: 濁点を別タイルにして上の段に置く**（ドラゴンクエストの方式）。
  かなは 46＋9 を 2 組 = **110 字**＋゛゜。1 行が縦 16 ドットになる。
- **案 B: 濁音字を 1 文字として持つ**。かなは **161 字**。1 行 8 ドットで
  詰めて書けるが、英大文字・数字と合わせると 200 字を超える。
- **形の共用**: へ／ヘ、り／リ（字形を寄せる）、カ／力、ロ／口、
  ニ／二、エ／工（漢字を使うなら）など、似た字を 1 枚にまとめると
  5〜8 字減らせる。

画面 1 枚で 256 枚のうち地図・UI の部品も要ることを考えると、
**英小文字を外し、案 A か「案 B ＋ 文字バンク 2 枚の切りかえ」**
（第 10 節）が現実的。

### 6.3 漢字（任意）

数字の単位や能力値だけ漢字にすると画面がぐっと締まる。入れるなら
最小限にとどめる。

- 能力値（原典 `STR INT WIS DEX CON CHR`、`src/misc3.c:29`）:
  **腕力 知力 賢さ 器用 耐久 魅力** → 腕 力 知 賢 器 用 耐 久 魅 の 9 字
- 状態欄: **階**（地下 12 階）、**金**、**空腹 衰弱 盲目 混乱 恐怖 毒
  麻痺 休息 加速 減速 学習** → 空 腹 衰 弱 盲 目 混 乱 恐 怖 毒 麻 痺 休 息 加 速 減 学 習
  など 20 字前後

英大文字で `STR` などと書けば漢字はゼロで済む。

### 6.4 文字数の上限に効く原典の事情

- 原典の 1 行は 80 桁、ファミコンは実質 28〜30 桁。アイテム名
  （例: `a Two-Handed Sword (Zweihander) (+10,+10)`）もメッセージも
  **日本語化のときに短く書き直す**前提になる。
- 状態欄（原典 23 行目: `Hungry` `Blind` `Confused` `Afraid` `Poisoned`
  `Paralysed` `Rest` `Slow` `Fast` `Study`、`src/misc3.c:334-451`）は
  文字で並べる代わりに **8×8 のアイコン 10 個**にすると 1 行に収まる
  （第 7 節）。

---

## 7. 窓枠・UI 部品（8×8）

| 部品 | 数 | 備考 |
|---|---:|---|
| 窓枠（二重線） | 11 | ╔ ╗ ╚ ╝ ═ ║ ╠ ╣ ╦ ╩ ╬。第 8 節の「部屋」タイルと共用できる |
| 窓枠（一重線） | 11 | ┌ ┐ └ ┘ ─ │ ├ ┤ ┬ ┴ ┼。第 8 節の「通路」タイルと共用できる |
| カーソル | 2 | ▶（選択）、▼（原典の `-more-`、続きを待つ） |
| HP・MP ゲージ | 9 | 0/8〜8/8 の 9 段 |
| 状態異常アイコン | 10〜12 | 空腹・衰弱・盲目・混乱・恐怖・毒・麻痺・休息・探索・加速・減速・学習可能 |
| 装備欄の部位アイコン（任意） | 12 | 武器・弓・指輪 2・首・明かり・鎧・外套・盾・兜・手・靴（原典の装備欄 12 枠） |
| 手・コマンドのアイコン（任意） | — | ボタンが少ない機械なのでコマンドメニューは必須。文字だけでも作れる |

**コマンド入力について**: 原典はキー 1 つ 1 コマンドで 50 を超える
コマンドがある。ファミコンのパッドではコマンドメニュー（窓）から選ぶ
形になるので、窓枠とカーソルは最優先で要る。

---

## 8. 全体マップ（罫線タイル）

### 8.1 原典のしくみ

`screen_map()`（`src/io.c:395`）は 198×66 のダンジョンを **3×3 マスずつ
1 文字**に縮め、66×22 文字の中に描く。3×3 の中でいちばん優先度の高い
記号を代表にする（`@` 10、`<` `>` 5、`'` -3、`#` -5、`.` -10、空白 -15、
ほかは 0）。枠には IBM の罫線（201, 187, 200, 188, 205, 186 = ╔ ╗ ╚ ╝ ═ ║）を
使う。

ファミコンは 1 行 32 タイルなので、このままでは幅が足りない。

### 8.2 区画は正方形にする

ファミコン版では 1 マスを 16×16 の正方形で描くので、全体マップの区画も
**縦横同じマス数**にする。そうしないと、地図の画面と全体マップで部屋の
形が変わって見える（原典の 3×3 は、縦長の端末の文字で見て形が合っていた）。

**1 タイル（8×8）＝ ダンジョンの 4×4 マス**とする。1 マスがちょうど
2×2 ドットになるので、縮めても形が崩れず、現在地も 2 ドット単位で出せる。

ただし原典の大きさ（198×66）を 4×4 で割ると 50×17 タイルになり、横が
画面に入らない。区画を正方形にするなら、**ダンジョンの大きさのほうを
変える**（第 8.4 節）。推奨の 96×80 マスなら全体マップは **24×20 タイル**
（192×160 ドット）で、枠と見出しを付けても安全な範囲に収まる。

### 8.3 罫線タイル

区画ごとに次を決め、タイルを選ぶ。

1. **種類**: 覚えている床のうち部屋の床（`fval <= MAX_CAVE_ROOM`）か階段が
   あれば「部屋」、通路の床（`CORR_FLOOR` `BLOCKED_FLOOR`）や扉だけなら
   「通路」、どちらも無ければ「空白」。
2. **通路のつながり（上・右・下・左の 4 ビット）**: 区画の境目をはさんで、
   覚えている通れるマスどうしが接していればその方向のビットを立てる。
   隣が部屋の区画でも立てる（線が部屋の枠まで届く）。
3. **部屋の枠（4 ビット）**: 隣の区画が部屋でない向きにだけ枠線を描く。
   いくつかの区画にまたがる部屋は、枠がつながって 1 つの箱に見える。

| タイル群 | 数 | 絵 |
|---|---:|---|
| 空白（未探索） | 1 | 黒 |
| 通路 | 16 | 4 ビットの全組み合わせ: ─ │ ┌ ┐ └ ┘ ├ ┤ ┬ ┴ ┼、行き止まり 4 つ（╴ ╵ ╶ ╷）、孤立した点 ・ |
| 部屋 | 16 | 暗い色で塗った中身に、部屋でない向きだけ明るい枠線（4 ビット） |
| 枠（二重線） | 6 | ╔ ╗ ╚ ╝ ═ ║。第 7 節の窓枠と共用できる |

**BG は 39 枚**（試作では 38 枚。空白は共通の 0 番を使う）。

- **階段と現在地はスプライト**にする。最初の版では階段を BG の目印
  タイルにしていたが、そうすると区画の部屋の枠や通路の線が消える。
  スプライトなら区画の絵をそのまま残し、階段のマスを 2 ドット単位で
  指せる。1 階の階段は多くても 5〜6 個なので、横 1 列 8 個の制限にも
  かからない。
- 見えているモンスターを出すなら、同じくスプライトの点で出す。多いときは
  点滅で回す。

区画ごとの 4 ビット 2 組（種類とつながり）を「マスを覚えた瞬間」に
更新しておけば、マップを開くときは 480 バイトを並べるだけで済む。

### 8.4 ダンジョンの大きさ

原典の 198×66 は「80×24 の端末の 3 画面×3 画面」から来た大きさで、
ファミコンに合わせる理由は無い。決め手は 3 つ。

- **全体マップ**: 4×4 マス＝1 タイルで、枠と見出しを付けて安全な範囲
  （およそ 28×26 タイル）に収まること。
- **RAM**: 1 マスに要る情報は、地形 4 ビット＋フラグ 4 ビット
  （`lr` `fm` `pl` `tl`）で 1 バイト、モンスター番号（`cptr`）と物の番号
  （`tptr`）を持つと 3 バイト。カートリッジの RAM は 8KB（MMC3 など）か
  32KB（MMC1 の SXROM、MMC5 の EWROM など）。
- **遊びの量**: 原典は 13,068 マスで、部屋の数の平均は 32（`DUN_ROO_MEA`）。

| 案 | 大きさ | マス数（原典比） | 全体マップ（4×4） | RAM 1 バイト/マス | RAM 3 バイト/マス |
|---|---|---|---|---:|---:|
| 原典 | 198×66 | 13,068（100%） | 50×17 タイル（**幅が入らない**） | 12.8KB | 38.3KB |
| **A（推奨）** | **96×80** | 7,680（59%） | **24×20** | 7.5KB | 22.5KB |
| B | 112×88 | 9,856（75%） | 28×22（枠が安全域ぎりぎり） | 9.6KB | 28.9KB |
| C | 128×96 | 12,288（94%） | 32×24（画面の端まで。見切れる） | 12KB | 36KB |
| D | 80×64 | 5,120（39%） | 20×16 | 5KB | 15KB |

**A の 96×80 を推す理由**:

- 全体マップが 24×20 タイルで、二重線の枠と見出しを付けても画面の
  真ん中に余裕をもって収まる（`assets/famicom/mock_minimap.png`）。
- 32KB の RAM なら 3 バイト/マスでも 22.5KB で、残り約 9KB にモンスター
  （最大 125）・床の物（最大 175）・持ち物などが入る。8KB の RAM なら
  マスは 1 バイトにして、モンスターと物はそれぞれの一覧から位置で
  引く形になる。
- 地図の窓（16×11 マス）で横 6 画面・縦 7 画面強。原典（3×3 画面）より
  画面の枚数では広く感じる。

広さは原典の 6 割になるので、部屋の数と大きさもそろえて直す。

- **部屋の区画**: 原典は画面の半分（11×33 マス）ごとに部屋の候補地を
  置き、6×6 = 36 区画（`src/generate.c:1051` の `row_rooms` `col_rooms`）。
  これを画面の大きさ（`SCREEN_HEIGHT` `SCREEN_WIDTH`）から切りはなし、
  **16×16 マスの正方形の区画**にすると 96×80 で 6×5 = 30 区画。
- **部屋の形**: 原典の部屋は高さ 2〜8・幅 2〜23 マス
  （`build_room()` `src/generate.c:320` の `randint(4)` `randint(3)`
  `randint(11)`）。縦長の文字で見て形が整うように横に長い。正方形の
  マスではひどく平たくなるので、**横の広がりを `randint(11)` から
  `randint(5)` くらいに縮める**（幅 2〜11）。高さはそのまま。
- **部屋の数**: 平均 32 から、面積に合わせて 20 前後に。
- **町**: 原典は 66×22 マスの 1 画面。窓が 16×11 なので、32×22 マス
  （2×2 画面）に縮めて 6 軒を 3×2 に並べると歩きやすい。

---

## 9. 1 枚絵・特別な画面

| 画面 | 原典 | 必要な絵 |
|---|---|---|
| タイトル | `data/splash.hlp` | ロゴ（タイル 60〜100 枚） |
| キャラクター作成 | 種族 8・職業 6・性別・能力値ロール | 文字と窓で足りる。種族や職業の顔を出すなら 14 枚（任意） |
| キャラクター画面 | `C` | 文字と窓。プレイヤーの 16×16 を 1 つ |
| 持ち物・装備 | `i` `e` | アイテムの 16×16 を並べると見やすい（任意） |
| 店の中 | 店 6 軒 × 店主 3 人（`src/tables.c` の 18 人） | 店の看板の絵 6 枚。店主の顔 18 枚は任意 |
| 呪文・祈りの一覧 | `m` `p` | 文字と窓 |
| 全体マップ | `M` | 第 8 節 |
| 墓石 | `print_tomb()` `src/death.c:122` | 墓石と「RIP」の 1 枚絵（原典は ASCII アート）。名前・レベル・死因・日付を文字で重ねる |
| 勝利 | Balrog を倒して王位に就く（`kingly()` `src/death.c:414`。原典は ASCII アートの王冠） | 王冠の 1 枚絵 |
| 最高得点 | `scores.dat` | 文字と窓 |

---

## 10. ファミコン側の制約とタイルの載せかた

絵の枚数そのものより、**同時に何枚を画面に出せるか**が効いてくる。

- **BG タイルは 1 画面 256 枚**。16×16 の絵 1 つで 4 枚使うので、地図の
  中で同時に出せる絵は 64 種ほど。
- **スプライトは横 1 列に 8 個**。16×16 のモンスターは 2 個使うので、
  横に 4 体並ぶとちらつく。部屋にモンスターが群れる Umoria では、
  **モンスターとアイテムは BG で描き、スプライトはプレイヤー・カーソル・
  エフェクトだけ**にするのが安全。ターン制なので BG の書きかえで
  間に合う。
- **パレット**: BG 4 本 × 3 色＋共通の背景色。「地形用」「暖色」「寒色」
  「白・灰」のように割りふり、モンスターとアイテムの色違いはこの 4 本の
  中で選ぶ。階ごとにパレットの中身を入れかえてもよい。

そのうえで、タイルの持ちかたは 2 通り。

1. **CHR-RAM に「いま見えるもの」だけ送る**（UNROM・MMC1 など）。
   地形の約 30 は常駐、モンスターとアイテムは視界に入ったときに
   ROM から転送する。16×11 マスの視界に同時に出る種類は多くても
   20〜30 なので収まる。転送は 1 フレームに数枚だが、ターン制なので
   間に合う。
2. **CHR-ROM のバンク切りかえ**（MMC3）。走査線割りこみで**地図の領域と
   文字の領域のバンクを分ける**と、文字（約 200 枚）と地図の絵を別々に
   256 枚ずつ使える。モンスターは階の深さで出る顔ぶれが決まる
   （付録 A の階層）ので、深さ帯ごとにバンクを組むこともできる。
3. MMC5 を使えるなら、拡張属性モードで 8×8 ごとにタイル番号の上位
   ビットとパレットを選べるので、上の悩みはほぼ消える。

本題から外れるが、ダンジョン配列は本体 RAM 2KB に入らないので、どの
方式でもカートリッジ側に RAM が要る。大きさとの兼ねあいは第 8.4 節。

---

## 付録

以下は `src/monsters.c` と `src/treasure.c` から機械的に作った表。
名前は原典の英語のまま（`&` と `~` は外した）。

### 付録 A. モンスター全 279 種（記号別・原典の出現順。括弧内は出現階層）

| 記号 | 数 | 階層 | 全種類 |
|---|---:|---|---|
| `a` | 8 | 2–32 | Giant Black Ant(2)、Giant White Ant(3)、Giant Red Ant(9)、Giant Clear Ant(12)、Giant Ebony Ant(15)、Giant Silver Ant(23)、Giant Static Ant(30)、Giant Hunter Ant(32) |
| `b` | 16 | 3–30 | Huge Brown Bat(3)、Giant Brown Bat(6)、Giant Black Bat(9)、Giant Long-Eared Bat(13)、White Dragon Bat(14)、Giant Grey Bat(15)、Huge White Bat(15)、Giant Tan Bat(15)、Green Dragon Bat(16)、Giant Red Bat(20)、Black Dragon Bat(21)、Blue Dragon Bat(21)、Red Dragon Bat(23)、Disenchanter Bat(26)、Giant Fire Bat(29)、Giant Lightning Bat(30) |
| `c` | 9 | 1–15 | Giant Yellow Centipede(1)、Giant White Centipede(1)、Metallic Green Centipede(2)、Metallic Blue Centipede(3)、Metallic Red Centipede(3)、Giant Black Centipede(4)、Giant Blue Centipede(4)、Giant Red Centipede(10)、Giant Clear Centipede(15) |
| `d` | 12 | 29–38 | Young Blue Dragon(29)、Young White Dragon(29)、Young Green Dragon(29)、Young Black Dragon(35)、Young Red Dragon(35)、Mature White Dragon(35)、Young Multi-Hued Dragon(36)、Mature Blue Dragon(36)、Mature Green Dragon(36)、Mature Red Dragon(37)、Mature Black Dragon(37)、Mature Multi-Hued Dragon(38) |
| `e` | 4 | 1–7 | Floating Eye(1)、Radiation Eye(3)、Disenchanter Eye(5)、Bloodshot Eye(7) |
| `f` | 4 | 2–13 | Giant Green Frog(2)、Giant Black Frog(5)、Giant Red Frog(7)、Giant Red Speckled Frog(13) |
| `g` | 4 | 14–22 | Flesh Golem(14)、Clay Golem(15)、Stone Golem(19)、Iron Golem(22) |
| `h` | 4 | 2–9 | White Harpy(2)、Drooling Harpy(3)、Grey Harpy(6)、Black Harpy(9) |
| `i` | 8 | 1–9 | White Icky-Thing(1)、Clear Icky-Thing(1)、Blubbering Icky-Thing(2)、Grey Icky-Thing(5)、Blue Icky-Thing(6)、Green Icky-Thing(7)、Red Icky-Thing(8)、Bloodshot Icky-Thing(9) |
| `j` | 1 | 4–4 | Jackal(4) |
| `k` | 1 | 1–1 | Kobold(1) |
| `l` | 2 | 3–14 | Giant White Louse(3)、Giant Black Louse(14) |
| `m` | 11 | 3–27 | Yellow Mold(3)、Brown Mold(6)、Green Mold(8)、Hairy Mold(10)、Disenchanter Mold(10)、Violet Mold(15)、Red Mold(19)、Black Mold(22)、Crimson Mold(23)、Wooden Mold(25)、Shimmering Mold(27) |
| `n` | 4 | 3–15 | Black Naga(3)、Green Naga(5)、Red Naga(7)、Guardian Naga(15) |
| `o` | 7 | 6–31 | Orc(6)、Orc Shaman(9)、Orc Warrior(11)、Ogre(13)、Black Orc(13)、Uruk-Hai Orc(18)、Ogre Mage(31) |
| `p` | 29 | 0–50 | Filthy Street Urchin(0)、Blubbering Idiot(0)、Pitiful-Looking Beggar(0)、Mangy-Looking Leper(0)、Squint-Eyed Rogue(0)、Singing, Happy Drunk(0)、Mean-Looking Mercenary(0)、Battle-Scarred Veteran(0)、Novice Warrior(2)、Novice Rogue(2)、Novice Priest(2)、Novice Mage(2)、Scruffy-Looking Hobbit(3)、Greedy Little Gnome(7)、Seedy Looking Human(8)、Bandit(8)、Brigand(10)、Nasty Little Gnome(11)、Priest(12)、Swordsman(12)、Magic User(13)、Warrior(23)、Berzerker(24)、Black Knight(28)、Mage(28)、Ninja(32)、Necromancer(35)、Sorcerer(39)、Evil Iggy(50) |
| `q` | 2 | 16–40 | Quasit(16)、Death Quasit(40) |
| `r` | 8 | 1–29 | Giant White Mouse(1)、Silver Mouse(4)、Giant White Rat(4)、Giant Grey Rat(9)、Vorpal Bunny(11)、Giant Black Rat(16)、Giant Spotted Rat(19)、Giant Glowing Rat(29) |
| `s` | 6 | 5–33 | Skeleton Kobold(5)、Skeleton Orc(8)、Skeleton Human(12)、Skeleton Hobgoblin(14)、Skeleton Troll(30)、Skeleton 2-Headed Troll(33) |
| `t` | 4 | 10–26 | Giant White Tick(10)、Giant Yellow Tick(15)、Giant Brown Tick(25)、Giant Fire Tick(26) |
| `w` | 7 | 1–40 | White Worm mass(1)、Green Worm mass(2)、Yellow Worm mass(3)、Blue Worm mass(4)、Red Worm mass(5)、Giant Purple Worm(29)、Disenchanter Worm(40) |
| `y` | 5 | 2–12 | Blue Yeek(2)、Black Yeek(5)、Brown Yeek(8)、Clear Yeek(9)、Master Yeek(12) |
| `z` | 3 | 7–12 | Zombie Kobold(7)、Orc Zombie(11)、Human Zombie(12) |
| `A` | 5 | 26–36 | Giant Grey Ant Lion(26)、Giant White Ant Lion(30)、Giant Black Ant Lion(31)、Giant Red Ant Lion(35)、Giant Mottled Ant Lion(36) |
| `B` | 1 | 100–100 | Balrog(100) |
| `C` | 1 | 16–16 | Gelatinous Cube(16) |
| `D` | 6 | 38–40 | Ancient White Dragon(38)、Ancient Blue Dragon(39)、Ancient Green Dragon(39)、Ancient Black Dragon(39)、Ancient Red Dragon(40)、Ancient Multi-Hued Dragon(40) |
| `E` | 8 | 12–34 | Air Spirit(12)、Water Spirit(17)、Earth Spirit(17)、Fire Spirit(18)、Invisible Stalker(32)、Water Elemental(33)、Fire Elemental(33)、Earth Elemental(34) |
| `F` | 10 | 5–27 | Giant House Fly(5)、Giant Green Fly(7)、Giant Fruit Fly(10)、Giant Gnat(13)、Giant Flea(14)、Giant White Dragon Fly(14)、Giant Green Dragon Fly(16)、Giant Black Dragon Fly(20)、Giant Blue Dragon Fly(25)、Giant Red Dragon Fly(27) |
| `G` | 7 | 3–34 | Poltergeist(3)、Green Glutton Ghost(5)、Lost Soul(7)、Moaning Spirit(12)、Banshee(24)、Ghost(31)、Spirit Troll(34) |
| `H` | 1 | 11–11 | Hobgoblin(11) |
| `J` | 8 | 2–12 | White Jelly(2)、Yellow Jelly(3)、Blue Jelly(4)、Green Jelly(5)、Silver Jelly(5)、Rot Jelly(5)、Red Jelly(7)、Grape Jelly(12) |
| `K` | 11 | 13–37 | Killer Brown Beetle(13)、Killer Green Beetle(14)、Killer Black Beetle(19)、Killer Boring Beetle(21)、Killer Stag Beetle(22)、Killer Blue Beetle(23)、Killer Red Beetle(25)、Killer Fire Beetle(27)、Killer Slicer Beetle(30)、Death Watch Beetle(31)、Iridescent Beetle(37) |
| `L` | 3 | 34–40 | Lich(34)、King Lich(37)、Emperor Lich(40) |
| `M` | 4 | 19–35 | Mummified Kobold(19)、Mummified Orc(21)、Mummified Human(24)、Mummified Troll(35) |
| `O` | 6 | 19–40 | Grey Ooze(19)、Disenchanter Ooze(19)、Green Ooze(22)、Black Ooze(23)、Clear Ooze(26)、Crystal Ooze(40) |
| `P` | 5 | 14–20 | Hill Giant(14)、Frost Giant(15)、Fire Giant(16)、Stone Giant(18)、Cloud Giant(20) |
| `Q` | 2 | 20–40 | Quylthulg(20)、Rotting Quylthulg(40) |
| `R` | 9 | 1–12 | Large Brown Snake(1)、Large White Snake(1)、Large Black Snake(2)、Large Green Snake(3)、Large Grey Snake(4)、Copperhead Snake(5)、Rattlesnake(6)、King Cobra(9)、Black Mamba(12) |
| `S` | 4 | 17–34 | Giant Brown Scorpion(17)、Giant Yellow Scorpion(22)、Giant Black Scorpion(26)、Giant Red Scorpion(34) |
| `T` | 4 | 17–32 | Troll(17)、Giant Troll(25)、Ice Troll(28)、Two-Headed Troll(32) |
| `U` | 1 | 16–16 | Umber Hulk(16) |
| `V` | 3 | 27–37 | Vampire(27)、Master Vampire(34)、King Vampire(37) |
| `W` | 8 | 24–39 | Forest Wight(24)、White Wraith(26)、Grave Wight(30)、Barrow Wight(33)、Grey Wraith(36)、Emperor Wight(38)、Black Wraith(38)、Nether Wraith(39) |
| `X` | 1 | 36–36 | Xorn(36) |
| `Y` | 1 | 9–9 | Yeti(9) |
| `$` | 3 | 4–10 | Creeping Copper Coins(4)、Creeping Silver Coins(6)、Creeping Gold Coins(10) |
| `,` | 8 | 1–10 | Grey Mushroom patch(1)、Shrieker Mushroom patch(2)、Yellow Mushroom patch(2)、Spotted Mushroom patch(3)、Black Mushroom patch(4)、White Mushroom patch(5)、Purple Mushroom patch(6)、Clear Mushroom patch(10) |

### 付録 B. 物・地形オブジェクト全 420 件（`object_list`、tval と記号の組ごと）

| tval | 記号 | 件数 | 名前（重複はまとめた） |
|---|---|---:|---|
| TV_FOOD | `,` | 34 | Poison、Blindness、Paranoia、Confusion、Hallucination、Cure Poison、Cure Blindness、Cure Paranoia、Cure Confusion、Weakness、Unhealth、Restore Constitution、First-Aid、Minor Cures、Light Cures、Restoration、Major Cures、Ration of Food、Slime Mold、Piece of Elvish Waybread、Hard Biscuit、Strip of Beef Jerky、Pint of Fine Ale、Pint of Fine Wine、Pint of Fine Grade Mush |
| TV_SWORD | `\|` | 24 | Dagger (Main Gauche)、Dagger (Misericorde)、Dagger (Stiletto)、Dagger (Bodkin)、Broken Dagger、Backsword、Bastard Sword、Thrusting Sword (Bilbo)、Thrusting Sword (Baselard)、Broadsword、Two-Handed Sword (Claymore)、Cutlass、Two-Handed Sword (Espadon)、Executioner's Sword、Two-Handed Sword (Flamberge)、Foil、Katana、Longsword、Two-Handed Sword (No-Dachi)、Rapier、Sabre、Small Sword、Two-Handed Sword (Zweihander)、Broken Sword |
| TV_HAFTED | `\` | 9 | Ball and Chain、Cat-o'-Nine-Tails、Wooden Club、Flail、Two-Handed Great Flail、Morningstar、Mace、War Hammer、Lead-Filled Mace |
| TV_POLEARM | `/` | 13 | Awl-Pike、Beaked Axe、Fauchard、Glaive、Halberd、Lucerne Hammer、Pike、Spear、Lance、Javelin、Battle Axe (Balestarius)、Battle Axe (European)、Broad Axe |
| TV_BOW | `}` | 6 | Short Bow、Long Bow、Composite Bow、Light Crossbow、Heavy Crossbow、Sling |
| TV_ARROW | `{` | 2 | Arrow |
| TV_BOLT | `{` | 2 | Bolt |
| TV_SLING_AMMO | `{` | 2 | Rounded Pebble、Iron Shot |
| TV_SPIKE | `~` | 1 | Iron Spike |
| TV_LIGHT | `~` | 4 | Brass Lantern、Wooden Torch |
| TV_DIGGING | `\` | 6 | Orcish Pick、Dwarven Pick、Gnomish Shovel、Dwarven Shovel、Pick、Shovel |
| TV_BOOTS | `]` | 3 | Pair of Soft Leather Shoes、Pair of Soft Leather Boots、Pair of Hard Leather Boots |
| TV_HELM | `]` | 8 | Soft Leather Cap、Hard Leather Cap、Metal Cap、Iron Helm、Steel Helm、Silver Crown、Golden Crown、Jewel-Encrusted Crown |
| TV_SOFT_ARMOR | `(` | 9 | Robe、Soft Leather Armor、Soft Studded Leather、Hard Leather Armor、Hard Studded Leather、Woven Cord Armor、Soft Leather Ring Mail、Hard Leather Ring Mail、Leather Scale Mail |
| TV_HARD_ARMOR | `[` | 12 | Metal Scale Mail、Chain Mail、Rusty Chain Mail、Double Chain Mail、Augmented Chain Mail、Bar Chain Mail、Metal Brigandine Armor、Laminated Armor、Partial Plate Armor、Metal Lamellar Armor、Full Plate Armor、Ribbed Plate Armor |
| TV_CLOAK | `(` | 1 | Cloak |
| TV_GLOVES | `]` | 2 | Set of Leather Gloves、Set of Gauntlets |
| TV_SHIELD | `)` | 6 | Small Leather Shield、Medium Leather Shield、Large Leather Shield、Small Metal Shield、Medium Metal Shield、Large Metal Shield |
| TV_RING | `=` | 30 | Strength、Dexterity、Constitution、Intelligence、Speed、Searching、Teleportation、Slow Digestion、Resist Fire、Resist Cold、Feather Falling、Adornment、Weakness、Lordly Protection (FIRE)、Lordly Protection (ACID)、Lordly Protection (COLD)、WOE、Stupidity、Increase Damage、Increase To-Hit、Protection、Aggravate Monster、See Invisible、Sustain Strength、Sustain Intelligence、Sustain Wisdom、Sustain Constitution、Sustain Dexterity、Sustain Charisma、Slaying |
| TV_AMULET | `"` | 9 | Wisdom、Charisma、Searching、Teleportation、Slow Digestion、Resist Acid、Adornment、the Magi、DOOM |
| TV_SCROLL1 | `?` | 46 | Enchant Weapon To-Hit、Enchant Weapon To-Dam、Enchant Armor、Identify、Remove Curse、Light、Summon Monster、Phase Door、Teleport、Teleport Level、Monster Confusion、Magic Mapping、Sleep Monster、Rune of Protection、Treasure Detection、Object Detection、Trap Detection、Door/Stair Location、Mass Genocide、Detect Invisible、Aggravate Monster、Trap Creation、Trap/Door Destruction、Door Creation、Recharging、Genocide、Darkness、Protection from Evil、Create Food、Dispel Undead |
| TV_SCROLL2 | `?` | 12 | *Enchant Weapon*、Curse Weapon、*Enchant Armor*、Curse Armor、Summon Undead、Blessing、Holy Chant、Holy Prayer、Word-of-Recall、*Destruction* |
| TV_POTION1 | `!` | 33 | Slime Mold Juice、Apple Juice、Water、Strength、Weakness、Restore Strength、Intelligence、Lose Intelligence、Restore Intelligence、Wisdom、Lose Wisdom、Restore Wisdom、Charisma、Ugliness、Restore Charisma、Cure Light Wounds、Cure Serious Wounds、Cure Critical Wounds、Healing、Constitution、Gain Experience、Sleep、Blindness、Confusion、Poison、Haste Self、Slowness、Dexterity、Restore Dexterity、Restore Constitution |
| TV_POTION2 | `!` | 17 | Lose Experience、Salt Water、Invulnerability、Heroism、Super Heroism、Boldness、Restore Life Levels、Resist Heat、Resist Cold、Detect Invisible、Slow Poison、Neutralize Poison、Restore Mana、Infra-Vision |
| TV_FLASK | `!` | 2 | Flask of Oil |
| TV_WAND | `-` | 24 | Light、Lightning Bolts、Frost Bolts、Fire Bolts、Stone-to-Mud、Polymorph、Heal Monster、Haste Monster、Slow Monster、Confuse Monster、Sleep Monster、Drain Life、Trap/Door Destruction、Magic Missile、Wall Building、Clone Monster、Teleport Away、Disarming、Lightning Balls、Cold Balls、Fire Balls、Stinking Cloud、Acid Balls、Wonder |
| TV_STAFF | `_` | 25 | Light、Door/Stair Location、Trap Location、Treasure Location、Object Location、Teleportation、Earthquakes、Summoning、*Destruction*、Starlight、Haste Monsters、Slow Monsters、Sleep Monsters、Cure Light Wounds、Detect Invisible、Speed、Slowness、Mass Polymorph、Remove Curse、Detect Evil、Curing、Dispel Evil、Darkness |
| TV_MAGIC_BOOK | `?` | 4 | [Beginners-Magick]、[Magick I]、[Magick II]、[The Mages' Guide to Power] |
| TV_PRAYER_BOOK | `?` | 4 | [Beginners Handbook]、[Words of Wisdom]、[Chants and Blessings]、[Exorcisms and Dispellings] |
| TV_CHEST | `&` | 7 | Small Wooden Chest、Large Wooden Chest、Small Iron Chest、Large Iron Chest、Small Steel Chest、Large Steel Chest、ruined chest |
| TV_MISC | `s` | 8 | Rat Skeleton、Giant Centipede Skeleton、Human Skeleton、Dwarf Skeleton、Elf Skeleton、Gnome Skeleton、broken set of teeth、large broken bone |
| TV_SOFT_ARMOR | `~` | 1 | some Filthy Rags |
| TV_MISC | `!` | 1 | empty bottle |
| TV_MISC | `~` | 2 | some shards of pottery、broken stick |
| TV_OPEN_DOOR | `'` | 1 | open door |
| TV_CLOSED_DOOR | `+` | 1 | closed door |
| TV_SECRET_DOOR | `#` | 1 | secret door |
| TV_UP_STAIR | `<` | 1 | an up staircase |
| TV_DOWN_STAIR | `>` | 1 | a down staircase |
| TV_STORE_DOOR | `1` | 1 | General Store |
| TV_STORE_DOOR | `2` | 1 | Armory |
| TV_STORE_DOOR | `3` | 1 | Weapon Smiths |
| TV_STORE_DOOR | `4` | 1 | Temple |
| TV_STORE_DOOR | `5` | 1 | Alchemy Shop |
| TV_STORE_DOOR | `6` | 1 | Magic Shop |
| TV_VIS_TRAP | 空白 | 1 | an open pit |
| TV_INVIS_TRAP | `^` | 16 | an arrow trap、a covered pit、a trap door、a gas trap、a dart trap、a strange rune、some loose rock、a blackened spot、some corroded rock |
| TV_INVIS_TRAP | `;` | 1 | a loose rock |
| TV_RUBBLE | `:` | 1 | some rubble |
| TV_VIS_TRAP | `^` | 1 | a strange rune |
| TV_GOLD | `$` | 11 | copper、silver、gold、mithril |
| TV_GOLD | `*` | 7 | garnets、opals、sapphires、rubies、diamonds、emeralds |
| TV_NOTHING | 空白 | 2 | nothing、（空） |
