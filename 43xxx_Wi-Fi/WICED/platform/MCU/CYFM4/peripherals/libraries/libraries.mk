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

NAME = CYFM4_Peripheral_Libraries

GLOBAL_INCLUDES :=  .                                       \
                    common                                  \
                    drivers                                 \
                    drivers/adc                             \
                    drivers/bt                              \
                    drivers/can                             \
                    drivers/clk                             \
                    drivers/cr                              \
                    drivers/crc                             \
                    drivers/csv                             \
                    drivers/dac                             \
                    drivers/dma                             \
                    drivers/dstc                            \
                    drivers/dt                              \
                    drivers/exint                           \
                    drivers/extif                           \
                    drivers/flash                           \
                    drivers/gpio                            \
                    drivers/hbif                            \
                    drivers/hsspi                           \
                    drivers/i2cs                            \
                    drivers/i2s                             \
                    drivers/icc                             \
                    drivers/lcd                             \
                    drivers/lpm                             \
                    drivers/lvd                             \
                    drivers/mfs                             \
                    drivers/mft                             \
                    drivers/pcrc                            \
                    drivers/ppg                             \
                    drivers/qprc                            \
                    drivers/rc                              \
                    drivers/reset                           \
                    drivers/rtc                             \
                    drivers/sdif                            \
                    drivers/uid                             \
                    drivers/vbat                            \
                    drivers/wc                              \
                    drivers/wdg                             \
                    utilities                               \
                    ../../../$(HOST_ARCH)/CMSIS             \
                    devices/fm4/$(HOST_MCU_PART_NUMBER)/common  
                    
$(NAME)_SOURCES := \
                    drivers/adc/adc.c                       \
                    drivers/bt/bt.c                         \
                    drivers/can/can_pre.c                   \
                    drivers/can/can.c                       \
                    drivers/can/canfd.c                     \
                    drivers/clk/clk.c                       \
                    drivers/cr/cr.c                         \
                    drivers/crc/crc.c                       \
                    drivers/csv/csv.c                       \
                    drivers/dac/dac.c                       \
                    drivers/dma/dma.c                       \
                    drivers/dstc/dstc.c                     \
                    drivers/dt/dt.c                         \
                    drivers/exint/exint.c                   \
                    drivers/extif/extif.c                   \
                    drivers/flash/dualflash.c               \
                    drivers/flash/mainflash.c               \
                    drivers/flash/workflash.c               \
                    drivers/hbif/hbif.c                     \
                    drivers/hsspi/hsspi.c                   \
                    drivers/i2cs/i2cs.c                     \
                    drivers/i2s/i2s.c                       \
                    drivers/icc/icc.c                       \
                    drivers/lcd/lcd.c                       \
                    drivers/lpm/lpm.c                       \
                    drivers/lvd/lvd.c                       \
                    drivers/mfs/mfs.c                       \
                    drivers/mft/mft_adcmp.c                 \
                    drivers/mft/mft_frt.c                   \
                    drivers/mft/mft_icu.c                   \
                    drivers/mft/mft_ocu.c                   \
                    drivers/mft/mft_wfg.c                   \
                    drivers/pcrc/pcrc.c                     \
                    drivers/ppg/ppg.c                       \
                    drivers/qprc/qprc.c                     \
                    drivers/rc/rc.c                         \
                    drivers/reset/reset.c                   \
                    drivers/rtc/rtc.c                       \
                    drivers/sdif/sdif.c                     \
                    drivers/uid/uid.c                       \
                    drivers/vbat/vbat.c                     \
                    drivers/wc/wc.c                         \
                    drivers/wdg/hwwdg.c                     \
                    drivers/wdg/swwdg.c                     \
                    drivers/interrupts_fm0p_type_1-a.c      \
                    drivers/interrupts_fm0p_type_1-b.c      \
                    drivers/interrupts_fm0p_type_2-a.c      \
                    drivers/interrupts_fm0p_type_2-b.c      \
                    drivers/interrupts_fm0p_type_3.c        \
                    drivers/interrupts_fm3_type_a.c         \
                    drivers/interrupts_fm3_type_b.c         \
                    drivers/interrupts_fm3_type_c.c         \
                    drivers/interrupts_fm4_type_a.c         \
                    drivers/interrupts_fm4_type_b.c         \
                    drivers/interrupts_fm4_type_c.c         \
                    drivers/pdl.c                           \
                    utilities/eeprom/i2c_int_at24cxx.c      \
                    utilities/eeprom/i2c_polling_at24cxx.c  \
                    utilities/hyper_flash/s26kl512s.c       \
                    utilities/i2s_codec/wm8731.c            \
                    utilities/nand_flash/s34ml01g.c         \
                    utilities/printf_scanf/uart_io.c        \
                    utilities/qspi_flash/flashS25FL164K.c   \
                    utilities/sd_card/sd_card.c             \
                    utilities/sd_card/sd_cmd.c              \
                    utilities/sdram/is42s16800.c            \
                    utilities/seg_lcd/cl010/cl010.c         \
                    utilities/seg_lcd/tsdh1188/tsdh1188.c   \
                    devices/fm4/$(HOST_MCU_PART_NUMBER)/common/system_$(HOST_MCU_VARIANT).c

ifeq ($(TOOLCHAIN_NAME),GCC)
GLOBAL_CFLAGS += -fms-extensions -fno-strict-aliasing -Wno-missing-braces # Required for compiling unnamed structure and union fields
endif