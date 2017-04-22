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

NAME := App_sigma_dut_$(RTOS)_$(NETWORK)

$(NAME)_SOURCES := wiced_init.c \
                   wifi/wifi.c \
                   wifi_cert/wifi_cert_commands.c \
                   wifi_cert/$(RTOS)_wifi_cert_ping.c \
                   wifi_cert/wifi_cert_traffic_stream.c \
                   wifi_cert/wifi_cert_thread.c

$(NAME)_INCLUDES := .
$(NAME)_DEFINES  := NETWORK_$(NETWORK)



$(NAME)_COMPONENTS += utilities/command_console

GLOBAL_DEFINES += STDIO_BUFFER_SIZE=1024
GLOBAL_DEFINES += configUSE_MUTEXES=1

GLOBAL_DEFINES += DISABLE_LOGGING

#==============================================================================
# P2P inclusion
# Uncomment the following lines if testing P2P
#==============================================================================
#WICED_USE_WIFI_P2P_INTERFACE = 1
#GLOBAL_DEFINES += WICED_USE_WIFI_P2P_INTERFACE
#GLOBAL_DEFINES += WICED_DCT_INCLUDE_P2P_CONFIG

#==============================================================================
# Platform specific settings.
#==============================================================================
# Increase packet pool size for particular platforms.  Note that the packet pools need to be large enough to handle a 30000 byte ping if running the standard 11n test plan.
ifeq ($(PLATFORM),$(filter $(PLATFORM), BCM943362WCD4))
GLOBAL_DEFINES += TX_PACKET_POOL_SIZE=10 \
                  RX_PACKET_POOL_SIZE=10 \
                  WICED_TCP_TX_DEPTH_QUEUE=8 \
                  TCP_WINDOW_SIZE=8192
else
# Set TX and RX pool length for 4343WWCD2
# Added this separately because 4343WWCD2 needs atleast ~14 TX pools to be able to send 10k Tx ping size and at least 20 RX buffers to support 30k Rx ping size
ifeq ($(PLATFORM),$(filter $(PLATFORM), BCM94343WWCD2))
GLOBAL_DEFINES += TX_PACKET_POOL_SIZE=22 \
                  RX_PACKET_POOL_SIZE=22 \
                  WICED_TCP_TX_DEPTH_QUEUE=8 \
                  TCP_WINDOW_SIZE=8192
else
# Set TX and RX pool length for 43364WCD1
# Added this separately because 43364Wcd1 needs atleasti ~14 TX pools to be able to send 10k Tx ping size and at least 20 RX buffers to support 30k Rx ping size
# Unlike 4343WWCD2, 43364 cannot accommodate more than 40 packets. So the Tx pool is restricted to 18.
ifeq ($(PLATFORM),$(filter $(PLATFORM), BCM943364WCD1))
GLOBAL_DEFINES += TX_PACKET_POOL_SIZE=15 \
                  RX_PACKET_POOL_SIZE=20 \
                  WICED_TCP_TX_DEPTH_QUEUE=8 \
                  TCP_WINDOW_SIZE=8192
else
# Set 43909 specific packet pool settings
BCM94390x_PLATFORMS := BCM943909* BCM943907* BCM943903*
$(eval BCM94390x_PLATFORMS := $(call EXPAND_WILDCARD_PLATFORMS,$(BCM94390x_PLATFORMS)))
ifeq ($(PLATFORM),$(filter $(PLATFORM), $(BCM94390x_PLATFORMS)))
GLOBAL_DEFINES += TX_PACKET_POOL_SIZE=60 \
                  RX_PACKET_POOL_SIZE=60 \
                  WICED_TCP_TX_DEPTH_QUEUE=8 \
                  WICED_ETHERNET_DESCNUM_TX=32 \
                  WICED_ETHERNET_DESCNUM_RX=16 \
                  WICED_ETHERNET_RX_PACKET_POOL_SIZE=40+WICED_ETHERNET_DESCNUM_RX \
                  TCP_WINDOW_SIZE=8192
else
# Otherwise use default values
endif
endif
endif
endif
