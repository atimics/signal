/**
 * @file input_constants.h
 * @brief Input key constants to replace magic numbers
 */

#ifndef INPUT_CONSTANTS_H
#define INPUT_CONSTANTS_H

#include "sokol_app.h"

// Keyboard mappings using Sokol constants
#define KEY_W               SAPP_KEYCODE_W          // 87
#define KEY_A               SAPP_KEYCODE_A          // 65  
#define KEY_S               SAPP_KEYCODE_S          // 83
#define KEY_D               SAPP_KEYCODE_D          // 68
#define KEY_Q               SAPP_KEYCODE_Q          // 81
#define KEY_E               SAPP_KEYCODE_E          // 69

#define KEY_UP              SAPP_KEYCODE_UP         // 265
#define KEY_DOWN            SAPP_KEYCODE_DOWN       // 264
#define KEY_LEFT            SAPP_KEYCODE_LEFT       // 263
#define KEY_RIGHT           SAPP_KEYCODE_RIGHT      // 262

#define KEY_ENTER           SAPP_KEYCODE_ENTER      // 257
#define KEY_SPACE           SAPP_KEYCODE_SPACE      // 32  
#define KEY_ESCAPE          SAPP_KEYCODE_ESCAPE     // 27
#define KEY_TAB             SAPP_KEYCODE_TAB        // 258

#define KEY_LEFT_SHIFT      SAPP_KEYCODE_LEFT_SHIFT // 340
#define KEY_LEFT_CONTROL    SAPP_KEYCODE_LEFT_CONTROL // 341

// Gamepad button mappings (Xbox controller standard)
#define GAMEPAD_BTN_A       0
#define GAMEPAD_BTN_B       1
#define GAMEPAD_BTN_X       2  
#define GAMEPAD_BTN_Y       3
#define GAMEPAD_BTN_LB      4   // Left Bumper
#define GAMEPAD_BTN_RB      5   // Right Bumper
#define GAMEPAD_BTN_LT      6   // Left Trigger
#define GAMEPAD_BTN_RT      7   // Right Trigger

// Gamepad axis mappings
#define GAMEPAD_AXIS_LEFT_X     0   // Left stick horizontal
#define GAMEPAD_AXIS_LEFT_Y     1   // Left stick vertical
#define GAMEPAD_AXIS_RIGHT_X    2   // Right stick horizontal  
#define GAMEPAD_AXIS_RIGHT_Y    3   // Right stick vertical
#define GAMEPAD_AXIS_LT         4   // Left trigger analog
#define GAMEPAD_AXIS_RT         5   // Right trigger analog

#endif // INPUT_CONSTANTS_H