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
PLATFORM_NAME            := BCM943907WCD1

WLAN_CHIP_REVISION       := B0
APPS_CHIP_REVISION       := B1
WLAN_CHIP                := 43909
WLAN_CHIP_FAMILY         := 4390x
HOST_MCU_FAMILY          := BCM4390x
HOST_MCU_VARIANT         := BCM43907
HOST_MCU_PART_NUMBER     := BCM43907WLCSP

# BCM943907WCD1 includes BCM20707 BT chip
BT_CHIP                  := 20707
BT_CHIP_REVISION         := A1
BT_CHIP_XTAL_FREQUENCY   := 40MHz

DEFAULT_BOARD_REVISION   := P205
BOARDS_WITH_B1_CHIP      := P205
SUPPORTED_BOARD_REVISION := $(BOARDS_WITH_B1_CHIP)
BOARD_REVISION_DIR       := board_revision

#if no board revision specified in build string
ifeq ($(BOARD_REVISION),)
BOARD_REVISION           := $(DEFAULT_BOARD_REVISION)
endif

ifneq ($(filter $(BOARD_REVISION), $(BOARDS_WITH_B0_CHIP)),)
APPS_CHIP_REVISION       := B0
else ifneq ($(filter $(BOARD_REVISION), $(BOARDS_WITH_B1_CHIP) $(BOARDS_WITH_B2_CHIP) ),)
APPS_CHIP_REVISION       := B1
endif

NAME                     := Platform_$(PLATFORM_NAME)_$(BOARD_REVISION)_$(APPS_CHIP_REVISION)


PLATFORM_SUPPORTS_BUTTONS := 1

PLATFORM_NO_DDR := 1
PLATFORM_NO_GMAC := 1

GLOBAL_DEFINES += SFLASH_SUPPORT_MACRONIX_PARTS
GLOBAL_DEFINES += WICED_DCT_INCLUDE_BT_CONFIG
# FIXME: flag for applications to disable bluetooth low-power mode on a specified platform - DON'T copy this to other platforms !!!
GLOBAL_DEFINES += DISABLE_BLUETOOTH_LPM

WICED_USB_SUPPORT := yes

WICED_BASE := ../
PLATFORM_SOURCES := $(WICED_BASE)/platforms/$(PLATFORM_DIRECTORY)/

$(NAME)_SOURCES := platform.c

GLOBAL_INCLUDES := . \
                   ./$(BOARD_REVISION_DIR)/$(BOARD_REVISION)

ifeq (,$(APP_WWD_ONLY)$(NS_WWD_ONLY)$(RTOS_WWD_ONLY))
$(NAME)_SOURCES += platform_audio.c \
                   wiced_audio.c
$(NAME)_COMPONENTS += drivers/audio/AK4954 \
                      drivers/audio/WM8533 \
                      drivers/audio/spdif
endif  # (,$(APP_WWD_ONLY)$(NS_WWD_ONLY)$(RTOS_WWD_ONLY))