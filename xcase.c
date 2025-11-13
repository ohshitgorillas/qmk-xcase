/* Simple X-Case implementation
 * Replaces spaces with delimiters for snake_case, kebab-case, camelCase
 */

 #include "xcase.h"

 #include "deferred_exec.h"

 static bool xcase_active = false;
 static uint16_t xcase_delimiter = KC_UNDS;
 static uint16_t last_keycode = KC_NO;

 void enable_xcase_with(uint16_t delimiter) {
     xcase_active = true;
     xcase_delimiter = delimiter;
     last_keycode = KC_NO;

     // if the delimiter is a shift key, convert it to OS_LSFT for simplicity
     if (delimiter == KC_LSFT || delimiter == KC_RSFT || delimiter == OS_LSFT || delimiter == OS_RSFT) {
        xcase_delimiter = OS_LSFT;
     }
 }

 void disable_xcase(void) {
     xcase_active = false;
     last_keycode = KC_NO;
 }

 bool is_xcase_active(void) {
     return xcase_active;
 }

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

     // Only process on key press
     if (!record->event.pressed) {
         return true;
     }

     // Strip mod-tap and layer-tap to get base keycode
     uint16_t base_keycode = keycode;
     if (IS_QK_MOD_TAP(keycode)) {
         base_keycode = QK_MOD_TAP_GET_TAP_KEYCODE(keycode);
     } else if (IS_QK_LAYER_TAP(keycode)) {
         base_keycode = QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
     }

     // Replace space with delimiter
     if (base_keycode == KC_SPC) {
         // Double space exits xcase mode
         if (last_keycode == KC_SPC) {
             if (xcase_delimiter != OS_LSFT) {
               tap_code(KC_BSPC); // remove the trailing delimiter
             }
             disable_xcase();
             return true; // Let the second space through
         }

         // replace space with one-shot shift
         switch (xcase_delimiter) {
            case OS_LSFT:
            case KC_LSFT:
            case KC_RSFT:
            case OS_RSFT:
                add_oneshot_mods(MOD_BIT(KC_LSFT));
                break;
            default:
                tap_code16(xcase_delimiter);
                break;
         }

         last_keycode = KC_SPC;
         return false; // Do not send space
     }

     // Check if this key should continue xcase mode
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
