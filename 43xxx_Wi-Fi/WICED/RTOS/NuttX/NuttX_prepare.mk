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

.PHONY: $(MAKECMDGOALS) configure_nuttx
$(MAKECMDGOALS): configure_nuttx

NUTTX_BOARD_DEFAULT := bcm4390x-wcd1_3
NUTTX_PLATFORM_DEFAULT := bcm4390x

NUTTX_COPY_COMMAND := "$(COMMON_TOOLS_PATH)cp" --remove-destination
NUTTX_RECURSIVE_COPY_COMMAND := $(NUTTX_COPY_COMMAND) --recursive
NUTTX_CREATE_DIR := "$(COMMON_TOOLS_PATH)mkdir" -p

NUTTX_DESTINATION_DIR := $(OUTPUT_DIR)/$(NUTTX_FULL_PATH)

ifeq (,$(NUTTX_BOARD))
ifneq (,$(wildcard $(NUTTX_FULL_PATH)/configs/$(PLATFORM)))
NUTTX_BOARD := $(PLATFORM)
else
NUTTX_BOARD := $(NUTTX_BOARD_DEFAULT)
endif
endif

ifeq (,$(NUTTX_PLATFORM))
NUTTX_PLATFORM := $(NUTTX_PLATFORM_DEFAULT)
endif

configure_nuttx:
	$(QUIET)$(ECHO) Configuring NuttX: NuttX board $(NUTTX_BOARD), NuttX platform $(NUTTX_PLATFORM), WICED platform $(PLATFORM)
	$(QUIET)$(ECHO) Create directories
	$(NUTTX_CREATE_DIR) $(NUTTX_DESTINATION_DIR)/include/nuttx
	$(NUTTX_CREATE_DIR) $(NUTTX_DESTINATION_DIR)/include/arch/chip
	$(NUTTX_CREATE_DIR) $(NUTTX_DESTINATION_DIR)/include/arch/board
	$(QUIET)$(ECHO) Copy math header
	$(NUTTX_COPY_COMMAND) $(NUTTX_FULL_PATH)/include/nuttx/math.h $(NUTTX_DESTINATION_DIR)/include/
	$(QUIET)$(ECHO) Copy arch headers
	$(NUTTX_RECURSIVE_COPY_COMMAND) $(NUTTX_FULL_PATH)/arch/arm/include/* $(NUTTX_DESTINATION_DIR)/include/arch/
	$(QUIET)$(ECHO) Copy chip headers
	$(NUTTX_RECURSIVE_COPY_COMMAND) $(NUTTX_FULL_PATH)/arch/arm/include/$(NUTTX_PLATFORM)/* $(NUTTX_DESTINATION_DIR)/include/arch/chip/
	$(QUIET)$(ECHO) Copy board headers
	$(NUTTX_RECURSIVE_COPY_COMMAND) $(NUTTX_FULL_PATH)/configs/$(NUTTX_BOARD)/include/* $(NUTTX_DESTINATION_DIR)/include/arch/board/
	$(QUIET)$(ECHO) Copy application configuration
	$(NUTTX_COPY_COMMAND) $(NUTTX_FULL_PATH)/configs/$(NUTTX_BOARD)/$(NUTTX_APP)/config.h $(NUTTX_DESTINATION_DIR)/include/nuttx/
