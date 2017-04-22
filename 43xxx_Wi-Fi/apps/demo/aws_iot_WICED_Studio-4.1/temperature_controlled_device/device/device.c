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
 * This application should run on the device which needs to be controlled using IOT protocols and Cloud.
 * The device used here is LED1 on the WICED board.
 *
 * It has 2 states (ON/OFF) which could be controlled using one of the below mentioned topics which the device subscribes:
 * a) Name of this device (thing name). Whoever publishes message to this thing name (device) can control the device.
 *    This can be treated as SMART CONTROL of the device. This publish message can be triggered from one of the below:
 *    i) AWS Ice Breaker.
 *    ii) Other AWS Cloud service like Lambda.
 *    iii) Directly from remote sensor. We are using this for this demo.
 * b) AWS shadow for this thing name (device). This is used to create a virtual instance of this device in cloud.
 *    Users can get to know the status of the thing (device) by checking shadow. Also the device can be controlled using Shadow.
 *    This topic is derived from thing name which needs to be configured as already mentioned in item a) above.
 *    This can be treated as REMOTE CONTROL of the device.
 *
 * Also the device can be configured to be smart controlled or not using 2 ways:
 * i) Press SW3 button to allow/disallow smart control.
 * ii) Update "auto" element in "desired" object to "YES" or "NO" in AWS Shadow for the given thing name
 *
 * The device publishes to shadow when there is any change in its status.
 *
 * To demonstrate the app, work through the following steps.
 * 1. Make sure AWS Root Certifcate 'resources/apps/aws_iot/rootca.cer' is proper while building the app.
 * 2. Configuring the device using Web-UI:
 *    a) Connect from a PC to the device SSID "WICED_AWS" which runs as soft AP. SSID and credentials are as per DCT configuration.
 *    b) After successful connection, this device will act as WebServer with IP address 192.168.0.1.
 *    c) From host system type URL in browser as 192.168.0.1.
 *    d) Configure settings for thing name and upload certificate and private key. And join to a router which is connected to internet.
 *    e) The device will now reboot.
 * 3. The device will connect to selected Wi-Fi configurations. And then connect to broker.
 * 4. Try to control the device with one of the following:
 *    a) Start sensor app and try to get ON/OFF published to the device thing name.
 *    b) Update desired shadow status (ON/OFF) for the device thing name in the AWS IOT service.
 *    Item a) can be disabled/enabled by toggling the SW3 button on the device or by changing the "auto" state in the shadow to YES/NO.
 *
 */

#include "wiced.h"
#include "mqtt_api.h"
#include "resources.h"
#include "JSON.h"
#include "aws_common.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define SHADOW_PUBLISH_MESSAGE_STR_OFF_DESIRED_AND_REPORTED    "{\
                                                                    \"state\":\
                                                                    {\
                                                                        \"desired\": { \"status\": \"OFF\" , \"auto\": \"NO\"} ,\
                                                                        \"reported\": { \"status\": \"OFF\" ,\"auto\": \"NO\"} \
                                                                    }\
                                                                }"

#define SHADOW_PUBLISH_MESSAGE_STR_ON_DESIRED_AND_REPORTED     "{\
                                                                    \"state\":\
                                                                    {\
                                                                        \"desired\": { \"status\": \"ON\" , \"auto\": \"YES\" } ,\
                                                                        \"reported\": { \"status\": \"ON\" ,\"auto\": \"YES\"} \
                                                                    }\
                                                                }"
#define SHADOW_PUBLISH_MESSAGE_STR_OFF_DESIRED_AND_REPORTED_STATUS_ON    "{\
                                                                    \"state\":\
                                                                    {\
                                                                        \"desired\": { \"status\": \"OFF\" , \"auto\": \"YES\" } ,\
                                                                        \"reported\": { \"status\": \"OFF\" ,\"auto\": \"YES\"} \
                                                                    }\
                                                                }"

#define SHADOW_PUBLISH_MESSAGE_STR_ON_DESIRED_AND_REPORTED_STATUS_OFF     "{\
                                                                    \"state\":\
                                                                    {\
                                                                        \"desired\": { \"status\": \"ON\" , \"auto\": \"NO\" },\
                                                                        \"reported\": { \"status\": \"ON\" ,\"auto\": \"NO\"}\
                                                                    }\
                                                                }"

#define CLIENT_ID                           "wiced_subcriber_aws"
#define WICED_MQTT_SUBSCRIBE_RETRY_COUNT    (3)

/******************************************************
 *               Variable Definitions
 ******************************************************/
static aws_app_info_t app_info =
{
    .mqtt_client_id = CLIENT_ID
};
static char*          led_status = "OFF";
static char           req_led_status[8] = "OFF";

static uint8_t        smart_control = 0;

/******************************************************
 *               Static Function Definitions
 ******************************************************/
static wiced_result_t parse_json_shadow_status(wiced_json_object_t * json_object )
{
    if(strncmp(json_object->object_string, "status", strlen("status")) == 0)
    {
        if(json_object->value_length > 0 && json_object->value_length < sizeof(req_led_status)-1)
        {
            memcpy(req_led_status, json_object->value, json_object->value_length);
            req_led_status[json_object->value_length] = '\0';
        }
    }
    else if(strncmp(json_object->object_string, "auto", strlen("auto")) == 0)
    {
        if ( strncmp( json_object->value, "YES", 3 ) == 0 )
        {
            smart_control = 1;
        }
        else
        {
            smart_control = 0;
        }
        WPRINT_APP_INFO (( "smart_control %d\n", smart_control ));
    }

    return WICED_SUCCESS;
}


/*
 * Call back function to handle connection events.
 */
wiced_result_t mqtt_connection_event_cb( wiced_mqtt_object_t mqtt_object, wiced_mqtt_event_info_t *event )
{
    wiced_result_t ret = WICED_SUCCESS;

    switch ( event->type )
    {
        case WICED_MQTT_EVENT_TYPE_CONNECT_REQ_STATUS:
        case WICED_MQTT_EVENT_TYPE_DISCONNECTED:
        case WICED_MQTT_EVENT_TYPE_PUBLISHED:
        case WICED_MQTT_EVENT_TYPE_SUBCRIBED:
        case WICED_MQTT_EVENT_TYPE_UNSUBSCRIBED:
        {
            app_info.expected_event = event->type;
            wiced_rtos_set_semaphore( &app_info.msg_semaphore );
        }
            break;
        case WICED_MQTT_EVENT_TYPE_PUBLISH_MSG_RECEIVED:
        {
            wiced_mqtt_topic_msg_t msg = event->data.pub_recvd;
            WPRINT_APP_DEBUG(( "[MQTT] Received %.*s  for TOPIC : %.*s\n", (int) msg.data_len, msg.data, (int) msg.topic_len, msg.topic ));

            if ( strncmp( (char*) msg.topic, (const char*) app_info.thing_name, msg.topic_len ) == 0 )
            {
                WPRINT_APP_INFO (( "From Sensor :: Requested LED State [%.*s]. Current LED State [%s]\n", (int) msg.data_len, msg.data, led_status ));

                if ( strncasecmp( (char*) msg.data, led_status, msg.data_len ) )
                {
                    if ( smart_control == 1 )
                    {
                        if ( strncmp( (char*) msg.data, "ON", 2 ) == 0 )
                            led_status = "ON";
                        else
                            led_status = "OFF";

                        wiced_rtos_set_semaphore( &app_info.wake_semaphore );
                    }
                }
                else
                {
                    break;
                }
            }
            else if ( strncmp( (char*) msg.topic, app_info.shadow_delta_topic, msg.topic_len ) == 0 )
            {
                ret = wiced_JSON_parser( (const char*)msg.data , msg.data_len );
                if ( ret == WICED_SUCCESS )
                {
                    WPRINT_APP_INFO(( "From Shadow :: Requested LED State [%s]. Current LED State [%s]\n", req_led_status, led_status ));
                    if ( strncmp( req_led_status, "ON", 2 ) == 0 )
                    {
                        led_status = "ON";
                    }
                    else
                    {
                        led_status = "OFF";
                    }
                    wiced_rtos_set_semaphore( &app_info.wake_semaphore );
                }
            }
            else
            {
                WPRINT_APP_INFO(( "Topic Not found\n" ));
            }
        }
            break;
        default:
            break;
    }
    return WICED_SUCCESS;
}

static void publish_callback( void* arg )
{
    if ( smart_control == 0 )
    {
        smart_control = 1;
    }
    else
    {
        smart_control = 0;
    }

    WPRINT_APP_INFO (( "smart_control %d\n", smart_control ));
}
/******************************************************
 *               Function Definitions
 ******************************************************/
void application_start( void )
{
    wiced_result_t ret = WICED_SUCCESS;
    uint32_t       connection_retries = 0;
    uint32_t       retries = 0;

    ret = aws_app_init( &app_info );

    wiced_JSON_parser_register_callback(parse_json_shadow_status);

    wiced_gpio_input_irq_enable( WICED_BUTTON1, IRQ_TRIGGER_RISING_EDGE, publish_callback, NULL );
    do
    {
        ret = aws_mqtt_conn_open( app_info.mqtt_object, mqtt_connection_event_cb );
        connection_retries++ ;
    } while ( ( ret != WICED_SUCCESS ) && ( connection_retries < WICED_MQTT_CONNECTION_NUMBER_OF_RETRIES ) );

    aws_mqtt_app_publish( app_info.mqtt_object, WICED_MQTT_QOS_DELIVER_AT_LEAST_ONCE, (uint8_t*) app_info.shadow_state_topic, (uint8_t*) SHADOW_PUBLISH_MESSAGE_STR_OFF_DESIRED_AND_REPORTED, sizeof( SHADOW_PUBLISH_MESSAGE_STR_OFF_DESIRED_AND_REPORTED ) );

    wiced_rtos_delay_milliseconds( MQTT_DELAY_IN_MILLISECONDS * 2 );

    aws_mqtt_app_subscribe( app_info.mqtt_object, app_info.shadow_delta_topic, WICED_MQTT_QOS_DELIVER_AT_LEAST_ONCE );

    do
    {
        ret = aws_mqtt_app_subscribe( app_info.mqtt_object, app_info.thing_name, WICED_MQTT_QOS_DELIVER_AT_MOST_ONCE );
        retries++ ;
    } while ( ( ret != WICED_SUCCESS ) && ( retries < WICED_MQTT_SUBSCRIBE_RETRY_COUNT ) );
    if ( ret != WICED_SUCCESS )
    {
        return;
    }

    while ( 1 )
    {
        /* Wait forever on wake semaphore until the LED status is changed */
        wiced_rtos_get_semaphore( &app_info.wake_semaphore, WICED_NEVER_TIMEOUT );

        /* Toggle the LED */
        if ( ( strncasecmp( led_status, "OFF", 3 ) == 0 ) && smart_control == 1 )
        {
            wiced_gpio_output_low( WICED_LED1 );
            led_status = "OFF";
            strcpy(req_led_status, led_status);
            WPRINT_APP_INFO(("[MQTT] Publishing to Thing state topic\n"));
            aws_mqtt_app_publish( app_info.mqtt_object, WICED_MQTT_QOS_DELIVER_AT_LEAST_ONCE, (uint8_t*) app_info.shadow_state_topic, (uint8_t*) SHADOW_PUBLISH_MESSAGE_STR_OFF_DESIRED_AND_REPORTED_STATUS_ON, sizeof( SHADOW_PUBLISH_MESSAGE_STR_OFF_DESIRED_AND_REPORTED_STATUS_ON ) );
        }
        else if ( ( strncasecmp( led_status, "ON", 2 ) == 0 ) && smart_control == 0 )
        {
            wiced_gpio_output_high( WICED_LED1 );
            led_status = "ON";
            strcpy(req_led_status, led_status);
            WPRINT_APP_INFO(("[MQTT] Publishing to Thing state topic\n"));
            aws_mqtt_app_publish( app_info.mqtt_object, WICED_MQTT_QOS_DELIVER_AT_LEAST_ONCE, (uint8_t*) app_info.shadow_state_topic, (uint8_t*) SHADOW_PUBLISH_MESSAGE_STR_ON_DESIRED_AND_REPORTED_STATUS_OFF, sizeof( SHADOW_PUBLISH_MESSAGE_STR_ON_DESIRED_AND_REPORTED_STATUS_OFF ) );
        }
        else if ( ( strncasecmp( led_status, "ON", 2 ) == 0 ) && smart_control == 1 )
        {
            wiced_gpio_output_high( WICED_LED1 );
            led_status = "ON";
            strcpy(req_led_status, led_status);
            WPRINT_APP_INFO(("[MQTT] Publishing to Thing state topic\n"));
            aws_mqtt_app_publish( app_info.mqtt_object, WICED_MQTT_QOS_DELIVER_AT_LEAST_ONCE, (uint8_t*) app_info.shadow_state_topic, (uint8_t*) SHADOW_PUBLISH_MESSAGE_STR_ON_DESIRED_AND_REPORTED, sizeof( SHADOW_PUBLISH_MESSAGE_STR_ON_DESIRED_AND_REPORTED ) );
        }
        else
        {
            wiced_gpio_output_low( WICED_LED1 );
            led_status = "OFF";
            strcpy(req_led_status, led_status);
            WPRINT_APP_INFO(("[MQTT] Publishing to Thing state topic\n"));
            aws_mqtt_app_publish( app_info.mqtt_object, WICED_MQTT_QOS_DELIVER_AT_LEAST_ONCE, (uint8_t*) app_info.shadow_state_topic, (uint8_t*) SHADOW_PUBLISH_MESSAGE_STR_OFF_DESIRED_AND_REPORTED, sizeof( SHADOW_PUBLISH_MESSAGE_STR_OFF_DESIRED_AND_REPORTED ) );
        }
    }

    aws_mqtt_conn_close( app_info.mqtt_object );

    wiced_rtos_deinit_semaphore( &app_info.msg_semaphore );
    ret = wiced_mqtt_deinit( app_info.mqtt_object );
    wiced_rtos_deinit_semaphore( &app_info.wake_semaphore );
    free( app_info.mqtt_object );
    app_info.mqtt_object = NULL;

    return;
}
