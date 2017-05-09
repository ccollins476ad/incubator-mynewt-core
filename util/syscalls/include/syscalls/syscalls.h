#ifndef H_UTIL_SYSCALLS_
#define H_UTIL_SYSCALLS_

#include <ifaddrs.h>
#include <inttypes.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
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
void *syscalls_malloc(size_t size);
int syscalls_fputs(const char *restrict s, FILE *restrict stream);
int syscalls_dprintf(int fd, const char *restrict format, ...);
ssize_t syscalls_read(int fildes, void *buf, size_t nbyte);
int syscalls_setitimer(int which, const struct itimerval *restrict value,
                       struct itimerval *restrict ovalue);

#define ctime(clock)                syscalls_ctime(clock)
#define socket(d, t, p)             syscalls_socket(d, t, p)
#define close(f)                    syscalls_close(f)
#define write(f, b, n)              syscalls_write(f, b, n)
#define sendto(s, b, l, f, d, e)    syscalls_sendto(s, b, l, f, d, e)
#define recvfrom(s, b, l, f, a, e)  syscalls_recvfrom(s, b, l, f, a, e)
#define getpeername(s, a, l)        syscalls_getpeername(s, a, l)
#define getsockname(s, a, l)        syscalls_getsockname(s, a, l)
#define setsockopt(s, l, n, v, e)   syscalls_setsockopt(s, l, n, v, e)
#define getsockopt(s, l, n, v, e)   syscalls_getsockopt(s, l, n, v, e)
#define poll(f, n, t)               syscalls_poll(f, n, t)
#define accept(s, a, l)             syscalls_accept(s, a, l)
#define getifaddrs(i)               syscalls_getifaddrs(i)
#define freeifaddrs(i)              syscalls_freeifaddrs(i)
#define if_nametoindex(i)           syscalls_if_nametoindex(i)
#define fputs(s, t)                 syscalls_fputs(s, t)
#define dprintf(f, ...)             syscalls_dprintf(f, __VA_ARGS__)
#define read(f, b, n)               syscalls_read(f, b, n)
#define setitimer(w, v, o)          syscalls_setitimer(w, v, o)

#undef  malloc
#define malloc(s)                   syscalls_malloc(s)

#endif
