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

/*
 * From https://github.com/floodyberry/poly1305-donna
 * License: "MIT or PUBLIC DOMAIN"
 */

#ifndef POLY1305_DONNA_H
#define POLY1305_DONNA_H

#include "crypto_structures.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define POLY1305_KEYLEN 32
#define POLY1305_TAGLEN 16

/**
 *  Get the poly1305 Message-Authentication code for a buffer of message data
 *  The Key MUST ONLY BE USED ONCE ONLY
 *  The sender MUST NOT use poly1305_auth to authenticate
 *  more than one message under the same key. Authenticators
 *  for two messages under the same key should be expected
 *  to reveal enough information to allow forgeries of
 *  authenticators on other messages.
 *
 *  @param mac           the output message-authentication code
 *  @param message_data  the message data to be processed through the authenticator
 *  @param bytes         number of bytes of message data to read
 *  @param key           the UNIQUE 32 byte key to be used for this session
 */
void poly1305_auth   ( unsigned char mac[16], const unsigned char *message_data, size_t bytes, const unsigned char key[32]);



/*  Use these functions for processing non-contiguous data */

/**
 *  Initialise a poly1305 session with a key.
 *  The Key MUST ONLY BE USED ONCE ONLY
 *  The sender MUST NOT use poly1305_auth to authenticate
 *  more than one message under the same key. Authenticators
 *  for two messages under the same key should be expected
 *  to reveal enough information to allow forgeries of
 *  authenticators on other messages.
 *
 *  @param context a poly1305_context which will be used for scratch space
 *                 until poly1305_finish is called
 *  @param key     the UNIQUE 32 byte key to be used for this session
 */
void poly1305_init   ( poly1305_context *context, const unsigned char key[32]);

/**
 *  Process message data through poly1305 authenticator
 *
 *  @param context       a poly1305_context which has been initialised with poly1305_init
 *                       and will be used for scratch space until poly1305_finish is called
 *  @param message_data  the message data to be processed through the authenticator
 *  @param bytes         number of bytes of message data to read
 */
void poly1305_update ( poly1305_context *context, const unsigned char *message_data, size_t bytes);

/**
 *  Finish processing a poly1305 authenticator session
 *
 *  @param context       a poly1305_context which has been filled via poly1305_init and
 *                       poly1305_update. Will not be be used subsequently.
 *  @param mac           the output message-authentication code
 */
void poly1305_finish ( poly1305_context *context, unsigned char mac[16]);


/**
 *  Verify that two Message-Authentication Codes match
 *
 *  @param mac1  the second message-authentication code
 *  @param mac2  the second message-authentication code
 *
 *  @return  1 if mac1 matches mac2,    0 otherwise
 */
int poly1305_verify(const unsigned char mac1[16], const unsigned char mac2[16]);


/**
 *  Run tests on the poly1305 algorithm
 *
 *  @return  1 if successful,    0 otherwise
 */
int poly1305_power_on_self_test(void);

/**
 *  Run TLS tests on the poly1305 algorithm
 *
 *  @return  0 if successful
 */
int test_poly1305_tls( void );


#ifdef __cplusplus
} /*extern "C" */
#endif

#endif /* POLY1305_DONNA_H */

