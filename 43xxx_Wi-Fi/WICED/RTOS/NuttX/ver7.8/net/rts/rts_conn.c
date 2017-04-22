/****************************************************************************
 * net/rts/rts_conn.c
 *
 *   Copyright (C) 2007-2009, 2011-2012, 2016 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Large parts of this file were leveraged from uIP logic:
 *
 *   Copyright (c) 2001-2003, Adam Dunkels.
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

#include <stdint.h>
#include <string.h>
#include <semaphore.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>

#include <netinet/in.h>

#include <arch/irq.h>

#include <nuttx/net/netconfig.h>
#include <nuttx/net/net.h>
#include <nuttx/net/netdev.h>
#include <nuttx/net/ip.h>
#include <nuttx/net/rts.h>

#include "devif/devif.h"
#include "netdev/netdev.h"
#include "iob/iob.h"
#include "rts/rts.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* The array containing all RTS connections. */

struct rts_conn_s g_rts_connections[CONFIG_NET_RTS_CONNS];

/* A list of all free RTS connections */

static dq_queue_t g_free_rts_connections;
static sem_t g_free_sem;

/* A list of all allocated RTS connections */

static dq_queue_t g_active_rts_connections;


/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: _rts_semtake() and _rts_semgive()
 *
 * Description:
 *   Take/give semaphore
 *
 ****************************************************************************/

static inline void _rts_semtake(FAR sem_t *sem)
{
  /* Take the semaphore (perhaps waiting) */

  while (net_lockedwait(sem) != 0)
    {
      /* The only case that an error should occur here is if
       * the wait was awakened by a signal.
       */

      ASSERT(get_errno() == EINTR);
    }
}

#define _rts_semgive(sem) sem_post(sem)

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: rts_initialize
 *
 * Description:
 *   Initialize the RTS connection structures.  Called once and only from
 *   the UIP layer.
 *
 ****************************************************************************/

void rts_initialize(void)
{
  int i;

  /* Initialize the queues */

  dq_init(&g_free_rts_connections);
  dq_init(&g_active_rts_connections);
  sem_init(&g_free_sem, 0, 1);

  for (i = 0; i < CONFIG_NET_RTS_CONNS; i++)
    {
      /* Mark the connection closed and move it to the free list */

       dq_addlast(&g_rts_connections[i].node, &g_free_rts_connections);
    }
}

/****************************************************************************
 * Name: rts_alloc
 *
 * Description:
 *   Allocate a new, uninitialized RTS connection structure.  This is
 *   normally something done by the implementation of the socket() API
 *
 ****************************************************************************/

FAR struct rts_conn_s *rts_alloc(void)
{
  FAR struct rts_conn_s *conn;

  /* The free list is only accessed from user, non-interrupt level and
   * is protected by a semaphore (that behaves like a mutex).
   */

  _rts_semtake(&g_free_sem);
  conn = (FAR struct rts_conn_s *)dq_remfirst(&g_free_rts_connections);
  if (conn)
    {
      /* Enqueue the connection into the active list */

      dq_addlast(&conn->node, &g_active_rts_connections);
    }

  _rts_semgive(&g_free_sem);
  return conn;
}

/****************************************************************************
 * Name: rts_free
 *
 * Description:
 *   Free a RTS connection structure that is no longer in use. This should be
 *   done by the implementation of close().
 *
 ****************************************************************************/

void rts_free(FAR struct rts_conn_s *conn)
{
  /* The free list is only accessed from user, non-interrupt level and
   * is protected by a semaphore (that behaves like a mutex).
   */

  DEBUGASSERT(conn->crefs == 0);

  _rts_semtake(&g_free_sem);

  /* Remove the connection from the active list */

  dq_rem(&conn->node, &g_active_rts_connections);

  /* Release any read-ahead buffers attached to the connection */

  iob_free_queue(&conn->readahead);

  /* Free the connection */

  dq_addlast(&conn->node, &g_free_rts_connections);
  _rts_semgive(&g_free_sem);
}

/****************************************************************************
 * Name: rts_active
 *
 * Description:
 *   Find a connection structure that is the appropriate
 *   connection to be used within the provided RTS header
 *
 * Assumptions:
 *   This function is called from UIP logic at interrupt level
 *
 ****************************************************************************/

FAR struct rts_conn_s *rts_active(FAR struct net_driver_s *dev)
{
  struct rts_conn_s * conn;
  (void) dev;
  conn = (FAR struct rts_conn_s *)g_active_rts_connections.head;

  return conn;
}

/****************************************************************************
 * Name: rts_nextconn
 *
 * Description:
 *   Traverse the list of allocated RTS connections
 *
 * Assumptions:
 *   This function is called from UIP logic at interrupt level (or with
 *   interrupts disabled).
 *
 ****************************************************************************/

FAR struct rts_conn_s *rts_nextconn(FAR struct rts_conn_s *conn)
{
  if (!conn)
    {
      return (FAR struct rts_conn_s *)g_active_rts_connections.head;
    }
  else
    {
      return (FAR struct rts_conn_s *)conn->node.flink;
    }
}

/****************************************************************************
 * Name: rts_bind
 *
 * Description:
 *   This function implements the low level parts of the standard RTS
 *   bind() operation.
 *
 * Assumptions:
 *   This function is called from normal user level code.
 *
 ****************************************************************************/

int rts_bind(FAR struct rts_conn_s *conn, FAR const struct sockaddr *addr)
{

  (void) conn;
  (void) addr;
  /* not supported yet */
  return 0;
}

/****************************************************************************
 * Name: rts_connect
 *
 * Description:
 *   This function simply assigns a remote address to RTS "connection"
 *   structure.  This function is called as part of the implementation of:
 *
 *   - connect().  If connect() is called for a SOCK_DGRAM socket, then
 *       this logic performs the moral equivalent of connec() operation
 *       for the RTS socket.
 *   - recvfrom() and sendto().  This function is called to set the
 *       remote address of the peer.
 *
 *   The function will automatically allocate an unused local port for the
 *   new connection if the socket is not yet bound to a local address.
 *   However, another port can be chosen by using the rts_bind() call,
 *   after the rts_connect() function has been called.
 *
 * Input Parameters:
 *   conn - A reference to RTS connection structure
 *   addr - The address of the remote host.
 *   connected - a flag to indicate if the RTS socket is connected
 *
 * Assumptions:
 *   This function is called user code.  Interrupts may be enabled.
 *
 ****************************************************************************/

int rts_connect(FAR struct rts_conn_s *conn, FAR const struct sockaddr *addr, int connected)
{
  (void) conn;
  (void) addr;
  (void) connected;
  /* not supported yet */
  return 0;
}

#endif /* CONFIG_NET && CONFIG_NET_RTS */
