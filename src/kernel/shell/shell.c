#include "shell.h"
#include "../stdlib/keyboard/input.h"
#include "../stdlib/mem/heap.h"
#include "../stdlib/stdio.h"
#include "../stdlib/string.h"
#include "../stdlib/vga/print.h"

#define MAX_INPUT 128

static void show_prompt() { printf("fshell> "); }

static void shell_ls(fat32_t *fs) {
  if (fat32_listdir(fs, "/") != 0)
    printf("Error listing directory\n");
}

static void shell_cat(fat32_t *fs, const char *filename) {
  uint32_t size;
  if (fat32_file_size(fs, filename, &size) != 0) {
    printf("File not found: %s\n", filename);
    return;
  }

  uint8_t *buf = kmalloc(size + 1);
  if (!buf)
    return;

  int read_bytes = fat32_read(fs, filename, buf, size);
  if (read_bytes <= 0) {
    printf("Error reading file\n");
    kfree(buf);
    return;
  }

  buf[read_bytes] = 0;
  printf("%s\n", buf);
  kfree(buf);
}

int shell(fat32_t *fs) {
  char *input = kmalloc(MAX_INPUT);
  if (!input)
    return -1;

  int buf_pos = 0;

  show_prompt();

  while (1) {
    keyCode_t key = scanKey();
    if (keyReleased(key))
      continue;

    if (isAscii(key)) {
      char c = toAscii(key, false);
      if (buf_pos < MAX_INPUT - 1) {
        input[buf_pos++] = c;
        printf("%c", c);
      }
      continue;
    }

    switch (key) {
    case KEY_ENTER:
      input[buf_pos] = 0;
      printf("\n");

      if (buf_pos > 0) {
        printf("%s\n", input);
        if (strncmp(input, "ls", 2) == 0) {
          shell_ls(fs);
        } else if (strncmp(input, "cat ", 4) == 0) {
          shell_cat(fs, input + 4);
        } else if (strncmp(input, "exit", 4) == 0) {
          kfree(input);
          return 0;
        } else {
          printf("Unknown command: %s\n", input);
        }
      }

      buf_pos = 0;
      memset(input, 0, MAX_INPUT);
      show_prompt();
      break;

    case KEY_BACKSPACE:
      if (buf_pos > 0) {
        buf_pos--;
        printf("\b \b");
      }
      break;

    default:
      break;
    }
  }

  kfree(input);
  return 0;
}
