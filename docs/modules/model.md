# model/ modules design notes

Design notes for the model/ modules: the detail that does not belong in the headers.

## model/game — game settings and status

### game_settings (options + variable)

The player's boolean options. Each option has a prompt (shown on the options screen), a pointer to its global variable storage, a bit position for save files, and an initial value. The table is terminated by an entry whose prompt is NULL.

The listing order is not the bit order: find_ignore_doors is shown fifth but was assigned the ninth bit. Carrying the bit per entry is what lets both orders stay what they were.

The actual global variables (highscore_fp, free_turn_flag, and the 11 option bools) live in variable.c. These started in the main externs.h and were collected here when the options module was extracted. The starting values live in options.c beside the prompt and save file bit of each option; game_options_reset() puts them into the globals at startup.

### game_status (progress + save_state + score_death)

#### progress: How far the game has got

The turn counter, the two seeds that let a run be reproduced, and whether this run is a wizard run.

This is storage and only storage. The window does not range-check or clamp, and it does not decide anything on the caller's behalf, because the old code did neither and doing it here would change behaviour rather than preserve it.

**Turn counter:** The current turn starts at -1, which is not a turn at all: a negative turn means no character is in play. The main loop raises it to 0 on its first pass, so 0 is a real turn and `> 0` and `>= 0` are not the same test. The callers that ask about the sign do their own comparing for that reason.

**Seeds:** The color seed is what object colours and town layout are generated from. Kept so that a saved game rebuilds the same world it was saved from.

**Wizard mode:** progress_wizard_mode() is true once the player has actually entered wizard mode. progress_wizard_requested() is true when wizard mode was asked for on the command line but has not been granted yet. Startup turns this into progress_set_wizard_mode().

#### save_state: Where the game is saved and how far the saving has got

The save file path, whether a character exists yet, whether it has been written out, and whether what was written out was a panic save.

This is storage plus the three questions the callers keep asking of it. It does not range-check or clamp, and it does no file I/O: the file name lives here, opening it does not (which is what lets this file go without externs.h, where fopen is replaced by tfopen).

**Save file path:** The pointer addresses the record itself and stays valid for the rest of the run: app/main.c writes through it with strcpy() from the command line, the environment or MORIA_SAV, and save.c rewrites it when the player names a different file.

**Saving progress:** character_is_generated() is true once character generation has finished. Until then there is nothing worth saving and nothing worth scoring. character_is_saved() is true once the character has been written out. Cleared again when a run is resumed, so it is "is the file up to date", not a one-way latch. is_panic_save() is true when the file on disk came from a panic save (written from a fatal signal handler rather than from the player quitting). Such a run is not scored.

**Questions the callers ask:** save_state_has_unsaved_character() answers "is there a character that exists but has not been written out?" Asked at four places in three different spellings before this window existed. save_state_has_live_character() is the same question with "and still alive" on top — what signals.c asks before trying a panic save. This is the one place where the three groups of this batch of globals meet, so it is the one place with a dependency: it reads the death flag from score_death. save_state_character_is_in_play() asks "is a character in play at all?" This is what a turn counter of 0 or more means; a negative counter is not a turn but "nobody is playing". Reads the counter from progress. Covers the `turn >= 0` tests; it does NOT cover the one place that asks `turn > 0` and so excludes turn 0; that caller keeps its own comparison, because folding it in here would change what happens on the first turn.

#### score_death: How this life ended and whether it counts

The death flag, what did it, when the character was born, the reasons the score is not to be recorded, whether the game was won, and the best score the run has reached.

This is storage and only storage. The window does not range-check or clamp, and it does not decide anything on the caller's behalf, because the old code did neither and doing it here would change behaviour rather than preserve it.

**Death flag:** player_is_dead() is true once the player has died. Read from twelve files: the monster and the main loop both stop what they are doing when it goes true, so it is a "leave off" signal as much as a record of the ending.

**What did it:** death_cause() returns the cause of death, as it goes on the tombstone and into the score file. Also set for endings that are not deaths ("(saved)", "Interrupting", "(panic save %d)"), so it says how the run ended rather than what killed the character. The pointer addresses the record itself and stays valid for the rest of the run: the callers write through it with strcpy() and sprintf(), and the save file reads straight into it.

**Birth date:** character_birth_date() returns the wall clock at character creation. The score file keeps it to tell two runs of the same player apart.

**Score disqualifications:** The reasons this run is not to be scored, as a set of bits rather than a flag: 0x1 resurrected, 0x2 entered wizard mode, 0x4 already on the scoreboard. Callers test single bits (prt_winner) and also the whole word against zero ("is there any reason at all"), so the window hands the word over rather than answering either question.

**Victory:** player_has_won() is true once the player has beaten the game (killing a CM_WIN monster). It outlives the win: dying afterwards clears it, the tomb and the character sheet print a different title while it is true, the win monster is not placed again, and the save file refuses a resurrection.

**Best score:** best_score_so_far() returns the highest total the run has ever been worth. The score is recomputed from the character and the dungeon level every time it is asked for, so it would otherwise fall when the player loses items or climbs; this record is what keeps it from decreasing. Written only by the save file's reader, which is why there is no "raise it to" window: the running game just reads it and hands back the larger of the two numbers.

## model/dungeon — dungeon state

### dungeon_state (dungeon_level + dungeon_map + dungeon_size + floor_items)

The dungeon state comprises four pieces that are always updated together: which level the game is on, the map of that level, how big the map is, and what items lie on the floor.

**Dungeon level (dungeon_level):** One number, and almost everything that scales with difficulty comes out of it. Level 0 is the town; 1 and deeper are the dungeon. Thirteen of thirty reads use it for difficulty: which monsters may spawn, how good objects are, gold pile size, whether rooms are lit, whether a Balrog appears. The deepest the game allows is 99 (wizard prompt) or 1 (deep-descent scroll limit). The number starts at 0 (town) and is never checked against a ceiling here — limits belong to the commands that change levels.

The question "are we in town?" was written three ways: `!= 0` (store turnover), `> 0` (word of recall direction, lighting, destruction spell), and `== 0` (which size for next level). They're all one question since nothing ever puts a negative depth in, so there's one window: player_is_in_town().

**Dungeon map (dungeon_map):** Every square of the level the player walks on. It holds the town too (name was narrower than the thing). The table is always MAX_HEIGHT × MAX_WIDTH (66 × 198); town is generated into top-left 22 × 66, generate_cave() blanks the whole table before building either kind of level. What varies is how much is in play (dungeon_size's question).

Each square holds seven fields (usage counts over src/*.c): fval (214 uses, what the square is made of), tptr (161 uses, row in floor_items table lying here, 0 for nothing), cptr (118 uses, row in monster_list standing here, 0 for nothing/1 for player), pl (91 uses, player has seen this square), fm (66 uses, always in field of view even in dark), lr (44 uses, room is lit), tl (38 uses, torch-lit square).

Two fields are indices into other tables that had to be defined first: cptr indexes monster_list, tptr indexes floor_items. That's why dungeon_map came last among the three tables.

**Dungeon size (dungeon_size):** How tall and wide this level is. One pair of numbers, and it only ever takes two values: generate_cave() picks 22 × 66 for town and 66 × 198 for dungeon, so the pair is really a cache of "am I in town?" — the same thing depth says. Nothing else ever writes it, and nothing scales with it: a level is one of those two shapes for its whole life. Both writers set both halves one line after the other (generate.c twice, save.c's restore once), and readers almost always use both.

**Floor items (floor_items):** The things lying on the floor of this level, and how far the table is filled. Same arrangement as monsters: one table whose rows are PACKED, and one mark saying how far packing reaches. Every square carries tptr indexing into this table (cave[y][x].tptr says which thing lies there).

It's not just treasure: doors, staircases, rubble, traps, shop entrances (six shops), and spell-grown mushrooms are all rows of this table, placed by the same invcopy() call as gold piles and swords.

An empty row is a copy of object_list[OBJ_NOTHING] (TV_NOTHING, character=space), not a named constant like blank_monster. Blanking a row means calling invcopy(), which lives in model/item/object_copy.c and reads the definition table. The dependency points the same way the module does (toward read-only data). Row 0 is always blank: it's where unchecked t_list[c_ptr->tptr] lands, and it must answer TV_NOTHING.

### geometry

Geometry on the map: whether a point is inside it, how far apart two points are, whether one can be seen from the other, and what surrounds a point. Kept separate because it's pure utility functions with no state.

## model/item — item inventory, knowledge and kinds

### inventory — what the player carries and wears

#### Storage structure

One array holds both the pack and the equipment. Slots 0..21 are the pack and slots 22..33 are what the player wears or wields. inventory.h provides windows on both parts: inventory_at() for the pack, equipment_at() for the equipment.

The windows use the same index space: an index means the same thing whichever window it is handed to. This design avoids translation (subtracting 22) which would change the arithmetic at call sites.

#### Why no range checking

The windows are thin: they do not range-check, clamp or assert. The old code did not check either, and checking here would change behaviour rather than preserve it.

#### Pointer lifetime

Pointers returned by inventory_at() and equipment_at() stay valid for the rest of the run and always address the same slot. Callers keep them in local variables (inven_type *i_ptr) across other calls, and the save file reads straight into them.

#### Pack

The pack is kept packed: used slots are 0..inventory_count()-1 with no gaps. inventory_slot_count() returns 22, which is both the number of slots and the index of the first equipment slot (INVEN_WIELD).

The pack weight is tracked separately. Callers keep it up to date themselves as they add and remove objects; nothing in this module recomputes it.

#### Equipment

Equipment slots run from 22 to 33, named by the INVEN_* constants in constant.h. Unlike the pack, filled slots are not packed together — equipment_count() is a total count, not a bound to loop up to.

The number of equipment slots (12) is fixed in constant.h as part of INVEN_ARRAY_SIZE. This cannot be changed without breaking existing save files, which serialize the equipment as 12 slots in sequence.

#### Crossing window

inventory_and_equipment_at() provides a window for places that walk both the pack and the equipment by index. This is what get_item() (inven_menu.c) uses — it treats 0 to INVEN_ARRAY_SIZE-1 as one contiguous index space.

#### Save file format

The save file serializes inventory[] as one run: pack first, then equipment. This is why the array cannot be split — doing so would change the save file order.

### item_knowledge — what the player has learned about items

#### Display hints

The ident field holds display hints that control what objdes() shows: ID_SHOW_HITDAM (show hit/dam bonuses), ID_SHOW_P1 (show p1), ID_NOSHOW_P1 (hide p1). These are set by identification and usage.

#### Item notes

Three more ident bits record what the player has noticed: ID_MAGIK (sensed as magical), ID_EMPTY (tried and found empty), ID_DAMD (known to be cursed). item_has_any_note() checks if any of these three are set.

#### Kind knowledge records

Seven groups of secret object kinds each have 64 records tracking whether the kind is known by name (OD_KNOWN1) and whether it has been tried (OD_TRIED). The groups are: amulets, rings, staffs, wands, scrolls, potions, and mushrooms (the secret subset of TV_FOOD).

Weapons, armor, and non-mushroom foods are never secret, so they have no records. Asking about such a kind answers "not known, not tried", and marking one does nothing.

#### Record lookup

The record for a kind is found by combining tval (which group) and the low 6 bits of subval (which kind in that group, 0..63). The bit above those 6 (ITEM_SINGLE_STACK_MIN) means "stacks as a single item" and is masked off — it is not part of the kind number.

Kinds are shared: drinking one unknown potion teaches the player about every potion of that sort, in the pack, on the floor, or still in the dungeon.

#### Marking known clears tried

item_kind_mark_known() sets the known bit and also clears the tried bit, since knowing the kind by name supersedes having tried it. The two always moved together at the one place that did this, spelled out as two lines against the same record.

#### Save file storage

The save file stores object_ident as a fixed run of OBJECT_IDENT_SIZE bytes (7 groups × 64 records). Each byte starts as 0 ("nothing known, nothing tried"), which is correct for a new character.

### item_kinds — object generation tables

#### Level index

Every kind of object that can be generated is laid out in order of the depth it first appears at. This lets random generation bound its choice by dungeon level.

Two arrays make the one table: sorted_objects[] holds object_list indexes in level order, and t_level[] is the cumulative index. t_level[L] counts kinds at level L or shallower, so level L's band runs from t_level[L-1] to t_level[L]-1.

Unlike monsters (whose definition table is already sorted by level and needs no second array), object_list is not sorted by level, so sorted_objects[] is necessary.

#### Building the table

object_levels_init() scans object_list once at startup to build the index. It uses a counting sort (O(n)) rather than a comparison sort. The table is built once and never changes, so nothing saves it — loading a game rebuilds it.

#### invcopy

invcopy() copies fields from object_list[from_index] into an inven_type. Used to initialize inventory slots, floor items, and store items from the definition table. It sets index to from_index, clears name2 and inscrip, copies all other fields from the template, and sets ident to 0.

### device — magic device use calculation

#### Purpose

Extracted from the duplicated chance calculation in staffs.c and wands.c. The only difference between the two was the staff's constant penalty (5 vs 0 for wands), which is now the penalty argument.

#### Formula

The chance is: save + int_adj - item_level - penalty + (class_device_adj * player_level / 3)

If confused, the chance is halved. If the result is below USE_DEVICE, randint() gives a hopeless character a slight chance (the "everyone gets a chance" rule). The final chance is clamped to at least 1.

#### Side effect

device_use_chance() calls randint() once for the "everyone gets a chance" rule, so calling it has a side effect on the random number state.

#### Dependency isolation

This file deliberately depends on nothing but randint() so that the calculation can be tested without pulling in the display and global player state.

### missile_serial — missile batch tracking

#### Purpose

Every stack of missiles made in the dungeon gets a unique number, kept in the item's p1 field. Stacking compares it: items_can_stack() (inven_ops.c) lets two lots of the same arrow merge only when their p1 agree, so two batches found in different places stay apart even though they look identical.

#### Counter behavior

The counter walks the whole range of int16_t and wraps from MAX_SHORT back to -MAX_SHORT-1. This is why the old code spelled the step out instead of writing ++: at MAX_SHORT the increment would leave the range, and the value has to stay something p1 can hold.

Wrapping is safe for what the number is used for — by the time it comes round, the batch that first had it is long gone. The numbers mean nothing beyond "not the same batch" — nothing reads them as a count or an order.

#### First batch

The counter starts at 0, and next_missile_serial() steps first then hands out the value. This means the first batch is stamped 1, which matches the old behavior.

#### Save file

missile_serial_value() and set_missile_serial() exist for the save file only (save.c writes it as a short and reads it back). Everything else asks for the next number instead.

### armor_selection — picking worn armor

#### Random selection

pick_random_worn_armor() returns a random worn armor slot (INVEN_BODY, INVEN_ARM, INVEN_OUTER, INVEN_HANDS, INVEN_HEAD, INVEN_FEET) or 0 if no armor is worn. Each worn piece has equal probability.

The function builds a temporary array of worn slots, then picks one at random. It calls randint() exactly once if at least one piece is worn, and does not call it if none are worn.

#### First cursed

pick_first_cursed_armor() returns the first cursed armor piece in the order INVEN_BODY, INVEN_ARM, INVEN_OUTER, INVEN_HEAD, INVEN_HANDS, INVEN_FEET, or 0 if no cursed armor is worn. The order is not the body-slot order (which would be HEAD, BODY, ARM, OUTER, HANDS, FEET) but the order the old code used.

Does not call randint().

## model/monster — monster state and knowledge

### monster_state (monster_levels + monster_list + monster_recall)

The monster modules track three independent pieces of state: the level index (which monsters appear at which dungeon levels), the list of monsters currently on the level, and what the player has learned about each monster kind.

**Level index (monster_levels):** The m_level array counts monsters at each level or shallower (cumulative). Level L's monsters run from m_level[L-1] to m_level[L]-1. The monster definitions are already sorted by level, so the counts form bands and no second table is needed (unlike objects which need sorted_objects). monster_levels_init() scans the creature table once at startup to build the index.

**Monster list (monster_list):** The list of monsters standing on the current level. The table is kept PACKED (no gaps) with a mark saying how far it's filled. Rows 0 and 1 are never used: cave[y][x].cptr uses 0 for "no monster" and 1 for "the player", so MIN_MONIX is 2. The table starts filling there. This is the monster equivalent of the floor_items table — both are indexed by cave squares.

blank_monster (the value for an empty row) stays in model/data/monsters.c with the creature definition table, since it belongs to the read-only-data group rather than this state module.

**Monster recall (monster_recall):** What the player has learned about each kind of monster. The c_recall array parallels the creature definition table. Each entry tracks characteristics seen (r_cdefense, r_cmove), spells cast (r_spells, with frequency counter), treasure carried, kills, and deaths. Update functions set bits, increment counters (capped at MAX_SHORT for kills/deaths and CS_FREQ for spell frequency), and merge the maximum treasure carry count seen.

### monster_turn

creatures() がいまどのモンスターのために動いているか、を 1 つの数で持つ。問いは 1 つで、
「いまモンスターの表を詰めなおしてよいか」。

- 表は詰めて持つ。delete_monster() は最後のモンスターを穴に移して印を 1 つ戻すので、
  穴より上の番号がずれる。手番のあいだは害が無いが、creatures() が表を歩いている最中は
  歩きが番号を握っているので、ずらしてはいけない。
- モンスターを消す 2 か所（monster_place.c の compact_monsters() と、monster_damage.c の
  mon_take_hit() の死）は、先に monster_delete_may_shift() を訊き、詰めてよければ
  delete_monster()、だめなら穴を残す fix1_delete_monster() を選ぶ。
- monster_turn_begin()/end() は creature.c の 5 か所（召喚 2、増殖の place_monster() 2、
  壁に埋まったモンスターへの一撃）を、その 1 呼び出しだけ囲む。囲みを手番全体に広げると、
  その手番のほかの削除が穴を残すようになり、ふるまいが変わる。
- 休みの値は -1。本当の番号は MIN_MONIX（2）以上なので、手番の外ではいつも「詰めてよい」。
  歩きがもう通りすぎた番号（いまの番号より大きい）も「よい」で、召喚の最中の詰めなおしが
  できるのはこのため。
- creature.c のモンスターがモンスターを食べるところは、同じ問いを `monptr < c_ptr->cptr`
  でじかに訊く（歩きの番号が手元にある）。
- セーブファイルには入らない。新しいゲームも読みこんだゲームも「誰の手番でもない」から始まる。
- monster_turn_index() は app/game_state.c の写しとりだけが読む。
