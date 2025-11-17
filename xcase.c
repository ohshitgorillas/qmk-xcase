/* Simple xcase implementation
* Replaces spaces with delimiters for snake_case, kebab-case, camelCase
*/

#include "xcase.h"


// private variables
static bool xcase_active = false;
static uint16_t xcase_delimiter = KC_UNDS;
static uint16_t last_keycode = KC_NO;

#define MAX_EXClUSION_KEYCODES 16
static uint16_t exclusion_keycodes[MAX_EXClUSION_KEYCODES];
static uint8_t exclusion_keycode_count = 0;


// public functions
void enable_xcase_with(uint16_t delimiter) {
    xcase_active = true;
    last_keycode = KC_NO;

    switch (delimiter) {
        case KC_LSFT:
        case KC_RSFT:
        case OS_LSFT:
        case OS_RSFT:
            xcase_delimiter = KC_LSFT;  // simplify shift to KC_LSFT for camelCase
            break;
        default:
            xcase_delimiter = delimiter;  // use the provided delimiter directly
            break;
    }
}


void disable_xcase(void) {
    xcase_active = false;
    last_keycode = KC_NO;
}


bool is_xcase_active(void) {
    return xcase_active;
}


bool is_exclusion_keycode(uint16_t keycode) {
    switch (keycode) {
        // alphanumeric keys
        case KC_A ... KC_Z:
        case KC_1 ... KC_0:
        case KC_P1 ... KC_P0:
        // common delimiters
        case KC_UNDS:
        case KC_MINS:
        case KC_PMNS:
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
        case OS_LSFT:
        case OS_RSFT:
        case KC_ALGR:  // AltGr, also right Alt/Opt
        case KC_LOPT:  // left Opt
            return true;
        default:
            for (uint8_t i = 0; i < exclusion_keycode_count; i++) {
                if (exclusion_keycodes[i] == keycode) {
                    return true;
                }
            }
        return false;
    }
}


void add_exclusion_keycode(uint16_t keycode) {
    if (exclusion_keycode_count >= MAX_EXClUSION_KEYCODES) {
        return;  // List is full
    }
    if (is_exclusion_keycode(keycode)) {
        return;  // Already in list
    }
    exclusion_keycodes[exclusion_keycode_count++] = keycode;
}


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


// main function
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
                if (xcase_delimiter != KC_LSFT) {
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
