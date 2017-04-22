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
 * audio player Application
 *
 * This application snippet demonstrates how to use the WICED
 * interface to play different audio codecs
 *
 * Application Instructions
 * 1. Modify the CLIENT_AP_SSID/CLIENT_AP_PASSPHRASE Wi-Fi credentials
 *    in the wifi_config_dct.h header file to match your Wi-Fi access point
 * 2. Connect a PC terminal to the serial port of the WICED Eval board,
 *    then build and download the application as described in the WICED
 *    Quick Start Guide
 *
 * Create / Know where Audio files can be found
 * 1. Use a server program like mini-httpd on Linux or mongoose on Win32
 * 2. Create a file called "audio_playlist.txt" on your server
 *    Each line in the file is a uri to an audio file on the same server
 *    Do not use spaces in the file names
 *    file example:
     "/wav/track_01.wav\n"
     "/wav/track_02.wav\n"
     "/wav/track_03.wav\n"
     "/flac/track_01.flac\n"
     "/flac/track_02.flac\n"
     "/flac/track_03.flac\n"
 *
 *
 * After the download completes, it connects to the Wi-Fi AP specified in apps/snip/audio_player/wifi_config_dct.h
 *      You can try to join the AP using the "join" console command:
 *  join <ssid> <open|wep|wpa_aes|wpa_tkip|wpa2|wpa2_tkip> [key] [ip netmask gateway]
 *      --Encapsulate SSID in quotes in order to include spaces
 *      - Join an AP. DHCP assumed if no IP address provided
 *
 * Issue these commands to play an audio file from your server:
 *
 * "connect <your server uri>"
 *          ex: "connect http://192.165.100.37"
 *          This will connect to your server and get the file "audio_playlist.txt"
 *          and print out the list to the console.
 *
 * "list"
 *          Print out the list to the console.
 *
 * "play x [loop]"
 *          ex: "play 3"
 *          This plays the 3rd file in the list from the server.
 *          Adding "loop" will loop this file after it is finished playing.
 *
 * "play all [loop]"
 *          This will play through all of the FLAC file sin the list from the server
 *          Adding "loop" will loop after it is finished playing the whole list.
 *
 */


#include "ctype.h"
#include "wiced.h"
#include "wiced_log.h"
#include "platform_audio.h"
#include "command_console.h"
#include "console_wl.h"
#include "wifi/command_console_wifi.h"
#include "dct/command_console_dct.h"
#include "protocols/DNS/dns.h"

#include "audio_render.h"

#include "audio_player_types.h"

#include "audio_player.h"
#include "audio_player_config.h"

#include "audio_player_http.h"
#include "audio_player_flac.h"
#include "audio_player_wav.h"
#include "audio_player_util.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define AUDIO_PLAYER_CONSOLE_COMMANDS \
    { (char*) "exit",           audio_player_console_command,    0, NULL, NULL, (char *)"", (char *)"Exit application" }, \
    { (char*) "connect",        audio_player_console_command,    0, NULL, NULL, (char *)"", (char *)"Connect to server" }, \
    { (char*) "disconnect",     audio_player_console_command,    0, NULL, NULL, (char *)"", (char *)"Disconnect from server" }, \
    { (char*) "play",           audio_player_console_command,    0, NULL, NULL, (char *)"", (char *)"Start playing from resource" }, \
    { (char*) "info",           audio_player_console_command,    0, NULL, NULL, (char *)"", (char *)"Info on current song" }, \
    { (char*) "list",           audio_player_console_command,    0, NULL, NULL, (char *)"", (char *)"List song URLs" }, \
    { (char*) "skip",           audio_player_console_command,    0, NULL, NULL, (char *)"", (char *)"Skip current song" }, \
    { (char*) "stop",           audio_player_console_command,    0, NULL, NULL, (char *)"", (char *)"Stop playing" }, \
    { (char*) "log",            audio_player_console_command,    0, NULL, NULL, (char *)"", (char *)"Set log level" }, \
    { (char*) "wiced_log",      audio_player_console_command,    0, NULL, NULL, (char *)"", (char *)"Set wiced log level" }, \
    { (char*) "config",         audio_player_console_command,    0, NULL, NULL, (char *)"", (char *)"Display / change config values" }, \


/******************************************************
 *                    Constants
 ******************************************************/

#define MAX_COMMAND_LENGTH              (85)
#define audio_player_console_command_HISTORY_LENGTH  (10)

#define BAR_GRAPH_LENGTH    62

#define MILLISECONDS_PER_SECOND         (uint64_t)(1000)
#define SECONDS_PER_MINUTE              (uint64_t)(60)

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    AUDIO_PLAYER_CONSOLE_CMD_EXIT = 0,

    AUDIO_PLAYER_CONSOLE_CMD_CONNECT,
    AUDIO_PLAYER_CONSOLE_CMD_DISCONNECT,

    AUDIO_PLAYER_CONSOLE_CMD_PLAY,
    AUDIO_PLAYER_CONSOLE_CMD_SKIP,
    AUDIO_PLAYER_CONSOLE_CMD_STOP,

    AUDIO_PLAYER_CONSOLE_CMD_LIST,
    AUDIO_PLAYER_CONSOLE_CMD_INFO,

    AUDIO_PLAYER_CONSOLE_CMD_CONFIG,

    AUDIO_PLAYER_CONSOLE_CMD_LOG_LEVEL,
    AUDIO_PLAYER_CONSOLE_CMD_WICED_LOG_LEVEL,

    AUDIO_PLAYER_CONSOLE_CMD_MAX,
} AUDIO_PLAYER_CONSOLE_CMDS_T;

#define NUM_NSECONDS_IN_SECOND                      (1000000000LL)
#define NUM_USECONDS_IN_SECOND                      (1000000)
#define NUM_NSECONDS_IN_MSECOND                     (1000000)
#define NUM_NSECONDS_IN_USECOND                     (1000)
#define NUM_USECONDS_IN_MSECOND                     (1000)
#define NUM_MSECONDS_IN_SECOND                      (1000)

/******************************************************
 *                 Type Definitions
 ******************************************************/


/******************************************************
 *                    Structures
 ******************************************************/

typedef struct cmd_lookup_s
{
    char *cmd;
    uint32_t event;
} cmd_lookup_t;

/******************************************************
 *               Function Declarations
 ******************************************************/

static int audio_player_console_command(int argc, char *argv[]);

/******************************************************
 *               Variables Definitions
 ******************************************************/

static char audio_player_command_buffer[MAX_COMMAND_LENGTH];
static char audio_player_command_history_buffer[MAX_COMMAND_LENGTH * audio_player_console_command_HISTORY_LENGTH];

const command_t audio_command_table[] =
{
    AUDIO_PLAYER_CONSOLE_COMMANDS
    WL_COMMANDS
    WIFI_COMMANDS
    DCT_CONSOLE_COMMANDS
    CMD_TABLE_END
};

static cmd_lookup_t command_lookup[AUDIO_PLAYER_CONSOLE_CMD_MAX] =
{
        { "exit",           PLAYER_EVENT_SHUTDOWN           },
        { "connect",        PLAYER_EVENT_CONNECT            },
        { "disconnect",     PLAYER_EVENT_DISCONNECT         },
        { "play",           PLAYER_EVENT_PLAY               },
        { "info",           PLAYER_EVENT_INFO               },
        { "list",           PLAYER_EVENT_LIST               },
        { "skip",           PLAYER_EVENT_SKIP               },
        { "stop",           PLAYER_EVENT_STOP               },
        { "config",         0                               },
        { "log",            0                               },
        { "wiced_log",      0                               },
};

audio_player_t *g_player;

/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

/****************************************************************
 *  Audio Render and Audio LPCM Buffer Function Declarations
 ****************************************************************/

static int render_log_output(WICED_LOG_LEVEL_T level, char *logmsg)
{
    UNUSED_PARAMETER(level);

    printf("%s", logmsg);
    return 0;
}

wiced_result_t audio_player_audio_buffer_list_init(audio_player_t* player)
{
    audio_buff_list_t*  buf_ptr;
    int                 i;

    /* sanity check */
    if (player == NULL)
    {
        return WICED_BADARG;
    }

    wiced_rtos_init_mutex( &player->audio_buffer_mutex );

    /* allocate a list & buffers to go with it */
    player->num_audio_buffer_nodes = AUDIO_PLAYER_AUDIO_BUFFER_NODES;
    player->num_audio_buffer_used  = 0;
    player->audio_buffer_list      = (audio_buff_list_t *)calloc_named("audio_buf_list", player->num_audio_buffer_nodes, sizeof(audio_buff_list_t));
    if (player->audio_buffer_list == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Unable to create audio buffer list\n");
        return WICED_OUT_OF_HEAP_SPACE;
    }

    wiced_rtos_lock_mutex( &player->audio_buffer_mutex );

    buf_ptr = player->audio_buffer_list;
    for (i = 0; i < player->num_audio_buffer_nodes - 1; ++i)
    {
        buf_ptr[i].next          = &buf_ptr[i + 1];
        buf_ptr[i].data_buf_size = AUDIO_PLAYER_AUDIO_BUFFER_SIZE;
    }
    /* set the data for the last one */
    buf_ptr[i].data_buf_size = AUDIO_PLAYER_AUDIO_BUFFER_SIZE;

    wiced_rtos_unlock_mutex( &player->audio_buffer_mutex );

    /* we keep the original buffer_list pointer to free it later, use the audio_buffer_free_list to manipulate */
    player->audio_buffer_free_list = player->audio_buffer_list;

    return WICED_SUCCESS;
}

wiced_result_t audio_player_audio_buffer_list_deinit(audio_player_t* player)
{
    if (player == NULL) return WICED_ERROR;

    player->audio_buffer_free_list = NULL;

    wiced_rtos_lock_mutex( &player->audio_buffer_mutex );

    if (player->audio_buffer_list != NULL)
    {
        player->num_audio_buffer_nodes = 0;
        player->num_audio_buffer_used  = 0;
        free(player->audio_buffer_list);
    }
    player->audio_buffer_list = NULL;

    wiced_rtos_deinit_mutex( &player->audio_buffer_mutex );

    return WICED_SUCCESS;
}

wiced_result_t audio_player_audio_render_buffer_release(audio_render_buf_t *rend_buf, void* userdata)
{
    audio_buff_list_t*  buff_ptr;
    audio_player_t*      player = (audio_player_t *)userdata;

    if (player == NULL || player->tag != PLAYER_TAG_VALID)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "buffer release with bad player handle %p\n", player);
        return WICED_ERROR;
    }

    /* add the buffer back to the list */

    buff_ptr = (audio_buff_list_t*)rend_buf->opaque;
    if (buff_ptr != NULL)
    {
        wiced_rtos_lock_mutex( &player->audio_buffer_mutex );

        buff_ptr->next = player->audio_buffer_free_list;
        player->audio_buffer_free_list = buff_ptr;
        player->num_audio_buffer_used--;

        wiced_rtos_unlock_mutex( &player->audio_buffer_mutex );
    }
    else
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_audio_render_buffer_release() Bad Ptr! %p! player->num_audio_buffer_used:%d\n", rend_buf, player->num_audio_buffer_used);
    }

    if (player->report_render_complete && player->num_audio_buffer_used <= 1)
    {
        wiced_rtos_set_event_flags(&player->player_events, PLAYER_EVENT_AUDIO_RENDER_COMPLETE);
        player->report_render_complete = WICED_FALSE;
    }

    return WICED_SUCCESS;
}

wiced_result_t audio_player_audio_render_buffer_push(audio_player_t* player, audio_decoder_buffer_info_t *buff_info)
{
    audio_render_buf_t ar_buf;

    /* sanity check */
    if ((player == NULL) || (player->tag != PLAYER_TAG_VALID) || (buff_info == NULL))
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_audio_render_buffer_push: BAD ARG! %p %p\n", player, buff_info);
        return WICED_BADARG;
    }

    /* play the sample */
    ar_buf.data_buf     = buff_info->data_buf;
    ar_buf.data_offset  = buff_info->filled_data_offset;
    ar_buf.data_length  = buff_info->filled_data_length;
    ar_buf.pts          = 0;
    ar_buf.opaque       = buff_info->opaque;

    if (audio_render_push(player->audio_render, &ar_buf) != WICED_SUCCESS)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "wiced_audio_render_push() failed!\n");
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

/* caller fill in bits per sample before calling! */
wiced_result_t audio_player_audio_render_buffer_get(audio_player_t* player, audio_decoder_buffer_info_t *buff_info)
{
    audio_buff_list_t*  buf_ptr = NULL;

    /* sanity check */
    if (player == NULL || player->tag != PLAYER_TAG_VALID || buff_info == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_get_free_audio_buffer: BAD args! %p %p\n", player, buff_info);
        return WICED_BADARG;
    }

    if (player->audio_buffer_free_list == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_DEBUG0, "audio_player_get_free_audio_buffer() No buffers!\n");
        return WICED_ERROR;
    }

    /*
     * Grab a node off of the free list.
     */

    wiced_rtos_lock_mutex( &player->audio_buffer_mutex );

    buf_ptr = player->audio_buffer_free_list;
    player->audio_buffer_free_list = player->audio_buffer_free_list->next;
    player->num_audio_buffer_used++;

    buf_ptr->next = NULL;

    wiced_rtos_unlock_mutex( &player->audio_buffer_mutex );

    /* fill in buff_info struct */
    memset(buff_info, 0, sizeof(audio_decoder_buffer_info_t));
    buff_info->data_buf      = buf_ptr->data_buf;
    buff_info->data_buf_size = buf_ptr->data_buf_size;
    /* so we can know which one it was later */
    buff_info->opaque        = buf_ptr;

    return WICED_SUCCESS;
}

wiced_result_t audio_player_audio_render_deinit(audio_player_t* player)
{
    if (player == NULL)
    {
        return WICED_ERROR;
    }

    if (player->audio_render != NULL)
    {
        audio_render_deinit(player->audio_render);
        player->audio_render = NULL;
    }

    return WICED_SUCCESS;
}


/****************************************************************
 *  Console command Function Declarations
 ****************************************************************/
static int audio_player_console_command(int argc, char *argv[])
{
    uint32_t event = 0;
    int i;

    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "Received command: %s\n", argv[0]);

    if (g_player == NULL || g_player->tag != PLAYER_TAG_VALID)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_console_command() Bad player structure\n");
        return ERR_CMD_OK;
    }

    /*
     * Lookup the command in our table.
     */

    for (i = 0; i < AUDIO_PLAYER_CONSOLE_CMD_MAX; ++i)
    {
        if (strcmp(command_lookup[i].cmd, argv[0]) == 0)
            break;
    }

    if (i >= AUDIO_PLAYER_CONSOLE_CMD_MAX)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Unrecognized command: %s\n", argv[0]);
        return ERR_CMD_OK;
    }

    switch (i)
    {
        case AUDIO_PLAYER_CONSOLE_CMD_PLAY:
            if (g_player->http_thread_ptr != NULL)
            {
                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Streaming session currently active\n");
                return ERR_CMD_OK;
            }
            if (argc < 2)
            {
                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Missing play argument\n");
                return ERR_CMD_OK;
            }

            if (g_player->server_playlist == NULL)
            {
                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Playlist has not been loaded\n");
                return ERR_CMD_OK;
            }

            g_player->playback_type = PLAYBACK_TYPE_NONE;    /* no special flags (all, loop)( */
            memset(g_player->uri_to_stream, 0, sizeof(g_player->uri_to_stream));
            if (strcasecmp(argv[1], "all") == 0)
            {
                printf("argv[1] = %s  list:%p count:%d\n", argv[1], g_player->server_playlist, g_player->server_playlist_count);
                if ( g_player->server_playlist != NULL )
                {
                    g_player->playback_type |= PLAYBACK_TYPE_ALL;
                    g_player->current_play_list_index = 0;
                    strncpy(g_player->uri_to_stream,
                            (char*)g_player->server_playlist[g_player->current_play_list_index].uri,
                            sizeof(g_player->uri_to_stream));
                }
            }
            else
            {
                /* play a single file here - either an index, or a specified file **/
                int16_t index = atoi(argv[1]);
                if ( (g_player->server_playlist != NULL) && (index >= 0) && (index < g_player->server_playlist_count) )
                {
                    strncpy(g_player->uri_to_stream, (char*)g_player->server_playlist[index].uri, sizeof(g_player->uri_to_stream));
                }
                else
                {
                    strncpy(g_player->uri_to_stream, argv[1], (sizeof(g_player->uri_to_stream) - 1) );
                    printf("g_player->uri_to_stream(user):%s\n", g_player->uri_to_stream);
                }
            }

            if (argc > 2 && strcasecmp(argv[2], "loop") == 0)
            {
                /* set loop mode */
                g_player->playback_type |= PLAYBACK_TYPE_LOOP;
            }

            event = command_lookup[i].event;
            break;

        case AUDIO_PLAYER_CONSOLE_CMD_CONNECT:
            if (g_player->connect_state != WICED_FALSE)
            {
                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Already connected\n");
                return ERR_CMD_OK;
            }

            g_player->playback_type = PLAYBACK_TYPE_NONE;
            memset(g_player->server_uri, 0, sizeof(g_player->server_uri));
            if (argc > 1)
            {
                strncpy(g_player->server_uri, argv[1], (sizeof(g_player->server_uri) - 1) );
                event = command_lookup[i].event;
            }
            break;

        case AUDIO_PLAYER_CONSOLE_CMD_DISCONNECT:
        case AUDIO_PLAYER_CONSOLE_CMD_EXIT:
        case AUDIO_PLAYER_CONSOLE_CMD_LIST:
        case AUDIO_PLAYER_CONSOLE_CMD_INFO:
        case AUDIO_PLAYER_CONSOLE_CMD_SKIP:
        case AUDIO_PLAYER_CONSOLE_CMD_STOP:
            event = command_lookup[i].event;
            break;

        case AUDIO_PLAYER_CONSOLE_CMD_CONFIG:
            audio_player_set_config(g_player, argc, argv);
            break;

        case AUDIO_PLAYER_CONSOLE_CMD_LOG_LEVEL:
            audio_player_log_set_level(atoi(argv[1]));
            break;

        case AUDIO_PLAYER_CONSOLE_CMD_WICED_LOG_LEVEL:
            wiced_log_set_level(atoi(argv[1]));
            break;
    }

    if (event)
    {
        /*
         * Send off the event to the main audio loop.
         */

        wiced_rtos_set_event_flags(&g_player->player_events, event);
    }

    return ERR_CMD_OK;
}

/****************************************************************
 *  HTTP URI connect / disconnect Function Declarations
 ****************************************************************/
wiced_result_t audio_player_uri_split(const char* uri, char* host_buff, uint16_t host_buff_len, char* path_buff, uint16_t path_buff_len, uint16_t* port)
{
   const char *uri_start, *host_start, *host_end;
   const char *path_start;
   uint16_t host_len, path_len;

  if ((uri == NULL) || (host_buff == NULL) || (path_buff == NULL) || (port == NULL))
  {
      return WICED_ERROR;
  }

  *port = 0;

  /* drop http:// or https://"  */
  uri_start = strstr(uri, "http");
  if (uri_start == NULL)
  {
      uri_start = uri;
  }
  if (strncasecmp(uri_start, "http://", 7) == 0)
  {
      uri_start += 7;
  }
  else if (strncasecmp(uri_start, "https://", 8) == 0)
  {
      uri_start += 8;
  }

  memset(host_buff, 0, host_buff_len);

  host_start = uri_start;
  host_len = strlen(host_start);
  host_end = strchr(host_start, ':');
  if (host_end != NULL)
  {
      *port = atoi(host_end + 1);
  }
  else
  {
      host_end = strchr(host_start, '/');
  }

  if (host_end != NULL)
  {
      host_len = host_end - host_start;
  }
  if( host_len > (host_buff_len - 1))
  {
      host_len = host_buff_len - 1;
  }
  memcpy(host_buff, host_start, host_len);

  memset(path_buff, 0, path_buff_len);
  path_start = strchr(host_start, '/');
  if( path_start != NULL)
  {
      path_len = strlen(path_start);
      if( path_len > (path_buff_len - 1))
      {
          path_len = path_buff_len - 1;
      }
      memcpy(path_buff, path_start, path_len);
  }

  return WICED_SUCCESS;
}


wiced_result_t audio_player_check_socket_created(audio_player_t* player)
{
    wiced_result_t result;
    if (player->tcp_socket_created == WICED_TRUE)
    {
        return WICED_TRUE;
    }

    result = wiced_tcp_create_socket( &player->tcp_socket, WICED_STA_INTERFACE );
    if ( result != WICED_SUCCESS )
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "wiced_tcp_create_socket() failed!\n");
        return result;
    }

    player->tcp_socket_created = WICED_TRUE;
    return result;
}


/* get the file list */
wiced_result_t get_server_file_list(audio_player_t* player)
{
    wiced_result_t result;
    wiced_packet_t*     reply_packet;

    sprintf(player->http_query, audio_player_get_request_template, AUDIO_PLAYER_PLAYLIST_NAME, player->last_connected_host_name, player->last_connected_port);
    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "Sending query for audio file list...\n");
    audio_player_log_msg(AUDIO_PLAYER_LOG_DEBUG0, "Sending query: [%s]\n", player->http_query);

    result = wiced_tcp_send_buffer( &player->tcp_socket, player->http_query, (uint16_t)strlen(player->http_query) );
    if ( result != WICED_SUCCESS )
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "get_server_file_list: wiced_tcp_send_buffer() failed!\n");
        return result;
    }

    /* get and parse audio list file */
    if (result == WICED_SUCCESS )
    {
        uint16_t            file_count, i, len;
        char*               data_end;
        char*               data_walker;    /* walk through the list of files */
        char*               line_end;
        audio_play_list_t*   new_playlist;

        uint8_t*        in_data;
        uint16_t        avail_data_length;
        uint16_t        total_data_length;
        uint16_t        wait_loop_count;
        uint32_t        data_received;

        uint8_t*        body;
        uint32_t        body_length;
        uint32_t        content_length;

        char*           incoming_data;
        uint32_t        incoming_data_length;

        http_header_t   headers[1];
        headers[0].name = "Content-Length";
        headers[0].value = NULL;

        content_length = 1; /* so we continue to loop until we have a real content length */
        data_received = 0;
        incoming_data = NULL;
        incoming_data_length = 0;
        wait_loop_count = 0;
        do
        {
            audio_player_log_msg(AUDIO_PLAYER_LOG_DEBUG0, "calling wiced_tcp_receive(%s -- %s, timeout:%d ms)\n", player->last_connected_host_name, AUDIO_PLAYER_PLAYLIST_NAME, 500);

            result = wiced_tcp_receive( &player->tcp_socket, &reply_packet, 500 ); /* short timeout */
            if (result == WICED_TIMEOUT)
            {
                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "get_server_file_list: wiced_tcp_receive() %d timeout - retry!\n", result);
                result = WICED_SUCCESS; /* so we stay in our loop */
                wait_loop_count++;
                continue;
            }

            if (reply_packet != NULL)
            {
                body = NULL;
                body_length = 0;

                /* does this have headers? what are they ? */
                result = http_extract_headers( reply_packet, headers, 1 );
                if (result == WICED_SUCCESS)
                {
                    content_length = atol(headers[0].value);
                }

                result = http_get_body( reply_packet, &body, &body_length );
                if ((result != WICED_SUCCESS) || (body == NULL))
                {
                    /* no body defined - just look for the data in the packet */
                    result = wiced_packet_get_data( reply_packet, 0, &in_data, &avail_data_length, &total_data_length);
                    if (result == WICED_SUCCESS)
                    {
                        if ( (data_received == 0) && (strncmp( (char *)in_data, "HTTP", 4) == 0))
                        {
                            /* this is just a header message, ignore it */
                            continue;
                        }
                        body = in_data;
                        body_length = avail_data_length;
                    }
                }

                /* if we got data, copy it over */
                if ((body != NULL) && (body_length > 0))
                {
                    if (incoming_data == NULL)
                    {
                        incoming_data = malloc(body_length + 2);
                        if (incoming_data != NULL)
                        {
                            memcpy(incoming_data, body, body_length);
                            incoming_data_length = body_length;
                            data_received = body_length;
                        }
                        else
                        {
                            audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "get_server_file_list: alloc() %d failed\n", result);
                            return WICED_ERROR;
                        }
                    }
                    else
                    {
                        incoming_data = realloc(incoming_data, incoming_data_length + body_length + 2);
                        if (incoming_data != NULL)
                        {
                            memcpy(&incoming_data[incoming_data_length], body, body_length);
                            incoming_data_length += body_length;
                            data_received += body_length;
                        }
                        else
                        {
                            audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "get_server_file_list: realloc() %d failed\n", result);
                            return WICED_ERROR;
                        }
                    }
                }

                /* free the packet */
                wiced_packet_delete( reply_packet );
            }
        } while ((result == WICED_SUCCESS)&& (wait_loop_count < 100) && (data_received < content_length));

        /* got it - parse it */

        if ( (incoming_data != NULL) && (incoming_data_length > 0) )
        {
            file_count = 0;
            data_walker = incoming_data;
            data_end = &incoming_data[incoming_data_length];
            /* count # files in the list */
            while( (data_walker != NULL) && (data_walker < data_end) && (*data_walker != '\0'))
            {
                line_end = strchr((char*)data_walker, '\r');
                if (line_end == NULL)
                {
                    line_end = strchr((char*)data_walker, '\n');
                }
                if (line_end == NULL)
                {
                    break;
                }

                file_count++;

                /* skip to next line */
                data_walker = line_end;
                while( (data_walker != NULL) && (*data_walker != '\0') &&
                       ( (*data_walker == '\r') || (*data_walker == '\n') ) )
                {
                    data_walker++;
                }
            }

            if (file_count <= 0)
            {
                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "get_server_file_list: No files in playlist\n");
                if (incoming_data != NULL)
                {
                    free(incoming_data);
                }
                return WICED_SUCCESS;
            }

            /* malloc play list */
            new_playlist = malloc(file_count * sizeof(audio_play_list_t));
            if (new_playlist == NULL)
            {
                return WICED_OUT_OF_HEAP_SPACE;
            }
            memset(new_playlist, 0x00, (file_count * sizeof(audio_play_list_t)));

            /* copy data from incoming data to play ;list */
            i = 0;
            data_walker = incoming_data;
            while( (data_walker != NULL) && (data_walker < data_end) && (*data_walker != '\0') && ( i < file_count))
            {
                line_end = strchr((char*)data_walker, '\r');
                if (line_end == NULL)
                {
                    line_end = strchr((char*)data_walker, '\n');
                }

                if ((line_end != NULL) && (line_end > data_walker && (line_end < data_end)))
                {
                    len = line_end - data_walker;
                    if( len > AUDIO_PLAYER_PLAYLIST_ENTRY_MAX)
                    {
                        len = AUDIO_PLAYER_PLAYLIST_ENTRY_MAX;
                    }
                    strncpy((char*)new_playlist[i].uri, data_walker, len);
                    i++;
                }
                /* move to next entry */
                data_walker = line_end;
                while( (data_walker != NULL) && (*data_walker != '\0') &&
                       ( (*data_walker == '\r') || (*data_walker == '\n') ) )
                {
                    data_walker++;
                }
            }

            /* delete the old list, if there is one */
            if (player->server_playlist != NULL)
            {
                free(player->server_playlist);
                player->server_playlist = NULL;
                player->server_playlist_count = 0;
            }
            player->server_playlist = new_playlist;
            player->server_playlist_count = file_count;

            if (file_count > 0)
            {
                result = WICED_SUCCESS;
            }
        }

        /* free our temp data */
        if (incoming_data != NULL)
        {
            free(incoming_data);
        }

    } /* if we were successful in sending the request */

    /* print out the list */
    if (player->server_playlist != NULL)
    {
        uint16_t idx;

        for (idx = 0; idx < player->server_playlist_count; idx++)
        {
            audio_player_printf("%d: %s\n", idx, player->server_playlist[idx].uri);
        }
    }

    return result;
}

wiced_result_t audio_player_connect(audio_player_t* player)
{
    wiced_result_t      result;
    wiced_ip_address_t  ip_address;
    uint16_t            port = 0;
    uint16_t            connect_tries;

    char                host_name[MAX_HTTP_HOST_NAME_SIZE];
    char                object_path[MAX_HTTP_OBJECT_PATH];

    audio_player_check_socket_created(player);

    result = audio_player_uri_split(player->server_uri, host_name, sizeof(host_name), object_path, sizeof(object_path), &port);
    if (result != WICED_SUCCESS)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "connect: audio_player_uri_split() failed %d\n", result);
        return result;
    }

    /* check that we have a port # */
    if (port == 0)
    {
        port = player->last_connected_port;
        if (port == 0)
        {
            port = HTTP_PORT_DEFAULT;
        }
    }

    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "Connect to: host:%s path:%s port:%d\n", host_name, object_path, port);

    if (isdigit((unsigned char)host_name[0]) && isdigit((unsigned char)host_name[1]) && isdigit((unsigned char)host_name[2])
            && host_name[3] == '.')
    {
        if (str_to_ip(host_name, &ip_address))
        {
            return WICED_ERROR;
        }

        audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "Using %ld.%ld.%ld.%ld\n",
                ((ip_address.ip.v4 >> 24) & 0xff), ((ip_address.ip.v4 >> 16) & 0x0ff),
                ((ip_address.ip.v4 >>  8) & 0xff), ((ip_address.ip.v4 >>  0) & 0x0ff));
    }
    else
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "connect: dns_client_hostname_lookup(%s)\n", host_name);
        result =  dns_client_hostname_lookup( host_name, &ip_address, 10000 );
        if (result!= WICED_SUCCESS)
        {
            audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "connect: dns_client_hostname_lookup(%s) failed\n", host_name);
            return result;
        }
    }

    connect_tries = 0;
    result = WICED_ERROR;
    while ((connect_tries < 3) && (result == WICED_ERROR))
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "Try %d Connecting to %s:%d  %ld.%ld.%ld.%ld\n",
                 connect_tries, host_name, port,
                ((ip_address.ip.v4 >> 24) & 0xff), ((ip_address.ip.v4 >> 16) & 0x0ff),
                ((ip_address.ip.v4 >>  8) & 0xff), ((ip_address.ip.v4 >>  0) & 0x0ff));
        result = wiced_tcp_connect(&player->tcp_socket, &ip_address, port, 2000);
        connect_tries++;;
    }

    if (result != WICED_SUCCESS)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "connect: wiced_tcp_connect() failed! %d (%d)\n", result, player->connect_state);
        return result;
    }
    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "Connected to %ld.%ld.%ld.%ld : %d\n",
            ((ip_address.ip.v4 >> 24) & 0xff), ((ip_address.ip.v4 >> 16) & 0x0ff),
            ((ip_address.ip.v4 >>  8) & 0xff), ((ip_address.ip.v4 >>  0) & 0x0ff), port);


    strcpy(player->last_connected_host_name, host_name);
    player->last_connected_port = port;
    player->connect_state = WICED_TRUE;

    return result;
}

wiced_result_t audio_player_disconnect(audio_player_t* player)
{
    wiced_result_t result = WICED_SUCCESS;

    player->connect_state = WICED_FALSE;

    if (player->tcp_socket_created == WICED_TRUE)
    {
        result = wiced_tcp_disconnect(&player->tcp_socket);
        if (result != WICED_SUCCESS)
        {
            audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Error returned form disconnect %d\n", result);
        }
    }

    return result;
}


static void audio_player_cleanup_playback(audio_player_t* player)
{
    audio_player_http_stream_stop(player);
    player->decoders[player->audio_codec].decoder_stop(player);
    audio_player_audio_render_deinit(player);
}


wiced_result_t audio_player_skip_playback(audio_player_t* player)
{

    audio_player_log_msg(AUDIO_PLAYER_LOG_DEBUG0, "audio_player_skip_playback()\n");

    if (player == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_skip_playback: Bad ARG\n");
        return WICED_BADARG;
    }

    player->skip_received = WICED_TRUE;
    audio_player_cleanup_playback(player);

    return WICED_SUCCESS;
}


wiced_result_t audio_player_stop_playback(audio_player_t* player)
{

    audio_player_log_msg(AUDIO_PLAYER_LOG_DEBUG0, "audio_player_stop_playback()\n");

    if (player == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_stop_playback: Bad ARG\n");
        return WICED_BADARG;
    }

    player->stop_received = WICED_TRUE;
    audio_player_cleanup_playback(player);

    return WICED_SUCCESS;
}

void audio_player_print_sys_info(audio_player_t* player)
{
    wiced_time_t curr_time;

    wiced_time_get_time(&curr_time);

    uint32_t    total_ms;
    uint32_t    min, sec, ms;

    audio_player_printf("\nAudio Player Application\n");

    audio_player_printf("   play type :: %s %s\n",
            ((player->playback_type & PLAYBACK_TYPE_ALL) != 0) ? "ALL" : "ONE",
            ((player->playback_type & PLAYBACK_TYPE_LOOP) != 0) ? "LOOPING" : "ONCE");

    total_ms = (uint32_t)(curr_time - player->start_time);
    ms = total_ms % (uint32_t)MILLISECONDS_PER_SECOND;
    sec = total_ms / (uint32_t)MILLISECONDS_PER_SECOND;
    min = sec / (uint32_t)SECONDS_PER_MINUTE;
    sec -= (min * (uint32_t)SECONDS_PER_MINUTE);
    audio_player_printf("    uptime   :: %ld:%02ld.%03ld\n", min, sec, ms);

    total_ms = (uint32_t)(curr_time - player->play_start_time);
    ms = total_ms % (uint32_t)MILLISECONDS_PER_SECOND;
    sec = total_ms / (uint32_t)MILLISECONDS_PER_SECOND;
    min = sec / (uint32_t)SECONDS_PER_MINUTE;
    sec -= (min * (uint32_t)SECONDS_PER_MINUTE);

    audio_player_printf("   play time :: %ld:%02ld.%03ld\n", min, sec, ms);
}

wiced_result_t audio_player_print_server_song_list(audio_player_t* player)
{
    int i;

    if (player == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_print_server_song_list: Bad ARG\n");
        return WICED_BADARG;
    }

    audio_player_print_sys_info(player);

    /* print out the songs list */
    if (player->server_playlist_count > 0)
    {
        audio_player_printf("\n");
        for (i = 0; i < player->server_playlist_count; i++)
        {
            audio_player_printf("%d %s\n", i, player->server_playlist[i].uri);
        }
    }

    return WICED_SUCCESS;
}

void audio_player_print_song_info(audio_player_t* player)
{
    audio_player_source_info_t source_info = {0};

    if (player == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_print_song_info: Bad ARG\n");
        return;
    }

    if (player->decoders[player->audio_codec].decoder_info != NULL)
    {
        player->decoders[player->audio_codec].decoder_info(player, &source_info);
    }

    audio_player_print_sys_info(player);

    {
        char            bar_graph[BAR_GRAPH_LENGTH] = {0};
        char            partial_time[16] = {0};
        char            total_time[16] = {0};
        uint16_t        i;
        uint64_t        offset, total;
        uint32_t        total_ms;
        uint32_t        min, sec, ms;
        wiced_bool_t    print_bar_graph = WICED_FALSE;
        wiced_bool_t    print_time = WICED_FALSE;

        if (source_info.source_total_samples != 0)
        {
            total = source_info.source_total_samples;
        }
        else
        {
            total = player->http_source.size;
        }
        if (total != 0)
        {
            offset = (source_info.source_current_sample * (BAR_GRAPH_LENGTH -2)) / total;
            print_bar_graph = WICED_TRUE;

            bar_graph[0] = '|';
            bar_graph[BAR_GRAPH_LENGTH - 2] = '|';
            for (i = 1; i < (BAR_GRAPH_LENGTH - 2); i++)
            {
                if (i == offset)
                {
                    bar_graph[i] = '|';
                }
                else
                {
                    bar_graph[i] = '-';
                }
            }
            bar_graph[BAR_GRAPH_LENGTH - 1] = '\0';

            total_ms = (uint32_t)((source_info.source_current_sample * (uint64_t)MILLISECONDS_PER_SECOND) /
                                  (uint64_t)source_info.source_sample_rate);
            ms = total_ms % (uint32_t)MILLISECONDS_PER_SECOND;
            sec = total_ms / (uint32_t)MILLISECONDS_PER_SECOND;
            min = sec / (uint32_t)SECONDS_PER_MINUTE;
            sec  -= (min * (uint32_t)SECONDS_PER_MINUTE);
            snprintf(partial_time, sizeof(partial_time), "%ld:%ld.%ld", min, sec, ms);

            total_ms = (uint32_t)((source_info.source_total_samples *(uint64_t)MILLISECONDS_PER_SECOND) /
                                  (uint64_t)source_info.source_sample_rate);
            ms = total_ms % (uint32_t)MILLISECONDS_PER_SECOND;
            sec = total_ms / (uint32_t)MILLISECONDS_PER_SECOND;
            min = sec / (uint32_t)SECONDS_PER_MINUTE;
            sec -= (min * (uint32_t)SECONDS_PER_MINUTE);
            snprintf(total_time, sizeof(partial_time), "%ld:%ld.%ld", min, sec, ms);
            print_time = WICED_TRUE;

        }

        /* print out the song info */
        audio_player_printf("\nplayback info for %s\n", player->uri_to_stream);
        if (print_bar_graph == WICED_TRUE)
        {
            audio_player_printf("%s\n", bar_graph);
        }
        if (print_time == WICED_TRUE)
        {
            audio_player_printf("            time: %s of %s\n", partial_time, total_time);
        }
        audio_player_printf("     last played: %9ld\n", (uint32_t)source_info.source_current_sample);
        audio_player_printf("   total samples: %9ld\n", (uint32_t)total);
        audio_player_printf("            rate: %ld\n", source_info.source_sample_rate);
        audio_player_printf("        channels: %d\n", source_info.source_channels);
        audio_player_printf(" bits per sample: %d\n", source_info.source_bps);
    }
}


static void audio_player_check_for_next_track(audio_player_t* player)
{
    /*
     * If we received a user stop than nothing to do.
     */

    if (player->stop_received == WICED_TRUE)
    {
        return;
    }

    /* if in "all" play mode and not at end of list, start next playback */
    if ((player->playback_type & PLAYBACK_TYPE_ALL) != 0)
    {
        player->current_play_list_index++;
        if (player->current_play_list_index < (player->server_playlist_count - 1))
        {
            strncpy(player->uri_to_stream,
                    (char*)player->server_playlist[player->current_play_list_index].uri,
                    sizeof(player->uri_to_stream));
            /* kick off another play */
            wiced_rtos_set_event_flags(&player->player_events, PLAYER_EVENT_PLAY);
        }
        else if ((player->playback_type & PLAYBACK_TYPE_LOOP) != 0)
        {
            player->current_play_list_index = 0;
            strncpy(player->uri_to_stream,
                    (char*)player->server_playlist[player->current_play_list_index].uri,
                    sizeof(player->uri_to_stream));
            /* kick off another play */
            wiced_rtos_set_event_flags(&player->player_events, PLAYER_EVENT_PLAY);
        }
    }
    else if ((player->playback_type & PLAYBACK_TYPE_LOOP) != 0)
    {
        /* kick off another play */
        wiced_rtos_set_event_flags(&player->player_events, PLAYER_EVENT_PLAY);
    }
}


static wiced_result_t audio_player_start_track(audio_player_t* player)
{
    audio_render_params_t params;

    if (player->http_thread_ptr != NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "HTTP thread already running\n");
        return WICED_ERROR;
    }

    if (player->decoder_thread_ptr != NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Decoder thread already running\n");
        return WICED_ERROR;
    }

    if (player->audio_render != NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Audio render already running\n");
        return WICED_ERROR;
    }

    if (strstr(player->uri_to_stream, ".flac") != NULL)
    {
        player->audio_codec = AUDIO_PLAYER_CODEC_FLAC;
    }
    else if (strstr(player->uri_to_stream, ".wav") != NULL)
    {
        player->audio_codec = AUDIO_PLAYER_CODEC_WAV;
    }
    else
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Unknown audio codec: %s\n", player->uri_to_stream);
        return WICED_ERROR;
    }

    /*
     * Make sure that we have a decoder configured.
     */

    if (player->decoders[player->audio_codec].decoder_start == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "No decoder configured for: %s\n", player->uri_to_stream);
        return WICED_ERROR;
    }

    wiced_time_get_time(&player->play_start_time);
    audio_player_print_sys_info(player);

    player->http_source.uri  = player->uri_to_stream;
    player->http_source.size = 0;               /* unknown */

    /*
     * Clear out our input buffers in case there's anything left over a previous playback session.
     */

    memset(player->dbufs, 0, sizeof(data_buf_t) * AUDIO_PLAYER_DBUF_NUM_BUFS);
    player->dbuf_write_idx = 0;
    player->dbuf_read_idx  = 0;

    /* START STREAMING DATA ! */

    audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Start streaming %s\n", player->http_source.uri);

    player->stop_received          = WICED_FALSE;
    player->skip_received          = WICED_FALSE;
    player->report_render_complete = WICED_FALSE;

    /*
     * Kick off the audio render component.
     */

    memset(&params, 0, sizeof(audio_render_params_t));
    params.buffer_nodes   = AUDIO_PLAYER_AUDIO_BUFFER_NODES;
    params.buffer_ms      = AUDIO_PLAYER_AUDIO_BUFFER_MS;
    params.threshold_ms   = AUDIO_PLAYER_AUDIO_THRESH_MS;
    params.clock_enable   = AUDIO_PLAYER_AUDIO_CLOCK_DISABLE;
    params.device_id      = player->dct_app->audio_device_tx;
    params.userdata       = player;
    params.buf_release_cb = audio_player_audio_render_buffer_release;

    player->audio_render  = audio_render_init(&params);
    if (player->audio_render == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Unable to create audio render\n");
        return WICED_ERROR;
    }

    /*
     * Now the decoder.
     */

    if (player->decoders[player->audio_codec].decoder_start(player) != WICED_SUCCESS)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Unable to initialize decoder\n");
        return WICED_ERROR;
    }

    /*
     * And the HTTP streamer.
     */

    if (audio_player_http_stream_start(player) != WICED_SUCCESS)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_http_stream_start: returned Error\n");
        player->decoders[player->audio_codec].decoder_stop(player);
    }

    return WICED_SUCCESS;
}


/****************************************************************
 *  Application Main loop Function
 ****************************************************************/

static void audio_player_mainloop(audio_player_t *player)
{
    wiced_result_t      result;
    uint32_t            events;

    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "Begin audio player mainloop\n");

    /*
     * If auto play is set then start off by sending ourselves a play event.
     */

    while (player->tag == PLAYER_TAG_VALID)
    {
        events = 0;

        result = wiced_rtos_wait_for_event_flags(&player->player_events, PLAYER_ALL_EVENTS, &events, WICED_TRUE, WAIT_FOR_ANY_EVENT, WICED_WAIT_FOREVER);
        if (result != WICED_SUCCESS)
        {
            continue;
        }

        if (events & PLAYER_EVENT_SHUTDOWN)
        {
            audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "mainloop received EVENT_SHUTDOWN\n");
            break;
        }

        if (events & PLAYER_EVENT_CONNECT)
        {
            if (player->connect_state != WICED_TRUE)
            {
                result = audio_player_connect(player);
                if (result != WICED_SUCCESS)
                {
                    audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "FAILED connect:%d %s\n", result, player->server_uri);
                }
                else
                {
                    result = get_server_file_list(player);
                    if (result != WICED_SUCCESS)
                    {
                        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "FAILED get_server_file_list:%d %s\n", result, player->server_uri);
                    }
                    result = audio_player_disconnect(player);
                    if (result != WICED_SUCCESS)
                    {
                        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "FAILED disconnect:%d %s\n", result, player->server_uri);
                    }
                }
            }
            else
            {
                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Already connected to %s\n", player->server_uri);
            }
        }

        if (events & PLAYER_EVENT_DISCONNECT)
        {
            if (player->connect_state == WICED_TRUE)
            {
                result = audio_player_disconnect(player);
                if (result != WICED_SUCCESS)
                {
                    audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "FAILED to disconnect: %s\n", player->server_uri);
                }
            }
            else
            {
                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Not connected\n");
            }
        }

        if (events & PLAYER_EVENT_SKIP)
        {
            audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "SKIP song\n");
            audio_player_print_sys_info(player);

            audio_player_skip_playback(player);
        }

        if (events & (PLAYER_EVENT_STOP | PLAYER_EVENT_AUTOSTOP))
        {
            audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "STOP\n");
            audio_player_print_sys_info(player);

            audio_player_stop_playback(player);
        }

        if (events & PLAYER_EVENT_HTTP_THREAD_DONE)
        {
            audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "mainloop received PLAYER_EVENT_HTTP_THREAD_DONE\n");

            audio_player_http_stream_stop(player);
        }

        if (events & PLAYER_EVENT_DECODER_THREAD_DONE)
        {
            player->report_render_complete = WICED_TRUE;
            if (player->num_audio_buffer_used <= 1)
            {
                /*
                 * We need to assume that the render has already finished.
                 */

                wiced_rtos_set_event_flags(&player->player_events, PLAYER_EVENT_AUDIO_RENDER_COMPLETE);
            }

            /*
             * Make sure that HTTP is stopped also. If the decoder shut down by itself (say due to a unsupported format),
             * HTTP may still be chugging along.
             */

            audio_player_http_stream_stop(player);
            if (player->decoders[player->audio_codec].decoder_stop)
            {
                player->decoders[player->audio_codec].decoder_stop(player);
            }

            audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "mainloop received PLAYER_EVENT_DECODER_THREAD_DONE\n");
        }

        if (events & PLAYER_EVENT_AUDIO_RENDER_COMPLETE)
        {
            audio_player_printf("Playback complete\n");

            audio_player_cleanup_playback(player);

            audio_player_check_for_next_track(player);
        }

        if (events & PLAYER_EVENT_PLAY)
        {
            audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "PLAY song\n");
            audio_player_start_track(player);
        }

        if (events & PLAYER_EVENT_LIST)
        {
            audio_player_print_server_song_list(player);
        }
        if (events & PLAYER_EVENT_INFO)
        {
            audio_player_print_song_info(player);
        }

        if (events & PLAYER_EVENT_RELOAD_DCT_WIFI)
        {
            audio_player_config_reload_dct_wifi(player);
        }
        if (events & PLAYER_EVENT_RELOAD_DCT_NETWORK)
        {
            audio_player_config_reload_dct_network(player);
        }

    }   /* while */

    /*
     * Make sure that playback has been shut down.
     */

    if (player->connect_state == WICED_TRUE)
    {
        result = audio_player_disconnect(player);
        if (result == WICED_SUCCESS)
        {
            player->connect_state = WICED_FALSE;
        }
        else
        {
            audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "FAILED to disconnect: %s\n", player->server_uri);
        }
    }

    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "End audio player mainloop\n");
}


static void shutdown_audio_player_app(audio_player_t *player)
{
    /*
     * Shutdown the console.
     */

    command_console_deinit();

    if (player->tcp_socket_created == WICED_TRUE)
    {
        wiced_tcp_delete_socket(&player->tcp_socket);
        player->tcp_socket_created = WICED_FALSE;
    }

    if (audio_player_audio_buffer_list_deinit(player) != WICED_SUCCESS)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_audio_buffer_list_deinit() failed\n");
    }

    wiced_rtos_deinit_event_flags(&player->player_events);
    wiced_rtos_deinit_event_flags(&player->decoder_events);

    audio_player_config_deinit(player);

    player->tag = PLAYER_TAG_INVALID;
    free(player);
}


static void audio_player_console_dct_callback(console_dct_struct_type_t struct_changed, void* app_data)
{
    audio_player_t*      player;

    /* sanity check */
    if (app_data == NULL)
    {
        return;
    }

    player = (audio_player_t*)app_data;
    switch(struct_changed)
    {
        case CONSOLE_DCT_STRUCT_TYPE_WIFI:
            /* Get WiFi configuration */
            wiced_rtos_set_event_flags(&player->player_events, PLAYER_EVENT_RELOAD_DCT_WIFI);
            break;
        case CONSOLE_DCT_STRUCT_TYPE_NETWORK:
            /* Get network configuration */
            wiced_rtos_set_event_flags(&player->player_events, PLAYER_EVENT_RELOAD_DCT_NETWORK);
            break;
        default:
            break;
    }
}

static audio_player_t *init_player(void)
{
    audio_player_t*         player;
    wiced_result_t          result;
    uint32_t                tag;

    tag = PLAYER_TAG_VALID;

    /* Initialize the device */
    result = wiced_init();
    if (result != WICED_SUCCESS)
    {
        return NULL;
    }

    /*
     * Initialize our logging subsystems.
     */

    audio_player_log_init(AUDIO_PLAYER_LOG_ERR);
    wiced_log_init(WICED_LOG_ERR, render_log_output, NULL);    /* For audio render logging messages */

    /* initialize audio */
    platform_init_audio();

    /*
     * Allocate the main player structure.
     */
    player = calloc_named("audio_player", 1, sizeof(audio_player_t));
    if (player == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Unable to allocate player structure\n");
        return NULL;
    }

    /*
     * Create the command console.
     */

    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "Start the command console\n");
    result = command_console_init(STDIO_UART, sizeof(audio_player_command_buffer), audio_player_command_buffer, audio_player_console_command_HISTORY_LENGTH, audio_player_command_history_buffer, " ");
    if (result != WICED_SUCCESS)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Error starting the command console\n");
        free(player);
        return NULL;
    }
    console_add_cmd_table(audio_command_table);
    console_dct_register_callback(audio_player_console_dct_callback, player);


    /*
     * Create our event flags.
     */

    result = wiced_rtos_init_event_flags(&player->player_events);
    if (result != WICED_SUCCESS)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Error initializing player event flags\n");
        tag = PLAYER_TAG_INVALID;
    }

    result = wiced_rtos_init_event_flags(&player->decoder_events);
    if (result != WICED_SUCCESS)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Error initializing decoder event flags\n");
        tag = PLAYER_TAG_INVALID;
    }

    if (audio_player_audio_buffer_list_init(player) != WICED_SUCCESS)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_audio_buffer_list_init() failed\n");
    }

    /* read in our configurations */
    /* read in our configurations */
    audio_player_config_init(player);

    /* print out our current configuration */
    audio_player_config_print_info(player);

    /* Bring up the network interface */
    result = wiced_network_up_default(&player->dct_network->interface, NULL);
    if (result != WICED_SUCCESS)
    {
        /*
         * The network didn't initialize but we don't want to consider that a fatal error.
         */

        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Bringing up network interface failed\n");
    }
    else
    {
        /* create a socket */
        audio_player_check_socket_created(player);
    }

    /*
     * Set up the decoder functions.
     */

    player->decoders[AUDIO_PLAYER_CODEC_FLAC].decoder_start = audio_player_flac_decoder_start;
    player->decoders[AUDIO_PLAYER_CODEC_FLAC].decoder_stop  = audio_player_flac_decoder_stop;
    player->decoders[AUDIO_PLAYER_CODEC_FLAC].decoder_info  = audio_player_flac_decoder_info;
    player->decoders[AUDIO_PLAYER_CODEC_WAV].decoder_start  = audio_player_wav_decoder_start;
    player->decoders[AUDIO_PLAYER_CODEC_WAV].decoder_stop   = audio_player_wav_decoder_stop;
    player->decoders[AUDIO_PLAYER_CODEC_WAV].decoder_info   = audio_player_wav_decoder_info;

    /* set our valid tag */
    player->tag = tag;

    return player;
}


void application_start(void)
{
    audio_player_t *player;

    /*
     * Main initialization.
     */

    if ((player = init_player()) == NULL)
    {
        return;
    }
    g_player = player;

    wiced_time_get_time(&player->start_time);

    /*
     * Drop into our main loop.
     */
    audio_player_mainloop(player);

    /*
     * Cleanup and exit.
     */
    wiced_log_shutdown();

    g_player = NULL;
    shutdown_audio_player_app(player);
    player = NULL;
}
