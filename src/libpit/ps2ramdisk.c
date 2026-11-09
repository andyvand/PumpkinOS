// RAM disk in the main memory of the PlayStation 2 (ps2sdk newlib glue).
//
// The PS2 counterpart of the GameCube's ARAM RAM disk (gcramdisk.c): the
// PumpkinOS home when nothing better is wanted, instead of a memory card.
// A memory card is very slow (a few hundred milliseconds per small file:
// the first deploy of the application storage, about 1250 files, took more
// than 15 minutes) and has only 3 file handles; the storage itself is small
// (about 800KB of data). So the home lives in RAM, seeded from the boot
// medium (the disc, a USB stick, the host) at every boot. It does not
// survive a reboot.
//
// Unlike the GameCube the PS2 has no spare memory the CPU cannot address
// (the 2MB of the IOP and of the sound processor and the 4MB of the GS are
// taken by the drivers and the frame buffers), so the files live in the EE
// heap: every file is one malloc() block holding its whole contents, grown
// as it is written and trimmed to its size when the last descriptor is
// closed. The total (contents, names and nodes) is capped (see
// sys_ps2_ramdisk_mount) so that the RAM disk cannot eat the memory the
// window provider and the tasks still need; past the cap writes fail with
// ENOSPC.
//
// The device is plugged into the C library glue (libcglue): its path
// operation table (open, stat, mkdir, rmdir, remove, rename) is replaced by
// one that handles "<name>:" paths here and passes everything else to the
// previous table (fileXio: memory cards, USB, CD, host...), and the
// descriptors it opens carry their own file operations, so open/read/write/
// lseek/close/stat/opendir/readdir/mkdir/unlink/rename work on it like on
// any other device.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ps2sdkapi.h>

#include "sys.h"
#include "debug.h"

#define RD_MAX_NAME   255
#define RD_MAX_DEV    15
// bookkeeping charged for every node on top of its name and contents
// (the node itself plus the malloc headers)
#define RD_NODE_COST  (sizeof(rd_node_t) + 32)
// a file keeps up to this much unused capacity after its last close
#define RD_SLACK      64

typedef struct rd_node {
  char *name;
  struct rd_node *parent;
  struct rd_node *next;    // next entry in the parent directory
  struct rd_node *child;   // first entry (directories)
  int isdir;
  uint8_t *data;
  uint32_t size;
  uint32_t cap;
  time_t mtime;
  int open;                // open descriptors
  int deleted;             // unlinked while open: freed on the last close
} rd_node_t;

typedef struct {
  rd_node_t *node;
  uint32_t pos;
  int flags;
  // directories: a snapshot of the entry names taken at open time (entries
  // may be removed while the listing goes on), one after the other
  char *names;
  int nnames, cur, off;
  char filename[];         // the full path, for fstat()
} rd_file_t;

static rd_node_t rd_root;
static uint32_t rd_used;   // bytes charged (contents capacity, names, nodes)
static uint32_t rd_peak;
static uint32_t rd_limit;
static uint32_t rd_nfiles;
static char rd_dev[RD_MAX_DEV + 2];  // "<name>:"
static int rd_devlen;
static int rd_mounted;

static _libcglue_fdman_path_ops_t *rd_next_ops;
static _libcglue_fdman_path_ops_t rd_path_ops;
static _libcglue_fdman_fd_ops_t rd_file_ops;
static _libcglue_fdman_fd_ops_t rd_dir_ops;

static void rd_charge(int32_t delta) {
  rd_used += delta;
  if (rd_used > rd_peak) rd_peak = rd_used;
}

static int rd_is_mine(const char *path) {
  return rd_mounted && path && !strncmp(path, rd_dev, rd_devlen);
}

// the path after "<name>:" without leading separators
static const char *rd_strip(const char *path) {
  path += rd_devlen;
  while (*path == '/' || *path == '\\') path++;
  return path;
}

static rd_node_t *rd_find_child(rd_node_t *dir, const char *name, int len) {
  rd_node_t *n;

  for (n = dir->child; n; n = n->next) {
    if ((int)strlen(n->name) == len && !strncmp(n->name, name, len)) return n;
  }

  return NULL;
}

// Resolves a path. Returns the node (NULL when it does not exist); *parent
// gets the directory that holds (or would hold) the last component, *last
// and *lastlen that component (NULL parent: a directory on the way is
// missing or is a file, or the name is too long).
static rd_node_t *rd_lookup(const char *path, rd_node_t **parent, const char **last, int *lastlen) {
  rd_node_t *dir = &rd_root, *n = &rd_root;
  const char *p = rd_strip(path), *s;
  int len;

  if (parent) *parent = NULL;
  if (last) *last = NULL;
  if (lastlen) *lastlen = 0;

  while (*p) {
    for (s = p; *s && *s != '/' && *s != '\\'; s++);
    len = (int)(s - p);
    while (*s == '/' || *s == '\\') s++;
    if (len > RD_MAX_NAME) return NULL;
    if (len == 1 && p[0] == '.') {
      p = s;
      continue;
    }
    if (len == 2 && p[0] == '.' && p[1] == '.') {
      if (n->parent) n = n->parent;
      dir = n;
      p = s;
      continue;
    }
    if (!n->isdir) {
      // a file on the way
      if (parent) *parent = NULL;
      if (last) *last = NULL;
      if (lastlen) *lastlen = 0;
      return NULL;
    }
    dir = n;
    if (parent) *parent = dir;
    if (last) *last = p;
    if (lastlen) *lastlen = len;
    n = rd_find_child(dir, p, len);
    if (n == NULL) {
      // only the last component may be missing
      if (*s && parent) *parent = NULL;
      return NULL;
    }
    p = s;
  }

  if (n == &rd_root && parent) *parent = NULL;
  return n;
}

static rd_node_t *rd_create(rd_node_t *dir, const char *name, int len, int isdir) {
  rd_node_t *n;

  if (rd_used + RD_NODE_COST + len + 1 > rd_limit) return NULL;
  if ((n = calloc(1, sizeof(rd_node_t))) == NULL) return NULL;
  if ((n->name = malloc(len + 1)) == NULL) {
    free(n);
    return NULL;
  }
  memcpy(n->name, name, len);
  n->name[len] = 0;
  n->isdir = isdir;
  n->parent = dir;
  n->mtime = time(NULL);
  n->next = dir->child;
  dir->child = n;
  dir->mtime = n->mtime;
  rd_charge(RD_NODE_COST + len + 1);
  if (!isdir) rd_nfiles++;

  return n;
}

static void rd_detach(rd_node_t *n) {
  rd_node_t **p;

  if (n->parent == NULL) return;
  for (p = &n->parent->child; *p; p = &(*p)->next) {
    if (*p == n) {
      *p = n->next;
      break;
    }
  }
  n->parent->mtime = time(NULL);
  n->parent = NULL;
  n->next = NULL;
}

static void rd_free_node(rd_node_t *n) {
  rd_charge(-(int32_t)(RD_NODE_COST + strlen(n->name) + 1 + n->cap));
  if (!n->isdir) rd_nfiles--;
  free(n->data);
  free(n->name);
  free(n);
}

// makes room for `size` bytes of contents
static int rd_reserve(rd_node_t *n, uint32_t size) {
  uint32_t cap;
  uint8_t *data;

  if (size <= n->cap) return 0;
  cap = n->cap + n->cap / 2;
  if (cap < size) cap = size;
  if (cap < 64) cap = 64;
  cap = (cap + 15) & ~15;
  if (rd_used - n->cap + cap > rd_limit) {
    // no room for the growth margin: just what is needed
    cap = (size + 15) & ~15;
    if (rd_used - n->cap + cap > rd_limit) return -ENOSPC;
  }
  if ((data = realloc(n->data, cap)) == NULL) return -ENOSPC;
  rd_charge((int32_t)cap - (int32_t)n->cap);
  n->data = data;
  n->cap = cap;

  return 0;
}

// gives back the unused capacity
static void rd_trim(rd_node_t *n) {
  uint32_t cap = (n->size + 15) & ~15;
  uint8_t *data;

  if (n->cap <= cap + RD_SLACK) return;
  if (cap == 0) {
    free(n->data);
    n->data = NULL;
  } else if ((data = realloc(n->data, cap)) != NULL) {
    n->data = data;
  } else {
    return;
  }
  rd_charge((int32_t)cap - (int32_t)n->cap);
  n->cap = cap;
}

static int rd_set_size(rd_node_t *n, uint32_t size) {
  int r;

  if (size > n->size) {
    if ((r = rd_reserve(n, size)) != 0) return r;
    memset(n->data + n->size, 0, size - n->size);
  }
  n->size = size;
  n->mtime = time(NULL);

  return 0;
}

static void rd_fill_stat(rd_node_t *n, struct stat *st) {
  memset(st, 0, sizeof(struct stat));
  st->st_mode = n->isdir ? (S_IFDIR | 0777) : (S_IFREG | 0666);
  st->st_size = n->isdir ? 0 : n->size;
  st->st_nlink = 1;
  st->st_blksize = 512;
  st->st_blocks = (n->size + 511) / 512;
  st->st_atime = st->st_mtime = st->st_ctime = n->mtime;
}

// --- descriptor operations

static int rd_getfd(void *userdata) {
  return 0;
}

static char *rd_getfilename(void *userdata) {
  return userdata ? ((rd_file_t *)userdata)->filename : NULL;
}

static int rd_close(void *userdata) {
  rd_file_t *f = (rd_file_t *)userdata;
  rd_node_t *n;

  if (f == NULL) return -EBADF;
  n = f->node;
  if (n && --n->open == 0) {
    if (n->deleted) {
      rd_free_node(n);
    } else if (!n->isdir) {
      rd_trim(n);
    }
  }
  free(f->names);
  free(f);

  return 0;
}

static int rd_read(void *userdata, void *buf, int nbytes) {
  rd_file_t *f = (rd_file_t *)userdata;
  rd_node_t *n = f->node;
  uint32_t len;

  if ((f->flags & O_ACCMODE) == O_WRONLY) return -EBADF;
  if (nbytes <= 0 || f->pos >= n->size) return 0;
  len = n->size - f->pos;
  if (len > (uint32_t)nbytes) len = nbytes;
  memcpy(buf, n->data + f->pos, len);
  f->pos += len;

  return (int)len;
}

static int rd_write(void *userdata, const void *buf, int nbytes) {
  rd_file_t *f = (rd_file_t *)userdata;
  rd_node_t *n = f->node;
  uint32_t end;
  int r;

  if ((f->flags & O_ACCMODE) == O_RDONLY) return -EBADF;
  if (nbytes <= 0) return 0;
  if (f->flags & O_APPEND) f->pos = n->size;
  end = f->pos + nbytes;
  if (end < f->pos) return -EFBIG;
  if (end > n->size) {
    if ((r = rd_reserve(n, end)) != 0) return r;
    if (f->pos > n->size) memset(n->data + n->size, 0, f->pos - n->size);
    n->size = end;
  }
  memcpy(n->data + f->pos, buf, nbytes);
  f->pos = end;
  n->mtime = time(NULL);

  return nbytes;
}

static int64_t rd_lseek64(void *userdata, int64_t offset, int whence) {
  rd_file_t *f = (rd_file_t *)userdata;
  int64_t pos;

  switch (whence) {
    case SEEK_SET: pos = offset; break;
    case SEEK_CUR: pos = (int64_t)f->pos + offset; break;
    case SEEK_END: pos = (int64_t)f->node->size + offset; break;
    default: return -EINVAL;
  }
  if (pos < 0 || pos > 0x7FFFFFFF) return -EINVAL;
  f->pos = (uint32_t)pos;

  return pos;
}

static int rd_lseek(void *userdata, int offset, int whence) {
  return (int)rd_lseek64(userdata, offset, whence);
}

static int rd_dread(void *userdata, struct dirent *dir) {
  rd_file_t *f = (rd_file_t *)userdata;
  rd_node_t *n;
  char *name;

  // the snapshot entries still present, in order
  while (f->cur < f->nnames) {
    name = f->names + f->off;
    f->off += strlen(name) + 1;
    f->cur++;
    if (f->node->deleted) break;
    if ((n = rd_find_child(f->node, name, strlen(name))) == NULL) continue;
    memset(dir, 0, sizeof(struct dirent));
    dir->d_fileno = f->cur;
    dir->d_type = n->isdir ? DT_DIR : DT_REG;
    strncpy(dir->d_name, n->name, MAXNAMLEN);
    dir->d_name[MAXNAMLEN] = 0;
    return 1;
  }

  return 0;
}

// --- path operations (the ones not for the RAM disk go to rd_next_ops)

static int rd_open(_libcglue_fdman_fd_info_t *info, const char *path, int flags, mode_t mode) {
  rd_node_t *n, *parent;
  rd_file_t *f;
  const char *last;
  int lastlen, plen, r, total, count;

  if (!rd_is_mine(path)) {
    return rd_next_ops && rd_next_ops->open ? rd_next_ops->open(info, path, flags, mode) : -ENOSYS;
  }

  n = rd_lookup(path, &parent, &last, &lastlen);
  if (flags & O_DIRECTORY) {
    if (n == NULL) return -ENOENT;
    if (!n->isdir) return -ENOTDIR;
  } else if (n == NULL) {
    if (!(flags & O_CREAT)) return -ENOENT;
    if (parent == NULL || lastlen == 0) return -ENOENT;
    if ((n = rd_create(parent, last, lastlen, 0)) == NULL) return -ENOSPC;
  } else {
    if ((flags & O_CREAT) && (flags & O_EXCL)) return -EEXIST;
    if (n->isdir) return -EISDIR;
    if ((flags & O_TRUNC) && (flags & O_ACCMODE) != O_RDONLY) {
      if ((r = rd_set_size(n, 0)) != 0) return r;
    }
  }

  plen = strlen(path);
  if ((f = calloc(1, sizeof(rd_file_t) + plen + 1)) == NULL) return -ENOMEM;
  memcpy(f->filename, path, plen + 1);
  f->node = n;
  f->flags = flags;

  if (n->isdir) {
    rd_node_t *c;
    char *p;

    for (c = n->child, total = 0, count = 0; c; c = c->next) {
      total += strlen(c->name) + 1;
      count++;
    }
    if (total > 0) {
      if ((f->names = malloc(total)) == NULL) {
        free(f);
        return -ENOMEM;
      }
      for (c = n->child, p = f->names; c; c = c->next) {
        strcpy(p, c->name);
        p += strlen(c->name) + 1;
      }
    }
    f->nnames = count;
  }

  n->open++;
  info->userdata = f;
  info->ops = n->isdir ? &rd_dir_ops : &rd_file_ops;

  return 0;
}

static int rd_stat(const char *path, struct stat *st) {
  rd_node_t *n;

  if (!rd_is_mine(path)) {
    return rd_next_ops && rd_next_ops->stat ? rd_next_ops->stat(path, st) : -ENOSYS;
  }
  if ((n = rd_lookup(path, NULL, NULL, NULL)) == NULL) return -ENOENT;
  rd_fill_stat(n, st);

  return 0;
}

static int rd_mkdir(const char *path, int mode) {
  rd_node_t *n, *parent;
  const char *last;
  int lastlen;

  if (!rd_is_mine(path)) {
    return rd_next_ops && rd_next_ops->mkdir ? rd_next_ops->mkdir(path, mode) : -ENOSYS;
  }
  if ((n = rd_lookup(path, &parent, &last, &lastlen)) != NULL) return -EEXIST;
  if (parent == NULL || lastlen == 0) return -ENOENT;
  if (rd_create(parent, last, lastlen, 1) == NULL) return -ENOSPC;

  return 0;
}

static int rd_rmdir(const char *path) {
  rd_node_t *n;

  if (!rd_is_mine(path)) {
    return rd_next_ops && rd_next_ops->rmdir ? rd_next_ops->rmdir(path) : -ENOSYS;
  }
  if ((n = rd_lookup(path, NULL, NULL, NULL)) == NULL) return -ENOENT;
  if (!n->isdir) return -ENOTDIR;
  if (n == &rd_root) return -EBUSY;
  if (n->child) return -ENOTEMPTY;
  rd_detach(n);
  if (n->open) {
    n->deleted = 1;
  } else {
    rd_free_node(n);
  }

  return 0;
}

static int rd_remove(const char *path) {
  rd_node_t *n;

  if (!rd_is_mine(path)) {
    return rd_next_ops && rd_next_ops->remove ? rd_next_ops->remove(path) : -ENOSYS;
  }
  if ((n = rd_lookup(path, NULL, NULL, NULL)) == NULL) return -ENOENT;
  if (n->isdir) return -EISDIR;
  rd_detach(n);
  if (n->open) {
    n->deleted = 1;
  } else {
    rd_free_node(n);
  }

  return 0;
}

static int rd_rename(const char *oldpath, const char *newpath) {
  rd_node_t *n, *t, *parent, *p;
  const char *last;
  int lastlen;
  char *name;

  if (!rd_is_mine(oldpath) && !rd_is_mine(newpath)) {
    return rd_next_ops && rd_next_ops->rename ? rd_next_ops->rename(oldpath, newpath) : -ENOSYS;
  }
  if (!rd_is_mine(oldpath) || !rd_is_mine(newpath)) return -EXDEV;

  if ((n = rd_lookup(oldpath, NULL, NULL, NULL)) == NULL) return -ENOENT;
  if (n == &rd_root) return -EBUSY;
  t = rd_lookup(newpath, &parent, &last, &lastlen);
  if (t == n) return 0;
  if (parent == NULL || lastlen == 0) return -ENOENT;
  // a directory cannot go inside itself
  for (p = parent; p; p = p->parent) {
    if (p == n) return -EINVAL;
  }
  if (t) {
    if (t->isdir != n->isdir) return t->isdir ? -EISDIR : -ENOTDIR;
    if (t->isdir && t->child) return -ENOTEMPTY;
  }
  if ((name = malloc(lastlen + 1)) == NULL) return -ENOMEM;
  memcpy(name, last, lastlen);
  name[lastlen] = 0;

  if (t) {
    rd_detach(t);
    if (t->open) {
      t->deleted = 1;
    } else {
      rd_free_node(t);
    }
  }
  rd_detach(n);
  rd_charge((int32_t)lastlen - (int32_t)strlen(n->name));
  free(n->name);
  n->name = name;
  n->parent = parent;
  n->next = parent->child;
  parent->child = n;
  parent->mtime = n->mtime = time(NULL);

  return 0;
}

static int rd_readlink(const char *path, char *buf, size_t bufsiz) {
  if (!rd_is_mine(path)) {
    return rd_next_ops && rd_next_ops->readlink ? rd_next_ops->readlink(path, buf, bufsiz) : -ENOSYS;
  }
  return -EINVAL;
}

static int rd_symlink(const char *target, const char *linkpath) {
  if (!rd_is_mine(linkpath)) {
    return rd_next_ops && rd_next_ops->symlink ? rd_next_ops->symlink(target, linkpath) : -ENOSYS;
  }
  return -ENOSYS;
}

// ftruncate() on a RAM disk descriptor (libpit's ftruncate, ps2sys.c, asks
// here first): 0 or -errno, 1 when the descriptor is not one of ours
int sys_ps2_ramdisk_ftruncate(void *fdinfo, int64_t length) {
  _libcglue_fdman_fd_info_t *info = (_libcglue_fdman_fd_info_t *)fdinfo;
  rd_file_t *f;

  if (info == NULL || info->ops != &rd_file_ops) return 1;
  f = (rd_file_t *)info->userdata;
  if (length < 0 || length > 0x7FFFFFFF) return -EINVAL;
  if ((f->flags & O_ACCMODE) == O_RDONLY) return -EBADF;

  return rd_set_size(f->node, (uint32_t)length);
}

int sys_ps2_ramdisk_mount(const char *name, uint32_t limit) {
  int len = name ? (int)strlen(name) : 0;

  if (rd_mounted) return 0;
  if (len == 0 || len > RD_MAX_DEV - 1) return -1;
  if (_libcglue_fdman_path_ops == NULL) {
    debug(DEBUG_ERROR, "SYS", "RAM disk: no file system glue to plug into");
    return -1;
  }

  memset(&rd_root, 0, sizeof(rd_root));
  rd_root.name = "";
  rd_root.isdir = 1;
  rd_root.mtime = time(NULL);
  rd_used = rd_peak = 0;
  rd_nfiles = 0;
  rd_limit = limit;
  snprintf(rd_dev, sizeof(rd_dev), "%s:", name);
  rd_devlen = len + 1;

  memset(&rd_file_ops, 0, sizeof(rd_file_ops));
  rd_file_ops.getfd = rd_getfd;
  rd_file_ops.getfilename = rd_getfilename;
  rd_file_ops.close = rd_close;
  rd_file_ops.read = rd_read;
  rd_file_ops.write = rd_write;
  rd_file_ops.lseek = rd_lseek;
  rd_file_ops.lseek64 = rd_lseek64;

  memset(&rd_dir_ops, 0, sizeof(rd_dir_ops));
  rd_dir_ops.getfd = rd_getfd;
  rd_dir_ops.getfilename = rd_getfilename;
  rd_dir_ops.close = rd_close;
  rd_dir_ops.dread = rd_dread;

  // plug in front of the current table (fileXio, set up by
  // init_ps2_filesystem_driver)
  rd_next_ops = _libcglue_fdman_path_ops;
  rd_path_ops.open = rd_open;
  rd_path_ops.remove = rd_remove;
  rd_path_ops.rename = rd_rename;
  rd_path_ops.mkdir = rd_mkdir;
  rd_path_ops.rmdir = rd_rmdir;
  rd_path_ops.stat = rd_stat;
  rd_path_ops.readlink = rd_readlink;
  rd_path_ops.symlink = rd_symlink;
  _libcglue_fdman_path_ops = &rd_path_ops;
  rd_mounted = 1;

  debug(DEBUG_INFO, "SYS", "RAM disk \"%s/\" mounted, up to %u KB of main memory", rd_dev, limit / 1024);
  return 0;
}

void sys_ps2_ramdisk_usage(uint32_t *used, uint32_t *peak, uint32_t *limit, uint32_t *files) {
  if (used) *used = rd_used;
  if (peak) *peak = rd_peak;
  if (limit) *limit = rd_limit;
  if (files) *files = rd_nfiles;
}
