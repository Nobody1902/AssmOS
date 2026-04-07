#include "input.h"

keyCode_t scanKey() {
  while (true)
    if (inb(0x64) & 0x1)
      return inb(0x60);
}

bool keyReleased(keyCode_t keycode) { return (keycode & 0x80) != 0; }

bool isAscii(keyCode_t keycode) {
  // Ignore extended keys (0xE0 prefix)
  unsigned int scancode = (unsigned int)keycode;
  if ((scancode & 0xFF00) == 0xE000) {
    return false;
  }

  unsigned char index = (unsigned char)(scancode & 0xFF);
  unsigned char ascii = keycode_to_ascii[index];

  // Only printable ASCII (0x20 to 0x7E)
  if (ascii >= 0x20 && ascii <= 0x7E) {
    return true;
  }

  return false;
}

char toAscii(keyCode_t key, bool shiftPressed) {
  if (!isAscii(key))
    return '\0';
  return shiftPressed ? keycode_to_ascii_shift[key] : keycode_to_ascii[key];
}
