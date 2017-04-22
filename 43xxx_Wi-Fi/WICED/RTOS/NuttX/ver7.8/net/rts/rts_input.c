/****************************************************************************
 * net/rts/rts_input.c
 * Handling incoming RTS input
 *
 *   Copyright (C) 2007-2009, 2011 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Adapted for NuttX from logic in uIP which also has a BSD-like license:
 *
 *   Original author Adam Dunkels <adam@dunkels.com>
 *   Copyright () 2001-2003, Adam Dunkels.
 *   All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#if defined(CONFIG_NET) && defined(CONFIG_NET_RTS)

#include <debug.h>

#include <nuttx/net/netconfig.h>
#include <nuttx/net/netdev.h>
#include <nuttx/net/rts.h>
#include <nuttx/net/netstats.h>

#include "devif/devif.h"
#include "utils/utils.h"
#include "rts/rts.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Variables
 ****************************************************************************/

/****************************************************************************
 * Private Variables
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: rts_input
 *
 * Description:
 *   Handle incoming RTS input in an IPv4 packet
 *
 * Parameters:
 *   dev - The device driver structure containing the received RTS packet
 *
 * Return:
 *   OK  The packet has been processed  and can be deleted
 *   ERROR Hold the packet and try again later. There is a listening socket
 *         but no receive in place to catch the packet yet.
 *
 * Assumptions:
 *   Called from network stack logic with the network stack locked
 *
 ****************************************************************************/

int rts_input(FAR struct net_driver_s *dev)
{
    
#if 0
  FAR struct rts_hdr_s *rts;
  FAR struct rts_conn_s *conn;
  unsigned int hdrlen;
  int ret = OK;

  rts_select(dev);

  /* Update the count of RTS packets received */

  /* Get a pointer to the RTS header.  The RTS header lies just after the
   * the link layer header and the IP header.
   */

  rts = (FAR struct rts_hdr_s *)&dev->d_buf[NET_LL_HDRLEN(dev)];

    {
      /* Demultiplex this RTS packet between the RTS "connections".
       *
       * REVISIT:  The logic here expects either a single receive socket or
       * none at all.  However, multiple sockets should be capable of
       * receiving a RTS datagram (multicast reception).  This could be
       * handled easily by something like:
       *
       *   for (conn = NULL; conn = rts_active(dev, rts); )
       *
       * If the callback logic that receives a packet responds with an
       * outgoing packet, then it will over-write the received buffer,
       * however.  recvfrom() will not do that, however.  We would have to
       * make that the rule: Recipients of a RTS packet must treat the
       * packet as read-only.
       */

      conn = rts_active(dev, rts);
      if (conn)
        {
          uint32_t flags;

          /* Set-up for the application callback */

          dev->d_appdata = &dev->d_buf[hdrlen];
          dev->d_sndlen  = 0;

          /* Perform the application callback */

          flags = rts_callback(dev, conn, RTS_NEWDATA);

          /* If the operation was successful, the RTS_NEWDATA flag is removed
           * and thus the packet can be deleted (OK will be returned).
           */

          if ((flags & RTS_NEWDATA) != 0)
            {
              /* No.. the packet was not processed now.  Return ERROR so
               * that the driver may retry again later.
               */

              ret = ERROR;
            }

          /* If the application has data to send, setup the RTS/IP header */

          if (dev->d_sndlen > 0)
            {
              rts_send(dev, conn);
            }
        }
      else
        {
          nlldbg("No listener on RTS port\n");
          dev->d_len = 0;
        }
    }

  return ret;
#endif
  return 0;
}

#endif /* CONFIG_NET && CONFIG_NET_RTS */
