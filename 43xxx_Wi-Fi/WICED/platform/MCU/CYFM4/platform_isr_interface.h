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
 * Declares ISR prototypes for STM32F4xx MCU family
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

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
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

extern void NMIException           ( void );  // 2  Non Maskable Interrupt
extern void HardFaultException     ( void );  // 3  Hard Fault interrupt
extern void MemManageException     ( void );  // 4  Memory Management Fault interrupt
extern void BusFaultException      ( void );  // 5  Bus Fault interrupt
extern void UsageFaultException    ( void );  // 6  Usage Fault interrupt
extern void SVC_irq                ( void );  // 11 SVC interrupt
extern void DebugMonitor           ( void );  // 12 Debug Monitor interrupt
extern void PENDSV_irq             ( void );  // 14 PendSV interrupt
extern void SYSTICK_irq            ( void );  // 15 Sys Tick Interrupt
/* Note: renaming to device dependent ISR function names are done in
 *       pdl.h (section "IRQ name definition for all type MCUs")
 */
extern void IRQ000_Handler         ( void );  // 16 IRQ000
extern void IRQ001_Handler         ( void );  // 17 IRQ001
extern void IRQ002_Handler         ( void );  // 18 IRQ002
extern void IRQ003_Handler         ( void );  // 19 IRQ003
extern void IRQ004_Handler         ( void );  // 20 IRQ004
extern void IRQ005_Handler         ( void );  // 21 IRQ005
extern void IRQ006_Handler         ( void );  // 22 IRQ006
extern void IRQ007_Handler         ( void );  // 23 IRQ007
extern void IRQ008_Handler         ( void );  // 24 IRQ008
extern void IRQ009_Handler         ( void );  // 25 IRQ009
extern void IRQ010_Handler         ( void );  // 26 IRQ010
extern void IRQ011_Handler         ( void );  // 27 IRQ011
extern void IRQ012_Handler         ( void );  // 28 IRQ012
extern void IRQ013_Handler         ( void );  // 29 IRQ013
extern void IRQ014_Handler         ( void );  // 30 IRQ014
extern void IRQ015_Handler         ( void );  // 31 IRQ015
extern void IRQ016_Handler         ( void );  // 32 IRQ016
extern void IRQ017_Handler         ( void );  // 33 IRQ017
extern void IRQ018_Handler         ( void );  // 34 IRQ018
extern void IRQ019_Handler         ( void );  // 35 IRQ019
extern void IRQ020_Handler         ( void );  // 36 IRQ020
extern void IRQ021_Handler         ( void );  // 37 IRQ021
extern void IRQ022_Handler         ( void );  // 38 IRQ022
extern void IRQ023_Handler         ( void );  // 39 IRQ023
extern void IRQ024_Handler         ( void );  // 40 IRQ024
extern void IRQ025_Handler         ( void );  // 41 IRQ025
extern void IRQ026_Handler         ( void );  // 42 IRQ026
extern void IRQ027_Handler         ( void );  // 43 IRQ027
extern void IRQ028_Handler         ( void );  // 44 IRQ028
extern void IRQ029_Handler         ( void );  // 45 IRQ029
extern void IRQ030_Handler         ( void );  // 46 IRQ030
extern void IRQ031_Handler         ( void );  // 47 IRQ031
extern void IRQ032_Handler         ( void );  // 48 IRQ032
extern void IRQ033_Handler         ( void );  // 49 IRQ033
extern void IRQ034_Handler         ( void );  // 50 IRQ034
extern void IRQ035_Handler         ( void );  // 51 IRQ035
extern void IRQ036_Handler         ( void );  // 52 IRQ036
extern void IRQ037_Handler         ( void );  // 53 IRQ037
extern void IRQ038_Handler         ( void );  // 54 IRQ038
extern void IRQ039_Handler         ( void );  // 55 IRQ039
extern void IRQ040_Handler         ( void );  // 56 IRQ040
extern void IRQ041_Handler         ( void );  // 57 IRQ041
extern void IRQ042_Handler         ( void );  // 58 IRQ042
extern void IRQ043_Handler         ( void );  // 59 IRQ043
extern void IRQ044_Handler         ( void );  // 60 IRQ044
extern void IRQ045_Handler         ( void );  // 61 IRQ045
extern void IRQ046_Handler         ( void );  // 62 IRQ046
extern void IRQ047_Handler         ( void );  // 63 IRQ047
extern void IRQ048_Handler         ( void );  // 64 IRQ048
extern void IRQ049_Handler         ( void );  // 65 IRQ049
extern void IRQ050_Handler         ( void );  // 66 IRQ050
extern void IRQ051_Handler         ( void );  // 67 IRQ051
extern void IRQ052_Handler         ( void );  // 68 IRQ052
extern void IRQ053_Handler         ( void );  // 69 IRQ053
extern void IRQ054_Handler         ( void );  // 70 IRQ054
extern void IRQ055_Handler         ( void );  // 71 IRQ055
extern void IRQ056_Handler         ( void );  // 72 IRQ056
extern void IRQ057_Handler         ( void );  // 73 IRQ057
extern void IRQ058_Handler         ( void );  // 74 IRQ058
extern void IRQ059_Handler         ( void );  // 75 IRQ059
extern void IRQ060_Handler         ( void );  // 76 IRQ060
extern void IRQ061_Handler         ( void );  // 77 IRQ061
extern void IRQ062_Handler         ( void );  // 78 IRQ062
extern void IRQ063_Handler         ( void );  // 79 IRQ063
extern void IRQ064_Handler         ( void );  // 80 IRQ064
extern void IRQ065_Handler         ( void );  // 81 IRQ065
extern void IRQ066_Handler         ( void );  // 82 IRQ066
extern void IRQ067_Handler         ( void );  // 83 IRQ067
extern void IRQ068_Handler         ( void );  // 84 IRQ068
extern void IRQ069_Handler         ( void );  // 85 IRQ069
extern void IRQ070_Handler         ( void );  // 86 IRQ070
extern void IRQ071_Handler         ( void );  // 87 IRQ071
extern void IRQ072_Handler         ( void );  // 88 IRQ072
extern void IRQ073_Handler         ( void );  // 89 IRQ073
extern void IRQ074_Handler         ( void );  // 90 IRQ074
extern void IRQ075_Handler         ( void );  // 91 IRQ075
extern void IRQ076_Handler         ( void );  // 92 IRQ076
extern void IRQ077_Handler         ( void );  // 93 IRQ077
extern void IRQ078_Handler         ( void );  // 94 IRQ078
extern void IRQ079_Handler         ( void );  // 95 IRQ079
extern void IRQ080_Handler         ( void );  // 96 IRQ080
extern void IRQ081_Handler         ( void );  // 97 IRQ081
extern void IRQ082_Handler         ( void );  // 98 IRQ082
extern void IRQ083_Handler         ( void );  // 99 IRQ083
extern void IRQ084_Handler         ( void );  // 100 IRQ084
extern void IRQ085_Handler         ( void );  // 101 IRQ085
extern void IRQ086_Handler         ( void );  // 102 IRQ086
extern void IRQ087_Handler         ( void );  // 103 IRQ087
extern void IRQ088_Handler         ( void );  // 104 IRQ088
extern void IRQ089_Handler         ( void );  // 105 IRQ089
extern void IRQ090_Handler         ( void );  // 106 IRQ090
extern void IRQ091_Handler         ( void );  // 107 IRQ091
extern void IRQ092_Handler         ( void );  // 108 IRQ092
extern void IRQ093_Handler         ( void );  // 109 IRQ093
extern void IRQ094_Handler         ( void );  // 110 IRQ094
extern void IRQ095_Handler         ( void );  // 111 IRQ095
extern void IRQ096_Handler         ( void );  // 112 IRQ096
extern void IRQ097_Handler         ( void );  // 113 IRQ097
extern void IRQ098_Handler         ( void );  // 114 IRQ098
extern void IRQ099_Handler         ( void );  // 115 IRQ099
extern void IRQ100_Handler         ( void );  // 116 IRQ100
extern void IRQ101_Handler         ( void );  // 117 IRQ101
extern void IRQ102_Handler         ( void );  // 118 IRQ102
extern void IRQ103_Handler         ( void );  // 119 IRQ103
extern void IRQ104_Handler         ( void );  // 120 IRQ104
extern void IRQ105_Handler         ( void );  // 121 IRQ105
extern void IRQ106_Handler         ( void );  // 122 IRQ106
extern void IRQ107_Handler         ( void );  // 123 IRQ107
extern void IRQ108_Handler         ( void );  // 124 IRQ108
extern void IRQ109_Handler         ( void );  // 125 IRQ109
extern void IRQ110_Handler         ( void );  // 126 IRQ110
extern void IRQ111_Handler         ( void );  // 127 IRQ111
extern void IRQ112_Handler         ( void );  // 128 IRQ112
extern void IRQ113_Handler         ( void );  // 129 IRQ113
extern void IRQ114_Handler         ( void );  // 120 IRQ114
extern void IRQ115_Handler         ( void );  // 121 IRQ115
extern void IRQ116_Handler         ( void );  // 122 IRQ116
extern void IRQ117_Handler         ( void );  // 123 IRQ117
extern void IRQ118_Handler         ( void );  // 124 IRQ118
extern void IRQ119_Handler         ( void );  // 125 IRQ119
extern void IRQ120_Handler         ( void );  // 126 IRQ120
extern void IRQ121_Handler         ( void );  // 127 IRQ121
extern void IRQ122_Handler         ( void );  // 128 IRQ122
extern void IRQ123_Handler         ( void );  // 129 IRQ123
extern void IRQ124_Handler         ( void );  // 130 IRQ124
extern void IRQ125_Handler         ( void );  // 131 IRQ125
extern void IRQ126_Handler         ( void );  // 132 IRQ126
extern void IRQ127_Handler         ( void );  // 133 IRQ127

#ifdef __cplusplus
} /* extern "C" */
#endif

