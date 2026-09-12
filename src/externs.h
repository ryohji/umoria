// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Declarations for global variables and initialized data

// 実体は variable.c:17 で 17 要素。長らく 5 と書かれていたが、変数を
// 定義している variable.c がこのヘッダを include していなかったので
// 誰も気づけなかった。include を入れて食いちがいを見つけた。
extern const char *copyright[17];

// horrible hack: needed because compact_monster() can be called from
// deep within creatures() via place_monster() and summon_monster().
extern int hack_monptr;

extern vtype died_from;
extern vtype savefile; // The save file. -CJS-
extern int32_t birth_date;

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
extern bool new_level_flag; // Next level when true
extern bool teleport_flag;  // Handle teleport traps
extern int eof_flag;        // Used to handle eof/HANGUP
extern bool player_light;   // Player carrying light
extern int find_flag;       // Used in MORIA
extern bool free_turn_flag; // Used in MORIA
extern bool weapon_heavy;   // Flag if the weapon too heavy -CJS-
extern int pack_heavy;      // Flag if the pack too heavy -CJS-
extern char doing_inven;    // Track inventory commands
extern bool screen_change;  // Screen changes (used in inven_commands)

extern bool character_generated;    // don't save score until char gen finished
extern bool character_saved;        // prevents save on kill after save_char()
extern FILE *highscore_fp;          // High score file pointer
extern int command_count;           // Repetition of commands. -CJS-
extern bool default_dir;            // Use last direction in repeated commands
extern int16_t noscore;             // Don't score this game. -CJS-
extern uint32_t randes_seed;        // For encoding colors
extern uint32_t town_seed;          // Seed for town genera
extern int16_t dun_level;           // Cur dungeon level
extern int16_t missile_ctr;         // Counter for missiles
// The top line (was: msg_flag, old_msg[MAX_SAVE_MSG], last_msg and
// wait_for_more) is private to messages.c now, together with the code that
// walks the ring and the -more- prompt; see messages.h.
extern bool death;                  // True if died
extern int32_t turn;                // Cur trun of game
extern bool wizard;                 // Wizard flag
extern bool to_be_wizard;
extern bool panic_save; // this is true if playing from a panic save

extern char days[7][29];
extern int closing_flag; // Used for closing

extern int16_t cur_height; // Current dungeon height
extern int16_t cur_width;  // Current dungeon width

// Following are calculated from max dungeon sizes
// The panel (the ten values that say which part of the dungeon is on screen)
// is private to panel.c now, together with the arithmetic that derives the six
// coordinates from the two indexes. See panel.h.

// Following are all floor definitions
extern cave_type cave[MAX_HEIGHT][MAX_WIDTH];

// Following are player variables
extern player_type py;
extern const char *player_title[MAX_CLASS][MAX_PLAYER_LEVEL];
extern race_type race[MAX_RACES];
extern background_type background[MAX_BACKGROUND];
extern uint32_t player_exp[MAX_PLAYER_LEVEL];
extern uint16_t player_hp[MAX_PLAYER_LEVEL];
extern int16_t char_row;
extern int16_t char_col;

extern uint8_t rgold_adj[MAX_RACES][MAX_RACES];

extern class_type class[MAX_CLASS];
extern int16_t class_level_adj[MAX_CLASS][MAX_LEV_ADJ];

// Warriors don't have spells, so there is no entry for them.
extern spell_type magic_spell[MAX_CLASS - 1][31];
extern const char *spell_names[62];
extern uint32_t spell_learned;   // Bit field for spells learnt -CJS-
extern uint32_t spell_worked;    // Bit field for spells tried -CJS-
extern uint32_t spell_forgotten; // Bit field for spells forgotten -JEW-
extern uint8_t spell_order[32];  // remember order that spells are learned in
extern uint16_t player_init[MAX_CLASS][5];
extern bool total_winner;
extern int32_t max_score;

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
extern uint8_t object_ident[OBJECT_IDENT_SIZE];
extern int16_t t_level[MAX_OBJ_LEVEL + 1];
extern inven_type t_list[MAX_TALLOC];
extern inven_type inventory[INVEN_ARRAY_SIZE];
extern const char *special_names[SN_ARRAY_SIZE];
extern int16_t sorted_objects[MAX_DUNGEON_OBJ];
extern int16_t inven_ctr;    // Total different obj's
extern int16_t inven_weight; // Cur carried weight
extern int16_t equip_ctr;    // Cur equipment ctr
extern int16_t tcptr;        // Cur treasure heap ptr

// Following are creature arrays and variables
extern monster_type m_list[MAX_MALLOC];
extern int16_t m_level[MAX_MONS_LEVEL + 1];
extern monster_type blank_monster; // Blank monster values
extern int16_t mfptr;              // Cur free monster ptr
extern int16_t mon_tot_mult;       // # of repro's of creature

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

// Initialized data which had to be moved from some other file
// Since these get modified, macrsrc.c must be able to access
// them Otherwise, game cannot be made restartable dungeon.c.
extern char last_command; // Memory of previous command.

// moria1.c
// Track if temporary light about player.
extern bool light_flag;

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
int16_t object_offset(inven_type *);
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
// misc1.c
void init_seeds(uint32_t);
void set_seed(uint32_t);
void reset_seed(void);
int randint(int);
int randnor(int, int);
int bit_pos(uint32_t *);
bool in_bounds(int, int);
// panel_bounds() は panel.c の static になった（外から呼ぶ必要が無かった）。
// panel_contains() は panel.h。
int get_panel(int, int, int);
int distance(int, int, int, int);
int next_to_walls(int, int);
int next_to_corr(int, int);
int damroll(int, int);
int pdamroll(const uint8_t *);
bool los(int, int, int, int);
uint8_t loc_symbol(int, int);
bool test_light(int, int);
void prt_map(void);
bool compact_monsters(void);
void add_food(int);
int popm(void);
int max_hp(const uint8_t *);
bool place_monster(int, int, creature_handle, int);
void place_win_monster(void);
void alloc_monster(int, int, int);
bool summon_monster(int *, int *, int);
bool summon_undead(int *, int *);
int popt(void);
void pusht(uint8_t);
bool magik(int);
int m_bonus(int, int, int);

// misc2.c
void magic_treasure(int, int);
void set_options(void);

// misc3.c
void place_trap(int, int, int);
void place_rubble(int, int);
void place_gold(int, int);
int get_obj_num(int, bool);
void place_object(int, int, bool);
// 引数の関数は sets.c の set_room / set_corr / set_floor。いずれも
// cave[][].fval（床の種類）を受けとる bool f(int) 型。
void alloc_object(bool (*)(int), int, int);
void random_object(int, int, int);
void cnv_stat(uint8_t, char *);
void prt_stat(int);
void prt_field(const char *, int, int);
int stat_adj(int);
int chr_adj(void);
int con_adj(void);
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
uint8_t modify_stat(int, int16_t);
void set_use_stat(int);
bool inc_stat(int);
bool dec_stat(int);
bool res_stat(int);
void bst_stat(int, int);
int tohit_adj(void);
int toac_adj(void);
int todis_adj(void);
int todam_adj(void);
void prt_stat_block(void);
void draw_cave(void);
void put_character(void);
void put_stats(void);
const char *likert(int, int);
void put_misc1(void);
void put_misc2(void);
void put_misc3(void);
void display_char(void);
void get_name(void);
void change_name(void);
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
int spell_chance(int);
void print_spells(int *, int, int, int);
int get_spell(int *, int, int *, int *, const char *, int);
void calc_spells(int);
void gain_spells(void);
void calc_mana(int);
void prt_experience(void);
void calc_hitpoints(void);
void insert_str(char *, const char *, const char *);
void insert_lnum(char *, const char *, int32_t, int);
bool enter_wiz_mode(void);
int attack_blows(int, int *);
int tot_dam(inven_type *, int, creature_handle);
int critical_blow(int, int, int, int);
int mmove(int, int *, int *);
bool player_saves(void);
int find_range(int, int, int *, int *);
void teleport(int);

// misc4.c
void scribe_object(void);
void add_inscribe(inven_type *, uint8_t);
void inscribe(inven_type *, const char *);
void check_view(void);
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

// moria2.c
void change_trap(int, int);
void search(int, int, int);
void find_init(int);
void find_run(void);
void end_find(void);
void area_affect(int, int, int);
int minus_ac(uint32_t);
void corrode_gas(const char *);
void poison_gas(int, const char *);
void fire_dam(int, const char *);
void cold_dam(int, char *);
void light_dam(int, char *);
void acid_dam(int, const char *);

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

// store1.c
int32_t item_value(inven_type *);
int32_t sell_price(int, int32_t *, int32_t *, inven_type *);
bool store_check_num(inven_type *, int);
void store_carry(int, int *, inven_type *);
void store_destroy(int, int, int);
void store_init(void);
void store_maint(void);
bool noneedtobargain(int, int32_t);
void updatebargain(int, int32_t, int32_t);

// store2.c
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

// wizard.c
void wizard_light(void);
void change_character(void);
void wizard_create(void);
