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
#pragma once

#include "wiced_rtos.h"
#include "bt_packet_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/* Many of the callbacks of higher layer Bluetooth protocols run on WICED_NETWORK_WORKER_PRIORITY
 * context. Bluetooth transport thread priority is to 1 higher than that of
 * WICED_NETWORK_WORKER_PRIORITY to let it preempt WICED_NETWORK_WORKER_PRIORITY.
 */
#define BT_TRANSPORT_THREAD_PRIORITY WICED_NETWORK_WORKER_PRIORITY - 1

/* ~4K of stack space is for printf and stack check.
 */
#define BT_TRANSPORT_STACK_SIZE      4096


#define BT_TRANSPORT_QUEUE_SIZE      10

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef wiced_result_t (*bt_transport_thread_received_packet_handler_t)( bt_packet_t* packet );
typedef wiced_result_t (*bt_transport_thread_callback_handler_t)( void* arg );

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t bt_transport_thread_init( bt_transport_thread_received_packet_handler_t handler );

wiced_result_t bt_transport_thread_deinit( void );

wiced_result_t bt_transport_thread_send_packet( bt_packet_t* packet );

wiced_result_t bt_transport_thread_notify_packet_received( void );

wiced_result_t bt_transport_thread_execute_callback( bt_transport_thread_callback_handler_t callback_handler, void* arg );

wiced_result_t bt_transport_thread_enable_packet_dump( void );

wiced_result_t bt_transport_thread_disable_packet_dump( void );

#ifdef __cplusplus
} /* extern "C" */
#endif
