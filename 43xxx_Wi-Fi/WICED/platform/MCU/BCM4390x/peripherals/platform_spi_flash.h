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
 * API for accessing SPI layer.
 *
 */

#ifndef INCLUDED_SPI_LAYER_API_H
#define INCLUDED_SPI_LAYER_API_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include "platform_appscr4.h"
#include "platform_mcu_peripheral.h"
#include "platform_m2m.h"
#include "spi_flash/spi_flash.h"
//#include "wiced.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define DIRECT_WRITE_BURST_LENGTH    (64)
#define MAX_NUM_BURST                (2)
#if defined(PLATFORM_4390X_OVERCLOCK)
/* WAR, If PLATFORM_4390X_OVERCLOCK enabled, set NORMAL_READ_DIVIDER=12 and FAST_READ_DIVIDER=4, or read/write sflash data fail. */
#define NORMAL_READ_DIVIDER          (12)
#define FAST_READ_DIVIDER            (4)
#else
#define NORMAL_READ_DIVIDER          (6)
#define FAST_READ_DIVIDER            (2)
#endif
#define MAX_TIMEOUT_FOR_43909_BUSY   (3000)

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/
typedef enum{
    DMA_BUSY,
    SPI_BUSY
} busy_type_t;

typedef enum{
    COMMAND_TXDATA,
    COMMAND_RXDATA,
    COMMAND_ONLY,
    COMMAND_UNKNOWN_PURPOSE
} command_purpose_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Variables Definitions
 ******************************************************/
static const uint32_t address_masks[ SFLASH_ACTIONCODE_MAX_ENUM ] =
{
        [ SFLASH_ACTIONCODE_ONLY ]           = 0x00,
        [ SFLASH_ACTIONCODE_1DATA ]          = 0x00,
        [ SFLASH_ACTIONCODE_3ADDRESS ]       = 0x00ffffff,
        [ SFLASH_ACTIONCODE_3ADDRESS_1DATA ] = 0x00ffffff,
        [ SFLASH_ACTIONCODE_3ADDRESS_4DATA ] = 0x00ffffff,
        [ SFLASH_ACTIONCODE_2DATA ]          = 0x00,
        [ SFLASH_ACTIONCODE_4DATA ]          = 0x00,
};

static const uint32_t data_masks[ SFLASH_ACTIONCODE_MAX_ENUM ] =
{
        [ SFLASH_ACTIONCODE_ONLY ]           = 0x00000000,
        [ SFLASH_ACTIONCODE_1DATA ]          = 0x000000ff,
        [ SFLASH_ACTIONCODE_3ADDRESS ]       = 0x00000000,
        [ SFLASH_ACTIONCODE_3ADDRESS_1DATA ] = 0x000000ff,
        [ SFLASH_ACTIONCODE_3ADDRESS_4DATA ] = 0xffffffff,
        [ SFLASH_ACTIONCODE_2DATA ]          = 0x0000ffff,
        [ SFLASH_ACTIONCODE_4DATA ]          = 0xffffffff,
};

static const uint8_t data_bytes[ SFLASH_ACTIONCODE_MAX_ENUM ] =
{
        [ SFLASH_ACTIONCODE_ONLY ]           = 0,
        [ SFLASH_ACTIONCODE_1DATA ]          = 1,
        [ SFLASH_ACTIONCODE_3ADDRESS ]       = 0,
        [ SFLASH_ACTIONCODE_3ADDRESS_1DATA ] = 1,
        [ SFLASH_ACTIONCODE_3ADDRESS_4DATA ] = 4,
        [ SFLASH_ACTIONCODE_2DATA ]          = 2,
        [ SFLASH_ACTIONCODE_4DATA ]          = 4,
};

/******************************************************
 *               Function Definitions
 ******************************************************/

int bcm4390x_sflash_semaphore_init( void );
void bcm4390x_sflash_semaphore_deinit( void );
int bcm4390x_sflash_semaphore_get( void );
int bcm4390x_sflash_semaphore_set( void );

void spi_layer_init( void );
void spi_layer_deinit( void );
int spi_sflash_send_command( sflash_handle_t* sflash_handle, sflash_command_t command, uint32_t device_address, void* data, uint32_t* data_length );

#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_SPI_LAYER_API_H */
