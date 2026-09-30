// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Declarations for global variables and initialized data

// 実体は variable.c:17 で 17 要素。長らく 5 と書かれていたが、変数を
// 定義している variable.c がこのヘッダを include していなかったので
// 誰も気づけなかった。include を入れて食いちがいを見つけた。
extern const char *copyright[17];

// These are options, set with set_options command -CJS-
extern bool rogue_like_commands;
extern bool find_cut;          // Cut corners on a run
extern bool find_examine;      // Check corners on a run
extern bool find_prself;       // Print yourself on a run (slower)
extern bool find_bound;        // Stop run when the map shifts
extern bool prompt_carry_flag; // Prompt to pick something up
extern bool show_weight_flag;  // Display weights in inventory
extern bool highlight_seams;   // Highlight magma and quartz
extern bool find_ignore_doors; // Run through open doors
extern bool sound_beep_flag;   // Beep for invalid character
extern bool display_counts;    // Display rest/repeat counts

// global flags
// eof_flag moved to input_ended.c: whether the input has run out, and how many
// EOFs it took (#18-11-5C). The windows are in input_ended.h
// find_flag moved to running.c: whether the player is running, and how many
// steps in (#18-11-6C). The windows are in running.h
extern bool free_turn_flag; // Used in MORIA
extern FILE *highscore_fp;          // High score file pointer (init_scorefile only)
// command_count and default_dir moved to command_state.c, together with
// last_command: how many repeats are left and whether the direction is taken
// from memory (#18-11-7C). The windows are in command_state.h
// Which level the game is on now is not declared here. The number is private to
// dungeon_level.c and is reached through src/dungeon/dungeon_level.h, which also answers
// "am I in the town?" for the six callers that used to spell that question three
// ways (!= 0, > 0, == 0). The one alias -- the save file's restore path reading
// the short straight through a faked pointer -- is gone with it.
// The top line (was: msg_flag, old_msg[MAX_SAVE_MSG], last_msg and
// wait_for_more) is private to messages.c now, together with the code that
// walks the ring and the -more- prompt; see messages.h.

// How far the game has got (turn, randes_seed, town_seed, wizard,
// to_be_wizard), how this life ended (death, died_from, birth_date, noscore)
// and where it is saved (savefile, character_generated, character_saved,
// panic_save) are not declared here. Each group is private to its own file and
// is reached through src/data/progress.h, src/save/score_death.h and src/save/save_state.h.

extern char days[7][29];
extern int closing_flag; // Used for closing

// How tall and how wide this level is are not declared here. The pair is
// private to dungeon_size.c and is reached through src/dungeon/dungeon_size.h. Two
// names, one act: one setter takes both halves, so no caller can change half
// of a size, and the two aliases that read the pair straight out of the save
// file (rd_short through a faked pointer) are gone with it.

// Following are calculated from max dungeon sizes
// The panel (the ten values that say which part of the dungeon is on screen)
// is private to panel.c now, together with the arithmetic that derives the six
// coordinates from the two indexes. See panel.h.

// The floor of the level is not declared here. Every square -- what it is made
// of, which monster stands on it, which thing lies on it, and the four light
// bits -- is private to dungeon_map.c, handed out one square at a time by
// square_at(y, x). See src/dungeon/dungeon_map.h. The table always covers MAX_HEIGHT x
// MAX_WIDTH; how much of it the level in play uses is dungeon_size.c's
// question. With it went the last of the eleven dungeon globals.

// Following are player variables
extern player_type py;
extern const char *player_title[MAX_CLASS][MAX_PLAYER_LEVEL];
extern race_type race[MAX_RACES];
extern background_type background[MAX_BACKGROUND];
extern uint32_t player_exp[MAX_PLAYER_LEVEL];

extern uint8_t rgold_adj[MAX_RACES][MAX_RACES];

extern class_type class[MAX_CLASS];
extern int16_t class_level_adj[MAX_CLASS][MAX_LEV_ADJ];

// Warriors don't have spells, so there is no entry for them.
extern spell_type magic_spell[MAX_CLASS - 1][31];
extern const char *spell_names[62];
extern uint16_t player_init[MAX_CLASS][5];

// Following are store definitions
extern owner_type owners[MAX_OWNERS];
// 6 軒の記録の実体は stores.c の static。窓口は stores.h の
// store_at() / store_count()。
extern uint16_t store_choice[MAX_STORES][STORE_CHOICES];
// 実体は tables.c:90。店ごとの買いとり判定で、引数は品物の tval。
// 戻り値は長らく int と書かれていたが、実体は bool を返す。
extern bool (*store_buy[MAX_STORES])(int);

// Following are treasure arrays  and variables
extern treasure_type object_list[MAX_OBJECTS];
// t_list is not declared here. What is lying on the floor of this level is
// private to floor_items.c, reached through src/dungeon/floor_items.h. It and the mark
// below were always one container -- a table whose rows are packed, and how far
// it is filled -- and it is not a list of treasure: doors, staircases, rubble,
// traps and shop entrances are rows of it too. Everything on a square that is
// not a monster is in there, which is why it reached seventeen files.
extern const char *special_names[SN_ARRAY_SIZE];
// tcptr is not declared here. How far the floor table is filled went with the
// table itself in #18-14-7C (see above). Its one alias, save.c's
// `rd_short((uint16_t *)&tcptr)`, went through a local uint16_t and is gone.

// What the player carries and wears (inventory[], inven_ctr, inven_weight,
// equip_ctr) is not declared here. It is private to inventory.c and is
// reached through src/item/inventory.h and src/item/equipment.h.

// What the player has learned about each kind of object (object_ident[]) is not
// declared here either. It is private to item_ident.c, reached through
// src/item/item_ident.h.

// Which kinds of object the dungeon can produce, in order of depth
// (sorted_objects[], t_level[]) is private to object_levels.c, reached through
// src/item/object_levels.h. The two were always one table -- an index and a body --
// and every reader wanted a band out of them, never the raw arrays.

// Following are creature arrays and variables
// m_list is not declared here. Which monsters are standing on this level is
// private to monster_list.c, reached through src/monster/monster_list.h. It and the mark
// below were always one container -- a table whose rows are packed, and how far
// it is filled -- and every reader wanted a row, a bound or a free slot, never
// the raw array.
// m_level is not declared here. Where each level's monsters sit in the
// definition table is private to monster_levels.c, reached through
// src/monster/monster_levels.h. Every reader wanted a band -- a count, a width or the
// number it starts at -- never the raw array; the building of it used to be a
// static of main.c, out of reach of any test.
extern monster_type blank_monster; // Blank monster values
// mfptr is not declared here. How far the monster list is filled went with the
// table itself in #18-14-4C (see above). Its one alias, save.c's
// `rd_short((uint16_t *)&mfptr)`, went through a local uint16_t and is gone.
// mon_tot_mult is not declared here. How many monsters have been bred on this
// level is private to monster_breeding.c, reached through
// src/monster/monster_breeding.h. Every reader wanted the question, not the count --
// may another be bred, one has been, one is gone, a new level -- and only
// save.c wanted the number itself.

// Following are arrays for descriptive pieces
extern const char *colors[MAX_COLORS];
extern const char *mushrooms[MAX_MUSH];
extern const char *woods[MAX_WOODS];
extern const char *metals[MAX_METALS];
extern const char *rocks[MAX_ROCKS];
extern const char *amulets[MAX_AMULETS];
extern const char *syllables[MAX_SYLLABLES];

extern uint8_t blows_table[7][6];

extern uint16_t normal_table[NORMAL_TABLE_SIZE];

// The command before this one moved to command_state.c (last_command,
// #18-11-7C), beside the repeat count that is the reason it is remembered. Only
// "was the previous command this key?" is asked, so the character itself no
// longer leaves the module; the windows are in command_state.h

// function return values

// only extern functions declared here, static functions
// declared inside the file that defines them.

#define LENGTH_OF(a) (sizeof(a) / sizeof((a)[0]))

#define END_OF(a) ((a) + LENGTH_OF(a))

#define CONCAT(...) concat((vtype){0}, __VA_ARGS__, NULL)

// create.c
void create_character(void);

// creature.c
void update_mon(int);
bool multiply_monster(int, int, creature_handle, int);
void creatures(int);

// death.c
void display_scores(int);
bool duplicate_character(void);
int32_t total_points(void);
// 末尾で exit(0) するので、呼びだしの後ろへは戻らない。それを型で表明して
// おくと「この後は到達しない」ことをコンパイラが判断できる（main.c の
// switch で case を貫通しているという誤検出が消える）。
_Noreturn void exit_game(void);

// desc.c
bool is_a_vowel(char);
void magic_init(void);
void known1(inven_type *);
int known1_p(inven_type *);
void known2(inven_type *);
int known2_p(inven_type *);
void clear_known2(inven_type *);
void clear_empty(inven_type *);
void store_bought(inven_type *);
int store_bought_p(inven_type *);
void sample(inven_type *);
void identify(int *);
void unmagic_name(inven_type *);
void objdes(char *, inven_type *, int);
void invcopy(inven_type *, int);
void desc_charges(int);
void desc_remain(int);

// dungeon.c
void dungeon(void);

// eat.c
void eat(void);

// files.c
void init_scorefile(void);
void read_times(void);
void helpfile(const char *);
void print_objects(void);
bool file_character(char *);

// generate.c
void generate_cave(void);

// help.c
void ident_char(void);

// io.c
void put_buffer(const char *, int, int);
void put_qio(void);
void shell_out(void);
char inkey(void);
void flush(void);
void erase_line(int, int);
void clear_screen(void);
void clear_from(int);
void print(char, int, int);
void move_cursor_relative(int, int);
void count_msg_print(const char *);
void prt(const char *, int, int);
void move_cursor(int, int);
void msg_print(const char *);
bool get_check(const char *);
int get_com(const char *, char *);
bool get_string(char *, int, int, int);
void pause_line(int);
void pause_exit(int, int);
void save_screen(void);
void restore_screen(void);
void bell(void);
void screen_map(void);
void sleep_in_seconds(int);
bool check_input(int);
void user_name(char *);

#ifndef _WIN32
// call functions which expand tilde before calling open/fopen
#define open topen
#define fopen tfopen

int tilde(const char *, char *);
FILE *tfopen(const char *, const char *);
int topen(char *, int, int);
#endif

// magic.c
void cast(void);

// main.c
// core/dice.c
int damroll(int, int);
int pdamroll(const uint8_t *);
int max_hp(const uint8_t *);
// The groups below, down to m_bonus(), were misc1.c until #54.
// game_state.c
void init_seeds(uint32_t);
// core/bits.c, which declares it in bits.h as well
int bit_pos(uint32_t *);
// dungeon/geometry.c
bool in_bounds(int, int);
int distance(int, int, int, int);
bool los(int, int, int, int);
int next_to_walls(int, int);
int next_to_corr(int, int);
int mmove(int, int *, int *);
// item/inven_ops.c
void inven_destroy(int);
void take_one_item(inven_type *, inven_type *);
void inven_drop(int, int);
// 引数の関数は sets.c の set_corrodes / set_flammable /
// set_frost_destroy / set_lightning_destroy / set_acid_affect。
// いずれも持ち物 1 つを受けとる bool f(inven_type *) 型。
int inven_damage(bool (*)(inven_type *), int);
int weight_limit(void);
bool inven_check_num(inven_type *);
bool inven_check_weight(inven_type *);
void check_strength(void);
int inven_carry(inven_type *);
int find_range(int, int, int *, int *);
// player/player_move.c
void teleport(int);
// ui/map_view.c
// panel_bounds() は panel.c の static になった（外から呼ぶ必要が無かった）。
// panel_contains() は panel.h。
int get_panel(int, int, int);
uint8_t loc_symbol(int, int);
bool test_light(int, int);
void prt_map(void);
// monster/monster_place.c
bool compact_monsters(void);
int popm(void);
bool place_monster(int, int, creature_handle, int);
void place_win_monster(void);
void alloc_monster(int, int, int);
bool summon_monster(int *, int *, int);
bool summon_undead(int *, int *);
// player/food_ops.c
void add_food(int);
// dungeon/object_place.c
int popt(void);
void pusht(uint8_t);

// item/item_enchant.c (magic_treasure() was misc2.c until #54)
int m_bonus(int, int, int);
void magic_treasure(int, int);

// ui/options_menu.c
void set_options(void);

// combat/hit_rolls.c
int attack_blows(int, int *);
int tot_dam(inven_type *, int, creature_handle);
int critical_blow(int, int, int, int);

// combat/player_damage.c
bool player_saves(void);
int minus_ac(uint32_t);
void corrode_gas(const char *);
void poison_gas(int, const char *);
void fire_dam(int, const char *);
void cold_dam(int, char *);
void light_dam(int, char *);
void acid_dam(int, const char *);

// dungeon/object_alloc.c
void place_trap(int, int, int);
void place_rubble(int, int);
void place_gold(int, int);
int get_obj_num(int, bool);
void place_object(int, int, bool);
// The function argument is set_room, set_corr or set_floor from sets.c. Each
// is a bool f(int) that takes cave[][].fval (the kind of floor).
void alloc_object(bool (*)(int), int, int);
void random_object(int, int, int);

// ui/status_line.c
void cnv_stat(uint8_t, char *);
void prt_stat(int);
void prt_field(const char *, int, int);
const char *title_string(void);
void prt_title(void);
void prt_level(void);
void prt_cmana(void);
void prt_mhp(void);
void prt_chp(void);
void prt_pac(void);
void prt_gold(void);
void prt_depth(void);
void prt_hunger(void);
void prt_blind(void);
void prt_confused(void);
void prt_afraid(void);
void prt_poisoned(void);
void prt_state(void);
void prt_speed(void);
void prt_study(void);
void prt_winner(void);
void prt_stat_block(void);
void draw_cave(void);

// ui/char_screen.c
void put_character(void);
void put_stats(void);
const char *likert(int, int);
void put_misc1(void);
void put_misc2(void);
void put_misc3(void);
void display_char(void);
void get_name(void);
void change_name(void);

// player/stat_ops.c
uint8_t modify_stat(int, int16_t);
void set_use_stat(int);
bool inc_stat(int);
bool dec_stat(int);
bool res_stat(int);
void bst_stat(int, int);

// item/spellbook.c
int spell_chance(int);
void print_spells(int *, int, int, int);
int get_spell(int *, int, int *, int *, const char *, int);
void calc_spells(int);
void gain_spells(void);
void calc_mana(int);

// player/level_ops.c
void prt_experience(void);
void calc_hitpoints(void);

// item/inscription.c (these five were misc4.c until #54)
void scribe_object(void);
void add_inscribe(inven_type *, uint8_t);
void inscribe(inven_type *, const char *);
// ui/map_view.c
void check_view(void);
// core/str_insert.c, which declares it in str_insert.h as well; CONCAT above
// expands to it
char *concat(char *buffer, ...);

// monsters.c
bool monster_attack_is_null(attack_handle h);
uint8_t monster_attack_get_type(attack_handle h);
uint8_t monster_attack_get_desc(attack_handle h);
uint8_t monster_attack_get_dice(attack_handle h);
uint8_t monster_attack_get_sides(attack_handle h);

// TODO: eliminate `monster_make_creature_handle`
// TODO: eliminate `m_ptr->creature.place` reference
creature_handle monster_make_creature_handle(uint16_t index);
creature_handle monster_get_creature_handle(creature_type *p);
creature_type *monster_get_creature(creature_handle h);

// モンスター定義表の逆順走査。使いかたは
//   const creature_rev_iterator end = monster_creature_rend();
//   for (creature_rev_iterator it = monster_creature_rbegin();
//        !monster_creature_rsame(it, end); it = monster_creature_rnext(it)) {
//       creature_type *const creature = monster_creature_rget(it);
creature_rev_iterator monster_creature_rbegin(void);
creature_rev_iterator monster_creature_rend(void);
bool monster_creature_rsame(creature_rev_iterator a, creature_rev_iterator b);
creature_rev_iterator monster_creature_rnext(creature_rev_iterator it);
creature_type *monster_creature_rget(creature_rev_iterator it);

const char *monster_name(vtype, const monster_type *);
const char *monster_name_lower(vtype, const monster_type *);
const char *monster_name_or_something(vtype, const monster_type *);
const char *monster_name_indefinite(vtype, const creature_type *);

// moria1.c
void change_speed(int);
void py_bonuses(inven_type *, int);
void calc_bonuses(void);
int show_inven(int, int, bool, int, const char *);
const char *describe_use(int);
int show_equip(bool, int);
void takeoff(int, int);
int verify(const char *, int);
void inven_command(char);
int get_item(int *, const char *, int, int, const char *, const char *);
bool no_light(void);
bool get_dir(const char *, int *);
bool get_alldir(const char *, int *);
void move_rec(int, int, int, int);
void light_room(int, int);
void lite_spot(int, int);
void move_light(int, int, int, int);
void disturb(int, int);
void search_on(void);
void search_off(void);
void rest(void);
void rest_off(void);
bool test_hit(int, int, int, int, int);
void take_hit(int, const char *);

// dungeon/search.c
void change_trap(int, int);
void search(int, int, int);

// player/run_path.c
void find_init(int);
void find_run(void);
void end_find(void);
void area_affect(int, int, int);

// moria3.c
int cast_spell(const char *, int, int *, int *);
void delete_monster(int);
void fix1_delete_monster(int);
void fix2_delete_monster(int);
int delete_object(int, int);
uint32_t monster_death(int, int, uint32_t);
int mon_take_hit(int, int);
void py_attack(int, int);
void move_char(int, bool);
void chest_trap(int, int);
void openobject(void);
void closeobject(void);
int twall(int, int, int, int);

// moria4.c
void tunnel(int);
void disarm_trap(void);
void look(void);
void throw_object(void);
void bash(void);

// potions.c
void quaff(void);

// prayer.c
void pray(void);

// recall.c
bool bool_roff_recall(creature_type *);
int roff_recall(creature_type *);

// rnd.c
uint32_t get_rnd_seed(void);
void set_rnd_seed(uint32_t);
int32_t rnd(void);
// The five below came from misc1.c (#54).
void set_seed(uint32_t);
void reset_seed(void);
int randint(int);
int randnor(int, int);
bool magik(int);

// save.c
bool save_char(void);
bool _save_char(char *);
bool get_char(bool *);
void set_fileptr(FILE *);
void wr_highscore(high_scores *);
void rd_highscore(high_scores *);

// scrolls.c
void read_scroll(void);

// sets.c
bool set_room(int);
bool set_corr(int);
bool set_floor(int);
bool set_corrodes(inven_type *);
bool set_flammable(inven_type *);
bool set_frost_destroy(inven_type *);
bool set_acid_affect(inven_type *);
bool set_lightning_destroy(inven_type *);
bool set_null(inven_type *);
bool set_acid_destroy(inven_type *);
bool set_fire_destroy(inven_type *);
bool set_large(treasure_type *);
bool general_store(int);
bool armory(int);
bool weaponsmith(int);
bool temple(int);
bool alchemist(int);
bool magic_shop(int);

// signals.c
void nosignals(void);
void signals(void);
void init_signals(void);
void handle_pending_signals(void);

// spells.c
int sleep_monsters1(int, int);
int detect_treasure(void);
int detect_object(void);
int detect_trap(void);
int detect_sdoor(void);
int detect_invisible(void);
int light_area(int, int);
int unlight_area(int, int);
void map_area(void);
int ident_spell(void);
int aggravate_monster(int);
int trap_creation(void);
int door_creation(void);
int td_destroy(void);
int detect_monsters(void);
void light_line(int, int, int);
void starlite(int, int);
int disarm_all(int, int, int);
// 第 4 引数は inven_damage() に渡す判定関数の受けとり先。
void get_flags(int, uint32_t *, int *, bool (**)(inven_type *));
void fire_bolt(int, int, int, int, int, const char *);
void fire_ball(int, int, int, int, int, const char *);
void breath(int, int, int, int, char *, int);
int recharge(int);
int hp_monster(int, int, int, int);
int drain_life(int, int, int);
int speed_monster(int, int, int, int);
int confuse_monster(int, int, int);
int sleep_monster(int, int, int);
int wall_to_mud(int, int, int);
int td_destroy2(int, int, int);
int poly_monster(int, int, int);
int build_wall(int, int, int);
bool clone_monster(int, int, int);
void teleport_away(int, int);
void teleport_to(int, int);
int teleport_monster(int, int, int);
int mass_genocide(void);
int genocide(void);
int speed_monsters(int);
int sleep_monsters2(void);
int mass_poly(void);
int detect_evil(void);
int hp_player(int);
int cure_confusion(void);
int cure_blindness(void);
int cure_poison(void);
int remove_fear(void);
void earthquake(void);
int protect_evil(void);
void create_food(void);
int dispel_creature(int, int);
int turn_undead(void);
void warding_glyph(void);
void lose_str(void);
void lose_int(void);
void lose_wis(void);
void lose_dex(void);
void lose_con(void);
void lose_chr(void);
void lose_exp(int32_t);
int slow_poison(void);
void bless(int);
void detect_inv2(int);
void destroy_area(int, int);
bool enchant(int16_t *, int16_t);
int remove_curse(void);
int restore_level(void);

// staffs.c
void use(void);

// The groups below, down to updatebargain(), were store1.c until #55.
// store/store_stock.c
bool store_check_num(inven_type *, int);
void store_carry(int, int *, inven_type *);
void store_destroy(int, int, int);
void store_init(void);
void store_maint(void);
// store/store_price.c
int32_t item_value(inven_type *);
int32_t sell_price(int, int32_t *, int32_t *, inven_type *);
bool noneedtobargain(int, int32_t);
void updatebargain(int, int32_t, int32_t);

// store/store_ui.c
void enter_store(int);

// tables.c

// treasur.c

// variable.c
recall_type *recall_get(creature_handle h);
void recall_update_characteristics(creature_handle h, int defence);
void recall_update_move(creature_handle h, int move);
void recall_update_carry(creature_handle h, uint8_t number);
void recall_update_spell(creature_handle h, uint32_t type);
void recall_increment_spell_chance(creature_handle h);
void recall_increment_kill(creature_handle h);
void recall_increment_death(creature_handle h);

// wands.c
void aim(void);

// ui/wizard.c
bool enter_wiz_mode(void);
void wizard_light(void);
void change_character(void);
void wizard_create(void);
