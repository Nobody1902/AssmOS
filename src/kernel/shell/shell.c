#include "../stdlib/keyboard/input.h"
#include "../stdlib/shutdown.h"
#include "../stdlib/stdio.h"
#include "../stdlib/string.h"

void run(char *buff, int len) {
  if (strcmp(buff, "exit") == 0 || strcmp(buff, "shutdown") == 0) {
    shutdown();
  }

  if (strcmp(buff, "reboot") == 0) {
    reboot();
  }
}

int shell() {
  printf("Hello World from shell!\n");

  bool shiftPressed;
  bool ctrlPressed;

  printf("> ");

  char *start;
  char *buffer = start;

  for (;;) {
    keyCode_t key = scanKey();

    if (keyReleased(key)) {
      key = key & ~0x80; // Remove released bit (0x80)
      if (isAscii(key))
        continue;

      switch (key) {
      case KEY_LEFT_CONTROL:
        ctrlPressed = false;
        break;
      case KEY_LEFT_SHIFT:
        shiftPressed = false;
        break;
      default:
        break;
      }
      continue;
    }
    if (isAscii(key) && !ctrlPressed) {
      printf("%c", toAscii(key, shiftPressed));
      if (key == KEY_ENTER) {
        printf("\n> ");
        *buffer++ = '\0';
        run(start, buffer - start);

        memset(start, 0, buffer - start);
        buffer = start;
      } else {
        *buffer++ = toAscii(key, shiftPressed);
      }
      continue;
    } else if (ctrlPressed) {
      printf("\\%c", toAscii(key, shiftPressed));
      continue;
    }
    switch (key) {
    case KEY_LEFT_CONTROL:
      ctrlPressed = true;
      break;
    case KEY_LEFT_SHIFT:
      shiftPressed = true;
      break;
    default:
      break;
    }
  }
}
