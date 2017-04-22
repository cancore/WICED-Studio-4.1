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

NAME := Lib_wlu_server

$(NAME)_SOURCES  := $(WLAN_CHIP)$(WLAN_CHIP_REVISION)/wl/exe/wlu_server_shared.c \
                   wlu_server.c \
                   wlu_wiced.c \
                   $(WLAN_CHIP)$(WLAN_CHIP_REVISION)/wl/exe/wlu_pipe.c \
                   $(WLAN_CHIP)$(WLAN_CHIP_REVISION)/wl/exe/wlu.c \
                   $(WLAN_CHIP)$(WLAN_CHIP_REVISION)/shared/miniopt.c \
                   $(WLAN_CHIP)$(WLAN_CHIP_REVISION)/shared/bcmutils.c \
                   $(WLAN_CHIP)$(WLAN_CHIP_REVISION)/shared/bcm_app_utils.c

$(NAME)_INCLUDES := ./$(WLAN_CHIP)$(WLAN_CHIP_REVISION)/include \
                    ./$(WLAN_CHIP)$(WLAN_CHIP_REVISION)/common/include \
                    ./$(WLAN_CHIP)$(WLAN_CHIP_REVISION)/wl/exe \
                    ./$(WLAN_CHIP)$(WLAN_CHIP_REVISION)/shared/bcmwifi/include \
                    ./$(WLAN_CHIP)$(WLAN_CHIP_REVISION)/wl/ppr/include


CHIP := $(WLAN_CHIP)$(WLAN_CHIP_REVISION)
include $(CURDIR)$(WLAN_CHIP)$(WLAN_CHIP_REVISION)/$(WLAN_CHIP)$(WLAN_CHIP_REVISION).mk
$(NAME)_SOURCES += $($(CHIP)_SOURCE_WL)

#Serial Support
$(NAME)_DEFINES  := RWL_SERIAL TARGET_wiced
#Ethernet Support
#$(NAME)_DEFINES  := RWL_SOCKET TARGET_wiced

#$(NAME)_COMPONENTS += test/malloc_debug

GLOBAL_INCLUDES  += .

#AVOID_GLOMMING_IOVAR AVOID_APSTA SET_COUNTRY_WITH_IOCTL_NOT_IOVAR

