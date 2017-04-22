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
 *  WICED MQTT APIs.
 */
#pragma once

#include "wiced.h"
#include "mqtt_common.h"
#include "mqtt_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/
/* NOTE : don't try to modify this value */
/* Indicate that application has to allocate memory and give the reference and this should not be in stack and application has to free it after use */
#define WICED_MQTT_OBJECT_MEMORY_SIZE_REQUIREMENT       sizeof(mqtt_connection_t)

/******************************************************
 *               Function Declarations
 * @endcond
 ******************************************************/

/*****************************************************************************/
/**
 * @defgroup mqtt       MQTT
 * @ingroup ipcoms
 *
 * Communication functions for MQTT (Message Queue Telemetry Transport)
 *
 *  @{
 */
/*****************************************************************************/

/** Initializes MQTT object
 *
 * @param[in] mqtt_obj          : Contains address of a memory location, having size of WICED_MQTT_OBJECT_MEMORY_SIZE_REQUIREMENT bytes
 *                                Application has to allocate it non stack memory area. And application has to free it after use
 * @return @ref wiced_result_t
 * NOTE :  The mqtt_obj memory here can be freed or reused by application after calling wiced_mqtt_deinit()
 *
 */
wiced_result_t wiced_mqtt_init( wiced_mqtt_object_t mqtt_obj );


/** De-initializes MQTT object
 *
 * @param[in] mqtt_obj          : Contains address of a memory location which is passed during MQTT init
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_mqtt_deinit( wiced_mqtt_object_t mqtt_obj );


/** Establishes connection with MQTT broker.
 *
 * NOTE:
 *      This is an asynchronous API. Connection status will be notified using callback function.
 *      WICED_MQTT_EVENT_TYPE_CONNECTED event will be sent using callback function
 *
 * @param[in] mqtt_obj          : Contains address of a memory location which is passed during MQTT init
 * @param[in] address           : IP address of the Broker
 * @param[in] interface         : Network interface to be used for establishing connection with Broker
 * @param[in] callback          : Event callback function which is used for notifying the events from library
 * @param[in] security          : Security related information for establishing secure connection with Broker. If NULL, connection with Broker will be unsecured.
 * @param[in] conninfo          : MQTT connect message related information
 *
 * @return @ref wiced_result_t
 * NOTE: Allocate memory for conninfo->client_id, conninfo->username, conninfo->password in non-stack area.
 *       And free/resuse them after getting event WICED_MQTT_EVENT_TYPE_CONNECT_REQ_STATUS or WICED_MQTT_EVENT_TYPE_DISCONNECTED
 */
wiced_result_t wiced_mqtt_connect( wiced_mqtt_object_t mqtt_obj, wiced_ip_address_t *address, wiced_interface_t interface, wiced_mqtt_callback_t callback, wiced_mqtt_security_t *security, wiced_mqtt_pkt_connect_t *conninfo );


/** Disconnect from MQTT broker.
 *
 * NOTE:
 *      This is an asynchronous API. Disconnect status will be notified using using callback function.
 *      WICED_MQTT_EVENT_TYPE_DISCONNECTED event will be sent using callback function
 *
 * @param[in] mqtt_obj          : Contains address of a memory location which is passed during MQTT init
 *
 * @return @ref wiced_result_t
 *
 * NOTE: Allocate memory for conninfo->client_id, conninfo->username, conninfo->password in non-stack area.
 *       And free/resuse them after getting event WICED_MQTT_EVENT_TYPE_CONNECT_REQ_STATUS or WICED_MQTT_EVENT_TYPE_DISCONNECTED
 *
 */
wiced_result_t wiced_mqtt_disconnect( wiced_mqtt_object_t mqtt_obj );


/** Publish message to MQTT Broker on the given Topic
 *
 * NOTE:
 *      This is an asynchronous API. Publish status will be notified using using callback function.
 *      WICED_MQTT_EVENT_TYPE_PUBLISHED event will be sent using callback function
 *
 * @param[in] mqtt_obj          : Contains address of a memory location which is passed during MQTT init
 * @param[in] topic             : Contains the topic on which the message to be published
 * @param[in] message           : Pointer to the message to be published
 * @param[in] msg_len           : Length of the message pointed by 'message' pointer
 * @param[in] qos               : QoS level to be used for publishing the given message
 *
 * @return wiced_mqtt_msgid_t   : ID for the message being published
 * NOTE: Allocate memory for topic, data in non-stack area.
 *       And free/resuse them after getting event WICED_MQTT_EVENT_TYPE_PUBLISHED
 *       or WICED_MQTT_EVENT_TYPE_DISCONNECTED for given message ID (wiced_mqtt_msgid_t)
 */
wiced_mqtt_msgid_t wiced_mqtt_publish( wiced_mqtt_object_t mqtt_obj, uint8_t *topic, uint8_t *data, uint32_t data_len, uint8_t qos );


/** Subscribe for a topic with MQTT Broker
 *
 * NOTE:
 *      This is an asynchronous API. Subscribe status will be notified using using callback function.
 *      WICED_MQTT_EVENT_TYPE_SUBCRIBED event will be sent using callback function
 *
 * @param[in] mqtt_obj          : Contains address of a memory location which is passed during MQTT init
 * @param[in] topic             : Contains the topic to be subscribed to
 * @param[in] qos               : QoS level to be used for receiving the message on the given topic
 *
 * @return wiced_mqtt_msgid_t   : ID for the message being subscribed
 * NOTE: Allocate memory for topic in non-stack area.
 *       And free/resuse them after getting event WICED_MQTT_EVENT_TYPE_SUBCRIBED
 *       or WICED_MQTT_EVENT_TYPE_DISCONNECTED for given message ID (wiced_mqtt_msgid_t)
 */
wiced_mqtt_msgid_t wiced_mqtt_subscribe( wiced_mqtt_object_t mqtt_obj, char *topic, uint8_t qos );


/** Unsubscribe the topic from MQTT Broker
 *
 * NOTE:
 *      This is an asynchronous API. Unsubscribe status will be notified using using callback function.
 *      WICED_MQTT_EVENT_TYPE_UNSUBCRIBED event will be sent using callback function
 *
 * @param[in] mqtt_obj          : Contains address of a memory location which is passed during MQTT init
 * @param[in] topic             : Contains the topic to be unsubscribed
 *
 * @return wiced_mqtt_msgid_t   : ID for the message being subscribed
 * NOTE: Allocate memory for topic in non-stack area.
 *       And free/resuse them after getting event WICED_MQTT_EVENT_TYPE_UNSUBSCRIBED
 *       or WICED_MQTT_EVENT_TYPE_DISCONNECTED for given message ID (wiced_mqtt_msgid_t)
 */
wiced_mqtt_msgid_t wiced_mqtt_unsubscribe( wiced_mqtt_object_t mqtt_obj, char *topic );

#ifdef __cplusplus
} /* extern "C" */
#endif
