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
 * Audio Client HTTP processing
 *
 */

#include <ctype.h>

#include "dns.h"

#include "wiced_result.h"
#include "wiced_defaults.h"
#include "wiced_rtos.h"
#include "wiced_tcpip.h"
#include "wiced_network.h"
#include "wiced_log.h"

#include "audio_client_private.h"
#include "audio_client_utils.h"

#include "audio_client.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define PRINT_IP(addr)    (int)((addr.ip.v4 >> 24) & 0xFF), (int)((addr.ip.v4 >> 16) & 0xFF), (int)((addr.ip.v4 >> 8) & 0xFF), (int)(addr.ip.v4 & 0xFF)

/******************************************************
 *                    Constants
 ******************************************************/

#define AUDIO_CLIENT_DNS_TIMEOUT_MS         (10 * 1000)
#define AUDIO_CLIENT_CONNECT_TIMEOUT_MS     (2 * 1000)
#define AUDIO_CLIENT_RECEIVE_TIMEOUT_MS     (WICED_NO_WAIT)

#define AUDIO_CLIENT_HTTP_THREAD_STACK_SIZE (2 * 4096)
#define AUDIO_CLIENT_HTTP_THREAD_PRIORITY   (4)

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

static char audio_client_HTTP_get_request_template[] =
{
    "GET %s HTTP/1.1\r\n"
    "Host: %s:%d\r\n"
    "\r\n"
};

static char audio_client_HTTP_get_range_template[] =
{
    "GET %s HTTP/1.1\r\n"
    "Host: %s:%d\r\n"
    "Range: bytes=%d-\r\n"
    "\r\n"
};

static char audio_client_HTTP_response[] =
{
    "HTTP/1.1"
};

static char audio_client_HTTP_body_separator[] =
{
    "\r\n\r\n"
};

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

static wiced_result_t received_data_callback(wiced_tcp_socket_t* socket, void* arg)
{
    audio_client_t* client = (audio_client_t*)arg;

    if (client != NULL && client->tag == AUDIO_CLIENT_TAG_VALID)
    {
        wiced_rtos_set_event_flags(&client->http_events, HTTP_EVENT_TCP_DATA);
    }

    return WICED_SUCCESS;
}


static void reset_initial_http_variables(audio_client_t* client, wiced_bool_t seek_request)
{
    client->http_done            = WICED_FALSE;
    client->http_error           = WICED_FALSE;
    client->http_range_requests  = WICED_TRUE;
    client->http_need_header     = WICED_TRUE;
    client->http_in_header       = WICED_FALSE;
    client->http_first_kick      = WICED_TRUE;
    client->http_header_idx      = 0;
    client->http_buf_idx         = 0;
    client->http_body_idx        = 0;
    client->http_content_length  = 0;
    client->http_content_read    = 0;
    client->initial_buffer_count = 0;
    client->threshold_high_sent  = WICED_FALSE;
    client->threshold_low_sent   = WICED_FALSE;
    client->http_content_type[0] = '\0';
    client->seek_in_progress     = seek_request;

    if ( seek_request == WICED_FALSE )
    {
        client->http_total_content_length = 0;

    }
}


static wiced_result_t audio_client_uri_split(const char* uri, char* host, int host_len, char* path, int path_len, int* port)
{
    const char* uri_start;
    int copy_len;
    int len;

    *port = 0;

    /*
     * Skip over http:// or https://
     */

    uri_start = uri;
    if ((uri[0] == 'h' || uri[0] == 'H') && (uri[1] == 't' || uri[1] == 'T') && (uri[2] == 't' || uri[2] == 'T') && (uri[3] == 'p' || uri[3] == 'P'))
    {
        uri_start += 4;
        if (uri[4] == 's' || uri[4] == 'S')
        {
            uri_start++;
        }
        if (uri_start[0] != ':' || uri_start[1] != '/' || uri_start[2] != '/')
        {
            return WICED_BADARG;
        }
        uri_start += 3;
    }

    /*
     * Isolate the host part of the URI.
     */

    for (len = 0; uri_start[len] != ':' && uri_start[len] != '/' && uri_start[len] != '\0'; )
    {
        len++;
    }

    if (uri_start[len] == ':')
    {
        *port = atoi(&uri_start[len + 1]);
    }
    else
    {
        *port = AUDIO_CLIENT_DEFAULT_HTTP_PORT;
    }

    copy_len = len;
    if (copy_len > host_len - 2)
    {
        copy_len = host_len - 2;
    }
    memcpy(host, uri_start, copy_len);
    host[copy_len] = '\0';

    /*
     * Now pull out the path part.
     */

    uri_start += len;
    if (*uri_start == ':')
    {
        while (*uri_start != '/' && uri_start != '\0')
        {
            uri_start++;
        }
    }

    if (uri_start != '\0')
    {
        copy_len = strlen(uri_start);
        if (copy_len > path_len - 2)
        {
            copy_len = path_len - 2;
        }
        memcpy(path, uri_start, copy_len);
        path[copy_len] = '\0';
    }
    else
    {
        *path = '\0';
    }

    return WICED_SUCCESS;
}


static wiced_result_t audio_client_connect(audio_client_t* client)
{
    wiced_result_t      result;
    wiced_ip_address_t  ip_address;
    int                 connect_tries;

    result = audio_client_uri_split(client->pending_uri, client->hostname, AUDIO_CLIENT_HOSTNAME_LEN, client->path, AUDIO_CLIENT_PATH_LEN, &client->port);
    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Unable to parse URI %s\n", client->pending_uri);
        return WICED_ERROR;
    }

    wiced_log_msg(WICED_LOG_INFO, "Connect to host: %s, port %d, path %s\n", client->hostname, client->port, client->path);

    /*
     * Are we dealing with an IP address or a hostname?
     */

    if (str_to_ip(client->hostname, &ip_address) != 0)
    {
        /*
         * Try a hostname lookup.
         */

        wiced_log_msg(WICED_LOG_INFO, "Attemping DNS lookup for %s\n", client->hostname);
        result = dns_client_hostname_lookup(client->hostname, &ip_address, AUDIO_CLIENT_DNS_TIMEOUT_MS);
        if (result != WICED_SUCCESS)
        {
            wiced_log_msg(WICED_LOG_ERR, "DNS lookup failed (%d)\n", result);
            return result;
        }
    }

    wiced_log_msg(WICED_LOG_INFO, "Using IP address %d.%d.%d.%d\n", PRINT_IP(ip_address));

    /*
     * Open a socket for the connection.
     */

    if (client->socket_ptr == NULL)
    {
        result = wiced_tcp_create_socket(&client->socket, client->params.interface);
        if (result != WICED_SUCCESS)
        {
            wiced_log_msg(WICED_LOG_ERR, "Unable to create socket (%d)\n", result);
            return result;
        }
        client->socket_ptr = &client->socket;

        result = wiced_tcp_register_callbacks(client->socket_ptr, NULL, received_data_callback, NULL, client);
        if (result != WICED_SUCCESS)
        {
            wiced_log_msg(WICED_LOG_ERR, "Unable to register socket callbacks (%d)\n", result);
            return result;
        }
    }

    /*
     * Now try and connect.
     */

    connect_tries = 0;
    result = WICED_ERROR;
    while (connect_tries < 3 && result == WICED_ERROR)
    {
        wiced_log_msg(WICED_LOG_DEBUG0, "Connection attempt %d\n", connect_tries);
        result = wiced_tcp_connect(client->socket_ptr, &ip_address, client->port, AUDIO_CLIENT_CONNECT_TIMEOUT_MS);
        connect_tries++;;
    }

    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Connect to host %s:%d failed (%d)\n", client->hostname, client->port, result);
        wiced_tcp_delete_socket(client->socket_ptr);
        client->socket_ptr = NULL;
        return result;
    }

    return WICED_SUCCESS;
}


static wiced_result_t audio_client_parse_http_header(audio_client_t* client)
{
    char*           ptr;
    char*           line_end;
    uint16_t        http_code;
    wiced_result_t  result;

    result = audio_client_http_get_response_code(client->http_buf, client->http_buf_idx, &http_code);
    if (result != WICED_SUCCESS || (http_code != HTTP_STATUS_OK && http_code != HTTP_STATUS_PARTIAL_CONTENT))
    {
        wiced_log_msg(WICED_LOG_INFO, "Bad HTTP status: %03d\n", http_code);
        client->params.event_cb(client, client->params.userdata, AUDIO_CLIENT_EVENT_ERROR, (void*)AUDIO_CLIENT_ERROR_HTTP_GET_ERROR);
        return WICED_ERROR;
    }

    /*
     * Look through the HTTP headers for the Content-Length header. That tells us how
     * much data to expect.
     */

    ptr = client->http_buf;
    while (ptr != NULL && ptr < &client->http_buf[client->http_buf_idx])
    {
        line_end = strchr( (const char*)ptr, '\n');
        if (line_end == NULL)
        {
            break;
        }
        line_end[-1] = '\0';
        wiced_log_msg(WICED_LOG_DEBUG0, "%s\n", (char*)ptr);

        if (strncasecmp((const char *)ptr, HTTP_HEADER_CONTENT_LENGTH, sizeof(HTTP_HEADER_CONTENT_LENGTH) - 1) == 0)
        {
            ptr += sizeof(HTTP_HEADER_CONTENT_LENGTH) + 1;          /* Skip over the ': ' characters also */

            client->http_content_length = atol((char*)ptr);
            wiced_log_msg(WICED_LOG_INFO, "HTTP Content-Length: %ld\n", client->http_content_length);

            if ( client->seek_in_progress == WICED_FALSE )
            {
                client->http_total_content_length  = client->http_content_length;
                wiced_log_msg(WICED_LOG_INFO, "HTTP Total Content-Length: %ld\n", client->http_total_content_length );
            }
        }
        else if (strncasecmp((const char *)ptr, HTTP_HEADER_CONTENT_TYPE, sizeof(HTTP_HEADER_CONTENT_TYPE) - 1) == 0)
        {
            ptr += sizeof(HTTP_HEADER_CONTENT_TYPE) + 1;            /* Skip over the ': ' characters also */
            strlcpy(client->http_content_type, ptr, AUDIO_CLIENT_CONTENT_TYPE_SIZE);
            wiced_log_msg(WICED_LOG_INFO, "HTTP Content-Type: %s\n", client->http_content_type);
        }
        else if (strncasecmp((const char *)ptr, HTTP_HEADER_ACCEPT_RANGES, sizeof(HTTP_HEADER_ACCEPT_RANGES) - 1) == 0)
        {
            /*
             * Do not try range request only when Server explicitly declares Accept-Ranges: none
             * General DLNA servers support range request, and we can check res protocolInfo info of meta data
             */
            ptr += sizeof( HTTP_HEADER_ACCEPT_RANGES ) + 1; /* Skip over the ': ' characters also */
            if ( strcasecmp( (const char *) ptr, "none" ) == 0 )
            {
                wiced_log_msg( WICED_LOG_INFO, "HTTP server doesn't support range requests\n" );
                client->http_range_requests = WICED_FALSE;
            }
        }

        ptr = line_end + 1;
    }

    if (client->http_content_length == 0)
    {
        wiced_log_msg(WICED_LOG_ERR, "HTTP Content-Length header not found in response\n");
        return WICED_ERROR;
    }

    if (client->http_load_file)
    {
        /*
         * We need to allocate memory for loading the file.
         */

        client->http_file_data = calloc(1, client->http_content_length + 1);
        if (client->http_file_data == NULL)
        {
            wiced_log_msg(WICED_LOG_ERR, "Unable to allocate memory for loading HTTP file\n");
            return WICED_ERROR;
        }
        client->http_file_idx = 0;
    }

    /*
     * Tell the main thread that we have finished parsing the HTTP headers.
     */

    wiced_rtos_set_event_flags(&client->events, AUDIO_CLIENT_EVENT_HTTP_HEADER_COMPLETE);

    return WICED_SUCCESS;
}


static wiced_result_t find_http_header(audio_client_t* client, wiced_packet_t* packet)
{
    uint8_t*        data;
    uint16_t        avail_data_length;
    uint16_t        total_data_length;
    int             idx;
    wiced_result_t  result;

    /*
     * Get the pointer to the packet data.
     */

    result = wiced_packet_get_data(packet, 0, &data, &avail_data_length, &total_data_length);
    if (result != WICED_SUCCESS)
    {
        return WICED_ERROR;
    }

    idx = 0;
    if (!client->http_in_header)
    {
        /*
         * We're still searching for the beginning of the HTTP header.
         */

        while (idx < avail_data_length)
        {
            if (data[idx] == audio_client_HTTP_response[client->http_header_idx])
            {
                client->http_buf[client->http_buf_idx++] = data[idx];
                if (audio_client_HTTP_response[++client->http_header_idx] == '\0')
                {
                    /*
                     * Found the beginning of the header...woohoo!
                     */

                    wiced_log_msg(WICED_LOG_DEBUG0, "Found beginning of HTTP header\n");
                    client->http_in_header = WICED_TRUE;
                    idx++;
                    break;
                }
            }
            else
            {
                client->http_header_idx = 0;
                client->http_buf_idx    = 0;
            }
            idx++;
        }
    }

    if (!client->http_in_header)
    {
        /*
         * Haven't found the header yet.
         */

        return WICED_SUCCESS;
    }

    /*
     * Keep copying over the header data until we find the end.
     */

    while (idx < avail_data_length && client->http_buf_idx < AUDIO_CLIENT_HTTP_BUF_SIZE - 1)
    {
        client->http_buf[client->http_buf_idx++] = data[idx];
        if (data[idx] == audio_client_HTTP_body_separator[client->http_body_idx])
        {
            if (audio_client_HTTP_body_separator[++client->http_body_idx] == '\0')
            {
                wiced_log_msg(WICED_LOG_DEBUG0, "Found end of HTTP header\n");
                idx++;
                client->http_buf[client->http_buf_idx] = '\0';
                client->http_need_header = WICED_FALSE;
                client->http_in_header   = WICED_FALSE;
                break;
            }
        }
        else
        {
            client->http_body_idx = 0;
        }
        idx++;
    }

    /*
     * Update the beginning of the data in the packet to the beginning of the response body.
     */

    wiced_packet_set_data_start(packet, data + idx);
    if (client->http_buf_idx >= AUDIO_CLIENT_HTTP_BUF_SIZE - 1)
    {
        wiced_log_msg(WICED_LOG_ERR, "HTTP buffer too small for entire header\n");
        return WICED_ERROR;
    }

    if (client->http_need_header)
    {
        /*
         * We haven't found all of the header yet.
         */

        return WICED_SUCCESS;
    }

    /*
     * Now that we have the header, parse it.
     */

    result = audio_client_parse_http_header(client);

    if (result == WICED_SUCCESS)
    {
        /*
         * Reset some housekeeping variables.
         */

        client->http_content_read = 0;

        if (client->seek_in_progress)
        {
            client->seek_in_progress = WICED_FALSE;
            client->decoders[client->audio_codec].decoder_ioctl(client, DECODER_IOCTL_SET_POSITION, (void*)client->seek_position);
        }
    }

    return result;
}


static void send_empty_buffer_to_decoder(audio_client_t* client)
{
    data_buf_t* dbuf;

    if (client->http_load_file)
    {
        /*
         * We're loading a file instead of sending the data to the decoder.
         */

        return;
    }

    /*
     * Tell the decoder that we're all done sending data by sending a zero length buffer.
     */

    while (client->http_done == WICED_FALSE)
    {
        dbuf = &client->data_bufs[client->data_buf_widx];

        if (dbuf->inuse == 0)
        {
            dbuf->buflen = 0;
            dbuf->inuse  = 1;
            client->data_buf_widx = (client->data_buf_widx + 1) % client->params.data_buffer_num;
            wiced_rtos_set_event_flags(&client->decoder_events, DECODER_EVENT_AUDIO_DATA);
            break;
        }

        wiced_rtos_delay_milliseconds(1);
    }
}


static wiced_result_t audio_client_http_seek(audio_client_t* client)
{
    wiced_result_t result;

    if (client->seek_in_progress)
    {
        wiced_log_msg(WICED_LOG_ERR, "Seek already in progress\n");
        return WICED_ERROR;
    }

    wiced_log_msg(WICED_LOG_INFO, "Begin seek to position %lu\n", client->seek_position);

    /*
     * Disconnect the existing session to stop current http rx
     */
    if ( client->socket_ptr != NULL )
    {
        result = wiced_tcp_disconnect_with_timeout( client->socket_ptr, WICED_NO_WAIT );

        if ( result != WICED_TCPIP_SUCCESS )
        {
            client->params.event_cb( client, client->params.userdata, AUDIO_CLIENT_EVENT_ERROR, (void*) AUDIO_CLIENT_ERROR_SEEK_ERROR );
            client->http_error = WICED_TRUE;
            return WICED_ERROR;
        }
    }

    /*
     * Tell the decoder to flush any buffers it has waiting to be processed.
     */

    wiced_rtos_set_event_flags(&client->decoder_events, DECODER_EVENT_FLUSH);

    /*
     * Reconnect for range request
     */

    if ( audio_client_connect( client ) != WICED_SUCCESS )
    {
        client->params.event_cb( client, client->params.userdata, AUDIO_CLIENT_EVENT_ERROR, (void*) AUDIO_CLIENT_ERROR_CONNECT_FAILED );
        client->http_error = WICED_TRUE;
        return WICED_ERROR;
    }
    /*
     * Send off the GET range request to the HTTP server.
     */

    snprintf(client->http_buf, AUDIO_CLIENT_HTTP_BUF_SIZE, audio_client_HTTP_get_range_template, client->path, client->hostname, client->port, client->seek_position);
    wiced_log_msg(WICED_LOG_DEBUG0, "Sending request:\n%s\n", client->http_buf);

    result = wiced_tcp_send_buffer(client->socket_ptr, client->http_buf, (uint16_t)strlen(client->http_buf));
    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Unable to send HTTP request (%d)\n", result);
        client->params.event_cb(client, client->params.userdata, AUDIO_CLIENT_EVENT_ERROR, (void*)AUDIO_CLIENT_ERROR_HTTP_QUERY_FAILED);
        client->http_error = WICED_TRUE;
        return WICED_ERROR;
    }

    reset_initial_http_variables( client, WICED_TRUE );
    return WICED_SUCCESS;
}


static wiced_result_t process_data(audio_client_t* client, uint8_t* data, uint16_t data_length)
{
    data_buf_t* dbuf;

    if (data_length == 0)
    {
        /*
         * If just the HTTP response was sent in the first packet, it's possible that we
         * don't have any data to process.
         */

        return WICED_SUCCESS;
    }

    if (client->http_load_file)
    {
        memcpy(&client->http_file_data[client->http_file_idx], data, data_length);
        client->http_file_idx += data_length;
    }
    else
    {
        /*
         * Copy the packet data into a buffer to be processed.
         */

        while (client->http_done == WICED_FALSE)
        {
            dbuf = &client->data_bufs[client->data_buf_widx];

            if (dbuf->inuse)
            {
                wiced_log_msg(WICED_LOG_DEBUG4, "Receive buffers full (%d)\n", client->data_buf_widx);
                wiced_rtos_delay_milliseconds(4);
                continue;
            }

            memcpy(&dbuf->buf, data, data_length);
            dbuf->buflen  = data_length;
            dbuf->bufused = 0;
            dbuf->inuse   = 1;
            client->data_buf_widx = (client->data_buf_widx + 1) % client->params.data_buffer_num;

            /*
             * Let the decoder know that there is data available.
             * Queue up the first set of buffers before kicking the decoder to help deal with network lag.
             */

            if (client->params.data_buffer_preroll == 0 || client->initial_buffer_count > client->params.data_buffer_preroll)
            {
                wiced_rtos_set_event_flags(&client->decoder_events, DECODER_EVENT_AUDIO_DATA);
                if (client->http_first_kick)
                {
                    client->params.event_cb(client, client->params.userdata, AUDIO_CLIENT_EVENT_PLAYBACK_STARTED, NULL);
                    client->http_first_kick = WICED_FALSE;
                }

                /*
                 * Do we need to send out a high buffer threshold event?
                 */

                CHECK_FOR_THRESHOLD_HIGH_EVENT(client);
            }
            else
            {
                client->initial_buffer_count++;
            }
            break;
        }
    }

    return WICED_SUCCESS;
}


static void audio_client_http_process_socket(audio_client_t* client)
{
    wiced_packet_t* packet = NULL;
    uint8_t*        data;
    uint16_t        avail_data_length;
    uint16_t        total_data_length;
    uint32_t        events;
    wiced_result_t  result;

    while (!client->http_done)
    {
        if (client->params.high_threshold_read_inhibit && client->threshold_high_sent)
        {
            /*
             * No reading from the socket after the high threshold event is sent. This helps
             * the behavior when the app wants to put the WiFi to sleep between the high
             * and low threshold events.
             */

            wiced_log_msg(WICED_LOG_DEBUG1, "Skip read due to read inhibit setting\n");
            break;
        }

        result = wiced_rtos_wait_for_event_flags(&client->http_events, (HTTP_ALL_EVENTS & ~HTTP_EVENT_TCP_DATA), &events, WICED_FALSE, WAIT_FOR_ANY_EVENT, WICED_NO_WAIT);
        if (result == WICED_SUCCESS && events != 0)
        {
            /*
             * We have a pending event notification. Bail out of the socket reading loop.
             */

            break;
        }

        /*
         * Read from the socket.
         */

        result = wiced_tcp_receive(client->socket_ptr, &packet, AUDIO_CLIENT_RECEIVE_TIMEOUT_MS);
        if (result != WICED_SUCCESS)
        {
            if (result == WICED_TIMEOUT || client->http_done)
            {
                break;
            }

            wiced_log_msg(WICED_LOG_ERR, "Error returned from socket receive (%d)\n", result);
            client->http_error = WICED_TRUE;
            break;
        }

        if (client->http_need_header)
        {
            result = find_http_header(client, packet);
            if (result != WICED_SUCCESS)
            {
                client->http_error = WICED_TRUE;
                break;
            }

            if (client->http_need_header)
            {
                /*
                 * Still searching for the HTTP response header. Toss this packet and get the next one.
                 */

                wiced_packet_delete(packet);
                packet = NULL;
                continue;
            }
        }

        result = wiced_packet_get_data(packet, 0, &data, &avail_data_length, &total_data_length);
        if (result != WICED_SUCCESS)
        {
            wiced_log_msg(WICED_LOG_ERR, "Error getting packet data (%d)\n", result);
            client->http_error = WICED_TRUE;
            break;
        }

        if (avail_data_length > AUDIO_CLIENT_DATA_BUF_SIZE)
        {
            wiced_log_msg(WICED_LOG_ERR, "Receive buffers too small!!! %d (max %d)\n", avail_data_length, AUDIO_CLIENT_DATA_BUF_SIZE);
            client->http_error = WICED_TRUE;
            break;
        }
        client->http_content_read += avail_data_length;

        /*
         * Send the data onwards.
         */

        process_data(client, data, avail_data_length);

        /*
         * All done with the packet.
         */

        wiced_packet_delete(packet);
        packet = NULL;
    }

    if (packet != NULL)
    {
        wiced_packet_delete(packet);
        packet = NULL;
    }
}


static void audio_client_http_thread(uint32_t arg)
{
    audio_client_t* client = (audio_client_t*)arg;
    uint32_t        events;
    uint32_t        event;
    wiced_result_t  result;

    wiced_log_msg(WICED_LOG_NOTICE, "Audio client HTTP thread begin\n");

    /*
     * See if we can connect to the HTTP server.
     */

    if (audio_client_connect(client) != WICED_SUCCESS)
    {
        client->params.event_cb(client, client->params.userdata, AUDIO_CLIENT_EVENT_ERROR, (void*)AUDIO_CLIENT_ERROR_CONNECT_FAILED);
        client->http_error = WICED_TRUE;
        goto _err_out;
    }

    /*
     * Let the overlord know that we are connected.
     */

    client->params.event_cb(client, client->params.userdata, AUDIO_CLIENT_EVENT_CONNECTED, NULL);

    /*
     * Send off the GET request to the HTTP server.
     */

    snprintf(client->http_buf, AUDIO_CLIENT_HTTP_BUF_SIZE, audio_client_HTTP_get_request_template, client->path, client->hostname, client->port);
    wiced_log_msg(WICED_LOG_DEBUG0, "Sending query:\n%s\n", client->http_buf);

    result = wiced_tcp_send_buffer(client->socket_ptr, client->http_buf, (uint16_t)strlen(client->http_buf));
    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Unable to send HTTP query (%d)\n", result);
        client->params.event_cb(client, client->params.userdata, AUDIO_CLIENT_EVENT_ERROR, (void*)AUDIO_CLIENT_ERROR_HTTP_QUERY_FAILED);
        client->http_error = WICED_TRUE;
        goto _err_out;
    }

    client->http_first_kick = WICED_TRUE;

    while (!client->http_done && !client->http_error)
    {
        events = 0;

        result = wiced_rtos_wait_for_event_flags(&client->http_events, HTTP_ALL_EVENTS, &events, WICED_TRUE, WAIT_FOR_ANY_EVENT, WICED_WAIT_FOREVER);
        if (result != WICED_SUCCESS || client->http_done)
        {
            break;
        }

        if (events & HTTP_EVENT_SEEK)
        {
            audio_client_http_seek(client);
        }

        if (events & HTTP_EVENT_TCP_DATA)
        {
            audio_client_http_process_socket(client);

            /*
             * Have we read all the data?
             */

            if (client->http_content_length > 0 && client->http_content_read >= client->http_content_length)
            {
                wiced_log_msg(WICED_LOG_INFO, "All done reading HTTP data\n");
                break;
            }
        }
    }

    wiced_log_msg(WICED_LOG_NOTICE, "Audio client HTTP thread end\n");

_err_out:

    /*
     * Tell the decoder that we're all done sending data.
     */

    send_empty_buffer_to_decoder(client);

    if (client->socket_ptr != NULL)
    {
        result = wiced_tcp_disconnect_with_timeout(client->socket_ptr, WICED_NO_WAIT);
    }

    free(client->pending_uri);
    client->pending_uri = NULL;

    if (client->http_error)
    {
        event = AUDIO_CLIENT_EVENT_HTTP_ERROR;
    }
    else
    {
        event = AUDIO_CLIENT_EVENT_HTTP_THREAD_DONE;
    }
    wiced_rtos_set_event_flags(&client->events, event);

    WICED_END_OF_CURRENT_THREAD();
}

wiced_result_t audio_client_http_reader_stop(audio_client_t* client)
{
    if (client == NULL)
    {
        wiced_log_msg(WICED_LOG_ERR, "audio_client_http_reader_stop: Bad ARG\n");
        return WICED_BADARG;
    }

    if (client->http_thread_ptr != NULL)
    {
        wiced_log_msg(WICED_LOG_INFO, "audio_client_http_reader_stop\n");

        client->http_done = WICED_TRUE;
        wiced_rtos_thread_force_awake(&client->http_thread);
        wiced_rtos_thread_join(&client->http_thread);
        wiced_rtos_delete_thread(&client->http_thread);
        client->http_thread_ptr = NULL;
    }

    return WICED_SUCCESS;
}


wiced_result_t audio_client_http_reader_start(audio_client_t* client)
{
    wiced_result_t result;

    if (client->http_thread_ptr != NULL)
    {
        /*
         * We already have a reader thread running...that's bad.
         */

        wiced_log_msg(WICED_LOG_ERR, "Audio client HTTP thread already running\n");
        return WICED_ERROR;
    }

    /*
     * Spawn off the HTTP reader thread.
     */

    reset_initial_http_variables( client, WICED_FALSE );

    client->data_buf_ridx       = 0;
    client->data_buf_widx       = 0;
    client->threshold_high_sent = WICED_FALSE;
    client->threshold_low_sent  = WICED_FALSE;
    memset(client->data_bufs, 0, client->params.data_buffer_num * sizeof(data_buf_t));

    result = wiced_rtos_create_thread(&client->http_thread, AUDIO_CLIENT_HTTP_THREAD_PRIORITY, "Audio client HTTP thread",
                                      audio_client_http_thread, AUDIO_CLIENT_HTTP_THREAD_STACK_SIZE, client);
    if (result == WICED_SUCCESS)
    {
        client->http_thread_ptr = &client->http_thread;
    }
    else
    {
        wiced_log_msg(WICED_LOG_ERR, "Unable to create audio client HTTP thread\n");
    }

    return result;
}
