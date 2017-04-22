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

/* Originally from https://github.com/openssh/openssh-portable/blob/master/cipher-chachapoly.c */

/*
 * Copyright (c) 2013 Damien Miller <djm@mindrot.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* $OpenBSD: cipher-chachapoly.c,v 1.4 2014/01/31 16:39:19 tedu Exp $ */


#include <stdarg.h> /* needed for log.h */
#include <string.h>
#include <stdio.h>  /* needed for misc.h */

//#include "log.h"
#include "misc.h"
#include "chachapoly.h"

void chachapoly_init(struct chachapoly_ctx *ctx,
    const uint8_t *key, uint32_t keylen)
{
   // if (keylen != (32 + 32)) /* 2 x 256 bit keys */
   //     fatal("%s: invalid keylen %u", __func__, keylen);
    chacha_keysetup(&ctx->main_ctx, key, 256);
    chacha_keysetup(&ctx->header_ctx, key + 32, 256);
}

/*
 * chachapoly_crypt() operates as following:
 * En/decrypt with header key 'aadlen' bytes from 'src', storing result
 * to 'dest'. The ciphertext here is treated as additional authenticated
 * data for MAC calculation.
 * En/decrypt 'len' bytes at offset 'aadlen' from 'src' to 'dest'. Use
 * POLY1305_TAGLEN bytes at offset 'len'+'aadlen' as the authentication
 * tag. This tag is written on encryption and verified on decryption.
 */
int
chachapoly_crypt(struct chachapoly_ctx *ctx, uint32_t seqnr, uint8_t *dest,
    const uint8_t *src, uint32_t len, uint32_t aadlen, uint32_t authlen, int do_encrypt)
{
    uint8_t seqbuf[8];
    const uint8_t one[8] = { 1, 0, 0, 0, 0, 0, 0, 0 }; /* NB little-endian */
    uint8_t expected_tag[POLY1305_TAGLEN], poly_key[POLY1305_KEYLEN];
    int r = -1;

    /*
     * Run ChaCha20 once to generate the Poly1305 key. The IV is the
     * packet sequence number.
     */
    memset(poly_key, 0, sizeof(poly_key));
    memcpy( seqbuf, &seqnr, sizeof(seqnr) );
    //chacha_ivsetup(&ctx->main_ctx, seqbuf, NULL);
    //chacha_encrypt_bytes(&ctx->main_ctx,
    //    poly_key, poly_key, sizeof(poly_key));
    /* Set Chacha's block counter to 1 */
    //chacha_ivsetup(&ctx->main_ctx, seqbuf, one);

    /* If decrypting, check tag before anything else */
    if (!do_encrypt) {
        const uint8_t *tag = src + aadlen + len;

        poly1305_auth(expected_tag, src, aadlen + len, poly_key);
        if (timingsafe_bcmp(expected_tag, tag, POLY1305_TAGLEN) != 0)
            goto out;
    }
    /* Crypt additional data */
    if (aadlen) {
        //chacha_ivsetup(&ctx->header_ctx, seqbuf, NULL);
        //chacha_encrypt_bytes(&ctx->header_ctx, src, dest, aadlen);
    }
    //chacha_encrypt_bytes(&ctx->main_ctx, src + aadlen,
    //    dest + aadlen, len);

    /* If encrypting, calculate and append tag */
    if (do_encrypt) {
        poly1305_auth(dest + aadlen + len, dest, aadlen + len,
            poly_key);
    }
    r = 0;

 out:
    explicit_bzero(expected_tag, sizeof(expected_tag));
    explicit_bzero(seqbuf, sizeof(seqbuf));
    explicit_bzero(poly_key, sizeof(poly_key));
    return r;
}

/* Decrypt and extract the encrypted packet length */
int
chachapoly_get_length(struct chachapoly_ctx *ctx,
    uint32_t *plenp, uint32_t seqnr, const uint8_t *cp, uint32_t len)
{
    uint8_t buf[4], seqbuf[8];

    if (len < 4)
        return -1; /* Insufficient length */
    put_u64(seqbuf, seqnr);
    //chacha_ivsetup(&ctx->header_ctx, seqbuf, NULL);
    //chacha_encrypt_bytes(&ctx->header_ctx, cp, buf, 4);
    *plenp = get_u32(buf);
    return 0;
}

