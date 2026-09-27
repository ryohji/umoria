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
    // `struct misc` STOOD HERE UNTIL #18-12-28C, and it is gone. TWENTY-TWO FIELDS
    // left it as THIRTEEN QUESTIONS, one unit at a time, and the last one took the
    // struct with it -- there was nothing left to be shorter. This is the record of
    // where each went. (`struct flags` went the same way in #18-12-15B; its record
    // is below the stats.)
    //
    // HOW DEEP THIS CHARACTER HAS BEEN left in #18-12-16: `max_dlv`, eight places,
    // now behind player_max_depth.h. THE CHEAPEST QUESTION ON THIS ROAD, and that is
    // why it went first. The comparison that decides whether a new deepest floor has
    // been reached went into the window with it, because every caller wrote it.
    //
    // THE HIT DIE'S NUMBER OF FACES left in #18-12-17: `hitdie`, nine places, now
    // behind player_hit_die.h. The race's base and the class's adjustment still come
    // from the `race` and `class` tables and the hit point table is still rolled in
    // create.c -- only the answer moved.
    //
    // THE ARMOUR CLASS left in #18-12-18: `pac` and `ptoac`, seventy-three places in
    // forty-seven lines, now behind player_armour_class.h. TWO FIELDS AND ONE
    // QUESTION -- all twenty-six readers wanted the sum and not one wanted a half,
    // which is why create.c could swap which half the race's bonus and the class's
    // bonus went into and nobody could tell (ledger observation 40).
    //
    // THE BASE TO-HIT left in #18-12-19: `bth` and `bthb`, thirty-five places, now
    // behind player_base_to_hit.h. THE MIRROR OF THE ARMOUR CLASS -- two fields
    // again, but every reader wants one half alone and every writer writes the pair.
    // They share a module because they share how they change: heroism is worth
    // twelve to each and a blessing five to each. Beware of the four other things
    // with these names -- race_type.bth, struct player_abilities.bth,
    // class_type.mbth and test_hit()'s first argument are all different questions.
    //
    // THE DISARMING SKILL left in #18-12-20: `disarm`, ten places, now behind
    // player_disarm.h. ONE FIELD, ONE ANSWER -- the plainest shape on this road. A
    // trap and a lock were never separate skills here. One copy of the creation-time
    // dexterity bonus is frozen inside the number while every reader adds two copies
    // of the current one; that oddity is the game's, and it stayed.
    //
    // THE SAVING THROW left in #18-12-21: `save`, eleven places, now behind
    // player_saving_throw.h. ONE NUMBER, TWO RATINGS: the sheet's "Saving Throw"
    // reads it with the wisdom and the CLA_SAVE column, its "Magic Device" with the
    // intelligence and the CLA_DEVICE column, which is also the base of how well a
    // staff or a wand is handled. Whether a particular attempt is resisted stayed
    // player_saves()' question, because the roll needs randint().
    //
    // WHICH OF THE EIGHT RACES left in #18-12-22: `prace`, thirteen places, now
    // behind player_race.h. THE FIRST QUESTION OUT OF THIS STRUCT THAT WAS NOT A
    // QUANTITY -- a row number into the `race` table, with no arithmetic on it
    // anywhere, so there is no `_adjust` window: a character does not become more of
    // a Dwarf. What a race GIVES, what the shops charge and where the life story
    // starts all stayed with their callers.
    //
    // THE BODY'S WEIGHT left in #18-12-23: `wt`, twelve places, now behind
    // player_body_weight.h. NOT AN APPEARANCE BUT A PHYSICAL QUANTITY -- it is
    // thrown against a shield, thrown against a door, and counted into how much the
    // character can carry.
    //
    // THE ATTACK BONUSES left in #18-12-24: `ptohit` and `ptodam`, twenty-one
    // places, now behind player_attack_bonuses.h. The module is named for the attack
    // and not the weapon because the aim's bonus reaches bows and thrown things too.
    // TWO FIELDS AND TWO ANSWERS, the same shape as the base to-hit: no reader ever
    // added the aim to the force, because a reader always knows which it wants.
    //
    // THE SEARCHING SKILL AND ITS FREQUENCY left in #18-12-25: `srh` and `fos`,
    // nineteen places, now behind player_search_skill.h. TWO FIELDS AND TWO
    // QUESTIONS that share every writer -- how good a search is, and how often one
    // happens without being asked for. Turning the frequency upside down for the
    // sheet and holding it at zero are the sheet's own doing.
    //
    // WHO THIS CHARACTER IS left in #18-12-26: `name`, `male`, `age`, `ht`, `sc` and
    // `history`, forty-seven places, now behind player_bio.h. SIX FIELDS AT ONCE,
    // more than any unit on this road. Every one is read apart from the others, so
    // the usual rule for splitting would have made six modules; what put them
    // together is that NOBODY EVER MOVES ANY OF THEM (not one `_adjust` window in
    // the header) and that THE SAME FOUR PLACES ASK FOR THEM -- the character sheet,
    // the character dump, and the save file's writer and reader.
    //
    // The life story is
    // `char the_history[PLAYER_HISTORY_LINES][PLAYER_HISTORY_LINE_SIZE]` there, one
    // byte wider per line than the `char history[4][60]` that stood here: a line is
    // sixty characters AND a terminator. Nothing reachable ever wrote past the
    // narrow array even so -- all 330,984 life stories the background table can make
    // were enumerated, and a sixty-character line is always the last line, so the
    // stray terminator always landed on the next line's leading '\0' (bug candidate
    // B21, which is why it is not a bug). The save file did not change.
    //
    // HOW QUIETLY THIS CHARACTER MOVES left in #18-12-27: `stl`, nine places, now
    // behind player_stealth.h -- THE SMALLEST UNIT ON THIS ROAD. THE UNIT IS A
    // NUMBER OF HALVINGS, not a chance: `notice-cubed <= (1L << (29 - stl))` in
    // creature.c is the only place in the game that uses it, and every point halves
    // the chance that a sleeper stirs, which is why twenty steps (-1 through 18) are
    // enough where the disarming skill wanted two hundred.
    //
    // WHICH OF THE SIX CLASSES left in #18-12-28 AND TOOK THIS STRUCT WITH IT:
    // `pclass`, FIFTY-FIVE PLACES IN SIXTEEN FILES -- more than any unit before it
    // -- now behind player_class.h. NOT A QUANTITY EITHER, like the race and unlike
    // everything else here: all fifty-five either indexed a table with the byte or
    // handed it on unchanged, so there is no `_adjust` window and there never will
    // be. A character does not slowly become more of a Rogue. The reason for the
    // fifty-five is not that the answer is hard -- it is one byte written twice in a
    // character's life -- but that FOUR CONSTANT TABLES ARE INDEXED BY IT, and all
    // four stayed where they were: class[] itself, class_level_adj[][],
    // magic_spell[][] and player_title[][], plus player_init[][] at the start of a
    // game. FOURTEEN ALIASES DIED WITH IT, twelve `struct misc *` and two
    // `class_type *`, and after that not one line outside player_class.c named this
    // struct.

    // Stats now kept in arrays, for more efficient access. -CJS-
    struct stats {
        uint8_t max_stat[6]; // What is restored
        uint8_t cur_stat[6]; // What is natural
        int16_t mod_stat[6]; // What is modified, may be +/-
        uint8_t use_stat[6]; // What is used
    } stats;

    // `struct flags` STOOD HERE UNTIL #18-12-15B, and it is gone. Twenty-six
    // fields left it, one question at a time, and the three that were left over
    // answered no question at all. This is the record of where each went.
    //
    // The status word left in #18-12-7C: thirty bits, asked and answered from a
    // hundred and five places, now behind player_status_flags.h. The eighteen
    // counters were a different question -- most of the marks that lived in the
    // word pair with one of them.
    //
    // Seventeen one-byte fields left in #18-12-8C: what the character can do and
    // resist because of what is being worn -- sees invisible, never paralyzed,
    // the four resistances and falling, slow digestion, regeneration, random
    // teleportation, aggravation and the six sustained stats. They were asked and
    // answered from a hundred and six places and are now behind
    // player_abilities.h, which WORKS THEM OUT FROM THE EQUIPMENT rather than
    // remembering them.
    //
    // The eighteen counters left in #18-12-9C: how much longer each temporary
    // state lasts, asked and answered from two hundred and eighty-one places,
    // now behind player_timed_effects.h. Twelve of them pair with a mark in the
    // status word, and that pairing is the reason they moved together.
    //
    // `rest` left in #18-12-10C: whether the character is resting and for how
    // many more turns, asked and answered from twenty-one places, now behind
    // player_resting.h. It went on its own because it is WHAT THE CHARACTER IS
    // DOING rather than a state wearing off -- and because it moves towards zero
    // FROM BOTH SIDES (a negative count is "until healed" and counts up).
    //
    // `speed` left in #18-12-11C: how many steps from normal speed the character
    // is moving, asked and answered from nine places, now behind player_speed.h.
    // It is NOT a clock -- nothing ticks it down; potions, items and traps add to
    // and subtract from it, and POSITIVE MEANS SLOW.
    //
    // `see_infra` left in #18-12-12C: HOW FAR AWAY the character can make out a
    // warm-blooded creature, IN SQUARES, asked and answered from nine places, now
    // behind player_infra_range.h. It is not a clock either -- the race, the
    // equipment and the potion put a number there and nothing ticks it down --
    // and it was the FIRST of these questions whose starting value is not zero.
    //
    // `confuse_monster` left in #18-12-13C: whether the character's hands are
    // glowing, ready to confuse whatever they next touch, asked and answered from
    // eight places, now behind player_glowing_hands.h. It is a CHARGE, not a
    // clock -- scroll 11 lights the hands and they stay lit however many turns
    // pass, until one blow actually connects. Its name was a lie about OWNERSHIP
    // (it named what happens to the monster, and collided with the unrelated
    // spell confuse_monster() in spells.c), so the module is named after the
    // hands the messages name.
    //
    // `new_spells` left in #18-12-14C: how many more spells (or prayers) the
    // character may still learn, asked and answered from nine places, now behind
    // player_spells_to_learn.h. Its name was a lie about CONTENTS -- it is a
    // COUNT, not a list of spells, and which spells are known lives in
    // spells_known.h -- so the module is named after the question the tests were
    // already asking (spells_to_learn).
    //
    // THE LAST THREE WERE NOT QUESTIONS, and #18-12-15 swept them out rather than
    // giving them windows. `food` and `food_digested` had no reader and no writer
    // left at all: the stomach moved to player_food.h in #18-12-2C and only these
    // two declarations stayed behind, so #18-12-15B simply struck them out.
    // `protection` ("Protection fr. evil") was written and read by save.c and by
    // nowhere else -- the game never looked at the number, and protection from
    // evil is itself a counter in player_timed_effects.h -- so #18-12-15A made it
    // a `static int16_t` in save.c. It is a HOLE IN THE FILE FORMAT, not a
    // question: two bytes kept at a fixed position so that a file written
    // elsewhere survives being read and written back. A hole belongs inside the
    // one file that reads it, and no module was made for it.
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
