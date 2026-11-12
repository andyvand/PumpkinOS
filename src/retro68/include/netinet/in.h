// IPv4 / IPv6 addresses for classic Mac OS (Retro68), see <sys/socket.h>.
// The Mac is big endian: the byte order conversions do nothing.

#ifndef MACCLASSIC_NETINET_IN_H
#define MACCLASSIC_NETINET_IN_H

#include <stdint.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t in_addr_t;
typedef uint16_t in_port_t;

struct in_addr {
  in_addr_t s_addr;
};

struct in6_addr {
  uint8_t s6_addr[16];
};

struct sockaddr_in {
  uint8_t sin_len;
  sa_family_t sin_family;
  in_port_t sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

// (no IPv6 transport: only lets the generic code compile)
struct sockaddr_in6 {
  uint8_t sin6_len;
  sa_family_t sin6_family;
  in_port_t sin6_port;
  uint32_t sin6_flowinfo;
  struct in6_addr sin6_addr;
  uint32_t sin6_scope_id;
};

#define IPPROTO_IP   0
#define IPPROTO_ICMP 1
#define IPPROTO_TCP  6
#define IPPROTO_UDP  17
#define IPPROTO_IPV6 41

#define IPV6_V6ONLY  27

#define INADDR_ANY       ((in_addr_t)0x00000000)
#define INADDR_BROADCAST ((in_addr_t)0xFFFFFFFF)
#define INADDR_NONE      ((in_addr_t)0xFFFFFFFF)
#define INADDR_LOOPBACK  ((in_addr_t)0x7F000001)

#define INET_ADDRSTRLEN  16
#define INET6_ADDRSTRLEN 46

extern const struct in6_addr in6addr_any;

#define htons(x) ((uint16_t)(x))
#define ntohs(x) ((uint16_t)(x))
#define htonl(x) ((uint32_t)(x))
#define ntohl(x) ((uint32_t)(x))

#ifdef __cplusplus
}
#endif

#endif
