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

/* Originally taken from TropicSSL
 * https://gitorious.org/tropicssl/
 * commit: 92bb3462dfbdb4568c92be19e8904129a17b1eed
 * Whitespace converted (Tab to 4 spaces, LF to CRLF)
 * int arguments/returns changed to int32_t
 * remove sha2_file
 */

/**
 * \file sha2.h
 *
 *  Based on XySSL: Copyright (C) 2006-2008  Christophe Devine
 *
 *  Copyright (C) 2009  Paul Bakker <polarssl_maintainer at polarssl dot org>
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the names of PolarSSL or XySSL nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef TROPICSSL_SHA2_H
#define TROPICSSL_SHA2_H

#include "crypto_structures.h"

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * \brief          SHA-256 context setup
     *
     * \param ctx      context to be initialized
     * \param is224    0 = use SHA256, 1 = use SHA224
     */
    void sha2_starts(sha2_context * ctx, int32_t is224);

    /**
     * \brief          SHA-256 process buffer
     *
     * \param ctx      SHA-256 context
     * \param input    buffer holding the  data
     * \param ilen     length of the input data
     */
    void sha2_update(sha2_context * ctx, const unsigned char *input, uint32_t ilen);

    /**
     * \brief          SHA-256 final digest
     *
     * \param ctx      SHA-256 context
     * \param output   SHA-224/256 checksum result
     */
    void sha2_finish(sha2_context * ctx, unsigned char output[32]);

    /**
     * \brief          Output = SHA-256( input buffer )
     *
     * \param input    buffer holding the  data
     * \param ilen     length of the input data
     * \param output   SHA-224/256 checksum result
     * \param is224    0 = use SHA256, 1 = use SHA224
     */
    void sha2(const unsigned char *input, uint32_t ilen,
          unsigned char output[32], int32_t is224);

    /**
     * \brief          SHA-256 HMAC context setup
     *
     * \param ctx      HMAC context to be initialized
     * \param key      HMAC secret key
     * \param keylen   length of the HMAC key
     * \param is224    0 = use SHA256, 1 = use SHA224
     */
    void sha2_hmac_starts(sha2_context * ctx, const unsigned char *key, uint32_t keylen,
                  int32_t is224);

    /**
     * \brief          SHA-256 HMAC process buffer
     *
     * \param ctx      HMAC context
     * \param input    buffer holding the  data
     * \param ilen     length of the input data
     */
    void sha2_hmac_update(sha2_context * ctx, const unsigned char *input,
                  uint32_t ilen );

    /**
     * \brief          SHA-256 HMAC final digest
     *
     * \param ctx      HMAC context
     * \param output   SHA-224/256 HMAC checksum result
     */
    void sha2_hmac_finish(sha2_context * ctx, unsigned char output[32]);

    /**
     * \brief          Output = HMAC-SHA-256( hmac key, input buffer )
     *
     * \param key      HMAC secret key
     * \param keylen   length of the HMAC key
     * \param input    buffer holding the  data
     * \param ilen     length of the input data
     * \param output   HMAC-SHA-224/256 result
     * \param is224    0 = use SHA256, 1 = use SHA224
     */
    void sha2_hmac(const unsigned char *key, uint32_t keylen,
               const unsigned char *input, uint32_t ilen,
               unsigned char output[32], int32_t is224);

    /**
     * \brief          Checkup routine
     *
     * \return         0 if successful, or 1 if the test failed
     */
    int32_t sha2_self_test(int32_t verbose);

#ifdef __cplusplus
}
#endif
#endif                /* sha2.h */
