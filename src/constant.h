// Copyright (c) 1989-94 James E. Wilson, Robert A. Koeneke
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Global constants used by Moria

// Current version number of Moria
#ifndef CONSTANT_H
#define CONSTANT_H

#define CUR_VERSION_MAJ  5
#define CUR_VERSION_MIN  7
#define PATCH_LEVEL      0

#define MAX_UCHAR         255
#define MAX_SHORT       32767   // maximum short/long signed ints

// Note to the Wizard:
//
// Tweaking these constants can *GREATLY* change the game.
// Two years of constant tuning have generated these
// values.  Minor adjustments are encouraged, but you must
// be very careful not to unbalance the game.  Moria was
// meant to be challenging, not a give away.  Many
// adjustments can cause the game to act strangely, or even
// cause errors. -RAK-

// Addendum:
// I have greatly expanded the number of defined constants.  However, if
// you change anything below, without understanding EXACTLY how the game
// uses the number, the program may stop working correctly.  Modify the
// constants at your own risk. -JEW-

// message line location
#define MSG_LINE 0

// number of messages to save in a buffer
#define MAX_SAVE_MSG    22   // How many messages to save -CJS-

// Dungeon size parameters
#define MAX_HEIGHT      66   // Multiple of 11; >= 22
#define MAX_WIDTH      198   // Multiple of 33; >= 66
#define SCREEN_HEIGHT   22
#define SCREEN_WIDTH    66

// Store constants
#define MAX_OWNERS               18   // Number of owners to choose from
#define MAX_STORES                6   // Number of different stores
#define STORE_INVEN_MAX          24   // Max number of discrete objs in inven
#define STORE_CHOICES            26   // NUMBER of items to choose stock from

// Treasure constants
#define INVEN_ARRAY_SIZE         34   // Size of inventory array(Do not change)
#define MAX_OBJ_LEVEL            50   // Maximum level of magic in dungeon

// Note that the following constants are all related, if you change one, you
// must also change all succeeding ones. Also, player_init[] and store_choice[]
// may also have to be changed.
#define MAX_OBJECTS             420   // Number of objects for universe
#define MAX_DUNGEON_OBJ         344   // Number of dungeon objects
#define OBJ_NOTHING             417
#define OBJECT_IDENT_SIZE       448   // 7 groups of 64 secret kinds; see item_knowledge.h

// with MAX_TALLOC 150, it is possible to get compacting objects during
// level generation, although it is extremely rare
#define MAX_TALLOC              175   // Max objects per level
#define MIN_TRIX                  1   // Minimum t_list index used

// Creature constants
#define MAX_CREATURES           279   // Number of creatures defined for univ

// with MAX_MALLOC 101, it is possible to get compacting monsters messages
// while breeding/cloning monsters
#define MAX_MALLOC              125   // Max that can be allocated
#define MAX_MONS_LEVEL           40   // Maximum level of creatures
#define MAX_SIGHT                20   // Maximum dis a creature can be seen
#define MAX_MON_MULT             75   // Maximum reproductions on a level
#define WIN_MON_TOT               2   // Total number of "win" creatures
#define WIN_MON_APPEAR           50   // Level where winning creatures begin
#define MON_SUMMON_ADJ            2   // Adjust level of summoned creatures
#define MAX_MON_NATTACK           4   // Max num attacks (used in mons memory) -CJS-
#define MIN_MONIX                 2   // Lowest row the monster list hands out (1 = the player, 0 = no mon)

#define SCARE_MONSTER            99

// Descriptive constants
#define MAX_COLORS               49   // Used with potions
#define MAX_MUSH                 22   // Used with mushrooms
#define MAX_WOODS                25   // Used with staffs
#define MAX_METALS               25   // Used with wands
#define MAX_ROCKS                32   // Used with rings
#define MAX_AMULETS              11   // Used with amulets
#define MAX_SYLLABLES           153   // Used with scrolls

// Player constants
#define MAX_PLAYER_LEVEL         40   // Maximum possible character level
#define MAX_EXP             9999999L  // Maximum amount of experience -CJS-
#define MAX_RACES                 8   // Number of defined races
#define MAX_CLASS                 6   // Number of defined classes
#define MAX_BACKGROUND          128   // Number of types of histories for univ
#define PLAYER_FOOD_MAX       15000   // Maximum food value, beyond is wasted
#define PLAYER_REGEN_NORMAL     197   // Regen factor*2^16 when full
#define PLAYER_REGEN_HPBASE    1442   // Min amount hp regen*2^16
#define PLAYER_REGEN_MNBASE     524   // Min amount mana regen*2^16

// class level adjustment constants
#define CLA_BTH                   0
#define CLA_BTHB                  1
#define CLA_DEVICE                2
#define CLA_DISARM                3
#define CLA_SAVE                  4
// this depends on the fact that CLA_SAVE values are all the same, if not,
// then should add a separate column for this
#define CLA_MISC_HIT              4
#define MAX_LEV_ADJ               5

// Base to hit constants
#define BTH_PLUS_ADJ              3   // Adjust BTH per plus-to-hit

// magic numbers for players inventory array
#define INVEN_WIELD              22   // must be first item in equipment list
#define INVEN_HEAD               23
#define INVEN_NECK               24
#define INVEN_BODY               25
#define INVEN_ARM                26
#define INVEN_HANDS              27
#define INVEN_RIGHT              28
#define INVEN_LEFT               29
#define INVEN_FEET               30
#define INVEN_OUTER              31
#define INVEN_LIGHT              32
#define INVEN_AUX                33

// Attribute indexes -CJS-
#define A_STR 0
#define A_INT 1
#define A_WIS 2
#define A_DEX 3
#define A_CON 4
#define A_CHR 5

// some systems have a non-ANSI definition of this, so undef it first
#undef CTRL_KEY
#define CTRL_KEY(x)   ((x) & 0x1F)

#undef DELETE
#define DELETE    0x7f

#undef ESCAPE
#define ESCAPE    '\033'  // ESCAPE character -CJS-

// This used to be NULL, but that was technically incorrect.
// CNIL is used instead of null to help avoid lint errors.
#ifndef CNIL
#define CNIL (char *)0
#endif

// Fval definitions: these describe the various types of dungeon floors and
// walls, if numbers above 15 are ever used, then the test against MIN_CAVE_WALL
// will have to be changed, also the save routines will have to be changed.
#define NULL_WALL            0
#define DARK_FLOOR           1
#define LIGHT_FLOOR          2
#define CORR_FLOOR           3
#define BLOCKED_FLOOR        4 // a corridor space with cl/st/se door or rubble
#define MAX_CAVE_FLOOR       4

#define MAX_OPEN_SPACE       3
#define MIN_CLOSED_SPACE     4

#define MIN_CAVE_WALL       12
#define GRANITE_WALL        12
#define MAGMA_WALL          13
#define QUARTZ_WALL         14
#define BOUNDARY_WALL       15

// Class spell types
#define NONE                 0
#define MAGE                 1
#define PRIEST               2

// definitions for the player's status field
#define PY_HUNGRY       0x00000001L
#define PY_WEAK         0x00000002L
#define PY_BLIND        0x00000004L
#define PY_CONFUSED     0x00000008L
#define PY_FEAR         0x00000010L
#define PY_POISONED     0x00000020L
#define PY_FAST         0x00000040L
#define PY_SLOW         0x00000080L
#define PY_SEARCH       0x00000100L
#define PY_REST         0x00000200L
#define PY_STUDY        0x00000400L

#define PY_INVULN       0x00001000L
#define PY_HERO         0x00002000L
#define PY_SHERO        0x00004000L
#define PY_BLESSED      0x00008000L
#define PY_DET_INV      0x00010000L
#define PY_TIM_INFRA    0x00020000L
#define PY_SPEED        0x00040000L
#define PY_STR_WGT      0x00080000L
#define PY_PARALYSED    0x00100000L
#define PY_REPEAT       0x00200000L
#define PY_ARMOR        0x00400000L

#define PY_STATS        0x3F000000L
#define PY_STR          0x01000000L // these 6 stat flags must be adjacent
#define PY_INT          0x02000000L
#define PY_WIS          0x04000000L
#define PY_DEX          0x08000000L
#define PY_CON          0x10000000L
#define PY_CHR          0x20000000L

#define PY_HP           0x40000000L
#define PY_MANA         0x80000000L

// definitions for objects that can be worn
#define TR_STR          0x00000001L
#define TR_SEARCH       0x00000040L
#define TR_SLOW_DIGEST  0x00000080L
#define TR_STEALTH      0x00000100L
#define TR_AGGRAVATE    0x00000200L
#define TR_TELEPORT     0x00000400L
#define TR_REGEN        0x00000800L
#define TR_SPEED        0x00001000L

#define TR_SLAY_DRAGON  0x00002000L
#define TR_SLAY_ANIMAL  0x00004000L
#define TR_SLAY_EVIL    0x00008000L
#define TR_SLAY_UNDEAD  0x00010000L
#define TR_FROST_BRAND  0x00020000L
#define TR_FLAME_TONGUE 0x00040000L

#define TR_RES_FIRE     0x00080000L
#define TR_RES_ACID     0x00100000L
#define TR_RES_COLD     0x00200000L
#define TR_SUST_STAT    0x00400000L
#define TR_FREE_ACT     0x00800000L
#define TR_SEE_INVIS    0x01000000L
#define TR_RES_LIGHT    0x02000000L
#define TR_FFALL        0x04000000L
#define TR_INFRA        0x40000000L
#define TR_CURSED       0x80000000L

// definitions for creatures, cmove field
#define CM_ALL_MV_FLAGS 0x0000003FL
#define CM_ATTACK_ONLY  0x00000001L
#define CM_MOVE_NORMAL  0x00000002L
// For Quylthulgs, which have no physical movement.
#define CM_ONLY_MAGIC   0x00000004L

#define CM_INVISIBLE    0x00010000L

#define CM_SMALL_OBJ    0x00800000L
#define CM_CARRY_OBJ    0x01000000L
#define CM_CARRY_GOLD   0x02000000L
#define CM_TREASURE     0x7C000000L
#define CM_TR_SHIFT             26  // used for recall of treasure
#define CM_60_RANDOM    0x04000000L
#define CM_90_RANDOM    0x08000000L
#define CM_1D2_OBJ      0x10000000L
#define CM_2D2_OBJ      0x20000000L
#define CM_4D2_OBJ      0x40000000L
#define CM_WIN          0x80000000L

// creature spell definitions
#define CS_FREQ         0x0000000FL
#define CS_TEL_LONG     0x00000020L
#define CS_TEL_TO       0x00000040L
#define CS_LGHT_WND     0x00000080L
#define CS_SER_WND      0x00000100L
#define CS_HOLD_PER     0x00000200L
#define CS_BLIND        0x00000400L
#define CS_CONFUSE      0x00000800L
#define CS_FEAR         0x00001000L
#define CS_SUMMON_MON   0x00002000L
#define CS_SUMMON_UND   0x00004000L
#define CS_SLOW_PER     0x00008000L
#define CS_DRAIN_MANA   0x00010000L

#define CS_BR_LIGHT     0x00080000L // if no spell frequency set

// creature defense flags
#define CD_DRAGON       0x0001
#define CD_ANIMAL       0x0002
#define CD_EVIL         0x0004
#define CD_UNDEAD       0x0008
#define CD_FROST        0x0010
#define CD_FIRE         0x0020

#define CD_NO_SLEEP     0x1000
#define CD_INFRA        0x2000
#define CD_MAX_HP       0x4000

// inventory stacking subvals
// these never stack
#define ITEM_NEVER_STACK_MIN    0
#define ITEM_NEVER_STACK_MAX   63
// these items always stack with others of same subval, always treated as
// single objects, must be power of 2
#define ITEM_SINGLE_STACK_MIN  64
#define ITEM_SINGLE_STACK_MAX 192  // see NOTE below
// these items stack with others only if have same subval and same p1,
// they are treated as a group for wielding, etc.
#define ITEM_GROUP_MIN        192
#define ITEM_GROUP_MAX        255

// NOTE: items with subval 192 are treated as single objects,
// but only stack with others of same subval if have the same
// p1 value, only used for torches.

// id's used for object description, stored in the per-kind records; see
// item_ident.h
#define OD_KNOWN1       0x2

// id's used for item description, stored in i_ptr->ident
#define ID_MAGIK        0x1
#define ID_DAMD         0x2
#define ID_EMPTY        0x4
#define ID_KNOWN2       0x8
#define ID_STOREBOUGHT  0x10
#define ID_SHOW_HITDAM  0x20
#define ID_NOSHOW_P1    0x40
#define ID_SHOW_P1      0x80

// indexes into the special name table
#define SN_NULL                0
#define SN_ARRAY_SIZE         56 // must be at end of this list

// defines for treasure type values (tval)
#define TV_NEVER              (-1) // used by find_range() for non-search
#define TV_NOTHING             0
#define TV_MISC                1
#define TV_CHEST               2

// items tested for enchantments, i.e. the MAGIK inscription,
// see the enchanted() procedure.
#define TV_MIN_ENCHANT        10
#define TV_SLING_AMMO         10
#define TV_BOLT               11
#define TV_ARROW              12
#define TV_SPIKE              13
#define TV_LIGHT              15
#define TV_BOW                20
#define TV_HAFTED             21
#define TV_POLEARM            22
#define TV_SWORD              23
#define TV_DIGGING            25
#define TV_BOOTS              30
#define TV_GLOVES             31
#define TV_CLOAK              32
#define TV_HELM               33
#define TV_SHIELD             34
#define TV_HARD_ARMOR         35
#define TV_SOFT_ARMOR         36
// max tval that uses the TR_* flags
#define TV_MAX_ENCHANT        39
#define TV_AMULET             40
#define TV_RING               45
// max tval for wearable items
#define TV_MAX_WEAR           50
#define TV_STAFF              55
#define TV_WAND               65
#define TV_SCROLL1            70
#define TV_SCROLL2            71
#define TV_POTION1            75
#define TV_POTION2            76
#define TV_FLASK              77
#define TV_FOOD               80
#define TV_MAGIC_BOOK         90
#define TV_PRAYER_BOOK        91
// objects with tval above this are never picked up by monsters
#define TV_MAX_OBJECT         99
#define TV_GOLD              100
// objects with higher tvals can not be picked up
#define TV_MAX_PICK_UP       100
#define TV_INVIS_TRAP        101
// objects between TV_MIN_VISIBLE and TV_MAX_VISIBLE are always visible,
// i.e. the cave fm flag is set when they are present
#define TV_MIN_VISIBLE       102
#define TV_VIS_TRAP          102
#define TV_RUBBLE            103
// following objects are never deleted when trying to create another one
// during level generation
#define TV_MIN_DOORS         104
#define TV_OPEN_DOOR         104
#define TV_CLOSED_DOOR       105
#define TV_UP_STAIR          107
#define TV_DOWN_STAIR        108
#define TV_SECRET_DOOR       109
#define TV_STORE_DOOR        110
#define TV_MAX_VISIBLE       110

// spell types used by get_flags(), breathe(), fire_bolt() and fire_ball()
#define GF_MAGIC_MISSILE       0
#define GF_LIGHTNING           1
#define GF_POISON_GAS          2
#define GF_ACID                3
#define GF_FROST               4
#define GF_FIRE                5
#define GF_HOLY_ORB            6

#endif // CONSTANT_H
