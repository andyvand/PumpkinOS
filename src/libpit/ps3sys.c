// System helpers for the Sony PlayStation 3 build (PSL1GHT).
//
// The memory log: the user memory the LV2 kernel grants the process (the
// program, the newlib heap, which librt's sbrk() maps in 64KB... 1MB
// chunks, the thread stacks) out of the console's 256MB of main memory.

#include <stdint.h>
#include <malloc.h>

#include <ppu-lv2.h>

#include "sys.h"
#include "debug.h"

typedef struct {
  uint32_t total;
  uint32_t avail;
} ps3_memory_info_t;

// sys_memory_get_user_memory_size (LV2 syscall 352)
static int32_t ps3_memory_info(ps3_memory_info_t *info) {
  lv2syscall1(352, (uint64_t)info);
  return_to_user_prog(int32_t);
}

uint32_t sys_ps3_mem_avail(void) {
  ps3_memory_info_t info;

  if (ps3_memory_info(&info) != 0) return 0;
  return info.avail;
}

void sys_ps3_memlog(const char *tag) {
  struct mallinfo mi = mallinfo();
  ps3_memory_info_t info;

  if (ps3_memory_info(&info) != 0) {
    info.total = info.avail = 0;
  }
  debug(DEBUG_INFO, "SYS", "memory at %s: %u KB of %u KB user memory left, malloc arena %u KB (%u KB in use, %u KB free)", tag, info.avail / 1024, info.total / 1024, (unsigned)mi.arena / 1024, (unsigned)mi.uordblks / 1024, (unsigned)mi.fordblks / 1024);
}
