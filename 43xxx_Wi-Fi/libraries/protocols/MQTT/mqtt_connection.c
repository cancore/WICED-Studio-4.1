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
 *  Connection functions
 *
 *  Provides connection methods for use by applications
 */

#include "wiced.h"
#include "mqtt_internal.h"
#include "mqtt_connection.h"
#include "mqtt_frame.h"
#include "mqtt_manager.h"

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
wiced_result_t mqtt_connection_init( const wiced_ip_address_t *address, uint16_t port_number, wiced_interface_t interface, const wiced_mqtt_callback_t callbacks, mqtt_connection_t *conn, const wiced_mqtt_security_t *security )
{
    wiced_result_t ret = WICED_SUCCESS;
    if ( port_number == 0 )
    {
        port_number = ( security == NULL ) ? WICED_MQTT_CONNECTION_DEFAULT_PORT : WICED_MQTT_CONNECTION_SECURE_PORT;
    }

    if ( conn->network_status == MQTT_NETWORK_DISCONNECTED )
    {
        conn->callbacks = callbacks;
        ret = mqtt_network_init( address, port_number, interface, conn, &conn->socket, security );
        if ( ret == WICED_SUCCESS )
        {
            conn->network_status = MQTT_NETWORK_CONNECTED;
        }
    }
    return ret;
}

wiced_result_t mqtt_connect( mqtt_connection_t *conn, mqtt_connect_arg_t *args, mqtt_session_t *session )
{
    mqtt_socket_t socket = conn->socket;
    mqtt_event_message_t current_event;

    if ( args->clean_session == 1 )
    {
        mqtt_session_init( session );
    }
    else
    {
        if ( conn->session_init == WICED_TRUE )
        {
            mqtt_session_init( session );
        }
        conn->session_init = WICED_FALSE;
    }
    conn->session = session;

    current_event.send_context.event_t = MQTT_EVENT_SEND_CONNECT;
    current_event.send_context.conn = conn;
    current_event.send_context.args.conn_args = *args;
    current_event.event_type = MQTT_SEND_EVENT;

    /*push into the main queue*/
    return wiced_rtos_push_to_queue( &socket.queue, &current_event, WICED_NO_WAIT );
}

wiced_result_t mqtt_disconnect( mqtt_connection_t *conn )
{
    mqtt_socket_t socket = conn->socket;
    mqtt_event_message_t current_event;

    current_event.send_context.event_t = MQTT_EVENT_SEND_DISCONNECT;
    current_event.send_context.conn = conn;
    current_event.event_type = MQTT_SEND_EVENT;

    return wiced_rtos_push_to_queue( &socket.queue, &current_event, WICED_NO_WAIT );
}

wiced_result_t mqtt_subscribe( mqtt_connection_t *conn, mqtt_subscribe_arg_t *args )
{
    mqtt_socket_t socket = conn->socket;
    mqtt_event_message_t current_event;

    current_event.send_context.event_t = MQTT_EVENT_SEND_SUBSCRIBE;
    current_event.send_context.conn = conn;
    current_event.send_context.args.sub_args = *args;
    current_event.event_type = MQTT_SEND_EVENT;

    return wiced_rtos_push_to_queue( &socket.queue, &current_event, WICED_NO_WAIT );
}

wiced_result_t mqtt_unsubscribe( mqtt_connection_t *conn, mqtt_unsubscribe_arg_t *args )
{
    mqtt_socket_t socket = conn->socket;
    mqtt_event_message_t current_event;
    wiced_result_t result = WICED_SUCCESS;

    current_event.send_context.event_t = MQTT_EVENT_SEND_UNSUBSCRIBE;
    current_event.send_context.conn = conn;
    current_event.send_context.args.unsub_args = *args;
    current_event.event_type = MQTT_SEND_EVENT;

    result = wiced_rtos_push_to_queue( &socket.queue, &current_event, WICED_NO_WAIT );
    return result;
}

wiced_result_t mqtt_publish( mqtt_connection_t *conn, mqtt_publish_arg_t *args )
{
    mqtt_socket_t socket = conn->socket;
    mqtt_event_message_t current_event;

    current_event.send_context.event_t = MQTT_EVENT_SEND_PUBLISH;
    current_event.send_context.conn = conn;
    current_event.send_context.args.pub_args = *args;
    current_event.event_type = MQTT_SEND_EVENT;

    return wiced_rtos_push_to_queue( &socket.queue, &current_event, WICED_NO_WAIT );
}

wiced_result_t mqtt_connection_deinit( mqtt_connection_t *conn )
{
    if ( conn->network_status == MQTT_NETWORK_CONNECTED )
    {
        mqtt_network_deinit( &conn->socket );
        conn->network_status = MQTT_NETWORK_DISCONNECTED;
    }
    return WICED_SUCCESS;
}

/******************************************************
 *      Backend functions called from mqtt_queue
 ******************************************************/
wiced_result_t mqtt_backend_put_connect( const mqtt_connect_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret;
    mqtt_frame_t frame;
    mqtt_connect_arg_t final_args = *args;

    /* Fixing connect args based on specification */
    if ( final_args.username_flag == WICED_FALSE )
    {
        /* make sure password flag is 0 as well */
        final_args.password_flag = WICED_FALSE;
    }

    if ( final_args.will_flag == WICED_FALSE )
    {
        /* make sure will_retain and will_qos are 0 */
        final_args.will_retain = WICED_FALSE;
        final_args.will_qos = MQTT_QOS_DELIVER_AT_MOST_ONCE;
    }

    /* Send Protocol Header */
    ret = mqtt_frame_create( MQTT_CONNECTION_FRAME_MAX, &frame, &conn->socket );
    if ( ret != WICED_SUCCESS )
    {
        return ret;
    }
    mqtt_frame_put_connect( &frame, &final_args );
    return mqtt_frame_send( &frame, &conn->socket );
}

wiced_result_t mqtt_backend_get_connack( mqtt_connack_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret = WICED_SUCCESS;
    UNUSED_PARAMETER(args);
    if ( ret == WICED_SUCCESS )
    {
        /* Publish is an async method (we don't get an OK), so we simulate the OK after sending it */

        if ( conn->callbacks != NULL )
        {

            wiced_mqtt_event_info_t event;
            event.type = WICED_MQTT_EVENT_TYPE_CONNECT_REQ_STATUS;
            event.data.err_code = args->return_code;
            conn->callbacks( (void*) conn, &event );
        }
    }
    return ret;
}

wiced_result_t mqtt_backend_put_publish( const mqtt_publish_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret;
    mqtt_frame_t frame;
    mqtt_publish_arg_t final_args = *args;

    /* Send Protocol Header */
    ret = mqtt_frame_create( MQTT_CONNECTION_FRAME_MAX, &frame, &conn->socket );
    if ( ret != WICED_SUCCESS )
    {
        return ret;
    }
    mqtt_frame_put_publish( &frame, &final_args );
    return mqtt_frame_send( &frame, &conn->socket );
}

wiced_result_t mqtt_backend_get_publish( mqtt_publish_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret;
    wiced_mqtt_event_info_t event;
    ret = mqtt_manager( MQTT_EVENT_RECV_PUBLISH, args, conn );
    if ( ( ret == WICED_SUCCESS ) && ( args->data != NULL ) )
    {
        /* Publish is an async method (we don't get an OK), so we simulate the OK after sending it */
        if ( conn->callbacks != NULL )
        {
            event.type = WICED_MQTT_EVENT_TYPE_PUBLISH_MSG_RECEIVED;
            event.data.pub_recvd.topic = args->topic.str;
            event.data.pub_recvd.topic_len = args->topic.len;
            event.data.pub_recvd.data = args->data;
            event.data.pub_recvd.data_len = args->data_len;
            conn->callbacks( (void*) conn, &event );
        }
    }
    return ret;
}

wiced_result_t mqtt_backend_put_puback( const mqtt_puback_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret;
    mqtt_frame_t frame;

    /* Send Protocol Header */
    ret = mqtt_frame_create( MQTT_CONNECTION_FRAME_MAX, &frame, &conn->socket );
    if ( ret != WICED_SUCCESS )
    {
        return ret;
    }
    mqtt_frame_put_puback( &frame, args );
    return mqtt_frame_send( &frame, &conn->socket );
}

wiced_result_t mqtt_backend_get_puback( mqtt_puback_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret;
    ret = mqtt_manager( MQTT_EVENT_RECV_PUBACK, args, conn );
    if ( ret == WICED_SUCCESS )
    {
        if ( conn->callbacks != NULL )
        {
            wiced_mqtt_event_info_t event;
            event.type = WICED_MQTT_EVENT_TYPE_PUBLISHED;
            event.data.msgid = args->packet_id;
            conn->callbacks( (void*) conn, &event );
        }
    }
    return ret;
}

wiced_result_t mqtt_backend_put_pubrec( const mqtt_pubrec_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret;
    mqtt_frame_t frame;

    /* Send Protocol Header */
    ret = mqtt_frame_create( MQTT_CONNECTION_FRAME_MAX, &frame, &conn->socket );
    if ( ret != WICED_SUCCESS )
    {
        return ret;
    }
    mqtt_frame_put_pubrec( &frame, args );
    return mqtt_frame_send( &frame, &conn->socket );
}

wiced_result_t mqtt_backend_get_pubrec( mqtt_pubrec_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret;
    ret = mqtt_manager( MQTT_EVENT_RECV_PUBREC, args, conn );
    if ( ret == WICED_SUCCESS )
    {
        /* Publish is an async method (we don't get an OK), so we simulate the OK after sending it */
        if ( conn->callbacks != NULL )
        {
            wiced_mqtt_event_info_t event;
            event.type = WICED_MQTT_EVENT_TYPE_UNKNOWN;
            event.data.msgid = args->packet_id;
            conn->callbacks( (void*) conn, &event );
        }

    }
    return ret;
}

wiced_result_t mqtt_backend_put_pubrel( const mqtt_pubrel_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret;
    mqtt_frame_t frame;

    /* Send Protocol Header */
    ret = mqtt_frame_create( MQTT_CONNECTION_FRAME_MAX, &frame, &conn->socket );
    if ( ret != WICED_SUCCESS )
    {
        return ret;
    }
    mqtt_frame_put_pubrel( &frame, args );
    return mqtt_frame_send( &frame, &conn->socket );
}

wiced_result_t mqtt_backend_get_pubrel( mqtt_pubrel_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret;
    ret = mqtt_manager( MQTT_EVENT_RECV_PUBREL, args, conn );
    if ( ret == WICED_SUCCESS )
    {
        if ( conn->callbacks != NULL )
        {
            wiced_mqtt_event_info_t event;
            event.type = WICED_MQTT_EVENT_TYPE_UNKNOWN;
            event.data.msgid = args->packet_id;
            conn->callbacks( (void*) conn, &event );
        }

    }
    return ret;
}

wiced_result_t mqtt_backend_put_pubcomp( const mqtt_pubcomp_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret;
    mqtt_frame_t frame;

    /* Send Protocol Header */
    ret = mqtt_frame_create( MQTT_CONNECTION_FRAME_MAX, &frame, &conn->socket );
    if ( ret != WICED_SUCCESS )
    {
        return ret;
    }
    mqtt_frame_put_pubcomp( &frame, args );
    return mqtt_frame_send( &frame, &conn->socket );
}

wiced_result_t mqtt_backend_get_pubcomp( mqtt_pubcomp_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret;
    ret = mqtt_manager( MQTT_EVENT_RECV_PUBCOMP, args, conn );
    if ( ret == WICED_SUCCESS )
    {
        /* Publish is an async method (we don't get an OK), so we simulate the OK after sending it */
        if ( conn->callbacks != NULL )
        {
            wiced_mqtt_event_info_t event;
            event.type = WICED_MQTT_EVENT_TYPE_PUBLISHED;
            event.data.msgid = args->packet_id;
            conn->callbacks( (void*) conn, &event );
        }

    }
    return ret;
}

wiced_result_t mqtt_backend_put_subscribe( const mqtt_subscribe_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret;
    mqtt_frame_t frame;
    mqtt_subscribe_arg_t final_args = *args;

    /* Send Protocol Header */
    ret = mqtt_frame_create( MQTT_CONNECTION_FRAME_MAX, &frame, &conn->socket );
    if ( ret != WICED_SUCCESS )
    {
        return ret;
    }
    mqtt_frame_put_subscribe( &frame, &final_args );
    return mqtt_frame_send( &frame, &conn->socket );
}

wiced_result_t mqtt_backend_get_suback( wiced_mqtt_suback_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret;
    ret = mqtt_manager( MQTT_EVENT_RECV_SUBACK, args, conn );
    if ( ret == WICED_SUCCESS )
    {
        /* Publish is an async method (we don't get an OK), so we simulate the OK after sending it */

        if ( conn->callbacks != NULL )
        {

            wiced_mqtt_event_info_t event;
            event.type = WICED_MQTT_EVENT_TYPE_SUBCRIBED;
            event.data.msgid = args->packet_id;
            conn->callbacks( (void*) conn, &event );
        }

    }
    return ret;
}

wiced_result_t mqtt_backend_put_unsubscribe( const mqtt_unsubscribe_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret;
    mqtt_frame_t frame;
    mqtt_unsubscribe_arg_t final_args = *args;

    /* Generate packet ID */
    // final_args.packet_id = conn->packet_id++;

    /* Send Protocol Header */
    ret = mqtt_frame_create( MQTT_CONNECTION_FRAME_MAX, &frame, &conn->socket );
    if ( ret != WICED_SUCCESS )
    {
        return ret;
    }
    mqtt_frame_put_unsubscribe( &frame, &final_args );
    return mqtt_frame_send( &frame, &conn->socket );
}

wiced_result_t mqtt_backend_get_unsuback( mqtt_unsuback_arg_t *args, mqtt_connection_t *conn )
{
    wiced_result_t ret;
    ret = mqtt_manager( MQTT_EVENT_RECV_UNSUBACK, args, conn );
    if ( ret == WICED_SUCCESS )
    {

        if ( conn->callbacks != NULL )
        {

            wiced_mqtt_event_info_t event;
            event.type = WICED_MQTT_EVENT_TYPE_UNSUBSCRIBED;
            event.data.msgid = args->packet_id;

            conn->callbacks( (void*) conn, &event );
        }
    }
    return ret;
}

wiced_result_t mqtt_backend_put_disconnect( mqtt_connection_t *conn )
{
    wiced_result_t ret;
    mqtt_frame_t frame;

    ret = mqtt_frame_create( MQTT_CONNECTION_FRAME_MAX, &frame, &conn->socket );
    if ( ret != WICED_SUCCESS )
    {
        return ret;
    }
    mqtt_frame_put_disconnect( &frame );
    return mqtt_frame_send( &frame, &conn->socket );
}

wiced_result_t mqtt_backend_connection_close( mqtt_connection_t *conn )
{
    wiced_result_t ret;
    ret = mqtt_manager( MQTT_EVENT_CONNECTION_CLOSE, NULL, conn );
    if ( ret == WICED_SUCCESS )
    {
        /* Publish is an async method (we don't get an OK), so we simulate the OK after sending it */

        if ( conn->callbacks != NULL )
        {

            wiced_mqtt_event_info_t event;
            event.type = WICED_MQTT_EVENT_TYPE_DISCONNECTED;
            conn->callbacks( (void*) conn, &event );
        }
        conn->callbacks = NULL;
    }
    return ret;
}

wiced_result_t mqtt_backend_put_pingreq( mqtt_connection_t *conn )
{
    wiced_result_t ret;
    mqtt_frame_t frame;

    ret = mqtt_frame_create( MQTT_CONNECTION_FRAME_MAX, &frame, &conn->socket );
    if ( ret != WICED_SUCCESS )
    {
        return ret;
    }
    mqtt_frame_put_pingreq( &frame );
    return mqtt_frame_send( &frame, &conn->socket );
}

wiced_result_t mqtt_backend_get_pingres( mqtt_connection_t *conn )
{
    return mqtt_manager( MQTT_EVENT_RECV_PINGRES, NULL, conn );
}
