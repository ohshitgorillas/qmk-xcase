# xcase: Dynamic Delimiters for QMK
xcase is a QMK feature that dynamically replaces spaces with a chosen delimiter, allowing you to automatically type in snake_case, kebab-case, or camelCase without interrupting your workflow.

This was inspired by the "case modes" scripts at https://github.com/andrewjrae/kyria-keymap/tree/master, which are now out of date due to reliance on a custom Caps Word implementation. xcase, on the other hand, does not rely on Caps Word (which has since become a standard QMK feature) but rather simply captures and replaces spaces.

Compatible with layer-tap and mod-tap keys.


## Adding xcase To Your Keymap
1. Add the xcase module to your QMK directory. You can do this in one of two ways:

   **Option A: Git Submodule (Recommended)**
   ```bash
   cd <qmk directory>
   git submodule add https://github.com/ohshitgorillas/qmk-xcase.git modules/ohshitgorillas/xcase
   ```

   **Option B: Manual Download**
   
   Download or clone this repository and place the module folder (containing xcase.c, xcase.h, etc.) at the following path inside your QMK firmware directory:
   ```
   <qmk directory>/modules/ohshitgorillas/xcase/
   ```

2. Add the following to your `keymap.json` file:

```json
{
    "modules": [
        "ohshitgorillas/xcase"
    ]
}
```

3. Include a way to activate xcase in your `keymap.c` file.


## How to Use
There are two ways to activate xcase: using the built-in keycodes or by calling the function directly. The latter mode supports arbitrary delimiters.

Once active, type your variable name using the spacebar as you normally would. xcase will intercept the space and replace it with your chosen delimiter.


### Built-in Keycodes (easy)
Add the following keycodes to your layout in `keymap.c`:
- `XCASE_SNAKE` [alias `XC_SNK`]: Activates snake_case.
- `XCASE_KEBAB` [alias `XC_KBB`]: Activates kebab-case.
- `XCASE_CAMEL` [alias `XC_CML`] Activates camelCase.
- `XCASE_OFF` [alias `XC_OFF`]: Deactivates xcase mode.

### Custom triggers (advanced)
Call `enable_xcase_with(KC_<delimiter>)` from your keymap to activate the mode. This can be done via the Leader key (e.g., Leader + S, C for snake_case), custom keycodes, or other means.

Leader key example:
```c
void leader_end_user(void) {
  // Leader + S, C for snake_case
  if (leader_sequence_two_keys(KC_S, KC_C)) {
    enable_xcase_with(KC_UNDS);
  }
  // Leader + K, C for kebab-case
  else if (leader_sequence_two_keys(KC_K, KC_C)) {
    enable_xcase_with(KC_MINS);
  }
  // Leader + C, C for camelCase
  else if (leader_sequence_two_keys(KC_C, KC_C)) {
    enable_xcase_with(OS_LSFT);
  }
}
```
For camelCase, you may pass KC_LSFT, KC_RSFT, OS_LSFT, or OS_RSFT. They all resolve to a one-shot left shift.


### Exiting xcase
The mode will terminate in one of two ways:
1. Press any key that is not on the "allowed" list. The allowed keys are:
    a. Alphanumeric (A-Z, 0-9)
    b. Either shift (including one-shots)
    c. Backspace or Delete
    d. Underscore or Minus (including numpad minus)
    e. The delimiter itself (if not listed above)
2. Press the spacebar twice. The script will automatically delete the last trailing delimiter it added (e.g., `my_var_` followed by a space becomes `my_var `), and xcase is exited.

### Adding Accepted Keys
To add keys to the list of keys that won't trigger the end of xcase, locate the following `switch` block in `xcase.c`:

```c
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
        case KC_LEFT:
        case KC_RIGHT:
        // Shift keys
        case KC_LSFT:
        case KC_RSFT:
        case OS_LSFT:
        case OS_RSFT:
        // misc
        case KC_ALGR:
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
```

If you want to keep xcase going after, for example, apostrophes and quotes, add `KC_QUOT` to the list of exceptions:

```c
    // Check if this key should continue xcase mode
    switch (base_keycode) {
        // ... other cases ...
        case KC_QUOT:  // <--- add this to exclude ' or " from ending xcase
        case KC_ALGR:
            last_keycode = base_keycode;
            return true;
        
        // ... default case ...
    }
```

## Functions
These functions are automatically made available to your `keymap.c`.

### enable_xcase_with
```c
void enable_xcase_with(uint16_t delimiter);
```
Enables the xcase feature using the provided keycode as a delimiter.

### disable_xcase
```c
void disable_xcase(void);
```
Disables xcase. This is called automatically whenever a non-accepted keycode is intercepted, or when the user presses space twice.

### is_xcase_active
```c
bool is_xcase_active(void);
```
Returns true when xcase is active.
