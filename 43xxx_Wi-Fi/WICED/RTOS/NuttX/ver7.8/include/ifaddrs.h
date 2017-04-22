/* ifaddrs.h */

#ifndef __INCLUDE_IFADDRS_H_
#define __INCLUDE_IFADDRS_H_

#include <sys/socket.h>

struct ifaddrs {
    struct ifaddrs  *ifa_next;
    char        *ifa_name;
    unsigned int     ifa_flags;
    struct sockaddr *ifa_addr;
    struct sockaddr *ifa_netmask;
    struct sockaddr *ifa_dstaddr;
    void        *ifa_data;
};

#ifndef ifa_broadaddr
#define ifa_broadaddr   ifa_dstaddr /* broadcast address interface */
#endif

int getifaddrs(struct ifaddrs **);
void freeifaddrs(struct ifaddrs *);

#endif /* __INCLUDE_IFADDRS_H_ */
