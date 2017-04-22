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

NAME                        := Lib_sbc
BLUETOOTH_sbc_DIR           := ./
BLUETOOTH_ROOT_DIR          := ../
BLUETOOTH_BTE_DIR           := ../BTE/
BLUETOOTH_LIB_TYPE          := sbc
BLUETOOTH_PATH_TO_PREBUILT  := ../
BLUETOOTH_DUAL_MODE_DIR     := ../dual_mode/

ifneq ($(wildcard $(CURDIR)$(BLUETOOTH_ROOT_DIR)bluetooth_$(BLUETOOTH_LIB_TYPE).$(RTOS).$(NETWORK).$(HOST_ARCH).release.a),)

$(NAME)_PREBUILT_LIBRARY := $(BLUETOOTH_PATH_TO_PREBUILT)bluetooth_$(BLUETOOTH_LIB_TYPE).$(RTOS).$(NETWORK).$(HOST_ARCH).release.a

GLOBAL_INCLUDES :=              $(BLUETOOTH_ROOT_DIR)include \
                                $(BLUETOOTH_BTE_DIR)Components/stack/include \
                                $(BLUETOOTH_BTE_DIR)Projects/bte/main \
                                $(BLUETOOTH_BTE_DIR)WICED
else
# Build from source (Broadcom internal)
include  $(CURDIR)$(BLUETOOTH_LIB_TYPE)_src.mk
endif

