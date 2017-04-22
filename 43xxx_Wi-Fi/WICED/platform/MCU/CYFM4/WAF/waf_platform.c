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
 * Defines STM32F4xx WICED application framework functions
 */
#include <string.h>
#include <stdlib.h>
#include "spi_flash.h"
#include "platform_config.h"
#include "platform_peripheral.h"
#include "wwd_assert.h"
#include "wiced_framework.h"
#include "elf.h"
#include "wiced_apps_common.h"
#include "waf_platform.h"

#define PLATFORM_APP_START_SECTOR      ( FLASH_Sector_3  )
#define PLATFORM_APP_END_SECTOR        ( FLASH_Sector_11 )

#define APP_CODE_START_ADDR   ((uint32_t)&app_code_start_addr_loc)
#define SRAM_START_ADDR       ((uint32_t)&sram_start_addr_loc)
extern void* app_code_start_addr_loc;
extern void* sram_start_addr_loc;

#define ERASE_VOLTAGE_RANGE ( VoltageRange_1 )
#define FLASH_WRITE_FUNC    ( FLASH_ProgramByte )
#define FLASH_WRITE_SIZE    ( 1 )
typedef uint8_t flash_write_t;

#if defined ( __ICCARM__ )

static inline void __jump_to( uint32_t addr )
{
    __asm( "ORR R0, R0, #1" );  /* Last bit of jump address indicates whether destination is Thumb or ARM code */
    __asm( "BX R0" );
}

#elif defined ( __GNUC__ )

__attribute__( ( always_inline ) ) static __INLINE void __jump_to( uint32_t addr )
{
    addr |= 0x00000001;  /* Last bit of jump address indicates whether destination is Thumb or ARM code */
  __ASM volatile ("BX %0" : : "r" (addr) );
}

#endif

void platform_start_app( uint32_t entry_point )
{

    /* Simulate a reset for the app: */
    /*   Switch to Thread Mode, and the Main Stack Pointer */
    /*   Change the vector table offset address to point to the app vector table */
    /*   Set other registers to reset values (esp LR) */
    /*   Jump to the reset vector */


    if ( entry_point == 0 )
    {
        uint32_t* vector_table =  (uint32_t*) APP_CODE_START_ADDR;
        entry_point = vector_table[1];
    }


    __asm( "MOV LR,        #0xFFFFFFFF" );
    __asm( "MOV R1,        #0x01000000" );
    __asm( "MSR APSR_nzcvq,     R1" );
    __asm( "MOV R1,        #0x00000000" );
    __asm( "MSR PRIMASK,   R1" );
    __asm( "MSR FAULTMASK, R1" );
    __asm( "MSR BASEPRI,   R1" );
    __asm( "MSR CONTROL,   R1" );

/*  Now rely on the app crt0 to load VTOR / Stack pointer

    SCB->VTOR = vector_table_address; - Change the vector table to point to app vector table
    __set_MSP( *stack_ptr ); */

    __jump_to( entry_point );

}

platform_result_t platform_erase_flash( uint16_t start_sector, uint16_t end_sector )
{
#ifdef TODO
    uint32_t i;

    /* Unlock the STM32 Flash */
    FLASH_Unlock( );

    /* Clear any error flags */
    FLASH_ClearFlag( FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR );

    platform_watchdog_kick( );

    for ( i = start_sector; i <= end_sector; i += 8 )
    {
        if ( FLASH_EraseSector( i, ERASE_VOLTAGE_RANGE ) != FLASH_COMPLETE )
        {
            /* Error occurred during erase. */
            /* TODO: Handle error */
            while ( 1 )
            {
            }
        }
        platform_watchdog_kick( );
    }

    FLASH_Lock( );

    return PLATFORM_SUCCESS;
#else
    UNUSED_PARAMETER(start_sector);
    UNUSED_PARAMETER(end_sector);
    return PLATFORM_SUCCESS;
#endif
}

platform_result_t platform_write_flash_chunk( uint32_t address, const void* data, uint32_t size )
{
#ifdef TODO
    platform_result_t result = PLATFORM_SUCCESS;
    uint32_t write_address   = address;
    flash_write_t* data_ptr  = (flash_write_t*) data;
    flash_write_t* end_ptr   = (flash_write_t*) &((char*)data)[size];

    FLASH_Unlock( );

    /* Clear any error flags */
    FLASH_ClearFlag( FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR );

    /* Write data to STM32 flash memory */
    while ( data_ptr <  end_ptr )
    {
        FLASH_Status status;

        if ( ( ( ((uint32_t)write_address) & 0x03 ) == 0 ) && ( end_ptr - data_ptr >= FLASH_WRITE_SIZE ) )
        {
            int tries = 0;
            /* enough data available to write as the largest size allowed by supply voltage */
            while ( ( FLASH_COMPLETE != ( status = FLASH_WRITE_FUNC( write_address, *data_ptr ) ) ) && ( tries < 10 ) )
            {
                tries++;
            }
            if ( FLASH_COMPLETE != status )
            {
                /* TODO: Handle error properly */
                wiced_assert("Error during write", 0 != 0 );
            }
            write_address += FLASH_WRITE_SIZE;
            data_ptr++;
        }
        else
        {
            int tries = 0;
            /* Limited data available - write in bytes */
            while ( ( FLASH_COMPLETE != ( status = FLASH_ProgramByte( write_address, (uint8_t) *data_ptr ) ) ) && ( tries < 10 ) )
            {
                tries++;
            }
            if ( FLASH_COMPLETE != status )
            {
                /* TODO: Handle error properly */
                wiced_assert("Error during write", 0 != 0 );
            }
            write_address++;
            data_ptr = (flash_write_t*)((uint32_t)data_ptr+1);
        }

    }
    if ( memcmp( (void*)address, (void*)data, size) != 0 )
    {
        result = PLATFORM_ERROR;
    }
    FLASH_Lock();

    return result;
#else
    UNUSED_PARAMETER(address);
    UNUSED_PARAMETER(data);
    UNUSED_PARAMETER(size);
    return PLATFORM_SUCCESS;
#endif
}

void platform_erase_app_area( uint32_t physical_address, uint32_t size )
{
#ifdef TODO
    /* if app in RAM, no need for erase */
    if ( physical_address < SRAM_START_ADDR )
    {
        if (physical_address == (uint32_t)DCT1_START_ADDR)
        {
            platform_erase_flash( PLATFORM_DCT_COPY1_START_SECTOR, PLATFORM_DCT_COPY1_END_SECTOR );
        }
        else
        {
            platform_erase_flash( PLATFORM_APP_START_SECTOR, PLATFORM_APP_END_SECTOR );
        }
    }
    (void) size;
#else
    UNUSED_PARAMETER(physical_address);
    UNUSED_PARAMETER(size);
#endif
}

/* The function would copy data from serial flash to internal flash.
 * The function assumes that the program area is already erased (for now).
 * TODO: Adding erasing the required area
 */
static wiced_result_t platform_copy_app_to_iflash( const image_location_t* app_header_location, uint32_t offset, uint32_t physical_address, uint32_t size )
{
#ifdef TODO
    /* Bootloader doesn't support BSS sections. */
    uint8_t buff[ 64 ];

    while ( size > 0 )
    {
        uint32_t write_size = MIN( sizeof(buff), size);
        wiced_apps_read( app_header_location, buff, offset, write_size );
        platform_write_flash_chunk( (uint32_t) physical_address, buff, write_size );
        if (memcmp((char *)physical_address, buff, write_size))
        {
            offset = 0;
            return WICED_ERROR;
        }
        offset           += write_size;
        physical_address += write_size;
        size             -= write_size;
    }
    return WICED_SUCCESS;
#else
    UNUSED_PARAMETER(app_header_location);
    UNUSED_PARAMETER(offset);
    UNUSED_PARAMETER(physical_address);
    UNUSED_PARAMETER(size);
    return WICED_SUCCESS;
#endif
}

void platform_load_app_chunk( const image_location_t* app_header_location, uint32_t offset, void* physical_address, uint32_t size )
{
    if ( (uint32_t) physical_address < SRAM_START_ADDR )
    {
        platform_copy_app_to_iflash( app_header_location, offset, (uint32_t) physical_address, size );
    }
    else
    {
        wiced_apps_read( app_header_location, physical_address, offset, size );
    }
}
