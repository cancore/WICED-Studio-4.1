/*
 * Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/** @file
 * STM32F4xx vector table
 */
#include <stdint.h>
#include "platform_cmsis.h"
#include "platform_assert.h"
#include "platform_constants.h"
#include "platform_isr.h"
#include "platform_isr_interface.h"
#include "wwd_rtos_isr.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#ifndef SVC_irq
#error SVC_irq not defined - this will probably cause RTOS to fail to run
#endif

#ifndef PENDSV_irq
#error PENDSV_irq not defined - this will probably cause RTOS to fail to run
#endif

#ifndef SYSTICK_irq
#error SYSTICK_irq not defined - this will probably cause RTOS to fail to run
#endif

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

extern void UnhandledInterrupt( void );
extern void reset_handler     ( void );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* Pointer to stack location */
extern void* link_stack_end;

PLATFORM_DEFINE_INTERRUPT_VECTOR_TABLE_ARRAY( interrupt_vector_table, PLATFORM_INTERRUPT_VECTOR_TABLE_HAS_VARIABLE_SIZE ) =
{
    (uint32_t)&link_stack_end       , // 0  Initial stack location
    (uint32_t)reset_handler         , // 1  Reset vector
    (uint32_t)NMIException          , // 2  Non Maskable Interrupt
    (uint32_t)HardFaultException    , // 3  Hard Fault interrupt
    (uint32_t)MemManageException    , // 4  Memory Management Fault interrupt
    (uint32_t)BusFaultException     , // 5  Bus Fault interrupt
    (uint32_t)UsageFaultException   , // 6  Usage Fault interrupt
    (uint32_t)0                     , // 7  Reserved
    (uint32_t)0                     , // 8  Reserved
    (uint32_t)0                     , // 9  Reserved
    (uint32_t)0                     , // 10 Reserved
    (uint32_t)SVC_irq               , // 11 SVC interrupt
    (uint32_t)DebugMonitor          , // 12 Debug Monitor interrupt
    (uint32_t)0                     , // 13 Reserved
    (uint32_t)PENDSV_irq            , // 14 PendSV interrupt
    (uint32_t)SYSTICK_irq           , // 15 Sys Tick Interrupt

    /* Note: renaming to device dependent ISR function names are done in
     *       pdl.h (section "IRQ name definition for all type MCUs")
     */
    (uint32_t)IRQ000_Handler        , // 16 IRQ000
    (uint32_t)IRQ001_Handler        , // 17 IRQ001
    (uint32_t)IRQ002_Handler        , // 18 IRQ002
    (uint32_t)IRQ003_Handler        , // 19 IRQ003
    (uint32_t)IRQ004_Handler        , // 20 IRQ004
    (uint32_t)IRQ005_Handler        , // 21 IRQ005
    (uint32_t)IRQ006_Handler        , // 22 IRQ006
    (uint32_t)IRQ007_Handler        , // 23 IRQ007
    (uint32_t)IRQ008_Handler        , // 24 IRQ008
    (uint32_t)IRQ009_Handler        , // 25 IRQ009
    (uint32_t)IRQ010_Handler        , // 26 IRQ010
    (uint32_t)IRQ011_Handler        , // 27 IRQ011
    (uint32_t)IRQ012_Handler        , // 28 IRQ012
    (uint32_t)IRQ013_Handler        , // 29 IRQ013
    (uint32_t)IRQ014_Handler        , // 30 IRQ014
    (uint32_t)IRQ015_Handler        , // 31 IRQ015
    (uint32_t)IRQ016_Handler        , // 32 IRQ016
    (uint32_t)IRQ017_Handler        , // 33 IRQ017
    (uint32_t)IRQ018_Handler        , // 34 IRQ018
    (uint32_t)IRQ019_Handler        , // 35 IRQ019
    (uint32_t)IRQ020_Handler        , // 36 IRQ020
    (uint32_t)IRQ021_Handler        , // 37 IRQ021
    (uint32_t)IRQ022_Handler        , // 38 IRQ022
    (uint32_t)IRQ023_Handler        , // 39 IRQ023
    (uint32_t)IRQ024_Handler        , // 40 IRQ024
    (uint32_t)IRQ025_Handler        , // 41 IRQ025
    (uint32_t)IRQ026_Handler        , // 42 IRQ026
    (uint32_t)IRQ027_Handler        , // 43 IRQ027
    (uint32_t)IRQ028_Handler        , // 44 IRQ028
    (uint32_t)IRQ029_Handler        , // 45 IRQ029
    (uint32_t)IRQ030_Handler        , // 46 IRQ030
    (uint32_t)IRQ031_Handler        , // 47 IRQ031
    (uint32_t)IRQ032_Handler        , // 48 IRQ032
    (uint32_t)IRQ033_Handler        , // 49 IRQ033
    (uint32_t)IRQ034_Handler        , // 50 IRQ034
    (uint32_t)IRQ035_Handler        , // 51 IRQ035
    (uint32_t)IRQ036_Handler        , // 52 IRQ036
    (uint32_t)IRQ037_Handler        , // 53 IRQ037
    (uint32_t)IRQ038_Handler        , // 54 IRQ038
    (uint32_t)IRQ039_Handler        , // 55 IRQ039
    (uint32_t)IRQ040_Handler        , // 56 IRQ040
    (uint32_t)IRQ041_Handler        , // 57 IRQ041
    (uint32_t)IRQ042_Handler        , // 58 IRQ042
    (uint32_t)IRQ043_Handler        , // 59 IRQ043
    (uint32_t)IRQ044_Handler        , // 60 IRQ044
    (uint32_t)IRQ045_Handler        , // 61 IRQ045
    (uint32_t)IRQ046_Handler        , // 62 IRQ046
    (uint32_t)IRQ047_Handler        , // 63 IRQ047
    (uint32_t)IRQ048_Handler        , // 64 IRQ048
    (uint32_t)IRQ049_Handler        , // 65 IRQ049
    (uint32_t)IRQ050_Handler        , // 66 IRQ050
    (uint32_t)IRQ051_Handler        , // 67 IRQ051
    (uint32_t)IRQ052_Handler        , // 68 IRQ052
    (uint32_t)IRQ053_Handler        , // 69 IRQ053
    (uint32_t)IRQ054_Handler        , // 70 IRQ054
    (uint32_t)IRQ055_Handler        , // 71 IRQ055
    (uint32_t)IRQ056_Handler        , // 72 IRQ056
    (uint32_t)IRQ057_Handler        , // 73 IRQ057
    (uint32_t)IRQ058_Handler        , // 74 IRQ058
    (uint32_t)IRQ059_Handler        , // 75 IRQ059
    (uint32_t)IRQ060_Handler        , // 76 IRQ060
    (uint32_t)IRQ061_Handler        , // 77 IRQ061
    (uint32_t)IRQ062_Handler        , // 78 IRQ062
    (uint32_t)IRQ063_Handler        , // 79 IRQ063
    (uint32_t)IRQ064_Handler        , // 80 IRQ064
    (uint32_t)IRQ065_Handler        , // 81 IRQ065
    (uint32_t)IRQ066_Handler        , // 82 IRQ066
    (uint32_t)IRQ067_Handler        , // 83 IRQ067
    (uint32_t)IRQ068_Handler        , // 84 IRQ068
    (uint32_t)IRQ069_Handler        , // 85 IRQ069
    (uint32_t)IRQ070_Handler        , // 86 IRQ070
    (uint32_t)IRQ071_Handler        , // 87 IRQ071
    (uint32_t)IRQ072_Handler        , // 88 IRQ072
    (uint32_t)IRQ073_Handler        , // 89 IRQ073
    (uint32_t)IRQ074_Handler        , // 90 IRQ074
    (uint32_t)IRQ075_Handler        , // 91 IRQ075
    (uint32_t)IRQ076_Handler        , // 92 IRQ076
    (uint32_t)IRQ077_Handler        , // 93 IRQ077
    (uint32_t)IRQ078_Handler        , // 94 IRQ078
    (uint32_t)IRQ079_Handler        , // 95 IRQ079
    (uint32_t)IRQ080_Handler        , // 96 IRQ080
    (uint32_t)IRQ081_Handler        , // 97 IRQ081
    (uint32_t)IRQ082_Handler        , // 98 IRQ082
    (uint32_t)IRQ083_Handler        , // 99 IRQ083
    (uint32_t)IRQ084_Handler        , // 100 IRQ084
    (uint32_t)IRQ085_Handler        , // 101 IRQ085
    (uint32_t)IRQ086_Handler        , // 102 IRQ086
    (uint32_t)IRQ087_Handler        , // 103 IRQ087
    (uint32_t)IRQ088_Handler        , // 104 IRQ088
    (uint32_t)IRQ089_Handler        , // 105 IRQ089
    (uint32_t)IRQ090_Handler        , // 106 IRQ090
    (uint32_t)IRQ091_Handler        , // 107 IRQ091
    (uint32_t)IRQ092_Handler        , // 108 IRQ092
    (uint32_t)IRQ093_Handler        , // 109 IRQ093
    (uint32_t)IRQ094_Handler        , // 110 IRQ094
    (uint32_t)IRQ095_Handler        , // 111 IRQ095
    (uint32_t)IRQ096_Handler        , // 112 IRQ096
    (uint32_t)IRQ097_Handler        , // 113 IRQ097
    (uint32_t)IRQ098_Handler        , // 114 IRQ098
    (uint32_t)IRQ099_Handler        , // 115 IRQ099
    (uint32_t)IRQ100_Handler        , // 116 IRQ100
    (uint32_t)IRQ101_Handler        , // 117 IRQ101
    (uint32_t)IRQ102_Handler        , // 118 IRQ102
    (uint32_t)IRQ103_Handler        , // 119 IRQ103
    (uint32_t)IRQ104_Handler        , // 120 IRQ104
    (uint32_t)IRQ105_Handler        , // 121 IRQ105
    (uint32_t)IRQ106_Handler        , // 122 IRQ106
    (uint32_t)IRQ107_Handler        , // 123 IRQ107
    (uint32_t)IRQ108_Handler        , // 124 IRQ108
    (uint32_t)IRQ109_Handler        , // 125 IRQ109
    (uint32_t)IRQ110_Handler        , // 126 IRQ110
    (uint32_t)IRQ111_Handler        , // 127 IRQ111
    (uint32_t)IRQ112_Handler        , // 128 IRQ112
    (uint32_t)IRQ113_Handler        , // 129 IRQ113
    (uint32_t)IRQ114_Handler        , // 120 IRQ114
    (uint32_t)IRQ115_Handler        , // 121 IRQ115
    (uint32_t)IRQ116_Handler        , // 122 IRQ116
    (uint32_t)IRQ117_Handler        , // 123 IRQ117
    (uint32_t)IRQ118_Handler        , // 124 IRQ118
    (uint32_t)IRQ119_Handler        , // 125 IRQ119
    (uint32_t)IRQ120_Handler        , // 126 IRQ120
    (uint32_t)IRQ121_Handler        , // 127 IRQ121
    (uint32_t)IRQ122_Handler        , // 128 IRQ122
    (uint32_t)IRQ123_Handler        , // 129 IRQ123
    (uint32_t)IRQ124_Handler        , // 130 IRQ124
    (uint32_t)IRQ125_Handler        , // 131 IRQ125
    (uint32_t)IRQ126_Handler        , // 132 IRQ126
    (uint32_t)IRQ127_Handler        , // 133 IRQ127
};

/******************************************************
 *               Function Definitions
 ******************************************************/
