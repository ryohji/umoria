// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Global type declarations

#include "headers.h"

// some machines will not accept 'signed char' as a type, and some accept it
// but still treat it like an unsigned character, let's just avoid it,
// any variable which can ever hold a negative value must be 16 or 32 bits

#define VTYPESIZ 80
#define BIGVTYPESIZ 160
typedef char vtype[VTYPESIZ];
// note that since its output can easily exceed 80 characters, objdes must
// always be called with a bigvtype as the first paramter
typedef char bigvtype[BIGVTYPESIZ];

// 文字列を「Your %s glows faintly!」のような文に埋めこんだ結果を入れる。
// 埋めこみ先を埋めこむ文字列と同じ大きさ（bigvtype や vtype）にすると、
// 中身が長いときに配列の外へ書く。-Wformat-overflow が指していたのはこれ。
// 前後に付く語の分だけ余分にとってあるので、切り詰めも溢れも起こらない。
//
// 埋めこむ文字列でもっとも長いのは objdes() が返すアイテム説明（bigvtype）
// なので、それに 1 行分（vtype 1 つ分 = 画面 1 行）を足した大きさにする。
// この種の文で前後に付く語はどれも 1 行に収まる。
#define MSGTYPESIZ (BIGVTYPESIZ + VTYPESIZ)
typedef char msgtype[MSGTYPESIZ];

typedef char stat_type[7];

// Many of the character fields used to be fixed length, which greatly
// increased the size of the executable.  I have replaced many fixed
// length fields with variable length ones.
//
// all fields are given the smallest possbile type, and all fields are
// aligned within the structure to their natural size boundary, so that
// the structures contain no padding and are minimum size.
//
// bit fields are only used where they would cause a large reduction in
// data size, they should not be used otherwise because their use
// results in larger and slower code.

// handle to struct m_attack_type
typedef struct attack_handle {
    const uint8_t place;
} attack_handle;

typedef struct creature_type {
    const char *const name;                      // Descrip of creature
    const uint32_t cmove;                        // Bit field
    const uint32_t spells;                       // Creature spells
    const uint16_t cdefense;                     // Bit field
    const uint16_t mexp;                         // Exp value for kill
    const uint8_t sleep;                         // Inactive counter/10
    const uint8_t aaf;                           // Area affect radius
    const uint8_t ac;                            // AC
    const uint8_t speed;                         // Movement speed+10 (NOTE: +10 so that it can be a uint8_t)
    const uint8_t cchar;                         // Character rep.
    const uint8_t hd[2];                         // Creatures hit die
    const attack_handle attack[MAX_MON_NATTACK]; // Type attack and damage
    const uint8_t level;                         // Level of creature
} creature_type;

// Monster memories. -CJS-
typedef struct recall_type {
    uint32_t r_cmove;
    uint32_t r_spells;
    uint16_t r_kills, r_deaths;
    uint16_t r_cdefense;
    uint8_t r_wake, r_ignore;
    uint8_t r_attacks[MAX_MON_NATTACK];
} recall_type;

typedef struct {
    uint16_t place;
} creature_handle;

// モンスター定義表を末尾から先頭へたどる反復子。
//
// 持つのは「指したい要素そのもの」ではなく「その 1 つ先」（base）。
// std::reverse_iterator と同じ持ちかたで、先頭を指す状態でも base は先頭に
// 留まるので、配列の直前を指すポインタが現れない。C17 6.5.6p8 が認めるのは
// 「同じ配列の要素、または末尾の 1 つ先」までで、それより手前は参照しなくても
// 値を作った時点で未定義動作になる。以前の実装は終端に c_list - 1 を使って
// いた（-Warray-bounds が指していたのはこれ）。
//
// creature_type * を裸で持ちまわらず構造体に包んでいるのは、1 つずれた値を
// うっかり -> で読めないようにするため。要素を得るには
// monster_creature_rget() を通す必要がある。
typedef struct {
    creature_type *base;
} creature_rev_iterator;

typedef struct monster_type {
    int16_t hp;               // Hit points
    int16_t csleep;           // Inactive counter
    int16_t cspeed;           // Movement speed
    creature_handle creature; // Pointer into creature

    // Note: fy, fx, and cdis constrain dungeon size to less than 256 by 256
    uint8_t fy;   // Y Pointer into map
    uint8_t fx;   // X Pointer into map
    uint8_t cdis; // Cur dis from player

    uint8_t ml;
    uint8_t stunned;
    uint8_t confused;
} monster_type;

typedef struct treasure_type {
    const char *name;  // Object name
    uint32_t flags;    // Special flags
    uint8_t tval;      // Category number
    uint8_t tchar;     // Character representation
    int16_t p1;        // Misc. use variable
    int32_t cost;      // Cost of item
    uint8_t subval;    // Sub-category number
    uint8_t number;    // Number of items
    uint16_t weight;   // Weight
    int16_t tohit;     // Plusses to hit
    int16_t todam;     // Plusses to damage
    int16_t ac;        // Normal AC
    int16_t toac;      // Plusses to AC
    uint8_t damage[2]; // Damage when hits
    uint8_t level;     // Level item first found
} treasure_type;

// only damage, ac, and tchar are constant; level could possibly be made
// constant by changing index instead; all are used rarely.
//
// extra fields x and y for location in dungeon would simplify pusht().
//
// making inscrip a pointer and mallocing space does not work, there are
// two many places where inven_types are copied, which results in dangling
// pointers, so we use a char array for them instead
#define INSCRIP_SIZE 13 // notice alignment, must be 4*x + 1
typedef struct inven_type {
    uint16_t index;             // Index to object_list
    uint8_t name2;              // Object special name
    char inscrip[INSCRIP_SIZE]; // Object inscription
    uint32_t flags;             // Special flags
    uint8_t tval;               // Category number
    uint8_t tchar;              // Character representation
    int16_t p1;                 // Misc. use variable
    int32_t cost;               // Cost of item
    uint8_t subval;             // Sub-category number
    uint8_t number;             // Number of items
    uint16_t weight;            // Weight
    int16_t tohit;              // Plusses to hit
    int16_t todam;              // Plusses to damage
    int16_t ac;                 // Normal AC
    int16_t toac;               // Plusses to AC
    uint8_t damage[2];          // Damage when hits
    uint8_t level;              // Level item first found
    uint8_t ident;              // Identify information
} inven_type;

#define PLAYER_NAME_SIZE 27

typedef struct player_type {
    struct misc {
        char name[PLAYER_NAME_SIZE]; // Name of character
        uint8_t male;                // Sex of character
        // The purse moved to player_gold.c (au, #18-12-1C). How much gold is
        // carried is the first question to leave this struct: the windows are
        // in player_gold.h, and no caller needs the number's address any more
        // (the save file's reader takes it through a local).
        //
        // How far the character has come moved to player_level.c (max_exp, exp,
        // exp_frac, lev and expfact, #18-12-6C). Five fields answered one
        // question -- the level is stored and yet it is fixed by the experience,
        // so the promise between them now has one home. The windows are in
        // player_level.h; the save file's reader takes all five through locals.
        uint16_t age;                // Characters age
        uint16_t ht;                 // Height
        uint16_t wt;                 // Weight
        uint16_t max_dlv;            // Max level explored
        int16_t srh;                 // Chance in search
        int16_t fos;                 // Frenq of search
        int16_t bth;                 // Base to hit
        int16_t bthb;                // BTH with bows
        int16_t ptohit;              // Plusses to hit
        int16_t ptodam;              // Plusses to dam
        int16_t pac;                 // Total AC
        int16_t ptoac;               // Magical AC
        int16_t disarm;              // % to Disarm
        int16_t save;                // Saving throw
        int16_t sc;                  // Social Class
        int16_t stl;                 // Stealth factor
        uint8_t pclass;              // # of class
        uint8_t prace;               // # of race
        uint8_t hitdie;              // Char hit die
        char history[4][60];         // History record
    } misc;

    // Stats now kept in arrays, for more efficient access. -CJS-
    struct stats {
        uint8_t max_stat[6]; // What is restored
        uint8_t cur_stat[6]; // What is natural
        int16_t mod_stat[6]; // What is modified, may be +/-
        uint8_t use_stat[6]; // What is used
    } stats;

    struct flags {
        // The status word left here in #18-12-7C: thirty bits, asked and
        // answered from a hundred and five places, now behind
        // player_status_flags.h. The counters below are a different question --
        // most of the marks that lived in the word pair with one of them.
        // The eighteen counters left here in #18-12-9C: how much longer each
        // temporary state lasts, asked and answered from two hundred and
        // eighty-one places, now behind player_timed_effects.h. Twelve of them
        // pair with a mark in the status word above, and that pairing is the
        // reason they moved together. The six below stayed: `rest` is WHAT the
        // character is doing rather than a state wearing off, `protection` is
        // never read outside the save file, `speed` and `see_infra` are amounts
        // rather than clocks, and the stomach's two belong to player_food.h
        // (still here only because the save file keeps them in this run).
        int16_t rest;            // Rest counter
        int16_t food;            // Food counter
        int16_t food_digested;   // Food per round
        int16_t protection;      // Protection fr. evil
        int16_t speed;           // Cur speed adjust
        int16_t see_infra;       // See warm creatures
        // Seventeen one-byte fields left here in #18-12-8C: what the character
        // can do and resist because of what is being worn -- sees invisible,
        // never paralyzed, the four resistances and falling, slow digestion,
        // regeneration, random teleportation, aggravation and the six sustained
        // stats. They were asked and answered from a hundred and six places and
        // are now behind player_abilities.h, which WORKS THEM OUT FROM THE
        // EQUIPMENT rather than remembering them. The two bytes below stayed:
        // neither is granted by equipment and calc_bonuses() does not touch
        // either.
        uint8_t confuse_monster; // Glowing hands.
        uint8_t new_spells;      // Number of spells can learn.
    } flags;
} player_type;

// spell name is stored in spell_names[] array at index i, +31 if priest
typedef struct spell_type {
    uint8_t slevel;
    uint8_t smana;
    uint8_t sfail;
    uint8_t sexp; // 1/4 of exp gained for learning spell
} spell_type;

typedef struct race_type {
    const char *trace; // Type of race
    int16_t str_adj; // adjustments
    int16_t int_adj;
    int16_t wis_adj;
    int16_t dex_adj;
    int16_t con_adj;
    int16_t chr_adj;
    uint8_t b_age;   // Base age of character
    uint8_t m_age;   // Maximum age of character
    uint8_t m_b_ht;  // base height for males
    uint8_t m_m_ht;  // mod height for males
    uint8_t m_b_wt;  // base weight for males
    uint8_t m_m_wt;  // mod weight for males
    uint8_t f_b_ht;  // base height females
    uint8_t f_m_ht;  // mod height for females
    uint8_t f_b_wt;  // base weight for female
    uint8_t f_m_wt;  // mod weight for females
    int16_t b_dis;   // base chance to disarm
    int16_t srh;     // base chance for search
    int16_t stl;     // Stealth of character
    int16_t fos;     // frequency of auto search
    int16_t bth;     // adj base chance to hit
    int16_t bthb;    // adj base to hit with bows
    int16_t bsav;    // Race base for saving throw
    uint8_t bhitdie; // Base hit points for race
    uint8_t infra;   // See infra-red
    uint8_t b_exp;   // Base experience factor
    uint8_t rtclass; // Bit field for class types
} race_type;

typedef struct class_type {
    const char *title;       // type of class
    uint8_t adj_hd;          // Adjust hit points
    uint8_t mdis;            // mod disarming traps
    uint8_t msrh;            // modifier to searching
    uint8_t mstl;            // modifier to stealth
    uint8_t mfos;            // modifier to freq-of-search
    uint8_t mbth;            // modifier to base to hit
    uint8_t mbthb;           // modifier to base to hit - bows
    uint8_t msav;            // Class modifier to save
    int16_t madj_str;        // Class modifier for strength
    int16_t madj_int;        // Class modifier for intelligence
    int16_t madj_wis;        // Class modifier for wisdom
    int16_t madj_dex;        // Class modifier for dexterity
    int16_t madj_con;        // Class modifier for constitution
    int16_t madj_chr;        // Class modifier for charisma
    uint8_t spell;           // class use mage spells
    uint8_t m_exp;           // Class experience factor
    uint8_t first_spell_lev; // First level where class can use spells.
} class_type;

typedef struct background_type {
    const char *info; // History information
    uint8_t roll;  // Die roll needed for history
    uint8_t chart; // Table number
    uint8_t next;  // Pointer to next table
    uint8_t bonus; // Bonus to the Social Class+50
} background_type;

typedef struct cave_type {
    uint8_t cptr;
    uint8_t tptr;
    uint8_t fval;

    unsigned int lr : 1; // Room should be lit with perm light, walls with
                         //     this set should be perm lit after tunneled out.
    unsigned int fm : 1; // Field mark, used for traps/doors/stairs, object is
                         //     hidden if fm is false.
    unsigned int pl : 1; // Permanent light, used for walls and lighted rooms.
    unsigned int tl : 1; // Temporary light, used for player's lamp light,etc.
} cave_type;

typedef struct owner_type {
    const char *owner_name;
    int16_t max_cost;
    uint8_t max_inflate;
    uint8_t min_inflate;
    uint8_t haggle_per;
    uint8_t owner_race;
    uint8_t insult_max;
} owner_type;

typedef struct inven_record {
    int32_t scost;
    inven_type sitem;
} inven_record;

typedef struct store_type {
    int32_t store_open;
    int16_t insult_cur;
    uint8_t owner;
    uint8_t store_ctr;
    uint16_t good_buy;
    uint16_t bad_buy;
    inven_record store_inven[STORE_INVEN_MAX];
} store_type;

// 64 bytes for this structure
typedef struct high_scores {
    int32_t points;
    int32_t birth_date;
    int16_t uid;
    int16_t mhp;
    int16_t chp;
    uint8_t dun_level;
    uint8_t lev;
    uint8_t max_dlv;
    uint8_t sex;
    uint8_t race;
    uint8_t class;
    char name[PLAYER_NAME_SIZE];
    char died_from[25];
} high_scores;
