# model/ modules design notes

This document preserves the detailed design comments from the small model/* modules before they were merged into larger units in ticket #88.

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

(To be filled in when dungeon modules are merged)

## model/item — item inventory and knowledge

(To be filled in when item modules are merged)

## model/monster — monster state and knowledge

(To be filled in when monster modules are merged)
