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

/**
 * @file
 *
 * Curve25519 elliptic curve key generation functions
 * See http://cr.yp.to/ecdh.html
 *
 */

#ifndef INCLUDED_CURVE25519_H_
#define INCLUDED_CURVE25519_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Calculate a public (shared) key given a basepoint and secret key
 *
 * @param mypublic_output Receives the 32 byte output shared key
 * @param secret          The 32 byte secret key. Must have been randomly
 *                        generated then have the following operations performed
 *                        on it:
 *                               secret[0]  &= 248;
 *                               secret[31] &= 127;
 *                               secret[31] |= 64;
 * @param basepoint       The starting point for the calculation - usually the
 *                        public key of the other party
 *
 * @return 0 when successful
 */
int curve25519( uint8_t *mypublic_output, const uint8_t *secret, const uint8_t *basepoint );

#ifdef __cplusplus
}
#endif

#endif /* ifndef INCLUDED_CURVE25519_H_ */
