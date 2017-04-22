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

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/* Curve selection options. */
#define uECC_secp160r1     1
#define uECC_secp192r1     2
#define uECC_secp256r1     3
#define uECC_secp256k1     4
#define uECC_secp224r1     5

/* Inline assembly options.
uECC_asm_none  - Use standard C99 only.
uECC_asm_small - Use GCC inline assembly for the target platform (if available), optimized for
                 minimum size.
uECC_asm_fast  - Use GCC inline assembly optimized for maximum speed. */
#define uECC_asm_none      0
#define uECC_asm_small     1
#define uECC_asm_fast      2


/* uECC_SQUARE_FUNC - If enabled (defined as nonzero), this will cause a specific function to be
used for (scalar) squaring instead of the generic multiplication function. This will make things
faster by about 8% but increases the code size. */

/* Platform selection options.
If uECC_PLATFORM is not defined, the code will try to guess it based on compiler macros.
Possible values for uECC_PLATFORM are defined below: */
#define uECC_arch_other     0
#define uECC_x86            1
#define uECC_x86_64         2
#define uECC_arm            3
#define uECC_arm_thumb      4
#define uECC_avr            5
#define uECC_arm_thumb2     6

#define uECC_secp160r1_size     20
#define uECC_secp192r1_size     24
#define uECC_secp224r1_size     28
#define uECC_secp256k1_size     32
#define uECC_secp256r1_size     32


/* Selected curve */
#define uECC_CURVE   uECC_secp256r1
#define uECC_BYTES   uECC_secp256r1_size

/* Selected assembly type */
#define uECC_ASM     uECC_asm_fast

/* Enable square function by default */
#define uECC_SQUARE_FUNC 1

/* Set the platform */  /* see http://sourceforge.net/p/predef/wiki/Architectures/ */
#if defined ( __x86_64__ )  || defined ( __i386__ ) || defined ( _M_X64 ) || defined ( _M_IX86 )
#define uECC_PLATFORM   uECC_x86
#else
#define uECC_PLATFORM   uECC_arm_thumb2
#endif


#define uECC_WORD_SIZE    4

#include "secp256r1_constants.h"

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
