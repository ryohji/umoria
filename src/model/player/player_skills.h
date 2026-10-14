// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Player skills: disarm, stealth, saving throw, search, infra-vision range, and speed.

#ifndef PLAYER_SKILLS_H
#define PLAYER_SKILLS_H

// Disarm skill - percentage-like chance to disarm traps and pick locks.
// Bigger means more likely to succeed. Range: ~0-200.
int player_disarm(void);
void player_disarm_set(int chance);
void player_disarm_adjust(int chance);

// Stealth - number of halvings for monster notice chance.
// Bigger means quieter. Range: -1 to 18.
int player_stealth(void);
void player_stealth_set(int stealth);
void player_stealth_adjust(int amount);

// Saving throw - resistance to magical attacks.
// Bigger means better resistance. Range: ~0-100+.
int player_saving_throw(void);
void player_saving_throw_set(int chance);
void player_saving_throw_adjust(int chance);

// Search skill - two values: chance to find and frequency of automatic search.
// Chance is percentage-like, frequency is turns between automatic searches.
int player_search_chance(void);
int player_search_frequency(void);
void player_search_chance_set(int chance);
void player_search_frequency_set(int frequency);
void player_search_skill_adjust(int chance_amount, int frequency_amount);

// Infra-vision range - how many squares the character can see in the dark.
// Measured in grid squares. Range: 0-5.
int player_infra_range(void);
void player_infra_range_set(int squares);
void player_infra_range_adjust(int squares);

// Speed - movement rate modifier.
// Positive is faster, negative is slower. Normal is 0.
int player_speed(void);
void player_speed_set(int num_steps);
void player_speed_adjust(int num_steps);

#endif // PLAYER_SKILLS_H
