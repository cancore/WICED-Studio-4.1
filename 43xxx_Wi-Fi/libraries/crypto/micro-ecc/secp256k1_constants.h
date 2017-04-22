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
#pragma once

#include "configuration.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#if (uECC_WORD_SIZE == 1)

#define uECC_WORDS 32
#define uECC_N_WORDS 32

#define Curve_P {0x2F, 0xFC, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, \
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, \
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, \
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
#define Curve_B {0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define Curve_G { \
    {0x98, 0x17, 0xF8, 0x16, 0x5B, 0x81, 0xF2, 0x59, \
        0xD9, 0x28, 0xCE, 0x2D, 0xDB, 0xFC, 0x9B, 0x02, \
        0x07, 0x0B, 0x87, 0xCE, 0x95, 0x62, 0xA0, 0x55, \
        0xAC, 0xBB, 0xDC, 0xF9, 0x7E, 0x66, 0xBE, 0x79}, \
    {0xB8, 0xD4, 0x10, 0xFB, 0x8F, 0xD0, 0x47, 0x9C, \
        0x19, 0x54, 0x85, 0xA6, 0x48, 0xB4, 0x17, 0xFD, \
        0xA8, 0x08, 0x11, 0x0E, 0xFC, 0xFB, 0xA4, 0x5D, \
        0x65, 0xC4, 0xA3, 0x26, 0x77, 0xDA, 0x3A, 0x48}}
#define Curve_N {0x41, 0x41, 0x36, 0xD0, 0x8C, 0x5E, 0xD2, 0xBF, \
                   0x3B, 0xA0, 0x48, 0xAF, 0xE6, 0xDC, 0xAE, 0xBA, \
                   0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, \
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

#elif (uECC_WORD_SIZE == 4)

#define uECC_WORDS 8
#define uECC_N_WORDS 8

#define Curve_P {0xFFFFFC2F, 0xFFFFFFFE, 0xFFFFFFFF, 0xFFFFFFFF, \
                   0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}
#define Curve_B {0x00000007, 0x00000000, 0x00000000, 0x00000000, \
                   0x00000000, 0x00000000, 0x00000000, 0x00000000}
#define Curve_G { \
    {0x16F81798, 0x59F2815B, 0x2DCE28D9, 0x029BFCDB,  \
     0xCE870B07, 0x55A06295, 0xF9DCBBAC, 0x79BE667E}, \
    {0xFB10D4B8, 0x9C47D08F, 0xA6855419, 0xFD17B448,  \
     0x0E1108A8, 0x5DA4FBFC, 0x26A3C465, 0x483ADA77}}
#define Curve_N {0xD0364141, 0xBFD25E8C, 0xAF48A03B, 0xBAAEDCE6, \
                   0xFFFFFFFE, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}

#elif (uECC_WORD_SIZE == 8)

#define uECC_WORDS 4
#define uECC_N_WORDS 4
#define Curve_P {0xFFFFFFFEFFFFFC2Full, 0xFFFFFFFFFFFFFFFFull, \
                   0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull}
#define Curve_B {0x0000000000000007ull, 0x0000000000000000ull, \
                   0x0000000000000000ull, 0x0000000000000000ull}
#define Curve_G { \
    {0x59F2815B16F81798ull, 0x029BFCDB2DCE28D9ull, 0x55A06295CE870B07ull, 0x79BE667EF9DCBBACull}, \
    {0x9C47D08FFB10D4B8ull, 0xFD17B448A6855419ull, 0x5DA4FBFC0E1108A8ull, 0x483ADA7726A3C465ull}}
#define Curve_N {0xBFD25E8CD0364141ull, 0xBAAEDCE6AF48A03Bull, \
                   0xFFFFFFFFFFFFFFFEull, 0xFFFFFFFFFFFFFFFFull}

#endif /* (uECC_WORD_SIZE == 8) */


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


#ifdef __cplusplus
} /* extern "C" */
#endif
