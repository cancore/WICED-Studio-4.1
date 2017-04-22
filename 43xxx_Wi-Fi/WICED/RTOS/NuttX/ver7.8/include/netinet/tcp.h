/* include/netinet/tcp.h */
#ifndef __NETINET_TCP_H_
#define __NETINET_TCP_H_

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/*
 * User-settable options (used with setsockopt).
 */
#define TCP_NODELAY      1  /* Don't delay send to coalesce packets  */
#define TCP_MAXSEG       2  /* Set maximum segment size  */
#define TCP_CORK         3  /* Control sending of partial frames  */
#define TCP_KEEPIDLE     4  /* Start keeplives after this period */
#define TCP_KEEPINTVL    5  /* Interval between keepalives */
#define TCP_KEEPCNT      6  /* Number of keepalives before death */
#define TCP_SYNCNT       7  /* Number of SYN retransmits */
#define TCP_LINGER2      8  /* Life time of orphaned FIN-WAIT-2 state */
#define TCP_DEFER_ACCEPT 9  /* Wake up listener only when data arrive */
#define TCP_WINDOW_CLAMP 10 /* Bound advertised window */
#define TCP_INFO         11 /* Information about this connection. */
#define TCP_QUICKACK     12 /* Bock/reenable quick ACKs.  */
#define TCP_CONGESTION   13 /* Congestion control algorithm.  */
#define TCP_MD5SIG       14 /* TCP MD5 Signature (RFC2385) */

# define SOL_TCP        6   /* TCP level */

#endif /* __NETINET_TCP_H_ */
