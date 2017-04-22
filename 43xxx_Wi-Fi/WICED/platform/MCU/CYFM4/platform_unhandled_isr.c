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
 * Defines STM32F4xx default unhandled ISR and default mappings to unhandled ISR
 */
#include <stdint.h>
#include "platform_cmsis.h"
#include "platform_assert.h"
#include "platform_constants.h"
#include "platform_isr.h"
#include "platform_isr_interface.h"
#include "wwd_rtos.h"
#include "platform_peripheral.h"

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
 *               Static Function Declarations
 ******************************************************/

extern void UnhandledInterrupt( void );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

PLATFORM_DEFINE_ISR( UnhandledInterrupt )
{
    uint32_t active_interrupt_vector = (uint32_t) ( SCB->ICSR & 0x3fU );

    /* This variable tells you which interrupt vector is currently active */
    (void)active_interrupt_vector;
    WICED_TRIGGER_BREAKPOINT( );

    /* reset the processor immeditly if not debug */
    platform_mcu_reset( );

    while( 1 )
    {
    }
}

/******************************************************
 *          Default IRQ Handler Declarations
 ******************************************************/

PLATFORM_SET_DEFAULT_ISR( NMIException           , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( HardFaultException     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( MemManageException     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( BusFaultException      , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( UsageFaultException    , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( DebugMonitor           , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ000_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ001_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ002_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ003_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ004_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ005_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ006_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ007_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ008_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ009_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ010_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ011_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ012_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ013_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ014_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ015_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ016_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ017_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ018_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ019_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ020_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ021_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ022_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ023_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ024_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ025_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ026_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ027_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ028_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ029_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ030_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ031_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ032_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ033_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ034_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ035_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ036_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ037_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ038_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ039_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ040_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ041_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ042_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ043_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ044_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ045_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ046_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ047_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ048_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ049_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ050_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ051_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ052_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ053_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ054_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ055_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ056_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ057_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ058_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ059_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ060_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ061_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ062_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ063_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ064_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ065_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ066_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ067_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ068_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ069_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ070_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ071_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ072_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ073_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ074_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ075_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ076_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ077_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ078_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ079_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ080_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ081_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ082_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ083_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ084_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ085_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ086_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ087_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ088_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ089_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ090_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ091_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ092_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ093_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ094_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ095_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ096_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ097_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ098_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ099_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ100_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ101_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ102_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ103_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ104_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ105_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ106_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ107_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ108_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ109_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ110_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ111_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ112_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ113_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ114_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ115_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ116_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ117_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ118_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ119_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ120_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ121_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ122_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ123_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ124_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ125_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ126_Handler          ,  UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR(IRQ127_Handler          ,  UnhandledInterrupt )
