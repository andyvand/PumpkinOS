// Watt-32 TCP/IP initialization for the MS-DOS (DJGPP) build.
//
// Watt-32 installs DJGPP file system extensions for its sockets, so after
// sock_init() the regular read()/write()/close()/fcntl()/select() calls in
// sys.c work on socket descriptors. The library normally exits the program
// when no packet driver is loaded: _watt_do_exit turns that into an error
// code, the network is then simply unavailable (sockets fail to open).

#include <stdio.h>
#include <tcp.h>

#include "sys.h"
#include "debug.h"

extern int _watt_do_exit;

static int net_ready = 0;

int sys_dos_net_init(void) {
  int rc;

  _watt_do_exit = 0;
  if ((rc = sock_init()) != 0) {
    debug(DEBUG_ERROR, "SYS", "Watt-32 initialization failed (%d): %s", rc, sock_init_err(rc));
    debug(DEBUG_ERROR, "SYS", "network unavailable (is a packet driver loaded ?)");
    return -1;
  }

  net_ready = 1;
  debug(DEBUG_INFO, "SYS", "Watt-32 initialized, IP address %d.%d.%d.%d",
    (int)(my_ip_addr >> 24) & 0xff, (int)(my_ip_addr >> 16) & 0xff, (int)(my_ip_addr >> 8) & 0xff, (int)my_ip_addr & 0xff);
  return 0;
}

int sys_dos_net_ready(void) {
  return net_ready;
}
