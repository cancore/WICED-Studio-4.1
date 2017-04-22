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

NAME := App_Bluetooth_Audio

$(NAME)_SOURCES    := bluetooth_audio.c \
                      bluetooth_audio_nv.c

$(NAME)_COMPONENTS := utilities/command_console \
                      utilities/wiced_log

PLATFORMS_FOR_WICED_HCI := BCM943907WCD2

ifeq ($(filter $(PLATFORM),$(PLATFORMS_FOR_WICED_HCI) ), $(PLATFORM))
GLOBAL_DEFINES     := USE_WICED_HCI
endif

ifneq (,$(findstring USE_WICED_HCI,$(GLOBAL_DEFINES)))
$(info "Embedded Stack")
$(NAME)_COMPONENTS += libraries/drivers/bluetooth/wiced_hci_bt\
                      libraries/protocols/wiced_hci
$(NAME)_SOURCES    += bluetooth_audio_common_wiced_hci.c
GLOBAL_INCLUDES   += libraries/drivers/bluetooth/wiced_hci_bt
# TODO: remove the CHIP, CHIP REVISION, XTAL FREQUENCY when the actual board is used
# These params will already be defined in the platform files.
EMBEDDED_APP_NAME := headset
BT_CHIP      := 20706
BT_CHIP_REVISION  := A2
BT_CHIP_XTAL_FREQUENCY := 20MHz

VALID_PLATFORMS    := BCM943907WCD2*
INVALID_PLATFORMS  := BCM9WCD1AUDIO BCM943909* BCM943907WAE_1* BCM943907APS* BCM943907WCD1* BCM943909QT
else
$(info "Host Stack")
$(NAME)_COMPONENTS += libraries/drivers/bluetooth/dual_mode \
                      libraries/audio/codec/codec_framework \
                      libraries/audio/codec/sbc_if

$(NAME)_SOURCES += bluetooth_audio_player.c \
                   bluetooth_audio_decoder.c

VALID_PLATFORMS    := BCM9WCD1AUDIO BCM943909* BCM943907WAE* BCM943907APS* BCM943907WCD1*
INVALID_PLATFORMS  := BCM943909QT BCM943907WCD2*
endif

BT_CONFIG_DCT_H := bt_config_dct.h

GLOBAL_DEFINES += BUILDCFG \
                  WICED_USE_AUDIO \
                  WICED_NO_WIFI \
                  NO_WIFI_FIRMWARE \
                  TX_PACKET_POOL_SIZE=1 \
                  RX_PACKET_POOL_SIZE=1 \
                  USE_MEM_POOL \
                  WICED_DCT_INCLUDE_BT_CONFIG

#GLOBAL_DEFINES += WICED_DISABLE_WATCHDOG

ifneq (,$(findstring USE_MEM_POOL,$(GLOBAL_DEFINES)))
$(NAME)_SOURCES   += mem_pool/mem_pool.c
$(NAME)_INCLUDES  += ./mem_pool
#GLOBAL_DEFINES    += MEM_POOL_DEBUG
endif

PLATFORM_USING_AUDIO_PLL := BCM943909WCD1_3* BCM943909B0FCBU BCM943907WAE_1* BCM943907WAE2_1* BCM943907APS* BCM943907WCD1*
$(eval PLATFORM_USING_AUDIO_PLL := $(call EXPAND_WILDCARD_PLATFORMS,$(PLATFORM_USING_AUDIO_PLL)))
ifneq ($(filter $(PLATFORM),$(PLATFORM_USING_AUDIO_PLL)),)
# Use h/w audio PLL to tweak the master clock going into the I2S engine
GLOBAL_DEFINES     += USE_AUDIO_PLL
$(NAME)_SOURCES    += bluetooth_audio_pll_tuning.c
$(NAME)_COMPONENTS += audio/apollo/audio_pll_tuner
endif

# Define ENABLE_BT_PROTOCOL_TRACES to enable Bluetooth protocol/profile level
# traces.
#GLOBAL_DEFINES     += ENABLE_BT_PROTOCOL_TRACES

NO_WIFI_FIRMWARE   := YES
