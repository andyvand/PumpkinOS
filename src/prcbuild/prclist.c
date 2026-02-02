#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef __GNUC__
#include <unistd.h>
#endif

#include <fcntl.h>
#include <time.h>

#ifndef _MSC_VER
#include <sys/time.h>
#endif

#include "pdb.h"

static int prclist(char *filename) {
  int f = 0, r = -1;
  FILE *file = NULL;

#if __STDC_WANT_SECURE_LIB__
  fopen_s(&file, filename, "rb");
#else
  file = fopen(filename, "rb");
#endif

  if (file != NULL) {
#ifdef _MSC_VER
    f = _fileno(file);
#else
    f = fileno(file);
#endif

	if (f != -1) {
      r = pdb_list(f);
	}

    f = 0;
    fclose(file);
  }

  return r;
}

int main(int argc, char *argv[]) {
  if (argc == 2) {
    prclist(argv[1]);
  } else {
    fprintf(stderr, "usage: %s <filename>\n", argv[0]);
  }

  return 0;
}
