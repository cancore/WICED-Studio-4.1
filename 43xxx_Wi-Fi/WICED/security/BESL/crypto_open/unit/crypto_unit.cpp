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

/** @file
 *  Unit Tester for WicedFS
 *
 *  Runs a suite of tests on the crypto API to attempt
 *  to discover bugs
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "gtest/gtest.h"
#include "wiced_utilities.h"
#include "wiced_time.h"
#include "x509.h"
#include "base64.h"
#include "poly1305.h"

/******************************************************
 *            Compatibility Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                    Macros
 ******************************************************/

/******************************************************
 *                    Global Variables
 ******************************************************/

/******************************************************
 *                    Static Variables
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

extern "C" int base64_test( void );

extern "C" int aes_test(int *t);
extern "C" int aes_test_cbc(void);
extern "C" int aes_test_ctr(void);
extern "C" int aes_test_ccm(void);
extern "C" int aes_test_ccmp(void);
extern "C" int aes_test_ccmp_timing(int *t);
extern "C" int aes_test_cmac(void);
extern "C" int32_t arc4_self_test( int32_t verbose );
extern "C" int32_t aes_self_test( int32_t verbose );
extern "C" int32_t mpi_self_test( int32_t verbose );
extern "C" int32_t des_self_test( int32_t verbose );
extern "C" int32_t md4_self_test( int32_t verbose );
extern "C" int32_t md5_self_test( int32_t verbose );
extern "C" int32_t rsa_self_test( int32_t verbose );
extern "C" int32_t sha1_self_test( int32_t verbose );
extern "C" int32_t sha2_self_test( int32_t verbose );
extern "C" int32_t x509_self_test( int32_t verbose );
extern "C" int chacha_self_test( void );
extern "C" int test_chacha20_tls_keystream( void );
extern "C" int curve25519_self_test( void );
extern "C" int ed25519_test( void );
extern "C" int32_t sha4_self_test( int32_t verbose );
extern "C" int camellia_self_test( int verbose );
extern "C" int test_chacha20_block_function( void );
extern "C" int test_chacha20_poly1305_aead( void );
extern "C" int test_chacha20_poly1305_aead_tls( void );


/******************************************************
 *               Function Definitions
 ******************************************************/





/******************************************************
 *            wicedfs_init tests
 ******************************************************/

class unit_test_crypto :  public ::testing::Test
{

    protected:

    virtual void SetUp()
    {
        malloc_leak_set_ignored( LEAK_CHECK_GLOBAL );
    }
    virtual void TearDown()
    {
        malloc_leak_check( NULL, LEAK_CHECK_GLOBAL );
    }
};



TEST_F(unit_test_crypto, base64)
{
    int result = base64_test( );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, arc4)
{
    int result = arc4_self_test( 0 );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, aes)
{
    int result = aes_self_test( 0 );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, bignum)
{
    int result = mpi_self_test( 0 );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, des)
{
    int result = des_self_test( 0 );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, md4)
{
    int result = md4_self_test( 0 );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, md5)
{
    int result = md5_self_test( 0 );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, rsa)
{
    int result = rsa_self_test( 0 );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, sha1)
{
    int result = sha1_self_test( 0 );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, sha2)
{
    int result = sha2_self_test( 0 );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, sha4)
{
    int result = sha4_self_test( 0 );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, chacha)
{
    int result = chacha_self_test( );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, chacha20_tls_keystream)
{
    int result = test_chacha20_tls_keystream( );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, chacha20_block)
{
    int result = test_chacha20_block_function( );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, chacha20_poly1305_aead)
{
    int result = test_chacha20_poly1305_aead( );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, chacha20_poly1305_aead_tls)
{
    int result = test_chacha20_poly1305_aead_tls( );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, curve25519)
{
    int result = curve25519_self_test( );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, ed25519)
{
    int result = ed25519_test( );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, camellia)
{
    int result = camellia_self_test( 0 );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, poly1305)
{
    int result = poly1305_power_on_self_test( );
    EXPECT_EQ( 1, result );
    result = test_poly1305_tls( );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, x509_valid_cert)
{
    int result;
    struct tm datetime1 = { .tm_sec  = 0,  /* 1st Jan 2008 */
                            .tm_min  = 0,
                            .tm_hour = 0,
                            .tm_mday = 1,
                            .tm_mon  = 1,
                            .tm_year = 104,
    };
    wiced_utc_time_ms_t datetime1_ms = (wiced_utc_time_ms_t) mktime ( &datetime1 ) * 1000;

    wiced_time_set_utc_time_ms( &datetime1_ms );
    result = x509_self_test( 0 );
    EXPECT_EQ( 0, result );
}

TEST_F(unit_test_crypto, x509_expired_cert)
{
    int result;
    struct tm datetime2 = { .tm_sec  = 0,  /* 9th April 2015 */
                            .tm_min  = 0,
                            .tm_hour = 0,
                            .tm_mday = 9,
                            .tm_mon  = 3,
                            .tm_year = 115,
    };
    wiced_utc_time_ms_t datetime2_ms = (wiced_utc_time_ms_t) mktime ( &datetime2 ) * 1000;

    wiced_time_set_utc_time_ms( &datetime2_ms );
    result = x509_self_test( 0 );
    EXPECT_EQ( TROPICSSL_ERR_X509_CERT_VERIFY_FAILED, result );
}



