/****************************************************************************
 * net/rts/rts_notify.c
 *
 *   Copyright (C) 2007-2009, 2015 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#if defined(CONFIG_NET) && defined(CONFIG_NET_RTS)

#include <stdint.h>
#include <string.h>
#include <debug.h>

#include <nuttx/net/netconfig.h>
#include <nuttx/net/netdev.h>
#include <nuttx/net/netstats.h>
#include <nuttx/net/rts.h>

#include "devif/devif.h"
#include "iob/iob.h"
#include "rts/rts.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Function: rts_datahandler
 *
 * Description:
 *   Handle the receipt of RTS data by adding the newly received packet to
 *   the RTS read-ahead buffer.
 *
 ****************************************************************************/

static uint16_t rts_datahandler(FAR struct net_driver_s *dev, FAR struct rts_conn_s *conn,
                                FAR uint8_t *buffer, uint16_t buflen)
{
  FAR struct iob_s *iob;
  int ret;

  /* Allocate on I/O buffer to start the chain (throttling as necessary).
   * We will not wait for an I/O buffer to become available in this context.
   */
  if ( buffer == NULL || buflen <= 0 )
    {
      nlldbg("ERROR: invalid input parameter buffer=0x%p, len=%d\n", buffer, buflen);
      return 0;      
    }

  iob = iob_tryalloc(true);
  if (iob == NULL)
    {
      nlldbg("ERROR: Failed to create new I/O buffer chain\n");
      return 0;
    }

  /* Copy the the appdata into the I/O buffer chain.  We will not wait
   * for an I/O buffer to become available in this context.  It there is
   * any failure to allocated, the entire I/O buffer chain will be discarded.
   */

  ret = iob_trycopyin(iob, buffer, buflen, 0, true);

  if (ret < 0)
    {
      /* On a failure, iob_trycopyin return a negated error value but
       * does not free any I/O buffers.
       */

      nlldbg("ERROR: Failed to add data to the I/O buffer chain: %d\n",
             ret);
      (void)iob_free_chain(iob);
       return 0;
    }

  /* Add the new I/O buffer chain to the tail of the read-ahead queue */

  ret = iob_tryadd_queue(iob, &conn->readahead);
  if (ret < 0)
    {
      nlldbg("ERROR: Failed to queue the I/O buffer chain: %d\n", ret);
      (void)iob_free_chain(iob);
      return 0;
    }

  nllvdbg("Buffered %d bytes\n", buflen);
  return buflen;
}

/****************************************************************************
 * Function: net_dataevent
 *
 * Description:
 *   Handling the network RTS_NOTIFY event.
 *
 ****************************************************************************/

static inline uint32_t
net_dataevent(FAR struct net_driver_s *dev, FAR struct rts_conn_s *conn,
              uint8_t *buffer, int buflen, uint32_t flags)
{
  uint16_t recvlen;

  /* Is there new data?  With non-zero length?  (Certain connection events
   * can have zero-length with RTS_NOTIFY set just to cause an ACK).
   */

   nllvdbg("No receive on connection\n");

  /* Save as the packet data as in the read-ahead buffer.  NOTE that
   * partial packets will not be buffered.
   */

  recvlen = rts_datahandler(dev, conn, buffer, buflen);
  if (recvlen < buflen)
    {
      /* There is no handler to receive new data and there are no free
       * read-ahead buffers to retain the data -- drop the packet.
       */

     nllvdbg("Dropped %d bytes\n", dev->d_len);
    }

  /* In any event, the new data has now been handled */

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Function: rts_notify
 *
 * Description:
 *   Inform the application holding the RTS socket of a change in state.
 *
 * Returned Value:
 *   OK if packet has been processed, otherwise ERROR.
 *
 * Assumptions:
 *   This function is called at the interrupt level with interrupts disabled.
 *
 ****************************************************************************/

int rts_notify(uint8_t *buffer, int buflen, uint32_t flags)
{
  FAR struct rts_conn_s *conn = NULL;

  nllvdbg("flags: %08x buf=0x%p buflen=%d\n", flags, buffer, buflen);

  /* Some sanity checking */
  if (buffer == NULL || buflen <= 0)
    {
       nlldbg("Bad parameter buffer=0x%p, buflen=%d\n", buffer, buflen);
       return ERROR;       
    }
 
   /* broadcast the notification to all active RTS socket */ 
   while ((conn = rts_nextconn(conn)))
    {
      /* Perform the callback */
      /* NOTE: currently only support polling, So flags won't change */
      flags = devif_conn_event(NULL, conn, flags, conn->list);

      if ((flags & RTS_NOTIFY) != 0)
        {
          /* data is buffered */
          flags = net_dataevent(NULL, conn, buffer, buflen, flags);
        }
    }

  return OK;
}

#endif /* CONFIG_NET && CONFIG_NET_RTS */
