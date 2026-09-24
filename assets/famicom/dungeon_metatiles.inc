; scripts/famicom/make_dungeon_tiles.py が生成。手で直さないこと
; メタタイル: 左上・右上・左下・右下のタイル番号, 属性（パレット番号）
; 壁の _nesw は床に面した辺（n 上・e 右・s 下・w 左、0 は面していない）
; パレット（深さ帯ごと。各行の先頭が共通の背景色＝床）:
;   B1-10   P0=$36,$35,$38,$0F  P1=$36,$0F,$17,$27  P2=$36,$0F,$00,$30  P3=$36,$0F,$1A,$2A
;   B11-20  P0=$10,$00,$37,$0F  P1=$10,$0F,$08,$18  P2=$10,$0F,$00,$30  P3=$10,$0F,$1A,$2A
;   B21-30  P0=$3B,$2B,$3A,$0F  P1=$3B,$0F,$0B,$1A  P2=$3B,$0F,$00,$30  P3=$3B,$0F,$1A,$2A
;   B31-40  P0=$31,$21,$32,$0F  P1=$31,$0F,$02,$12  P2=$31,$0F,$00,$30  P3=$31,$0F,$1A,$2A
;   B41-    P0=$06,$07,$08,$0F  P1=$06,$0F,$00,$10  P2=$06,$0F,$00,$30  P3=$06,$0F,$1A,$2A

MT_GRANITE_0 = 0  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: 0）
MT_GRANITE_N = 1  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: n）
MT_GRANITE_E = 2  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: e）
MT_GRANITE_NE = 3  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: ne）
MT_GRANITE_S = 4  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: s）
MT_GRANITE_NS = 5  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: ns）
MT_GRANITE_ES = 6  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: es）
MT_GRANITE_NES = 7  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: nes）
MT_GRANITE_W = 8  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: w）
MT_GRANITE_NW = 9  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: nw）
MT_GRANITE_EW = 10  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: ew）
MT_GRANITE_NEW = 11  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: new）
MT_GRANITE_SW = 12  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: sw）
MT_GRANITE_NSW = 13  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: nsw）
MT_GRANITE_ESW = 14  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: esw）
MT_GRANITE_NESW = 15  ; 花崗岩の壁。外周の壁と隠し扉も同じ絵（床に面した辺: nesw）
MT_GRANITE2_0 = 16  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: 0）
MT_GRANITE2_N = 17  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: n）
MT_GRANITE2_E = 18  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: e）
MT_GRANITE2_NE = 19  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: ne）
MT_GRANITE2_S = 20  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: s）
MT_GRANITE2_NS = 21  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: ns）
MT_GRANITE2_ES = 22  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: es）
MT_GRANITE2_NES = 23  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: nes）
MT_GRANITE2_W = 24  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: w）
MT_GRANITE2_NW = 25  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: nw）
MT_GRANITE2_EW = 26  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: ew）
MT_GRANITE2_NEW = 27  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: new）
MT_GRANITE2_SW = 28  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: sw）
MT_GRANITE2_NSW = 29  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: nsw）
MT_GRANITE2_ESW = 30  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: esw）
MT_GRANITE2_NESW = 31  ; 花崗岩の壁（模様 2。位置で交互に使う）（床に面した辺: nesw）
MT_MAGMA_0 = 32  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: 0）
MT_MAGMA_N = 33  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: n）
MT_MAGMA_E = 34  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: e）
MT_MAGMA_NE = 35  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: ne）
MT_MAGMA_S = 36  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: s）
MT_MAGMA_NS = 37  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: ns）
MT_MAGMA_ES = 38  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: es）
MT_MAGMA_NES = 39  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: nes）
MT_MAGMA_W = 40  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: w）
MT_MAGMA_NW = 41  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: nw）
MT_MAGMA_EW = 42  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: ew）
MT_MAGMA_NEW = 43  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: new）
MT_MAGMA_SW = 44  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: sw）
MT_MAGMA_NSW = 45  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: nsw）
MT_MAGMA_ESW = 46  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: esw）
MT_MAGMA_NESW = 47  ; 溶岩の鉱脈（黒い岩に光る筋）（床に面した辺: nesw）
MT_QUARTZ_0 = 48  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: 0）
MT_QUARTZ_N = 49  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: n）
MT_QUARTZ_E = 50  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: e）
MT_QUARTZ_NE = 51  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: ne）
MT_QUARTZ_S = 52  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: s）
MT_QUARTZ_NS = 53  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: ns）
MT_QUARTZ_ES = 54  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: es）
MT_QUARTZ_NES = 55  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: nes）
MT_QUARTZ_W = 56  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: w）
MT_QUARTZ_NW = 57  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: nw）
MT_QUARTZ_EW = 58  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: ew）
MT_QUARTZ_NEW = 59  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: new）
MT_QUARTZ_SW = 60  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: sw）
MT_QUARTZ_NSW = 61  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: nsw）
MT_QUARTZ_ESW = 62  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: esw）
MT_QUARTZ_NESW = 63  ; 石英の鉱脈（灰色の岩に結晶）（床に面した辺: nesw）
MT_MAGMA_T_0 = 64  ; 溶岩の鉱脈＋宝（床に面した辺: 0）
MT_MAGMA_T_N = 65  ; 溶岩の鉱脈＋宝（床に面した辺: n）
MT_MAGMA_T_E = 66  ; 溶岩の鉱脈＋宝（床に面した辺: e）
MT_MAGMA_T_NE = 67  ; 溶岩の鉱脈＋宝（床に面した辺: ne）
MT_MAGMA_T_S = 68  ; 溶岩の鉱脈＋宝（床に面した辺: s）
MT_MAGMA_T_NS = 69  ; 溶岩の鉱脈＋宝（床に面した辺: ns）
MT_MAGMA_T_ES = 70  ; 溶岩の鉱脈＋宝（床に面した辺: es）
MT_MAGMA_T_NES = 71  ; 溶岩の鉱脈＋宝（床に面した辺: nes）
MT_MAGMA_T_W = 72  ; 溶岩の鉱脈＋宝（床に面した辺: w）
MT_MAGMA_T_NW = 73  ; 溶岩の鉱脈＋宝（床に面した辺: nw）
MT_MAGMA_T_EW = 74  ; 溶岩の鉱脈＋宝（床に面した辺: ew）
MT_MAGMA_T_NEW = 75  ; 溶岩の鉱脈＋宝（床に面した辺: new）
MT_MAGMA_T_SW = 76  ; 溶岩の鉱脈＋宝（床に面した辺: sw）
MT_MAGMA_T_NSW = 77  ; 溶岩の鉱脈＋宝（床に面した辺: nsw）
MT_MAGMA_T_ESW = 78  ; 溶岩の鉱脈＋宝（床に面した辺: esw）
MT_MAGMA_T_NESW = 79  ; 溶岩の鉱脈＋宝（床に面した辺: nesw）
MT_QUARTZ_T_0 = 80  ; 石英の鉱脈＋宝（床に面した辺: 0）
MT_QUARTZ_T_N = 81  ; 石英の鉱脈＋宝（床に面した辺: n）
MT_QUARTZ_T_E = 82  ; 石英の鉱脈＋宝（床に面した辺: e）
MT_QUARTZ_T_NE = 83  ; 石英の鉱脈＋宝（床に面した辺: ne）
MT_QUARTZ_T_S = 84  ; 石英の鉱脈＋宝（床に面した辺: s）
MT_QUARTZ_T_NS = 85  ; 石英の鉱脈＋宝（床に面した辺: ns）
MT_QUARTZ_T_ES = 86  ; 石英の鉱脈＋宝（床に面した辺: es）
MT_QUARTZ_T_NES = 87  ; 石英の鉱脈＋宝（床に面した辺: nes）
MT_QUARTZ_T_W = 88  ; 石英の鉱脈＋宝（床に面した辺: w）
MT_QUARTZ_T_NW = 89  ; 石英の鉱脈＋宝（床に面した辺: nw）
MT_QUARTZ_T_EW = 90  ; 石英の鉱脈＋宝（床に面した辺: ew）
MT_QUARTZ_T_NEW = 91  ; 石英の鉱脈＋宝（床に面した辺: new）
MT_QUARTZ_T_SW = 92  ; 石英の鉱脈＋宝（床に面した辺: sw）
MT_QUARTZ_T_NSW = 93  ; 石英の鉱脈＋宝（床に面した辺: nsw）
MT_QUARTZ_T_ESW = 94  ; 石英の鉱脈＋宝（床に面した辺: esw）
MT_QUARTZ_T_NESW = 95  ; 石英の鉱脈＋宝（床に面した辺: nesw）
MT_VOID = 96  ; 未探索・闇（P0 の黒）
MT_FLOOR_A = 97  ; 床（さざ波 A）
MT_FLOOR_B = 98  ; 床（さざ波 B）
MT_FLOOR_PAVED = 99  ; 床（敷石）。部屋に多め
MT_FLOOR_PAVED2 = 100  ; 床（敷石が欠けた所）
MT_FLOOR_PEBBLE = 101  ; 床（小石）。P1
MT_SHADOW_N = 102  ; 床＋上の壁の影
MT_SHADOW_W = 103  ; 床＋左の壁の影
MT_SHADOW_NW = 104  ; 床＋上と左の壁の影
MT_SHADOW_C = 105  ; 床＋左上の角の影
MT_DOOR_CLOSED_H = 106  ; 閉じた扉（横の壁）。鍵・つっかえも同じ
MT_DOOR_CLOSED_V = 107  ; 閉じた扉（縦の壁）
MT_DOOR_OPEN_H = 108  ; 開いた扉・壊れた扉（横の壁）
MT_DOOR_OPEN_V = 109  ; 開いた扉・壊れた扉（縦の壁）
MT_STAIRS_UP = 110  ; 上り階段 <
MT_STAIRS_DOWN = 111  ; 下り階段 >
MT_RUBBLE = 112  ; 瓦礫 :
MT_TRAP_PIT = 113  ; 落とし穴（開いた・覆われた）
MT_TRAP_DOOR = 114  ; 落とし戸
MT_TRAP_DART = 115  ; 矢・吹き矢の罠（穴の空いた板）
MT_TRAP_GAS = 116  ; ガスの罠（通気口）
MT_TRAP_RUNE = 117  ; 不思議なルーン（テレポート・召喚）
MT_TRAP_FIRE = 118  ; 焦げ跡（火炎の罠）
MT_TRAP_ROCK = 119  ; 落石の罠（ゆるんだ岩）
MT_TRAP_ACID = 120  ; 酸の罠（腐食した岩）
MT_GLYPH = 121  ; 守りのルーン（Scare Monster）
MT_EX_JELLY = 122  ; 見本: ゼリー J
MT_EX_GOLD = 123  ; 見本: 金貨の山 $（這う硬貨と共用）
MT_EX_POTION = 124  ; 見本: 薬瓶 !

metatile_tiles:
    .byte $01,$02,$03,$04  ; granite_0
    .byte $05,$06,$03,$04  ; granite_n
    .byte $01,$07,$03,$08  ; granite_e
    .byte $05,$09,$03,$08  ; granite_ne
    .byte $01,$02,$0A,$0B  ; granite_s
    .byte $05,$06,$0A,$0B  ; granite_ns
    .byte $01,$07,$0A,$0C  ; granite_es
    .byte $05,$09,$0A,$0C  ; granite_nes
    .byte $0D,$02,$0E,$04  ; granite_w
    .byte $0F,$06,$0E,$04  ; granite_nw
    .byte $0D,$07,$0E,$08  ; granite_ew
    .byte $0F,$09,$0E,$08  ; granite_new
    .byte $0D,$02,$10,$0B  ; granite_sw
    .byte $0F,$06,$10,$0B  ; granite_nsw
    .byte $0D,$07,$10,$0C  ; granite_esw
    .byte $0F,$09,$10,$0C  ; granite_nesw
    .byte $11,$12,$13,$14  ; granite2_0
    .byte $15,$16,$13,$14  ; granite2_n
    .byte $11,$17,$13,$18  ; granite2_e
    .byte $15,$19,$13,$18  ; granite2_ne
    .byte $11,$12,$1A,$1B  ; granite2_s
    .byte $15,$16,$1A,$1B  ; granite2_ns
    .byte $11,$17,$1A,$1C  ; granite2_es
    .byte $15,$19,$1A,$1C  ; granite2_nes
    .byte $1D,$12,$1E,$14  ; granite2_w
    .byte $1F,$16,$1E,$14  ; granite2_nw
    .byte $1D,$17,$1E,$18  ; granite2_ew
    .byte $1F,$19,$1E,$18  ; granite2_new
    .byte $1D,$12,$20,$1B  ; granite2_sw
    .byte $1F,$16,$20,$1B  ; granite2_nsw
    .byte $1D,$17,$20,$1C  ; granite2_esw
    .byte $1F,$19,$20,$1C  ; granite2_nesw
    .byte $21,$22,$23,$24  ; magma_0
    .byte $25,$26,$23,$24  ; magma_n
    .byte $21,$27,$23,$28  ; magma_e
    .byte $25,$29,$23,$28  ; magma_ne
    .byte $21,$22,$2A,$2B  ; magma_s
    .byte $25,$26,$2A,$2B  ; magma_ns
    .byte $21,$27,$2A,$2C  ; magma_es
    .byte $25,$29,$2A,$2C  ; magma_nes
    .byte $2D,$22,$2E,$24  ; magma_w
    .byte $2F,$26,$2E,$24  ; magma_nw
    .byte $2D,$27,$2E,$28  ; magma_ew
    .byte $2F,$29,$2E,$28  ; magma_new
    .byte $2D,$22,$30,$2B  ; magma_sw
    .byte $2F,$26,$30,$2B  ; magma_nsw
    .byte $2D,$27,$30,$2C  ; magma_esw
    .byte $2F,$29,$30,$2C  ; magma_nesw
    .byte $31,$32,$33,$34  ; quartz_0
    .byte $35,$36,$33,$34  ; quartz_n
    .byte $31,$37,$33,$38  ; quartz_e
    .byte $35,$39,$33,$38  ; quartz_ne
    .byte $31,$32,$3A,$3B  ; quartz_s
    .byte $35,$36,$3A,$3B  ; quartz_ns
    .byte $31,$37,$3A,$3C  ; quartz_es
    .byte $35,$39,$3A,$3C  ; quartz_nes
    .byte $3D,$32,$3E,$34  ; quartz_w
    .byte $3F,$36,$3E,$34  ; quartz_nw
    .byte $3D,$37,$3E,$38  ; quartz_ew
    .byte $3F,$39,$3E,$38  ; quartz_new
    .byte $3D,$32,$40,$3B  ; quartz_sw
    .byte $3F,$36,$40,$3B  ; quartz_nsw
    .byte $3D,$37,$40,$3C  ; quartz_esw
    .byte $3F,$39,$40,$3C  ; quartz_nesw
    .byte $41,$22,$23,$42  ; magma_t_0
    .byte $43,$44,$23,$42  ; magma_t_n
    .byte $41,$45,$23,$46  ; magma_t_e
    .byte $43,$47,$23,$46  ; magma_t_ne
    .byte $41,$22,$48,$49  ; magma_t_s
    .byte $43,$44,$48,$49  ; magma_t_ns
    .byte $41,$45,$48,$4A  ; magma_t_es
    .byte $43,$47,$48,$4A  ; magma_t_nes
    .byte $4B,$22,$4C,$42  ; magma_t_w
    .byte $4D,$44,$4C,$42  ; magma_t_nw
    .byte $4B,$45,$4C,$46  ; magma_t_ew
    .byte $4D,$47,$4C,$46  ; magma_t_new
    .byte $4B,$22,$4E,$49  ; magma_t_sw
    .byte $4D,$44,$4E,$49  ; magma_t_nsw
    .byte $4B,$45,$4E,$4A  ; magma_t_esw
    .byte $4D,$47,$4E,$4A  ; magma_t_nesw
    .byte $4F,$32,$33,$50  ; quartz_t_0
    .byte $51,$52,$33,$50  ; quartz_t_n
    .byte $4F,$53,$33,$54  ; quartz_t_e
    .byte $51,$55,$33,$54  ; quartz_t_ne
    .byte $4F,$32,$56,$57  ; quartz_t_s
    .byte $51,$52,$56,$57  ; quartz_t_ns
    .byte $4F,$53,$56,$58  ; quartz_t_es
    .byte $51,$55,$56,$58  ; quartz_t_nes
    .byte $59,$32,$5A,$50  ; quartz_t_w
    .byte $5B,$52,$5A,$50  ; quartz_t_nw
    .byte $59,$53,$5A,$54  ; quartz_t_ew
    .byte $5B,$55,$5A,$54  ; quartz_t_new
    .byte $59,$32,$5C,$57  ; quartz_t_sw
    .byte $5B,$52,$5C,$57  ; quartz_t_nsw
    .byte $59,$53,$5C,$58  ; quartz_t_esw
    .byte $5B,$55,$5C,$58  ; quartz_t_nesw
    .byte $5D,$5D,$5D,$5D  ; void
    .byte $5E,$5F,$60,$61  ; floor_a
    .byte $62,$63,$64,$65  ; floor_b
    .byte $66,$66,$66,$66  ; floor_paved
    .byte $67,$61,$00,$66  ; floor_paved2
    .byte $68,$69,$6A,$6B  ; floor_pebble
    .byte $6C,$6D,$60,$61  ; shadow_n
    .byte $6E,$5F,$6F,$61  ; shadow_w
    .byte $70,$6D,$6F,$61  ; shadow_nw
    .byte $71,$5F,$60,$61  ; shadow_c
    .byte $72,$73,$74,$75  ; door_closed_h
    .byte $76,$77,$78,$79  ; door_closed_v
    .byte $7A,$7B,$7C,$7D  ; door_open_h
    .byte $7E,$7F,$80,$81  ; door_open_v
    .byte $82,$83,$84,$85  ; stairs_up
    .byte $86,$87,$88,$89  ; stairs_down
    .byte $8A,$8B,$8C,$8D  ; rubble
    .byte $8E,$8F,$90,$91  ; trap_pit
    .byte $92,$93,$94,$95  ; trap_door
    .byte $96,$97,$98,$99  ; trap_dart
    .byte $9A,$9B,$9C,$9D  ; trap_gas
    .byte $9E,$9F,$A0,$A1  ; trap_rune
    .byte $A2,$A3,$A4,$A5  ; trap_fire
    .byte $A6,$A7,$A8,$A9  ; trap_rock
    .byte $AA,$AB,$AC,$AD  ; trap_acid
    .byte $AE,$AF,$B0,$B1  ; glyph
    .byte $B2,$B3,$B4,$B5  ; ex_jelly
    .byte $B6,$B7,$B8,$B9  ; ex_gold
    .byte $BA,$BB,$BC,$BD  ; ex_potion

metatile_attr:
    .byte 1  ; granite_0
    .byte 1  ; granite_n
    .byte 1  ; granite_e
    .byte 1  ; granite_ne
    .byte 1  ; granite_s
    .byte 1  ; granite_ns
    .byte 1  ; granite_es
    .byte 1  ; granite_nes
    .byte 1  ; granite_w
    .byte 1  ; granite_nw
    .byte 1  ; granite_ew
    .byte 1  ; granite_new
    .byte 1  ; granite_sw
    .byte 1  ; granite_nsw
    .byte 1  ; granite_esw
    .byte 1  ; granite_nesw
    .byte 1  ; granite2_0
    .byte 1  ; granite2_n
    .byte 1  ; granite2_e
    .byte 1  ; granite2_ne
    .byte 1  ; granite2_s
    .byte 1  ; granite2_ns
    .byte 1  ; granite2_es
    .byte 1  ; granite2_nes
    .byte 1  ; granite2_w
    .byte 1  ; granite2_nw
    .byte 1  ; granite2_ew
    .byte 1  ; granite2_new
    .byte 1  ; granite2_sw
    .byte 1  ; granite2_nsw
    .byte 1  ; granite2_esw
    .byte 1  ; granite2_nesw
    .byte 1  ; magma_0
    .byte 1  ; magma_n
    .byte 1  ; magma_e
    .byte 1  ; magma_ne
    .byte 1  ; magma_s
    .byte 1  ; magma_ns
    .byte 1  ; magma_es
    .byte 1  ; magma_nes
    .byte 1  ; magma_w
    .byte 1  ; magma_nw
    .byte 1  ; magma_ew
    .byte 1  ; magma_new
    .byte 1  ; magma_sw
    .byte 1  ; magma_nsw
    .byte 1  ; magma_esw
    .byte 1  ; magma_nesw
    .byte 2  ; quartz_0
    .byte 2  ; quartz_n
    .byte 2  ; quartz_e
    .byte 2  ; quartz_ne
    .byte 2  ; quartz_s
    .byte 2  ; quartz_ns
    .byte 2  ; quartz_es
    .byte 2  ; quartz_nes
    .byte 2  ; quartz_w
    .byte 2  ; quartz_nw
    .byte 2  ; quartz_ew
    .byte 2  ; quartz_new
    .byte 2  ; quartz_sw
    .byte 2  ; quartz_nsw
    .byte 2  ; quartz_esw
    .byte 2  ; quartz_nesw
    .byte 1  ; magma_t_0
    .byte 1  ; magma_t_n
    .byte 1  ; magma_t_e
    .byte 1  ; magma_t_ne
    .byte 1  ; magma_t_s
    .byte 1  ; magma_t_ns
    .byte 1  ; magma_t_es
    .byte 1  ; magma_t_nes
    .byte 1  ; magma_t_w
    .byte 1  ; magma_t_nw
    .byte 1  ; magma_t_ew
    .byte 1  ; magma_t_new
    .byte 1  ; magma_t_sw
    .byte 1  ; magma_t_nsw
    .byte 1  ; magma_t_esw
    .byte 1  ; magma_t_nesw
    .byte 2  ; quartz_t_0
    .byte 2  ; quartz_t_n
    .byte 2  ; quartz_t_e
    .byte 2  ; quartz_t_ne
    .byte 2  ; quartz_t_s
    .byte 2  ; quartz_t_ns
    .byte 2  ; quartz_t_es
    .byte 2  ; quartz_t_nes
    .byte 2  ; quartz_t_w
    .byte 2  ; quartz_t_nw
    .byte 2  ; quartz_t_ew
    .byte 2  ; quartz_t_new
    .byte 2  ; quartz_t_sw
    .byte 2  ; quartz_t_nsw
    .byte 2  ; quartz_t_esw
    .byte 2  ; quartz_t_nesw
    .byte 0  ; void
    .byte 0  ; floor_a
    .byte 0  ; floor_b
    .byte 0  ; floor_paved
    .byte 0  ; floor_paved2
    .byte 1  ; floor_pebble
    .byte 0  ; shadow_n
    .byte 0  ; shadow_w
    .byte 0  ; shadow_nw
    .byte 0  ; shadow_c
    .byte 1  ; door_closed_h
    .byte 1  ; door_closed_v
    .byte 1  ; door_open_h
    .byte 1  ; door_open_v
    .byte 0  ; stairs_up
    .byte 0  ; stairs_down
    .byte 1  ; rubble
    .byte 0  ; trap_pit
    .byte 0  ; trap_door
    .byte 0  ; trap_dart
    .byte 0  ; trap_gas
    .byte 0  ; trap_rune
    .byte 0  ; trap_fire
    .byte 1  ; trap_rock
    .byte 3  ; trap_acid
    .byte 1  ; glyph
    .byte 3  ; ex_jelly
    .byte 1  ; ex_gold
    .byte 2  ; ex_potion

; 全体マップ用 8x8（全体マップ画面では黒地のパレットに入れかえる）
MM_BLANK = $00
MM_CORR_00 = $BE
MM_CORR_01 = $BF
MM_CORR_02 = $C0
MM_CORR_03 = $C1
MM_CORR_04 = $C2
MM_CORR_05 = $C3
MM_CORR_06 = $C4
MM_CORR_07 = $C5
MM_CORR_08 = $C6
MM_CORR_09 = $C7
MM_CORR_10 = $C8
MM_CORR_11 = $C9
MM_CORR_12 = $CA
MM_CORR_13 = $CB
MM_CORR_14 = $CC
MM_CORR_15 = $CD
MM_ROOM_00 = $CE
MM_ROOM_01 = $CF
MM_ROOM_02 = $D0
MM_ROOM_03 = $D1
MM_ROOM_04 = $D2
MM_ROOM_05 = $D3
MM_ROOM_06 = $D4
MM_ROOM_07 = $D5
MM_ROOM_08 = $D6
MM_ROOM_09 = $D7
MM_ROOM_10 = $D8
MM_ROOM_11 = $D9
MM_ROOM_12 = $DA
MM_ROOM_13 = $DB
MM_ROOM_14 = $DC
MM_ROOM_15 = $DD
MM_FRAME_TL = $DE
MM_FRAME_TR = $DF
MM_FRAME_BL = $E0
MM_FRAME_BR = $E1
MM_FRAME_H = $E2
MM_FRAME_V = $E3
; 全体マップのスプライト（階段）は BG とは別に持つ

; 文字（黒地に白。パレット P2）
CH_SPACE = $CE
CH_SLASH = $E4
CH_0 = $E5
CH_1 = $E6
CH_2 = $E7
CH_3 = $E8
CH_4 = $E9
CH_5 = $EA
CH_6 = $EB
CH_7 = $EC
CH_8 = $ED
CH_9 = $EE
CH_A = $EF
CH_B = $F0
CH_F = $F1
CH_H = $F2
CH_L = $F3
CH_M = $F4
CH_P = $F5
CH_U = $F6
CH_V = $F7
