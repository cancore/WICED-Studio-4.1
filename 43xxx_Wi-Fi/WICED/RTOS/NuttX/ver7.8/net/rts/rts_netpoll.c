/****************************************************************************
 * net/rts/rts_netpoll.c
 *
 *   Copyright (C) 2008-2009, 2011-2015 Gregory Nutt. All rights reserved.
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

#include <stdint.h>
#include <poll.h>
#include <debug.h>

#include <nuttx/kmalloc.h>
#include <nuttx/net/net.h>

#include <devif/devif.h>
#include "rts/rts.h"
#include "socket/socket.h"

#ifdef CONFIG_NET_RTS

/****************************************************************************
 * Private Types
 ****************************************************************************/
/* This is an allocated container that holds the poll-related information */

struct rts_poll_s
{
  FAR struct socket *psock;        /* Needed to handle loss of connection */
  FAR struct net_driver_s *dev;    /* Needed to free the callback structure */
  struct pollfd *fds;              /* Needed to handle poll events */
  FAR struct devif_callback_s *cb; /* Needed to teardown the poll */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Function: rts_poll_interrupt
 *
 * Description:
 *   This function is called from the interrupt level to perform the actual
 *   RTS receive operation via by the device interface layer.
 *
 * Parameters:
 *   dev      The structure of the network driver that caused the interrupt
 *   conn     The connection structure associated with the socket
 *   flags    Set of events describing why the callback was invoked
 *
 * Returned Value:
 *   None
 *
 * Assumptions:
 *   Running at the interrupt level
 *
 ****************************************************************************/

static uint32_t rts_poll_interrupt(FAR struct net_driver_s *dev, FAR void *conn,
                                   FAR void *pvpriv, uint32_t flags)
{
  FAR struct rts_poll_s *info = (FAR struct rts_poll_s *)pvpriv;

  nllvdbg("flags: %04x\n", flags);

  DEBUGASSERT(!info || (info->psock && info->fds));

  /* 'priv' might be null in some race conditions (?) */

  if (info)
    {
      pollevent_t eventset = 0;

      /* Check for data or connection availability events. */

      if ((flags & RTS_NOTIFY) != 0)
        {
          eventset |= (POLLIN & info->fds->events);
        }

#if 0
      /*  poll is a sign that we are free to send data. */

      if ((flags & RTS_POLL) != 0)
        {
          eventset |= (POLLOUT & info->fds->events);
        }

      /* Check for loss of connection events. */

      if ((flags & NETDEV_DOWN) != 0)
        {
          eventset |= ((POLLHUP | POLLERR) & info->fds->events);
        }
#endif
      /* Awaken the caller of poll() is requested event occurred. */

      if (eventset)
        {
          info->fds->revents |= eventset;
          sem_post(info->fds->sem);
        }
    }

  return flags;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Function: rts_pollsetup
 *
 * Description:
 *   Setup to monitor events on one AF_ROUTE socket
 *
 * Input Parameters:
 *   psock - The AF_ROUTE socket of interest
 *   fds   - The structure describing the events to be monitored, OR NULL if
 *           this is a request to stop monitoring events.
 *
 * Returned Value:
 *  0: Success; Negated errno on failure
 *
 ****************************************************************************/

int rts_pollsetup(FAR struct socket *psock, FAR struct pollfd *fds)
{
  FAR struct rts_conn_s *conn = psock->s_conn;
  FAR struct rts_poll_s *info;
  FAR struct devif_callback_s *cb;
  net_lock_t flags;
  int ret;

  /* Sanity check */

#ifdef CONFIG_DEBUG
  if (!conn || !fds)
    {
      return -EINVAL;
    }
#endif

  /* Allocate a container to hold the poll information */

  info = (FAR struct rts_poll_s *)kmm_malloc(sizeof(struct rts_poll_s));
  if (!info)
    {
      return -ENOMEM;
    }

  /* Some of the  following must be atomic */

  flags = net_lock();

  /* Get the device that will provide the provide the NETDEV_DOWN event.
   * NOTE: in the event that the local socket is bound to INADDR_ANY, the
   * dev value will be zero and there will be no NETDEV_DOWN notifications.
   */

  info->dev = rts_find_device(conn);

  /* Allocate a RTS callback structure */

  cb = rts_callback_alloc(info->dev, conn);
  if (!cb)
    {
      ret = -EBUSY;
      goto errout_with_lock;
    }

  /* Initialize the poll info container */

  info->psock  = psock;
  info->fds    = fds;
  info->cb     = cb;

  /* Initialize the callback structure.  Save the reference to the info
   * structure as callback private data so that it will be available during
   * callback processing.
   */

  cb->flags    = 0;
  cb->priv     = (FAR void *)info;
  cb->event    = rts_poll_interrupt;

#if 0
  if ((info->fds->events & POLLOUT) != 0)
    {
    cb->flags |= RTS_POLL;
    }
#endif

  if ((info->fds->events & POLLIN) != 0)
    {
    cb->flags |= RTS_NOTIFY;
    }

#if 0
  if ((info->fds->events & (POLLHUP | POLLERR)) != 0)
    {
      cb->flags |= NETDEV_DOWN;
    }
#endif

  /* Save the reference in the poll info structure as fds private as well
   * for use during poll teardown as well.
   */

  fds->priv    = (FAR void *)info;

  /* Check for read data availability now */

  if (!IOB_QEMPTY(&conn->readahead))
    {
      /* Normal data may be read without blocking. */

      fds->revents |= (RTS_NOTIFY & fds->events);
    }

  /* Check if any requested events are already in effect */

  if (fds->revents != 0)
    {
      /* Yes.. then signal the poll logic */
      sem_post(fds->sem);
    }

  net_unlock(flags);
  return OK;

errout_with_lock:
  kmm_free(info);
  net_unlock(flags);
  return ret;
}

/****************************************************************************
 * Function: rts_pollteardown
 *
 * Description:
 *   Teardown monitoring of events on an RTS/IP socket
 *
 * Input Parameters:
 *   psock - The TCP/IP socket of interest
 *   fds   - The structure describing the events to be monitored, OR NULL if
 *           this is a request to stop monitoring events.
 *
 * Returned Value:
 *  0: Success; Negated errno on failure
 *
 ****************************************************************************/

int rts_pollteardown(FAR struct socket *psock, FAR struct pollfd *fds)
{
  FAR struct rts_conn_s *conn = psock->s_conn;
  FAR struct rts_poll_s *info;
  net_lock_t flags;

  /* Sanity check */

#ifdef CONFIG_DEBUG
  if (!conn || !fds->priv)
    {
      return -EINVAL;
    }
#endif

  /* Recover the socket descriptor poll state info from the poll structure */

  info = (FAR struct rts_poll_s *)fds->priv;
  DEBUGASSERT(info && info->fds && info->cb);
  if (info)
    {
      /* Release the callback */

      flags = net_lock();
      rts_callback_free(info->dev, conn, info->cb);
      net_unlock(flags);

      /* Release the poll/select data slot */

      info->fds->priv = NULL;

      /* Then free the poll info container */

      kmm_free(info);
    }

  return OK;
}

#endif /* !HAVE_NET_RTS */
