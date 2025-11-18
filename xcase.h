#pragma once

#include "quantum.h"

void enable_xcase_with(uint16_t delimiter);
void disable_xcase(void);
bool is_xcase_active(void);

// add/remove exclusion keycodes
void add_exclusion_keycode(uint16_t keycode);
void remove_exclusion_keycode(uint16_t keycode);
bool is_exclusion_keycode(uint16_t keycode);

bool process_record_xcase(uint16_t keycode, keyrecord_t *record);
