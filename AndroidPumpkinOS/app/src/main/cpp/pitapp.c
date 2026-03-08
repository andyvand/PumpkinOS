#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <jni.h>

#include <PalmOS.h>

#include "sys.h"
#include "script.h"
#include "thread.h"
#include "pwindow.h"
#include "media.h"
#include "sys.h"
#include "ptr.h"
#include "vfs.h"
#include "vfslocal.h"
#include "gps.h"
#include "mutex.h"
#include "AppRegistry.h"
#include "storage.h"
#include "pumpkin.h"
#include "secure.h"
#include "endianness.h"
#include "debug.h"
#include "xalloc.h"
#include "pitapp.h"

extern int libos_app_init(int pe);
extern int libos_start(int pe);

int pitInit(void) {
  script_engine_t *engine;
  int pe = -1;

  debug_setsyslevel(NULL, DEBUG_INFO);
  sys_init();
  debug_init(NULL);
  ptr_init();
  thread_init();
  thread_setmain();

  debug(DEBUG_INFO, "MAIN", "%s starting on %s (%s endian)", SYSTEM_NAME, SYSTEM_OS, little_endian() ? "little" : "big");
  engine = script_load_engine("libscriptlua.so");
  vfs_init();
  //vfs_local_mount("/data/user/0/com.pit.pit/", "/");
  vfs_local_mount("/data/data/com.pit.pit/", "/");

  if (script_init(engine) != -1) {
    if ((pe = script_create(engine)) != -1) {
      window_init(pe);
      libos_app_init(pe);
      libos_start(pe);
    }
  }

  return pe;
}

void pitFinish(int pe) {
  sys_set_finish(0);
  sys_usleep(1000);
  thread_wait_all();

  if (pe != -1) {
    pumpkin_global_finish();
    script_destroy(pe);
  }
  script_finish(script_get_engine(pe));
  vfs_finish();

  thread_close();
  debug(DEBUG_INFO, "MAIN", "%s stopping", SYSTEM_NAME);
  debug_close();
}

void pitRequestFinish(void) {
  EventType event;
  debug(DEBUG_INFO, "MAIN", "pitRequestFinish");

  event.eType = MSG_KEY;
  event.data.keyDown.chr = WINDOW_KEY_HOME;
  event.data.keyDown.keyCode = 0;
  event.data.keyDown.modifiers = 0;

  pumpkin_forward_event(0, &event);
}

void pitDeploy(char *path) {
  //StoDeployFile(path);
  // XXX update Launcher
}

void pitUpdate(JNIEnv *env, jobject bitmap) {
  window_bitmap(env, bitmap);
}

void pitPause(int paused) {
  pumpkin_pause(paused);
}

void pitSetBattery(int level) {
  pumpkin_set_battery(level);
}
