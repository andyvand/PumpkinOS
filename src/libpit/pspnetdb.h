#ifndef PIT_PSPNETDB_H
#define PIT_PSPNETDB_H

// getaddrinfo() for the PlayStation Portable build (see pspnet.c).
//
// The pspsdk newlib glue only has gethostbyname() (on sceNetResolver); this
// is the subset of <netdb.h> that libpit/sys.c uses, implemented on top of
// it. IPv4 only, like the PSP network stack.

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// the PSP network headers stop at IPv4: these only let the generic code in
// sys.c compile (no IPv6 socket is ever created, see ipv4_only there)
#ifndef INADDR_NONE
#define INADDR_NONE ((in_addr_t)0xffffffff)
#endif

struct in6_addr {
  uint8_t s6_addr[16];
};

struct sockaddr_in6 {
  uint8_t sin6_len;
  sa_family_t sin6_family;
  in_port_t sin6_port;
  uint32_t sin6_flowinfo;
  struct in6_addr sin6_addr;
  uint32_t sin6_scope_id;
};

struct addrinfo {
  int ai_flags;
  int ai_family;
  int ai_socktype;
  int ai_protocol;
  socklen_t ai_addrlen;
  struct sockaddr *ai_addr;
  char *ai_canonname;
  struct addrinfo *ai_next;
};

#define AI_PASSIVE     0x01
#define AI_CANONNAME   0x02
#define AI_NUMERICHOST 0x04

#define EAI_AGAIN      2
#define EAI_FAIL       4
#define EAI_FAMILY     5
#define EAI_MEMORY     6
#define EAI_NONAME     8
#define EAI_SOCKTYPE   10

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);

void freeaddrinfo(struct addrinfo *res);

const char *gai_strerror(int errcode);

#endif
