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

MEMORY
{
    BTLDR_VECTORS    (rx)  : ORIGIN = 0x00000000, LENGTH = 1K
    SHARED_API       (rx)  : ORIGIN = 0x00000400, LENGTH = 512
    BTLDR_CODE       (rx)  : ORIGIN = 0x00000600, LENGTH = 14848    /* 14.5K */
    DCT1_FLASH       (rx)  : ORIGIN = 0x00004000, LENGTH = 16K
    DCT2_FLASH       (rx)  : ORIGIN = 0x00008000, LENGTH = 16K
    APP_CODE         (rx)  : ORIGIN = 0x0000C000, LENGTH = 976K
    SRAM             (rwx) : ORIGIN = 0x1FFE0000, LENGTH = 128K
    BTLDR_SRAM       (rwx) : ORIGIN = 0x1FFFE000, LENGTH = 8k       /* Boot loader stack at the end. */
    SRAM1            (rwx) : ORIGIN = 0x20038000, LENGTH = 32K      /* Not used */
    SRAM2            (rwx) : ORIGIN = 0x20040000, LENGTH = 32K      /* Not used */
}
