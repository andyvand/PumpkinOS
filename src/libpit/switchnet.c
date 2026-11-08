// BSD socket service initialization for the Nintendo Switch build.
//
// libnx implements the BSD socket API (socket/connect/select/...) on top of
// the system's bsd service; it has to be initialized explicitly, with a
// transfer memory for the socket buffers, before any socket call succeeds
// (socket() fails with ENOSYS otherwise). Done once at startup, together
// with a log line telling whether the console is online.

// the umbrella header: socket.h relies on the ssl and nifm services being
// declared before it
#include <switch.h>

#include "sys.h"
#include "debug.h"

static int net_ready = 0;

int sys_switch_net_init(void) {
  Result rc;
  u32 ip = 0;

  if (R_FAILED(rc = socketInitializeDefault())) {
    debug(DEBUG_ERROR, "SYS", "socket service initialization failed (0x%08X), network unavailable", (unsigned int)rc);
    return -1;
  }
  net_ready = 1;

  if (R_SUCCEEDED(nifmInitialize(NifmServiceType_User))) {
    if (R_SUCCEEDED(nifmGetCurrentIpAddress(&ip)) && ip != 0) {
      debug(DEBUG_INFO, "SYS", "socket service initialized, IP address %u.%u.%u.%u",
        (unsigned int)(ip & 0xFF), (unsigned int)((ip >> 8) & 0xFF), (unsigned int)((ip >> 16) & 0xFF), (unsigned int)((ip >> 24) & 0xFF));
    } else {
      debug(DEBUG_INFO, "SYS", "socket service initialized, no network connection");
    }
    nifmExit();
  } else {
    debug(DEBUG_INFO, "SYS", "socket service initialized");
  }

  return 0;
}

int sys_switch_net_ready(void) {
  return net_ready;
}

void sys_switch_net_finish(void) {
  if (net_ready) {
    socketExit();
    net_ready = 0;
  }
}
