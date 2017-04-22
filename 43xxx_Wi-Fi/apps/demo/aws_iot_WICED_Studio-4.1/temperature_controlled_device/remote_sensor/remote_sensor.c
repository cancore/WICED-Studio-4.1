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
 * This application should be run on the sensors. The sensor used here is Thermistor (temperature sensor).
 * The app gets outside temperature from openweathermap.org and room temperature from the sensor.
 * Finds the difference and publishes ON/OFF to the device (thing name) which it wants to control smartly.
 *
 * This application reads room temperature every 5 seconds and gets outside temperature from www.openweathermap.org.
 * And publishes "ON" or "OFF" based on the difference in temperature with QOS-1.
 * If temperature difference is >= 5 degrees it publishes "ON" to the topic configured via Web-Ui else publishes "OFF".
 *
 * To demonstrate the app, work through the following steps.
 * 1. Make sure AWS Root Certifcate 'resources/apps/aws_iot/rootca.cer' is proper while building the app.
 * 2. Configuring the sensor using Web-UI:
 *    a) Connect from a PC to the sensor SSID "WICED_AWS" which runs as soft AP. SSID and credentials are as per DCT configuration.
 *    b) After successful connection, this sensor will act as WebServer with IP address 192.168.0.1.
 *    c) From host system type URL in browser as 192.168.0.1.
 *    d) Configure settings for thing name and upload certificate and private key. And join to a router which is connected to internet.
 *    e) The WICED board (sensor) will now reboot.
 * 3. The device will connect to selected Wi-Fi configurations. And then connect to broker.
 * 4. To send "ON" to the device try to make the temperature difference >= 5 degrees else make it < 5.
 *
 * NOTE: In this APP, for getting weather data from openweathermap.org, Cypress specific user ID is used.
 *       Follow the mentioned link http://openweathermap.org/appid to generate own APPID
 *       Please use your own user ID instead for better results. Currently used ID "APPID=611684a92a4de8b5f4281ecb71f12002".
 *
 */

#include "wiced.h"
#include "mqtt_api.h"
#include "resources.h"
#include <math.h>
#include "thermistor.h" // Using Murata NCP18XH103J03RB thermistor
#include "http.h"
#include "JSON.h"
#include "aws_common.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define CLIENT_ID                           "wiced_publisher_aws"
#define BUFFER_LENGTH                       (2048)
#define MQTT_PUBLISH_RETRY_COUNT            (3)
#define WEATHER_HOST_NAME                   "api.openweathermap.org"
#define WEATHER_CITY_NAME                   "bangalore"
/* Follow the http://openweathermap.org/appid to get your own APP ID */
#define WEATHER_USER_ID                     "APPID=611684a92a4de8b5f4281ecb71f12002"
#define WEATHER_HTTP_GET_REQUEST            "GET /data/2.5/weather?q="WEATHER_CITY_NAME"&"WEATHER_USER_ID" HTTP/1.1\r\n" \
                                            "Host: "WEATHER_HOST_NAME"\r\n" \
                                            "Connection: close\r\n" \
                                            "\r\n"
#define MSG_ON                              "ON"
#define MSG_OFF                             "OFF"

/******************************************************
 *                    Structures
 ******************************************************/
typedef struct
{
    int16_t               last_sample;
} temp_data_t;

/******************************************************
 *               Variable Definitions
 ******************************************************/
static aws_app_info_t  app_info =
{
    .mqtt_client_id = CLIENT_ID
};

static temp_data_t               temperature_data;
static float                     outside_temperature;
static wiced_ip_address_t        http_address;
static uint8_t                   buffer[BUFFER_LENGTH];

/******************************************************
 *               Static Function Definitions
 ******************************************************/
static wiced_result_t parse_json_weather_info(wiced_json_object_t * json_object )
{
    if(strncmp(json_object->object_string, "temp", json_object->object_string_length) == 0)
    {
        outside_temperature = atof(json_object->value);
        WPRINT_APP_DEBUG(("json_object->value [%.*s] outside_temperature [%f]\n", json_object->value_length, json_object->value, outside_temperature));
    }

    return WICED_SUCCESS;
}

static wiced_result_t get_weather_info( float *value)
{
    int http_status_code = 0;
    wiced_result_t result;
    char *data;
    int content_length = 0;

    result = wiced_http_get( &http_address, WEATHER_HTTP_GET_REQUEST, buffer, sizeof( buffer ) );
    if ( result == WICED_SUCCESS )
    {
        http_status_code = atoi((char*)(buffer+(strlen( "HTTP/1.1" ) + 1)));
        if ( http_status_code < 200 || http_status_code > 299 )
        {
            WPRINT_APP_INFO( ( "HTTP error code [%d] while getting weather data!\n", http_status_code ) );
            return WICED_UNSUPPORTED;
        }

        data = strcasestr( (char*) buffer, "Content-Length:" );
        if(data != NULL)
        {
            data += strlen("Content-Length:");
            content_length = atoi((char*)(data));

            data = strstr( (char*) data, "\r\n\r\n" );
            if ( data != NULL )
            {
                data += strlen("\r\n\r\n");
                result = wiced_JSON_parser( (const char*)data,  content_length);
                if(result == WICED_SUCCESS)
                {
                    *value = outside_temperature;
                }
            }
        }
    }
    else
        WPRINT_APP_INFO( ( "Get failed: %u\n", result ) );

    return result;
}

/*
 * Call back function to handle MQTT events
 */
wiced_result_t mqtt_connection_event_cb( wiced_mqtt_object_t mqtt_object, wiced_mqtt_event_info_t *event )
{
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
        default:
            break;
    }
    return WICED_SUCCESS;
}

/******************************************************
 *               Function Definitions
 ******************************************************/
/*
 * Main application
 */
void application_start( void )
{
    float            value = 0, delta = 0;
    char            *msg = MSG_OFF;
    wiced_result_t   ret = WICED_SUCCESS;
    int              retries;

    ret = aws_app_init(&app_info);

    wiced_JSON_parser_register_callback(parse_json_weather_info);

    ret = wiced_hostname_lookup( WEATHER_HOST_NAME, &http_address, 10000 );
    if ( ret == WICED_ERROR )
    {
        WPRINT_APP_INFO(("Error in resolving DNS\n"));
        return;
    }
    WPRINT_APP_INFO( ( "Weather Server IP: %u.%u.%u.%u\n", (uint8_t)(GET_IPV4_ADDRESS(http_address) >> 24),
                    (uint8_t)(GET_IPV4_ADDRESS(http_address) >> 16),
                    (uint8_t)(GET_IPV4_ADDRESS(http_address) >> 8),
                    (uint8_t)(GET_IPV4_ADDRESS(http_address) >> 0) ) );

    /* Initialize thermistor */
    wiced_adc_init( WICED_THERMISTOR_JOINS_ADC, 5 );
    memset( &temperature_data, 0, sizeof( temperature_data ) );

    do
    {
        ret = aws_mqtt_conn_open( app_info.mqtt_object, mqtt_connection_event_cb );
        if ( ret != WICED_SUCCESS )
        {
            WPRINT_APP_INFO(("Failed\n"));
            break;
        }

        while ( 1 )
        {
            /* Getting the temperature value for bangalore location from www.openweathermap.org */
            if ( get_weather_info( &value ) == WICED_SUCCESS )
            {
                /* Reading temperature sensor value */
                thermistor_take_sample( WICED_THERMISTOR_JOINS_ADC, &temperature_data.last_sample );
                delta = temperature_data.last_sample - value;
                WPRINT_APP_INFO(( "Delta = %.1f - %.1f = %.1f degrees\n", temperature_data.last_sample/10.0, value/10.0, (delta/10) ));

                if ( abs( delta ) >= 50 )
                {
                    msg = MSG_ON;
                }
                else
                {
                    msg = MSG_OFF;
                }
                /* Controlling the LED by publishing to mqtt topic "WICED_BULB" */
                retries = 0;
                do
                {
                    ret = aws_mqtt_app_publish( app_info.mqtt_object, WICED_MQTT_QOS_DELIVER_AT_LEAST_ONCE, (uint8_t*) app_info.thing_name, (uint8_t*) msg, strlen( msg ) );
                    retries++;
                } while ( ( ret != WICED_SUCCESS ) && ( retries < MQTT_PUBLISH_RETRY_COUNT ) );
                if ( ret != WICED_SUCCESS )
                {
                    break;
                }
            }

            wiced_rtos_delay_milliseconds( 5000 );
        }

        aws_mqtt_conn_close( app_info.mqtt_object );

        wiced_rtos_delay_milliseconds( MQTT_DELAY_IN_MILLISECONDS * 2 );
    } while ( 1 );

    aws_mqtt_conn_close( app_info.mqtt_object );

    wiced_rtos_deinit_semaphore( &app_info.msg_semaphore );
    WPRINT_APP_INFO(("[MQTT] Deinit connection...\n"));
    ret = wiced_mqtt_deinit( app_info.mqtt_object );
    free( app_info.mqtt_object );
    app_info.mqtt_object = NULL;

    return;
}
