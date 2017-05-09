#include <ifaddrs.h>
#include <inttypes.h>
#include <poll.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "os/os.h"

/* XXX: This is a horrible thing to do, but it is impossible to include the
 * appropriate header (net/if.h) here without causing a bunch of redefinition
 * conflicts for the queue macros.
 */
int if_nametoindex(const char *);

char *
syscalls_ctime(const time_t *clock)
{
    os_sr_t sr;
    char *val;

    OS_ENTER_CRITICAL(sr);
    val = ctime(clock);
    OS_EXIT_CRITICAL(sr);

    return val;
}

int
syscalls_socket(int domain, int type, int protocol)
{
    os_sr_t sr;
    int val;

    OS_ENTER_CRITICAL(sr);
    val = socket(domain, type, protocol);
    OS_EXIT_CRITICAL(sr);

    return val;
}

int
syscalls_close(int fildes)
{
    os_sr_t sr;
    int val;

    OS_ENTER_CRITICAL(sr);
    val = close(fildes);
    OS_EXIT_CRITICAL(sr);

    return val;
}

ssize_t
syscalls_write(int fildes, const void *buf, size_t nbyte)
{
    os_sr_t sr;
    ssize_t val;

    OS_ENTER_CRITICAL(sr);
    val = write(fildes, buf, nbyte);
    OS_EXIT_CRITICAL(sr);

    return val;
}

ssize_t
syscalls_sendto(int socket, const void *buffer, size_t length, int flags,
              const struct sockaddr *dest_addr, socklen_t dest_len)
{
    os_sr_t sr;
    ssize_t val;

    OS_ENTER_CRITICAL(sr);
    val = sendto(socket, buffer, length, flags, dest_addr, dest_len);
    OS_EXIT_CRITICAL(sr);

    return val;
}

ssize_t
syscalls_recvfrom(int socket, void *restrict buffer, size_t length,
                int flags, struct sockaddr *restrict address,
                socklen_t *restrict address_len)
{
    os_sr_t sr;
    ssize_t val;

    OS_ENTER_CRITICAL(sr);
    val = recvfrom(socket, buffer, length, flags, address, address_len);
    OS_EXIT_CRITICAL(sr);

    return val;
}

int
syscalls_getpeername(int socket, struct sockaddr *restrict address,
                   socklen_t *restrict address_len)
{
    os_sr_t sr;
    int val;

    OS_ENTER_CRITICAL(sr);
    val = getpeername(socket, address, address_len);
    OS_EXIT_CRITICAL(sr);

    return val;
}

int
syscalls_getsockname(int socket, struct sockaddr *restrict address,
                   socklen_t *restrict address_len)
{
    os_sr_t sr;
    int val;

    OS_ENTER_CRITICAL(sr);
    val = getsockname(socket, address, address_len);
    OS_EXIT_CRITICAL(sr);

    return val;
}

int
syscalls_setsockopt(int socket, int level, int option_name,
                  const void *option_value, socklen_t option_len)
{
    os_sr_t sr;
    int val;

    OS_ENTER_CRITICAL(sr);
    val = setsockopt(socket, level, option_name, option_value,
                            option_len);
    OS_EXIT_CRITICAL(sr);

    return val;
}

int
syscalls_getsockopt(int socket, int level, int option_name,
                    void *option_value, socklen_t *option_len)
{
    os_sr_t sr;
    int val;

    OS_ENTER_CRITICAL(sr);
    val = getsockopt(socket, level, option_name, option_value,
                     option_len);
    OS_EXIT_CRITICAL(sr);

    return val;
}

int
syscalls_poll(struct pollfd fds[], nfds_t nfds, int timeout)
{
    os_sr_t sr;
    int val;

    OS_ENTER_CRITICAL(sr);
    val = poll(fds, nfds, timeout);
    OS_EXIT_CRITICAL(sr);

    return val;
}

int
syscalls_accept(int socket, struct sockaddr *restrict address,
                socklen_t *restrict address_len)
{
    os_sr_t sr;
    int val;

    OS_ENTER_CRITICAL(sr);
    val = accept(socket, address, address_len);
    OS_EXIT_CRITICAL(sr);

    return val;
}

int
syscalls_getifaddrs(struct ifaddrs **ifap)
{
    os_sr_t sr;
    int val;

    OS_ENTER_CRITICAL(sr);
    val = getifaddrs(ifap);
    OS_EXIT_CRITICAL(sr);

    return val;
}

void
syscalls_freeifaddrs(struct ifaddrs *ifp)
{
    os_sr_t sr;

    OS_ENTER_CRITICAL(sr);
    freeifaddrs(ifp);
    OS_EXIT_CRITICAL(sr);
}

unsigned int
syscalls_if_nametoindex(const char *ifname)
{
    os_sr_t sr;
    unsigned int val;

    OS_ENTER_CRITICAL(sr);
    val = if_nametoindex(ifname);
    OS_EXIT_CRITICAL(sr);

    return val;
}
