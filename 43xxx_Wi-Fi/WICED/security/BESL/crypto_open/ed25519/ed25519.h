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
 * Ed25519 elliptic curve digital signing functions
 *
 */

#ifndef ED25519_H
#define ED25519_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned char ed25519_signature[64];
typedef unsigned char ed25519_public_key[32];
typedef unsigned char ed25519_secret_key[32];

/**
 * Generate a Ed25519 public key from a secret key
 *
 * @param  secret_key The 32 byte secret key
 * @param  public_key Receives the 32 byte output public key
 */
void ed25519_publickey( const ed25519_secret_key secret_key, ed25519_public_key public_key );


/**
 * Sign a message using Ed25519
 *
 * @param  message_data     The message data to sign
 * @param  message_len      The length in bytes of the message data
 * @param  secret_key       The 32 byte secret key
 * @param  public_key       The 32 byte public key
 * @param  signature_output Receives the 64 byte output signature
 */
void ed25519_sign( const unsigned char *message_data, size_t message_len, const ed25519_secret_key secret_key, const ed25519_public_key public_key, ed25519_signature signature_output );

/**
 * Verify an Ed25519 message signature
 *
 * @param  message_data     The message data to verify
 * @param  message_len      The length in bytes of the message data
 * @param  public_key       The 32 byte public key
 * @param  signature Receives the 64 byte output signature
 *
 * @return 0 if signature matches
 */
int ed25519_sign_open( const unsigned char *message_data, size_t message_len, const ed25519_public_key public_key, const ed25519_signature signature);


//int ed25519_sign_open_batch(const unsigned char **m, size_t *mlen, const unsigned char **pk, const unsigned char **RS, size_t num, int *valid);

#ifdef __cplusplus
}
#endif

#endif // ED25519_H
