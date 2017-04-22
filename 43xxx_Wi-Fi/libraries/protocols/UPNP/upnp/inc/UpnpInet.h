#ifndef UPNPINET_H
#define UPNPINET_H

/*!
 * \addtogroup Sock
 * 
 * @{
 * 
 * \file
 *
 * \brief Provides a platform independent way to include TCP/IP types and functions.
 */

#include "UpnpUniStd.h" /* for close() */

#ifdef WIN32
	#include <stdarg.h>
	#ifndef UPNP_USE_MSVCPP
		/* Removed: not required (and cause compilation issues) */
		#include <winbase.h>
		#include <windef.h>
	#endif
	#include <winsock2.h>
	#include <iphlpapi.h>
	#include <ws2tcpip.h>

	#define UpnpCloseSocket closesocket

	#if(_WIN32_WINNT < 0x0600)
		typedef short sa_family_t;
	#else
		typedef ADDRESS_FAMILY sa_family_t;
	#endif

#else /* WIN32 */

    #ifdef WICED

    #include <sys/param.h>

    #include "wiced_tcpip.h"

	/*! INVALID_SOCKET is unsigned on win32. */
    #define INVALID_SOCKET (NULL)

    /*! select() returns SOCKET_ERROR on win32. */
    #define SOCKET_ERROR (WICED_ERROR)

    #define AF_UNSPEC        (0)
    #define AF_INET          WICED_IPV4
    #define AF_INET6         WICED_IPV6

    #define INET_ADDRSTRLEN  16
    #define INET6_ADDRSTRLEN 46

    #define IPV6_ADDRESS_LINKLOCAL      0x00000001
	#define IN6_IS_ADDR_LINKLOCAL(ipv6) (IPv6_Address_Type((ULONG*)ipv6) & IPV6_ADDRESS_LINKLOCAL)

    #define UPNP_UDP_DEFAULT_WAIT_TIMEOUT_MSECS       (10)
    #define UPNP_UDP_DEFAULT_DNS_LOOKUP_TIMEOUT_MSECS (5000)

    #define WICED_UPNP_SOCKET_MAGIC    (0xFADADEEB)

    #define SOCKET_PTR_VALID(x)        (((x)->magic == WICED_UPNP_SOCKET_MAGIC) ? WICED_TRUE : WICED_FALSE)
    #define SET_SOCKET_MAGIC(x)        ((x)->magic = WICED_UPNP_SOCKET_MAGIC)
    #define CLEAR_SOCKET_MAGIC(x)      ((x)->magic = 0)
    #define GET_UDP_SOCKET(x)          (&((x)->socket.udp))
    #define GET_TCP_SOCKET(x)          (&((x)->socket.tcp))
    #define GET_SOCKET_TYPE(x)         ((x)->type)
    #define GET_SOCKET_STREAM(x)       (&((x)->stream))
    #define SOCKET_STREAM_PTR_VALID(x) (((x)->stream_ptr != NULL) ? WICED_TRUE : WICED_FALSE)
    #define SET_SOCKET_STREAM_PTR(x)   ((x)->stream_ptr = &((x)->stream))
    #define CLEAR_SOCKET_STREAM_PTR(x) ((x)->stream_ptr = NULL)
    #define SET_UDP_SOCKET_TYPE(x)     ((x)->type = UPNP_SOCKET_UDP)
    #define SET_TCP_SOCKET_TYPE(x)     ((x)->type = UPNP_SOCKET_TCP)

	typedef enum
	{
	    UPNP_SOCKET_UNSPEC = 0,
	    UPNP_SOCKET_UDP,
	    UPNP_SOCKET_TCP
	} wiced_upnp_socket_type_t;

	typedef struct
	{
	    uint32_t magic;
	    union
	    {
	        wiced_udp_socket_t udp;
	        wiced_tcp_socket_t tcp;
	    } socket;

	    wiced_upnp_socket_type_t type;
	    wiced_tcp_stream_t       stream;
	    wiced_tcp_stream_t*      stream_ptr;
	} wiced_upnp_socket_t;

	typedef wiced_upnp_socket_t* SOCKET;
    typedef uint32_t socklen_t;

    typedef struct
    {
            wiced_ip_address_t addr;
            uint16_t           port;
    } wiced_ip_address_with_port_t;

    extern ULONG IPv6_Address_Type(ULONG *ip_address);

    static inline int UpnpCloseSocket(SOCKET socket)
    {
        int rc = 0;

        if ( socket != INVALID_SOCKET && SOCKET_PTR_VALID(socket) )
        {
            if ( GET_SOCKET_TYPE(socket) == UPNP_SOCKET_UDP )
            {
                rc = wiced_udp_delete_socket( GET_UDP_SOCKET(socket) );
            }
            else if ( GET_SOCKET_TYPE(socket) == UPNP_SOCKET_TCP )
            {
                rc = wiced_tcp_delete_socket( GET_TCP_SOCKET(socket) );
            }
            if ( rc != WICED_SUCCESS )
            {
                rc = SOCKET_ERROR;
            }
            CLEAR_SOCKET_MAGIC(socket);
            free(socket);
        }

        return rc;
    }

    #else /* !WICED */

	#if defined(__sun)
		#include <fcntl.h>
		#include <sys/sockio.h>
	#elif (defined(BSD) && BSD >= 199306) || defined (__FreeBSD_kernel__)
		#include <ifaddrs.h>
		/* Do not move or remove the include below for "sys/socket"!
		 * Will break FreeBSD builds. */
		#include <sys/socket.h>
	#endif

	#include <arpa/inet.h>  /* for inet_pton() */
	#include <net/if.h>
	#include <netinet/in.h>

	/*! This typedef makes the code slightly more WIN32 tolerant.
	 * On WIN32 systems, SOCKET is unsigned and is not a file
	 * descriptor. */
	typedef int SOCKET;

	/*! INVALID_SOCKET is unsigned on win32. */
	#define INVALID_SOCKET (-1)

	/*! select() returns SOCKET_ERROR on win32. */
	#define SOCKET_ERROR (-1)

	/*! Alias to close() to make code more WIN32 tolerant. */
	#define UpnpCloseSocket close

	#endif /* WICED */

#endif /* WIN32 */

/* @} Sock */

#endif /* UPNPINET_H */
