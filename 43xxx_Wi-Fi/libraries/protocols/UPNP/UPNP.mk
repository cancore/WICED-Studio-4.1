#
# Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of 
 # Cypress Semiconductor Corporation. All Rights Reserved.
 # This software, including source code, documentation and related
 # materials ("Software"), is owned by Cypress Semiconductor Corporation
 # or one of its subsidiaries ("Cypress") and is protected by and subject to
 # worldwide patent protection (United States and foreign),
 # United States copyright laws and international treaty provisions.
 # Therefore, you may use this Software only as provided in the license
 # agreement accompanying the software package from which you
 # obtained this Software ("EULA").
 # If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 # non-transferable license to copy, modify, and compile the Software
 # source code solely for use in connection with Cypress's
 # integrated circuit products. Any reproduction, modification, translation,
 # compilation, or representation of this Software except as specified
 # above is prohibited without the express written permission of Cypress.
 #
 # Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 # EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 # WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 # reserves the right to make changes to the Software without notice. Cypress
 # does not assume any liability arising out of the application or use of the
 # Software or any product or circuit described in the Software. Cypress does
 # not authorize its products for use in any products where a malfunction or
 # failure of the Cypress product may reasonably be expected to result in
 # significant property damage, injury or death ("High Risk Product"). By
 # including Cypress's product in a High Risk Product, the manufacturer
 # of such system or application assumes all risk of such use and in doing
 # so agrees to indemnify Cypress against all liability.
#

NAME               := Lib_UPNP

IXML_SOURCES       := \
					ixml/src/attr.c \
					ixml/src/document.c \
					ixml/src/element.c \
					ixml/src/ixml.c \
					ixml/src/ixmldebug.c \
					ixml/src/ixmlparser.c \
					ixml/src/ixmlmembuf.c \
					ixml/src/namedNodeMap.c \
					ixml/src/node.c \
					ixml/src/nodeList.c

THREADUTIL_SOURCES := \
					threadutil/src/FreeList.c \
					threadutil/src/LinkedList.c \
					threadutil/src/ThreadPool.c \
					threadutil/src/TimerThread.c
					
UPNP_SOURCES       := \
					upnp/src/ssdp/ssdp_device.c \
					upnp/src/ssdp/ssdp_ctrlpt.c \
					upnp/src/ssdp/ssdp_server.c \
					upnp/src/soap/soap_device.c \
					upnp/src/soap/soap_ctrlpt.c \
					upnp/src/soap/soap_common.c \
					upnp/src/genlib/miniserver/miniserver.c \
					upnp/src/genlib/service_table/service_table.c \
					upnp/src/genlib/util/membuffer.c \
					upnp/src/genlib/util/strintmap.c \
					upnp/src/genlib/util/upnp_timeout.c \
					upnp/src/genlib/util/util.c \
					upnp/src/genlib/client_table/client_table.c \
					upnp/src/genlib/net/sock.c \
					upnp/src/genlib/net/http/httpparser.c \
					upnp/src/genlib/net/http/httpreadwrite.c \
					upnp/src/genlib/net/http/statcodes.c \
					upnp/src/genlib/net/http/parsetools.c \
					upnp/src/genlib/net/http/webserver.c \
					upnp/src/genlib/net/uri/uri.c \
					upnp/src/gena/gena_device.c \
					upnp/src/gena/gena_ctrlpt.c \
					upnp/src/gena/gena_callback2.c \
					upnp/src/api/upnpdebug.c \
					upnp/src/api/UpnpString.c \
					upnp/src/api/upnpapi.c \
					upnp/src/api/upnptools.c \
					upnp/src/urlconfig/urlconfig.c \
					upnp/src/uuid/md5.c \
					upnp/src/uuid/sysdep.c \
					upnp/src/uuid/uuid.c \
					upnp/src/inet_pton.c

$(NAME)_SOURCES    := $(IXML_SOURCES) $(THREADUTIL_SOURCES) $(UPNP_SOURCES)

$(NAME)_INCLUDES   := \
					. \
					ixml/src/inc \
					upnp/src/inc

$(NAME)_CFLAGS     += -std=gnu99

GLOBAL_INCLUDES    := \
                    . \
					ixml/inc \
					upnp/inc \
					threadutil/inc

GLOBAL_DEFINES     := WICED

$(NAME)_COMPONENTS += utilities/linked_list

KEEP_LIST          := *