#include "input.h"

keyCode_t scanKey() {
  keyCode_t scancode;
  while (true) {
    if (inb(0x64) & 0x1) {
      scancode = inb(0x60);
      if (scancode < 128)
        return scancode;
    }
  }
}

bool keyReleased(keyCode_t keycode) { return (keycode & 0x80) != 0; }

bool isAscii(keyCode_t keycode) {
  if (keycode >= 128)
    return false;
  unsigned char ascii = keycode_to_ascii[keycode];
  return ascii >= 0x20 && ascii <= 0x7E;
}

char toAscii(keyCode_t key, bool shiftPressed) {
  if (!isAscii(key))
    return '\0';
  return shiftPressed ? keycode_to_ascii_shift[key] : keycode_to_ascii[key];
}
