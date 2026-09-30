// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the player is running, and how far

#ifndef RUNNING_H
#define RUNNING_H

// Running is the "." command: the player picks a direction and keeps moving
// until something worth stopping for turns up. While it lasts, the game behaves
// differently in several places that have nothing to do with moving -- the '@' is
// not drawn (map_view.c), the lamp's glow is not painted square by square
// (moria1.c), a monster right next to the player is noticed even in the dark
// (creature.c), blocked ways and objects underfoot are passed over in silence
// (moria3.c), and the main loop does not wait ten seconds for a keypress before
// looking for an interruption (dungeon.c). All of those ask the same one
// question, which is why the answer is kept here.
//
// The run also has a length, and that exists for one reason: a run in an open
// area could otherwise go on forever, so it is cut off (see keep_running below).
// The old global was a single int doing both jobs -- zero meant "not running",
// and any other value was the number of steps so far. The two questions are
// separated here; the number never leaves the module.

// Start running. Said by find_init() (run_path.c) once it knows the first step can
// be taken. The length starts over, so a new run always gets a full 100 steps.
void begin_run(void);

// Is the player running? The question asked in all six files above.
bool player_is_running(void);

// Stop. Said by find_init() when the first step turns out to be impossible, by
// end_find() (run_path.c), and by disturb() (moria1.c). **Stopping is not the whole
// of what those two do** -- end_find() also puts the light back with
// move_light(), disturb() calls check_view(), and both of them do it only when a
// run was actually going on. That part stays with them: the two paths differ, and
// the difference is about the screen, not about running.
void stop_running(void);

// Nobody is running: a level has begun. dungeon.c does this in its setup, next to
// begin_level() (level_exit.h) and forget_pending_teleport()
// (pending_teleport.h). It is not "stop" -- no run is being cut short, and no
// screen work follows -- so it is a separate window.
void forget_run(void);

// One more step of the run, and may it go on? False means the run has gone on
// long enough: find_run() (run_path.c) then says "You stop running to catch your
// breath." and calls end_find(). **Saying no does not stop the run** -- this
// module only counts; stopping is the caller's to do, as it always was.
//
// The number of steps lives here rather than at the call site (the same choice as
// input_ended.h, the opposite of the clamps in level_exit.h) because it is the
// only reason the steps are counted at all, and there is one caller. The original
// wrote it as "find_flag++ > 100", "prevent infinite loops in find mode, will
// stop after moving 100 times".
bool keep_running(void);

// How many steps, for the game state snapshot only (game_state.c copies the old
// global's value). Nothing decides anything from this.
int running_steps(void);

#endif // RUNNING_H
