// System helpers for the Nintendo Wii U build (wut).
//
// The network: wut's socket layer (nsysnet through newlib descriptors, with
// select(), poll(), getaddrinfo() and fcntl(O_NONBLOCK)) starts the
// connection by itself (ACConnectAsync() in its initializer, using the
// console's default connection settings); sys_wiiu_net_init() only waits
// for it to come up, so that the first HTTP request does not fail, and logs
// the address.
//
// The memory log: wut's malloc() allocates from the default heap, the MEM2
// expanded heap (about 800MB for an application).

#include <stdint.h>
#include <malloc.h>

#include <coreinit/memheap.h>
#include <coreinit/memexpheap.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <nn/ac.h>

#include "sys.h"
#include "debug.h"

static int net_initialized = 0;
static int net_ready = 0;

static void wiiu_log_address(void) {
  uint32_t ip = 0;

  if (NNResult_IsSuccess(ACGetAssignedAddress(&ip)) && ip != 0) {
    debug(DEBUG_INFO, "SYS", "network up, address %u.%u.%u.%u", (ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff);
  } else {
    debug(DEBUG_INFO, "SYS", "network up");
  }
}

int sys_wiiu_net_init(int timeout_ms) {
  BOOL connected = FALSE;
  int waited;

  if (!net_initialized) {
    if (NNResult_IsFailure(ACInitialize())) {
      debug(DEBUG_ERROR, "SYS", "ACInitialize failed");
      return -1;
    }
    net_initialized = 1;
  }

  // wut's socket initializer has already asked for the connection
  // (ACConnectAsync); asking again is harmless when it is up
  if (NNResult_IsFailure(ACIsApplicationConnected(&connected)) || !connected) {
    ACConnectAsync();
  }

  for (waited = 0; ; waited += 100) {
    connected = FALSE;
    if (NNResult_IsSuccess(ACIsApplicationConnected(&connected)) && connected) {
      net_ready = 1;
      wiiu_log_address();
      return 0;
    }
    if (waited >= timeout_ms) break;
    OSSleepTicks(OSMillisecondsToTicks(100));
  }

  debug(DEBUG_ERROR, "SYS", "no network connection after %d ms (it may still come up later)", timeout_ms);
  return -1;
}

int sys_wiiu_net_ready(void) {
  BOOL connected = FALSE;

  if (!net_ready && net_initialized && NNResult_IsSuccess(ACIsApplicationConnected(&connected)) && connected) {
    net_ready = 1;
    wiiu_log_address();
  }

  return net_ready;
}

void sys_wiiu_net_finish(void) {
  if (net_initialized) {
    // (the connection itself belongs to wut's socket layer, which closes it
    // at exit)
    ACFinalize();
    net_initialized = 0;
    net_ready = 0;
  }
}

uint32_t sys_wiiu_mem_avail(void) {
  MEMHeapHandle heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM2);

  return heap ? MEMGetTotalFreeSizeForExpHeap(heap) : 0;
}

void sys_wiiu_memlog(const char *tag) {
  MEMHeapHandle heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM2);
  struct mallinfo mi = mallinfo();
  uint32_t total = 0, largest = 0;

  if (heap) {
    total = MEMGetTotalFreeSizeForExpHeap(heap);
    largest = MEMGetAllocatableSizeForExpHeapEx(heap, 4);
  }
  // (the MEM2 numbers stay 0 when wut has handed that heap to newlib's
  // sbrk, and under Cemu)
  debug(DEBUG_INFO, "SYS", "memory at %s: %u KB free in MEM2 (largest block %u KB), malloc arena %u KB (%u KB in use, %u KB free)", tag, total / 1024, largest / 1024, (unsigned)mi.arena / 1024, (unsigned)mi.uordblks / 1024, (unsigned)mi.fordblks / 1024);
}
