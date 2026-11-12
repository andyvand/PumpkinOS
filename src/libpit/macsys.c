// Classic Mac OS (Retro68) POSIX layer: files and directories on top of the
// File Manager, clocks on top of the Time Manager, and the descriptor table
// shared with the MacTCP sockets of macnet.c. See macsys.h for the path
// conventions.
//
// Every file is resolved to (volume, parent directory ID, HFS name) one
// component at a time with PBGetCatInfo, so there is no limit on the length
// of a path (a full HFS path string is limited to 255 characters) and a
// file in a deep folder costs one catalog lookup per folder (the last
// directory resolved is cached).
//
// A descriptor keeps its own position and passes it with every read and
// write (fsFromStart), so several descriptors can share one File Manager
// access path: HFS refuses a second read/write path to a file (opWrErr)
// and hands back the existing one instead, which PumpkinOS needs when it
// opens a database file twice. A file deleted while open (fBsyErr) is
// deleted when its last descriptor is closed.

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>
#include <dirent.h>

#include <MacTypes.h>
#include <Files.h>
#include <Processes.h>
#include <MacMemory.h>
#include <Events.h>
#include <OSUtils.h>

#include "macsys.h"

// (libpit/thread.h needs sys.h, whose names clash with nothing here, but
// only this is used)
extern void thread_yield(int waiting);

// macsys.h maps the newlib names to the mac_ functions for sys.c: not here
#undef open
#undef close
#undef read
#undef write
#undef lseek
#undef stat
#undef lstat
#undef fstat
#undef mkdir
#undef unlink
#undef remove
#undef rename
#undef mkstemp
#undef isatty
#undef fcntl
#undef gettimeofday
#undef select

// the Time Manager counter (Multiversal does not declare Microseconds())
typedef struct {
  uint32_t hi, lo;
} mac_wide_t;

#ifdef __m68k__
// _Microseconds; MOVEA.L (SP)+,A1; MOVE.L A0,(A1)+; MOVE.L D0,(A1)
static pascal void MacMicroseconds(mac_wide_t *w) = { 0xA193, 0x225F, 0x22C8, 0x2280 };
#else
extern void Microseconds(mac_wide_t *w);
#define MacMicroseconds Microseconds
#endif

// (missing from the Multiversal interfaces)
#ifndef fsRtParID
#define fsRtParID 1
#endif
#ifndef fsRtDirID
#define fsRtDirID 2
#endif
#ifndef afpAccessDenied
#define afpAccessDenied -5000
#endif

// seconds from 1904-01-01 (the Mac epoch) to 1970-01-01
#define MAC_EPOCH_DELTA 2082844800UL

#define MAC_MAX_FD   FD_SETSIZE
#define MAC_FIRST_FD 3

#define FD_FREE   0
#define FD_FILE   1
#define FD_SOCKET 2

#define FILE_CREATOR 'PmOS'
#define FILE_TYPE    '\?\?\?\?'
#define TEXT_TYPE    'TEXT'

typedef struct {
  int type;
  int flags;          // O_ACCMODE | O_APPEND | O_NONBLOCK
  int16_t refNum;     // File Manager access path (shared, see above)
  int16_t vRefNum;
  int32_t parID;
  unsigned char name[32];
  int32_t pos;
  int unlink_on_close;
} mac_fd_t;

struct mac_dir_t {
  struct dirent ent;
  char *names;        // the names, one after the other, zero terminated
  unsigned char *isdir;
  int count, index, size;
};

static mac_fd_t fds[MAC_MAX_FD];
static int fs_inited;

// the current directory
static int16_t cwd_vref;
static int32_t cwd_dirid;

// the application's folder
static int16_t app_vref;
static int32_t app_dirid;
static char app_dir[256];

// last directory resolved: "a/b/c" (the directory part of a path) -> dirID
static char cache_path[256];
static int16_t cache_vref;
static int32_t cache_dirid;
static int16_t cache_cwd_vref;
static int32_t cache_cwd_dirid;

// wall clock at startup (Unix seconds) and the Microseconds() at that time
static uint32_t base_secs;
static uint64_t base_us;

static int mac_errno(OSErr err) {
  switch (err) {
    case noErr:    return 0;
    case fnfErr:
    case dirNFErr:
    case nsvErr:   return ENOENT;
    case dupFNErr: return EEXIST;
    case fBsyErr:  return EBUSY;
    case opWrErr:
    case permErr:
    case fLckdErr:
    case afpAccessDenied: return EACCES;
    case wPrErr:
    case vLckdErr: return EROFS;
    case dskFulErr: return ENOSPC;
    case tmfoErr:  return EMFILE;
    case bdNamErr: return EINVAL;
    case memFullErr: return ENOMEM;
    default:       return EIO;
  }
}

static int fail(OSErr err) {
  errno = mac_errno(err);
  return -1;
}

uint64_t mac_microseconds(void) {
  mac_wide_t w;

  MacMicroseconds(&w);
  return ((uint64_t)w.hi << 32) | w.lo;
}

void mac_yield(uint32_t ticks) {
  EventRecord ev;

  // no event classes: nothing is taken from the queue, the other
  // applications (and the Finder) get their turn
  WaitNextEvent(0, &ev, ticks, NULL);
}

uint32_t sys_mac_heap_avail(void) {
  return (uint32_t)MaxBlock();
}

// Thread stacks. The Toolbox measures the free stack as the distance from
// the stack pointer down to the heap (StackSpace(), used by QuickDraw), and
// the stack sniffer stops a program whose stack pointer is below the top
// of the heap: a coroutine stack allocated in the heap would look like a
// stack overflow. mac_stacks_reserve() moves the application limit down
// (before MaxApplZone) so that the stack area holds a number of slots for
// the PumpkinOS threads below the main stack.
#define MAC_MAX_STACK_SLOTS 32

// (GetApplLimit() is glue on 68K: the low memory global itself)
#ifdef __m68k__
#define MAC_APPL_LIMIT() LMGetApplLimit()
#else
#define MAC_APPL_LIMIT() GetApplLimit()
#endif

static uint8_t *stack_area;
static uint32_t stack_slot_size;
static int stack_slots;
static uint8_t stack_used[MAC_MAX_STACK_SLOTS];

void mac_stacks_reserve(uint32_t slot_size, int slots, uint32_t main_stack) {
  Ptr limit;

  if (slots > MAC_MAX_STACK_SLOTS) slots = MAC_MAX_STACK_SLOTS;
  slot_size = (slot_size + 15) & ~15;
  limit = MAC_APPL_LIMIT();
  SetApplLimit(limit - (Size)slot_size * slots - (Size)main_stack);
  if (MemError() != noErr) return;
  stack_area = (uint8_t *)((uintptr_t)(MAC_APPL_LIMIT() + 15) & ~(uintptr_t)15);
  stack_slot_size = slot_size;
  stack_slots = slots;
}

void *mac_stack_alloc(uint32_t size) {
  int i;

  if (stack_area && size <= stack_slot_size) {
    for (i = 0; i < stack_slots; i++) {
      if (!stack_used[i]) {
        stack_used[i] = 1;
        return stack_area + (uint32_t)i * stack_slot_size;
      }
    }
  }

  // no slot left: a heap stack (works, but the Toolbox may think it is
  // out of stack space while that thread runs)
  return malloc(size);
}

void mac_stack_free(void *p) {
  uint8_t *q = (uint8_t *)p;
  int i;

  if (stack_area && q >= stack_area && q < stack_area + (uint32_t)stack_slots * stack_slot_size) {
    i = (int)((q - stack_area) / stack_slot_size);
    stack_used[i] = 0;
  } else {
    free(p);
  }
}

// "name" (C string, POSIX) -> Pascal string (HFS); ':' and '/' swap
static int to_hfs_name(const char *s, int len, unsigned char *p) {
  int i;

  if (len <= 0 || len > 31) {
    errno = len <= 0 ? ENOENT : ENAMETOOLONG;
    return -1;
  }
  p[0] = len;
  for (i = 0; i < len; i++) {
    p[i+1] = s[i] == ':' ? '/' : s[i];
  }

  return 0;
}

static void from_hfs_name(const unsigned char *p, char *s, int max) {
  int i, n = p[0];

  if (n > max-1) n = max-1;
  for (i = 0; i < n; i++) {
    s[i] = p[i+1] == '/' ? ':' : p[i+1];
  }
  s[n] = 0;
}

// catalog information of name in dirID (name NULL or empty: of the
// directory dirID itself)
static OSErr get_cat_info(int16_t vref, int32_t dirid, const unsigned char *name, CInfoPBRec *pb, unsigned char *namebuf) {
  memset(pb, 0, sizeof(CInfoPBRec));
  if (name && name[0]) {
    memcpy(namebuf, name, name[0] + 1);
    pb->hFileInfo.ioFDirIndex = 0;
  } else {
    namebuf[0] = 0;
    pb->hFileInfo.ioFDirIndex = -1;
  }
  pb->hFileInfo.ioNamePtr = namebuf;
  pb->hFileInfo.ioVRefNum = vref;
  pb->hFileInfo.ioDirID = dirid;

  return PBGetCatInfoSync(pb);
}

// the volume reference of a volume name (C string)
static int volume_ref(const char *name, int len, int16_t *vref) {
  HParamBlockRec pb;
  unsigned char vname[64];
  OSErr err;

  if (len <= 0 || len > 27) {
    errno = ENOENT;
    return -1;
  }
  vname[0] = len + 1;
  memcpy(&vname[1], name, len);
  vname[len+1] = ':';

  memset(&pb, 0, sizeof(pb));
  pb.volumeParam.ioNamePtr = vname;
  pb.volumeParam.ioVolIndex = -1;
  if ((err = PBHGetVInfoSync(&pb)) != noErr) return fail(err);
  *vref = pb.volumeParam.ioVRefNum;

  return 0;
}

// walks the directories of path (all components but the last); on success
// *vref / *dirid is the directory holding the last component, *last points
// to it (empty when the path ends with '/', ".", ".."; then the target is
// the directory itself). *root is set for "/" (the list of volumes).
static int resolve_dir(const char *path, int16_t *vref, int32_t *dirid, const char **last, int *root) {
  CInfoPBRec pb;
  unsigned char hname[32], namebuf[64];
  const char *p, *s, *end;
  int16_t v;
  int32_t d;
  int len, dirlen;
  OSErr err;

  *root = 0;
  if (!fs_inited) mac_fs_init();

  if (path == NULL || path[0] == 0) {
    errno = ENOENT;
    return -1;
  }

  // the directory part, for the cache
  end = strrchr(path, '/');
  dirlen = end ? (int)(end - path) : 0;
  if (dirlen > 0 && dirlen < (int)sizeof(cache_path) && cache_path[0] &&
      cache_cwd_vref == cwd_vref && cache_cwd_dirid == cwd_dirid &&
      (int)strlen(cache_path) == dirlen && !strncmp(cache_path, path, dirlen)) {
    *vref = cache_vref;
    *dirid = cache_dirid;
    *last = end + 1;
    return 0;
  }

  p = path;
  if (*p == '/') {
    while (*p == '/') p++;
    if (*p == 0) {
      *root = 1;
      *last = p;
      return 0;
    }
    // the volume
    for (s = p; *s && *s != '/'; s++);
    if (volume_ref(p, (int)(s - p), &v) != 0) return -1;
    d = fsRtDirID;
    p = s;
    while (*p == '/') p++;
    if (*p == 0) {
      // "/Volume" or "/Volume/": the root directory itself
      *vref = v;
      *dirid = d;
      *last = p;
      return 0;
    }
  } else {
    v = cwd_vref;
    d = cwd_dirid;
  }

  for (;;) {
    for (s = p; *s && *s != '/'; s++);
    if (*s == 0) break;   // p is the last component
    len = (int)(s - p);
    if (len == 0 || (len == 1 && p[0] == '.')) {
      // "//" or "/./": nothing
    } else if (len == 2 && p[0] == '.' && p[1] == '.') {
      if (d != fsRtDirID) {
        if ((err = get_cat_info(v, d, NULL, &pb, namebuf)) != noErr) return fail(err);
        d = pb.dirInfo.ioDrParID;
      }
    } else {
      if (to_hfs_name(p, len, hname) != 0) return -1;
      if ((err = get_cat_info(v, d, hname, &pb, namebuf)) != noErr) return fail(err);
      if (!(pb.hFileInfo.ioFlAttrib & ioDirMask)) {
        errno = ENOTDIR;
        return -1;
      }
      d = pb.dirInfo.ioDrDirID;
    }
    p = s + 1;
  }

  if (dirlen > 0 && dirlen < (int)sizeof(cache_path)) {
    memcpy(cache_path, path, dirlen);
    cache_path[dirlen] = 0;
    cache_vref = v;
    cache_dirid = d;
    cache_cwd_vref = cwd_vref;
    cache_cwd_dirid = cwd_dirid;
  }

  *vref = v;
  *dirid = d;
  *last = p;
  return 0;
}

// a path -> (volume, parent directory, HFS name); a path naming a directory
// by "/", "." or ".." is turned into its parent and its name as well, so
// that every object has the same representation. Returns 1 for "/" (the
// list of volumes), 0 for an object, -1 on error.
static int resolve(const char *path, int16_t *vref, int32_t *parid, unsigned char *hname) {
  CInfoPBRec pb;
  unsigned char namebuf[64];
  const char *last;
  int16_t v;
  int32_t d;
  int root, len;
  OSErr err;

  if (resolve_dir(path, &v, &d, &last, &root) != 0) return -1;
  if (root) return 1;

  len = strlen(last);
  if (len > 0 && last[len-1] == '/') len--;

  if (len == 0 || (len == 1 && last[0] == '.') || (len == 2 && last[0] == '.' && last[1] == '.')) {
    if (len == 2 && d != fsRtDirID) {
      if ((err = get_cat_info(v, d, NULL, &pb, namebuf)) != noErr) return fail(err);
      d = pb.dirInfo.ioDrParID;
    }
    // the directory d itself: its name and parent
    if ((err = get_cat_info(v, d, NULL, &pb, namebuf)) != noErr) return fail(err);
    *vref = v;
    if (d == fsRtDirID) {
      // the root directory: parent fsRtParID, named after the volume
      *parid = fsRtParID;
    } else {
      *parid = pb.dirInfo.ioDrParID;
    }
    memcpy(hname, namebuf, namebuf[0] + 1);
    return 0;
  }

  if (to_hfs_name(last, len, hname) != 0) return -1;
  *vref = v;
  *parid = d;
  return 0;
}

// the POSIX path of directory (vref, dirid), with a trailing slash
static int dir_path(int16_t vref, int32_t dirid, char *buf, int len) {
  CInfoPBRec pb;
  unsigned char namebuf[64];
  char comp[64], tmp[512];
  int32_t d = dirid;
  int n, m;
  OSErr err;

  tmp[0] = 0;
  for (;;) {
    if ((err = get_cat_info(vref, d, NULL, &pb, namebuf)) != noErr) return fail(err);
    from_hfs_name(namebuf, comp, sizeof(comp));
    n = strlen(comp);
    m = strlen(tmp);
    if (n + 1 + m + 1 >= (int)sizeof(tmp)) {
      errno = ENAMETOOLONG;
      return -1;
    }
    memmove(tmp + n + 1, tmp, m + 1);
    memcpy(tmp, comp, n);
    tmp[n] = '/';
    if (d == fsRtDirID) break;
    d = pb.dirInfo.ioDrParID;
  }

  if ((int)strlen(tmp) + 2 > len) {
    errno = ERANGE;
    return -1;
  }
  buf[0] = '/';
  strcpy(buf + 1, tmp);
  return 0;
}

void mac_fs_init(void) {
  ProcessSerialNumber psn;
  ProcessInfoRec info;
  FSSpec spec;
  unsigned long secs;

  if (fs_inited) return;
  fs_inited = 1;

  memset(fds, 0, sizeof(fds));

  // the folder holding the application
  memset(&info, 0, sizeof(info));
  memset(&spec, 0, sizeof(spec));
  info.processInfoLength = sizeof(info);
  info.processAppSpec = &spec;
  psn.highLongOfPSN = 0;
  psn.lowLongOfPSN = kCurrentProcess;
  if (GetProcessInformation(&psn, &info) == noErr) {
    app_vref = spec.vRefNum;
    app_dirid = spec.parID;
  } else {
    // the default directory (normally the application's folder as well)
    WDPBRec wd;
    memset(&wd, 0, sizeof(wd));
    PBHGetVolSync(&wd);
    app_vref = wd.ioWDVRefNum;
    app_dirid = wd.ioWDDirID;
  }
  cwd_vref = app_vref;
  cwd_dirid = app_dirid;
  if (dir_path(app_vref, app_dirid, app_dir, sizeof(app_dir)) != 0) {
    strcpy(app_dir, "./");
  }

  GetDateTime(&secs);
  base_secs = secs > MAC_EPOCH_DELTA ? (uint32_t)(secs - MAC_EPOCH_DELTA) : 0;
  base_us = mac_microseconds();
}

const char *mac_app_dir(void) {
  if (!fs_inited) mac_fs_init();
  return app_dir;
}

int mac_hfs_path(const char *path, char *hfs, int len) {
  int16_t vref;
  int32_t parid, d;
  unsigned char hname[32], namebuf[64];
  char comp[64], tmp[512], name[64];
  CInfoPBRec pb;
  int n, m, r;
  OSErr err;

  if ((r = resolve(path, &vref, &parid, hname)) != 0) return -1;

  // the name, then every directory up to the volume
  from_hfs_name(hname, name, sizeof(name));
  strcpy(tmp, (char *)hname + 1);
  tmp[hname[0]] = 0;
  d = parid;
  while (d != fsRtParID) {
    if ((err = get_cat_info(vref, d, NULL, &pb, namebuf)) != noErr) return fail(err);
    memcpy(comp, namebuf + 1, namebuf[0]);
    comp[namebuf[0]] = 0;
    n = strlen(comp);
    m = strlen(tmp);
    if (n + 1 + m + 1 >= (int)sizeof(tmp)) {
      errno = ENAMETOOLONG;
      return -1;
    }
    memmove(tmp + n + 1, tmp, m + 1);
    memcpy(tmp, comp, n);
    tmp[n] = ':';
    if (d == fsRtDirID) break;
    d = pb.dirInfo.ioDrParID;
  }

  if ((int)strlen(tmp) + 1 > len) {
    errno = ENAMETOOLONG;
    return -1;
  }
  strcpy(hfs, tmp);
  return 0;
}

static mac_fd_t *get_fd(int fd, int type) {
  if (fd < 0 || fd >= MAC_MAX_FD || fds[fd].type == FD_FREE || (type && fds[fd].type != type)) {
    errno = EBADF;
    return NULL;
  }
  return &fds[fd];
}

static int alloc_fd(int type) {
  int i;

  for (i = MAC_FIRST_FD; i < MAC_MAX_FD; i++) {
    if (fds[i].type == FD_FREE) {
      memset(&fds[i], 0, sizeof(mac_fd_t));
      fds[i].type = type;
      return i;
    }
  }

  errno = EMFILE;
  return -1;
}

// for macnet.c
int mac_fd_alloc_socket(void) {
  if (!fs_inited) mac_fs_init();
  return alloc_fd(FD_SOCKET);
}

void mac_fd_release(int fd) {
  if (fd >= 0 && fd < MAC_MAX_FD) fds[fd].type = FD_FREE;
}

int mac_fd_is_socket(int fd) {
  return fd >= 0 && fd < MAC_MAX_FD && fds[fd].type == FD_SOCKET;
}

static int same_file(mac_fd_t *f, int16_t vref, int32_t parid, const unsigned char *name) {
  return f->type == FD_FILE && f->vRefNum == vref && f->parID == parid && EqualString(f->name, name, false, true);
}

static OSType file_type(const unsigned char *name) {
  int n = name[0];

  if (n > 4 && name[n-3] == '.' && ((name[n-2] == 't' && name[n-1] == 'x' && name[n] == 't') ||
                                    (name[n-2] == 'l' && name[n-1] == 'o' && name[n] == 'g') ||
                                    (name[n-2] == 'c' && name[n-1] == 'f' && name[n] == 'g') ||
                                    (name[n-2] == 'p' && name[n-1] == 'e' && name[n] == 'm'))) {
    return TEXT_TYPE;
  }
  if (n > 4 && name[n-3] == '.' && name[n-2] == 'e' && name[n-1] == 'r' && name[n] == 'r') {
    return TEXT_TYPE;
  }

  return FILE_TYPE;
}

int mac_open(const char *path, int flags, ...) {
  HParamBlockRec pb;
  unsigned char hname[32];
  int16_t vref, refnum;
  int32_t parid;
  int fd, i, acc, r;
  OSErr err;

  if ((r = resolve(path, &vref, &parid, hname)) != 0) {
    if (r == 1) errno = EISDIR;
    return -1;
  }

  acc = flags & O_ACCMODE;

  if (flags & O_CREAT) {
    err = HCreate(vref, parid, hname, FILE_CREATOR, file_type(hname));
    if (err == dupFNErr) {
      if (flags & O_EXCL) return fail(err);
    } else if (err != noErr) {
      return fail(err);
    }
  }

  memset(&pb, 0, sizeof(pb));
  pb.ioParam.ioNamePtr = hname;
  pb.ioParam.ioVRefNum = vref;
  pb.fileParam.ioDirID = parid;
  pb.ioParam.ioPermssn = acc == O_RDONLY ? fsRdPerm : fsRdWrPerm;
  err = PBHOpenDFSync(&pb);
  refnum = pb.ioParam.ioRefNum;

  if (err == opWrErr) {
    // already open for writing (by us): share that access path
    for (i = MAC_FIRST_FD; i < MAC_MAX_FD; i++) {
      if (same_file(&fds[i], vref, parid, hname)) break;
    }
    if (i == MAC_MAX_FD) return fail(err);
    refnum = fds[i].refNum;
  } else if (err == fnfErr && acc == O_RDONLY) {
    return fail(err);
  } else if (err != noErr) {
    // a folder?
    CInfoPBRec cpb;
    unsigned char namebuf[64];
    if (get_cat_info(vref, parid, hname, &cpb, namebuf) == noErr && (cpb.hFileInfo.ioFlAttrib & ioDirMask)) {
      errno = EISDIR;
      return -1;
    }
    return fail(err);
  }

  if ((fd = alloc_fd(FD_FILE)) == -1) {
    if (err == noErr) FSClose(refnum);
    return -1;
  }

  fds[fd].flags = flags & (O_ACCMODE | O_APPEND | O_NONBLOCK);
  fds[fd].refNum = refnum;
  fds[fd].vRefNum = vref;
  fds[fd].parID = parid;
  memcpy(fds[fd].name, hname, hname[0] + 1);
  fds[fd].pos = 0;

  if ((flags & O_TRUNC) && acc != O_RDONLY) {
    SetEOF(refnum, 0);
  }

  return fd;
}

static int refnum_users(int16_t refnum) {
  int i, n = 0;

  for (i = MAC_FIRST_FD; i < MAC_MAX_FD; i++) {
    if (fds[i].type == FD_FILE && fds[i].refNum == refnum) n++;
  }

  return n;
}

int mac_close(int fd) {
  mac_fd_t *f;
  unsigned char name[32];
  int16_t vref;
  int32_t parid;
  int del;

  if (fd >= 0 && fd < MAC_FIRST_FD) return 0;
  if ((f = get_fd(fd, 0)) == NULL) return -1;

  if (f->type == FD_SOCKET) {
    return mac_sock_close(fd);
  }

  del = f->unlink_on_close;
  vref = f->vRefNum;
  parid = f->parID;
  memcpy(name, f->name, f->name[0] + 1);
  f->type = FD_FREE;

  if (refnum_users(f->refNum) == 0) {
    FSClose(f->refNum);
    FlushVol(NULL, vref);
    if (del) HDelete(vref, parid, name);
  } else if (del) {
    // the file is still open through another descriptor: pass the
    // pending delete on
    int i;
    for (i = MAC_FIRST_FD; i < MAC_MAX_FD; i++) {
      if (fds[i].type == FD_FILE && fds[i].refNum == f->refNum) fds[i].unlink_on_close = 1;
    }
  }

  return 0;
}

ssize_t mac_read(int fd, void *buf, size_t len) {
  ParamBlockRec pb;
  mac_fd_t *f;
  OSErr err;

  if (fd >= 0 && fd < MAC_FIRST_FD) return 0;
  if ((f = get_fd(fd, 0)) == NULL) return -1;
  if (f->type == FD_SOCKET) return mac_sock_read(fd, buf, len);
  if (len == 0) return 0;

  memset(&pb, 0, sizeof(pb));
  pb.ioParam.ioRefNum = f->refNum;
  pb.ioParam.ioBuffer = buf;
  pb.ioParam.ioReqCount = len;
  pb.ioParam.ioPosMode = fsFromStart;
  pb.ioParam.ioPosOffset = f->pos;
  err = PBReadSync(&pb);
  if (err != noErr && err != eofErr) return fail(err);
  f->pos += pb.ioParam.ioActCount;

  return pb.ioParam.ioActCount;
}

ssize_t mac_write(int fd, const void *buf, size_t len) {
  ParamBlockRec pb;
  mac_fd_t *f;
  long eof;
  OSErr err;

  if (fd >= 0 && fd < MAC_FIRST_FD) return len;
  if ((f = get_fd(fd, 0)) == NULL) return -1;
  if (f->type == FD_SOCKET) return mac_sock_write(fd, buf, len);
  if ((f->flags & O_ACCMODE) == O_RDONLY) {
    errno = EBADF;
    return -1;
  }
  if (len == 0) return 0;

  if (f->flags & O_APPEND) {
    if (GetEOF(f->refNum, &eof) == noErr) f->pos = eof;
  }

  memset(&pb, 0, sizeof(pb));
  pb.ioParam.ioRefNum = f->refNum;
  pb.ioParam.ioBuffer = (Ptr)buf;
  pb.ioParam.ioReqCount = len;
  pb.ioParam.ioPosMode = fsFromStart;
  pb.ioParam.ioPosOffset = f->pos;
  err = PBWriteSync(&pb);
  if (err != noErr && pb.ioParam.ioActCount == 0) return fail(err);
  f->pos += pb.ioParam.ioActCount;

  return pb.ioParam.ioActCount;
}

off_t mac_lseek(int fd, off_t offset, int whence) {
  mac_fd_t *f;
  long eof;
  OSErr err;
  off_t pos;

  if ((f = get_fd(fd, FD_FILE)) == NULL) {
    if (mac_fd_is_socket(fd)) errno = ESPIPE;
    return -1;
  }

  switch (whence) {
    case SEEK_SET:
      pos = offset;
      break;
    case SEEK_CUR:
      pos = f->pos + offset;
      break;
    case SEEK_END:
      if ((err = GetEOF(f->refNum, &eof)) != noErr) return fail(err);
      pos = eof + offset;
      break;
    default:
      errno = EINVAL;
      return -1;
  }

  if (pos < 0 || pos > 0x7FFFFFFF) {
    errno = EINVAL;
    return -1;
  }
  f->pos = (int32_t)pos;

  return pos;
}

int ftruncate(int fd, off_t length) {
  mac_fd_t *f;
  OSErr err;

  if ((f = get_fd(fd, FD_FILE)) == NULL) return -1;
  if ((err = SetEOF(f->refNum, (long)length)) != noErr) return fail(err);

  return 0;
}

int fsync(int fd) {
  ParamBlockRec pb;
  mac_fd_t *f;

  if ((f = get_fd(fd, FD_FILE)) == NULL) return -1;
  memset(&pb, 0, sizeof(pb));
  pb.ioParam.ioRefNum = f->refNum;
  PBFlushFileSync(&pb);

  return 0;
}

static time_t mac_to_unix(uint32_t t) {
  return t > MAC_EPOCH_DELTA ? (time_t)(t - MAC_EPOCH_DELTA) : 0;
}

static void fill_stat(CInfoPBRec *pb, int16_t vref, struct stat *st) {
  memset(st, 0, sizeof(struct stat));
  st->st_dev = (uint16_t)vref;
  st->st_nlink = 1;
  if (pb->hFileInfo.ioFlAttrib & ioDirMask) {
    st->st_mode = S_IFDIR | 0755;
    st->st_ino = pb->dirInfo.ioDrDirID;
    st->st_size = 0;
    st->st_mtime = mac_to_unix(pb->dirInfo.ioDrMdDat);
    st->st_ctime = mac_to_unix(pb->dirInfo.ioDrCrDat);
  } else {
    st->st_mode = S_IFREG | ((pb->hFileInfo.ioFlAttrib & 0x01) ? 0444 : 0644);
    st->st_ino = pb->hFileInfo.ioDirID;
    st->st_size = pb->hFileInfo.ioFlLgLen;
    st->st_mtime = mac_to_unix(pb->hFileInfo.ioFlMdDat);
    st->st_ctime = mac_to_unix(pb->hFileInfo.ioFlCrDat);
  }
  st->st_atime = st->st_mtime;
  st->st_blksize = 4096;
  st->st_blocks = (st->st_size + 511) / 512;
}

int mac_stat(const char *path, struct stat *st) {
  CInfoPBRec pb;
  unsigned char hname[32], namebuf[64];
  int16_t vref;
  int32_t parid;
  int r;
  OSErr err;

  if ((r = resolve(path, &vref, &parid, hname)) != 0) {
    if (r == 1) {
      memset(st, 0, sizeof(struct stat));
      st->st_mode = S_IFDIR | 0755;
      st->st_nlink = 1;
      return 0;
    }
    return -1;
  }

  if (parid == fsRtParID) {
    // a volume's root directory
    err = get_cat_info(vref, fsRtDirID, NULL, &pb, namebuf);
  } else {
    err = get_cat_info(vref, parid, hname, &pb, namebuf);
  }
  if (err != noErr) return fail(err);
  fill_stat(&pb, vref, st);

  return 0;
}

int mac_fstat(int fd, struct stat *st) {
  CInfoPBRec pb;
  unsigned char namebuf[64];
  mac_fd_t *f;
  long eof;

  if (fd >= 0 && fd < MAC_FIRST_FD) {
    memset(st, 0, sizeof(struct stat));
    st->st_mode = S_IFCHR | 0666;
    return 0;
  }
  if ((f = get_fd(fd, 0)) == NULL) return -1;

  if (f->type == FD_SOCKET) {
    memset(st, 0, sizeof(struct stat));
    st->st_mode = S_IFSOCK | 0666;
    return 0;
  }

  if (get_cat_info(f->vRefNum, f->parID, f->name, &pb, namebuf) == noErr) {
    fill_stat(&pb, f->vRefNum, st);
  } else {
    memset(st, 0, sizeof(struct stat));
    st->st_mode = S_IFREG | 0644;
  }
  // the size of the open file, which may not be on the disk yet
  if (GetEOF(f->refNum, &eof) == noErr) st->st_size = eof;

  return 0;
}

int mac_mkdir(const char *path, mode_t mode) {
  HParamBlockRec pb;
  unsigned char hname[32];
  int16_t vref;
  int32_t parid;
  int r;
  OSErr err;

  (void)mode;
  if ((r = resolve(path, &vref, &parid, hname)) != 0) {
    if (r == 1) errno = EEXIST;
    return -1;
  }

  memset(&pb, 0, sizeof(pb));
  pb.fileParam.ioNamePtr = hname;
  pb.fileParam.ioVRefNum = vref;
  pb.fileParam.ioDirID = parid;
  if ((err = PBDirCreateSync(&pb)) != noErr) return fail(err);
  cache_path[0] = 0;

  return 0;
}

int rmdir(const char *path) {
  CInfoPBRec pb;
  unsigned char hname[32], namebuf[64];
  int16_t vref;
  int32_t parid;
  int r;
  OSErr err;

  if ((r = resolve(path, &vref, &parid, hname)) != 0) {
    if (r == 1) errno = EBUSY;
    return -1;
  }
  if ((err = get_cat_info(vref, parid, hname, &pb, namebuf)) != noErr) return fail(err);
  if (!(pb.hFileInfo.ioFlAttrib & ioDirMask)) {
    errno = ENOTDIR;
    return -1;
  }
  if ((err = HDelete(vref, parid, hname)) != noErr) {
    if (err == fBsyErr) errno = ENOTEMPTY;
    else fail(err);
    return -1;
  }
  cache_path[0] = 0;

  return 0;
}

int mac_unlink(const char *path) {
  CInfoPBRec pb;
  unsigned char hname[32], namebuf[64];
  int16_t vref;
  int32_t parid;
  int i, r, open = 0;
  OSErr err;

  if ((r = resolve(path, &vref, &parid, hname)) != 0) {
    if (r == 1) errno = EISDIR;
    return -1;
  }
  if ((err = get_cat_info(vref, parid, hname, &pb, namebuf)) != noErr) return fail(err);
  if (pb.hFileInfo.ioFlAttrib & ioDirMask) {
    // (remove() also takes directories)
    return rmdir(path);
  }

  for (i = MAC_FIRST_FD; i < MAC_MAX_FD; i++) {
    if (same_file(&fds[i], vref, parid, hname)) {
      fds[i].unlink_on_close = 1;
      open = 1;
    }
  }
  if (open) return 0;

  if ((err = HDelete(vref, parid, hname)) != noErr) return fail(err);

  return 0;
}

int mac_rename(const char *from, const char *to) {
  CMovePBRec mpb;
  CInfoPBRec pb;
  unsigned char hfrom[32], hto[32], namebuf[64];
  int16_t vfrom, vto;
  int32_t pfrom, pto;
  OSErr err;

  if (resolve(from, &vfrom, &pfrom, hfrom) != 0) return -1;
  if (resolve(to, &vto, &pto, hto) != 0) return -1;
  if (vfrom != vto) {
    errno = EXDEV;
    return -1;
  }
  if (pfrom == pto && EqualString(hfrom, hto, true, true)) return 0;

  // POSIX rename() replaces the target (when it is not a folder)
  if (get_cat_info(vto, pto, hto, &pb, namebuf) == noErr) {
    if (pb.hFileInfo.ioFlAttrib & ioDirMask) {
      if ((err = HDelete(vto, pto, hto)) != noErr) return fail(err);
    } else if ((err = HDelete(vto, pto, hto)) != noErr) {
      return fail(err);
    }
  }

  if (pfrom != pto) {
    // move it first (under its old name), then rename it
    memset(&mpb, 0, sizeof(mpb));
    mpb.ioNamePtr = hfrom;
    mpb.ioVRefNum = vfrom;
    mpb.ioDirID = pfrom;
    mpb.ioNewDirID = pto;
    mpb.ioNewName = NULL;
    if ((err = PBCatMoveSync(&mpb)) != noErr) return fail(err);
    pfrom = pto;
  }

  if (!EqualString(hfrom, hto, true, true)) {
    if ((err = HRename(vfrom, pfrom, hfrom, hto)) != noErr) return fail(err);
  }
  cache_path[0] = 0;

  return 0;
}

int access(const char *path, int mode) {
  struct stat st;

  (void)mode;
  return mac_stat(path, &st);
}

int chdir(const char *path) {
  CInfoPBRec pb;
  unsigned char hname[32], namebuf[64];
  int16_t vref;
  int32_t parid;
  int r;
  OSErr err;

  if ((r = resolve(path, &vref, &parid, hname)) != 0) {
    if (r == 1) errno = EACCES;
    return -1;
  }
  if (parid == fsRtParID) {
    err = get_cat_info(vref, fsRtDirID, NULL, &pb, namebuf);
  } else {
    err = get_cat_info(vref, parid, hname, &pb, namebuf);
  }
  if (err != noErr) return fail(err);
  if (!(pb.hFileInfo.ioFlAttrib & ioDirMask)) {
    errno = ENOTDIR;
    return -1;
  }
  cwd_vref = vref;
  cwd_dirid = pb.dirInfo.ioDrDirID;
  cache_path[0] = 0;

  return 0;
}

char *getcwd(char *buf, size_t len) {
  if (!fs_inited) mac_fs_init();
  if (buf == NULL || len == 0) {
    errno = EINVAL;
    return NULL;
  }
  if (dir_path(cwd_vref, cwd_dirid, buf, len) != 0) return NULL;
  // without the trailing slash, as getcwd() does (but "/" stays)
  if (strlen(buf) > 1) buf[strlen(buf)-1] = 0;

  return buf;
}

int mac_mkstemp(char *template_path) {
  static uint32_t seed;
  static const char letters[] = "abcdefghijklmnopqrstuvwxyz0123456789";
  char *x;
  int i, n, tries, fd;

  n = strlen(template_path);
  if (n < 6 || strcmp(template_path + n - 6, "XXXXXX")) {
    errno = EINVAL;
    return -1;
  }
  x = template_path + n - 6;
  if (seed == 0) seed = (uint32_t)mac_microseconds() ^ TickCount();

  for (tries = 0; tries < 100; tries++) {
    for (i = 0; i < 6; i++) {
      seed = seed * 1103515245 + 12345;
      x[i] = letters[(seed >> 16) % 36];
    }
    if ((fd = mac_open(template_path, O_RDWR | O_CREAT | O_EXCL, 0600)) != -1) return fd;
    if (errno != EEXIST) return -1;
  }

  errno = EEXIST;
  return -1;
}

int mac_isatty(int fd) {
  (void)fd;
  return 0;
}

int mac_fcntl(int fd, int cmd, ...) {
  mac_fd_t *f;
  va_list ap;
  int arg;

  if ((f = get_fd(fd, 0)) == NULL) return -1;

  switch (cmd) {
    case F_GETFL:
      return f->flags;
    case F_SETFL:
      va_start(ap, cmd);
      arg = va_arg(ap, int);
      va_end(ap);
      f->flags = (f->flags & ~O_NONBLOCK) | (arg & O_NONBLOCK);
      if (f->type == FD_SOCKET) mac_sock_set_nonblock(fd, (arg & O_NONBLOCK) ? 1 : 0);
      return 0;
    case F_GETFD:
    case F_SETFD:
      return 0;
  }

  errno = EINVAL;
  return -1;
}

int mac_gettimeofday(struct timeval *tv, void *tz) {
  uint64_t us;

  (void)tz;
  if (!fs_inited) mac_fs_init();
  if (tv) {
    us = mac_microseconds() - base_us;
    tv->tv_sec = base_secs + (time_t)(us / 1000000);
    tv->tv_usec = (suseconds_t)(us % 1000000);
  }

  return 0;
}

int clock_gettime(clockid_t clockid, struct timespec *tp) {
  struct timeval tv;
  uint64_t us;

  if (clockid == CLOCK_MONOTONIC || clockid == CLOCK_MONOTONIC_RAW) {
    us = mac_microseconds();
    tp->tv_sec = (time_t)(us / 1000000);
    tp->tv_nsec = (long)(us % 1000000) * 1000L;
  } else {
    mac_gettimeofday(&tv, NULL);
    tp->tv_sec = tv.tv_sec;
    tp->tv_nsec = tv.tv_usec * 1000L;
  }

  return 0;
}

int usleep(useconds_t us) {
  uint64_t end = mac_microseconds() + us;

  // whole ticks are given to the other applications, the rest is waited
  // here (a cooperative system: nothing else would run otherwise)
  if (us >= 16667) {
    mac_yield(us / 16667);
  } else {
    mac_yield(0);
  }
  while (mac_microseconds() < end) {
    SystemTask();
  }

  return 0;
}

int mac_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
  fd_set rin, win;
  uint64_t end = 0;
  int i, n, r;

  if (nfds > MAC_MAX_FD) nfds = MAC_MAX_FD;
  if (readfds) rin = *readfds; else FD_ZERO(&rin);
  if (writefds) win = *writefds; else FD_ZERO(&win);
  if (exceptfds) FD_ZERO(exceptfds);
  if (timeout) end = mac_microseconds() + (uint64_t)timeout->tv_sec * 1000000 + timeout->tv_usec;

  for (;;) {
    if (readfds) FD_ZERO(readfds);
    if (writefds) FD_ZERO(writefds);
    n = 0;
    for (i = 0; i < nfds; i++) {
      if (FD_ISSET(i, &rin)) {
        if (fds[i].type == FD_SOCKET) {
          r = mac_sock_ready(i, 0);
        } else {
          r = (fds[i].type == FD_FILE || i < MAC_FIRST_FD) ? 1 : -1;
        }
        if (r < 0) {
          errno = EBADF;
          return -1;
        }
        if (r > 0) {
          FD_SET(i, readfds);
          n++;
        }
      }
      if (FD_ISSET(i, &win)) {
        if (fds[i].type == FD_SOCKET) {
          r = mac_sock_ready(i, 1);
        } else {
          r = (fds[i].type == FD_FILE || i < MAC_FIRST_FD) ? 1 : -1;
        }
        if (r < 0) {
          errno = EBADF;
          return -1;
        }
        if (r > 0) {
          FD_SET(i, writefds);
          n++;
        }
      }
    }
    if (n > 0) return n;
    if (timeout && mac_microseconds() >= end) return 0;
    // the other threads (one of them may be what this waits for) and the
    // other applications
    thread_yield(0);
    mac_yield(0);
  }
}

// directories

static int dir_add(DIR *dir, const char *name, int isdir) {
  int n = strlen(name) + 1;
  char *names;
  unsigned char *flags;

  names = realloc(dir->names, dir->size + n);
  if (names == NULL) return -1;
  dir->names = names;
  flags = realloc(dir->isdir, dir->count + 1);
  if (flags == NULL) return -1;
  dir->isdir = flags;
  memcpy(dir->names + dir->size, name, n);
  dir->size += n;
  dir->isdir[dir->count] = isdir;
  dir->count++;

  return 0;
}

DIR *opendir(const char *path) {
  CInfoPBRec pb;
  HParamBlockRec vpb;
  unsigned char hname[32], namebuf[64];
  char name[64];
  int16_t vref;
  int32_t parid, dirid;
  int i, r;
  DIR *dir;
  OSErr err;

  if ((r = resolve(path, &vref, &parid, hname)) == -1) return NULL;

  if ((dir = calloc(1, sizeof(DIR))) == NULL) {
    errno = ENOMEM;
    return NULL;
  }

  if (r == 1) {
    // "/": the mounted volumes
    for (i = 1;; i++) {
      memset(&vpb, 0, sizeof(vpb));
      namebuf[0] = 0;
      vpb.volumeParam.ioNamePtr = namebuf;
      vpb.volumeParam.ioVolIndex = i;
      if (PBHGetVInfoSync(&vpb) != noErr) break;
      from_hfs_name(namebuf, name, sizeof(name));
      dir_add(dir, name, 1);
    }
    return dir;
  }

  if (parid == fsRtParID) {
    err = get_cat_info(vref, fsRtDirID, NULL, &pb, namebuf);
  } else {
    err = get_cat_info(vref, parid, hname, &pb, namebuf);
  }
  if (err != noErr) {
    free(dir);
    fail(err);
    return NULL;
  }
  if (!(pb.hFileInfo.ioFlAttrib & ioDirMask)) {
    free(dir);
    errno = ENOTDIR;
    return NULL;
  }
  dirid = pb.dirInfo.ioDrDirID;

  // the whole directory is listed now: the callers often delete or create
  // entries while they walk it, which shifts the catalog indexes
  for (i = 1;; i++) {
    memset(&pb, 0, sizeof(pb));
    namebuf[0] = 0;
    pb.hFileInfo.ioNamePtr = namebuf;
    pb.hFileInfo.ioVRefNum = vref;
    pb.hFileInfo.ioDirID = dirid;
    pb.hFileInfo.ioFDirIndex = i;
    if (PBGetCatInfoSync(&pb) != noErr) break;
    from_hfs_name(namebuf, name, sizeof(name));
    if (dir_add(dir, name, (pb.hFileInfo.ioFlAttrib & ioDirMask) ? 1 : 0) != 0) break;
  }

  return dir;
}

struct dirent *readdir(DIR *dir) {
  char *p;
  int i;

  if (dir == NULL || dir->index >= dir->count) return NULL;

  for (p = dir->names, i = 0; i < dir->index; i++) {
    p += strlen(p) + 1;
  }
  memset(&dir->ent, 0, sizeof(struct dirent));
  strncpy(dir->ent.d_name, p, sizeof(dir->ent.d_name) - 1);
  dir->ent.d_type = dir->isdir[dir->index] ? DT_DIR : DT_REG;
  dir->ent.d_ino = dir->index + 1;
  dir->index++;

  return &dir->ent;
}

void rewinddir(DIR *dir) {
  if (dir) dir->index = 0;
}

int closedir(DIR *dir) {
  if (dir) {
    if (dir->names) free(dir->names);
    if (dir->isdir) free(dir->isdir);
    free(dir);
  }

  return 0;
}

// what the serial port code of sys.c expects from <termios.h>: there are
// no serial devices here
int tcgetattr(int fd, void *t) {
  (void)fd; (void)t;
  errno = ENOTTY;
  return -1;
}

int tcsetattr(int fd, int act, const void *t) {
  (void)fd; (void)act; (void)t;
  errno = ENOTTY;
  return -1;
}
