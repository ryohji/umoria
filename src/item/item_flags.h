#ifndef ITEM_FLAGS_H
#define ITEM_FLAGS_H

// Must include config.h, constant.h, types.h before this header
typedef struct inven_type inven_type;

// Display hint setters
void item_show_hit_dam(inven_type *i_ptr);
void item_show_p1(inven_type *i_ptr);
void item_hide_p1(inven_type *i_ptr);

// Display hint readers
bool item_shows_hit_dam(const inven_type *i_ptr);
bool item_shows_p1(const inven_type *i_ptr);
bool item_hides_p1(const inven_type *i_ptr);

// Note readers
bool item_noted_magical(const inven_type *i_ptr);
bool item_noted_empty(const inven_type *i_ptr);
bool item_noted_damned(const inven_type *i_ptr);
bool item_has_any_note(const inven_type *i_ptr);

#endif
