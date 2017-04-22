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
 *
 */

#include "wiced.h"
#include "linked_list.h"
#include "bt_smartbridge_socket_manager.h"

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

static wiced_bool_t smartbridge_socket_manager_find_socket_by_handle_callback  ( linked_list_node_t* node_to_compare, void* user_data );
static wiced_bool_t smartbridge_socket_manager_find_socket_by_address_callback ( linked_list_node_t* node_to_compare, void* user_data );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* Socket Management Globals */
static linked_list_t connected_socket_list;
static wiced_mutex_t connected_socket_list_mutex;
static uint8_t       max_number_of_connections = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t bt_smartbridge_socket_manager_init( void )
{
    wiced_result_t result;

    result = linked_list_init( &connected_socket_list );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_INFO( ( "Error creating linked list\n" ) );
        return result;
    }

    result = wiced_rtos_init_mutex( &connected_socket_list_mutex );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_INFO( ( "Error creating mutex\n" ) );
        return result;
    }

    max_number_of_connections = 1;

    return WICED_BT_SUCCESS;
}

wiced_result_t bt_smartbridge_socket_manager_deinit( void )
{
    wiced_rtos_deinit_mutex( &connected_socket_list_mutex );
    linked_list_deinit( &connected_socket_list );
    max_number_of_connections = 0;
    return WICED_BT_SUCCESS;
}

wiced_result_t bt_smartbridge_socket_manager_set_max_concurrent_connections( uint8_t count )
{
    max_number_of_connections = count;
    return WICED_BT_SUCCESS;
}

wiced_bool_t   bt_smartbridge_socket_manager_is_full( void )
{
    uint32_t active_connection_count;

    linked_list_get_count( &connected_socket_list, &active_connection_count );

    return ( active_connection_count == max_number_of_connections ) ? WICED_TRUE : WICED_FALSE;
}

wiced_result_t bt_smartbridge_socket_manager_insert_socket( wiced_bt_smartbridge_socket_t* socket )
{
    wiced_result_t result;
    uint32_t       count;

    linked_list_get_count( &connected_socket_list, &count );

    if ( count == max_number_of_connections )
    {
        return WICED_BT_MAX_CONNECTIONS_REACHED;
    }

    /* Lock protection */
    wiced_rtos_lock_mutex( &connected_socket_list_mutex );

    result = linked_list_insert_node_at_rear( &connected_socket_list, &socket->node );

    /* Unlock protection */
    wiced_rtos_unlock_mutex( &connected_socket_list_mutex );

    return result;
}

wiced_result_t bt_smartbridge_socket_manager_remove_socket( uint16_t connection_handle, wiced_bt_smartbridge_socket_t** socket )
{
    wiced_result_t      result;
    uint32_t            count;
    linked_list_node_t* node_found;
    uint32_t            user_data = connection_handle;

    linked_list_get_count( &connected_socket_list, &count );

    if ( count == 0 )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    /* Lock protection */
    wiced_rtos_lock_mutex( &connected_socket_list_mutex );

    result = linked_list_find_node( &connected_socket_list, smartbridge_socket_manager_find_socket_by_handle_callback, (void*)user_data, &node_found );
    if ( result == WICED_BT_SUCCESS )
    {
        result = linked_list_remove_node( &connected_socket_list, node_found );

        if ( result == WICED_BT_SUCCESS )
        {
            *socket = (wiced_bt_smartbridge_socket_t*)node_found->data;
        }
    }

    /* Unlock protection */
    wiced_rtos_unlock_mutex( &connected_socket_list_mutex );

    return result;
}

wiced_result_t bt_smartbridge_socket_manager_find_socket_by_handle( uint16_t connection_handle, wiced_bt_smartbridge_socket_t** socket )
{
    wiced_result_t      result;
    uint32_t            count;
    linked_list_node_t* node_found;
    uint32_t            user_data = connection_handle;

    linked_list_get_count( &connected_socket_list, &count );

    if ( count == 0 )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    /* Lock protection */
    wiced_rtos_lock_mutex( &connected_socket_list_mutex );

    result = linked_list_find_node( &connected_socket_list, smartbridge_socket_manager_find_socket_by_handle_callback, (void*)user_data, &node_found );

    if ( result == WICED_BT_SUCCESS )
    {
        *socket = (wiced_bt_smartbridge_socket_t*)node_found->data;
    }

    /* Unlock protection */
    wiced_rtos_unlock_mutex( &connected_socket_list_mutex );

    return result;
}

wiced_result_t bt_smartbridge_socket_manager_find_socket_by_address( const wiced_bt_device_address_t* address, wiced_bt_smartbridge_socket_t** socket )
{
    wiced_result_t      result;
    uint32_t            count;
    linked_list_node_t* node_found;

    linked_list_get_count( &connected_socket_list, &count );

    if ( count == 0 )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    /* Lock protection */
    wiced_rtos_lock_mutex( &connected_socket_list_mutex );

    result = linked_list_find_node( &connected_socket_list, smartbridge_socket_manager_find_socket_by_address_callback, (void*)address, &node_found );

    if ( result == WICED_SUCCESS )
    {
        *socket = (wiced_bt_smartbridge_socket_t*)node_found->data;
    }

    /* Unlock protection */
    wiced_rtos_unlock_mutex( &connected_socket_list_mutex );

    return result;
}

static wiced_bool_t smartbridge_socket_manager_find_socket_by_handle_callback( linked_list_node_t* node_to_compare, void* user_data )
{
    wiced_bt_smartbridge_socket_t* socket = (wiced_bt_smartbridge_socket_t*)node_to_compare->data;
    uint32_t connection_handle = (uint32_t)user_data;

    return ( socket->connection_handle == connection_handle ) ? WICED_TRUE : WICED_FALSE;
}

static wiced_bool_t smartbridge_socket_manager_find_socket_by_address_callback( linked_list_node_t* node_to_compare, void* user_data )
{
    wiced_bt_smartbridge_socket_t* socket = (wiced_bt_smartbridge_socket_t*)node_to_compare->data;
    wiced_bt_device_address_t*    address = (wiced_bt_device_address_t*)user_data;

    return ( memcmp( socket->remote_device.address, address, sizeof( *address ) ) == 0 ) ? WICED_TRUE : WICED_FALSE;
}
