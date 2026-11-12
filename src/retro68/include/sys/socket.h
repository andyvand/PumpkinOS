// BSD sockets for classic Mac OS (Retro68): the C library has none.
// libpit/macnet.c implements them on top of MacTCP (or the MacTCP
// compatibility of Open Transport): TCP streams and UDP, IPv4 only. The
// descriptors are shared with the files of libpit/macsys.c.

#ifndef MACCLASSIC_SYS_SOCKET_H
#define MACCLASSIC_SYS_SOCKET_H

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t socklen_t;
typedef uint8_t sa_family_t;

#define AF_UNSPEC 0
#define AF_UNIX   1
#define AF_INET   2
#define AF_INET6  30
#define PF_UNSPEC AF_UNSPEC
#define PF_INET   AF_INET
#define PF_INET6  AF_INET6

#define SOCK_STREAM 1
#define SOCK_DGRAM  2
#define SOCK_RAW    3

#define SOL_SOCKET  0xFFFF

#define SO_DEBUG      0x0001
#define SO_ACCEPTCONN 0x0002
#define SO_REUSEADDR  0x0004
#define SO_KEEPALIVE  0x0008
#define SO_BROADCAST  0x0020
#define SO_LINGER     0x0080
#define SO_SNDBUF     0x1001
#define SO_RCVBUF     0x1002
#define SO_SNDTIMEO   0x1005
#define SO_RCVTIMEO   0x1006
#define SO_ERROR      0x1007
#define SO_TYPE       0x1008

#define MSG_OOB      0x01
#define MSG_PEEK     0x02
#define MSG_DONTWAIT 0x80
#define MSG_NOSIGNAL 0x00

#define SHUT_RD   0
#define SHUT_WR   1
#define SHUT_RDWR 2

#define SOMAXCONN 8

struct sockaddr {
  uint8_t sa_len;
  sa_family_t sa_family;
  char sa_data[14];
};

struct sockaddr_storage {
  uint8_t ss_len;
  sa_family_t ss_family;
  char ss_data[126];
};

struct linger {
  int l_onoff;
  int l_linger;
};

int socket(int domain, int type, int protocol);
int socketpair(int domain, int type, int protocol, int sv[2]);
int connect(int sock, const struct sockaddr *addr, socklen_t len);
int bind(int sock, const struct sockaddr *addr, socklen_t len);
int listen(int sock, int backlog);
int accept(int sock, struct sockaddr *addr, socklen_t *len);
ssize_t send(int sock, const void *buf, size_t len, int flags);
ssize_t recv(int sock, void *buf, size_t len, int flags);
ssize_t sendto(int sock, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);
ssize_t recvfrom(int sock, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
int shutdown(int sock, int how);
int setsockopt(int sock, int level, int name, const void *value, socklen_t len);
int getsockopt(int sock, int level, int name, void *value, socklen_t *len);
int getsockname(int sock, struct sockaddr *addr, socklen_t *len);
int getpeername(int sock, struct sockaddr *addr, socklen_t *len);

#ifdef __cplusplus
}
#endif

#endif
