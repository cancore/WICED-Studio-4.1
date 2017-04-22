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

#ifdef __cplusplus
extern "C" {
#endif

#include "wiced.h"

#include "protocols/HTTP/http_stream.h"

#include "audio_render.h"

#include "audio_player_types.h"
#include "audio_player_dct.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define MILLISECONDS_PER_SECOND         (uint64_t)(1000)

#define SAMPLES_TO_MILLISECONDS(number_of_samples, sample_rate) ((MILLISECONDS_PER_SECOND * number_of_samples) / sample_rate)

/******************************************************
 *                    Constants
 ******************************************************/

#define PLAYER_TAG_VALID                0x51EDBA15
#define PLAYER_TAG_INVALID              0xDEADBEEF

#define AUDIO_PLAYER_AUDIO_BUFFER_SIZE              (4096)
#define AUDIO_PLAYER_AUDIO_BUFFER_NODES             80
#define AUDIO_PLAYER_AUDIO_BUFFER_MS                50
#define AUDIO_PLAYER_AUDIO_THRESH_MS                40
#define AUDIO_PLAYER_AUDIO_CLOCK_DISABLE             0
#define AUDIO_PLAYER_AUDIO_CLOCK_ENABLE              1

#define AUDIO_PLAYER_APP_URI_MAX                1024
#define AUDIO_PLAYER_HTTP_QUERY_SIZE            1024

/* name of file on server that has the audio play list */
#define AUDIO_PLAYER_PLAYLIST_NAME          "/audio_playlist.txt"
#define AUDIO_PLAYER_PLAYLIST_ENTRY_MAX     128

#define AUDIO_PLAYER_DBUF_NUM_BUFS                  (180)
#define AUDIO_PLAYER_DBUF_SIZE                      (1500)

/******************************************************
 *                   Enumerations
 ******************************************************/
typedef enum
{
    AUDIO_PLAYER_CODEC_FLAC = 0,
    AUDIO_PLAYER_CODEC_WAV,
//    AUDIO_PLAYER_CODEC_MP3,
//    AUDIO_PLAYER_CODEC_ALAW,
//    AUDIO_PLAYER_CODEC_ULAW,

    AUDIO_PLAYER_CODEC_MAX
} AUDIO_PLAYER_CODEC_FORMAT_T;


typedef enum
{
    PLAYER_EVENT_SHUTDOWN               = (1 <<  0),

    PLAYER_EVENT_CONNECT                = (1 <<  1),
    PLAYER_EVENT_DISCONNECT             = (1 <<  2),

    PLAYER_EVENT_PLAY                   = (1 <<  3),
    PLAYER_EVENT_SKIP                   = (1 <<  4),
    PLAYER_EVENT_STOP                   = (1 <<  5),
    PLAYER_EVENT_AUTOSTOP               = (1 <<  6),

    PLAYER_EVENT_LIST                   = (1 <<  7),
    PLAYER_EVENT_INFO                   = (1 <<  8),

    PLAYER_EVENT_RELOAD_DCT_WIFI        = (1 <<  9),
    PLAYER_EVENT_RELOAD_DCT_NETWORK     = (1 << 10),

    PLAYER_EVENT_HTTP_THREAD_DONE       = (1 << 20),
    PLAYER_EVENT_DECODER_THREAD_DONE    = (1 << 21),
    PLAYER_EVENT_AUDIO_RENDER_COMPLETE  = (1 << 22),

} PLAYER_EVENTS_T;

#define PLAYER_ALL_EVENTS       (-1)

typedef enum
{
    DECODER_EVENT_AUDIO_DATA        = (1 <<  0),
} DECODER_EVENTS_T;

#define DECODER_ALL_EVENTS       (-1)

typedef enum
{
    PLAYBACK_TYPE_NONE          = 0,
    PLAYBACK_TYPE_ALL           = (1 <<  1),    /* play all the files */
    PLAYBACK_TYPE_LOOP          = (1 <<  0),    /* loop one file - if "all", loop over all the files */

} PLAYBACK_TYPE_T;


/******************************************************
 *                 Type Definitions
 ******************************************************/

struct audio_player_s;

typedef wiced_result_t (*audio_player_decoder_start_t)(struct audio_player_s* player);
typedef wiced_result_t (*audio_player_decoder_stop_t)(struct audio_player_s* player);
typedef wiced_result_t (*audio_player_decoder_info_t)(struct audio_player_s* player, audio_player_source_info_t* info);

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    int         inuse;
    int         buflen;
    int         bufused;
    uint8_t     buf[AUDIO_PLAYER_DBUF_SIZE];
} data_buf_t;


#define MAX_HTTP_OBJECT_PATH        128
typedef struct audio_player_source_http_s
{
    char*                           uri;
    uint32_t                        size;
    http_stream_t                   http_stream;
} audio_player_source_http_t;


/* audio render buffers */
typedef struct audio_buff_list_t_s
{
    struct audio_buff_list_t_s*   next;
    uint8_t                     data_buf[AUDIO_PLAYER_AUDIO_BUFFER_SIZE];     /* raw LPCM data to send to renderer */
    uint32_t                    data_buf_size;
} audio_buff_list_t;

/* list of songs on server */
typedef struct audio_play_list_s
{
    uint8_t                     uri[AUDIO_PLAYER_PLAYLIST_ENTRY_MAX + 1];   /* path on server (after http://xxxxx/) */
} audio_play_list_t;

typedef struct
{
    audio_player_decoder_start_t    decoder_start;
    audio_player_decoder_stop_t     decoder_stop;
    audio_player_decoder_info_t     decoder_info;
} audio_player_decoder_api_t;

typedef struct audio_player_s
{
    uint32_t tag;

    wiced_bool_t                    skip_received;      /* skip current song, continue if all or loop */
    wiced_bool_t                    stop_received;      /* stop playback */
    wiced_bool_t                    report_render_complete;

    AUDIO_PLAYER_CODEC_FORMAT_T     audio_codec;        /* which codec are we playing? */

    wiced_event_flags_t             player_events;
    wiced_event_flags_t             decoder_events;

    platform_dct_network_config_t*  dct_network;
    platform_dct_wifi_config_t*     dct_wifi;
    audio_player_app_dct_t*         dct_app;

    wiced_thread_t                  http_thread;
    wiced_thread_t*                 http_thread_ptr;
    wiced_bool_t                    http_done;

    wiced_ip_address_t              ip_address;

    /* source info for http streaming */
    audio_player_source_http_t      http_source;
    char*                           uri_desc;
    char                            uri_to_stream[AUDIO_PLAYER_APP_URI_MAX];
    wiced_tcp_socket_t              tcp_socket;
    wiced_bool_t                    tcp_socket_created;

    char                            http_query[AUDIO_PLAYER_HTTP_QUERY_SIZE];       /* for building the http query */
    char                            server_uri[AUDIO_PLAYER_APP_URI_MAX];
    wiced_bool_t                    connect_state;      /* WICED_TRUE when connected to a server */
    char                            last_connected_host_name[MAX_HTTP_HOST_NAME_SIZE + 1];
    uint16_t                        last_connected_port;
    uint16_t                        server_playlist_count;
    audio_play_list_t*              server_playlist;

    PLAYBACK_TYPE_T                 playback_type;
    uint16_t                        current_play_list_index;

    /*
     * Received data buffers.
     */

    data_buf_t                      dbufs[AUDIO_PLAYER_DBUF_NUM_BUFS];
    int                             dbuf_write_idx;
    int                             dbuf_read_idx;

    /*
     * Decoder management.
     */

    audio_player_decoder_api_t      decoders[AUDIO_PLAYER_CODEC_MAX];
    wiced_thread_t                  decoder_thread;
    wiced_thread_t*                 decoder_thread_ptr;
    wiced_bool_t                    decoder_done;
    void*                           decoder_handle;

    /*
     * Audio LPCM Buffer management.
     */

    audio_render_ref                audio_render;
    wiced_mutex_t                   audio_buffer_mutex;
    uint16_t                        num_audio_buffer_nodes;
    uint16_t                        num_audio_buffer_used;
    audio_buff_list_t*              audio_buffer_list;
    audio_buff_list_t*              audio_buffer_free_list;

    /* debugging */
    wiced_time_t                    start_time;         /* when the app started */
    wiced_time_t                    play_start_time;    /* when the last play started */

} audio_player_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

extern char audio_player_get_request_template[];

/******************************************************
 *               Function Declarations
 ******************************************************/
wiced_result_t audio_player_uri_split(const char* uri, char* host_buff, uint16_t host_buff_len, char* path_buff, uint16_t path_buff_len, uint16_t* port);
wiced_result_t audio_player_connect(audio_player_t* player);
wiced_result_t audio_player_disconnect(audio_player_t* player);
wiced_result_t audio_player_audio_render_deinit(audio_player_t* player);

wiced_result_t audio_player_audio_render_buffer_push(audio_player_t* player, audio_decoder_buffer_info_t *buff_info);
wiced_result_t audio_player_audio_render_buffer_get(audio_player_t* player, audio_decoder_buffer_info_t *buff_info);

#ifdef __cplusplus
} /* extern "C" */
#endif
