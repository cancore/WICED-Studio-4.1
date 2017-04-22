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

NAME := App_console

#==============================================================================
# Console specific files
#==============================================================================
$(NAME)_SOURCES := wiced_init.c

$(NAME)_COMPONENTS += utilities/command_console

#==============================================================================
# Additional command console modules
#==============================================================================
$(NAME)_COMPONENTS += utilities/command_console/wps \
                      utilities/command_console/wifi \
                      utilities/command_console/thread \
                      utilities/command_console/ping \
                      utilities/command_console/platform \
                      utilities/command_console/tracex \
                      utilities/command_console/mallinfo

ifeq ($(PLATFORM),$(filter $(PLATFORM), BCM943438WCD1 BCM943907WAE2_1))
$(NAME)_COMPONENTS += utilities/command_console/bt_hci \
                      drivers/bluetooth/mfg_test
GLOBAL_DEFINES += BT_BUS_RX_FIFO_SIZE=512
$(NAME)_DEFINES += CONSOLE_INCLUDE_BT
endif

ifneq ($(PLATFORM),$(filter $(PLATFORM), BCM943341WAE BCM943340WCD1 ))
$(NAME)_COMPONENTS           += utilities/command_console/p2p
WICED_USE_WIFI_P2P_INTERFACE := 1
$(NAME)_DEFINES              += CONSOLE_INCLUDE_P2P
GLOBAL_DEFINES               += WICED_DCT_INCLUDE_P2P_CONFIG
endif

BCM94390x_ETHERNET_PLATFORMS := BCM943909* BCM943907*
$(eval BCM94390x_ETHERNET_PLATFORMS := $(call EXPAND_WILDCARD_PLATFORMS,$(BCM94390x_ETHERNET_PLATFORMS)))
ifeq ($(PLATFORM),$(filter $(PLATFORM), $(BCM94390x_ETHERNET_PLATFORMS)))
$(NAME)_COMPONENTS += utilities/command_console/ethernet
$(NAME)_DEFINES += CONSOLE_INCLUDE_ETHERNET
endif

#==============================================================================
# Includes
#==============================================================================
$(NAME)_INCLUDES := .

#==============================================================================
# Configuration
#==============================================================================

#==============================================================================
# Global defines
#==============================================================================
GLOBAL_DEFINES += STDIO_BUFFER_SIZE=128

#Increase packet pool size for particular platforms
#To use Enterprise Security please use below commented settings
#GLOBAL_DEFINES += TX_PACKET_POOL_SIZE=10 \
#                  RX_PACKET_POOL_SIZE=10
ifeq ($(PLATFORM),$(filter $(PLATFORM),BCM943362WCD4 BCM943364WCD1 BCM94343WWCD1 BCM94343WWCD2 BCM943438WCD1))
ifeq ($(WICED_DISABLE_COMMON_PKT_POOL),1)
GLOBAL_DEFINES += TX_PACKET_POOL_SIZE=14 \
                  RX_PACKET_POOL_SIZE=12 \
                  WICED_TCP_TX_DEPTH_QUEUE=10 \
                  TCP_WINDOW_SIZE=131072
else #WICED_DISABLE_COMMON_PKT_POOL
#This flag enables the NetX stack to use commong packet pool for both RX and TX direction
# Also this create a smaller packet pool of 128 bytes.
#To use Enterprise Security please use below commented settings
#GLOBAL_DEFINES += WICED_USE_COMMON_PKT_POOL \
#                  COM_PACKET_POOL_SIZE=20
GLOBAL_DEFINES += WICED_USE_COMMON_PKT_POOL \
                  PACKET_POOL_SIZE_128=10 \
                  COM_PACKET_POOL_SIZE=26 \
                  WICED_TCP_TX_DEPTH_QUEUE=24 \
                  WICED_UDP_QUEUE_MAX=8 \
                  TCP_WINDOW_SIZE=131072

# This flag makes the netx stack to use smaller 128 byte pkt pool (PACKET_POOL_SIZE_128) for it's IP operation.
# Netx uses this pool for tcp ack sending, to fragment a big IP packet etc. For various operation 128 byte pool is suffice.
# But if you requires handing very big IP packet (like a ping with 10K ping), then DONOT SET THIS FLAG, as this big IP
# need to be fragmented, which can't be handled using smaller pool.
# As most of the appilication request IP packet of MTU size, and doesn't require such a big IP packet, it is enabled by default
GLOBAL_DEFINES += WICED_USE_128_POOL_FOR_IP_STACK
endif #WICED_DISABLE_COMMON_PKT_POOL
else
BCM943x_PLATFORMS := BCM9WCD2REFAD.*
$(eval BCM943x_PLATFORMS := $(call EXPAND_WILDCARD_PLATFORMS,$(BCM943x_PLATFORMS)))
ifeq ($(PLATFORM),$(filter $(PLATFORM), $(BCM943x_PLATFORMS)))
GLOBAL_DEFINES += TX_PACKET_POOL_SIZE=16 \
                  RX_PACKET_POOL_SIZE=12 \
                  WICED_TCP_TX_DEPTH_QUEUE=15 \
                  TCP_WINDOW_SIZE=16384
else
# Set 43909 specific packet pool settings
BCM94390x_PLATFORMS := BCM943909* BCM943907* BCM943903*
$(eval BCM94390x_PLATFORMS := $(call EXPAND_WILDCARD_PLATFORMS,$(BCM94390x_PLATFORMS)))
ifeq ($(PLATFORM),$(filter $(PLATFORM), $(BCM94390x_PLATFORMS)))
GLOBAL_DEFINES += TX_PACKET_POOL_SIZE=40 \
                  RX_PACKET_POOL_SIZE=40 \
                  WICED_TCP_TX_DEPTH_QUEUE=32 \
                  WICED_ETHERNET_DESCNUM_TX=32 \
                  WICED_ETHERNET_DESCNUM_RX=8 \
                  WICED_ETHERNET_RX_PACKET_POOL_SIZE=40+WICED_ETHERNET_DESCNUM_RX \
                  TCP_WINDOW_SIZE=32768
# Enlarge stack size of test.console for solving WPA-Enterprise issue of BCM4390x.
GLOBAL_DEFINES += CONSOLE_THREAD_STACK_SIZE=8*1024
else
# Otherwise use default values
endif
endif
endif
#endif

INVALID_PLATFORMS := BCM943362WCD4_LPCX1769 \
                     BCM943362WCDA \
                     STMDiscovery411_BCM43438

#==============================================================================
# Wl tool inclusion
#==============================================================================
# Platforms & combinations with enough memory to fit WL tool, can declare CONSOLE_ENABLE_WL := 1
CONSOLE_ENABLE_WL ?= 0
# Enable wl commands which require large data buffer. ex: wl curpower, wl dump ampdu, ...
# set WL_COMMAND_LARGE_BUFFER=1 to enable this. Default is disabled.
WL_COMMAND_LARGE_BUFFER ?= 0

ifeq ($(CONSOLE_ENABLE_WL),1)
ifeq ($(WL_COMMAND_LARGE_BUFFER),1)
# Some wl command might still not work when WICED_PAYLOAD_MTU=2560 (ex: scanresults might fail when there're many AP)
# Increasing WICED_PAYLOAD_MTU will increase the total packet buffer pools size and might not fit into the available ram size.
# 2560 is chosen for 1) selected wl commands <ex: curpower, dump ampdu, ...>, 2) acceptable packet pools size.
GLOBAL_DEFINES += WICED_PAYLOAD_MTU=2560
endif
endif

#==============================================================================
# Provision to replace wlan production fw with mfg_test FW
#==============================================================================
CONSOLE_USE_MFG_TEST_FW ?= 0
ifeq ($(CONSOLE_USE_MFG_TEST_FW),1)
ifneq ($(PLATFORM),$(filter $(PLATFORM),BCM943362WCD4 BCM943364WCD1 BCM94343WWCD1 BCM943438WCD1 BCM94343WWCD2))
$(NAME)_RESOURCES += firmware/$(WLAN_CHIP)/$(WLAN_CHIP)$(WLAN_CHIP_REVISION)-mfgtest.bin
else
# Set the WIFI firmware in multi application file system to point to firmware
MULTI_APP_WIFI_FIRMWARE   := resources/firmware/$(WLAN_CHIP)/$(WLAN_CHIP)$(WLAN_CHIP_REVISION)-mfgtest.bin
endif
NO_WIFI_FIRMWARE := YES
endif

#==============================================================================
# Network stack-specific inclusion
#==============================================================================
ifeq ($(NETWORK),NetX)
#$(NAME)_SOURCES += NetX/netdb.c

ifdef CONSOLE_ENABLE_WPS
GLOBAL_DEFINES  += ADD_NETX_EAPOL_SUPPORT
endif
endif

ifeq ($(NETWORK),NetX_Duo)
#$(NAME)_SOURCES += NetX_Duo/netdb.c

ifdef CONSOLE_ENABLE_WPS
GLOBAL_DEFINES  += ADD_NETX_EAPOL_SUPPORT
endif
endif

GLOBAL_DEFINES += CONSOLE_ENABLE_THREADS

#==============================================================================
# iperf inclusion
#==============================================================================
ifndef CONSOLE_NO_IPERF
$(NAME)_COMPONENTS += test/iperf
$(NAME)_DEFINES    += CONSOLE_ENABLE_IPERF
endif

#==============================================================================
# Traffic generation inclusion
#==============================================================================
#$(NAME)_COMPONENTS += utilities/command_console/traffic_generation

#Optional Phyrate Logging
ifeq ($(RVR_PHYRATE_LOGGING),1)
GLOBAL_DEFINES     += RVR_PHYRATE_LOGGING
endif
