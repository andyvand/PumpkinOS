// SceNet (TCP/IP) initialization for the PlayStation Vita build.
//
// The vitasdk newlib implements the BSD socket API on top of SceNet and
// initializes it lazily (loading the SceNet module, allocating its memory
// pool, sceNetInit + sceNetCtlInit) the first time socket() is called. Doing
// it explicitly at startup reports the network state in the log before any
// application needs it.

#include <psp2/net/net.h>
#include <psp2/net/netctl.h>

#include "sys.h"
#include "debug.h"

// newlib's lazy initializer (idempotent)
extern int _vita_net_init(void);

static int net_ready = 0;

int sys_vita_net_init(void) {
  SceNetCtlInfo info;
  int r;

  if ((r = _vita_net_init()) < 0) {
    debug(DEBUG_ERROR, "SYS", "SceNet initialization failed (0x%08X), network unavailable", r);
    return -1;
  }

  net_ready = 1;
  sys_memset(&info, 0, sizeof(info));
  if (sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info) == 0 && info.ip_address[0]) {
    debug(DEBUG_INFO, "SYS", "SceNet initialized, IP address %s", info.ip_address);
  } else {
    debug(DEBUG_INFO, "SYS", "SceNet initialized, no network connection");
  }

  return 0;
}

int sys_vita_net_ready(void) {
  return net_ready;
}
