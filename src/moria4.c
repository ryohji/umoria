// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Misc code, mainly to handle player commands

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "burden.h"
#include "command_state.h"
#include "dungeon_map.h"
#include "externs.h"
#include "floor_items.h"
#include "equipment.h"
#include "inventory.h"
#include "monster_list.h"
#include "panel.h"
#include "player_attack_bonuses.h"
#include "player_base_to_hit.h"
#include "player_body_weight.h"
#include "player_class.h"
#include "player_disarm.h"
#include "player_level.h"
#include "player_pos.h"
#include "player_search_skill.h"
#include "player_status_flags.h"
#include "player_timed_effects.h"
#include "stats.h"

static bool look_ray(int, int, int);
static bool look_see(int, int, bool *);

/*
  An enhanced look, with peripheral vision. Looking all 8 -CJS- directions will
  see everything which ought to be visible. Can specify direction 5, which looks
  in all directions.

  For the purpose of hindering vision, each place is regarded as a diamond just
  touching its four immediate neighbours. A diamond is opaque if it is a wall,
  or shut door, or something like that. A place is visible if any part of its
  diamond is visible: i.e. there is a line from the view point to part of the
  diamond which does not pass through any opaque diamonds.

  Consider the following situation:

    @....                         X   X   X   X   X
    .##..                        / \ / \ / \ / \ / \
    .....                       X @ X . X . X 1 X . X
                                 \ / \ / \ / \ / \ /
                                  X   X   X   X   X
          Expanded view, with    / \ / \ / \ / \ / \
          diamonds inscribed    X . X # X # X 2 X . X
          about each point,      \ / \ / \ / \ / \ /
          and some locations      X   X   X   X   X
          numbered.              / \ / \ / \ / \ / \
                                X . X . X . X 3 X 4 X
                                 \ / \ / \ / \ / \ /
                                  X   X   X   X   X

       - Location 1 is fully visible.
       - Location 2 is visible, even though partially obscured.
       - Location 3 is invisible, but if either # were
         transparent, it would be visible.
       - Location 4 is completely obscured by a single #.

  The function which does the work is look_ray. It sets up its own co-ordinate
  frame (global variables map back to the dungeon frame) and looks for
  everything between two angles specified from a central line. It is recursive,
  and each call looks at stuff visible along a line parallel to the center line,
  and a set distance away from it. A diagonal look uses more extreme peripheral
  vision from the closest horizontal and vertical directions; horizontal or
  vertical looks take a call for each side of the central line.

  Globally accessed variables: gl_nseen counts the number of places where
  something is seen. gl_rock indicates a look for rock or objects.

  The others map co-ords in the ray frame to dungeon co-ords.

  dungeon y = player_row() + gl_fyx * (ray x) + gl_fyy * (ray y)
  dungeon x = player_col() + gl_fxx * (ray x) + gl_fxy * (ray y)
*/
static int gl_fxx, gl_fxy, gl_fyx, gl_fyy;
static int gl_nseen;
static bool gl_noquery;
static int gl_rock;

// Intended to be indexed by dir/2, since is only
// relevant to horizontal or vertical directions.
static int set_fxy[] = {0, 1, 0, 0, -1};
static int set_fxx[] = {0, 0, -1, 1, 0};
static int set_fyy[] = {0, 0, 1, -1, 0};
static int set_fyx[] = {0, 1, 0, 0, -1};

// Map diagonal-dir/2 to a normal-dir/2.
static int map_diag1[] = {1, 3, 0, 2, 4};
static int map_diag2[] = {2, 1, 0, 4, 3};

#define GRADF 10000 // Any sufficiently big number will do

// Look at what we can see. This is a free move.
//
// Prompts for a direction, and then looks at every object in turn within a cone of
// vision in that direction. For each object, the cursor is moved over the object,
// a description is given, and we wait for the user to type something. Typing
// ESCAPE will abort the entire look.
//
// Looks first at real objects and monsters, and looks at rock types only after all
// other things have been seen.  Only looks at rock types if the highlight_seams
// option is set.
void look(void) {
    int dir;

    if (player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
        msg_print("You can't see a damn thing!");
    } else if (player_timed_in_force(PLAYER_TIMED_HALLUCINATION)) {
        msg_print("You can't believe what you are seeing! It's like a dream!");
    } else if (get_alldir("Look which direction?", &dir)) {
        gl_nseen = 0;
        gl_rock = 0;

        // Have to set this up for the look_see
        gl_noquery = false;

        bool dummy;
        if (look_see(0, 0, &dummy)) {
            // NOTE: `abort` is not read after this so commenting out. -MRC-
            // abort = true;
        } else {
            bool abort;

            do {
                abort = false;
                if (dir == 5) {
                    for (int i = 1; i <= 4; i++) {
                        gl_fxx = set_fxx[i];
                        gl_fyx = set_fyx[i];
                        gl_fxy = set_fxy[i];
                        gl_fyy = set_fyy[i];
                        if (look_ray(0, 2 * GRADF - 1, 1)) {
                            abort = true;
                            break;
                        }
                        gl_fxy = -gl_fxy;
                        gl_fyy = -gl_fyy;
                        if (look_ray(0, 2 * GRADF, 2)) {
                            abort = true;
                            break;
                        }
                    }
                } else if ((dir & 1) == 0) {
                    // Straight directions

                    int i = dir >> 1;
                    gl_fxx = set_fxx[i];
                    gl_fyx = set_fyx[i];
                    gl_fxy = set_fxy[i];
                    gl_fyy = set_fyy[i];
                    if (look_ray(0, GRADF, 1)) {
                        abort = true;
                    } else {
                        gl_fxy = -gl_fxy;
                        gl_fyy = -gl_fyy;
                        abort = look_ray(0, GRADF, 2);
                    }
                } else {
                    int i = map_diag1[dir >> 1];
                    gl_fxx = set_fxx[i];
                    gl_fyx = set_fyx[i];
                    gl_fxy = -set_fxy[i];
                    gl_fyy = -set_fyy[i];
                    if (look_ray(1, 2 * GRADF, GRADF)) {
                        abort = true;
                    } else {
                        i = map_diag2[dir >> 1];
                        gl_fxx = set_fxx[i];
                        gl_fyx = set_fyx[i];
                        gl_fxy = set_fxy[i];
                        gl_fyy = set_fyy[i];
                        abort = look_ray(1, 2 * GRADF - 1, GRADF);
                    }
                }
            } while (abort == false && highlight_seams && (++gl_rock < 2));

            if (abort) {
                msg_print("--Aborting look--");
            } else {
                if (gl_nseen) {
                    if (dir == 5) {
                        msg_print("That's all you see.");
                    } else {
                        msg_print("That's all you see in that direction.");
                    }
                } else if (dir == 5) {
                    msg_print("You see nothing of interest.");
                } else {
                    msg_print("You see nothing of interest in that direction.");
                }
            }
        }
    }
}

// Look at everything within a cone of vision between two ray lines emanating
// from  the player, and y or more places away from the direct line of view.
// This is recursive.
//
// Rays are specified by gradients, y over x, multiplied by 2*GRADF. This is ONLY
// called with gradients between 2*GRADF (45 degrees) and 1 (almost horizontal).
//
//   (y axis)/ angle from
//     ^    /      ___ angle to
//     |   /   ___
//  ...|../.....___.................... parameter y (look at things in the
//     | /   ___                        cone, and on or above this line)
//     |/ ___
//     @-------------------->   direction in which you are looking. (x axis)
//     |
//     |
static bool look_ray(int y, int from, int to) {
    // from is the larger angle of the ray, since we scan towards the
    // center line. If from is smaller, then the ray does not exist.
    if (from <= to || y > MAX_SIGHT) {
        return false;
    }

    // Find first visible location along this line. Minimum x such
    // that (2x-1)/x < from/GRADF <=> x > GRADF(2x-1)/from. This may
    // be called with y=0 whence x will be set to 0. Thus we need a
    // special fix.
    int x = (int)((int32_t)GRADF * (2 * y - 1) / from + 1);
    if (x <= 0) {
        x = 1;
    }

    // Find last visible location along this line.
    // Maximum x such that (2x+1)/x > to/GRADF <=> x < GRADF(2x+1)/to
    int max_x = (int)(((int32_t)GRADF * (2 * y + 1) - 1) / to);
    if (max_x > MAX_SIGHT) {
        max_x = MAX_SIGHT;
    }
    if (max_x < x) {
        return false;
    }

    // gl_noquery is a HACK to prevent doubling up on direct lines of
    // sight. If 'to' is  greater than 1, we do not really look at
    // stuff along the direct line of sight, but we do have to see
    // what is opaque for the purposes of obscuring other objects.
    if ((y == 0 && to > 1) || (y == x && from < GRADF * 2)) {
        gl_noquery = true;
    } else {
        gl_noquery = false;
    }

    bool transparent;
    if (look_see(x, y, &transparent)) {
        return true;
    }
    if (y == x) {
        gl_noquery = false;
    }
    if (transparent) {
        goto init_transparent;
    }

    for (;;) {
        // Look down the window we've found.
        if (look_ray(y + 1, from, ((2 * y + 1) * (int32_t)GRADF / x))) {
            return true;
        }
        // Find the start of next window.
        do {
            if (x == max_x) {
                return false;
            }

            // See if this seals off the scan. (If y is zero, then it will.)
            from = ((2 * y - 1) * (int32_t)GRADF / x);
            if (from <= to) {
                return false;
            }
            x++;
            if (look_see(x, y, &transparent)) {
                return true;
            }
        } while (!transparent);

    init_transparent:
        // Find the end of this window of visibility.
        do {
            if (x == max_x) {
                // The window is trimmed by an earlier limit.
                return look_ray(y + 1, from, to);
            }
            x++;
            if (look_see(x, y, &transparent)) {
                return true;
            }
        } while (transparent);
    }
}

static bool look_see(int x, int y, bool *transparent) {
    bigvtype tmp_str;
    if (x < 0 || y < 0 || y > x) {
        (void)sprintf(tmp_str, "Illegal call to look_see(%d, %d)", x, y);
        msg_print(tmp_str);
    }

    const char *dstring;
    if (x == 0 && y == 0) {
        dstring = "You are on";
    } else {
        dstring = "You see";
    }

    int j = player_col() + gl_fxx * x + gl_fxy * y;
    y = player_row() + gl_fyx * x + gl_fyy * y;
    x = j;
    if (!panel_contains(y, x)) {
        *transparent = false;
        return false;
    }

    cave_type *c_ptr = square_at(y, x);
    *transparent = c_ptr->fval <= MAX_OPEN_SPACE;

    if (gl_noquery) {
        return false; // Don't look at a direct line of sight. A hack.
    }

    // This was uninitialized but the `query == ESCAPE` below was causing
    // a warning. Perhaps we can set it to `ESCAPE` here as default. -MRC-
    char query = ESCAPE;

    msgtype out_val;
    out_val[0] = 0;

    if (gl_rock == 0 && c_ptr->cptr > 1 && monster_list_at(c_ptr->cptr)->ml) {
        creature_type *const r_ptr = monster_get_creature(monster_list_at(c_ptr->cptr)->creature);
        (void)sprintf(out_val, "%s %s %s. [(r)ecall]", dstring, is_a_vowel(r_ptr->name[0]) ? "an" : "a", r_ptr->name);
        dstring = "It is on";
        prt(out_val, 0, 0);
        move_cursor_relative(y, x);
        query = inkey();
        if (query == 'r' || query == 'R') {
            save_screen();
            query = roff_recall(r_ptr);
            restore_screen();
        }
    }

    if (c_ptr->tl || c_ptr->pl || c_ptr->fm) {
        if (c_ptr->tptr != 0) {
            if (floor_item_at(c_ptr->tptr)->tval == TV_SECRET_DOOR) {
                goto granite;
            }
            if (gl_rock == 0 && floor_item_at(c_ptr->tptr)->tval != TV_INVIS_TRAP) {
                bigvtype obj_string;
                objdes(obj_string, floor_item_at(c_ptr->tptr), true);
                (void)snprintf(out_val, sizeof(out_val), "%s %s ---pause---", dstring, obj_string);
                dstring = "It is in";
                prt(out_val, 0, 0);
                move_cursor_relative(y, x);
                query = inkey();
            }
        }

        if ((gl_rock || out_val[0]) && c_ptr->fval >= MIN_CLOSED_SPACE) {
            const char *string;

            switch (c_ptr->fval) {
            case BOUNDARY_WALL:
            case GRANITE_WALL:
            granite:
                // Granite is only interesting if it contains something.
                if (out_val[0]) {
                    string = "a granite wall";
                } else {
                    string = CNIL; // In case we jump here
                }
                break;
            case MAGMA_WALL:
                string = "some dark rock";
                break;
            case QUARTZ_WALL:
                string = "a quartz vein";
                break;
            default:
                string = CNIL;
                break;
            }

            if (string) {
                (void)sprintf(out_val, "%s %s ---pause---", dstring, string);
                prt(out_val, 0, 0);
                move_cursor_relative(y, x);
                query = inkey();
            }
        }
    }

    if (out_val[0]) {
        gl_nseen++;
        if (query == ESCAPE) {
            return true;
        }
    }

    return false;
}
