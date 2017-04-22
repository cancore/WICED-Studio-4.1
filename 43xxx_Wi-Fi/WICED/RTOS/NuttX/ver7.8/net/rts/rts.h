/****************************************************************************
 * net/rts/rts.h
 *
 *   Copyright (C) 2014-2015 Gregory Nutt. All rights reserved.
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

#ifndef __NET_RTS_RTS_H
#define __NET_RTS_RTS_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <queue.h>
#include <nuttx/net/ip.h>
#include <nuttx/net/iob.h>

#ifdef CONFIG_NET_RTS

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Allocate a new RTS data callback */

#define rts_callback_alloc(dev, conn) \
  devif_callback_alloc(dev, &conn->list)
#define rts_callback_free(dev, conn, cb) \
  devif_conn_callback_free(dev, cb, &conn->list)

/****************************************************************************
 * Public Type Definitions
 ****************************************************************************/

/* Representation of a RTS connection */

struct devif_callback_s;  /* Forward reference */

struct rts_conn_s
{
  dq_entry_t node;                   /* Supports a doubly linked list */
  uint8_t  domain;                   /* AF domain: not used for now */
  uint8_t  crefs;                    /* Reference counts on this instance */
  struct iob_queue_s readahead;      /* Read-ahead buffering */
  FAR struct devif_callback_s *list; /* Defines the list of RTS callbacks */
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifdef __cplusplus
#  define EXTERN extern "C"
extern "C"
{
#else
#  define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

struct sockaddr;      /* Forward reference */
struct socket;        /* Forward reference */
struct net_driver_s;  /* Forward reference */
struct pollfd;        /* Forward reference */

/****************************************************************************
 * Name: rts_initialize
 *
 * Description:
 *   Initialize the RTS connection structures.  Called once and only from
 *   the UIP layer.
 *
 ****************************************************************************/

void rts_initialize(void);

/****************************************************************************
 * Name: rts_alloc
 *
 * Description:
 *   Allocate a new, uninitialized RTS connection structure.  This is
 *   normally something done by the implementation of the socket() API
 *
 ****************************************************************************/

FAR struct rts_conn_s *rts_alloc(void);

/****************************************************************************
 * Name: rts_free
 *
 * Description:
 *   Free a RTS connection structure that is no longer in use. This should be
 *   done by the implementation of close().
 *
 ****************************************************************************/

void rts_free(FAR struct rts_conn_s *conn);

/****************************************************************************
 * Name: rts_active
 *
 * Description:
 *   Find a connection structure that is the appropriate
 *   connection to be used within the provided UDP/IP header
 *
 * Assumptions:
 *   Called from network stack logic with the network stack locked
 *
 ****************************************************************************/

FAR struct rts_conn_s *rts_active(FAR struct net_driver_s *dev);

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

FAR struct rts_conn_s *rts_nextconn(FAR struct rts_conn_s *conn);

/****************************************************************************
 * Name: rts_poll
 *
 * Description:
 *   Poll a RTS "connection" structure for availability of TX data
 *
 * Parameters:
 *   dev  - The device driver structure to use in the send operation
 *   conn - The RTS "connection" to poll for TX data
 *
 * Return:
 *   None
 *
 * Assumptions:
 *   Called from network stack logic with the network stack locked
 *
 ****************************************************************************/

void rts_poll(FAR struct net_driver_s *dev, FAR struct rts_conn_s *conn);

/****************************************************************************
 * Function: rts_find_device
 *
 * Description:
 *   Select the network driver to use with the IPv4 RTS transaction.
 *
 * Input Parameters:
 *   conn - RTS connection structure (not currently used).
 *
 * Returned Value:
 *   A pointer to the network driver to use.
 *
 ****************************************************************************/

FAR struct net_driver_s *rts_find_device(FAR struct rts_conn_s *conn);

/****************************************************************************
 * Function: rts_callback
 *
 * Description:
 *   Inform the application holding the RTS socket of a change in state.
 *
 * Returned Value:
 *   OK if packet has been processed, otherwise ERROR.
 *
 * Assumptions:
 *   Called from network stack logic with the network stack locked
 *
 ****************************************************************************/

uint32_t rts_callback(FAR struct net_driver_s *dev,
                      FAR struct rts_conn_s *conn, uint32_t flags);

/****************************************************************************
 * Function: psock_rts_send
 *
 * Description:
 *   Implements send() for connected RTS sockets
 *
 ****************************************************************************/

ssize_t psock_rts_send(FAR struct socket *psock, FAR const void *buf,
                       size_t len);

/****************************************************************************
 * Function: rts_pollsetup
 *
 * Description:
 *   Setup to monitor events on one RTS/IP socket
 *
 * Input Parameters:
 *   psock - The RTS/IP socket of interest
 *   fds   - The structure describing the events to be monitored, OR NULL if
 *           this is a request to stop monitoring events.
 *
 * Returned Value:
 *  0: Success; Negated errno on failure
 *
 ****************************************************************************/

int rts_pollsetup(FAR struct socket *psock, FAR struct pollfd *fds);

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

int rts_pollteardown(FAR struct socket *psock, FAR struct pollfd *fds);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* CONFIG_NET_RTS */
#endif /* __NET_RTS_RTS_H */
