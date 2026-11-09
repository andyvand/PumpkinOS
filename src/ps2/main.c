// PumpkinOS entry point for the Sony PlayStation 2 (ps2sdk + SDL3).
//
// Mirrors src/psp/main.c: everything is linked statically, there is no
// script engine and the OS runs in single application mode (the Launcher and
// the applications run in-process, as cooperative threads).
//
// Layout: the ELF is started from wherever a loader finds it (a USB stick,
// "mass:/PumpkinOS/PUMPKINOS.ELF", a memory card, the host under PCSX2 or
// ps2link, a CD/DVD: the BIOS boots "cdrom0:\PUMPKINOS.ELF;1" from the
// SYSTEM.CNF of the disc image GNUMakefile builds, uLaunchELF starts
// "cdfs:/PUMPKINOS.ELF"). The home is a RAM disk, like on the GameCube:
// "ram:/PumpkinOS/" (libpit/ps2ramdisk.c), lost at power off; at every boot
// the vfs/ directory shipped next to the ELF is copied to ram:/PumpkinOS/vfs/
// and PumpkinOS deploys the application PRCs from there, with the temporary
// files (tmp/) next to it. A memory card is far too slow for the storage
// (more than 15 minutes for the first deploy) and has only 3 file handles;
// -memcard keeps the home on the first formatted card (mc0:/PumpkinOS/, or
// mc1:) and -home puts it anywhere else. The log (pumpkin.log) and stderr
// (pumpkin.err) go next to the ELF when that place is writable, else to the
// home when it is not the RAM disk (on a memory card the log is buffered),
// else to the EE console (stderr; -log names a file instead).
//
// A disc is read through the cdfs device of ps2sdk's cdfs.irx (loaded by
// ps2_drivers): "cdrom0:" (the BIOS device, no directory listing) is
// mapped to "cdfs:/". A disc cannot carry command line options, so the
// options can also come from a pumpkin.cfg file next to the ELF.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#include <kernel.h>
#include <sifrpc.h>
#include <iopcontrol.h>
#include <sbv_patches.h>
#include <libmc.h>
#include <ps2_filesystem_driver.h>

#include <SDL3/SDL.h>

#include "sys.h"
#include "thread.h"
#include "ptr.h"
#include "vfs.h"
#include "vfslocal.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "pwindow.h"
#include "script.h"
#include "debug.h"
#include "ps2.h"

#define MAX_PATH_LEN 256

// the PumpkinOS directory on the RAM disk or the memory card
#define PS2_HOME_DIR "PumpkinOS"

// the RAM disk device (libpit/ps2ramdisk.c) and how much it may hold by
// default (the deployed storage takes about 1MB, the PRCs it is deployed
// from another 0.9MB while the deploy goes on)
#define PS2_RAMDISK "ram"
#define PS2_RAMDISK_KB 3072

// where to look for the ELF (and its vfs/) when the loader gives no path
#define PS2_DEFAULT_BOOT "mass:/PumpkinOS/"

// how long to wait for the link and then for a DHCP lease at startup
#define PS2_NET_TIMEOUT_MS 10000

// the options file next to the ELF (a disc has no command line)
#define PS2_CONFIG_FILE "pumpkin.cfg"
#define PS2_CONFIG_MAX_ARGS 64
#define PS2_CONFIG_MAX_SIZE 4096

// the CD file system device of cdfs.irx; what the BIOS calls "cdrom0:"
#define PS2_CDFS_ROOT "cdfs:/"

// the log on a memory card is kept in memory and appended to the file every
// so often (see cardlog_write)
#define PS2_CARDLOG_SIZE 16384
#define PS2_CARDLOG_FLUSH_US 5000000

extern int liblsdl3_load(void);
extern int liblsdl3_init(int pe, script_ref_t obj);
extern int liblsdl3_unload(void);

extern int libos_app_init(int pe);
extern int libos_start(int pe);

#ifndef NO_HTTPS
extern int liblopenssl_load(void);
extern int liblopenssl_init(int pe, script_ref_t obj);
#endif

// gsKit render queues: SDL's PS2 renderer asks gsKit for a 2MB one-shot
// queue (allocated twice, it is double buffered) and a 256KB persistent one,
// 4.3MB of the 32MB in all. PumpkinOS draws two or three textured quads per
// frame, so the call is wrapped (-Wl,--wrap in GNUMakefile) and the queues
// cut down to 256KB + 32KB.
#define PS2_GS_OS_POOL  (256 * 1024)
#define PS2_GS_PER_POOL (32 * 1024)

extern void *__real_gsKit_init_global_custom(int ospoolsize, int perpoolsize);

void *__wrap_gsKit_init_global_custom(int ospoolsize, int perpoolsize) {
  return __real_gsKit_init_global_custom(PS2_GS_OS_POOL, PS2_GS_PER_POOL);
}

// IOP reset (a loader may have left modules behind), SIF RPC, the usual
// patches and the file system drivers of ps2_drivers (fileXio, memory
// cards, USB mass storage, MX4SIO, CDFS, HDD when booted from it). SDL's own
// main wrapper does the same; it is done here because PumpkinOS has its own
// main() and needs the memory card before SDL comes up.
static void init_iop(void) {
  SifInitRpc(0);
  while (!SifIopReset(NULL, 0)) {
  }
  while (!SifIopSync()) {
  }
  SifInitRpc(0);
  sbv_patch_enable_lmb();
  sbv_patch_disable_prefix_check();
  sbv_patch_fileio();

  init_ps2_filesystem_driver();
}

static void usage(void) {
  fprintf(stderr, "usage: PUMPKINOS.ELF [options]\n");
  fprintf(stderr, "  -width <n>     screen width in pixels (default 640)\n");
  fprintf(stderr, "  -height <n>    screen height in pixels (default 448)\n");
  fprintf(stderr, "  -density <n>   72 (low, default) or 144 (double)\n");
  fprintf(stderr, "  -depth <n>     application bit depth (default 16)\n");
  fprintf(stderr, "  -hdepth <n>    host bit depth, 16 or 32 (default 16)\n");
  fprintf(stderr, "  -mode <n>      1 = single application (default), 0 = desktop\n");
  fprintf(stderr, "  -dia <0|1>     dynamic input area (default 1)\n");
  fprintf(stderr, "  -app <name>    start this application instead of the Launcher\n");
  fprintf(stderr, "  -boot <dir>    directory holding the ELF and the vfs/ to seed from (default: where the ELF was loaded from)\n");
  fprintf(stderr, "  -home <dir>    PumpkinOS home (default: the RAM disk, %s:/%s/)\n", PS2_RAMDISK, PS2_HOME_DIR);
  fprintf(stderr, "  -memcard       home on the first formatted PS2 memory card (%s/ on mc0: or mc1:)\n", PS2_HOME_DIR);
  fprintf(stderr, "  -ramdisk <KB>  how much the RAM disk may hold (default %d)\n", PS2_RAMDISK_KB);
  fprintf(stderr, "  -vfs <dir>     VFS root directory (default <home>/vfs)\n");
  fprintf(stderr, "  -log <file>    log file (default pumpkin.log next to the ELF, else in a home that is not the RAM disk, else stderr)\n");
  fprintf(stderr, "  -seed          copy vfs/ from the boot directory again (overwrites the application PRCs)\n");
  fprintf(stderr, "  -nonet         do not bring the network up\n");
  fprintf(stderr, "  -pal | -ntsc   force the TV mode (default: the console's region)\n");
  fprintf(stderr, "  -progressive   480p output (component cable)\n");
  fprintf(stderr, "  -d <level>     log level: 0 = errors, 1 = info (default), 2 = trace; <level>:<system> for one system\n");
  fprintf(stderr, "the same options are read from %s next to the ELF first (a disc has no command line)\n", PS2_CONFIG_FILE);
}

// does the directory exist ? (sys_stat() double checks the memory card
// driver's answers, see libpit/sys.c)
static int dir_exists(const char *path) {
  sys_stat_t st;

  return sys_stat(path, &st) == 0 && (st.mode & SYS_IFDIR);
}

static void mkdir_if_missing(const char *path) {
  if (!dir_exists(path)) {
    if (sys_mkdir(path) != 0) {
      debug(DEBUG_ERROR, "MAIN", "could not create directory \"%s\"", path);
    }
  }
}

// "dev:/dir/" from "dev:/dir/" or "dev:/dir": a device prefix is kept as is
static void add_slash(char *path, int len) {
  int n = sys_strlen(path);

  if (n > 0 && n < len-1 && path[n-1] != '/' && path[n-1] != ':') {
    path[n] = '/';
    path[n+1] = 0;
  }
}

// is this a path on the disc drive ? ("cdrom0:", "cdrom:", "cdfs:")
static int is_cdrom(const char *path) {
  return !strncmp(path, "cdrom", 5) || !strncmp(path, "cdfs:", 5);
}

// the BIOS and the loaders name the disc "cdrom0:" (a device without
// directory listing, with ISO9660 paths like "\\DIR\\FILE.EXT;1"): the
// disc is read through cdfs.irx instead, whose device is "cdfs:"
static void cdrom_to_cdfs(char *boot, int len) {
  if (!strncmp(boot, "cdrom", 5)) {
    sys_strncpy(boot, PS2_CDFS_ROOT, len-1);
    boot[len-1] = 0;
  }
}

// the directory the ELF was loaded from, from argv[0] ("mass:/PumpkinOS/
// PUMPKINOS.ELF", "host:pumpkinos.elf", "cdrom0:\\PUMPKINOS.ELF;1", ...);
// the C library already made it the current directory (libcglue __init_cwd),
// getcwd() is the fallback when argv[0] is not usable
static void find_boot(const char *arg0, char *boot, int len) {
  char buf[MAX_PATH_LEN];
  char *p, *last;
  int i;

  boot[0] = 0;

  if (arg0 && arg0[0]) {
    sys_strncpy(buf, arg0, MAX_PATH_LEN-1);
    buf[MAX_PATH_LEN-1] = 0;
    if ((p = strchr(buf, ';')) != NULL) *p = 0;  // "cdrom0:\\X.ELF;1"
    for (i = 0; buf[i]; i++) {
      if (buf[i] == '\\') buf[i] = '/';
    }
    for (last = NULL, p = buf; *p; p++) {
      if (*p == '/' || *p == ':') last = p;
    }
    if (last != NULL && strchr(buf, ':') != NULL) {
      last[1] = 0;
      sys_strncpy(boot, buf, len-1);
      boot[len-1] = 0;
    }
  }

  if (boot[0] == 0) {
    if (getcwd(buf, MAX_PATH_LEN) != NULL && buf[0]) {
      sys_strncpy(boot, buf, len-1);
      boot[len-1] = 0;
    } else {
      sys_strncpy(boot, PS2_DEFAULT_BOOT, len-1);
    }
  }
  cdrom_to_cdfs(boot, len);
  add_slash(boot, len);
}

// the options file next to the ELF: the same options as the command line,
// separated by blanks or line breaks, '#' starts a comment. They are placed
// before the real arguments, which therefore win.
static int read_config(const char *boot, char **argv, int max, char *buf, int size) {
  char path[MAX_PATH_LEN];
  int fd, n, i, argc = 0, comment = 0;

  sys_snprintf(path, MAX_PATH_LEN-1, "%s%s", boot, PS2_CONFIG_FILE);
  if ((fd = sys_open(path, SYS_READ)) == -1) {
    return 0;
  }
  n = sys_read(fd, (uint8_t *)buf, size-1);
  sys_close(fd);
  if (n <= 0) return 0;
  buf[n] = 0;

  for (i = 0; i < n && argc < max; i++) {
    if (buf[i] == '#') comment = 1;
    if (buf[i] == '\n' || buf[i] == '\r') comment = 0;
    if (comment || buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n' || buf[i] == '\r') {
      buf[i] = 0;
    } else if (i == 0 || buf[i-1] == 0) {
      argv[argc++] = &buf[i];
    }
  }

  return argc;
}

static const char *mc_type_name(int type) {
  switch (type) {
    case sceMcTypeNoCard: return "no card";
    case sceMcTypePS1:    return "PS1 card";
    case sceMcTypePS2:    return "PS2 card";
    case sceMcTypePDA:    return "PocketStation";
  }
  return "unknown";
}

// the first slot with a formatted PS2 memory card: home = "mcN:/PumpkinOS/"
static int select_memory_card(char *home, int len) {
  int port, type, free, format, result;

  if (mcInit(MC_TYPE_MC) < 0) {
    debug(DEBUG_ERROR, "MAIN", "memory card library initialization failed");
    return -1;
  }

  for (port = 0; port < 2; port++) {
    type = sceMcTypeNoCard;
    free = 0;
    format = 0;
    result = -100;
    if (mcGetInfo(port, 0, &type, &free, &format) < 0) {
      debug(DEBUG_ERROR, "MAIN", "mcGetInfo(%d) failed", port);
      continue;
    }
    mcSync(0, NULL, &result);
    debug(DEBUG_INFO, "MAIN", "memory card slot %d: %s%s, %d KB free (%d)", port+1, mc_type_name(type), type == sceMcTypePS2 ? (format == MC_FORMATTED ? ", formatted" : ", not formatted") : "", free, result);
    if (type == sceMcTypePS2 && format == MC_FORMATTED) {
      sys_snprintf(home, len-1, "mc%d:/%s/", port, PS2_HOME_DIR);
      return port;
    }
  }

  return -1;
}

static int copy_file(const char *src, const char *dst) {
  uint8_t buf[16384];
  int fin, fout, n, r = 0;

  if ((fin = sys_open(src, SYS_READ)) == -1) {
    return -1;
  }
  if ((fout = sys_create(dst, SYS_WRITE | SYS_TRUNC, 0644)) == -1) {
    sys_close(fin);
    return -1;
  }
  while ((n = sys_read(fin, buf, sizeof(buf))) > 0) {
    if (sys_write(fout, buf, n) != n) {
      r = -1;
      break;
    }
  }
  if (n < 0) r = -1;
  sys_close(fout);
  sys_close(fin);

  return r;
}

// recursive copy of a directory tree (the initial VFS to the memory card).
// The directory is listed first and closed before its entries are copied:
// the CD file system driver has only 4 directory descriptors (and 4 file
// descriptors), and vfs/app_card/PALM/Programs is already 4 levels deep.
static int copy_tree(const char *src, const char *dst) {
  char s[MAX_PATH_LEN], d[MAX_PATH_LEN];
  char *names, *name;
  sys_dir_t *dir;
  sys_stat_t st;
  int n, count, len, r = 0;

  if (!dir_exists(dst) && sys_mkdir(dst) != 0) {
    debug(DEBUG_ERROR, "MAIN", "could not create directory \"%s\"", dst);
    return -1;
  }

  if ((dir = sys_opendir(src)) == NULL) {
    debug(DEBUG_ERROR, "MAIN", "could not open directory \"%s\"", src);
    return -1;
  }

  // the names, one after the other, each terminated by a zero
  len = 16 * FILE_PATH;
  if ((names = sys_calloc(1, len)) == NULL) {
    sys_closedir(dir);
    return -1;
  }
  for (n = 0, count = 0;;) {
    name = &names[n];
    if (sys_readdir(dir, name, FILE_PATH) != 0) break;
    if (!strcmp(name, ".") || !strcmp(name, "..")) continue;
    n += sys_strlen(name) + 1;
    count++;
    if (n + FILE_PATH > len) {
      len *= 2;
      if ((names = sys_realloc(names, len)) == NULL) {
        sys_closedir(dir);
        return -1;
      }
    }
  }
  sys_closedir(dir);

  for (n = 0, name = names; count > 0; count--, name += sys_strlen(name) + 1) {
    sys_snprintf(s, MAX_PATH_LEN-1, "%s/%s", src, name);
    sys_snprintf(d, MAX_PATH_LEN-1, "%s/%s", dst, name);
    if (sys_stat(s, &st) != 0) continue;
    if (st.mode & SYS_IFDIR) {
      if (copy_tree(s, d) != 0) r = -1;
    } else {
      debug(DEBUG_INFO, "MAIN", "copying \"%s\" to \"%s\"", s, d);
      if (copy_file(s, d) != 0) {
        debug(DEBUG_ERROR, "MAIN", "could not copy \"%s\"", s);
        r = -1;
      }
    }
  }
  sys_free(names);

  return r;
}

// is this a directory with no entries ? (a missing directory counts as empty)
static int dir_empty(const char *path) {
  char name[FILE_PATH];
  sys_dir_t *dir;
  int empty = 1;

  if ((dir = sys_opendir(path)) == NULL) return 1;
  while (sys_readdir(dir, name, sizeof(name)) == 0) {
    if (strcmp(name, ".") && strcmp(name, "..")) {
      empty = 0;
      break;
    }
  }
  sys_closedir(dir);

  return empty;
}

// is this a path on the RAM disk ? ("ram:")
static int is_ramdisk(const char *path) {
  return !strncmp(path, PS2_RAMDISK ":", sizeof(PS2_RAMDISK));
}

// is this a path on a memory card ? ("mc0:", "mc1:")
static int is_memory_card(const char *path) {
  return path[0] == 'm' && path[1] == 'c' && path[3] == ':';
}

// The log on a memory card. The card driver (mcman) has 3 file handles in
// all: a log and a stderr kept open there leave one for PumpkinOS, whose
// database installs then fail (mkdir: "File descriptor value too large").
// So the log lines are collected in memory and appended to the file, which
// is opened and closed for the occasion, when the buffer is full or 5
// seconds after the previous flush; stderr is sent to the same stream.
// (A crash can lose the last seconds of the log; the host and USB logs
// are written at once, as before.)
static char cardlog_path[MAX_PATH_LEN];
static char cardlog_buf[PS2_CARDLOG_SIZE];
static int cardlog_len;
static int64_t cardlog_last;

static void cardlog_flush(void) {
  int fd, n, w;

  if (cardlog_len > 0) {
    if ((fd = open(cardlog_path, O_WRONLY | O_CREAT, 0644)) != -1) {
      lseek(fd, 0, SEEK_END);
      for (n = 0; n < cardlog_len; n += w) {
        if ((w = write(fd, cardlog_buf + n, cardlog_len - n)) <= 0) break;
      }
      close(fd);
    }
    cardlog_len = 0;
  }
  cardlog_last = sys_get_clock();
}

static int cardlog_write(void *cookie, const char *buf, int n) {
  if (n > PS2_CARDLOG_SIZE - cardlog_len) cardlog_flush();
  if (n > PS2_CARDLOG_SIZE) n = PS2_CARDLOG_SIZE;
  sys_memcpy(cardlog_buf + cardlog_len, buf, n);
  cardlog_len += n;
  if (sys_get_clock() - cardlog_last > PS2_CARDLOG_FLUSH_US) cardlog_flush();

  return n;
}

// can files be created in this directory ? (the boot medium may be a disc)
static int writable_dir(const char *dir) {
  char path[MAX_PATH_LEN];
  int fd;

  sys_snprintf(path, MAX_PATH_LEN-1, "%spumpkin.err", dir);
  if ((fd = sys_create(path, SYS_WRITE | SYS_TRUNC, 0644)) == -1) {
    return 0;
  }
  sys_close(fd);

  return 1;
}

int main(int argc, char *argv[]) {
  char boot[MAX_PATH_LEN], home[MAX_PATH_LEN], work[MAX_PATH_LEN];
  char vfsdir[MAX_PATH_LEN], tmpdir[MAX_PATH_LEN], logfile[MAX_PATH_LEN], errfile[MAX_PATH_LEN], path[MAX_PATH_LEN], seed[MAX_PATH_LEN];
  char *args[PS2_CONFIG_MAX_ARGS + 1], config[PS2_CONFIG_MAX_SIZE];
  const char *vfs_arg = NULL, *log_arg = NULL, *boot_arg = NULL, *home_arg = NULL, *tvmode = NULL;
  char *d, *p;
  sys_stat_t st;
  int i, fd, nargs = 0, level = DEBUG_INFO, net = 1, reseed = 0, progressive = 0, mc = -1, memcard = 0, ramdisk_kb = PS2_RAMDISK_KB;

  init_iop();

  // "-boot" is the only option that must be known before the options file
  // next to the ELF can be read
  for (i = 1; i < argc-1; i++) {
    if (!strcmp(argv[i], "-boot")) boot_arg = argv[i+1];
  }
  if (boot_arg) {
    sys_strncpy(boot, boot_arg, MAX_PATH_LEN-1);
    boot[MAX_PATH_LEN-1] = 0;
    cdrom_to_cdfs(boot, MAX_PATH_LEN);
    add_slash(boot, MAX_PATH_LEN);
  } else {
    find_boot(argc > 0 ? argv[0] : NULL, boot, MAX_PATH_LEN);
  }

  // the options: pumpkin.cfg next to the ELF first, then the command line
  args[nargs++] = argc > 0 ? argv[0] : "PUMPKINOS.ELF";
  i = PS2_CONFIG_MAX_ARGS - argc;
  if (i > 0) nargs += read_config(boot, &args[nargs], i, config, PS2_CONFIG_MAX_SIZE);
  for (i = 1; i < argc && nargs < PS2_CONFIG_MAX_ARGS; i++) {
    args[nargs++] = argv[i];
  }
  args[nargs] = NULL;
  argv = args;
  argc = nargs;

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-nodia")) {
      ps2_script_set("dia", 0);
    } else if (!strcmp(argv[i], "-nonet")) {
      net = 0;
    } else if (!strcmp(argv[i], "-seed")) {
      reseed = 1;
    } else if (!strcmp(argv[i], "-memcard")) {
      memcard = 1;
    } else if (i < argc-1 && !strcmp(argv[i], "-ramdisk")) {
      ramdisk_kb = sys_atoi(argv[++i]);
      if (ramdisk_kb < 512) ramdisk_kb = 512;
    } else if (!strcmp(argv[i], "-pal")) {
      tvmode = "PAL";
    } else if (!strcmp(argv[i], "-ntsc")) {
      tvmode = "NTSC";
    } else if (!strcmp(argv[i], "-progressive")) {
      progressive = 1;
    } else if (i < argc-1 && !strcmp(argv[i], "-app")) {
      ps2_script_set_launcher(argv[++i]);
    } else if (i < argc-1 && !strcmp(argv[i], "-boot")) {
      i++;  // already handled
    } else if (i < argc-1 && !strcmp(argv[i], "-home")) {
      home_arg = argv[++i];
    } else if (i < argc-1 && !strcmp(argv[i], "-vfs")) {
      vfs_arg = argv[++i];
    } else if (i < argc-1 && !strcmp(argv[i], "-log")) {
      log_arg = argv[++i];
    } else if (i < argc-1 && !strcmp(argv[i], "-d")) {
      // "-d <level>" or "-d <level>:<system>" (like pit_main)
      d = argv[++i];
      if ((p = strchr(d, ':')) != NULL) {
        debug_setsyslevel(p+1, sys_atoi(d));
      } else {
        level = sys_atoi(d);
      }
    } else if (i < argc-1 && argv[i][0] == '-' && ps2_script_set(argv[i]+1, sys_atoi(argv[i+1])) == 0) {
      i++;
    } else if (argv[i][0] == '-') {
      usage();
      return 1;
    }
    // anything else is ignored
  }

  sys_init();
  debug_setsyslevel(NULL, level);

  // the home: the RAM disk, unless -home or -memcard say otherwise
  home[0] = 0;
  if (home_arg) {
    sys_strncpy(home, home_arg, MAX_PATH_LEN-1);
    home[MAX_PATH_LEN-1] = 0;
    add_slash(home, MAX_PATH_LEN);
  } else if (memcard) {
    mc = select_memory_card(home, MAX_PATH_LEN);
    // (no card: the RAM disk, reported below)
  }
  if (home[0] == 0) {
    sys_snprintf(home, MAX_PATH_LEN-1, "%s:/%s/", PS2_RAMDISK, PS2_HOME_DIR);
  }
  if (is_ramdisk(home) && sys_ps2_ramdisk_mount(PS2_RAMDISK, (uint32_t)ramdisk_kb * 1024) != 0) {
    // (cannot happen short of a broken C library glue)
    sys_strncpy(home, boot, MAX_PATH_LEN-1);
  }

  // the log and stderr: next to the ELF when that is possible (a USB stick,
  // the host; never a disc), else in the home unless it is the RAM disk
  // (it would grow there without end, and be lost anyway), else stderr
  if (!is_cdrom(boot) && writable_dir(boot)) {
    sys_strncpy(work, boot, MAX_PATH_LEN-1);
  } else if (!is_ramdisk(home)) {
    sys_strncpy(work, home, MAX_PATH_LEN-1);
  } else {
    work[0] = 0;
  }
  if (log_arg) {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%s", log_arg);
  } else if (work[0]) {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%spumpkin.log", work);
  } else {
    sys_strncpy(logfile, "stderr", MAX_PATH_LEN-1);
  }
  errfile[0] = 0;
  if (work[0]) sys_snprintf(errfile, MAX_PATH_LEN-1, "%spumpkin.err", work);

  // the home directory must exist before anything is written there
  sys_strncpy(path, home, MAX_PATH_LEN-1);
  i = sys_strlen(path);
  if (i > 1 && path[i-1] == '/') path[i-1] = 0;
  mkdir_if_missing(path);

  if (is_memory_card(logfile)) {
    // the log on a memory card: buffered, with stderr going to it as well
    // (the card driver has 3 file handles in all, see cardlog_write)
    sys_strncpy(cardlog_path, logfile, MAX_PATH_LEN-1);
    cardlog_last = sys_get_clock();
    if ((fd = open(cardlog_path, O_WRONLY | O_CREAT | O_TRUNC, 0644)) != -1) close(fd);
    stderr = funopen(NULL, NULL, cardlog_write, NULL, NULL);
    debug_init("stderr");
  } else {
    // there is no console: keep stderr in a file next to the log
    if (errfile[0] && sys_strcmp(logfile, "stderr")) freopen(errfile, "w", stderr);
    debug_init(logfile);
  }
  debug(DEBUG_INFO, "MAIN", "%s starting on %s (boot \"%s\"%s)", SYSTEM_NAME, SYSTEM_OS, boot, is_cdrom(boot) ? ", a disc" : "");
  if (argc > 0) debug(DEBUG_INFO, "MAIN", "argv[0] \"%s\"", argv[0]);
  for (i = 1; i < argc; i++) {
    debug(DEBUG_INFO, "MAIN", "option \"%s\"", argv[i]);
  }
  if (memcard && mc < 0 && !home_arg) {
    debug(DEBUG_ERROR, "MAIN", "no formatted PS2 memory card found: using the RAM disk");
  }
  if (mc >= 0) {
    debug(DEBUG_INFO, "MAIN", "home \"%s\" (memory card slot %d)", home, mc+1);
  } else if (is_ramdisk(home)) {
    debug(DEBUG_INFO, "MAIN", "home \"%s\" (RAM disk, %d KB at most, lost at power off)", home, ramdisk_kb);
  } else {
    debug(DEBUG_INFO, "MAIN", "home \"%s\"", home);
  }

  if (vfs_arg) {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%s", vfs_arg);
  } else {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%svfs", home);
  }
  add_slash(vfsdir, MAX_PATH_LEN);
  // the temporary files live in the home, next to vfs/
  sys_snprintf(tmpdir, MAX_PATH_LEN-1, "%stmp", home);

  if (!vfs_arg) {
    // first boot (every boot on the RAM disk): copy the VFS shipped next to
    // the ELF (the application PRCs in vfs/app_install) to the home; -seed
    // does it again, and
    // so does a home whose app_install and app_storage are both empty (an
    // install that failed: the PRCs are removed as they are deployed)
    sys_snprintf(seed, MAX_PATH_LEN-1, "%svfs", boot);
    sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_install", vfsdir);
    if (!reseed && dir_exists(path) && dir_empty(path)) {
      sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_storage", vfsdir);
      if (dir_empty(path)) {
        debug(DEBUG_INFO, "MAIN", "no applications installed: seeding again");
        reseed = 1;
      }
      sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_install", vfsdir);
    }
    if (sys_strcmp(seed, vfsdir) && sys_strncmp(seed, vfsdir, sys_strlen(seed)) && (reseed || !dir_exists(path))) {
      sys_snprintf(path, MAX_PATH_LEN-1, "%svfs", home);
      debug(DEBUG_INFO, "MAIN", "first boot: copying \"%s\" to \"%s\"", seed, path);
      if (dir_exists(seed)) {
        if (copy_tree(seed, path) != 0) {
          debug(DEBUG_ERROR, "MAIN", "the VFS copy failed (home full?)");
        }
      } else {
        debug(DEBUG_ERROR, "MAIN", "\"%s\" not found: nothing to install", seed);
      }
    }
    // make sure the writable parts exist
    sys_snprintf(path, MAX_PATH_LEN-1, "%svfs", home);
    mkdir_if_missing(path);
    sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_install", vfsdir);
    mkdir_if_missing(path);
    sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_storage", vfsdir);
    mkdir_if_missing(path);
    sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_card", vfsdir);
    mkdir_if_missing(path);
    sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_card/PALM", vfsdir);
    mkdir_if_missing(path);
    sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_card/PALM/Programs", vfsdir);
    mkdir_if_missing(path);
  }
  debug(DEBUG_INFO, "MAIN", "vfs \"%s\"", vfsdir);

  // the network adapter (fat consoles with the expansion bay adapter, slim
  // consoles with built-in ethernet), DHCP
  if (net) {
    sys_ps2_net_init(PS2_NET_TIMEOUT_MS);
  } else {
    debug(DEBUG_INFO, "MAIN", "network disabled");
  }

  sys_ps2_memlog("startup");
  ptr_init();
  thread_init();
  vfs_init();

  // the GS frame buffer follows the window size; the TV mode defaults to the
  // console's region (NTSC 640x448 / PAL 640x512 are the usual full frames)
  sys_snprintf(path, MAX_PATH_LEN-1, "%d", ps2_script_get("width"));
  SDL_SetHint(SDL_HINT_PS2_GS_WIDTH, path);
  sys_snprintf(path, MAX_PATH_LEN-1, "%d", ps2_script_get("height"));
  SDL_SetHint(SDL_HINT_PS2_GS_HEIGHT, path);
  if (tvmode) SDL_SetHint(SDL_HINT_PS2_GS_MODE, tvmode);
  if (progressive) SDL_SetHint(SDL_HINT_PS2_GS_PROGRESSIVE, "1");

  if (liblsdl3_load() != 0) {
    debug(DEBUG_ERROR, "MAIN", "SDL3 initialization failed");
    vfs_finish();
    thread_close();
    debug_close();
    fprintf(stderr, "SDL3 initialization failed, see %s\n", logfile);
    fflush(stderr);
    cardlog_flush();
    sys_ps2_net_finish();
    return 1;
  }
  liblsdl3_init(0, 0);
  sys_ps2_memlog("SDL loaded");

  vfs_local_mount(vfsdir, "/");
  mkdir_if_missing(tmpdir);
  sys_set_tmpdir(tmpdir);

#ifndef NO_HTTPS
  // OpenSSL looks for the CA bundle in SSL_CERT_FILE: use cert.pem next to
  // the ELF, or in the home, when it exists
  sys_snprintf(path, MAX_PATH_LEN-1, "%scert.pem", boot);
  if (sys_stat(path, &st) != 0) {
    sys_snprintf(path, MAX_PATH_LEN-1, "%scert.pem", home);
  }
  if (sys_stat(path, &st) == 0) {
    debug(DEBUG_INFO, "MAIN", "CA bundle \"%s\"", path);
    setenv("SSL_CERT_FILE", path, 0);
  }
  debug(DEBUG_INFO, "MAIN", "loading the TLS provider");
  liblopenssl_load();
  liblopenssl_init(0, 0);
  debug(DEBUG_INFO, "MAIN", "TLS provider loaded (OpenSSL)");
#endif
  sys_ps2_memlog("TLS provider loaded");

  libos_app_init(0);
  libos_start(0);

  vfs_finish();
  thread_close();
  liblsdl3_unload();
  debug(DEBUG_INFO, "MAIN", "%s stopping", SYSTEM_NAME);
  debug_close();
  cardlog_flush();

  sys_ps2_net_finish();

  return 0;
}
