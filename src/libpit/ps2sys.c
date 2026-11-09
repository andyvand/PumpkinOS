// POSIX functions missing from the ps2sdk newlib glue (PlayStation 2 build).
//
// ftruncate(): the glue only has a stub returning ENOSYS, and the IOP file
// drivers have no call to shorten an open file. The glue's descriptor table
// knows the name a file was opened with (the fileXio operations keep it), so
// the file is rewritten through a second descriptor: its first `length`
// bytes are copied to a side file, it is reopened with O_TRUNC and the bytes
// are copied back (extending a file just pads it with zeros through the
// given descriptor). The caller's descriptor keeps its offset, like POSIX
// says. Used by vfs_truncate() when a database shrinks.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>

#include <ps2sdkapi.h>

// libcglue's fdman.h (not installed): the descriptor table
#define PS2_FILENO_MAX 64

typedef struct {
  uint32_t flags;
  uint32_t ref_count;
  _libcglue_fdman_fd_info_t info;
} ps2_descriptor_t;

extern ps2_descriptor_t *__descriptormap[PS2_FILENO_MAX];

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
  ps2_descriptor_t *d;
  uint8_t buf[4096];
  char *filename, *tmp;
  off_t pos, size, left;
  int in, out, n, r = -1;

  if (fd < 0 || fd >= PS2_FILENO_MAX || (d = __descriptormap[fd]) == NULL || d->info.ops == NULL || d->info.ops->getfilename == NULL) {
    errno = EBADF;
    return -1;
  }
  if ((filename = d->info.ops->getfilename(d->info.userdata)) == NULL || filename[0] == 0) {
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
  if ((tmp = malloc(strlen(filename) + 8)) == NULL) {
    lseek(fd, pos, SEEK_SET);
    errno = ENOMEM;
    return -1;
  }
  sprintf(tmp, "%s.trunc", filename);

  if ((out = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644)) >= 0) {
    lseek(fd, 0, SEEK_SET);
    r = copy_bytes(fd, out, length, buf, sizeof(buf));
    close(out);

    if (r == 0) {
      r = -1;
      if ((in = open(tmp, O_RDONLY, 0)) >= 0) {
        if ((out = open(filename, O_WRONLY | O_TRUNC, 0644)) >= 0) {
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

// Thread-specific data: the ps2sdk pthread library (pthread-embedded) only
// has 29 keys, and OpenSSL 4 creates one for every RCU lock and thread-local
// it sets up (well over that during its initialization: the default
// provider then fails to load, or a NULL lock hangs it). These replace the
// four key functions with a larger table (the link picks them over the
// library's); the values are kept per EE kernel thread, so PumpkinOS's
// cooperative tasks (which share one kernel thread) share them, like on the
// other handheld ports. Key destructors are not run at thread exit (the
// threads that use them never exit).

#include <pthread.h>

#define PS2_TLS_KEYS    256
#define PS2_TLS_THREADS 16

extern int GetThreadId(void);

typedef struct {
  int used;
  void (*destructor)(void *);
} ps2_tls_key_t;

typedef struct {
  int tid;
  void *value[PS2_TLS_KEYS];
} ps2_tls_thread_t;

static ps2_tls_key_t tls_keys[PS2_TLS_KEYS];
static ps2_tls_thread_t tls_threads[PS2_TLS_THREADS];
static int tls_last = 0;

static ps2_tls_thread_t *tls_thread(int create) {
  int tid = GetThreadId();
  int i;

  if (tls_threads[tls_last].tid == tid) return &tls_threads[tls_last];
  for (i = 0; i < PS2_TLS_THREADS; i++) {
    if (tls_threads[i].tid == tid) {
      tls_last = i;
      return &tls_threads[i];
    }
  }
  if (!create) return NULL;
  for (i = 0; i < PS2_TLS_THREADS; i++) {
    if (tls_threads[i].tid == 0) {
      tls_threads[i].tid = tid;
      tls_last = i;
      return &tls_threads[i];
    }
  }

  return NULL;
}

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *)) {
  int i;

  for (i = 1; i < PS2_TLS_KEYS; i++) {   // 0 is never handed out (an unset key_t)
    if (!tls_keys[i].used) {
      tls_keys[i].used = 1;
      tls_keys[i].destructor = destructor;
      *key = (pthread_key_t)i;
      return 0;
    }
  }

  return EAGAIN;
}

int pthread_key_delete(pthread_key_t key) {
  int k = (int)key, i;

  if (k <= 0 || k >= PS2_TLS_KEYS || !tls_keys[k].used) return EINVAL;
  tls_keys[k].used = 0;
  tls_keys[k].destructor = NULL;
  for (i = 0; i < PS2_TLS_THREADS; i++) {
    tls_threads[i].value[k] = NULL;
  }

  return 0;
}

void *pthread_getspecific(pthread_key_t key) {
  ps2_tls_thread_t *t;
  int k = (int)key;

  if (k <= 0 || k >= PS2_TLS_KEYS || !tls_keys[k].used) return NULL;
  if ((t = tls_thread(0)) == NULL) return NULL;

  return t->value[k];
}

int pthread_setspecific(pthread_key_t key, const void *value) {
  ps2_tls_thread_t *t;
  int k = (int)key;

  if (k <= 0 || k >= PS2_TLS_KEYS || !tls_keys[k].used) return EINVAL;
  if ((t = tls_thread(1)) == NULL) return ENOMEM;
  t->value[k] = (void *)value;

  return 0;
}

// C heap usage, for the log: what sbrk() can still hand out and what malloc
// holds (the console has 32MB in all, see the heap sizing in pumpkin.c)
#include <malloc.h>
#include "sys.h"
#include "debug.h"

extern void *EndOfHeap(void);

void sys_ps2_memlog(const char *tag) {
  struct mallinfo mi = mallinfo();
  uint32_t avail = (uint32_t)((uint8_t *)EndOfHeap() - (uint8_t *)sbrk(0));

  debug(DEBUG_INFO, "SYS", "memory at %s: %u KB left for sbrk, malloc arena %u KB (%u KB in use, %u KB free)", tag, avail / 1024, (unsigned)mi.arena / 1024, (unsigned)mi.uordblks / 1024, (unsigned)mi.fordblks / 1024);
}
