#include <stdio.h>

#include "game.h"

int gameVariants(void) {
  return 2;
}

char *gameName(void) {
  return "Strife";
}

char *gameVariant(int i) {
  switch (i) {
    case 0: return "Strife";
    case 1: return "Strife (demo)";
  }
  return NULL;
}

char *gameWad(int i) {
  switch (i) {
    case 0: return "strife1.wad";
    case 1: return "strife0.wad";
  }
  return NULL;
}

char *gameMsgOn(void) {
  return "messageson";
}
