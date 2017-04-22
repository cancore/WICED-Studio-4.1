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
#include "bt_transport_driver.h"
#include "linked_list.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/* Driver thread priority is set to 1 higher than BT transport thread */
#define BT_UART_THREAD_PRIORITY WICED_NETWORK_WORKER_PRIORITY - 2
#define BT_UART_THREAD_NAME     "BT UART"
#define BT_UART_STACK_SIZE      600
#define BT_UART_PACKET_TYPE     0x0A

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

static void bt_transport_driver_uart_thread_main  ( uint32_t arg );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static bt_transport_driver_event_handler_t    driver_event_handler    = NULL;
static bt_transport_driver_bus_read_handler_t driver_bus_read_handler = NULL;
static volatile wiced_bool_t                  driver_initialised      = WICED_FALSE;
static volatile wiced_bool_t                  uart_thread_running     = WICED_FALSE;
static wiced_thread_t                         uart_thread;
static wiced_mutex_t                          packet_list_mutex;
static linked_list_t                          uart_rx_packet_list;

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t bt_transport_driver_init( bt_transport_driver_event_handler_t event_handler, bt_transport_driver_bus_read_handler_t bus_read_handler )
{
    wiced_result_t result;
    wiced_bool_t   ready;


    if ( event_handler == NULL || bus_read_handler == NULL )
    {
        return WICED_BT_BADARG;
    }

    if ( driver_initialised == WICED_TRUE )
    {
        return WICED_BT_SUCCESS;
    }

    /* Check if bus is ready */
    ready = bt_bus_is_ready( );
    if ( ready == WICED_FALSE )
    {
        wiced_assert( "BT bus is NOT ready\n", 0!=0 );
        return WICED_BT_BUS_UNINITIALISED;
    }

    driver_initialised = WICED_TRUE;

    /* Create a linked list to hold packet momentarily before being passed to
     * the transport thread.
     */
    result = linked_list_init( &uart_rx_packet_list );
    if ( result != WICED_BT_SUCCESS )
    {
        wiced_assert("Error creating UART RX packet linked list\n", result == WICED_SUCCESS );
        goto error;
    }

    /* Create a semaphore. Once set, this semaphore is used to notify the UART
     * thread that the packet has been read by the upper layer. The UART thread
     * can now continue polling for another packet from the UART circular buffer.
     */
    result = wiced_rtos_init_mutex( &packet_list_mutex );
    if ( result != WICED_SUCCESS )
    {
        wiced_assert("Error creating UART driver mutex\n", result == WICED_SUCCESS );
        goto error;
    }

    /* Create UART thread. WICED UART API does not support callback mechanism.
     * The API blocks in semaphore until the transmission is complete.
     * Consequently, a dedicated thread is required to recieve and dispatch
     * incoming packets to the upper layer.
     */
    uart_thread_running     = WICED_TRUE;
    driver_event_handler    = event_handler;
    driver_bus_read_handler = bus_read_handler;

    result = wiced_rtos_create_thread( &uart_thread, BT_UART_THREAD_PRIORITY, BT_UART_THREAD_NAME, bt_transport_driver_uart_thread_main, BT_UART_STACK_SIZE, NULL );
    if ( result != WICED_SUCCESS )
    {
        wiced_assert("Error creating UART driver thread\n", result == WICED_SUCCESS );
        goto error;
    }

    return WICED_BT_SUCCESS;

    error:
    bt_transport_driver_deinit();
    return result;
}

wiced_result_t bt_transport_driver_deinit( void )
{
    if ( driver_initialised == WICED_FALSE )
    {
        return WICED_BT_SUCCESS;
    }

    uart_thread_running = WICED_FALSE;
    wiced_rtos_delete_thread( &uart_thread );
    wiced_rtos_deinit_mutex( &packet_list_mutex );
    linked_list_deinit( &uart_rx_packet_list );
    driver_event_handler    = NULL;
    driver_bus_read_handler = NULL;
    driver_initialised      = WICED_FALSE;
    return WICED_BT_SUCCESS;
}

wiced_result_t bt_transport_driver_send_packet( bt_packet_t* packet )
{
    wiced_result_t result;

    result = bt_bus_transmit( packet->packet_start, (uint32_t)(packet->data_end - packet->packet_start) );
    if ( result != WICED_BT_SUCCESS )
    {
        wiced_assert("Error transmitting MPAF packet\n", result == WICED_SUCCESS );
        return result;
    }

    /* Destroy packet */
    return bt_packet_pool_free_packet( packet );
}

wiced_result_t bt_transport_driver_receive_packet( bt_packet_t** packet )
{
    uint32_t        count;
    linked_list_node_t* node;
    wiced_result_t  result;

    linked_list_get_count( &uart_rx_packet_list, &count );

    if ( count == 0 )
    {
        return WICED_BT_PACKET_POOL_EXHAUSTED;
    }

    wiced_rtos_lock_mutex( &packet_list_mutex );

    result = linked_list_remove_node_from_front( &uart_rx_packet_list, &node );
    if ( result == WICED_BT_SUCCESS )
    {
        *packet = (bt_packet_t*)node->data;
    }

    wiced_rtos_unlock_mutex( &packet_list_mutex );
    return result;
}

static void bt_transport_driver_uart_thread_main( uint32_t arg )
{
    wiced_assert( "driver_bus_read_handler isn't set", driver_bus_read_handler != NULL );
    wiced_assert( "driver_event_handler isn't set",    driver_event_handler    != NULL );

    while ( uart_thread_running == WICED_TRUE )
    {
        bt_packet_t* packet = NULL;

        if ( driver_bus_read_handler( &packet ) != WICED_BT_SUCCESS )
        {
            continue;
        }

        /* Read successful. Notify upper layer via driver_callback that a new packet is available */
        wiced_rtos_lock_mutex( &packet_list_mutex );
        linked_list_set_node_data( &packet->node, (void*)packet );
        linked_list_insert_node_at_rear( &uart_rx_packet_list, &packet->node );
        wiced_rtos_unlock_mutex( &packet_list_mutex );
        driver_event_handler( TRANSPORT_DRIVER_INCOMING_PACKET_READY );
    }

    WICED_END_OF_CURRENT_THREAD( );
}
