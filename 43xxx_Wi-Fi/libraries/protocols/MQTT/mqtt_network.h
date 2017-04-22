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
 * Network APIs.
 *
 * Internal, not to be used directly by applications.
 */
#pragma once

#include "wiced.h"
#include "mqtt_api.h"

#ifdef __cplusplus
extern "C" {
#endif

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

typedef struct
{
    wiced_tcp_socket_t              socket;
    wiced_queue_t                   queue;
    wiced_thread_t                  net_thread;
    wiced_ip_address_t              server_ip_address;
    uint16_t                        portnumber;
    void*                           p_user;
    wiced_tls_context_t             tls_context;
    wiced_tls_identity_t            tls_identity;
}mqtt_socket_t;

typedef struct wiced_mqtt_buffer_s
{
    wiced_packet_t*                 packet;
    uint8_t*                        data;
}wiced_mqtt_buffer_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

/*
 * Network functions.
 *
 * Internal functions not to be used by user applications.
 */
wiced_result_t mqtt_network_init            ( const wiced_ip_address_t *server_ip_address, uint16_t portnumber, wiced_interface_t interface, void *p_user, mqtt_socket_t *socket, const wiced_mqtt_security_t *security );
wiced_result_t mqtt_network_deinit          ( mqtt_socket_t *socket );
wiced_result_t mqtt_network_connect         ( mqtt_socket_t *socket );
wiced_result_t mqtt_network_disconnect      ( mqtt_socket_t *socket );

wiced_result_t mqtt_network_create_buffer   ( wiced_mqtt_buffer_t *buffer, uint16_t size, mqtt_socket_t *socket );
wiced_result_t mqtt_network_send_buffer     ( const wiced_mqtt_buffer_t *buffer, mqtt_socket_t *socket );
wiced_result_t mqtt_network_delete_buffer   ( wiced_mqtt_buffer_t *buffer );

#ifdef __cplusplus
} /* extern "C" */
#endif
