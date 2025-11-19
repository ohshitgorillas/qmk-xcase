/* Simple xcase implementation
* Replaces spaces with delimiters for snake_case, kebab-case, camelCase
*/

#include "xcase.h"


// private variables
static bool xcase_active = false;
static uint16_t xcase_delimiter = KC_UNDS;
static uint16_t last_keycode = KC_NO;

#define MAX_EXCLUSION_KEYCODES 16
static uint16_t exclusion_keycodes[MAX_EXCLUSION_KEYCODES];
static uint8_t exclusion_keycode_count = 0;


// public functions

/**
 * @brief Enable xcase with a given delimiter.
 * @param delimiter The keycode to use as a delimiter.
 */
void enable_xcase_with(uint16_t delimiter) {
    switch (delimiter) {
        // special handling for camelCase
        case KC_LSFT:
        case KC_RSFT:
        case OS_LSFT:
        case OS_RSFT:
            // simplify shifts to KC_LSFT
            xcase_delimiter = KC_LSFT;
            break;

        // unacceptable delimiters
#ifdef TRI_LAYER_ENABLE // Ignore Tri Layer keys.
        case QK_TRI_LAYER_LOWER ... QK_TRI_LAYER_UPPER:
#endif
#ifdef LAYER_LOCK_ENABLE // Ignore Layer Lock key.
        case QK_LAYER_LOCK:
#endif
        case !IS_QK_BASIC(delimiter):
        case KC_F1 ... KC_F24:
        case KC_INTERNATIONAL_1 ... KC_LANGUAGE_9:
        case KC_BACKSPACE:
        case KC_SPACE:             // ...seriously?
        case KC_SCRL ... KC_LSCR:  // lock keys
        case KC_LCTL ... KC_RCMD:  // mods (shifts excepted above)
        case KC_PSCR ... KC_EXSL:  // commands
        case KC_PWR ... KC_LPAD:   // power and media keys
            return;  // do not pass go, do not collect $200

        // use the provided delimiter directly
        default:
            xcase_delimiter = delimiter;
            break;
        }
    last_keycode = KC_NO;
    xcase_active = true;
}


/**
 * @brief Disable xcase.
 */
void disable_xcase(void) {
    xcase_active = false;
    last_keycode = KC_NO;
}


/**
 * @brief Check if xcase is active.
 * @return True if xcase is active, false otherwise.
 */
bool is_xcase_active(void) {
    return xcase_active;
}


/**
 * @brief Check if a keycode is an exclusion.
 * Exclusions are keycodes that will not trigger the end of xcase.
 * @param keycode The keycode to check.
 * @return True if the keycode is an exclusion, false otherwise.
 */
bool is_exclusion_keycode(uint16_t keycode) {
    // check if the keycode is in the user's exclusion list
    for (uint8_t i = 0; i < exclusion_keycode_count; i++) {
        if (exclusion_keycodes[i] == keycode) {
            return true;
        }
    }

    switch (keycode) {
#ifdef TRI_LAYER_ENABLE // Ignore Tri Layer keys.
        case QK_TRI_LAYER_LOWER ... QK_TRI_LAYER_UPPER:
#endif
#ifdef LAYER_LOCK_ENABLE // Ignore Layer Lock key.
        case QK_LAYER_LOCK:
#endif
        // alphanumeric keys
        case KC_A ... KC_Z:
        case KC_1 ... KC_0:
        case KC_P1 ... KC_P0:
        // international language keys
        case KC_INTERNATIONAL_1 ... KC_LANGUAGE_9:
        // common delimiters
        case KC_UNDS:
        case KC_MINS:
        case KC_PMNS:  // numpad minus
        // editing keys
        case KC_BSPC:
        case KC_DEL:
        case KC_LEFT:
        case KC_RIGHT:
        case KC_UP:
        case KC_DOWN:
        // modifier keys
        case KC_LSFT:
        case KC_RSFT:
        case KC_LCTL:
        case KC_RCTL:
        case KC_LCMD:  // also Win/GUI
        case KC_RCMD:  // also Win/GUI
        case KC_ROPT:  // also RAlt, AltGr
        case KC_LOPT:  // also LAlt
        case KC_CAPS:
        // layering and one-shot modifier keys
        case IS_QK_MOMENTARY(keycode):
        case IS_QK_DEF_LAYER(keycode):
        case IS_QK_TOGGLE_LAYER(keycode):
        case IS_QK_ONE_SHOT_LAYER(keycode):
        case IS_QK_TO(keycode):
        case IS_QK_LAYER_MOD(keycode):
        case IS_QK_ONE_SHOT_MOD(keycode):
        // the delimiter itself
        case xcase_delimiter:
            return true;
        default:
            return false;
    }
}


/**
 * @brief Add a keycode to the exclusion list.
 * @param keycode The keycode to add.
 */
void add_exclusion_keycode(uint16_t keycode) {
    if (exclusion_keycode_count >= MAX_EXCLUSION_KEYCODES) {
        return;  // List is full
    }
    if (is_exclusion_keycode(keycode)) {
        return;  // Already in list
    }
    exclusion_keycodes[exclusion_keycode_count++] = keycode;
}


/**
 * @brief Remove a keycode from the exclusion list.
 * @param keycode The keycode to remove.
 */
void remove_exclusion_keycode(uint16_t keycode) {
    for (uint8_t i = 0; i < exclusion_keycode_count; i++) {
        if (exclusion_keycodes[i] == keycode) {
            // Shift remaining elements down
            for (uint8_t j = i; j < exclusion_keycode_count - 1; j++) {
                exclusion_keycodes[j] = exclusion_keycodes[j + 1];
            }
            exclusion_keycode_count--;
            return;
        }
    }
}


/**
 * @brief Process a keypress with xcase.
 * @param keycode The keycode of the keypress.
 * @param record The keyrecord to process.
 * @return True if the keypress is returned to the caller, false otherwise.
 */
bool process_record_xcase(uint16_t keycode, keyrecord_t *record) {
    // Handle activation/deactivation keycodes first
    if (record->event.pressed) {
        switch (keycode) {
            case XCASE_SNAKE:
                enable_xcase_with(KC_UNDS);
                return false; // Keycode handled
            case XCASE_KEBAB:
                enable_xcase_with(KC_MINS);
                return false; // Keycode handled
            case XCASE_CAMEL:
                enable_xcase_with(KC_LSFT);
                return false; // Keycode handled
            case XCASE_OFF:
                disable_xcase();
                return false; // Keycode handled
        }
    }

    // If not active, pass all keys through
    if (!xcase_active) {
        return true;
    }

    // process on key down
    if (record->event.pressed) {
        // Strip mod-tap and layer-tap to get base keycode
        uint16_t base_keycode = keycode;
        if (IS_QK_MOD_TAP(keycode)) {
            base_keycode = QK_MOD_TAP_GET_TAP_KEYCODE(keycode);
        } else if (IS_QK_LAYER_TAP(keycode)) {
            base_keycode = QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
        }

        if (base_keycode == KC_SPC) {
            // check for double space to exit xcase mode
            if (last_keycode == KC_SPC) {
                if (xcase_delimiter != KC_LSFT &&
                    xcase_delimiter != KC_CAPS)
                {
                    tap_code(KC_BSPC); // remove the trailing delimiter for non-camelCase
                }
                disable_xcase();
                return true; // Let the second space through
            }

            // replace space with delimiter
            if (xcase_delimiter == KC_LSFT) {
                add_oneshot_mods(MOD_BIT(xcase_delimiter));  // add one-shot shift for camelCase
            } else {
                tap_code16(xcase_delimiter);  // send the delimiter
            }
            last_keycode = KC_SPC;
            return false; // do not send space
        }

        // check if this key should continue xcase mode
        if (!is_exclusion_keycode(base_keycode)) {
            disable_xcase();
        } else {
            last_keycode = base_keycode;
        }
        return true;
    }
    return true;
}
