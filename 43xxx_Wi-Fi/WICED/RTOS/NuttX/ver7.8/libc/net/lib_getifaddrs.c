/* lib_getifaddrs.c */

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>

#include "ifaddrs.h"


static struct sockaddr *sockaddr_dup(struct sockaddr *sa)
{
  struct sockaddr *ret;
  socklen_t socklen;
  socklen = sizeof(struct sockaddr_storage);
  ret = calloc(1, socklen);
  if (ret)
    {
      ret->sa_family = sa->sa_family;
      if (sa->sa_family == AF_INET)
        {
          struct sockaddr_in *s4 = (struct sockaddr_in *) sa;
          memcpy(ret, &s4->sin_addr.s_addr, sizeof(struct in_addr));
        }
      else
        {
          struct sockaddr_in6 *s6 = (struct sockaddr_in6 *) sa;
          memcpy(ret, &s6->sin6_addr.s6_addr, sizeof(struct in6_addr));
        }
    }
  return ret;
}

int
getifaddrs(struct ifaddrs **ifap)
{
    struct ifreq ifr;
#ifdef CONFIG_NET_IPv6
    struct lifreq lifr;
#endif
    int fd, i, n;
    struct ifaddrs *curif, *lastif = NULL;

    *ifap = NULL;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
      {
        return -1;
      }

    if (ioctl(fd, SIOCGIFCOUNT, &ifr) != 0)
      {
        close(fd);
        return -1;
      }

    n = ifr.ifr_count;

    /* Loop through interfaces, looking for given IP address */
    for (i = n; i > 0; i--)
    {
      ifr.ifr_ifindex = i;
      if (ioctl(fd, SIOCGIFNAME, &ifr) != 0)
        {
          freeifaddrs(*ifap);
          return -1;
        }
      if (ioctl(fd, SIOCGIFADDR, &ifr) != 0)
        {
          freeifaddrs(*ifap);
          return -1;
        }

      curif = calloc(1, sizeof(struct ifaddrs));
      if (lastif == NULL)
        {
          *ifap = curif;
        }
      else
        {
          lastif->ifa_next = curif;
        }

      curif->ifa_name = strdup(ifr.ifr_name);
      curif->ifa_addr = sockaddr_dup(&ifr.ifr_addr);
      curif->ifa_dstaddr = NULL;
      curif->ifa_data = NULL;
      curif->ifa_next = NULL;
      curif->ifa_netmask = NULL;

      if (ioctl(fd, SIOCGIFFLAGS, &ifr) != 0)
        {
          freeifaddrs(*ifap);
          return -1;
        }

      curif->ifa_flags = ifr.ifr_flags;

      if (ioctl(fd, SIOCGIFNETMASK, &ifr) != 0)
        {
          freeifaddrs(*ifap);
          return -1;
        }

      curif->ifa_netmask = sockaddr_dup(&ifr.ifr_addr);
      lastif = curif;

#ifdef CONFIG_NET_IPv6
      memcpy(lifr.lifr_name, ifr.ifr_name, IFNAMSIZ);
      if (ioctl(fd, SIOCGLIFADDR, &lifr) != 0)
        {
          freeifaddrs(*ifap);
          return -1;
        }

      curif = calloc(1, sizeof(struct ifaddrs));
      if (lastif == NULL)
        {
          *ifap = curif;
        }
      else
        {
          lastif->ifa_next = curif;
        }

      curif->ifa_name = strdup(lifr.lifr_name);
      curif->ifa_addr = sockaddr_dup((struct sockaddr *)&lifr.lifr_addr);
      curif->ifa_dstaddr = NULL;
      curif->ifa_data = NULL;
      curif->ifa_next = NULL;
      curif->ifa_netmask = NULL;

      if (ioctl(fd, SIOCGIFFLAGS, &lifr) != 0)
        {
          freeifaddrs(*ifap);
          return -1;
        }

      curif->ifa_flags = lifr.lifr_flags;

      if (ioctl(fd, SIOCGLIFNETMASK, &lifr) != 0)
        {
          freeifaddrs(*ifap);
          return -1;
        }

      curif->ifa_netmask = sockaddr_dup((struct sockaddr *)&lifr.lifr_addr);
      lastif = curif;

#endif
    }

    close(fd);

    return 0;
}

void
freeifaddrs(struct ifaddrs *ifp)
{
  struct ifaddrs *p, *q;

  for(p = ifp; p; )
    {
      free(p->ifa_name);
      free(p->ifa_addr);
      free(p->ifa_dstaddr);
      free(p->ifa_netmask);
      free(p->ifa_data);
      q = p;
      p = p->ifa_next;
      free(q);
    }
}


