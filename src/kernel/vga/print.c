#include "print.h"
#include "string.h"

const static size_t NUM_COLS = 80;
const static size_t NUM_ROWS = 25;

struct Char {
  uint8_t character;
  uint8_t color;
};

struct Char *buffer = (struct Char *)0xb8000;
size_t col = 0;
size_t row = 0;
uint8_t color = PRINT_COLOR_WHITE | PRINT_COLOR_BLACK << 4;

void clear_row(size_t row) {
  struct Char empty = (struct Char){' ', color};

  for (size_t col = 0; col < NUM_COLS; col++) {
    buffer[col + NUM_COLS * row] = empty;
  }
}

void print_clear() {
  for (size_t i = 0; i < NUM_ROWS; i++) {
    clear_row(i);
  }
}
void print_newline() {
  col = 0;

  if (row < NUM_ROWS - 1) {
    row++;
    return;
  }

  for (size_t row = 1; row < NUM_ROWS; row++) {
    for (size_t col = 0; col < NUM_COLS; col++) {
      struct Char character = buffer[col + NUM_COLS * row];
      buffer[col + NUM_COLS * (row - 1)] = character;
    }
  }

  clear_row(NUM_COLS - 1);
}

void print_char(char character) {
  if (character == '\n') {
    print_newline();
    return;
  }
  if (character == '\b') {
    if (col != 0) {
      col--;
    } else {
      return;
    }
    buffer[col + NUM_COLS * row] = (struct Char){' ', color};
    return;
  }
  if (character == '\r') {
    col = 0;
    return;
  }
  if (character == '\0') {
    print_char('\\');
    print_char('0');
    return;
  }
  if (col > NUM_COLS) {
    print_newline();
  }

  buffer[col + NUM_COLS * row] = (struct Char){(uint8_t)character, color};
  col++;
}

void print_str(char *string) {
  for (size_t i = 0; i < strlen(string); i++) {
    print_char((uint8_t)string[i]);
  }
}
void print_line(char *string) {
  print_str(string);
  print_char('\n');
}

void print_set_color(uint8_t foreground, uint8_t background) {
  color = foreground + (background << 4);
}
