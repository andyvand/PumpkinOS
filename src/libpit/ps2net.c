// Network initialization for the PlayStation 2 build.
//
// The ps2sdk newlib glue implements the BSD socket API (socket/connect/
// select/getaddrinfo/...) on top of a network stack the program has to bring
// up itself. ps2_drivers (ps2sdk-ports) carries the pieces: the network
// adapter IOP modules (dev9, netman, smap: the expansion bay adapter of the
// fat models or the built-in ethernet of the slim ones) and lwIP running on
// the EE (the "eeip" driver), whose socket operations it plugs into the glue.
// Done once at startup, bounded by timeouts so that a console without an
// adapter or a cable still boots quickly (offline).
//
// The glue's netdb.h declares getaddrinfo() but nothing defines
// gai_strerror(): a small one lives here as well.

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <ps2_network_driver.h>

#include "sys.h"
#include "debug.h"

static int net_ready = 0;
static int driver_loaded = 0;

static const char *progress_name(enum EEIP_PROGRESS_EVENT ev) {
  switch (ev) {
    case EEIP_PROGRESS_SETTING_LINK_MODE:  return "setting link mode";
    case EEIP_PROGRESS_TCPIP_INIT:         return "initializing TCP/IP";
    case EEIP_PROGRESS_APPLYING_IP_CONFIG: return "applying IP configuration";
    case EEIP_PROGRESS_WAITING_LINK_UP:    return "waiting for the link";
    case EEIP_PROGRESS_LINK_UP:            return "link up";
    case EEIP_PROGRESS_WAITING_DHCP:       return "waiting for DHCP";
    case EEIP_PROGRESS_DHCP_BOUND:         return "DHCP lease obtained";
    case EEIP_PROGRESS_READY:              return "ready";
  }
  return "?";
}

static void progress(enum EEIP_PROGRESS_EVENT ev, void *user) {
  debug(DEBUG_INFO, "SYS", "network: %s", progress_name(ev));
}

static const char *status_name(enum EEIP_NET_STATUS st) {
  switch (st) {
    case EEIP_NET_STATUS_OK:              return "ok";
    case EEIP_NET_STATUS_LINK_MODE_ERROR: return "link mode error";
    case EEIP_NET_STATUS_LINK_TIMEOUT:    return "no link (cable unplugged?)";
    case EEIP_NET_STATUS_DHCP_TIMEOUT:    return "no DHCP answer";
    case EEIP_NET_STATUS_CONFIG_ERROR:    return "configuration error";
  }
  return "unknown error";
}

int sys_ps2_net_init(int timeout_ms) {
  eeip_network_config_t cfg;
  struct ip4_addr ip, nm, gw;
  enum EEIP_INIT_STATUS is;
  enum EEIP_NET_STATUS st;
  char sip[20], snm[20], sgw[20];

  if (net_ready) return 0;

  // loads dev9, netman and smap (init_dependencies) and registers lwIP's
  // socket operations with the newlib glue
  if ((is = init_network_driver(true)) != EEIP_INIT_STATUS_OK) {
    debug(DEBUG_ERROR, "SYS", "network adapter initialization failed (%d), network unavailable", (int)is);
    return -1;
  }
  driver_loaded = 1;

  eeip_network_config_default_dhcp(&cfg);
  cfg.timeout_seconds = timeout_ms > 0 ? (timeout_ms + 999) / 1000 : 0;  // per phase (link, DHCP); 0 = driver default
  cfg.on_progress = progress;
  cfg.user = NULL;

  if ((st = configure_eeip_network(&cfg)) != EEIP_NET_STATUS_OK) {
    debug(DEBUG_ERROR, "SYS", "network configuration failed: %s (%d)", status_name(st), (int)st);
    return -1;
  }

  if (eeip_get_current_config(&ip, &nm, &gw) == 0) {
    sys_snprintf(sip, sizeof(sip)-1, "%d.%d.%d.%d", ip4_addr1(&ip), ip4_addr2(&ip), ip4_addr3(&ip), ip4_addr4(&ip));
    sys_snprintf(snm, sizeof(snm)-1, "%d.%d.%d.%d", ip4_addr1(&nm), ip4_addr2(&nm), ip4_addr3(&nm), ip4_addr4(&nm));
    sys_snprintf(sgw, sizeof(sgw)-1, "%d.%d.%d.%d", ip4_addr1(&gw), ip4_addr2(&gw), ip4_addr3(&gw), ip4_addr4(&gw));
    debug(DEBUG_INFO, "SYS", "network up: address %s netmask %s gateway %s", sip, snm, sgw);
  } else {
    debug(DEBUG_INFO, "SYS", "network up");
  }
  net_ready = 1;

  return 0;
}

int sys_ps2_net_ready(void) {
  return net_ready;
}

void sys_ps2_net_finish(void) {
  if (driver_loaded) {
    deinit_network_driver(true);
    driver_loaded = 0;
  }
  net_ready = 0;
}

const char *gai_strerror(int errcode) {
  switch (errcode) {
    case EAI_NONAME:  return "name or service not known";
    case EAI_SERVICE: return "service not supported";
    case EAI_FAIL:    return "non-recoverable failure in name resolution";
    case EAI_MEMORY:  return "memory allocation failure";
    case EAI_FAMILY:  return "address family not supported";
  }
  return "unknown resolver error";
}
