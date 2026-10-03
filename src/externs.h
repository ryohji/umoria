// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Declarations for global variables and initialized data

// Defined in variable.c with 17 elements.
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
extern bool free_turn_flag; // Used in MORIA
extern FILE *highscore_fp;          // High score file pointer (init_scorefile only)

extern char days[7][29];
extern int closing_flag; // Used for closing

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
// Store records are static in stores.c (see stores.h for store_at / store_count).
extern uint16_t store_choice[MAX_STORES][STORE_CHOICES];
// Defined in tables.c. Each function tests if a store will buy an item tval.
extern bool (*store_buy[MAX_STORES])(int);

// Following are treasure arrays  and variables
extern treasure_type object_list[MAX_OBJECTS];
extern const char *special_names[SN_ARRAY_SIZE];

// Following are creature arrays and variables
extern monster_type blank_monster; // Blank monster values

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

// function return values

// only extern functions declared here, static functions
// declared inside the file that defines them.

#define LENGTH_OF(a) (sizeof(a) / sizeof((a)[0]))

#define END_OF(a) ((a) + LENGTH_OF(a))

#define CONCAT(...) concat((vtype){0}, __VA_ARGS__, NULL)

// player/create.c
void create_character(void);

// combat/monster_melee.c
void make_attack(int);

// monster/creature.c
void update_mon(int);
bool multiply_monster(int, int, creature_handle, int);
void creatures(int);

// save/death.c
void display_scores(int);
bool duplicate_character(void);
int32_t total_points(void);
// Does not return (calls exit). Declared _Noreturn so the compiler knows.
_Noreturn void exit_game(void);

// item/desc.c
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

// item/eat.c
void eat(void);

// ui/files.c
void init_scorefile(void);
void read_times(void);
void helpfile(const char *);
void print_objects(void);
bool file_character(char *);

// dungeon/generate.c
void generate_cave(void);

// ui/help.c
void ident_char(void);

// ui/io.c
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
bool check_input(int);
void user_name(char *);

#ifndef _WIN32
// call functions which expand tilde before calling open/fopen
#define open topen
#define fopen tfopen

FILE *tfopen(const char *, const char *);
int topen(char *, int, int);
#endif

// item/magic.c
void cast(void);

// core/dice.c
int damroll(int, int);
int pdamroll(const uint8_t *);
int max_hp(const uint8_t *);
// game_state.c
void init_seeds(uint32_t);
// core/bits.c, which declares it in bits.h as well
int bit_pos(uint32_t *);
// core/distance.c, which declares it in distance.h as well
int distance(int, int, int, int);
// dungeon/geometry.c
bool in_bounds(int, int);
bool los(int, int, int, int);
int next_to_walls(int, int);
int next_to_corr(int, int);
int mmove(int, int *, int *);
// item/inven_ops.c
void inven_destroy(int);
void take_one_item(inven_type *, inven_type *);
void inven_drop(int, int);
// Function argument is a predicate from sets.c (set_corrodes, set_flammable, etc.),
// each taking bool f(inven_type *).
int inven_damage(bool (*)(inven_type *), int);
int weight_limit(void);
bool inven_check_num(inven_type *);
bool inven_check_weight(inven_type *);
void check_strength(void);
int inven_carry(inven_type *);
int find_range(int, int, int *, int *);
// player/player_move.c
void teleport(int);
void move_char(int, bool);
// ui/map_view.c
// panel_bounds is static in panel.c; panel_contains is in panel.h.
int get_panel(int, int, int);
uint8_t loc_symbol(int, int);
bool test_light(int, int);
void prt_map(void);
// monster/monster_place.c
bool compact_monsters(void);
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

// item/item_enchant.c
void magic_treasure(int, int);

// ui/options_menu.c
void set_options(void);

// combat/hit_rolls.c
int attack_blows(int, int *);
int tot_dam(inven_type *, int, creature_handle);
int critical_blow(int, int, int, int);
bool test_hit(int, int, int, int, int);

// combat/player_damage.c
bool player_saves(void);
void corrode_gas(const char *);
void poison_gas(int, const char *);
void fire_dam(int, const char *);
void cold_dam(int, char *);
void light_dam(int, char *);
void acid_dam(int, const char *);
void take_hit(int, const char *);

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
void set_use_stat(int);
bool inc_stat(int);
bool dec_stat(int);
bool res_stat(int);
void bst_stat(int, int);

// item/spellbook.c
void print_spells(int *, int, int, int);
void calc_spells(int);
void gain_spells(void);
void calc_mana(int);
int cast_spell(const char *, int, int *, int *);

// player/level_ops.c
void prt_experience(void);
void calc_hitpoints(void);

// item/inscription.c
void scribe_object(void);
void add_inscribe(inven_type *, uint8_t);
void inscribe(inven_type *, const char *);
// ui/map_view.c
void check_view(void);
// core/str_insert.c, which declares it in str_insert.h as well; CONCAT above
// expands to it
char *concat(char *buffer, ...);

// data/monsters.c
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

// Reverse iteration over the creature definition table. Usage:
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

// player/player_bonuses.c
void change_speed(int);
void py_bonuses(inven_type *, int);
void calc_bonuses(void);

// player/rest_command.c
void disturb(int, int);
void search_on(void);
void search_off(void);
void rest(void);
void rest_off(void);

// ui/direction.c
bool get_dir(const char *, int *);
bool get_alldir(const char *, int *);

// dungeon/lighting.c
bool no_light(void);
void move_rec(int, int, int, int);
void light_room(int, int);
void lite_spot(int, int);
void move_light(int, int, int, int);

// ui/inven_menu.c
int show_inven(int, int, bool, int, const char *);
const char *describe_use(int);
int show_equip(bool, int);
void takeoff(int, int);
void inven_command(char);
int get_item(int *, const char *, int, int, const char *, const char *);

// dungeon/search.c
void change_trap(int, int);
void search(int, int, int);

// player/run_path.c
void find_init(int);
void find_run(void);
void end_find(void);
void area_affect(int, int, int);

// monster/monster_death.c
void delete_monster(int);
void fix1_delete_monster(int);
void fix2_delete_monster(int);
int delete_object(int, int);
uint32_t monster_death(int, int, uint32_t);

// ui/look.c
void look(void);

// combat/player_melee.c
void py_attack(int, int);
void py_bash(int, int);

// combat/throw.c
void throw_object(void);

// dungeon/traps.c
void disarm_trap(void);
void hit_trap(int, int);
void chest_trap(int, int);

// dungeon/terrain_commands.c
void openobject(void);
void closeobject(void);
int twall(int, int, int, int);
void tunnel(int);
void bash(void);

// combat/monster_damage.c
int mon_take_hit(int, int);

// item/potions.c
void quaff(void);

// item/prayer.c
void pray(void);

// ui/recall.c
bool bool_roff_recall(creature_type *);
int roff_recall(creature_type *);

// core/rnd.c
void set_rnd_seed(uint32_t);
int32_t rnd(void);
void set_seed(uint32_t);
void reset_seed(void);
int randint(int);
int randnor(int, int);
bool magik(int);

// save/save.c
bool save_char(void);
bool _save_char(char *);
bool get_char(bool *);
void set_fileptr(FILE *);
void wr_highscore(high_scores *);
void rd_highscore(high_scores *);

// item/scrolls.c
void read_scroll(void);

// data/sets.c
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

// platform/signals.c
void nosignals(void);
void signals(void);
void init_signals(void);
void handle_pending_signals(void);

// combat/projectiles.c
void fire_bolt(int, int, int, int, int, const char *);
void fire_ball(int, int, int, int, int, const char *);
void breath(int, int, int, int, char *, int);

// item/spells.c
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

// item/staffs.c
void use(void);

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

// item/wands.c
void aim(void);

// ui/wizard.c
bool enter_wiz_mode(void);
void wizard_light(void);
void change_character(void);
void wizard_create(void);
