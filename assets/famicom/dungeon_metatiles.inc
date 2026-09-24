; scripts/famicom/make_dungeon_tiles.py が生成。手で直さないこと
; メタタイル: 左上・右上・左下・右下のタイル番号, 属性（パレット番号）
; パレット: P0=$0F,$0C,$1C,$2C  P1=$0F,$07,$17,$27  P2=$0F,$00,$10,$30  P3=$0F,$0A,$1A,$2A

MT_VOID = 0  ; 未探索・闇
MT_FLOOR_ROOM = 1  ; 部屋の床（目地つき）
MT_FLOOR_CORR = 2  ; 通路の床（小石）
MT_WALL_TOP = 3  ; 花崗岩の壁（上面・ブロック）。隠し扉も同じ絵
MT_WALL_FACE = 4  ; 花崗岩の壁（床に面した正面・レンガ）
MT_PERM_TOP = 5  ; 外周の壁（上面）
MT_PERM_FACE = 6  ; 外周の壁（正面）
MT_MAGMA_TOP = 7  ; 溶岩の鉱脈（上面）
MT_MAGMA_FACE = 8  ; 溶岩の鉱脈（正面）
MT_MAGMA_TOP_T = 9  ; 溶岩の鉱脈＋宝（上面）
MT_MAGMA_FACE_T = 10  ; 溶岩の鉱脈＋宝（正面）
MT_QUARTZ_TOP = 11  ; 石英の鉱脈（上面）
MT_QUARTZ_FACE = 12  ; 石英の鉱脈（正面）
MT_QUARTZ_TOP_T = 13  ; 石英の鉱脈＋宝（上面）
MT_QUARTZ_FACE_T = 14  ; 石英の鉱脈＋宝（正面）
MT_DOOR_CLOSED_H = 15  ; 閉じた扉（横の壁）。鍵・つっかえも同じ
MT_DOOR_CLOSED_V = 16  ; 閉じた扉（縦の壁）
MT_DOOR_OPEN_H = 17  ; 開いた扉・壊れた扉（横の壁）
MT_DOOR_OPEN_V = 18  ; 開いた扉・壊れた扉（縦の壁）
MT_STAIRS_UP = 19  ; 上り階段 <
MT_STAIRS_DOWN = 20  ; 下り階段 >
MT_RUBBLE = 21  ; 瓦礫 :
MT_TRAP_PIT = 22  ; 落とし穴（開いた・覆われた）
MT_TRAP_DOOR = 23  ; 落とし戸
MT_TRAP_DART = 24  ; 矢・吹き矢の罠（穴の空いた板）
MT_TRAP_GAS = 25  ; ガスの罠（通気口）
MT_TRAP_RUNE = 26  ; 不思議なルーン（テレポート・召喚）
MT_TRAP_FIRE = 27  ; 焦げ跡（火炎の罠）
MT_TRAP_ROCK = 28  ; 落石の罠（ゆるんだ岩）
MT_TRAP_ACID = 29  ; 酸の罠（腐食した岩）
MT_GLYPH = 30  ; 守りのルーン（Scare Monster）
MT_EX_JELLY = 31  ; 見本: ゼリー J（モンスターも BG・目地つき）
MT_EX_GOLD = 32  ; 見本: 金貨の山 $（這う硬貨と共用）
MT_EX_POTION = 33  ; 見本: 薬瓶 !

metatile_tiles:
    .byte $00,$00,$00,$00  ; void
    .byte $01,$02,$03,$04  ; floor_room
    .byte $05,$06,$07,$08  ; floor_corr
    .byte $09,$0A,$0B,$0C  ; wall_top
    .byte $0D,$0D,$0E,$0E  ; wall_face
    .byte $0F,$10,$11,$12  ; perm_top
    .byte $13,$14,$15,$16  ; perm_face
    .byte $17,$18,$19,$1A  ; magma_top
    .byte $1B,$1C,$1D,$1E  ; magma_face
    .byte $17,$1F,$20,$21  ; magma_top_t
    .byte $1B,$22,$23,$24  ; magma_face_t
    .byte $25,$26,$27,$28  ; quartz_top
    .byte $29,$2A,$2B,$2C  ; quartz_face
    .byte $25,$2D,$2E,$2F  ; quartz_top_t
    .byte $29,$30,$31,$32  ; quartz_face_t
    .byte $33,$34,$35,$36  ; door_closed_h
    .byte $37,$38,$39,$3A  ; door_closed_v
    .byte $3B,$3C,$3D,$3E  ; door_open_h
    .byte $3F,$40,$41,$42  ; door_open_v
    .byte $43,$44,$45,$46  ; stairs_up
    .byte $47,$48,$49,$4A  ; stairs_down
    .byte $4B,$4C,$4D,$4E  ; rubble
    .byte $4F,$50,$51,$52  ; trap_pit
    .byte $53,$54,$55,$56  ; trap_door
    .byte $57,$58,$59,$5A  ; trap_dart
    .byte $5B,$5C,$5D,$5E  ; trap_gas
    .byte $5F,$60,$61,$62  ; trap_rune
    .byte $63,$64,$65,$66  ; trap_fire
    .byte $67,$68,$69,$6A  ; trap_rock
    .byte $6B,$6C,$6D,$6E  ; trap_acid
    .byte $6F,$70,$71,$72  ; glyph
    .byte $73,$74,$75,$76  ; ex_jelly
    .byte $77,$78,$79,$7A  ; ex_gold
    .byte $7B,$7C,$7D,$7E  ; ex_potion

metatile_attr:
    .byte 0  ; void
    .byte 0  ; floor_room
    .byte 0  ; floor_corr
    .byte 0  ; wall_top
    .byte 0  ; wall_face
    .byte 0  ; perm_top
    .byte 0  ; perm_face
    .byte 1  ; magma_top
    .byte 1  ; magma_face
    .byte 1  ; magma_top_t
    .byte 1  ; magma_face_t
    .byte 2  ; quartz_top
    .byte 2  ; quartz_face
    .byte 2  ; quartz_top_t
    .byte 2  ; quartz_face_t
    .byte 1  ; door_closed_h
    .byte 1  ; door_closed_v
    .byte 1  ; door_open_h
    .byte 1  ; door_open_v
    .byte 0  ; stairs_up
    .byte 0  ; stairs_down
    .byte 0  ; rubble
    .byte 0  ; trap_pit
    .byte 1  ; trap_door
    .byte 2  ; trap_dart
    .byte 2  ; trap_gas
    .byte 2  ; trap_rune
    .byte 1  ; trap_fire
    .byte 0  ; trap_rock
    .byte 3  ; trap_acid
    .byte 1  ; glyph
    .byte 3  ; ex_jelly
    .byte 1  ; ex_gold
    .byte 2  ; ex_potion

; 全体マップ用 8x8（パレット P0）
MM_BLANK = $00
MM_CORR_00 = $7F
MM_CORR_01 = $80
MM_CORR_02 = $81
MM_CORR_03 = $82
MM_CORR_04 = $83
MM_CORR_05 = $84
MM_CORR_06 = $85
MM_CORR_07 = $86
MM_CORR_08 = $87
MM_CORR_09 = $88
MM_CORR_10 = $89
MM_CORR_11 = $8A
MM_CORR_12 = $8B
MM_CORR_13 = $8C
MM_CORR_14 = $8D
MM_CORR_15 = $8E
MM_ROOM_00 = $8F
MM_ROOM_01 = $90
MM_ROOM_02 = $91
MM_ROOM_03 = $92
MM_ROOM_04 = $93
MM_ROOM_05 = $94
MM_ROOM_06 = $95
MM_ROOM_07 = $96
MM_ROOM_08 = $97
MM_ROOM_09 = $98
MM_ROOM_10 = $99
MM_ROOM_11 = $9A
MM_ROOM_12 = $9B
MM_ROOM_13 = $9C
MM_ROOM_14 = $9D
MM_ROOM_15 = $9E
MM_FRAME_TL = $9F
MM_FRAME_TR = $A0
MM_FRAME_BL = $A1
MM_FRAME_BR = $A2
MM_FRAME_H = $A3
MM_FRAME_V = $A4
; 全体マップのスプライト（階段）は BG とは別に持つ

; 文字（パレット P2）
CH_SPACE = $00
CH_SLASH = $A5
CH_0 = $A6
CH_1 = $A7
CH_2 = $A8
CH_3 = $A9
CH_4 = $AA
CH_5 = $AB
CH_6 = $AC
CH_7 = $AD
CH_8 = $AE
CH_9 = $AF
CH_A = $B0
CH_B = $B1
CH_F = $B2
CH_H = $B3
CH_L = $B4
CH_M = $B5
CH_P = $B6
CH_U = $B7
CH_V = $B8
