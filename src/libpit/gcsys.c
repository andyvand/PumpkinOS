// System helpers for the Nintendo GameCube build (libogc).
//
// C heap usage, for the log: the newlib heap is the libogc arena, growing
// from its low end (sbrk) towards its high end (SYS_GetArena1Hi, which the
// frame buffers and other SYS_AllocArenaMemHi() users pull down); what is
// between is what malloc() can still get. The console has 24MB in all, see
// the heap sizing in libpumpkin/pumpkin.c.

#include <stdint.h>
#include <unistd.h>
#include <malloc.h>

#include <ogc/system.h>

#include "sys.h"
#include "debug.h"

void sys_gc_memlog(const char *tag) {
  struct mallinfo mi = mallinfo();
  uint32_t avail = (uint32_t)((uint8_t *)SYS_GetArena1Hi() - (uint8_t *)sbrk(0));

  debug(DEBUG_INFO, "SYS", "memory at %s: %u KB left for sbrk, malloc arena %u KB (%u KB in use, %u KB free)", tag, avail / 1024, (unsigned)mi.arena / 1024, (unsigned)mi.uordblks / 1024, (unsigned)mi.fordblks / 1024);
}
