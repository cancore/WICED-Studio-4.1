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
#include "wiced_bt_smartbridge.h"
#include "wiced_bt_smartbridge_gatt.h"
#include "bt_smartbridge_stack_interface.h"
#include "bt_smartbridge_helper.h"

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

wiced_result_t wiced_bt_smartbridge_gatt_discover_all_primary_services( const wiced_bt_smartbridge_socket_t* socket, wiced_bt_smart_attribute_list_t* service_list )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || service_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );

    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return smartbridge_bt_interface_discover_all_primary_services( socket->connection_handle, service_list );
}

wiced_result_t wiced_bt_smartbridge_gatt_discover_primary_services_by_uuid( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_list_t* service_list )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || uuid == NULL || service_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return smartbridge_bt_interface_discover_primary_services_by_uuid( socket->connection_handle, uuid, service_list );
}

wiced_result_t wiced_bt_smartbridge_gatt_find_included_services( const wiced_bt_smartbridge_socket_t* socket, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* include_list )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || include_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return smartbridge_bt_interface_find_included_services( socket->connection_handle, start_handle, end_handle, include_list );
}

wiced_result_t wiced_bt_smartbridge_gatt_discover_all_characteristics_in_a_service( const wiced_bt_smartbridge_socket_t* socket, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* characteristic_list )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || characteristic_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }
    return smartbridge_bt_interface_discover_all_characteristics_in_a_service( socket->connection_handle, start_handle, end_handle, characteristic_list );
}

wiced_result_t wiced_bt_smartbridge_gatt_discover_characteristic_by_uuid( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_uuid_t* uuid, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* characteristic_list )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || uuid == NULL || characteristic_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return smartbridge_bt_interface_discover_characteristic_by_uuid( socket->connection_handle, uuid, start_handle, end_handle, characteristic_list );
}

wiced_result_t wiced_bt_smartbridge_gatt_discover_handle_and_type_of_all_characteristic_descriptors( const wiced_bt_smartbridge_socket_t* socket, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* descriptor_list )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || descriptor_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return smartbridge_bt_interface_discover_all_characteristic_descriptors( socket->connection_handle, start_handle, end_handle, descriptor_list );
}

wiced_result_t wiced_bt_smartbridge_gatt_read_characteristic_descriptor( const wiced_bt_smartbridge_socket_t* socket, uint16_t handle, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_t** descriptor )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || uuid == NULL || descriptor == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return smartbridge_bt_interface_read_characteristic_descriptor( socket->connection_handle, handle, uuid, descriptor );
}

wiced_result_t wiced_bt_smartbridge_gatt_read_long_characteristic_descriptor( const wiced_bt_smartbridge_socket_t* socket, uint16_t handle, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_t** descriptor )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || uuid == NULL || descriptor == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return smartbridge_bt_interface_read_long_characteristic_descriptor( socket->connection_handle, handle, uuid, descriptor );
}

wiced_result_t wiced_bt_smartbridge_gatt_write_characteristic_descriptor( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* descriptor )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || descriptor == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return smartbridge_bt_interface_write_characteristic_descriptor( socket->connection_handle, (wiced_bt_smart_attribute_t*)descriptor );
}

wiced_result_t wiced_bt_smartbridge_gatt_write_long_characteristic_descriptor( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* descriptor )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || descriptor == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return smartbridge_bt_interface_write_long_characteristic_descriptor( socket->connection_handle, (wiced_bt_smart_attribute_t*)descriptor );
}

wiced_result_t wiced_bt_smartbridge_gatt_read_characteristic_value( const wiced_bt_smartbridge_socket_t* socket, uint16_t handle, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_t** characteristic_value )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || uuid == NULL || characteristic_value == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return smartbridge_bt_interface_read_characteristic_value( socket->connection_handle, handle, uuid, characteristic_value );
}

wiced_result_t wiced_bt_smartbridge_gatt_read_characteristic_values_using_uuid( const wiced_bt_smartbridge_socket_t* socket,  const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_list_t* characteristic_value_list )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || uuid == NULL || characteristic_value_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return smartbridge_bt_interface_read_characteristic_values_using_uuid( socket->connection_handle, uuid, characteristic_value_list );
}

wiced_result_t wiced_bt_smartbridge_gatt_read_long_characteristic_value( const wiced_bt_smartbridge_socket_t* socket, uint16_t handle, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_t** characteristic_value )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || uuid == NULL || characteristic_value == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return smartbridge_bt_interface_read_long_characteristic_value( socket->connection_handle, handle, uuid, characteristic_value );
}

wiced_result_t wiced_bt_smartbridge_gatt_write_characteristic_value_without_response( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* characteristic_value )
{
    return WICED_BT_UNSUPPORTED;
}

wiced_result_t wiced_bt_smartbridge_gatt_signed_write_characteristic_value_without_response( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* characteristic_value )
{
    return WICED_BT_UNSUPPORTED;
}

wiced_result_t wiced_bt_smartbridge_gatt_write_characteristic_value( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* characteristic_value )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || characteristic_value == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }
    return smartbridge_bt_interface_write_characteristic_value( socket->connection_handle, (wiced_bt_smart_attribute_t*)characteristic_value );
}

wiced_result_t wiced_bt_smartbridge_gatt_write_long_characteristic_value( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* characteristic_value )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || characteristic_value == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return smartbridge_bt_interface_write_long_characteristic_value( socket->connection_handle, (wiced_bt_smart_attribute_t*)characteristic_value );
}

wiced_result_t wiced_bt_smartbridge_gatt_reliable_write_characteristic_value( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* characteristic_value )
{
    return WICED_BT_UNSUPPORTED;
}
