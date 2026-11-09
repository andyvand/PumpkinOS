// PumpkinOS entry point for the Sony PlayStation 2 (ps2sdk + SDL3).
//
// Mirrors src/psp/main.c: everything is linked statically, there is no
// script engine and the OS runs in single application mode (the Launcher and
// the applications run in-process, as cooperative threads).
//
// Layout: the ELF is started from wherever a loader finds it (a USB stick,
// "mass:/PumpkinOS/PUMPKINOS.ELF", a memory card, the host under PCSX2 or
// ps2link, a disc...). The VFS lives on a memory card: on the first boot
// the vfs/ directory shipped next to the ELF is copied to
// mc0:/PumpkinOS/vfs/ (the first slot holding a formatted PS2 memory card;
// mc1: when the first one is empty or a PS1 card), with the temporary files
// (tmp/) next to it. The log (pumpkin.log) and stderr (pumpkin.err) go next
// to the ELF when that place is writable, else to the memory card as well.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

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

// the PumpkinOS directory on the memory card
#define PS2_HOME_DIR "PumpkinOS"

// where to look for the ELF (and its vfs/) when the loader gives no path
#define PS2_DEFAULT_BOOT "mass:/PumpkinOS/"

// how long to wait for the link and then for a DHCP lease at startup
#define PS2_NET_TIMEOUT_MS 10000

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
  fprintf(stderr, "  -home <dir>    PumpkinOS home (default: PumpkinOS/ on the first formatted PS2 memory card)\n");
  fprintf(stderr, "  -vfs <dir>     VFS root directory (default <home>/vfs)\n");
  fprintf(stderr, "  -log <file>    log file (default pumpkin.log next to the ELF, or in <home>)\n");
  fprintf(stderr, "  -seed          copy vfs/ from the boot directory again (overwrites the application PRCs)\n");
  fprintf(stderr, "  -nonet         do not bring the network up\n");
  fprintf(stderr, "  -pal | -ntsc   force the TV mode (default: the console's region)\n");
  fprintf(stderr, "  -progressive   480p output (component cable)\n");
  fprintf(stderr, "  -d <level>     log level: 0 = errors, 1 = info (default), 2 = trace; <level>:<system> for one system\n");
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
  add_slash(boot, len);
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

// recursive copy of a directory tree (the initial VFS to the memory card)
static int copy_tree(const char *src, const char *dst) {
  char name[FILE_PATH], s[MAX_PATH_LEN], d[MAX_PATH_LEN];
  sys_dir_t *dir;
  sys_stat_t st;
  int r = 0;

  if (!dir_exists(dst) && sys_mkdir(dst) != 0) {
    debug(DEBUG_ERROR, "MAIN", "could not create directory \"%s\"", dst);
    return -1;
  }

  if ((dir = sys_opendir(src)) == NULL) {
    debug(DEBUG_ERROR, "MAIN", "could not open directory \"%s\"", src);
    return -1;
  }

  while (sys_readdir(dir, name, sizeof(name)) == 0) {
    if (!strcmp(name, ".") || !strcmp(name, "..")) continue;
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
  sys_closedir(dir);

  return r;
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
  const char *vfs_arg = NULL, *log_arg = NULL, *boot_arg = NULL, *home_arg = NULL, *tvmode = NULL;
  char *d, *p;
  sys_stat_t st;
  int i, level = DEBUG_INFO, net = 1, reseed = 0, progressive = 0, mc = -1;

  init_iop();

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-nodia")) {
      ps2_script_set("dia", 0);
    } else if (!strcmp(argv[i], "-nonet")) {
      net = 0;
    } else if (!strcmp(argv[i], "-seed")) {
      reseed = 1;
    } else if (!strcmp(argv[i], "-pal")) {
      tvmode = "PAL";
    } else if (!strcmp(argv[i], "-ntsc")) {
      tvmode = "NTSC";
    } else if (!strcmp(argv[i], "-progressive")) {
      progressive = 1;
    } else if (i < argc-1 && !strcmp(argv[i], "-app")) {
      ps2_script_set_launcher(argv[++i]);
    } else if (i < argc-1 && !strcmp(argv[i], "-boot")) {
      boot_arg = argv[++i];
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

  // where the ELF (and the vfs/ to seed from) is
  if (boot_arg) {
    sys_strncpy(boot, boot_arg, MAX_PATH_LEN-1);
    boot[MAX_PATH_LEN-1] = 0;
    add_slash(boot, MAX_PATH_LEN);
  } else {
    find_boot(argc > 0 ? argv[0] : NULL, boot, MAX_PATH_LEN);
  }

  // the log and stderr go next to the ELF when that is possible (a USB
  // stick, the host), else to the home on the memory card
  if (writable_dir(boot)) {
    sys_strncpy(work, boot, MAX_PATH_LEN-1);
  } else {
    work[0] = 0;
  }

  sys_init();
  debug_setsyslevel(NULL, level);

  // the home: PumpkinOS/ on a memory card
  if (home_arg) {
    sys_strncpy(home, home_arg, MAX_PATH_LEN-1);
    home[MAX_PATH_LEN-1] = 0;
    add_slash(home, MAX_PATH_LEN);
  } else {
    home[0] = 0;
  }
  if (work[0] == 0) {
    // nothing writable yet: the log must wait for the memory card
    if (home[0] == 0) mc = select_memory_card(home, MAX_PATH_LEN);
    if (home[0] == 0) {
      // no card at all: run from the boot directory and hope it is writable
      sys_strncpy(home, boot, MAX_PATH_LEN-1);
    }
    sys_strncpy(work, home, MAX_PATH_LEN-1);
  }

  if (log_arg) {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%s", log_arg);
  } else {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%spumpkin.log", work);
  }
  sys_snprintf(errfile, MAX_PATH_LEN-1, "%spumpkin.err", work);

  // the home directory must exist before anything is written there
  if (home[0]) {
    sys_strncpy(path, home, MAX_PATH_LEN-1);
    i = sys_strlen(path);
    if (i > 1 && path[i-1] == '/') path[i-1] = 0;
    mkdir_if_missing(path);
  }
  // there is no console: keep stderr in a file next to the log
  freopen(errfile, "w", stderr);
  debug_init(logfile);
  debug(DEBUG_INFO, "MAIN", "%s starting on %s (boot \"%s\")", SYSTEM_NAME, SYSTEM_OS, boot);
  if (argc > 0) debug(DEBUG_INFO, "MAIN", "argv[0] \"%s\"", argv[0]);

  if (home[0] == 0) {
    mc = select_memory_card(home, MAX_PATH_LEN);
    if (home[0] == 0) {
      debug(DEBUG_ERROR, "MAIN", "no formatted PS2 memory card found: using the boot directory as home");
      sys_strncpy(home, boot, MAX_PATH_LEN-1);
    }
    sys_strncpy(path, home, MAX_PATH_LEN-1);
    i = sys_strlen(path);
    if (i > 1 && path[i-1] == '/') path[i-1] = 0;
    mkdir_if_missing(path);
  }
  if (mc >= 0) {
    debug(DEBUG_INFO, "MAIN", "home \"%s\" (memory card slot %d)", home, mc+1);
  } else {
    debug(DEBUG_INFO, "MAIN", "home \"%s\"", home);
  }

  if (vfs_arg) {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%s", vfs_arg);
  } else {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%svfs", home);
  }
  add_slash(vfsdir, MAX_PATH_LEN);
  // the temporary files live on the memory card, next to vfs/
  sys_snprintf(tmpdir, MAX_PATH_LEN-1, "%stmp", home);

  if (!vfs_arg) {
    // first boot: copy the VFS shipped next to the ELF (the application
    // PRCs in vfs/app_install) to the memory card; -seed does it again
    sys_snprintf(seed, MAX_PATH_LEN-1, "%svfs", boot);
    sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_install", vfsdir);
    if (sys_strcmp(seed, vfsdir) && sys_strncmp(seed, vfsdir, sys_strlen(seed)) && (reseed || !dir_exists(path))) {
      sys_snprintf(path, MAX_PATH_LEN-1, "%svfs", home);
      debug(DEBUG_INFO, "MAIN", "first boot: copying \"%s\" to \"%s\"", seed, path);
      if (dir_exists(seed)) {
        if (copy_tree(seed, path) != 0) {
          debug(DEBUG_ERROR, "MAIN", "the VFS copy failed (memory card full?)");
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

  sys_ps2_net_finish();

  return 0;
}
