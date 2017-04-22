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
#include "wiced_bt_smartbridge.h"

#include "wiced_bt_gatt.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_cfg.h"

#include "bt_smartbridge_socket_manager.h"
#include "bt_smartbridge_att_cache_manager.h"
#include "bt_smartbridge_helper.h"
#include "bt_smartbridge_stack_interface.h"

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

static wiced_bt_smart_scan_result_t*                scan_result_head        = NULL;
static wiced_bt_smart_scan_result_t*                scan_result_tail        = NULL;
static uint32_t                                     scan_result_count       = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t smartbridge_helper_get_scan_results( wiced_bt_smart_scan_result_t** result_list, uint32_t* count )
{
    if ( smartbridge_bt_interface_is_scanning() == WICED_TRUE )
    {
        WPRINT_LIB_INFO(("[Smartbridge] Can't Fetch Scan-Results [ Scan in-progress ? ]\n"));
        return WICED_BT_SCAN_IN_PROGRESS;
    }

    *result_list = scan_result_head;
    *count       = scan_result_count;
    return WICED_BT_SUCCESS;
}

wiced_result_t smartbridge_helper_delete_scan_result_list( void )
{
    wiced_bt_smart_scan_result_t* curr;

    if ( scan_result_count == 0 )
    {
        return WICED_BT_LIST_EMPTY;
    }

    curr = scan_result_head;

    /* Traverse through the list and delete all attributes */
    while ( curr != NULL )
    {
        /* Store pointer to next because curr is about to be deleted */
        wiced_bt_smart_scan_result_t* next = curr->next;

        /* Detach result from the list and free memory */
        curr->next = NULL;
        free( curr );

        /* Update curr */
        curr = next;
    }

    scan_result_count = 0;
    scan_result_head  = NULL;
    scan_result_tail  = NULL;
    return WICED_BT_SUCCESS;
}

wiced_result_t smartbridge_helper_add_scan_result_to_list( wiced_bt_smart_scan_result_t* result )
{
    if ( scan_result_count == 0 )
    {
        scan_result_head = result;
        scan_result_tail = result;
    }
    else
    {
        scan_result_tail->next = result;
        scan_result_tail       = result;
    }

    scan_result_count++;
    WPRINT_LIB_INFO(("[SmartBridge]New scan-result-count:%d\n", (int)scan_result_count));
    result->next = NULL;

    return WICED_BT_SUCCESS;
}

wiced_result_t smartbridge_helper_find_device_in_scan_result_list( wiced_bt_device_address_t* address, wiced_bt_smart_address_type_t type,  wiced_bt_smart_scan_result_t** result )
{
    wiced_bt_smart_scan_result_t* iterator = scan_result_head;

    while( iterator != NULL )
    {
        if ( ( memcmp( &iterator->remote_device.address, address, sizeof( *address ) ) == 0 ) && ( iterator->remote_device.address_type == type ) )
        {
            *result = iterator;
            return WICED_BT_SUCCESS;
        }

        iterator = iterator->next;
    }

    return WICED_BT_ITEM_NOT_IN_LIST;
}

/******************************************************
 *            Socket Action Helper Functions
 ******************************************************/

wiced_bool_t smartbridge_helper_socket_check_actions_enabled( wiced_bt_smartbridge_socket_t* socket, uint8_t action_bits )
{
    return ( ( socket->actions & action_bits ) == action_bits ) ? WICED_TRUE : WICED_FALSE;
}

wiced_bool_t smartbridge_helper_socket_check_actions_disabled( wiced_bt_smartbridge_socket_t* socket, uint8_t action_bits )
{
    return ( ( socket->actions | ~action_bits ) == ~action_bits ) ? WICED_TRUE : WICED_FALSE;
}

void smartbridge_helper_socket_set_actions( wiced_bt_smartbridge_socket_t* socket, uint8_t action_bits )
{
    socket->actions |= action_bits;
}

void smartbridge_helper_socket_clear_actions( wiced_bt_smartbridge_socket_t* socket, uint8_t action_bits )
{
    socket->actions &= ~action_bits;
}

/******************************************************
 *         GATT/GAP subprocedure Functions
 ******************************************************/

gatt_subprocedure_t subprocedure;
#define GATT_MAX_PROCEDURE_TIMEOUT (10000)

wiced_result_t subprocedure_lock( void )
{
    return wiced_rtos_lock_mutex( &subprocedure.mutex );
}

wiced_result_t subprocedure_unlock( void )
{
    return wiced_rtos_unlock_mutex( &subprocedure.mutex );
}

wiced_result_t subprocedure_reset( void )
{
    subprocedure.subprocedure      = GATT_SUBPROCEDURE_NONE;
    subprocedure.attr_head         = NULL;
    subprocedure.attr_tail         = NULL;
    subprocedure.attr_count        = 0;
    subprocedure.result            = WICED_BT_SUCCESS;
    subprocedure.start_handle      = 0;
    subprocedure.end_handle        = 0;
    //subprocedure.pdu               = 0;
    subprocedure.length            = 0;
    subprocedure.offset            = 0;
    subprocedure.connection_handle = 0;
    memset( &subprocedure.uuid, 0, sizeof( subprocedure.uuid ) );
    subprocedure_wait_clear_semaphore();
    return WICED_BT_SUCCESS;
}

wiced_result_t subprocedure_wait_for_completion( void )
{
    return wiced_rtos_get_semaphore( &subprocedure.done_semaphore, GATT_MAX_PROCEDURE_TIMEOUT );
}

wiced_result_t subprocedure_wait_clear_semaphore( void )
{
    while ( wiced_rtos_get_semaphore( &subprocedure.done_semaphore, WICED_NO_WAIT ) == WICED_BT_SUCCESS )
    {
    }
    return WICED_BT_SUCCESS;
}

wiced_result_t subprocedure_notify_complete( void )
{
    return wiced_rtos_set_semaphore( &subprocedure.done_semaphore );
}
