// Classic Mac OS (Retro68) POSIX layer used by libpit/sys.c.
//
// Retro68's newlib syscalls open files by HFS path (open() takes
// "Volume:Folder:file") and stub out stat(), mkdir(), unlink(), rename()
// and friends; there are no directories, no sockets and no select().
// macsys.c implements the calls PumpkinOS needs on top of the File Manager
// and macnet.c the sockets on top of MacTCP. Both share one descriptor
// table, so read(), write(), close() and select() work on files and
// sockets alike.
//
// Paths are POSIX style:
//   "/Volume/Folder/file"   absolute: the first component is a volume name
//   "Folder/file", "./x"    relative to the current directory, which starts
//                           as the folder holding the application
//   "..", "a/../b"          the parent directory, as usual
// A ':' in a POSIX name is a '/' in the HFS name and the other way around
// (as on Mac OS X). HFS names have at most 31 characters.
//
// The names that newlib already defines (open, read, stat, ...) are mapped
// to the mac_ functions below with function-like macros; only sys.c
// includes this header.

#ifndef PIT_MACSYS_H
#define PIT_MACSYS_H

#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>

#ifdef __cplusplus
extern "C" {
#endif

// (newlib hides these behind _POSIX_MONOTONIC_CLOCK; the values are its own)
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC     (clockid_t)4
#endif
#ifndef CLOCK_MONOTONIC_RAW
#define CLOCK_MONOTONIC_RAW (clockid_t)5
#endif

// initializes the layer: the current directory becomes the application's
// folder. Called by sys_init() (src/retro68/main.c calls it earlier too).
void mac_fs_init(void);

// the application's folder as a POSIX path ("/Volume/Folder/"), with the
// trailing slash
const char *mac_app_dir(void);

// converts a POSIX path to a full HFS path ("Volume:Folder:file") for the
// calls that only take HFS paths (newlib's fopen(), OpenSSL's file BIOs).
// Returns 0 on success.
int mac_hfs_path(const char *path, char *hfs, int len);

int mac_open(const char *path, int flags, ...);
int mac_close(int fd);
ssize_t mac_read(int fd, void *buf, size_t len);
ssize_t mac_write(int fd, const void *buf, size_t len);
off_t mac_lseek(int fd, off_t offset, int whence);
int mac_stat(const char *path, struct stat *st);
int mac_fstat(int fd, struct stat *st);
int mac_mkdir(const char *path, mode_t mode);
int mac_unlink(const char *path);
int mac_rename(const char *from, const char *to);
int mac_mkstemp(char *template_path);
int mac_isatty(int fd);
int mac_fcntl(int fd, int cmd, ...);
int mac_gettimeofday(struct timeval *tv, void *tz);
int mac_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

// not in newlib at all on this target: defined with their own names
int rmdir(const char *path);
int chdir(const char *path);
char *getcwd(char *buf, size_t len);
int ftruncate(int fd, off_t length);
int fsync(int fd);
int access(const char *path, int mode);
int usleep(useconds_t us);
int clock_gettime(clockid_t clockid, struct timespec *tp);

// descriptor kinds (macnet.c)
int mac_fd_is_socket(int fd);

// the free memory in the application partition, in bytes (the largest
// block the Memory Manager can still give)
uint32_t sys_mac_heap_avail(void);

// thread stacks in the stack area (see macsys.c): reserve before
// MaxApplZone(), then allocate / free one per thread
void mac_stacks_reserve(uint32_t slot_size, int slots, uint32_t main_stack);
void *mac_stack_alloc(uint32_t size);
void mac_stack_free(void *p);

// gives time to the other applications (WaitNextEvent with no events):
// ticks = 0 returns as soon as they had their turn
void mac_yield(uint32_t ticks);

// microseconds since startup (the Time Manager's Microseconds())
uint64_t mac_microseconds(void);

// networking (macnet.c): opens the MacTCP driver; 0 when it is there
int mac_net_init(void);
void mac_net_finish(void);
int mac_net_ready(void);

// the socket side of the descriptor table (macnet.c, used by macsys.c)
ssize_t mac_sock_read(int fd, void *buf, size_t len);
ssize_t mac_sock_write(int fd, const void *buf, size_t len);
int mac_sock_close(int fd);
// readable (want 0) / writable (want 1) right now: 1 yes, 0 no, -1 error
int mac_sock_ready(int fd, int want);
int mac_sock_set_nonblock(int fd, int on);

#define open(...)           mac_open(__VA_ARGS__)
#define close(fd)           mac_close(fd)
#define read(fd, b, n)      mac_read(fd, b, n)
#define write(fd, b, n)     mac_write(fd, b, n)
#define lseek(fd, o, w)     mac_lseek(fd, o, w)
#define stat(p, s)          mac_stat(p, s)
#define lstat(p, s)         mac_stat(p, s)
#define fstat(fd, s)        mac_fstat(fd, s)
#define mkdir(p, m)         mac_mkdir(p, m)
#define unlink(p)           mac_unlink(p)
#define remove(p)           mac_unlink(p)
#define rename(a, b)        mac_rename(a, b)
#define mkstemp(t)          mac_mkstemp(t)
#define isatty(fd)          mac_isatty(fd)
#define fcntl(...)          mac_fcntl(__VA_ARGS__)
#define gettimeofday(t, z)  mac_gettimeofday(t, z)
#define select(n, r, w, e, t) mac_select(n, r, w, e, t)

#ifdef __cplusplus
}
#endif

#endif
