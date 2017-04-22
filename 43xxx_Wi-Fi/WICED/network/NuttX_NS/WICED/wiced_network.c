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
 *  Wiced NuttX networking layer
 */

#include <nuttx/config.h>

#include <arch/chip/wifi.h>

#include "wwd_assert.h"
#include "wwd_buffer_interface.h"
#include "internal/wiced_internal_api.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Static Variables
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_network_init( void )
{
    WPRINT_NETWORK_INFO(("Initialising NuttX networking " RTOS_VERSION "\n"));

    WPRINT_NETWORK_INFO(("Creating Packet pools\n"));
    if ( wwd_buffer_init( NULL ) != WWD_SUCCESS )
    {
        WPRINT_NETWORK_ERROR(("Could not initialize buffer interface\n"));
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_network_deinit( void )
{
    WPRINT_NETWORK_INFO(("Deinitialising NuttX networking " RTOS_VERSION "\n"));

    WPRINT_NETWORK_INFO(("Destroying Packet pools\n"));
    if ( wwd_buffer_deinit( ) != WWD_SUCCESS )
    {
        WPRINT_NETWORK_ERROR(("Could not deinitialize buffer interface\n"));
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_ip_up( wiced_interface_t interface, wiced_network_config_t config, const wiced_ip_setting_t* ip_settings )
{
    wiced_bool_t ip_up = WICED_FALSE;

    UNUSED_PARAMETER( config );
    UNUSED_PARAMETER( ip_settings );

    if ( wifi_driver_is_valid( interface ) )
    {
        if ( wifi_driver_ip_up( interface ) == OK )
        {
            ip_up = WICED_TRUE;
        }
    }
    if ( !ip_up )
    {
        return WICED_ERROR;
    }

    SET_IP_NETWORK_INITED( interface, WICED_TRUE );

    return WICED_SUCCESS;
}

wiced_result_t wiced_ip_down( wiced_interface_t interface )
{
    wiced_bool_t ip_down = WICED_FALSE;

    if ( wifi_driver_is_valid( interface ) )
    {
        if ( wifi_driver_ip_down( interface ) == OK )
        {
            ip_down = WICED_TRUE;
        }
    }
    if ( !ip_down )
    {
        return WICED_ERROR;
    }

    SET_IP_NETWORK_INITED( interface, WICED_FALSE );

    return WICED_SUCCESS;
}
