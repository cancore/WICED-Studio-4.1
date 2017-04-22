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

NAME = ASF

ASF_VERSION := 3.9.1

$(NAME)_CFLAGS := -Wstrict-prototypes  -W -Wshadow  -Wwrite-strings -std=c99 

$(NAME)_NEVER_OPTIMISE := 1

GLOBAL_INCLUDES += . \
                    ../../CMSIS \
                    asf-$(ASF_VERSION)/common/boards \
                    asf-$(ASF_VERSION)/common/services/clock \
                    asf-$(ASF_VERSION)/common/services/ioport \
                    asf-$(ASF_VERSION)/common/utils \
                    asf-$(ASF_VERSION)/sam/drivers/adc \
                    asf-$(ASF_VERSION)/sam/drivers/efc \
                    asf-$(ASF_VERSION)/sam/drivers/hsmci \
                    asf-$(ASF_VERSION)/sam/drivers/matrix \
                    asf-$(ASF_VERSION)/sam/drivers/pdc \
                    asf-$(ASF_VERSION)/sam/drivers/pio \
                    asf-$(ASF_VERSION)/sam/drivers/pmc \
                    asf-$(ASF_VERSION)/sam/drivers/pwm \
                    asf-$(ASF_VERSION)/sam/drivers/rtc \
                    asf-$(ASF_VERSION)/sam/drivers/rtt \
                    asf-$(ASF_VERSION)/sam/drivers/spi \
                    asf-$(ASF_VERSION)/sam/drivers/twi \
                    asf-$(ASF_VERSION)/sam/drivers/supc \
                    asf-$(ASF_VERSION)/sam/drivers/tc \
                    asf-$(ASF_VERSION)/sam/drivers/uart \
                    asf-$(ASF_VERSION)/sam/drivers/usart \
                    asf-$(ASF_VERSION)/sam/drivers/wdt \
                    asf-$(ASF_VERSION)/sam/utils \
                    asf-$(ASF_VERSION)/sam/utils/cmsis/sam4s/include \
                    asf-$(ASF_VERSION)/sam/utils/cmsis/sam4s/source/templates \
                    asf-$(ASF_VERSION)/sam/utils/header_files \
                    asf-$(ASF_VERSION)/sam/utils/preprocessor \
                    asf-$(ASF_VERSION)/thirdparty/CMSIS/Include \
                    asf-$(ASF_VERSION)/sam/utils/cmsis

$(NAME)_SOURCES :=  asf-$(ASF_VERSION)/common/services/clock/sam4s/sysclk.c \
                    asf-$(ASF_VERSION)/common/utils/interrupt/interrupt_sam_nvic.c \
                    asf-$(ASF_VERSION)/sam/utils/cmsis/sam4s/source/templates/system_sam4s.c \
                    asf-$(ASF_VERSION)/sam/drivers/adc/adc.c \
                    asf-$(ASF_VERSION)/sam/drivers/efc/efc.c \
                    asf-$(ASF_VERSION)/sam/drivers/matrix/matrix.c \
                    asf-$(ASF_VERSION)/sam/drivers/pdc/pdc.c \
                    asf-$(ASF_VERSION)/sam/drivers/pio/pio.c \
                    asf-$(ASF_VERSION)/sam/drivers/pio/pio_handler.c \
                    asf-$(ASF_VERSION)/sam/drivers/pmc/pmc.c \
                    asf-$(ASF_VERSION)/sam/drivers/pwm/pwm.c \
                    asf-$(ASF_VERSION)/sam/drivers/pmc/sleep.c \
                    asf-$(ASF_VERSION)/sam/drivers/spi/spi.c \
                    asf-$(ASF_VERSION)/sam/drivers/supc/supc.c \
                    asf-$(ASF_VERSION)/sam/drivers/rtc/rtc.c \
                    asf-$(ASF_VERSION)/sam/drivers/rtt/rtt.c \
                    asf-$(ASF_VERSION)/sam/drivers/tc/tc.c \
                    asf-$(ASF_VERSION)/sam/drivers/twi/twi.c \
                    asf-$(ASF_VERSION)/sam/drivers/uart/uart.c \
                    asf-$(ASF_VERSION)/sam/drivers/usart/usart.c \
                    asf-$(ASF_VERSION)/sam/drivers/wdt/wdt.c
