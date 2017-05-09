#ifndef H_UTIL_SYSCALLS_
#define H_UTIL_SYSCALLS_

#include <ifaddrs.h>
#include <inttypes.h>
#include <poll.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

char *syscalls_ctime(const time_t *clock);
int syscalls_socket(int domain, int type, int protocol);
int syscalls_close(int fildes);
ssize_t syscalls_write(int fildes, const void *buf, size_t nbyte);
ssize_t syscalls_sendto(int socket, const void *buffer, size_t length,
                        int flags, const struct sockaddr *dest_addr,
                        socklen_t dest_len);
ssize_t syscalls_recvfrom(int socket, void *restrict buffer, size_t length,
                          int flags, struct sockaddr *restrict address,
                          socklen_t *restrict address_len);
int syscalls_getpeername(int socket, struct sockaddr *restrict address,
                         socklen_t *restrict address_len);
int syscalls_getsockname(int socket, struct sockaddr *restrict address,
                         socklen_t *restrict address_len);
int syscalls_setsockopt(int socket, int level, int option_name,
                        const void *option_value, socklen_t option_len);
int syscalls_getsockopt(int socket, int level, int option_name,
                        void *option_value, socklen_t *option_len);
int syscalls_poll(struct pollfd fds[], nfds_t nfds, int timeout);
int syscalls_accept(int socket, struct sockaddr *restrict address,
                     socklen_t *restrict address_len);
int syscalls_getifaddrs(struct ifaddrs **ifap);
void syscalls_freeifaddrs(struct ifaddrs *ifp);
unsigned int syscalls_if_nametoindex(const char *ifname);

#define ctime               syscalls_ctime
#define socket              syscalls_socket
#define close               syscalls_close
#define write               syscalls_write
#define sendto              syscalls_sendto
#define recvfrom            syscalls_recvfrom
#define getpeername         syscalls_getpeername
#define getsockname         syscalls_getsockname
#define setsockopt          syscalls_setsockopt
#define getsockopt          syscalls_getsockopt
#define poll                syscalls_poll
#define accept              syscalls_accept
#define getifaddrs          syscalls_getifaddrs
#define freeifaddrs         syscalls_freeifaddrs
#define if_nametoindex      syscalls_if_nametoindex

#endif
