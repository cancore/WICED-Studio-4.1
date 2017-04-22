/* libc/net/if_indextoname.c */

#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>

/*
 * Map an interface index into its name.
 * Returns NULL on error.
 */
char*
if_indextoname(unsigned ifindex, char *ifname)
  {
    int ctl_sock;
    struct ifreq ifr;
    char*  ret = NULL;

    memset(&ifr, 0, sizeof(struct ifreq));
    ifr.ifr_ifindex = ifindex;

    if ((ctl_sock = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
        if (ioctl(ctl_sock, SIOCGIFNAME, &ifr) >= 0) {
            ret = strncpy (ifname, ifr.ifr_name, IFNAMSIZ);
        } else {
            /* Posix requires ENXIO */
            if (errno == ENODEV)
                errno = ENXIO;
        }
        close(ctl_sock);
    }
    return ret;
  }

