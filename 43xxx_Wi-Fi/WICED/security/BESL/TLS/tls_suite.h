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

#include <stdint.h>
#include "tls_types.h"
#include "md5.h"
#include "sha1.h"
#include "des.h"
#include "aes.h"
#include "chacha.h"
#include "cipher_suites.h"

#ifdef __cplusplus
extern "C"
{
#endif



/* Common context for processing MAC data */
typedef union
{
    md5_context md5;
    sha1_context sha1;
    sha2_context sha2;
    sha4_context sha4;
} tls_mac_context;


/* Keyscheme driver API function types */
typedef int (*tls_create_premaster_secret)( void*       key_context,
                                            uint8_t        is_ssl_3,
                                            uint16_t       max_version,
                                            uint8_t*       premaster_secret_out,
                                            uint32_t*      pms_length_out,
                                            int32_t        (*f_rng)(void *),
                                            void*          p_rng,
                                            uint8_t*       encrypted_output,
                                            uint32_t*      encrypted_length_out );

typedef int (*tls_decode_premaster_secret)( const uint8_t* data,
                                            uint32_t       data_len,
                                            const uint8_t* key_context,
                                            uint8_t        is_ssl_3,
                                            uint16_t       max_version,
                                            uint8_t*       premaster_secret_out,
                                            uint32_t       pms_buf_length,
                                            uint32_t*      pms_length_out );

typedef int (*tls_parse_dhe_parameters)( ssl_context *ssl, const uint8_t* data, uint32_t data_length, tls_digitally_signed_signature_algorithm_t input_signature_algorithm );
typedef int (*tls_create_dhe_parameters)( ssl_context *ssl, uint8_t* data_out, uint32_t* data_buffer_length_out, tls_digitally_signed_signature_algorithm_t input_signature_algorithm );

/* Cipher driver API function types */
typedef void (*tls_set_encrypt_key_func_t)( tls_cipher_context *ctx, const uint8_t* key, uint32_t key_length_bits );
typedef void (*tls_set_decrypt_key_func_t)( tls_cipher_context *ctx, const uint8_t* key, uint32_t key_length_bits  );
typedef void (*tls_encrypt_func_t)(         tls_cipher_context *ctx, uint8_t* initial_value, const uint8_t *input, uint32_t length, uint8_t *output );
typedef void (*tls_decrypt_func_t)(         tls_cipher_context *ctx, uint8_t* initial_value, const uint8_t *input, uint32_t length, uint8_t *output );
typedef void (*tls_auth_encrypt_func_t)   ( tls_cipher_context *ctx, uint8_t* initial_value, const uint8_t *additional_data, uint32_t additional_length, const uint8_t *input, uint32_t length, uint8_t *output, uint8_t* tag, uint32_t tag_length );
typedef void (*tls_auth_decrypt_func_t)   ( tls_cipher_context *ctx, uint8_t* initial_value, const uint8_t *additional_data, uint32_t additional_length, const uint8_t *input, uint32_t length, uint8_t *output, uint8_t* tag, uint32_t tag_length );

/* MAC driver API function types */
typedef void (*ssl3_mac_func_t)( const unsigned char* secret,
                                 unsigned char*       buffer,
                                 int32_t              buffer_len,
                                 const unsigned char* ctr,
                                 int32_t              type );
typedef void (*tls_hmac_begin_func_t)(  tls_mac_context* ctx, const uint8_t* key,   uint32_t key_length );
typedef void (*tls_hmac_update_func_t)( tls_mac_context* ctx, const uint8_t* buffer, uint32_t buffer_length  );
typedef void (*tls_hmac_end_func_t)(    tls_mac_context* ctx, uint8_t* output );




/* Driver API structures */

struct keyscheme_api_t
{
    key_agreement_authentication_t type;
    tls_create_premaster_secret    create_premaster_secret;
    tls_decode_premaster_secret    decode_premaster_secret;
    tls_parse_dhe_parameters       parse_dhe_parameters;
    uint16_t                       minimum_tls_version;
    tls_create_dhe_parameters      create_dhe_parameters;
};

struct cipher_api_t
{
    int32_t                    key_length;
    int32_t                    minimum_length;
    int32_t                    initial_value_length;
    int32_t                    needs_padding;
    tls_set_encrypt_key_func_t set_encrypt_key;
    tls_set_decrypt_key_func_t set_decrypt_key;
    tls_encrypt_func_t         encrypt;
    tls_decrypt_func_t         decrypt;
    tls_auth_encrypt_func_t    auth_encrypt;
    tls_auth_decrypt_func_t    auth_decrypt;
    cipher_t                   type;
    uint16_t                   minimum_tls_version;
};


struct mac_api_t
{
    int32_t                  mac_length;
    tls_hmac_begin_func_t    tls_hmac_begin;
    tls_hmac_update_func_t   tls_hmac_update;
    tls_hmac_end_func_t      tls_hmac_end;
    ssl3_mac_func_t          ssl3_mac;
    message_authentication_t type;
    uint16_t                 minimum_tls_version;
};


struct ssl3_driver
{
    void     (*devive_master_key) ( ssl_context* ssl );
    void     (*devive_keyblock)   ( ssl_context* ssl, unsigned char keyblk[256] );
    void     (*calc_verify_hash)  ( ssl_context *ssl, unsigned char hash[36], md5_context *md5_ctx, sha1_context *sha1_ctx );
    int32_t  (*calc_hash)         ( ssl_context *ssl, unsigned char *buf, int32_t from, md5_context *md5_ctx, sha1_context *sha1_ctx );
};

extern const struct ssl3_driver* ssl3_driver;

void ssl_mac_md5( const unsigned char *secret, unsigned char *buf, int32_t len, const unsigned char *ctr, int32_t type );
void ssl_mac_sha1( const unsigned char *secret, unsigned char *buf, int32_t len, const unsigned char *ctr, int32_t type );


#ifdef __cplusplus
}
#endif
