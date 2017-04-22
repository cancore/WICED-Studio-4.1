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
 * NuttX TCP/IP library
 */

#include "wwd_constants.h"
#include "wwd_assert.h"

#include "wiced_constants.h"
#include "wiced_result.h"
#include "wiced_tcpip.h"

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
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_ip_get_ipv4_address( wiced_interface_t interface, wiced_ip_address_t* ipv4_address )
{
    UNUSED_PARAMETER( interface );
    UNUSED_PARAMETER( ipv4_address );
    wiced_assert( "not yet implemented", 0 );
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_ip_get_gateway_address( wiced_interface_t interface, wiced_ip_address_t* ipv4_address )
{
    UNUSED_PARAMETER( interface );
    UNUSED_PARAMETER( ipv4_address );
    wiced_assert( "not yet implemented", 0 );
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_ip_get_netmask( wiced_interface_t interface, wiced_ip_address_t* ipv4_address )
{
    UNUSED_PARAMETER( interface );
    UNUSED_PARAMETER( ipv4_address );
    wiced_assert( "not yet implemented", 0 );
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_packet_delete( wiced_packet_t* packet )
{
    UNUSED_PARAMETER( packet );
    wiced_assert( "not yet implemented", 0 );
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_packet_set_data_end( wiced_packet_t* packet, uint8_t* data_end )
{
    UNUSED_PARAMETER( packet );
    UNUSED_PARAMETER( data_end );
    wiced_assert( "not yet implemented", 0 );
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_packet_set_data_start( wiced_packet_t* packet, uint8_t* data_start )
{
    UNUSED_PARAMETER( packet );
    UNUSED_PARAMETER( data_start );
    wiced_assert( "not yet implemented", 0 );
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_packet_get_data( wiced_packet_t* packet, uint16_t offset, uint8_t** data, uint16_t* fragment_available_data_length, uint16_t *total_available_data_length )
{
    UNUSED_PARAMETER( packet );
    UNUSED_PARAMETER( offset );
    UNUSED_PARAMETER( data );
    UNUSED_PARAMETER( fragment_available_data_length );
    UNUSED_PARAMETER( total_available_data_length );
    wiced_assert( "not yet implemented", 0 );
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_packet_create_tcp( wiced_tcp_socket_t* socket, uint16_t content_length, wiced_packet_t** packet, uint8_t** data, uint16_t* available_space )
{
    UNUSED_PARAMETER( socket );
    UNUSED_PARAMETER( content_length );
    UNUSED_PARAMETER( packet );
    UNUSED_PARAMETER( data );
    UNUSED_PARAMETER( available_space );
    wiced_assert( "not yet implemented", 0 );
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t network_tcp_send_packet( wiced_tcp_socket_t* socket, wiced_packet_t* packet )
{
    UNUSED_PARAMETER( socket );
    UNUSED_PARAMETER( packet );
    wiced_assert( "not yet implemented", 0 );
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t network_tcp_receive( wiced_tcp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout )
{
    UNUSED_PARAMETER( socket );
    UNUSED_PARAMETER( packet );
    UNUSED_PARAMETER( timeout );
    wiced_assert( "not yet implemented", 0 );
    return WICED_TCPIP_SUCCESS;
}
