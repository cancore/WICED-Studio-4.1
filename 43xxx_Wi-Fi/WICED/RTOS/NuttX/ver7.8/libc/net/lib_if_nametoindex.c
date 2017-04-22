/* libc/net/if_nametoindex.c */

 #include <string.h>
 #include <unistd.h>
 #include <net/if.h>
 #include <sys/socket.h>
 #include <sys/ioctl.h>
 #include <errno.h>

 /*
 * Map an interface name into its corresponding index.
 * Returns 0 on error, as 0 is not a valid index.
 */
unsigned int if_nametoindex(const char *ifname)
{
    int index;
    int ctl_sock;
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    index = 0;
    if ((ctl_sock = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
        if (ioctl(ctl_sock, SIOCGIFINDEX, &ifr) >= 0) {
            index = ifr.ifr_ifindex;
        }
        close(ctl_sock);
    }
    return index;
}

