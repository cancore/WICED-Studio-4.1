/****************************************************************************
 * netutils/dhcpd/dhcpd.c
 *
 *   Copyright (C) 2007-2009, 2011-2014 Gregory Nutt. All rights reserved.
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

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netlib.h>
#include <stdio.h>

#include "dhcpd.h"

#define _DHCPD_DEBUG 1
#ifdef _DHCPD_DEBUG
#undef nvdbg
#undef ndbg
#  define ndbg(...) printf(__VA_ARGS__)
#  define nvdbg(...) printf(__VA_ARGS__)
#endif

static int send_selfconnected_message( int sockfd, const void *msg, size_t msglen );
static int open_selfconnected_socket( int *sockfd );

static struct dhcpd_contol_block g_dhcpd_contol_block;

int start_dhcpd(void)
{
   int err;
  
   err = open_selfconnected_socket(&g_dhcpd_contol_block.cmdfd);
   if (err < 0)
   {
      ndbg("start_dhcpd: failed to open command socket\n");
      goto exit;
   }
   
   err = pthread_create( &g_dhcpd_contol_block.tid, NULL, dhcpd_run, (void *) &g_dhcpd_contol_block);
   
   if (err != 0)
   {
      nvdbg("failed create dhcpd thread err=%d", errno);
      close(g_dhcpd_contol_block.cmdfd);
      g_dhcpd_contol_block.cmdfd = -1;
   }
   
exit:
   return err;
}

int stop_dhcpd(void)
{
   int err;
   
   err = send_selfconnected_message(g_dhcpd_contol_block.cmdfd, "q", 1);
   if (err < 0)
   {
      ndbg("failed to send quit command to dhcpd\n");
      goto exit;
   }
   
   err = pthread_join(g_dhcpd_contol_block.tid, NULL);
   if (err != 0)
   {
     ndbg("failed to join dhcpd thread\n");
   }

   close(g_dhcpd_contol_block.cmdfd);
   g_dhcpd_contol_block.cmdfd = -1;
exit:
   return err;   
}

static int open_selfconnected_socket( int *sockfd )
{
    int err;
    int sock;
    struct sockaddr_in sip;
    socklen_t len;
    
    sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if (sock < 0)
    {
       ndbg("failed to open socket errno=%d\n", errno);
       err = -1;
       goto exit;
    }

    memset( &sip, 0, sizeof(sip) );
    sip.sin_family      = AF_INET;
    sip.sin_port        = 0;
    sip.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
    err = bind( sock, (struct sockaddr *)&sip, sizeof( sip ) );
    if (err < 0)
    {
       ndbg("failed to bind socket fd=%d err=%d\n", sock, errno);
       goto exit;
    }
    
    len = (socklen_t) sizeof( sip );
    err = getsockname( sock, (struct sockaddr *)&sip, &len );
    if (err < 0)
    {
       ndbg("failed to getsockname fd=%d err=%d\n", sock, errno);
       goto exit;
    }
    
    err = connect( sock, (struct sockaddr *)&sip, len );
    if (err < 0)
    {
       ndbg("failed to connect socket fd=%d err=%d\n", sock, errno);
       goto exit;
    }
    
    *sockfd = sock;
    sock = -1;
    
exit:
    if (sock >= 0)
    {
       close(sock);
    }
    return( err );
}

static int send_selfconnected_message( int sockfd, const void *msg, size_t msglen )
{
    int err;
    struct sockaddr_in sip;
    socklen_t len;
    ssize_t n;
 
    len = (socklen_t) sizeof( sip );
    err = getsockname(sockfd, (struct sockaddr *)&sip, &len );
    if (err < 0)
    {
       ndbg("failed to getsockname fd=%d err=%d\n", sockfd, errno);
       err = -1;
       goto exit;
    }
    
    n = sendto( sockfd, (char *) msg, msglen, 0, (struct sockaddr *)&sip, (socklen_t) sizeof( sip ) );
    if( n < 0 && errno == EAGAIN )
    {
        n = send( sockfd, (char *) msg, msglen, 0 );
        if (n < 0)
        {
           err = -1;
           ndbg("send failed. fd=%d err=%d\n", sockfd, errno);
           goto exit;
        }
    }
    err = 0;
    
exit:
    return( err );
}

