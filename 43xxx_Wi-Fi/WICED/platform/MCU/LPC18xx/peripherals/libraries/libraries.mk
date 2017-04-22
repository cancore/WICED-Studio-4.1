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

NAME = LPC18xx_Peripheral_Libraries

GLOBAL_INCLUDES :=  . \
                    inc \
                    ../../../$(HOST_ARCH)/CMSIS

$(NAME)_SOURCES := \
                    src/adc_18xx_43xx.c      \
                    src/aes_18xx_43xx.c      \
                    src/atimer_18xx_43xx.c   \
                    src/ccan_18xx_43xx.c     \
                    src/chip_18xx_43xx.c     \
                    src/clock_18xx_43xx.c    \
                    src/dac_18xx_43xx.c      \
                    src/eeprom_18xx_43xx.c   \
                    src/emc_18xx_43xx.c      \
                    src/enet_18xx_43xx.c     \
                    src/evrt_18xx_43xx.c     \
                    src/gpdma_18xx_43xx.c    \
                    src/gpio_18xx_43xx.c     \
                    src/gpiogroup_18xx_43xx.c\
                    src/i2c_18xx_43xx.c      \
                    src/i2cm_18xx_43xx.c     \
                    src/i2s_18xx_43xx.c      \
                    src/iap_18xx_43xx.c      \
                    src/lcd_18xx_43xx.c      \
                    src/otp_18xx_43xx.c      \
                    src/pinint_18xx_43xx.c   \
                    src/pmc_18xx_43xx.c      \
                    src/rgu_18xx_43xx.c      \
                    src/ring_buffer.c        \
                    src/ritimer_18xx_43xx.c  \
                    src/rtc_18xx_43xx.c      \
                    src/sct_18xx_43xx.c      \
                    src/sct_pwm_18xx_43xx.c  \
                    src/scu_18xx_43xx.c      \
                    src/sdif_18xx_43xx.c     \
                    src/sdmmc_18xx_43xx.c    \
                    src/spi_18xx_43xx.c      \
                    src/ssp_18xx_43xx.c      \
                    src/stopwatch_18xx_43xx.c\
                    src/sysinit_18xx_43xx.c  \
                    src/timer_18xx_43xx.c    \
                    src/uart_18xx_43xx.c     \
                    src/wwdt_18xx_43xx.c     \
                    src/eeprom.c             \
                    src/fpu_init.c           \
                    src/spifilib_dev_common.c \
                    src/spifilib_fam_standard_cmd.c \