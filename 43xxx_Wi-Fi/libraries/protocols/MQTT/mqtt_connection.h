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
 *  MQTT internal APIs.
 *
 *  Internal, not to be used directly by applications.
 */
#pragma once

#include "wiced.h"
#include "mqtt_frame.h"
#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/
#define XSTR(S) #S
#define STR(S) XSTR(S)

/******************************************************
 *                    Constants
 ******************************************************/
#define MQTT_PROTOCOL_REPLY_SUCCESS     (200)

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
 *      Backend functions called from mqtt_queue
 ******************************************************/
/*
 * Connection and channel methods functions.
 *
 * Internal not to be used directly by user applications.
 */
wiced_result_t mqtt_backend_put_connect                     ( const mqtt_connect_arg_t     *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_get_connack                     (       mqtt_connack_arg_t     *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_put_publish                     ( const mqtt_publish_arg_t     *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_get_publish                     (       mqtt_publish_arg_t     *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_get_puback                      (       mqtt_puback_arg_t      *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_put_puback                      ( const mqtt_puback_arg_t      *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_put_subscribe                   ( const mqtt_subscribe_arg_t   *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_get_suback                      (       wiced_mqtt_suback_arg_t      *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_put_unsubscribe                 ( const mqtt_unsubscribe_arg_t *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_get_unsuback                    (       mqtt_unsuback_arg_t    *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_put_disconnect                  (                                           mqtt_connection_t *conn );
wiced_result_t mqtt_backend_connection_close                (                                           mqtt_connection_t *conn );
wiced_result_t mqtt_backend_put_pubrec                      ( const mqtt_pubrec_arg_t      *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_get_pubrec                      (       mqtt_pubrec_arg_t      *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_put_pubrel                      ( const mqtt_pubrel_arg_t      *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_get_pubrel                      (       mqtt_pubrel_arg_t      *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_put_pubcomp                     ( const mqtt_pubcomp_arg_t     *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_get_pubcomp                     (       mqtt_pubcomp_arg_t     *args, mqtt_connection_t *conn );
wiced_result_t mqtt_backend_put_pingreq                     (                                           mqtt_connection_t *conn );
wiced_result_t mqtt_backend_get_pingres                     (                                           mqtt_connection_t *conn );

#ifdef __cplusplus
} /* extern "C" */
#endif
