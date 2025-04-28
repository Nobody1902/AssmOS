#include "../stdlib/port.h"
#include "stdbool.h"
#include "stdint.h"

#ifndef INPUT_H
#define INPUT_H

typedef enum keyCode {
  // No event
  KEY_NONE = 0x00,

  // Standard keys (make sure these match your scancode set)
  KEY_ESCAPE = 0x01,
  KEY_1 = 0x02,
  KEY_2 = 0x03,
  KEY_3 = 0x04,
  KEY_4 = 0x05,
  KEY_5 = 0x06,
  KEY_6 = 0x07,
  KEY_7 = 0x08,
  KEY_8 = 0x09,
  KEY_9 = 0x0A,
  KEY_0 = 0x0B,
  KEY_MINUS = 0x0C,
  KEY_EQUALS = 0x0D,
  KEY_BACKSPACE = 0x0E,
  KEY_TAB = 0x0F,

  KEY_Q = 0x10,
  KEY_W = 0x11,
  KEY_E = 0x12,
  KEY_R = 0x13,
  KEY_T = 0x14,
  KEY_Y = 0x15,
  KEY_U = 0x16,
  KEY_I = 0x17,
  KEY_O = 0x18,
  KEY_P = 0x19,
  KEY_LEFT_BRACKET = 0x1A,
  KEY_RIGHT_BRACKET = 0x1B,
  KEY_ENTER = 0x1C,
  KEY_LEFT_CONTROL = 0x1D,

  KEY_A = 0x1E,
  KEY_S = 0x1F,
  KEY_D = 0x20,
  KEY_F = 0x21,
  KEY_G = 0x22,
  KEY_H = 0x23,
  KEY_J = 0x24,
  KEY_K = 0x25,
  KEY_L = 0x26,
  KEY_SEMICOLON = 0x27,
  KEY_APOSTROPHE = 0x28,
  KEY_GRAVE = 0x29,
  KEY_LEFT_SHIFT = 0x2A,
  KEY_BACKSLASH = 0x2B,

  KEY_Z = 0x2C,
  KEY_X = 0x2D,
  KEY_C = 0x2E,
  KEY_V = 0x2F,
  KEY_B = 0x30,
  KEY_N = 0x31,
  KEY_M = 0x32,
  KEY_COMMA = 0x33,
  KEY_PERIOD = 0x34,
  KEY_SLASH = 0x35,
  KEY_RIGHT_SHIFT = 0x36,
  KEY_KEYPAD_ASTERISK = 0x37,
  KEY_LEFT_ALT = 0x38,
  KEY_SPACE = 0x39,
  KEY_CAPS_LOCK = 0x3A,

  // Function keys
  KEY_F1 = 0x3B,
  KEY_F2 = 0x3C,
  KEY_F3 = 0x3D,
  KEY_F4 = 0x3E,
  KEY_F5 = 0x3F,
  KEY_F6 = 0x40,
  KEY_F7 = 0x41,
  KEY_F8 = 0x42,
  KEY_F9 = 0x43,
  KEY_F10 = 0x44,
  KEY_NUM_LOCK = 0x45,
  KEY_SCROLL_LOCK = 0x46,

  // Keypad keys
  KEY_KEYPAD_7 = 0x47,
  KEY_KEYPAD_8 = 0x48,
  KEY_KEYPAD_9 = 0x49,
  KEY_KEYPAD_MINUS = 0x4A,
  KEY_KEYPAD_4 = 0x4B,
  KEY_KEYPAD_5 = 0x4C,
  KEY_KEYPAD_6 = 0x4D,
  KEY_KEYPAD_PLUS = 0x4E,
  KEY_KEYPAD_1 = 0x4F,
  KEY_KEYPAD_2 = 0x50,
  KEY_KEYPAD_3 = 0x51,
  KEY_KEYPAD_0 = 0x52,
  KEY_KEYPAD_DOT = 0x53,

  // OEM / Misc keys
  KEY_OEM_1 = 0x54, // (varies by keyboard)
  KEY_OEM_2 = 0x55,
  KEY_OEM_3 = 0x56,

  KEY_F11 = 0x57,
  KEY_F12 = 0x58,

  // Extended keys (0xE0 prefix in scancode set 1)
  KEY_RIGHT_CONTROL = 0xE01D,
  KEY_RIGHT_ALT = 0xE038,
  KEY_HOME = 0xE047,
  KEY_UP_ARROW = 0xE048,
  KEY_PAGE_UP = 0xE049,
  KEY_LEFT_ARROW = 0xE04B,
  KEY_RIGHT_ARROW = 0xE04D,
  KEY_END = 0xE04F,
  KEY_DOWN_ARROW = 0xE050,
  KEY_PAGE_DOWN = 0xE051,
  KEY_INSERT = 0xE052,
  KEY_DELETE = 0xE053,
  KEY_LEFT_META = 0xE05B,
  KEY_RIGHT_META = 0xE05C,
  KEY_MENU = 0xE05D,

  // Key press and release events for modifiers (example)
  LEFT_CONTROL_PRESSED = 0x1D,
  LEFT_CONTROL_RELEASED = 0x9D, // 0x1D + 0x80 (break code)
  LEFT_SHIFT_PRESSED = 0x2A,
  LEFT_SHIFT_RELEASED = 0xAA, // 0x2A + 0x80
  RIGHT_SHIFT_PRESSED = 0x36,
  RIGHT_SHIFT_RELEASED = 0xB6, // 0x36 + 0x80
  LEFT_ALT_PRESSED = 0x38,
  LEFT_ALT_RELEASED = 0xB8, // 0x38 + 0x80
  RIGHT_CONTROL_PRESSED = 0xE01D,
  RIGHT_CONTROL_RELEASED = 0xE09D,
  RIGHT_ALT_PRESSED = 0xE038,
  RIGHT_ALT_RELEASED = 0xE0B8,

  // Add more keys or events as needed...

} keyCode_t;

static const char keycode_to_ascii[256] = {
    /* 0x00 - 0x0F */
    '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    '\b', '\t',

    /* 0x10 - 0x1F */
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '\0', 'a',
    's',

    /* 0x20 - 0x2F */
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0', '\\', 'z', 'x',
    'c', 'v',

    /* 0x30 - 0x3F */
    'b', 'n', 'm', ',', '.', '/', '\0', '*', '\0', ' ', '\0', '\0', '\0', '\0',
    '\0', '\0',

    /* 0x40 - 0x4F */
    '\0', '\0', '\0', '\0', '\0', '\0', '7', '8', '9', '-', '4', '5', '6', '+',
    '1', '2',

    /* 0x50 - 0x5F */
    '3', '0', '.', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0',

    /* 0x60 - 0x6F */
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0',

    /* 0x70 - 0x7F */
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0',

    /* 0x80 - 0xFF all unmapped */
    [0x80 ... 0xFF] = '\0'};

static const char keycode_to_ascii_shift[256] = {
    /* 0x00 - 0x0F */
    '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
    '\b', '\t',

    /* 0x10 - 0x1F */
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', '\0', 'A',
    'S',

    /* 0x20 - 0x2F */
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', '\0', '|', 'Z', 'X', 'C',
    'V',

    /* 0x30 - 0x3F */
    'B', 'N', 'M', '<', '>', '?', '\0', '*', '\0', ' ', '\0', '\0', '\0', '\0',
    '\0', '\0',

    /* 0x40 - 0x4F */
    '\0', '\0', '\0', '\0', '\0', '\0', '7', '8', '9', '-', '4', '5', '6', '+',
    '1', '2',

    /* 0x50 - 0x5F */
    '3', '0', '.', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0',

    /* 0x60 - 0x6F */
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0',

    /* 0x70 - 0x7F */
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0',

    /* 0x80 - 0xFF all unmapped */
    [0x80 ... 0xFF] = '\0'};

keyCode_t scanKey();
bool keyReleased(keyCode_t keycode);
bool isAscii(keyCode_t keycode);
char toAscii(keyCode_t key, bool shiftPressed);

#endif
