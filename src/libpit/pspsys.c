// POSIX functions missing from the pspsdk newlib glue (PlayStation Portable
// build).
//
// ftruncate(): the glue has truncate() (by path) but no ftruncate(), and
// the PSP I/O driver has no call to shorten an open file. The glue's
// descriptor table remembers the name a file was opened with, so the file
// is rewritten through a second descriptor: its first `length` bytes are
// copied to a side file, it is reopened with O_TRUNC and the bytes are
// copied back (extending a file just pads it with zeros through the given
// descriptor). The caller's descriptor keeps its offset, like POSIX says.
// Used by vfs_truncate() when a database shrinks.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>

// libcglue's fdman.h (not installed): the descriptor table
#define PSP_FILENO_MAX 1024
#define PSP_DESCRIPTOR_FILE 0

typedef struct {
  uint32_t descriptor;
  uint32_t flags;
  uint32_t ref_count;
  char *filename;
  uint8_t type;
} psp_descriptor_t;

extern psp_descriptor_t *__descriptormap[PSP_FILENO_MAX];

static int copy_bytes(int in, int out, off_t left, uint8_t *buf, int bsize) {
  int n;

  while (left > 0) {
    n = left > bsize ? bsize : (int)left;
    if ((n = read(in, buf, n)) <= 0) return -1;
    if (write(out, buf, n) != n) return -1;
    left -= n;
  }

  return 0;
}

int ftruncate(int fd, off_t length) {
  psp_descriptor_t *d;
  uint8_t buf[4096];
  char *tmp;
  off_t pos, size, left;
  int in, out, n, r = -1;

  if (fd < 0 || fd >= PSP_FILENO_MAX || (d = __descriptormap[fd]) == NULL || d->type != PSP_DESCRIPTOR_FILE || d->filename == NULL) {
    errno = EBADF;
    return -1;
  }
  if (length < 0) {
    errno = EINVAL;
    return -1;
  }

  if ((pos = lseek(fd, 0, SEEK_CUR)) < 0 || (size = lseek(fd, 0, SEEK_END)) < 0) {
    return -1;
  }

  if (size == length) {
    lseek(fd, pos, SEEK_SET);
    return 0;
  }

  if (length > size) {
    // extend: pad with zeros
    memset(buf, 0, sizeof(buf));
    for (left = length - size, r = 0; left > 0; left -= n) {
      n = left > (off_t)sizeof(buf) ? (int)sizeof(buf) : (int)left;
      if (write(fd, buf, n) != n) {
        r = -1;
        break;
      }
    }
    lseek(fd, pos, SEEK_SET);
    return r;
  }

  // shrink: keep the head in a side file, rewrite the file from it
  if ((tmp = malloc(strlen(d->filename) + 8)) == NULL) {
    lseek(fd, pos, SEEK_SET);
    errno = ENOMEM;
    return -1;
  }
  sprintf(tmp, "%s.trunc", d->filename);

  if ((out = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644)) >= 0) {
    lseek(fd, 0, SEEK_SET);
    r = copy_bytes(fd, out, length, buf, sizeof(buf));
    close(out);

    if (r == 0) {
      r = -1;
      if ((in = open(tmp, O_RDONLY, 0)) >= 0) {
        if ((out = open(d->filename, O_WRONLY | O_TRUNC, 0644)) >= 0) {
          r = copy_bytes(in, out, length, buf, sizeof(buf));
          close(out);
        }
        close(in);
      }
    }
    unlink(tmp);
  }

  free(tmp);
  lseek(fd, pos, SEEK_SET);

  return r;
}
