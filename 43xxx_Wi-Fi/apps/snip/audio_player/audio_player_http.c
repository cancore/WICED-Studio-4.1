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

/** @file Audio Player HTTP Routines
 *
 */

#include <ctype.h>
#include <string.h>

#include "wiced.h"
#include "audio_player.h"
#include "audio_player_util.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define AUDIO_PLAYER_HTTP_THREAD_PRIORITY       (WICED_DEFAULT_LIBRARY_PRIORITY)
#define AUDIO_PLAYER_HTTP_STACK_SIZE            (8 * 1024)

#define NUM_HTTP_HEADERS                        (10)

#define HTTP_STATUS_OK                          (200)

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

/* template for HTTP GET */
char audio_player_get_request_template[] =
{
    "GET %s HTTP/1.1\r\n"
    "Host: %s:%d \r\n"
    "\r\n"
};

/******************************************************
 *               Function Definitions
 ******************************************************/

static wiced_result_t check_HTTP_return_code(audio_player_t* player, wiced_packet_t* reply_packet)
{
    wiced_result_t  result;
    char*           data;
    uint16_t        avail_data_length;
    uint16_t        total_data_length;
    char*           ptr;
    int             code = 0;

    /*
     * Check for a good HTTP return code.
     */

    result = wiced_packet_get_data(reply_packet, 0, (uint8_t**)&data, &avail_data_length, &total_data_length);
    if (result == WICED_SUCCESS)
    {
        if (strncmp(data, "HTTP", 4) == 0)
        {
            ptr = &data[9];
            while (*ptr == ' ' || *ptr == '\t')
            {
                ptr++;
            }

            if (isdigit((int)*ptr))
            {
                code = atoi(ptr);
            }

            if (code != HTTP_STATUS_OK)
            {
                ptr = strchr(data, '\r');
                if (ptr != NULL)
                {
                    *ptr = '\0';
                }
                else
                {
                    data[avail_data_length - 1] = '\0';
                }
                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "HTTP Error: %s\n", data);

                return WICED_ERROR;
            }
        }
    }

    return (code == HTTP_STATUS_OK) ? WICED_SUCCESS : WICED_ERROR;
}


static void send_empty_buffer_to_decoder(audio_player_t* player)
{
    data_buf_t* dbuf;

    /*
     * Tell the decoder that we're all done sending data by sending a zero length buffer.
     */

    while (player->http_done == WICED_FALSE)
    {
        dbuf = &player->dbufs[player->dbuf_write_idx];

        if (dbuf->inuse)
        {
            audio_player_log_msg(AUDIO_PLAYER_LOG_DEBUG4, "Receive buffers full (%d) - buffer dropped\n", player->dbuf_write_idx);
            wiced_rtos_delay_milliseconds(2);
            continue;
        }

        dbuf->buflen = 0;
        dbuf->inuse  = 1;
        player->dbuf_write_idx = (player->dbuf_write_idx + 1) % AUDIO_PLAYER_DBUF_NUM_BUFS;
        wiced_rtos_set_event_flags(&player->decoder_events, DECODER_EVENT_AUDIO_DATA);
        break;
    }
}


static void audio_player_http_play_thread(uint32_t arg)
{
    audio_player_t*     player = (audio_player_t*)arg;
    data_buf_t*         dbuf;
    wiced_packet_t*     reply_packet = NULL;
    wiced_bool_t        looking_for_headers;
    wiced_result_t      result;
    uint16_t            port;
    uint8_t*            in_data;
    uint16_t            avail_data_length;
    uint16_t            total_data_length;
    uint8_t*            body;
    uint32_t            body_length;
    uint32_t            content_length;
    char                host_name[MAX_HTTP_HOST_NAME_SIZE + 1];
    char                object_path[MAX_HTTP_OBJECT_PATH + 1];
    http_header_t       headers[NUM_HTTP_HEADERS];
    uint16_t            num_headers;
    int                 initial_buffer_count = 0;

    if (player == NULL || player->tag != PLAYER_TAG_VALID)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_http_play_thread: Bad ARG\n");
        return;
    }

    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "Stream: %s\n", player->uri_to_stream);
    audio_player_uri_split(player->uri_to_stream, host_name, sizeof(host_name), object_path, sizeof(object_path), &port);

    if (strlen(host_name) < 4)
    {
        if (strlen(player->last_connected_host_name) >= 4)
        {
            strcpy(host_name, player->last_connected_host_name);
            strcpy(player->server_uri, player->last_connected_host_name);
        }
    }
    else
    {
        strcpy(player->server_uri, host_name);
    }

    result = audio_player_connect(player);
    if (result != WICED_SUCCESS)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_connect failed %d\n", result);
        goto _exit_http_thread;
    }

    sprintf(player->http_query, audio_player_get_request_template, player->uri_to_stream, host_name, player->last_connected_port);
    audio_player_log_msg(AUDIO_PLAYER_LOG_DEBUG0, "Sending query: [%s]\n", player->http_query);

    result = wiced_tcp_send_buffer( &player->tcp_socket, player->http_query, (uint16_t)strlen(player->http_query) );
    if ( result != WICED_SUCCESS )
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_http_play_thread: wiced_tcp_send_buffer failed\n");
        goto _exit_http_thread;
    }

    content_length      = 0;
    looking_for_headers = WICED_TRUE;
    while (result == WICED_SUCCESS && player->http_done == WICED_FALSE)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_DEBUG0, "waiting for HTTP reply for %s  %s [%s]\n", host_name, object_path, player->http_query);

        result = wiced_tcp_receive(&player->tcp_socket, &reply_packet, WICED_WAIT_FOREVER);
        if (result != WICED_SUCCESS)
        {
            if (result == WICED_TIMEOUT)
            {
                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_http_play_thread: receive timed out\n");
                /* re- try */
                result = WICED_SUCCESS;
                continue;
            }

            if (!player->http_done)
            {
                if (result == WICED_TCPIP_SOCKET_CLOSED)
                {
                    audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_http_play_thread: peer closed socket\n");
                }
                else
                {
                    audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_http_play_thread: receive failed (%d)\n", result);
                }
            }
            break;
        }

        if (looking_for_headers == WICED_TRUE)
        {
            /*
             * Check for a good HTTP return code.
             */

            if (check_HTTP_return_code(player, reply_packet) != WICED_SUCCESS)
            {
                break;
            }

            /*
             * Check for the location of the body of the HTTP response before processing the headers.
             * http_process_headers_in_place() modifies the data in the buffer during parsing.
             */

            result = http_get_body(reply_packet, &body, &body_length);
            if (result != WICED_SUCCESS || body == NULL)
            {
                /*
                 * The entire HTTP header need to be in the first TCP packet or our parsing won't work properly.
                 */

                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_http_play_thread: Entire HTTP header not in first packet\n");
                break;
            }

            num_headers = NUM_HTTP_HEADERS;
            result = http_process_headers_in_place(reply_packet, headers, &num_headers);
            if (result == WICED_SUCCESS)
            {
                int i;

                for (i = 0; i < num_headers; i++)
                {
                    audio_player_log_msg(AUDIO_PLAYER_LOG_DEBUG0, "HTTP Header %s, value %s\n", headers[i].name, headers[i].value);
                    if (strcasecmp(headers[i].name, "Content-Length") == 0)
                    {
                        player->http_source.size = atol(headers[i].value);
                        audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "audio_player_http_play_thread: Content-length: %ld\n", player->http_source.size);
                        break;
                    }
                }

                looking_for_headers = WICED_FALSE;
            }
            else
            {
                /*
                 * The HTTP headers need to be in the first TCP packet or our parsing won't work properly.
                 */

                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_http_play_thread: No HTTP headers in first packet\n");
                break;
            }

            audio_player_log_msg(AUDIO_PLAYER_LOG_DEBUG0, "audio_player_http_play_thread: body:%p body_length:%ld\n", body, body_length);
        }
        else
        {
            result = wiced_packet_get_data(reply_packet, 0, &in_data, &avail_data_length, &total_data_length);
            if (result == WICED_SUCCESS)
            {
                body        = in_data;
                body_length = avail_data_length;
            }
            else
            {
                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_http_play_thread: error getting packet data %d\n", result);
                break;
            }
        }

        content_length += body_length;

        /*
         * Copy the packet data into a buffer to be processed.
         */

        while (player->http_done == WICED_FALSE)
        {
            dbuf = &player->dbufs[player->dbuf_write_idx];

            if (dbuf->inuse)
            {
                audio_player_log_msg(AUDIO_PLAYER_LOG_DEBUG4, "Receive buffers full (%d)\n", player->dbuf_write_idx);
                wiced_rtos_delay_milliseconds(4);
                continue;
            }

            if (body_length > AUDIO_PLAYER_DBUF_SIZE)
            {
                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Receive buffers too small!!! %d (max %d)\n", body_length, AUDIO_PLAYER_DBUF_SIZE);
                player->http_done = WICED_TRUE;
                break;
            }

            memcpy(&dbuf->buf, body, body_length);
            dbuf->buflen = body_length;
            dbuf->inuse  = 1;
            player->dbuf_write_idx = (player->dbuf_write_idx + 1) % AUDIO_PLAYER_DBUF_NUM_BUFS;

            /*
             * Let the decoder know that there is data available.
             * Queue up the first set of buffers before kicking the decoder to help deal with network lag.
             */

            if (initial_buffer_count > AUDIO_PLAYER_DBUF_NUM_BUFS - 5)
            {
                wiced_rtos_set_event_flags(&player->decoder_events, DECODER_EVENT_AUDIO_DATA);
            }
            else
            {
                initial_buffer_count++;
            }
            break;
        }

        wiced_packet_delete(reply_packet);
        reply_packet = NULL;

        if (content_length >= player->http_source.size)
        {
            break;
        }
    } /* while still streaming data */

_exit_http_thread:
    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "audio_player_http_play_thread - exiting\n");

    /*
     * Tell the decoder that we're all done sending data.
     */

    send_empty_buffer_to_decoder(player);

    audio_player_disconnect(player);

    if (reply_packet != NULL)
    {
        wiced_packet_delete(reply_packet);
    }

    wiced_rtos_set_event_flags(&player->player_events, PLAYER_EVENT_HTTP_THREAD_DONE);

    WICED_END_OF_CURRENT_THREAD();
}


wiced_result_t audio_player_http_stream_start(audio_player_t* player)
{
    wiced_result_t result;

    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "audio_player_http_stream_start\n");

    if (player == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_http_stream_start: Bad ARG\n");
        return WICED_BADARG;
    }

    /* Start http thread */
    player->http_done = WICED_FALSE;
    result = wiced_rtos_create_thread(&player->http_thread, AUDIO_PLAYER_HTTP_THREAD_PRIORITY, "Audio Player HTTP", audio_player_http_play_thread, AUDIO_PLAYER_HTTP_STACK_SIZE, player);
    if (result != WICED_SUCCESS)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_http_stream_start: wiced_rtos_create_thread(http) failed %d\n", result);
        return WICED_ERROR;
    }
    else
    {
        player->http_thread_ptr = &player->http_thread;
    }

    return result;
}


wiced_result_t audio_player_http_stream_stop(audio_player_t* player)
{
    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "audio_player_http_stream_stop\n");

    if (player == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_http_stream_stop: Bad ARG\n");
        return WICED_BADARG;
    }

    player->http_done = WICED_TRUE;
    if (player->http_thread_ptr != NULL)
    {
        wiced_rtos_thread_force_awake(&player->http_thread);
        wiced_rtos_thread_join(&player->http_thread);
        wiced_rtos_delete_thread(&player->http_thread);
        player->http_thread_ptr = NULL;
    }

    return WICED_SUCCESS;
}
