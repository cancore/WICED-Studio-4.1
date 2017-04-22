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
#ifndef _CRYPTO_API_H
#define _CRYPTO_API_H

#include "typedefs.h"
#include <sbhnddma.h>
#include <platform_cache_def.h>
#include "platform_constants.h"
#include "wiced_utilities.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define HWCRYPTO_ALIGNMENT          16
#if defined( PLATFORM_L1_CACHE_SHIFT )
#define HWCRYPTO_ALIGNMENT_BYTES    MAX(HWCRYPTO_ALIGNMENT, PLATFORM_L1_CACHE_BYTES)
#else
#define HWCRYPTO_ALIGNMENT_BYTES    HWCRYPTO_ALIGNMENT
#define
#endif /* defined( PLATFORM_L1_CACHE_SHIFT ) */
#define HWCRYPTO_DESCRIPTOR_ALIGNMENT_BYTES HWCRYPTO_ALIGNMENT_BYTES

/*****************************************************
 *                    Constants
 ******************************************************/
#define DES_IV_LEN                      8
#define DES_BLOCK_SZ                    8
#define AES_IV_LEN                      16
#define AES_BLOCK_SZ                    16
#define AES128_KEY_LEN                  16
#define HMAC256_KEY_LEN                 32
#define HMAC224_KEY_LEN                 28
#define SHA256_HASH_SIZE                32
#define SHA224_HASH_SIZE                28
#define HMAC256_OUTPUT_SIZE             32
#define HMAC224_OUTPUT_SIZE             28
#define HMAC_INNERHASHCONTEXT_SIZE      64
#define HMAC_OUTERHASHCONTEXT_SIZE      64
#define HMAC_BLOCK_SIZE                 64
#define HMAC_IPAD                       0x36
#define HMAC_OPAD                       0x5C
#define SHA_BLOCK_SIZE                  64 /* 64bytes = 512 bits */
#define SHA_BLOCK_SIZE_MASK             ( SHA_BLOCK_SIZE - 1 ) /* SHA1/SHA2 Input data should be at least 64bytes aligned */
#define HWCRYPTO_MAX_PAYLOAD_SIZE       PLATFORM_L1_CACHE_ROUND_UP( 63*1024 )

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    OUTBOUND    = 0,
    INBOUND     = 1
} sctx_inbound_t;

typedef enum
{
    HW_AES_ENCRYPT     = 0,
    HW_AES_DECRYPT     = 1
} hw_aes_mode_type_t;

typedef enum
{
    HW_DES_ENCRYPT     = 0,
    HW_DES_DECRYPT     = 1
} hw_des_mode_type_t;

typedef enum
{
    ENCR_AUTH = 0,
    AUTH_ENCR = 1
} sctx_order_t;

typedef enum
{
    CRYPT_NULL  = 0,
    RC4         = 1,
    DES         = 2,
    _3DES       = 3,
    AES         = 4
} sctx_crypt_algo_t;

typedef enum
{
    ECB = 0,
    CBC = 1,
    OFB = 2,
    CFB = 3,
    CTR = 4,
    CCM = 5,
    GCM = 6,
    XTS = 7
} sctx_crypt_mode_t;

typedef enum
{
    NULL_   = 0,
    MD5     = 1,
    SHA1    = 2,
    SHA224  = 3,
    SHA256  = 4,
    AES_H   = 5,
    FHMAC   = 6
} sctx_hash_algo_t;

typedef enum
{
    HASH=0,
    CTXT=1,
    HMAC=2,
    CCM_H=5,
    GCM_H=6
} sctx_hash_mode_t;

typedef enum
{
    HASH_FULL   = 0,
    HASH_INIT   = 1,
    HASH_UPDT   = 2,
    HASH_FINAL  = 3
} sctx_hash_optype_t;

typedef enum
{
    HWCRYPTO_ENCR_ALG_NONE = 0,
    HWCRYPTO_ENCR_ALG_AES_128,
    HWCRYPTO_ENCR_ALG_AES_192,
    HWCRYPTO_ENCR_ALG_AES_256,
    HWCRYPTO_ENCR_ALG_DES,
    HWCRYPTO_ENCR_ALG_3DES,
    HWCRYPTO_ENCR_ALG_RC4_INIT,
    HWCRYPTO_ENCR_ALG_RC4_UPDT
} hwcrypto_encr_alg_t;

typedef enum
{
    HWCRYPTO_ENCR_MODE_NONE = 0,
    HWCRYPTO_ENCR_MODE_CBC,
    HWCRYPTO_ENCR_MODE_ECB ,
    HWCRYPTO_ENCR_MODE_CTR,
    HWCRYPTO_ENCR_MODE_CCM = 5,
    HWCRYPTO_ENCR_MODE_CMAC,
    HWCRYPTO_ENCR_MODE_OFB,
    HWCRYPTO_ENCR_MODE_CFB,
    HWCRYPTO_ENCR_MODE_GCM,
    HWCRYPTO_ENCR_MODE_XTS
} hwcrypto_encr_mode_t;

typedef enum
{
      HWCRYPTO_AUTH_ALG_NULL = 0,
      HWCRYPTO_AUTH_ALG_MD5 ,
      HWCRYPTO_AUTH_ALG_SHA1,
      HWCRYPTO_AUTH_ALG_SHA224,
      HWCRYPTO_AUTH_ALG_SHA256,
      HWCRYPTO_AUTH_ALG_AES
} hwcrypto_auth_alg_t;

typedef enum
{
      HWCRYPTO_AUTH_MODE_HASH = 0,
      HWCRYPTO_AUTH_MODE_CTXT,
      HWCRYPTO_AUTH_MODE_HMAC,
      HWCRYPTO_AUTH_MODE_FHMAC,
      HWCRYPTO_AUTH_MODE_CCM,
      HWCRYPTO_AUTH_MODE_GCM,
      HWCRYPTO_AUTH_MODE_XCBCMAC,
      HWCRYPTO_AUTH_MODE_CMAC
} hwcrypto_auth_mode_t;

typedef enum
{
      HWCRYPTO_AUTH_OPTYPE_FULL = 0,
      HWCRYPTO_AUTH_OPTYPE_INIT,
      HWCRYPTO_AUTH_OPTYPE_UPDATE,
      HWCRYPTO_AUTH_OPTYPE_FINAL,
      HWCRYPTO_AUTH_OPTYPE_HMAC_HASH
} hwcrypto_auth_optype_t;

typedef enum
{
    HWCRYPTO_CIPHER_MODE_NULL = 0,
    HWCRYPTO_CIPHER_MODE_ENCRYPT,
    HWCRYPTO_CIPHER_MODE_DECRYPT,
    HWCRYPTO_CIPHER_MODE_AUTHONLY
} hwcrypto_cipher_mode_t;

typedef enum
{
    HWCRYPTO_CIPHER_ORDER_NULL = 0,
    HWCRYPTO_CIPHER_ORDER_AUTH_CRYPT,
    HWCRYPTO_CIPHER_ORDER_CRYPT_AUTH
} hwcrypto_cipher_order_t;

typedef enum
{
    HWCRYPTO_CIPHER_OPTYPE_RC4_OPTYPE_INIT = 0,
    HWCRYPTO_CIPHER_OPTYPE_RC4_OPTYPE_UPDT,
    HWCRYPTO_CIPHER_OPTYPE_DES_OPTYPE_K56,
    HWCRYPTO_CIPHER_OPTYPE_3DES_OPTYPE_K168EDE,
    HWCRYPTO_CIPHER_OPTYPE_AES_OPTYPE_K128,
    HWCRYPTO_CIPHER_OPTYPE_AES_OPTYPE_K192,
    HWCRYPTO_CIPHER_OPTYPE_AES_OPTYPE_K256
} hwcrypto_cipher_optype_t;

typedef enum
{
    HWCRYPTO_HASHMODE_HASH_HASH = 0,
    HWCRYPTO_HASHMODE_HASH_CTXT,
    HWCRYPTO_HASHMODE_HASH_HMAC,
    HWCRYPTO_HASHMODE_HASH_FHMAC,
    HWCRYPTO_HASHMODE_AES_HASH_XCBC_MAC,
    HWCRYPTO_HASHMODE_AES_HASH_CMAC,
    HWCRYPTO_HASHMODE_AES_HASH_CCM,
    HWCRYPTO_HASHMODE_AES_HASH_GCM
}  hwcrypto_hash_mode_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/



typedef union
{
    uint32_t raw;
    struct
    {
        unsigned int reserved3      : 16;   /* [ 15:0 ] */
        unsigned int opcode         : 8;    /* [ 23:16] */
        unsigned int supdt_present  : 1;    /* [ 24:1 ] */
        unsigned int reserved2      : 1;    /* [ 25:1 ] */
        unsigned int hash_present   : 1;    /* [ 26:1 ] */
        unsigned int bd_present     : 1;    /* [ 27:1 ] */
        unsigned int mfm_present    : 1;    /* [ 28:1 ] */
        unsigned int bdesc_present  : 1;    /* [ 29:1 ] */
        unsigned int reserved1      : 1;    /* [ 30:1 ] */
        unsigned int sctx_present   : 1;    /* [ 31:1 ] */
    } bits;
} cryptofield_message_header_t;

typedef union
{
    uint32_t raw;
    struct
    {
        unsigned int sctx_size      : 8;    /* [ 7:0   ]  */
        unsigned int reserved       : 22;   /* [ 29:8  ]  */
        unsigned int sctx_type      : 2;    /* [ 31:30 ]  */
    } bits;
} cryptofield_sctx1_header_t;

typedef union
{
    uint32_t raw;
    struct
    {
        unsigned int updt_ofst      : 8;    /* [ 7:0   ]  */
        unsigned int hash_optype    : 2;    /* [ 9:8   ]  */
        unsigned int hash_mode      : 3;    /* [ 12:10 ]  */
        unsigned int hash_algo      : 3;    /* [ 15:13 ]  */
        unsigned int crypt_optype   : 2;    /* [ 17:16 ]  */
        unsigned int crypt_mode     : 3;    /* [ 20:18 ]  */
        unsigned int crypt_algo     : 3;    /* [ 23:21 ]  */
        unsigned int reserved       : 6;    /* [ 29:24 ]  */
        unsigned int order          : 1;    /* [ 30    ]  */
        unsigned int inbound        : 1;    /* [ 31    ]  */
    } bits;
} cryptofield_sctx2_header_t;

typedef union
{
    uint32_t raw;
    struct
    {
        unsigned int exp_iv_size    : 3;    /* [ 2:0   ]  */
        unsigned int iv_ov_ofst     : 2;    /* [ 4:3   ]  */
        unsigned int iv_flags       : 3;    /* [ 7:5   ]  */
        unsigned int icv_size       : 4;    /* [ 11:8  ]  */
        unsigned int icv_flags      : 2;    /* [ 13:12 ]  */
        unsigned int reserved1      : 6;    /* [ 19:14 ]  */
        unsigned int key_handle     : 9;    /* [ 28:20 ]  */
        unsigned int reserved2      : 2;    /* [ 30:29 ]  */
        unsigned int protected_key  : 1;    /* [ 31    ]  */
    } bits;
} cryptofield_sctx3_header_t;

typedef struct
{
    unsigned int length_crypto      : 16;    /* [ 15:0  ]  */
    unsigned int offset_crypto      : 16;    /* [ 31:16 ]  */
} cryptofield_bdesc_crypto_t;

typedef struct
{
    unsigned int length_mac         : 16;    /* [ 15:0  ]  */
    unsigned int offset_mac         : 16;    /* [ 31:16 ]  */
} cryptofield_bdesc_mac_t;

typedef struct
{
    unsigned int offset_iv          : 16;    /* [ 15:0  ]  */
    unsigned int offset_icv         : 16;    /* [ 31:16 ]  */
} cryptofield_bdesc_iv_t;

typedef struct
{
    cryptofield_bdesc_iv_t          iv;
    cryptofield_bdesc_crypto_t      crypto;
    cryptofield_bdesc_mac_t         mac;
} cryptofield_bdesc_t;

typedef struct
{
    unsigned int prev_length        : 16;    /* [ 15:0  ]  */
    unsigned int size               : 16;    /* [ 31:16 ]  */
} cryptofield_bd_t;

typedef struct
{
    cryptofield_message_header_t    message_header;
    uint32_t                        extended_header;
    cryptofield_sctx1_header_t      sctx1;
    cryptofield_sctx2_header_t      sctx2;
    cryptofield_sctx3_header_t      sctx3;
    cryptofield_bdesc_t             bdesc;
    cryptofield_bd_t                bd;
} spum_message_fields_t;

typedef struct crypto_cmd
{
    uint8_t*  source;          /* input buffer */
    uint8_t*  output;          /* output buffer */
    uint8_t*  hash_output;     /* buffer to store hash output
                                   (If NULL, hash output is stored at the end of output payload)*/
    uint8_t*  hash_key;        /* HMAC Key / HASH State (For incremental Hash operations) */
    uint32_t  hash_key_len;    /* HMAC Key / HASH State length */
    uint8_t*  crypt_key;       /* Crypt Key */
    uint32_t  crypt_key_len;   /* Crypt Key length */
    uint8_t*  crypt_iv;        /* Crypt IV */
    uint32_t  crypt_iv_len;    /* Crypt IV length */
    spum_message_fields_t msg; /* SPU-M (HWcrypto) message structure */

} crypto_cmd_t;

typedef struct
{
    uint8_t     state[ SHA256_HASH_SIZE ];               /* HMAC Key/ Result of previous HASH (Used in HASH_UPDT/HASH_FINISH) */
    uint8_t*    payload_buffer;                          /* Buffer to store Output payload
                                                            HWCrypto engine outputs the input payload + HASH result */
    uint8_t     hash_optype;                             /* HASH_INIT/HASH_UPDT/HASH_FINISH, Used for incremental hash operations */
    uint32_t    prev_len;                                /* Used for HASH_UPDT/HASH_FISH, length of data hashed till now */
    int32_t     is224;                                   /* 0 : SHA256, 1: SHA256 */
    uint8_t     i_key_pad[ HMAC_INNERHASHCONTEXT_SIZE ]; /* Used for HMAC > HWCRYPTO_MAX_PAYLOAD_SIZE */
    uint8_t     o_key_pad[ HMAC_INNERHASHCONTEXT_SIZE ]; /* Used for HMAC > HWCRYPTO_MAX_PAYLOAD_SIZE */
} sha256_context_t;

typedef struct
{
    uint8_t*      key;          /* AES Key */
    uint32_t      keylen;       /* AES Key length */
    uint32_t      direction;    /* OUTBOUND : HW_AES_ENCRYPT, INBOUND : HW_AES_DECRYPT */
    uint32_t      cipher_mode;  /* CBC/ECB/CTR/OFB */
} hw_aes_context_t;

typedef struct
{
    uint8_t*    key;            /* DES Key */
    uint32_t    keylen;         /* DES Key length */
    uint32_t    direction;      /* OUTBOUND : HW_DES_ENCRYPT, INBOUND : HW_DES_DECRYPT */
    uint32_t    cipher_mode;    /* CBC/ECB */
    uint32_t    crypt_algo;
} hw_des_context_t;
/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/* Initialize the Crypto Core */
void platform_hwcrypto_init( void );

/**
 * Creates a SPU-M (HWCrypto) Message and Transfers it to SPU-M
 * using M2MDma.
 * Results of the HWCrypto Operation are stored in the buffers
 * specified in cmd.output
 * @param[in] cmd   :   HWCrypto command
 *
 * @return @ref platform_result_t
 *
 */
platform_result_t platform_hwcrypto_execute(crypto_cmd_t cmd);

/**
 * SHA256 HASH on data size < HWCRYPTO_MAX_PAYLOAD_SIZE
 * @Deprecated, use platform_hwcrypto_sha2() instead
 * @Assumption
 * Size is < HWCRYPTO_MAX_PAYLOAD_SIZE
 * output is aligned to CRYPTO_OPTIMIZED_DESCRIPTOR_ALIGNMENT
 */
platform_result_t platform_hwcrypto_sha256_hash(uint8_t* source, uint32_t size, uint8_t* output_payload_buffer, uint8_t* hash_output);

/**
 * SHA256 INIT
 * @param[in]   ctx             sha256_context
 *                              HWcrypto Outputs the PAYLOAD + HASH RESULT
 *                              ctx->payload_buffer should be initialized to point to a buffer big
 *                              enough to contain the input payload
 * @param[in]   input           input data
 * @param[in]   ilen            size of data to be authenticated
 * @param[in]   is224           1: SHA224 0: SHA256
 *
 * The result of hash is stored in ctx->state
 */
platform_result_t platform_hwcrypto_sha2_starts( sha256_context_t* ctx, const unsigned char* input, uint32_t ilen, int32_t is224 );

/**
 * SHA256 UPDATE
 * @Assumption : input_len is multiple of 64 */
/*
 * @param[in]   ctx             sha256_context containing the previous sha result
 * @param[in]   input           input data
 * @param[in]   ilen            size of data to be authenticated

 * The result of hash is stored in ctx->state
 */
platform_result_t platform_hwcrypto_sha2_update( sha256_context_t* ctx, const unsigned char* input, uint32_t ilen );

/**
 * SHA256 FINISH
 * @param[in]   ctx             sha256_context containing the previous sha result
 * @param[in]   input           input data
 * @param[in]   ilen            size of data to be authenticated
 * @param[out]  output          buffer to hold result of authentication
 */
platform_result_t platform_hwcrypto_sha2_finish( sha256_context_t* ctx, const unsigned char* input, uint32_t ilen, unsigned char output[ SHA256_HASH_SIZE ] );

/**
 * SPU-M (HWCrypto Engine) can only hash HWCRYPTO_MAX_PAYLOAD_SIZE payload at a time.
 * for larger payloads, divide the hash into HASH_INIT, HASH_UPDT, HASH_FINISH.
 * SPU-M User guide Rev0.31 Page 41 Section 3.4.1
 *
 * @param[in]   input           pointer to input data/payload
 * @param[in]   ilen            length of input data/payload
 * @param[out]  output          buffer to store output data
 * @param[in]   is224           1: SHA224 0: SHA256
 */
platform_result_t platform_hwcrypto_sha2_incremental( const unsigned char* input, uint32_t ilen, unsigned char output[ SHA256_HASH_SIZE ], int32_t is224);

/**
 * SHA256
 * @param[in]   input           input data
 * @param[in]   ilen            size of data to be authenticated
 * @param[out]  output          buffer to hold result of authentication
 * @param[in]   is224           0 : SHA256 1: SHA224
 */
platform_result_t platform_hwcrypto_sha2(const unsigned char* input, uint32_t ilen, unsigned char hash_output[ SHA256_HASH_SIZE ], int32_t is224);

/**
 * SHA256 HMAC on data size < HWCRYPTO_MAX_PAYLOAD_SIZE
 * @Deprecated, use platform_hwcrypto_sha2_hmac() instead
 * @Assumption
 * Size is < HWCRYPTO_MAX_PAYLOAD_SIZE
 * output is aligned to CRYPTO_OPTIMIZED_DESCRIPTOR_ALIGNMENT
 */

platform_result_t platform_hwcrypto_sha256_hmac( uint8_t *hmac_key, uint32_t keysize, uint8_t *source, uint32_t size,
        uint8_t *payload_buffer, uint8_t *hash_output );

/**
 * SHA256 HMAC INIT
 * Calculates HASH(key XOR ipad)
 * HASH(key XOR ipad || data)
 *
 * @param[in]   ctx             sha256_context containing the previous sha result
 *                              HWcrypto Outputs the PAYLOAD + HASH RESULT
 *                              ctx->payload_buffer should be initialized to point to a buffer big
 *                              enough to contain the input payload
 * @param[in]   key             HMAC key
 * @param[in]   keylen          HMAC key length
 * @param[in]   is224           1: SHA224 0: SHA256
 *
 * The result of hash is stored in ctx->state
 */
platform_result_t platform_hwcrypto_sha2_hmac_starts( sha256_context_t* ctx, const unsigned char* key, uint32_t keylen, int32_t is224 );

/**
 * SHA256 HMAC UPDATE
 * HASH(key XOR ipad || data)
 * @Assumption : input_len is multiple of 64 */
/*
 * @param[in]   ctx             sha256_context containing the previous sha result
 * @param[in]   input           input data
 * @param[in]   ilen            size of data to be authenticated

 * The result of hash is stored in ctx->state
 */
platform_result_t platform_hwcrypto_sha2_hmac_update( sha256_context_t* ctx, const unsigned char* input, int32_t ilen );

/**
 * SHA256 HMAC FINISH
 * @param[in]   ctx             sha256_context containing the previous sha result
 * @param[in]   input           input data
 * @param[in]   ilen            size of data to be authenticated
 * @param[out]  output          buffer to hold result of authentication
 */
platform_result_t platform_hwcrypto_sha2_hmac_finish( sha256_context_t* ctx, uint8_t* input, int32_t ilen, unsigned char output[ SHA256_HASH_SIZE ] );

/**
 * SHA256HMAC Authentication
 * @param[in]   key             HMAC key
 * @param[in]   keylen          HMAC key length
 * @param[in]   input           input data
 * @param[in]   ilen            size of data to be authenticated
 * @param[out]  output          buffer to hold result of authentication
 * @param[in]   is224           0 : SHA256 1: SHA224
 */
platform_result_t platform_hwcrypto_sha2_hmac( const unsigned char* key, uint32_t keylen, const unsigned char* input, uint32_t ilen,
        unsigned char output[ SHA256_HASH_SIZE ], int32_t is224);

/**
 * SHA256HMAC Authentication For lengths > HWCRYPTO_MAX_PAYLOAD_SIZE
 * @param[in]   key             HMAC key
 * @param[in]   keylen          HMAC key length
 * @param[in]   input           input data
 * @param[in]   ilen            size of data to be authenticated
 * @param[out]  output          buffer to hold result of authentication
 * @param[in]   is224           0 : SHA256 1: SHA224
 *
 * SPU-M does not support SHA-256 HMAC of > 64K , but it supports SHA-256 Hash for > 64K data
 * So HMAC is computed by using Hash as described below.
 * HMAC = Hash(key XOR opad || HASH(key XOR ipad || data))
 * (|| -> append, ipad -> 64 byte array of 0x3C, opad -> 64 byte array of 0x5c )
 * InnexHashContext -> key XOR ipad
 * OuterHashContext -> key XOR opad
 *
 * For Details Refer to :
 * APPENDIX B: Summary of Hash Modes and
 * 3.4.1 : Code authentication Using Incremental Hash operation
 *
 * HWcrypto Outputs the PAYLOAD + HASH RESULT
 * ctx->payload_buffer has the output payload
 * output has the HASH result in case of init/updt/final
 */
platform_result_t platform_hwcrypto_sha2_hmac_incremental( const unsigned char* key, uint32_t keylen, const unsigned char* input, uint32_t ilen,
        unsigned char output[ SHA256_HASH_SIZE ], int32_t is224);

/**
 * Combo AES128 CBC decryption + SHA256HMAC Authentication
 * For data size < ( HWCRYPTO_MAX_PAYLOAD_SIZE )
 * @param[in]   crypt_key       AES key
 * @param[in]   crypt_iv        AES IV
 * @param[in]   crypt_size      Size of data to be decrypted
 * @param[in]   auth_size       Size of data to be authenticated
 * @param[in]   hmac_key        HMAC key
 * @param[in]   hmac_key_len    HMAC key length
 * @param[in]   src             input data
 * @param[out]  crypt_dest      Result of decryption
 * @param[out]  hash_dest       Result of authentication
 */
platform_result_t platform_hwcrypto_aescbc_decrypt_sha256_hmac(uint8_t* crypt_key, uint8_t* crypt_iv, uint32_t crypt_size,
        uint32_t auth_size, uint8_t* hmac_key, uint32_t hmac_key_len, uint8_t* src, uint8_t* crypt_dest, uint8_t* hash_dest);

/**
 * Combo SHA256HMAC Authentication + AES128 CBC encryption
 * For data size < ( HWCRYPTO_MAX_PAYLOAD_SIZE )
 * @param[in]   crypt_key       AES key
 * @param[in]   crypt_iv        AES IV
 * @param[in]   crypt_size      Size of data to be encrypted
 * @param[in]   auth_size       Size of data to be authenticated
 * @param[in]   hmac_key        HMAC key
 * @param[in]   hmac_key_len    HMAC key length
 * @param[in]   src             input data
 * @param[out]  crypt_dest      Result of encryption
 * @param[out]  hash_dest       Result of authentication
 */
platform_result_t platform_hwcrypto_sha256_hmac_aescbc_encrypt(uint8_t* crypt_key, uint8_t* crypt_iv, uint32_t crypt_size,
        uint32_t auth_size, uint8_t* hmac_key, uint32_t hmac_key_len, uint8_t* src, uint8_t* crypt_dest, uint8_t* hash_dest);

/**
 * Populate the hw_aes_context_t with key and keysize
 * @param[in]   ctx
 * @param[in]   key
 * @param[in]   keysize_bits
 */
void hw_aes_setkey_enc( hw_aes_context_t* ctx, unsigned char* key, uint32_t keysize_bits );

/**
 * Populate the hw_aes_context_t with key and keysize
 * @param[in]   ctx
 * @param[in]   key
 * @param[in]   keysize_bits
 */
void hw_aes_setkey_dec( hw_aes_context_t* ctx, unsigned char* key, uint32_t keysize_bits );

/**
 * AES CBC Encryption/Decryption
 * @param[in]   ctx         hw_aes_context
 * @param[in]   mode        mode HW_AES_ENCRYPT/HW_AES_DECRYPT
 * @param[in]   length      length of input data
 * @param[in]   iv          aes_iv used for AES
 * @param[in]   input       input data
 * @param[out]  output      output of the AES encryption/decryption
 */
void hw_aes_crypt_cbc(hw_aes_context_t* ctx, hw_aes_mode_type_t mode, uint32_t length, unsigned char iv[16], const unsigned char* input, unsigned char* output);

/**
 * AES ECB Encryption/Decryption
 * @param[in]   ctx         hw_aes_context
 * @param[in]   mode        mode HW_AES_ENCRYPT/HW_AES_DECRYPT
 * @param[in]   length      length of input data
 * @param[in]   iv          aes_iv used for AES
 * @param[in]   input       input data of size AES_BLOCK_SIZE
 * @param[out]  output      output of the AES encryption/decryption
 */
void hw_aes_crypt_ecb(hw_aes_context_t* ctx, hw_aes_mode_type_t mode, const unsigned char input[16], unsigned char output[16]);

/**
 * AES CFB Encryption/Decryption
 * @param[in]   ctx         hw_aes_context
 * @param[in]   mode        mode HW_AES_ENCRYPT/HW_AES_DECRYPT
 * @param[in]   length      length of input data
 * @param[in]   iv          aes_iv used for AES
 * @param[in]   input       input data
 * @param[out]  output      output of the AES encryption/decryption
 */
void hw_aes_crypt_cfb(hw_aes_context_t* ctx, hw_aes_mode_type_t mode, uint32_t length, uint32_t* iv_off, unsigned char iv[16], const unsigned char* input, unsigned char* output);

/**
 * AES CTR Encryption/Decryption
 * @param[in]   ctx         hw_aes_context
 * @param[in]   mode        mode HW_AES_ENCRYPT/HW_AES_DECRYPT
 * @param[in]   length      length of input data
 * @param[in]   iv          aes_iv used for AES
 * @param[in]   input       input data
 * @param[out]  output      output of the AES encryption/decryption
 */
void hw_aes_crypt_ctr(hw_aes_context_t* ctx, hw_aes_mode_type_t mode, uint32_t length, unsigned char iv[16], const unsigned char* input, unsigned char* output);

/**
 * DES/DES3 56 bit Key encryption/decryption
 * K1=K2=K3
 * @param[in]   ctx
 * @param[in]   key
 * */
void hw_des_setkey( hw_des_context_t* ctx, unsigned char* key );

/**
 * DES3 112 bit Key encryption/decryption
 * K3 = K1
 * @param[in]   ctx
 * @param[in]   key
 * */
void hw_des3_set2key(hw_des_context_t* ctx, unsigned char* key);

/**
 * DES3 168 bit Key encryption/decryption
 * @param[in]   ctx
 * @param[in]   key
 * */
void hw_des3_set3key(hw_des_context_t* ctx, unsigned char* key);

/**
 * DES ECB Encryption/Decryption
 * @param[in]   ctx         hw_aes_context
 * @param[in]   mode        mode HW_DES_ENCRYPT/HW_DES_DECRYPT
 * @param[in]   input       input data of size DES_BLOCK_SIZE
 * @param[out]  output      buffer to store result of DES encryption/decryption
 */
void hw_des_crypt_ecb( hw_des_context_t* ctx, hw_des_mode_type_t mode, const unsigned char input[8], unsigned char output[8] );

/**
 * DES CBC Encryption/Decryption
 * @param[in]   ctx         hw_aes_context
 * @param[in]   mode        mode HW_DES_ENCRYPT/HW_DES_DECRYPT
 * @param[in]   length      length of input data
 * @param[in]   iv          iv used for DES
 * @param[in]   input       input data
 * @param[out]  output      buffer to store result of DES encryption/decryption
 */
void hw_des_crypt_cbc( hw_des_context_t* ctx, hw_des_mode_type_t mode, uint32_t length, unsigned char iv[8], const unsigned char* input, unsigned char* output );

/**
 * DES3 ECB Encryption/Decryption
 * @param[in]   ctx         hw_aes_context
 * @param[in]   mode        mode HW_DES_ENCRYPT/HW_DES_DECRYPT
 * @param[in]   input       input data of size DES_BLOCK_SIZE
 * @param[out]  output      buffer to store result of DES encryption/decryption
 */
void hw_des3_crypt_ecb( hw_des_context_t* ctx, hw_des_mode_type_t mode, const unsigned char input[8], unsigned char output[8] );

/**
 * DES3 CBC Encryption/Decryption
 * @param[in]   ctx         hw_aes_context
 * @param[in]   mode        mode HW_DES_ENCRYPT/HW_DES_DECRYPT
 * @param[in]   length      length of input data
 * @param[in]   iv          iv used for DES
 * @param[in]   input       input data
 * @param[out]  output      buffer to store result of DES encryption/decryption
 */
void hw_des3_crypt_cbc( hw_des_context_t* ctx, hw_des_mode_type_t mode, uint32_t length, unsigned char iv[8], const unsigned char* input, unsigned char* output );

#endif  /*_CRYPTO_API_H */

