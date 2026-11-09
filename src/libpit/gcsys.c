// System helpers for the Nintendo GameCube build (libogc).
//
// C heap usage, for the log: the newlib heap is the libogc arena, growing
// from its low end (sbrk) towards its high end (SYS_GetArena1Hi, which the
// frame buffers and other SYS_AllocArenaMemHi() users pull down); what is
// between is what malloc() can still get. The console has 24MB in all, see
// the heap sizing in libpumpkin/pumpkin.c. Also used by the Wii build.

#include <stdint.h>
#include <unistd.h>
#include <malloc.h>

#include <ogc/system.h>

#include "sys.h"
#include "debug.h"

#ifdef WII
// Nintendo Wii: the libogc sbrk() moves on to MEM2 (the arena between
// SYS_GetArena2Lo and SYS_GetArena2Hi, IOS keeps the top of it) once MEM1 is
// full, so what is left is the rest of the current arena, plus all of MEM2
// while the heap is still in MEM1.
#define GC_MEM2_BASE 0x90000000

uint32_t sys_gc_heap_avail(void) {
  uint8_t *brk = (uint8_t *)sbrk(0);
  uint32_t mem2 = (uint32_t)((uint8_t *)SYS_GetArena2Hi() - (uint8_t *)SYS_GetArena2Lo());

  if ((uint32_t)brk >= GC_MEM2_BASE) {
    return (uint32_t)((uint8_t *)SYS_GetArena2Hi() - brk);
  }
  return (uint32_t)((uint8_t *)SYS_GetArena1Hi() - brk) + mem2;
}
#else
uint32_t sys_gc_heap_avail(void) {
  return (uint32_t)((uint8_t *)SYS_GetArena1Hi() - (uint8_t *)sbrk(0));
}
#endif

void sys_gc_memlog(const char *tag) {
  struct mallinfo mi = mallinfo();
  uint32_t avail = sys_gc_heap_avail();

  debug(DEBUG_INFO, "SYS", "memory at %s: %u KB left for sbrk, malloc arena %u KB (%u KB in use, %u KB free)", tag, avail / 1024, (unsigned)mi.arena / 1024, (unsigned)mi.uordblks / 1024, (unsigned)mi.fordblks / 1024);
}
