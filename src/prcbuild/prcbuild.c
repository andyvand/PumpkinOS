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
#else
#include <io.h>
#endif

#include "pdb.h"

#define MAX_RSRC 8192

static int prcbuild(char *filename, char *type, char *creator, char *name, char *rsrc[], int num, int verbose) {
  pdb_t *pdb;
  uint16_t id;
  int64_t size;
  uint8_t *data;
  char *s, stype[16];
  int i, aux, f, len, r = -1;
  FILE *file = NULL;
  FILE *rfile = NULL;

#if __STDC_WANT_SECURE_LIB__
  fopen_s(&file, filename, "wb");

  if (file != NULL) {
#else
  if ((file = fopen(filename, "wb")) != NULL) {
#endif
#ifdef _MSC_VER
    f = _fileno(file);
#else
    f = fileno(file);
#endif
    if (f != -1) {
      if ((pdb = pdb_new(name, type, creator)) != NULL) {
        for (i = 0; i < num; i++) {
          len = (int)strlen(rsrc[i]);
          if (len >= 12) {
            s = &rsrc[i][len-12];
#if __STDC_WANT_SECURE_LIB__
            if (sscanf_s(s, "%4s%04x.bin", stype, 16, &aux) == 2 || sscanf_s(s, "%4s%04x.dat", stype, 16, &aux) == 2) {
#else
            if (sscanf(s, "%4s%04x.bin", stype, &aux) == 2 || sscanf(s, "%4s%04x.dat", stype, &aux) == 2) {
#endif
              id = aux;
#if __STDC_WANT_SECURE_LIB__
              fopen_s(&rfile, rsrc[i], "rb");
#else
              rfile = fopen(rsrc[i], "rb");
#endif
              if (rfile != NULL) {
                fseek(rfile, 0, SEEK_END);
                if ((size = ftell(rfile)) > 0) {
                  fseek(rfile, 0, SEEK_SET);
                  if ((data = malloc((size_t)size)) != NULL) {
                    if (fread(data, 1, (size_t)size, rfile) == size) {
                      if (pdb_add_res(pdb, stype, id, (uint32_t)size, data) == 0) {
                        if (verbose) {
                          fprintf(stderr, "Adding resource %s %5d (%d bytes)\n", stype, id, (int)size);
                        }
                      } else {
                        free(data);
                      }
                    } else {
                      free(data);
                    }
                  }
                }
                fclose(rfile);
              }
            }
          }
        }
        r = pdb_save(pdb, f);
        pdb_destroy(pdb);
      }
      f = 0;
    }
    fclose(file);
  }

  return r;
}

int main(int argc, char *argv[]) {
  char *filename = NULL, *type = NULL, *creator = NULL, *name = NULL;
  char *rsrc[MAX_RSRC], *aux, hex[4], buf[32];
  int i, j, k, num, verbose = 0;

  for (i = 1, num = 0; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
        case 'f':
          filename = argv[++i];
          break;
        case 't':
          type = argv[++i];
          break;
        case 'c':
          creator = argv[++i];
          break;
        case 'n':
          aux = argv[++i];
          for (j = 0, k = 0; aux[j] && k < 31;) {
            if (aux[j] == '\\' && strlen(&aux[j]) >= 4 && aux[j + 1] == 'x') {
              hex[0] = aux[j + 2];
              hex[1] = aux[j + 3];
              hex[2] = 0;
              buf[k++] = (char)strtol(hex, NULL, 16);
              j += 4;
            } else {
              buf[k++] = aux[j++];
            }
          }
          buf[k++] = 0;
	  name = buf;
          break;
        case 'v':
          verbose = 1;
          break;
      }
    } else {
      if (num < MAX_RSRC) {
        rsrc[num++] = argv[i];
      }
    }
  }

  if (filename && type && creator && name && num > 0 && strlen(type) == 4 && strlen(creator) == 4) {
    prcbuild(filename, type, creator, name, rsrc, num, verbose);
  } else {
    fprintf(stderr, "usage: %s -f <filename> -t <type> -c <creator> -n <name> rsrc.bin ...\n", argv[0]);
  }

  exit(0);
}
