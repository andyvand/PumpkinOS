// POSIX directories for classic Mac OS (Retro68). newlib's own <dirent.h>
// is an #error on this target; libpit/macsys.c implements these on top of
// the File Manager (indexed PBGetCatInfo). Paths are POSIX style, see
// macsys.h.

#ifndef MACCLASSIC_DIRENT_H
#define MACCLASSIC_DIRENT_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DT_UNKNOWN 0
#define DT_DIR     4
#define DT_REG     8

struct dirent {
  ino_t d_ino;
  unsigned char d_type;
  char d_name[256];
};

typedef struct mac_dir_t DIR;

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dir);
void rewinddir(DIR *dir);
int closedir(DIR *dir);

#ifdef __cplusplus
}
#endif

#endif
