/**
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
 *
 */

#include "wiced.h"
#include "bt_bus.h"
#include "bt_hci.h"
#include "bt_hci_interface.h"
#include "bt_packet_internal.h"

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

wiced_result_t bt_mfgtest_transport_driver_bus_read_handler( bt_packet_t** packet )
{
    hci_event_header_t header;
    wiced_result_t     result;

    /* Get the packet type */
    result = bt_bus_receive( (uint8_t*)&header, sizeof( header ), WICED_NEVER_TIMEOUT );
    if ( result != WICED_BT_SUCCESS )
    {
        return result;
    }

    /* Allocate buffer for the incoming packet. Always use dynamic packet pool for the  */
    result = bt_packet_pool_dynamic_allocate_packet( packet, sizeof( header ), header.content_length );
    if ( result != WICED_BT_SUCCESS )
    {
        return result;
    }

    /* Copy header to the packet */
    memcpy( ( *packet )->packet_start, &header, sizeof( header ) );
    ( *packet )->data_end  = ( *packet )->data_start + header.content_length;

    if ( header.content_length > 0 )
    {
        /* Receive the remainder of the packet */
        result = bt_bus_receive( (uint8_t*)( ( *packet )->data_start ), (uint32_t)( ( *packet )->data_end - ( *packet )->data_start ), WICED_NEVER_TIMEOUT );
        if ( result != WICED_BT_SUCCESS )
        {
            /* Failed to receive the remainder of the data. Release packet and return error */
            bt_packet_pool_free_packet( *packet );
            return result;
        }
    }

    if ( header.packet_type == 0xff )
    {
        /* Unknown packet type. Release packet and return error */
        bt_packet_pool_free_packet( *packet );
        return WICED_BT_UNKNOWN_PACKET;
    }

    /* Packet successfully received. Pass up to the transport thread and return success */
    return WICED_BT_SUCCESS;
}
