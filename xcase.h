#pragma once

#include "quantum.h"

/**
 * @brief Enable xcase with a given delimiter.
 * @param delimiter The keycode to use as a delimiter.
 */
void enable_xcase_with(uint16_t delimiter);

/**
 * @brief Disable xcase.
 */
void disable_xcase(void);

/**
 * @brief Check if xcase is active.
 * @return True if xcase is active, false otherwise.
 */
bool is_xcase_active(void);

/**
 * @brief Add a keycode to the exclusion list
 * (keys that will not trigger the end of xcase).
 * @param keycode The keycode to add.
 */
void add_exclusion_keycode(uint16_t keycode);

/**
 * @brief Remove a keycode from the exclusion list
 * (keys that will not trigger the end of xcase).
 * @param keycode The keycode to remove.
 */
void remove_exclusion_keycode(uint16_t keycode);

/**
 * @brief Check if a keycode is on the user's exclusion list
 * (keys that will not trigger the end of xcase).
 * @param keycode The keycode to check.
 * @return True if the keycode is on the user's exclusion list, false otherwise.
 */
bool is_exclusion_keycode(uint16_t keycode);

/**
 * @brief Process a keypress with xcase.
 * @param keycode The keycode of the keypress.
 * @param record The keyrecord to process.
 * @return True if the keypress is returned to the caller, false otherwise.
 */
bool process_record_xcase(uint16_t keycode, keyrecord_t *record);
