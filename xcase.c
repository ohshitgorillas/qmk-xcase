/* Simple X-Case implementation
* Replaces spaces with delimiters for snake_case, kebab-case, camelCase
*/

#include "xcase.h"

#include "deferred_exec.h"

// private variables
static bool xcase_active = false;
static uint16_t xcase_delimiter = KC_UNDS;
static uint16_t last_keycode = KC_NO;

// public functions
void enable_xcase_with(uint16_t delimiter) {
    xcase_active = true;
    last_keycode = KC_NO;

    switch (delimiter) {
        case KC_LSFT:
        case KC_RSFT:
        case OS_LSFT:
        case OS_RSFT:
            xcase_delimiter = OS_LSFT;  // simplify shift to OS_LSFT for camelCase
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
                enable_xcase_with(OS_LSFT);
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
                if (xcase_delimiter != OS_LSFT) {
                    tap_code(KC_BSPC); // remove the trailing delimiter for non-camelCase
                }
                disable_xcase();
                return true; // Let the second space through
            }

            // replace space with delimiter
            if (xcase_delimiter == OS_LSFT) {
                add_oneshot_mods(MOD_BIT(KC_LSFT));  // add one-shot shift for camelCase
            } else {
                tap_code16(xcase_delimiter);  // send the delimiter
            }
            last_keycode = KC_SPC;
            return false; // do not send space
        }

        // check if this key should continue xcase mode
        switch (base_keycode) {
            // Alphabetic keys
            case KC_A ... KC_Z:
            // Number row
            case KC_1 ... KC_0:
            // Keypad numbers
            case KC_P1 ... KC_P0:
            // common delimiters
            case KC_UNDS:
            case KC_MINS:
            case KC_PMNS:
            // Editing keys
            case KC_BSPC:
            case KC_DEL:
            // navigation keys
            case KC_LEFT:
            case KC_RIGHT:
            case KC_UP:
            case KC_DOWN:
            // Shift keys
            case KC_LSFT:
            case KC_RSFT:
            case OS_LSFT:
            case OS_RSFT:
            // misc
            case KC_ALGR: // alt gr
                last_keycode = base_keycode;
                return true;

            // Delimiter key (dynamic)
            default:
                if (base_keycode == xcase_delimiter) {
                    last_keycode = base_keycode;
                    return true;
                }
                // Any other key terminates xcase
                disable_xcase();
                return true;
        }
    }
    return true;
}
