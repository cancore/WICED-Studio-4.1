#
# Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of 
 # Cypress Semiconductor Corporation. All Rights Reserved.
 # This software, including source code, documentation and related
 # materials ("Software"), is owned by Cypress Semiconductor Corporation
 # or one of its subsidiaries ("Cypress") and is protected by and subject to
 # worldwide patent protection (United States and foreign),
 # United States copyright laws and international treaty provisions.
 # Therefore, you may use this Software only as provided in the license
 # agreement accompanying the software package from which you
 # obtained this Software ("EULA").
 # If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 # non-transferable license to copy, modify, and compile the Software
 # source code solely for use in connection with Cypress's
 # integrated circuit products. Any reproduction, modification, translation,
 # compilation, or representation of this Software except as specified
 # above is prohibited without the express written permission of Cypress.
 #
 # Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 # EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 # WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 # reserves the right to make changes to the Software without notice. Cypress
 # does not assume any liability arising out of the application or use of the
 # Software or any product or circuit described in the Software. Cypress does
 # not authorize its products for use in any products where a malfunction or
 # failure of the Cypress product may reasonably be expected to result in
 # significant property damage, injury or death ("High Risk Product"). By
 # including Cypress's product in a High Risk Product, the manufacturer
 # of such system or application assumes all risk of such use and in doing
 # so agrees to indemnify Cypress against all liability.
#

NAME := Lib_crypto_open

GLOBAL_INCLUDES := . \
                   srp

$(NAME)_SOURCES += \
                   aes.c \
                   arc4.c \
                   bignum.c \
                   camellia.c \
                   certs.c \
                   chacha_reference.c \
                   curve25519.c \
                   des.c \
                   ed25519/ed25519.c \
                   md4.c \
                   md5.c \
                   poly1305.c \
                   seed.c \
                   sha1.c \
                   sha2.c \
                   sha4.c \
                   x509parse.c


# Used by x509parse
$(NAME)_DEFINES += TROPICSSL_DES_C

# Used by AES
$(NAME)_DEFINES += TROPICSSL_AES_ROM_TABLES

# Used by Bignum
$(NAME)_DEFINES += TROPICSSL_HAVE_LONGLONG

# Used by SRP
$(NAME)_DEFINES += STDC_HEADERS \
                   USE_SRP_SHA_512 \
                   OPENSSL \
                   ED25519_FORCE_32BIT

# Used by SEED
$(NAME)_DEFINES += OPENSSL_SMALL_FOOTPRINT

#$(NAME)_CFLAGS  = $(COMPILER_SPECIFIC_PEDANTIC_CFLAGS)

ifneq ($(HOST_MCU_FAMILY),ix86)
$(NAME)_ALWAYS_OPTIMISE := 1
endif

$(NAME)_UNIT_TEST_SOURCES := unit/crypto_unit.cpp \
                             unit/chacha_test_vectors.c \
                             unit/chacha_test.c \
                             unit/test-curve25519.c \
                             ed25519/test.c
