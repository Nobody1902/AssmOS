#include "input.h"

keyCode_t scanKey() {
  while (true)
    if (inb(0x64) & 0x1)
      return inb(0x60);
}

bool keyReleased(keyCode_t keycode) { return (keycode & 0x80) != 0; }

bool isAscii(keyCode_t keycode) {
  // Handle extended keys (0xE0 prefix) by ignoring high byte
  unsigned int scancode = (unsigned int)keycode;
  if ((scancode & 0xFF00) == 0xE000) {
    // Extended keys generally do not produce ASCII characters
    return false;
  }

  unsigned char index = (unsigned char)(scancode & 0xFF);
  unsigned char ascii = keycode_to_ascii[index];
  return (ascii != 0);
}

char toAscii(keyCode_t key, bool shiftPressed) {
  if (!isAscii(key))
    return '\0';
  return shiftPressed ? keycode_to_ascii_shift[key] : keycode_to_ascii[key];
}
