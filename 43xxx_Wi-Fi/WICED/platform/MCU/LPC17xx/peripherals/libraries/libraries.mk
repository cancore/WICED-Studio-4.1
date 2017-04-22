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

NAME = LPC17xx_Peripheral_Libraries

GLOBAL_INCLUDES :=  . \
                    inc \
                    ../../../$(HOST_ARCH)/CMSIS

$(NAME)_SOURCES := \
                   src/adc_17xx_40xx.c \
                   src/clock_17xx_40xx.c \
                   src/eeprom_17xx_40xx.c \
                   src/gpdma_17xx_40xx.c \
                   src/i2c_17xx_40xx.c \
                   src/i2s_17xx_40xx.c \
                   src/iocon_17xx_40xx.c \
                   src/ritimer_17xx_40xx.c \
                   src/rtc_17xx_40xx.c \
                   src/sdc_17xx_40xx.c \
                   src/sdmmc_17xx_40xx.c \
                   src/spi_17xx_40xx.c \
                   src/ssp_17xx_40xx.c \
                   src/sysctl_17xx_40xx.c \
                   src/timer_17xx_40xx.c \
                   src/uart_17xx_40xx.c \
                   src/wwdt_17xx_40xx.c \
                   src/atimer_001.c \
                   src/eeprom_001.c \
                   src/eeprom_002.c \
                   src/gpdma_001.c \
                   src/gpio_003.c \
                   src/gpiogrpint_001.c \
                   src/gpioint_001.c \
                   src/gpiopinint_001.c \
                   src/i2c_001.c \
                   src/i2s_001.c \
                   src/ritimer_001.c \
                   src/rtc_001.c \
                   src/sct_001.c \
                   src/sdc_001.c \
                   src/sdmmc_001.c \
                   src/spi_002.c \
                   src/ssp_001.c \
                   src/timer_001.c \
                   src/usart_001.c \
                   src/usart_002.c \
                   src/usart_004.c \
                   src/wwdt_001.c
