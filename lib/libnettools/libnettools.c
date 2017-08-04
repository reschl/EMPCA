#define DFLT_AF "inet"

#include "libnettools/libnettools.h"
#include "libnettools/config.h"

#include <features.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

/* Ugh.  But libc5 doesn't provide POSIX types.  */
#include <asm/types.h>


#ifdef HAVE_HWSLIP
#include <linux/if_slip.h>
#endif

#if HAVE_AFINET6

#ifndef _LINUX_IN6_H
/*
 *    This is in linux/include/net/ipv6.h.
 */

struct in6_ifreq {
    struct in6_addr ifr6_addr;
    __u32 ifr6_prefixlen;
    unsigned int ifr6_ifindex;
};

#endif

#endif				/* HAVE_AFINET6 */

#if HAVE_AFIPX
#if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1)
#include <netipx/ipx.h>
#else
#include "ipx.h"
#endif
#endif
#include "libnettools/net-support.h"
#include "libnettools/pathnames.h"
#include "libnettools/version.h"
#include "libnettools/intl.h"
#include "libnettools/interface.h"
#include "libnettools/sockets.h"
#include "libnettools/util.h"

int set_flag(char *ifname, short flag)
{
    struct ifreq ifr;

    safe_strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
        fprintf(stderr, _("%s: unknown interface: %s\n"),
                ifname,	strerror(errno));
        return (-1);
    }
    safe_strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ifr.ifr_flags |= flag;
    if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) {
        perror("SIOCSIFFLAGS");
        return -1;
    }
    return (0);
}

int clr_flag(char *ifname, short flag)
{
    struct ifreq ifr;
    int fd;

    if (strchr(ifname, ':')) {
        /* This is a v4 alias interface.  Downing it via a socket for
       another AF may have bad consequences. */
        fd = get_socket_for_af(AF_INET);
        if (fd < 0) {
            fprintf(stderr, _("No support for INET on this system.\n"));
            return -1;
        }
    } else
        fd = skfd;

    safe_strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        fprintf(stderr, _("%s: unknown interface: %s\n"),
                ifname, strerror(errno));
        return -1;
    }
    safe_strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ifr.ifr_flags &= ~flag;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
        perror("SIOCSIFFLAGS");
        return -1;
    }
    return (0);
}

int if_up(char* ifname)
{
    return set_flag(ifname, (IFF_UP | IFF_RUNNING));
}

int if_promsic(char* ifname)
{
    return set_flag(ifname, IFF_PROMISC);
}

int if_down(char* ifname)
{
    return clr_flag(ifname, IFF_UP);
}

int if_init()
{
    if(skfd == -1) {
        if ((skfd = sockets_open(0)) < 0) {
            perror("socket");
            exit(1);
        }
    }
    return (0);
}

int if_close()
{
    close(skfd);
    return (0);
}
