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

#define uECC_WORDS   20
#define uECC_N_WORDS 21

#define Curve_P     {0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
#define Curve_B     {0x45, 0xFA, 0x65, 0xC5, 0xAD, 0xD4, 0xD4, 0x81, 0x9F, 0xF8, 0xAC, 0x65, 0x8B, 0x7A, 0xBD, 0x54, 0xFC, 0xBE, 0x97, 0x1C}
#define Curve_G     { {0x82, 0xFC, 0xCB, 0x13, 0xB9, 0x8B, 0xC3, 0x68, 0x89, 0x69, 0x64, 0x46, 0x28, 0x73, 0xF5, 0x8E, 0x68, 0xB5, 0x96, 0x4A}, \
                        {0x32, 0xFB, 0xC5, 0x7A, 0x37, 0x51, 0x23, 0x04, 0x12, 0xC9, 0xDC, 0x59, 0x7D, 0x94, 0x68, 0x31, 0x55, 0x28, 0xA6, 0x23} }
#define Curve_N     {0x57, 0x22, 0x75, 0xCA, 0xD3, 0xAE, 0x27, 0xF9, 0xC8, 0xF4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}

#elif (uECC_WORD_SIZE == 4)

#define uECC_WORDS   5
#define uECC_N_WORDS 6

#define Curve_P    {0x7FFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}
#define Curve_B    {0xC565FA45, 0x81D4D4AD, 0x65ACF89F, 0x54BD7A8B, 0x1C97BEFC}
#define Curve_G    { {0x13CBFC82, 0x68C38BB9, 0x46646989, 0x8EF57328, 0x4A96B568}, {0x7AC5FB32, 0x04235137, 0x59DCC912, 0x3168947D, 0x23A62855} }
#define Curve_N    {0xCA752257, 0xF927AED3, 0x0001F4C8, 0x00000000, 0x00000000, 0x00000001}

#elif (uECC_WORD_SIZE == 8)

#define uECC_WORDS    3
#define uECC_N_WORDS  3

#define Curve_P    {0xFFFFFFFF7FFFFFFFull, 0xFFFFFFFFFFFFFFFFull, 0x00000000FFFFFFFFull}
#define Curve_B    {0x81D4D4ADC565FA45ull, 0x54BD7A8B65ACF89Full, 0x000000001C97BEFCull}
#define Curve_G    { {0x68C38BB913CBFC82ull, 0x8EF5732846646989ull, 0x000000004A96B568ull}, {0x042351377AC5FB32ull, 0x3168947D59DCC912ull, 0x0000000023A62855ull} }
#define Curve_N    {0xF927AED3CA752257ull, 0x000000000001F4C8ull, 0x0000000100000000ull}

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
