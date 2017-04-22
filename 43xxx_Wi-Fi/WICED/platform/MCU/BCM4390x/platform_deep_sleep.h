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
 *  Deep-sleep support functions.
 *
 */

#pragma once

#include <stdint.h>
#include "platform_config.h"
#include "platform_map.h"
#include "platform_mcu_peripheral.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
 *             Macros
 ******************************************************/

#ifndef PLATFORM_DEEP_SLEEP_HEADER_INCLUDED
#error "Header file must not be included directly. Please use wiced_deep_sleep.h instead."
#endif

#define WICED_DEEP_SLEEP_STR_EXPAND( name )                      #name

#define WICED_DEEP_SLEEP_SECTION_NAME_SAVED_VAR( name )          ".deep_sleep_saved_vars."WICED_DEEP_SLEEP_STR_EXPAND( name )
#define WICED_DEEP_SLEEP_SECTION_NAME_EVENT_HANDLER( name )      ".deep_sleep_event_handlers."WICED_DEEP_SLEEP_STR_EXPAND( name )
#define WICED_DEEP_SLEEP_SECTION_NAME_EVENT_REGISTRATION( name ) ".deep_sleep_event_registrations."WICED_DEEP_SLEEP_STR_EXPAND( name )

#define WICED_DEEP_SLEEP_IS_WARMBOOT( ) \
    platform_mcu_powersave_is_warmboot( )

#if PLATFORM_APPS_POWERSAVE

#define WICED_DEEP_SLEEP_SAVED_VAR( var ) \
    SECTION( WICED_DEEP_SLEEP_SECTION_NAME_SAVED_VAR( var ) ) var

#define WICED_DEEP_SLEEP_EVENT_HANDLER( func_name ) \
    static void SECTION( WICED_DEEP_SLEEP_SECTION_NAME_EVENT_HANDLER( func_name ) ) func_name( wiced_deep_sleep_event_type_t event ); \
    const wiced_deep_sleep_event_registration_t SECTION( WICED_DEEP_SLEEP_SECTION_NAME_EVENT_REGISTRATION( func_name ) ) func_name##_registration = { .handler = &func_name }; \
    static void MAY_BE_UNUSED func_name( wiced_deep_sleep_event_type_t event )

#define WICED_DEEP_SLEEP_CALL_EVENT_HANDLERS( cond, event ) \
    do { if ( cond ) wiced_deep_sleep_call_event_handlers( event ); } while( 0 )

#define WICED_DEEP_SLEEP_IS_ENABLED( )                           1

#endif /* PLATFORM_APPS_POWERSAVE */

#define WICED_DEEP_SLEEP_IS_AON_SEGMENT( segment_addr, segment_size ) \
    ( ( (segment_addr) >= PLATFORM_SOCSRAM_CH0_AON_RAM_BASE(0x0)) && ( (segment_addr) + (segment_size) <= PLATFORM_SOCSRAM_CH0_AON_RAM_BASE(PLATFORM_SOCSRAM_AON_RAM_SIZE) ) )

/******************************************************
 *             Constants
 ******************************************************/

/******************************************************
 *             Enumerations
 ******************************************************/

/******************************************************
 *             Structures
 ******************************************************/

typedef struct
{
    wiced_deep_sleep_event_handler_t handler;
} wiced_deep_sleep_event_registration_t;

typedef struct
{
    uint32_t entry_point;
    uint32_t app_address;
} wiced_deep_sleep_tiny_bootloader_config_t;

/******************************************************
 *             Variables
 ******************************************************/

/******************************************************
 *             Function declarations
 ******************************************************/

void SECTION( WICED_DEEP_SLEEP_SECTION_NAME_EVENT_HANDLER( wiced_deep_sleep_call_event_handlers ) ) wiced_deep_sleep_call_event_handlers( wiced_deep_sleep_event_type_t event );

#ifdef __cplusplus
} /* extern "C" */
#endif
