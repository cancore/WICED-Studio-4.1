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

NAME := Lib_Bluetooth_Manufacturing_Test_for_BCM$(BT_CHIP)$(BT_CHIP_REVISION)

################################################################################
# Default settings                                                             #
################################################################################

ifndef BT_TRANSPORT_BUS
BT_TRANSPORT_BUS := UART
endif

################################################################################
# Add defines for specific variants and check variants for validity            #
################################################################################

ifndef BT_CHIP
$(error ERROR: BT_CHIP is undefined!)
endif

ifndef BT_CHIP_REVISION
$(error ERROR: BT_CHIP_REVISION is undefined!)
endif

ifndef BT_TRANSPORT_BUS
$(error ERROR: BT_TRANSPORT_BUS is undefined!)
endif

################################################################################
# Specify global include directories                                           #
################################################################################

GLOBAL_INCLUDES  := . \
                    ../include \
                    internal/bus \
                    internal/firmware \
                    internal/packet \
                    internal/transport/driver \
                    internal/transport/HCI \
                    internal/transport/thread

ifeq ($(BT_CHIP_XTAL_FREQUENCY),)
BT_FIRMWARE_SOURCE = ../firmware/$(BT_CHIP)$(BT_CHIP_REVISION)/bt_firmware_image.c
else
BT_FIRMWARE_SOURCE = ../firmware/$(BT_CHIP)$(BT_CHIP_REVISION)/$(BT_CHIP_XTAL_FREQUENCY)/bt_firmware_image.c
endif

$(NAME)_SOURCES  += bt_mfg_test.c \
                    internal/bus/$(BT_TRANSPORT_BUS)/bt_bus.c \
                    internal/transport/driver/$(BT_TRANSPORT_BUS)/bt_transport_driver_receive.c \
                    internal/transport/driver/$(BT_TRANSPORT_BUS)/bt_transport_driver.c \
                    internal/transport/thread/bt_transport_thread.c \
                    internal/packet/bt_packet.c \
                    internal/firmware/bt_firmware.c \
                    $(BT_FIRMWARE_SOURCE)


$(NAME)_COMPONENTS += utilities/linked_list