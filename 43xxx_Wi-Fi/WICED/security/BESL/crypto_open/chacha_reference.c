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
Reference ChaCha Cipher implementation

chacha-ref.c version 20080118
D. J. Bernstein
Public domain.

From http://cr.yp.to/chacha.html
     ( http://cr.yp.to/streamciphers/timings/estreambench/submissions/salsa20/chacha8/ref/chacha.c )
*/

#include "chacha.h"

#define ROTATE(v,c) (ROTL32(v,c))
#define XOR(v,w) ((v) ^ (w))
#define PLUS(v,w) (U32V((v) + (w)))
#define PLUSONE(v) (PLUS((v),1))
#define U32V(v) ((uint32_t)(v) & 0xFFFFFFFFUL)
#define U32TO8_LITTLE(p, v) (((uint32_t*)(p))[0] = U32TO32_LITTLE(v))
#define U8TO32_LITTLE(p) U32TO32_LITTLE(((uint32_t*)(p))[0])
#define ROTL32(v, n) (U32V((v) << (n)) | ((v) >> (32 - (n))))

#define SWAP32(v) \
  ((ROTL32(v,  8) & U32C(0x00FF00FF)) | \
   (ROTL32(v, 24) & U32C(0xFF00FF00)))

#ifdef ECRYPT_BIG_ENDIAN
#define U32TO32_LITTLE(v) SWAP32((uint32_t)(v))
#define U32TO32_BIG(v) ((uint32_t)(v))
#else
#define U32TO32_LITTLE(v) ((uint32_t)(v))
#define U32TO32_BIG(v) SWAP32((uint32_t)(v))
#endif


#define QUARTERROUND(a,b,c,d) \
  x[a] = PLUS(x[a],x[b]); x[d] = ROTATE(XOR(x[d],x[a]),16); \
  x[c] = PLUS(x[c],x[d]); x[b] = ROTATE(XOR(x[b],x[c]),12); \
  x[a] = PLUS(x[a],x[b]); x[d] = ROTATE(XOR(x[d],x[a]), 8); \
  x[c] = PLUS(x[c],x[d]); x[b] = ROTATE(XOR(x[b],x[c]), 7);

static void chacha_wordtobyte( uint8_t output[64], const uint32_t input[16], uint8_t rounds )
{
  uint32_t x[16];
  int i;

  for (i = 0;i < 16;++i) x[i] = input[i];
  for (i = rounds;i > 0;i -= 2) {
    QUARTERROUND( 0, 4, 8,12)
    QUARTERROUND( 1, 5, 9,13)
    QUARTERROUND( 2, 6,10,14)
    QUARTERROUND( 3, 7,11,15)
    QUARTERROUND( 0, 5,10,15)
    QUARTERROUND( 1, 6,11,12)
    QUARTERROUND( 2, 7, 8,13)
    QUARTERROUND( 3, 4, 9,14)
  }
  for (i = 0;i < 16;++i) x[i] = PLUS(x[i],input[i]);
  for (i = 0;i < 16;++i) U32TO8_LITTLE(output + 4 * i,x[i]);
}


static const char sigma[16] = "expand 32-byte k";
static const char tau[16] = "expand 16-byte k";

void chacha_keysetup( chacha_context_t *context, const uint8_t *key, uint32_t key_bits )
{
  const char *constants;

  context->input[4] = U8TO32_LITTLE(key + 0);
  context->input[5] = U8TO32_LITTLE(key + 4);
  context->input[6] = U8TO32_LITTLE(key + 8);
  context->input[7] = U8TO32_LITTLE(key + 12);
  if (key_bits == 256) { /* recommended */
    key += 16;
    constants = sigma;
  } else { /* kbits == 128 */
    constants = tau;
  }
  context->input[8] = U8TO32_LITTLE(key + 0);
  context->input[9] = U8TO32_LITTLE(key + 4);
  context->input[10] = U8TO32_LITTLE(key + 8);
  context->input[11] = U8TO32_LITTLE(key + 12);
  context->input[0] = U8TO32_LITTLE(constants + 0);
  context->input[1] = U8TO32_LITTLE(constants + 4);
  context->input[2] = U8TO32_LITTLE(constants + 8);
  context->input[3] = U8TO32_LITTLE(constants + 12);
}

void chacha_ivsetup( chacha_context_t *context, const uint8_t *initial_value )
{
  context->input[12] = 0;
  context->input[13] = 0;
  context->input[14] = U8TO32_LITTLE(initial_value + 0);
  context->input[15] = U8TO32_LITTLE(initial_value + 4);
}

void chacha20_block_ivsetup( chacha_context_t *context, const uint8_t nonce[12], uint32_t block_count )
{
    context->input[12] = U32TO32_LITTLE(block_count);
    context->input[13] = U8TO32_LITTLE(nonce + 0);
    context->input[14] = U8TO32_LITTLE(nonce + 4);
    context->input[15] = U8TO32_LITTLE(nonce + 8);
}

void chacha20_tls_ivsetup( chacha_context_t *context, const uint8_t nonce[8], uint64_t block_count )
{
    context->input[12] = U32TO32_LITTLE( block_count & 0xffffffff );
    context->input[13] = U32TO32_LITTLE( ( block_count >> 32 ) & 0xffffffff );
    context->input[14] = U8TO32_LITTLE(nonce + 0);
    context->input[15] = U8TO32_LITTLE(nonce + 4);
}


void chacha_encrypt_bytes( chacha_context_t *context, const uint8_t *plaintext, uint8_t *ciphertext, uint32_t bytes, uint8_t rounds)
{
  uint8_t output[64];
  unsigned int i;

  if (!bytes) return;
  for (;;) {
    chacha_wordtobyte(output,context->input, rounds);
    context->input[12] = PLUSONE(context->input[12]);
    if (!context->input[12]) {
      context->input[13] = PLUSONE(context->input[13]);
      /* stopping at 2^70 bytes per nonce is user's responsibility */
    }
    if (bytes <= 64) {
      for (i = 0;i < bytes;++i) ciphertext[i] = plaintext[i] ^ output[i];
      return;
    }
    for (i = 0;i < 64;++i) ciphertext[i] = plaintext[i] ^ output[i];
    bytes -= 64;
    ciphertext += 64;
    plaintext += 64;
  }
}

void chacha_decrypt_bytes( chacha_context_t *context, const uint8_t *ciphertext, uint8_t *plaintext, uint32_t bytes, uint8_t rounds )
{
  chacha_encrypt_bytes(context,ciphertext,plaintext,bytes,rounds);
}

void chacha_keystream_bytes( chacha_context_t *context, uint8_t *stream, uint32_t bytes, uint8_t rounds )
{
  uint32_t i;
  for (i = 0;i < bytes;++i) stream[i] = 0;
  chacha_encrypt_bytes(context,stream,stream,bytes,rounds);
}


void chacha20_block_function( const uint8_t key[32],
                              const uint8_t nonce[12],
                              uint32_t block_count,
                              uint8_t output_random[64] )
{

    chacha_context_t rng_context;

    chacha_keysetup( &rng_context, key, 256 );

    chacha20_block_ivsetup( &rng_context, nonce, block_count );

    chacha_wordtobyte( output_random, rng_context.input, 20 );

}
