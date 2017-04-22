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

/**
 * @file
 *
 * Audio Client library
 */

#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include "platform_audio.h"

/******************************************************
 *                     Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define AUDIO_CLIENT_EFFECT_MODE_NONE   (0)

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    AUDIO_CLIENT_ERROR_SUCCESS          = 0,
    AUDIO_CLIENT_ERROR_HTTP_INIT,
    AUDIO_CLIENT_ERROR_CONNECT_FAILED,
    AUDIO_CLIENT_ERROR_HTTP_QUERY_FAILED,
    AUDIO_CLIENT_ERROR_HTTP_GET_ERROR,
    AUDIO_CLIENT_ERROR_BAD_CODEC,
    AUDIO_CLIENT_ERROR_DECODER_ERROR,
    AUDIO_CLIENT_ERROR_SEEK_ERROR,

} AUDIO_CLIENT_ERROR_T;

typedef enum
{
    AUDIO_CLIENT_EVENT_ERROR = 0,           /* Arg is AUDIO_CLIENT_ERROR_T              */
    AUDIO_CLIENT_EVENT_CONNECTED,
    AUDIO_CLIENT_EVENT_AUDIO_FORMAT,        /* Arg is pointer to wiced_audio_config_t   */
    AUDIO_CLIENT_EVENT_PLAYBACK_STARTED,
    AUDIO_CLIENT_EVENT_PLAYBACK_STOPPED,
    AUDIO_CLIENT_EVENT_PLAYBACK_PAUSED,
    AUDIO_CLIENT_EVENT_PLAYBACK_EOS,
    AUDIO_CLIENT_EVENT_HTTP_COMPLETE,       /* Arg is pointer to allocated file data    */

    AUDIO_CLIENT_EVENT_DATA_THRESHOLD_HIGH,
    AUDIO_CLIENT_EVENT_DATA_THRESHOLD_LOW,

} AUDIO_CLIENT_EVENT_T;

typedef enum
{
    AUDIO_CLIENT_IOCTL_STOP = 0,        /* Arg is NULL                                  */
    AUDIO_CLIENT_IOCTL_PLAY,            /* Arg is pointer to URI string                 */
    AUDIO_CLIENT_IOCTL_PAUSE,           /* Arg is NULL                                  */
    AUDIO_CLIENT_IOCTL_RESUME,          /* Arg is NULL                                  */
    AUDIO_CLIENT_IOCTL_SET_VOLUME,      /* Arg is uint32_t - volume parameter           */
    AUDIO_CLIENT_IOCTL_TRACK_INFO,      /* Arg is pointer to audio_client_track_info_t  */
    AUDIO_CLIENT_IOCTL_SEEK,            /* Arg is seek time in milliseconds             */
    AUDIO_CLIENT_IOCTL_EFFECT,          /* Arg is uint32_t effect mode                  */

    AUDIO_CLIENT_IOCTL_LOAD_FILE,       /* Arg is pointer to URI string                 */

    AUDIO_CLIENT_IOCTL_MAX
} AUDIO_CLIENT_IOCTL_T;


/*
 * Audio client command notes:
 *
 * AUDIO_CLIENT_IOCTL_LOAD_FILE is used to download a text file from a web server. The downloaded
 * file is stored in a dynamically allocated buffer as a nul terminated string. The pointer to
 * the buffer is passed to the caller with the AUDIO_CLIENT_EVENT_HTTP_COMPLETE event. It is the
 * responsibility of the caller to free this buffer after use.
 */

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct audio_client_s *audio_client_ref;

#define AUDIO_CLIENT_BUF_FLAG_EOS       (1 << 0)

typedef struct
{
    uint8_t*        buf;                /* Pointer to the audio data buffer                         */
    uint32_t        buflen;             /* Length of audio data buffer                              */
    uint32_t        offset;             /* Offset in bytes to the start of valid data in the buffer */
    uint32_t        curlen;             /* Current length of valid data in the buffer               */
    uint32_t        flags;              /* Informational flags                                      */
    void*           opaque;             /* Opaque pointer - not used by audio client                */
} audio_client_buf_t;

typedef struct
{
    uint64_t        current_sample;     /* last played sample           */
    uint64_t        total_samples;      /* total samples (if known)     */
    uint32_t        sample_rate;        /* sample rate                  */
    uint8_t         channels;           /* number of channels           */
    uint8_t         bps;                /* bits per sample              */
    uint32_t        bitrate;            /* bitrate                      */
} audio_client_track_info_t;

typedef wiced_result_t (*audio_client_event_cb_t)(audio_client_ref handle, void* userdata, AUDIO_CLIENT_EVENT_T event, void* arg);
typedef wiced_result_t (*audio_client_buf_get_cb_t)(audio_client_ref handle, void* userdata, audio_client_buf_t* ac_buf, uint32_t timeout_ms);
typedef wiced_result_t (*audio_client_buf_release_cb_t)(audio_client_ref handle, void* userdata, audio_client_buf_t* ac_buf);

/******************************************************
 *                    Structures
 ******************************************************/

/**
 * Audio client parameters.
 *
 * Note: The audio client library operates in two main modes.
 *
 *  1) Audio client plays the audio stream.
 *      Audio client will allocate an internal buffer pool for processing the decoded audio.
 *      Decoded audio will be played using the audio render component.
 *
 *  2) Audio client does not play the audio stream.
 *      Audio client will download and decode the audio data into buffers that are provided
 *      by the application.
 */

typedef struct
{
    wiced_bool_t                    enable_playback;    /* Is audio client playing the audio?                       */
    audio_client_event_cb_t         event_cb;           /* Application event callback                               */
    void*                           userdata;           /* Opaque userdata passed back in event callback            */

    wiced_interface_t               interface;          /* Interface to use for HTTP socket                         */

    /*
     * HTTP Buffering parameters
     */

    uint16_t                        data_buffer_num;    /* Number of HTTP data buffers (1500 bytes) to allocate     */
    uint16_t                        data_buffer_preroll;/* Initial buffer count to queue before starting decoder           */
    uint16_t                        data_threshold_high;/* Trigger value for sending HTTP data buffer high threshold event */
    uint16_t                        data_threshold_low; /* Trigger value for sending HTTP data buffer low threshold event  */
    wiced_bool_t                    high_threshold_read_inhibit; /* Inhibit reading from the TCP socket after high threshold events */
                                                                 /* Reading will resume after low threshold event is triggered      */
    /*
     * Parameters that are only applicable if audio client is responsible for playing the audio.
     */

    platform_audio_device_id_t      device_id;          /* Audio device ID for audio playback                                */
    int                             volume;             /* Audio volume (0 - 100)                                            */
    uint16_t                        audio_buffer_num;   /* Number of decoded audio buffers to allocate                       */
    uint16_t                        audio_buffer_size;  /* Decoded audio buffer size                                         */
    uint16_t                        audio_period_size;  /* If non-zero, use in place of the default audio render period size */

    /*
     * Parameters that are only applicable if audio client is not playing the audio.
     */

    audio_client_buf_get_cb_t       buffer_get;         /* Application routine to get a buffer for decoded audio    */
    audio_client_buf_release_cb_t   buffer_release;     /* Application routine to release a buffer of decoded audio */

} audio_client_params_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/** Initialize the audio client library.
 *
 * @param[in] params      : Pointer to the configuration parameters.
 *
 * @return Pointer to the audio client instance or NULL
 */
audio_client_ref audio_client_init(audio_client_params_t* params);

/** Deinitialize the audio client library.
 *
 * @param[in] apollo_client : Pointer to the audio client instance.
 *
 * @return    Status of the operation.
 */
wiced_result_t audio_client_deinit(audio_client_ref audio_client);

/** Send an IOCTL to the audio client session.
 *
 * @param[in] audio_client  : Pointer to the audio client instance.
 * @param[in] cmd           : IOCTL command to process.
 * @param[inout] arg        : Pointer to argument for IOTCL.
 *
 * @return    Status of the operation.
 */
wiced_result_t audio_client_ioctl(audio_client_ref audio_client, AUDIO_CLIENT_IOCTL_T cmd, void* arg);


#ifdef __cplusplus
} /*extern "C" */
#endif
