// RAM disk in the auxiliary memory of the Nintendo GameCube (libogc).
//
// The console has 24MB of main memory, almost all of it taken by the program,
// and 16MB of auxiliary RAM (ARAM) that the CPU cannot address: it is only
// reachable through DMA, 32 bytes at a time at least. Nothing in PumpkinOS
// uses it except the AESND audio library (a few voice buffers at its bottom),
// so it makes a fine RAM disk: the home directory when the console has no SD
// card adapter (or, in an emulator, no storage at all), seeded from the boot
// medium or from the archive linked into the program. It does not survive a
// reboot.
//
// The file system is a tree of nodes in main memory (a node is ~50 bytes
// plus its name and block list), the file contents live in ARAM in 2KB
// blocks; every transfer goes through one 32 byte aligned bounce buffer
// with the ARQ request queue, which serializes them with AESND's. The
// device is registered with newlib ("<name>:/...", see sys_gc_ramdisk_mount)
// so that the standard calls (open/read/write/stat/opendir/...) work on it
// like on an SD card.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/iosupport.h>
#include <sys/reent.h>

#include <ogc/aram.h>
#include <ogc/arqueue.h>
#include <ogc/cache.h>

#include "sys.h"
#include "debug.h"

#define RD_BLOCK     2048
// left below the RAM disk for the AR_Alloc() users (AESND: 32 voice
// buffers of 2304 bytes), rounded up generously
#define RD_RESERVE   (512 * 1024)
// AR_Alloc() block tracking entries (AESND would ask for 32 of them)
#define RD_AR_BLOCKS 32
#define RD_MAX_NAME  255
#define RD_OWNER     0x50554D50  // 'PUMP'

typedef struct rd_node {
  char *name;
  struct rd_node *parent;
  struct rd_node *next;    // next entry in the parent directory
  struct rd_node *child;   // first entry (directories)
  int isdir;
  uint32_t size;
  uint32_t nblocks;        // blocks in use
  uint32_t cblocks;        // capacity of blocks[]
  uint16_t *blocks;
  time_t mtime;
  int open;                // open descriptors
  int deleted;             // unlinked while open: freed on the last close
} rd_node_t;

typedef struct {
  rd_node_t *node;
  uint32_t pos;
  int flags;
} rd_file_t;

typedef struct {
  rd_node_t *dir;
  rd_node_t *cur;
} rd_dir_t;

static rd_node_t rd_root;
static uint32_t rd_base;        // ARAM address of block 0
static uint32_t rd_nblocks;     // blocks in all
static uint32_t rd_free;        // free blocks
static uint32_t rd_hint;        // where the free block search starts
static uint8_t *rd_bitmap;
static uint32_t rd_arstack[RD_AR_BLOCKS];
static uint8_t rd_bounce[RD_BLOCK] __attribute__((aligned(32)));
static int rd_mounted = 0;
static char rd_devname[16];

// DMA between the bounce buffer and ARAM (whole blocks)
static void rd_dma(int dir, uint32_t block) {
  ARQRequest req;
  uint32_t aram = rd_base + block * RD_BLOCK;

  if (dir == ARQ_MRAMTOARAM) {
    DCFlushRange(rd_bounce, RD_BLOCK);
  } else {
    DCInvalidateRange(rd_bounce, RD_BLOCK);
  }
  ARQ_PostRequest(&req, RD_OWNER, dir, ARQ_PRIO_HI, aram, (uint32_t)rd_bounce, RD_BLOCK);
  if (dir == ARQ_ARAMTOMRAM) {
    DCInvalidateRange(rd_bounce, RD_BLOCK);
  }
}

static int rd_alloc_block(uint32_t *block) {
  uint32_t i, b;

  if (rd_free == 0) return -1;
  for (i = 0; i < rd_nblocks; i++) {
    b = (rd_hint + i) % rd_nblocks;
    if (!(rd_bitmap[b >> 3] & (1 << (b & 7)))) {
      rd_bitmap[b >> 3] |= (1 << (b & 7));
      rd_free--;
      rd_hint = b + 1;
      *block = b;
      return 0;
    }
  }

  return -1;
}

static void rd_free_block(uint32_t b) {
  if (b < rd_nblocks && (rd_bitmap[b >> 3] & (1 << (b & 7)))) {
    rd_bitmap[b >> 3] &= ~(1 << (b & 7));
    rd_free++;
  }
}

// grows the file to hold `size` bytes (new blocks are zeroed)
static int rd_grow(rd_node_t *node, uint32_t size) {
  uint32_t need, b;
  uint16_t *blocks;

  need = (size + RD_BLOCK - 1) / RD_BLOCK;
  if (need > node->cblocks) {
    uint32_t cap = node->cblocks ? node->cblocks : 4;
    while (cap < need) cap *= 2;
    if ((blocks = realloc(node->blocks, cap * sizeof(uint16_t))) == NULL) return -1;
    node->blocks = blocks;
    node->cblocks = cap;
  }
  while (node->nblocks < need) {
    if (rd_alloc_block(&b) != 0) return -1;
    node->blocks[node->nblocks++] = (uint16_t)b;
    memset(rd_bounce, 0, RD_BLOCK);
    rd_dma(ARQ_MRAMTOARAM, b);
  }

  return 0;
}

static void rd_shrink(rd_node_t *node, uint32_t size) {
  uint32_t need = (size + RD_BLOCK - 1) / RD_BLOCK;

  while (node->nblocks > need) {
    rd_free_block(node->blocks[--node->nblocks]);
  }
}

static void rd_free_node(rd_node_t *node) {
  rd_shrink(node, 0);
  free(node->blocks);
  free(node->name);
  free(node);
}

// the path without the "dev:" prefix
static const char *rd_strip(const char *path) {
  const char *p;

  for (p = path; *p && *p != '/'; p++) {
    if (*p == ':') return p + 1;
  }

  return path;
}

static rd_node_t *rd_find_child(rd_node_t *dir, const char *name, int len) {
  rd_node_t *n;

  for (n = dir->child; n; n = n->next) {
    if ((int)strlen(n->name) == len && !strncmp(n->name, name, len)) return n;
  }

  return NULL;
}

// resolves a path to a node; when `parent` is given, the last component
// may be missing: *parent is its directory and *last the component
static rd_node_t *rd_lookup(const char *path, rd_node_t **parent, const char **last, int *lastlen) {
  rd_node_t *dir = &rd_root, *n;
  const char *p, *q;
  int len;

  if (parent) *parent = NULL;
  p = rd_strip(path);

  for (;;) {
    while (*p == '/') p++;
    if (*p == 0) {
      if (parent) {
        *parent = dir->parent ? dir->parent : NULL;
        *last = dir->name ? dir->name : "";
        *lastlen = dir->name ? strlen(dir->name) : 0;
      }
      return dir;
    }
    for (q = p; *q && *q != '/'; q++);
    len = q - p;
    if (len == 1 && p[0] == '.') {
      p = q;
      continue;
    }
    if (len == 2 && p[0] == '.' && p[1] == '.') {
      if (dir->parent) dir = dir->parent;
      p = q;
      continue;
    }
    if (len > RD_MAX_NAME) return NULL;
    n = rd_find_child(dir, p, len);
    // skip the trailing slashes to know whether this is the last component
    while (*q == '/') q++;
    if (*q == 0) {
      if (parent) {
        *parent = dir;
        *last = p;
        *lastlen = len;
      }
      return n;
    }
    if (n == NULL || !n->isdir) return NULL;
    dir = n;
    p = q;
  }
}

static rd_node_t *rd_create(rd_node_t *dir, const char *name, int len, int isdir) {
  rd_node_t *n;

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

  return n;
}

static void rd_detach(rd_node_t *n) {
  rd_node_t **pp;

  for (pp = &n->parent->child; *pp; pp = &(*pp)->next) {
    if (*pp == n) {
      *pp = n->next;
      break;
    }
  }
  n->parent->mtime = time(NULL);
  n->next = NULL;
}

static void rd_fill_stat(rd_node_t *n, struct stat *st) {
  memset(st, 0, sizeof(struct stat));
  st->st_mode = n->isdir ? (S_IFDIR | 0777) : (S_IFREG | 0666);
  st->st_nlink = 1;
  st->st_size = n->size;
  st->st_blksize = RD_BLOCK;
  st->st_blocks = n->nblocks * (RD_BLOCK / 512);
  st->st_mtime = st->st_atime = st->st_ctime = n->mtime;
  st->st_ino = (ino_t)(uintptr_t)n;
}

// --- the devoptab operations

static int rd_open_r(struct _reent *r, void *fileStruct, const char *path, int flags, int mode) {
  rd_file_t *f = (rd_file_t *)fileStruct;
  rd_node_t *n, *dir;
  const char *last;
  int lastlen;

  n = rd_lookup(path, &dir, &last, &lastlen);
  if (n == NULL) {
    if (!(flags & O_CREAT) || dir == NULL || lastlen == 0) {
      r->_errno = dir == NULL ? ENOTDIR : ENOENT;
      return -1;
    }
    if ((n = rd_create(dir, last, lastlen, 0)) == NULL) {
      r->_errno = ENOMEM;
      return -1;
    }
  } else {
    if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
      r->_errno = EEXIST;
      return -1;
    }
    if (n->isdir) {
      r->_errno = EISDIR;
      return -1;
    }
    if ((flags & O_TRUNC) && (flags & O_ACCMODE) != O_RDONLY) {
      rd_shrink(n, 0);
      n->size = 0;
      n->mtime = time(NULL);
    }
  }

  f->node = n;
  f->pos = (flags & O_APPEND) ? n->size : 0;
  f->flags = flags;
  n->open++;

  return 0;
}

static int rd_close_r(struct _reent *r, void *fd) {
  rd_file_t *f = (rd_file_t *)fd;

  if (f->node) {
    f->node->open--;
    if (f->node->deleted && f->node->open == 0) {
      rd_free_node(f->node);
    }
    f->node = NULL;
  }

  return 0;
}

static ssize_t rd_write_r(struct _reent *r, void *fd, const char *ptr, size_t len) {
  rd_file_t *f = (rd_file_t *)fd;
  rd_node_t *n = f->node;
  uint32_t pos, end, b, off, chunk;
  size_t done = 0;

  if ((f->flags & O_ACCMODE) == O_RDONLY) {
    r->_errno = EBADF;
    return -1;
  }
  if (len == 0) return 0;
  if (f->flags & O_APPEND) f->pos = n->size;

  pos = f->pos;
  end = pos + len;
  if (end < pos) {
    r->_errno = EFBIG;
    return -1;
  }
  if (end > n->nblocks * RD_BLOCK) {
    if (rd_grow(n, end) != 0) {
      if (n->nblocks * RD_BLOCK <= pos) {
        r->_errno = ENOSPC;
        return -1;
      }
      // partial: write what fits
      end = n->nblocks * RD_BLOCK;
      len = end - pos;
    }
  }

  while (done < len) {
    b = n->blocks[pos / RD_BLOCK];
    off = pos % RD_BLOCK;
    chunk = RD_BLOCK - off;
    if (chunk > len - done) chunk = len - done;
    if (chunk < RD_BLOCK) {
      // partial block: read, modify, write
      rd_dma(ARQ_ARAMTOMRAM, b);
    }
    memcpy(rd_bounce + off, ptr + done, chunk);
    rd_dma(ARQ_MRAMTOARAM, b);
    pos += chunk;
    done += chunk;
  }

  f->pos = pos;
  if (pos > n->size) n->size = pos;
  n->mtime = time(NULL);

  return done;
}

static ssize_t rd_read_r(struct _reent *r, void *fd, char *ptr, size_t len) {
  rd_file_t *f = (rd_file_t *)fd;
  rd_node_t *n = f->node;
  uint32_t pos, b, off, chunk;
  size_t done = 0;

  if ((f->flags & O_ACCMODE) == O_WRONLY) {
    r->_errno = EBADF;
    return -1;
  }
  pos = f->pos;
  if (pos >= n->size) return 0;
  if (len > n->size - pos) len = n->size - pos;

  while (done < len) {
    b = n->blocks[pos / RD_BLOCK];
    off = pos % RD_BLOCK;
    chunk = RD_BLOCK - off;
    if (chunk > len - done) chunk = len - done;
    rd_dma(ARQ_ARAMTOMRAM, b);
    memcpy(ptr + done, rd_bounce + off, chunk);
    pos += chunk;
    done += chunk;
  }
  f->pos = pos;

  return done;
}

static off_t rd_seek_r(struct _reent *r, void *fd, off_t pos, int dir) {
  rd_file_t *f = (rd_file_t *)fd;
  int64_t np;

  switch (dir) {
    case SEEK_SET: np = pos; break;
    case SEEK_CUR: np = (int64_t)f->pos + pos; break;
    case SEEK_END: np = (int64_t)f->node->size + pos; break;
    default:
      r->_errno = EINVAL;
      return -1;
  }
  if (np < 0 || np > 0x7fffffff) {
    r->_errno = EINVAL;
    return -1;
  }
  f->pos = (uint32_t)np;

  return f->pos;
}

static int rd_fstat_r(struct _reent *r, void *fd, struct stat *st) {
  rd_file_t *f = (rd_file_t *)fd;

  rd_fill_stat(f->node, st);
  return 0;
}

static int rd_stat_r(struct _reent *r, const char *file, struct stat *st) {
  rd_node_t *n;

  if ((n = rd_lookup(file, NULL, NULL, NULL)) == NULL) {
    r->_errno = ENOENT;
    return -1;
  }
  rd_fill_stat(n, st);

  return 0;
}

static int rd_link_r(struct _reent *r, const char *existing, const char *newLink) {
  r->_errno = ENOSYS;
  return -1;
}

static int rd_unlink_r(struct _reent *r, const char *name) {
  rd_node_t *n;

  if ((n = rd_lookup(name, NULL, NULL, NULL)) == NULL) {
    r->_errno = ENOENT;
    return -1;
  }
  if (n->isdir) {
    r->_errno = EISDIR;
    return -1;
  }
  rd_detach(n);
  if (n->open > 0) {
    n->deleted = 1;
  } else {
    rd_free_node(n);
  }

  return 0;
}

static int rd_chdir_r(struct _reent *r, const char *name) {
  rd_node_t *n;

  if ((n = rd_lookup(name, NULL, NULL, NULL)) == NULL) {
    r->_errno = ENOENT;
    return -1;
  }
  if (!n->isdir) {
    r->_errno = ENOTDIR;
    return -1;
  }

  return 0;
}

static int rd_rename_r(struct _reent *r, const char *oldName, const char *newName) {
  rd_node_t *n, *t, *dir, *p;
  const char *last;
  int lastlen;
  char *name;

  if ((n = rd_lookup(oldName, NULL, NULL, NULL)) == NULL || n == &rd_root) {
    r->_errno = ENOENT;
    return -1;
  }
  t = rd_lookup(newName, &dir, &last, &lastlen);
  if (dir == NULL || lastlen == 0) {
    r->_errno = ENOENT;
    return -1;
  }
  // a directory cannot be moved into itself
  for (p = dir; p; p = p->parent) {
    if (p == n) {
      r->_errno = EINVAL;
      return -1;
    }
  }
  if (t != NULL && t != n) {
    if (t->isdir != n->isdir) {
      r->_errno = t->isdir ? EISDIR : ENOTDIR;
      return -1;
    }
    if (t->isdir && t->child) {
      r->_errno = ENOTEMPTY;
      return -1;
    }
    rd_detach(t);
    if (t->open > 0) t->deleted = 1; else rd_free_node(t);
  }
  if ((name = malloc(lastlen + 1)) == NULL) {
    r->_errno = ENOMEM;
    return -1;
  }
  memcpy(name, last, lastlen);
  name[lastlen] = 0;
  rd_detach(n);
  free(n->name);
  n->name = name;
  n->parent = dir;
  n->next = dir->child;
  dir->child = n;
  dir->mtime = time(NULL);

  return 0;
}

static int rd_mkdir_r(struct _reent *r, const char *path, int mode) {
  rd_node_t *n, *dir;
  const char *last;
  int lastlen;

  n = rd_lookup(path, &dir, &last, &lastlen);
  if (n != NULL) {
    r->_errno = EEXIST;
    return -1;
  }
  if (dir == NULL || lastlen == 0) {
    r->_errno = ENOENT;
    return -1;
  }
  if (rd_create(dir, last, lastlen, 1) == NULL) {
    r->_errno = ENOMEM;
    return -1;
  }

  return 0;
}

static int rd_rmdir_r(struct _reent *r, const char *name) {
  rd_node_t *n;

  if ((n = rd_lookup(name, NULL, NULL, NULL)) == NULL) {
    r->_errno = ENOENT;
    return -1;
  }
  if (!n->isdir) {
    r->_errno = ENOTDIR;
    return -1;
  }
  if (n == &rd_root) {
    r->_errno = EBUSY;
    return -1;
  }
  if (n->child) {
    r->_errno = ENOTEMPTY;
    return -1;
  }
  rd_detach(n);
  rd_free_node(n);

  return 0;
}

static DIR_ITER *rd_diropen_r(struct _reent *r, DIR_ITER *dirState, const char *path) {
  rd_dir_t *d = (rd_dir_t *)dirState->dirStruct;
  rd_node_t *n;

  if ((n = rd_lookup(path, NULL, NULL, NULL)) == NULL) {
    r->_errno = ENOENT;
    return NULL;
  }
  if (!n->isdir) {
    r->_errno = ENOTDIR;
    return NULL;
  }
  d->dir = n;
  d->cur = n->child;
  n->open++;

  return dirState;
}

static int rd_dirreset_r(struct _reent *r, DIR_ITER *dirState) {
  rd_dir_t *d = (rd_dir_t *)dirState->dirStruct;

  d->cur = d->dir->child;
  return 0;
}

static int rd_dirnext_r(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat) {
  rd_dir_t *d = (rd_dir_t *)dirState->dirStruct;
  rd_node_t *n;

  if ((n = d->cur) == NULL) {
    r->_errno = ENOENT;
    return -1;
  }
  strncpy(filename, n->name, RD_MAX_NAME);
  filename[RD_MAX_NAME] = 0;
  if (filestat) rd_fill_stat(n, filestat);
  d->cur = n->next;

  return 0;
}

static int rd_dirclose_r(struct _reent *r, DIR_ITER *dirState) {
  rd_dir_t *d = (rd_dir_t *)dirState->dirStruct;

  if (d->dir) {
    d->dir->open--;
    d->dir = NULL;
  }

  return 0;
}

static int rd_statvfs_r(struct _reent *r, const char *path, struct statvfs *buf) {
  memset(buf, 0, sizeof(struct statvfs));
  buf->f_bsize = RD_BLOCK;
  buf->f_frsize = RD_BLOCK;
  buf->f_blocks = rd_nblocks;
  buf->f_bfree = rd_free;
  buf->f_bavail = rd_free;
  buf->f_namemax = RD_MAX_NAME;

  return 0;
}

static int rd_ftruncate_r(struct _reent *r, void *fd, off_t len) {
  rd_file_t *f = (rd_file_t *)fd;
  rd_node_t *n = f->node;

  if ((f->flags & O_ACCMODE) == O_RDONLY) {
    r->_errno = EBADF;
    return -1;
  }
  if (len < 0) {
    r->_errno = EINVAL;
    return -1;
  }
  if ((uint32_t)len > n->size) {
    uint32_t pos = n->size, end = (uint32_t)len, b, off, chunk;
    if (rd_grow(n, end) != 0) {
      r->_errno = ENOSPC;
      return -1;
    }
    // the tail of the former last block may hold stale data: zero it
    if (pos % RD_BLOCK) {
      b = n->blocks[pos / RD_BLOCK];
      off = pos % RD_BLOCK;
      chunk = RD_BLOCK - off;
      rd_dma(ARQ_ARAMTOMRAM, b);
      memset(rd_bounce + off, 0, chunk);
      rd_dma(ARQ_MRAMTOARAM, b);
    }
  } else {
    rd_shrink(n, (uint32_t)len);
  }
  n->size = (uint32_t)len;
  n->mtime = time(NULL);

  return 0;
}

static int rd_fsync_r(struct _reent *r, void *fd) {
  return 0;
}

static int rd_chmod_r(struct _reent *r, const char *path, mode_t mode) {
  return 0;
}

static int rd_fchmod_r(struct _reent *r, void *fd, mode_t mode) {
  return 0;
}

static int rd_utimes_r(struct _reent *r, const char *filename, const struct timeval times[2]) {
  rd_node_t *n;

  if ((n = rd_lookup(filename, NULL, NULL, NULL)) == NULL) {
    r->_errno = ENOENT;
    return -1;
  }
  n->mtime = times ? times[1].tv_sec : time(NULL);

  return 0;
}

static devoptab_t rd_devoptab = {
  .name = rd_devname,
  .structSize = sizeof(rd_file_t),
  .open_r = rd_open_r,
  .close_r = rd_close_r,
  .write_r = rd_write_r,
  .read_r = rd_read_r,
  .seek_r = rd_seek_r,
  .fstat_r = rd_fstat_r,
  .stat_r = rd_stat_r,
  .link_r = rd_link_r,
  .unlink_r = rd_unlink_r,
  .chdir_r = rd_chdir_r,
  .rename_r = rd_rename_r,
  .mkdir_r = rd_mkdir_r,
  .dirStateSize = sizeof(rd_dir_t),
  .diropen_r = rd_diropen_r,
  .dirreset_r = rd_dirreset_r,
  .dirnext_r = rd_dirnext_r,
  .dirclose_r = rd_dirclose_r,
  .statvfs_r = rd_statvfs_r,
  .ftruncate_r = rd_ftruncate_r,
  .fsync_r = rd_fsync_r,
  .deviceData = NULL,
  .chmod_r = rd_chmod_r,
  .fchmod_r = rd_fchmod_r,
  .rmdir_r = rd_rmdir_r,
  .lstat_r = rd_stat_r,
  .utimes_r = rd_utimes_r,
};

int sys_gc_ramdisk_mount(const char *name) {
  uint32_t base, size;

  if (rd_mounted) return 0;

  // AR_Init() is a no-op when already done (AESND, when the audio device was
  // opened first); the block tracking array given here serves whoever calls
  // AR_Alloc() afterwards (AESND: 32 voice buffers)
  base = AR_Init(rd_arstack, RD_AR_BLOCKS);
  size = AR_GetSize();
  ARQ_Init();
  if (size == 0 || size <= base + RD_RESERVE + RD_BLOCK) {
    debug(DEBUG_ERROR, "SYS", "ARAM: %u KB at 0x%08X, too small for a RAM disk", size / 1024, base);
    return -1;
  }

  // the RAM disk starts above the reserved area, 32 byte aligned
  rd_base = (base + RD_RESERVE + 31) & ~31;
  rd_nblocks = (size - rd_base) / RD_BLOCK;
  if (rd_nblocks > 65535) rd_nblocks = 65535;  // (block indices are 16 bits)
  if ((rd_bitmap = calloc(1, (rd_nblocks + 7) / 8)) == NULL) {
    return -1;
  }
  rd_free = rd_nblocks;
  rd_hint = 0;

  memset(&rd_root, 0, sizeof(rd_root));
  rd_root.isdir = 1;
  rd_root.mtime = time(NULL);

  strncpy(rd_devname, name, sizeof(rd_devname) - 1);
  if (AddDevice(&rd_devoptab) < 0) {
    debug(DEBUG_ERROR, "SYS", "ARAM RAM disk: AddDevice(\"%s\") failed", name);
    free(rd_bitmap);
    rd_bitmap = NULL;
    return -1;
  }
  rd_mounted = 1;
  debug(DEBUG_INFO, "SYS", "ARAM RAM disk \"%s:/\" mounted: %u KB in %u blocks of %d bytes (ARAM 0x%08X-0x%08X)", name, (rd_nblocks * RD_BLOCK) / 1024, rd_nblocks, RD_BLOCK, rd_base, rd_base + rd_nblocks * RD_BLOCK);

  return 0;
}
