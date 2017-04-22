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
 * Audio client library
 *
 */

#include "wiced_result.h"
#include "wiced_platform.h"
#include "platform_audio.h"
#include "wiced_log.h"

#include "audio_client.h"
#include "audio_client_private.h"
#include "audio_client_flac.h"
#include "audio_client_wav.h"
#include "audio_client_aac.h"
#include "audio_client_mp3.h"


/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/*
 * Audio render uses a small period buffer size by default
 * for best low latency performance. HTTP streaming can use
 * a larger size.
 */

#define AUDIO_CLIENT_PERIOD_SIZE            (512)

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct
{
    char* name;
    uint32_t value;
} lookup_t;


typedef struct
{
    int buflen;
    int maxlen;
    objhandle_t buf_handle;
    uint8_t buf[0];
} audio_buf_t;

/******************************************************
 *                    Structures
 ******************************************************/

static lookup_t codec_types[] =
{
    { ".flac",          AUDIO_CLIENT_CODEC_FLAC     },
    { ".wav",           AUDIO_CLIENT_CODEC_WAV      },
    { ".m4a",           AUDIO_CLIENT_CODEC_AAC      },
    { ".aac",           AUDIO_CLIENT_CODEC_AAC      },
    { ".mp3",           AUDIO_CLIENT_CODEC_MP3      },
    { NULL,             0                           }       /* Must be last! */
};

static lookup_t mime_types[] =
{
    { "audio/wav",      AUDIO_CLIENT_CODEC_WAV      },
    { "audio/x-wav",    AUDIO_CLIENT_CODEC_WAV      },
    { "audio/flac",     AUDIO_CLIENT_CODEC_FLAC     },
    { "audio/m4a",      AUDIO_CLIENT_CODEC_AAC      },
    { "audio/x-m4a",    AUDIO_CLIENT_CODEC_AAC      },
    { "audio/aac",      AUDIO_CLIENT_CODEC_AAC      },
    { "audio/x-aac",    AUDIO_CLIENT_CODEC_AAC      },
    { "audio/mpeg",     AUDIO_CLIENT_CODEC_MP3      },
    { "audio/mp3",      AUDIO_CLIENT_CODEC_MP3      },
    { NULL,             0                           }       /* Must be last! */
};

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

static wiced_result_t audio_client_null_decoder_start(audio_client_t* client)
{
    (void)client;
    return WICED_SUCCESS;
}


static wiced_result_t audio_client_null_decoder_stop(audio_client_t* client)
{
    (void)client;
    return WICED_SUCCESS;
}


static wiced_result_t audio_client_null_decoder_ioctl(struct audio_client_s* client, DECODER_IOCTL_T ioctl, void* arg)
{
    (void)client;
    (void)ioctl;
    (void)arg;

    return WICED_UNSUPPORTED;
}


static wiced_result_t audio_render_buffer_release(audio_render_buf_t* buf, void* userdata)
{
    audio_client_t* client = (audio_client_t*)userdata;

    /*
     * Ignore the tag valid/invalid flag since the flag gets set to invalid when we are shutting down.
     */

    if (client == NULL || buf == NULL)
    {
        return WICED_ERROR;
    }

    if (client->buf_pool && buf->opaque)
    {
        bufmgr_pool_free_buf(client->buf_pool, (objhandle_t)buf->opaque);
        buf->opaque = NULL;
    }

    return WICED_SUCCESS;
}


static wiced_result_t audio_render_event(audio_render_ref handle, void* userdata, AUDIO_RENDER_EVENT_T event, void* arg)
{
    audio_client_t* client = (audio_client_t*)userdata;

    if (client == NULL || client->tag != AUDIO_CLIENT_TAG_VALID)
    {
        return WICED_ERROR;
    }

    wiced_log_msg(WICED_LOG_DEBUG0, "Audio render event received: %d\n", event);

    switch (event)
    {
        case AUDIO_RENDER_EVENT_EOS:
            wiced_rtos_set_event_flags(&client->events, AUDIO_CLIENT_EVENT_PLAYBACK_COMPLETE);
            break;

        default:
            wiced_log_msg(WICED_LOG_ERR, "Unrecognized audio render event: %d\n", event);
            break;
    }

    return WICED_SUCCESS;
}


static wiced_result_t audio_client_effect(audio_client_t* client)
{
    wiced_result_t result;

    if (client->state != AUDIO_CLIENT_STATE_IDLE && client->audio)
    {
        /*
         * Tell the audio render to set effect mode.
         */

        result = audio_render_command(client->audio, AUDIO_RENDER_CMD_SET_EFFECT, (void*)client->effect_mode);
        if (result == WICED_SUCCESS)
        {
            wiced_log_msg(WICED_LOG_INFO, "Audio effect mode: %d\n", client->effect_mode);
        }
        else
        {
            wiced_log_msg(WICED_LOG_ERR, "Unable to set audio effect mode [mode: %d][error: %d]\n", client->effect_mode, result);
        }
    }

    return WICED_SUCCESS;
}


wiced_result_t audio_client_audio_config(audio_client_t* client, wiced_audio_config_t* config)
{
    wiced_result_t result;

    if (client == NULL || client->tag != AUDIO_CLIENT_TAG_VALID || config == NULL)
    {
        return WICED_ERROR;
    }

    if (client->audio)
    {
        result = audio_render_configure(client->audio, config);
        if (result == WICED_SUCCESS)
        {
            client->audio_configured = WICED_TRUE;

            if(client->effect_mode != AUDIO_CLIENT_EFFECT_MODE_NONE)
            {
                audio_client_effect(client);
            }
        }
        else
        {
            client->audio_configured = WICED_FALSE;
        }
    }
    else
    {
        result = client->params.event_cb(client, client->params.userdata, AUDIO_CLIENT_EVENT_AUDIO_FORMAT, config);
    }

    return result;
}


wiced_result_t audio_client_buffer_get(audio_client_t* client, audio_client_buf_t* buf, uint32_t timeout_ms)
{
    objhandle_t buf_handle;
    audio_buf_t* audio_buf;
    uint32_t buf_size;
    wiced_result_t result = WICED_SUCCESS;

    if (client == NULL || client->tag != AUDIO_CLIENT_TAG_VALID)
    {
        return WICED_ERROR;
    }

    if (client->audio)
    {
        if (bufmgr_pool_alloc_buf(client->buf_pool, &buf_handle, (char **)&audio_buf, &buf_size, timeout_ms) != BUFMGR_OK)
        {
            wiced_log_msg(WICED_LOG_DEBUG2, "Unable to allocate audio packet\n");
            return WICED_ERROR;
        }

        buf->opaque = buf_handle;
        buf->buf    = audio_buf->buf;
        buf->buflen = client->params.audio_buffer_size;
        buf->curlen = 0;
        buf->offset = 0;
        buf->flags  = 0;
    }
    else
    {
        result = client->params.buffer_get(client, client->params.userdata, buf, AUDIO_CLIENT_AUDIO_BUF_TIMEOUT);
    }

    return result;
}


wiced_result_t audio_client_buffer_release(audio_client_t* client, audio_client_buf_t* buf)
{
    audio_render_buf_t render_buf;
    wiced_result_t result = WICED_SUCCESS;

    if (client == NULL)
    {
        return WICED_ERROR;
    }

    if (client->audio)
    {
        /*
         * Pass this buffer on to audio render.
         */

        if (buf->buf && buf->curlen > 0)
        {
            render_buf.pts         = 0;
            render_buf.data_buf    = buf->buf;
            render_buf.data_offset = buf->offset;
            render_buf.data_length = buf->curlen;
            render_buf.opaque      = buf->opaque;

            result = audio_render_push(client->audio, &render_buf);

            if (result != WICED_SUCCESS)
            {
                /*
                 * Audio render didn't accept the buffer, make sure that we don't leak it.
                 */

                if (client->buf_pool && buf->opaque)
                {
                    bufmgr_pool_free_buf(client->buf_pool, (objhandle_t)buf->opaque);
                    buf->opaque = NULL;
                }
            }
        }

        if (buf->flags & AUDIO_CLIENT_BUF_FLAG_EOS)
        {
            /*
             * Tell audio render that there's no more audio coming for this stream.
             */

            result = audio_render_push(client->audio, NULL);
        }
    }
    else
    {
        result = client->params.buffer_release(client, client->params.userdata, buf);
    }

    return result;
}


static wiced_result_t audio_client_seek(audio_client_t* client)
{
    wiced_result_t result = WICED_UNSUPPORTED;
    uint32_t arg;

    /*
     * Ask the decoder where in the stream we need to seek.
     */

    arg = client->seek_request_ms;
    result = client->decoders[client->audio_codec].decoder_ioctl(client, DECODER_IOCTL_GET_SEEK_POSITION, &arg);

    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_INFO, "Seek not supported for current codec\n");
        client->params.event_cb( client, client->params.userdata, AUDIO_CLIENT_EVENT_ERROR, (void*) AUDIO_CLIENT_ERROR_SEEK_ERROR );
        return WICED_SUCCESS;
    }

    client->seek_position = arg;
    wiced_rtos_set_event_flags(&client->http_events, HTTP_EVENT_SEEK);

    return WICED_SUCCESS;
}


static wiced_result_t audio_client_pause(audio_client_t* client)
{
    if (client->state == AUDIO_CLIENT_STATE_PLAY)
    {
        /*
         * Tell the audio render to pause playback.
         */
        if (client->audio)
        {
            audio_render_command(client->audio, AUDIO_RENDER_CMD_PAUSE, NULL);
        }
        client->state = AUDIO_CLIENT_STATE_PAUSE;

        client->params.event_cb(client, client->params.userdata, AUDIO_CLIENT_EVENT_PLAYBACK_PAUSED, NULL);
    }

    return WICED_SUCCESS;
}


static wiced_result_t audio_client_resume(audio_client_t* client)
{
    if (client->state == AUDIO_CLIENT_STATE_PAUSE)
    {
        /*
         * Tell the audio render to pause playback.
         */

        if (client->audio)
        {
            audio_render_command(client->audio, AUDIO_RENDER_CMD_RESUME, NULL);
        }
        client->state = AUDIO_CLIENT_STATE_PLAY;

        client->params.event_cb(client, client->params.userdata, AUDIO_CLIENT_EVENT_PLAYBACK_STARTED, NULL);
    }

    return WICED_SUCCESS;
}


static wiced_result_t audio_client_stop(audio_client_t* client)
{
    /*
     * Shutdown the HTTP reader thread.
     */

    audio_client_http_reader_stop(client);

    /*
     * The decoder thread.
     */

    client->decoders[client->audio_codec].decoder_stop(client);
    client->audio_codec = AUDIO_CLIENT_CODEC_NULL;

    /*
     * And stop the audio render playback.
     */

    if (client->audio)
    {
        audio_render_command(client->audio, AUDIO_RENDER_CMD_STOP, NULL);
    }

    /*
     * Do we need to tell the app?
     */

    if (client->state == AUDIO_CLIENT_STATE_PLAY || client->state == AUDIO_CLIENT_STATE_PAUSE)
    {
        client->state = AUDIO_CLIENT_STATE_IDLE;
        if (!client->http_load_file)
        {
            client->params.event_cb(client, client->params.userdata, AUDIO_CLIENT_EVENT_PLAYBACK_STOPPED, NULL);
        }
    }

    return WICED_SUCCESS;
}


static wiced_result_t audio_client_start_decoder(audio_client_t* client, AUDIO_CLIENT_CODEC_T codec, AUDIO_CLIENT_ERROR_T *err)
{
    /*
     * Make sure that we have a decoder configured.
     */

    if (client->decoders[codec].decoder_start == NULL)
    {
        wiced_log_msg(WICED_LOG_ERR, "No decoder configured for %d\n", codec);
        *err = AUDIO_CLIENT_ERROR_BAD_CODEC;
        return WICED_ERROR;
    }

    /*
     * And fire off the decoder.
     */

    if (client->decoders[codec].decoder_start(client) != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Unable to initialize decoder\n");
        *err = AUDIO_CLIENT_ERROR_DECODER_ERROR;
        return WICED_ERROR;
    }

    /*
     * Save the active codec.
     */

    client->audio_codec = codec;

    return WICED_SUCCESS;
}


static wiced_result_t audio_client_play(audio_client_t* client, AUDIO_CLIENT_ERROR_T *err)
{
    AUDIO_CLIENT_CODEC_T codec;
    wiced_result_t result;
    int i;

    *err = AUDIO_CLIENT_ERROR_SUCCESS;

    /*
     * Make sure the audio configured flag is reset before starting a new stream.
     */

    client->audio_configured = WICED_FALSE;

    if (client->http_load_file == WICED_FALSE)
    {
        /*
         * Is this a format we like & understand?
         */

        for (i = 0; codec_types[i].name != NULL; i++)
        {
            if (strstr(client->pending_uri, codec_types[i].name) != NULL)
            {
                break;
            }
        }

        if (codec_types[i].name != NULL)
        {
            codec = codec_types[i].value;
        }
        else
        {
            /*
             * We don't recognize the extension. Don't error out here, we'll
             * try and pick out the audio mime type from the HTTP response.
             * The NULL decoder will be a no op for the decoder start.
             */

            codec = AUDIO_CLIENT_CODEC_NULL;
            wiced_log_msg(WICED_LOG_INFO, "No extension match. Check mime type in HTTP response\n");
        }

        result = audio_client_start_decoder(client, codec, err);
        if (result != WICED_SUCCESS)
        {
            return result;
        }
    }

    /*
     * Now spawn off the HTTP reader thread.
     */

    if (audio_client_http_reader_start(client) != WICED_SUCCESS)
    {
        client->decoders[client->audio_codec].decoder_stop(client);
        client->audio_codec = AUDIO_CLIENT_CODEC_NULL;
        *err = AUDIO_CLIENT_ERROR_HTTP_INIT;
    }

    client->state = AUDIO_CLIENT_STATE_PLAY;

    return WICED_SUCCESS;
}


static wiced_result_t audio_client_process_header_complete(audio_client_t* client)
{
    AUDIO_CLIENT_ERROR_T error = AUDIO_CLIENT_ERROR_SUCCESS;
    int i;

    if (client->audio_codec != AUDIO_CLIENT_CODEC_NULL || client->http_load_file)
    {
        /*
         * Nothing to see here...move along.
         */

        return WICED_SUCCESS;
    }

    /*
     * Check to see if it's a mime type we support.
     */

    for (i = 0; mime_types[i].name != NULL; i++)
    {
        if (strncasecmp(mime_types[i].name, client->http_content_type, strlen(mime_types[i].name)) == 0)
        {
            break;
        }
    }

    if (mime_types[i].name == NULL)
    {
        /*
         * We didn't find a match.
         */

        wiced_log_msg(WICED_LOG_ERR, "No decoder configured for mime type: %s\n", client->http_content_type);
        error = AUDIO_CLIENT_ERROR_BAD_CODEC;
    }
    else
    {
        audio_client_start_decoder(client, mime_types[i].value, &error);
    }

    /*
     * If we encountered an error we need to shut down the pipeline.
     */

    if (error != AUDIO_CLIENT_ERROR_SUCCESS)
    {
        /*
         * Set the state to IDLE first so that the stop action doesn't send an event
         * to the app. We'll send the proper error event ourselves.
         */

        client->state = AUDIO_CLIENT_STATE_IDLE;
        client->params.event_cb(client, client->params.userdata, AUDIO_CLIENT_EVENT_ERROR, (void*)error);
        audio_client_stop(client);
    }

    return WICED_SUCCESS;
}


static void audio_client_thread_main(uint32_t context)
{
    audio_client_t* client = (audio_client_t *)context;
    wiced_result_t  result;
    uint32_t        events;

    wiced_log_msg(WICED_LOG_NOTICE, "Audio client thread begin\n");

    if (client == NULL)
    {
        return;
    }

    while (client->tag == AUDIO_CLIENT_TAG_VALID)
    {
        events = 0;

        result = wiced_rtos_wait_for_event_flags(&client->events, AUDIO_CLIENT_ALL_EVENTS, &events, WICED_TRUE, WAIT_FOR_ANY_EVENT, WICED_WAIT_FOREVER);
        if (result != WICED_SUCCESS)
        {
            continue;
        }

        if (events & AUDIO_CLIENT_EVENT_SHUTDOWN)
        {
            break;
        }

        if (events & AUDIO_CLIENT_EVENT_PLAY)
        {
            if (client->pending_uri != NULL)
            {
                AUDIO_CLIENT_ERROR_T err;

                if (audio_client_play(client, &err) != WICED_SUCCESS)
                {
                    client->params.event_cb(client, client->params.userdata, AUDIO_CLIENT_EVENT_ERROR, (void*)err);
                    free(client->pending_uri);
                    client->pending_uri = NULL;
                }
            }
        }

        if (events & (AUDIO_CLIENT_EVENT_STOP | AUDIO_CLIENT_EVENT_HTTP_ERROR))
        {
            audio_client_stop(client);
            if (client->http_load_file && client->http_file_data)
            {
                free(client->http_file_data);
                client->http_file_data = NULL;
                client->http_load_file = WICED_FALSE;
            }
        }

        if (events & AUDIO_CLIENT_EVENT_PAUSE)
        {
            audio_client_pause(client);
        }

        if (events & AUDIO_CLIENT_EVENT_RESUME)
        {
            audio_client_resume(client);
        }

        if (events & AUDIO_CLIENT_EVENT_VOLUME)
        {
            client->params.volume = client->new_volume;
            if (client->audio)
            {
                audio_render_command(client->audio, AUDIO_RENDER_CMD_SET_VOLUME, (void*)client->params.volume);
            }
        }

        if (events & AUDIO_CLIENT_EVENT_SEEK)
        {
            audio_client_seek(client);
        }

        if(events & AUDIO_CLIENT_EVENT_EFFECT)
        {
            audio_client_effect(client);
        }

        if (events & AUDIO_CLIENT_EVENT_HTTP_HEADER_COMPLETE)
        {
            audio_client_process_header_complete(client);
        }

        if (events & AUDIO_CLIENT_EVENT_HTTP_THREAD_DONE)
        {
            audio_client_http_reader_stop(client);
            if (client->http_load_file && client->http_file_data)
            {
                wiced_log_msg(WICED_LOG_DEBUG0, "audio client transition to idle in response to HTTP_THREAD_DONE\n");
                client->params.event_cb(client, client->params.userdata, AUDIO_CLIENT_EVENT_HTTP_COMPLETE, client->http_file_data);
                client->http_load_file = WICED_FALSE;
                client->state          = AUDIO_CLIENT_STATE_IDLE;
            }
        }

        if (events & AUDIO_CLIENT_EVENT_DECODER_THREAD_DONE)
        {
            client->decoders[client->audio_codec].decoder_stop(client);
            client->audio_codec = AUDIO_CLIENT_CODEC_NULL;
            if (client->audio == NULL)
            {
                wiced_bool_t notify_eos = WICED_FALSE;
                /*
                 * If we're not in charge of playback then we go to idle when the decoder is done.
                 */

                wiced_log_msg(WICED_LOG_DEBUG0, "audio client transition to idle in response to DECODER_THREAD_DONE\n");
                if (client->state == AUDIO_CLIENT_STATE_PLAY)
                {
                    notify_eos = WICED_TRUE;
                }
                client->state = AUDIO_CLIENT_STATE_IDLE;
                if (notify_eos == WICED_TRUE)
                {
                    client->params.event_cb(client, client->params.userdata, AUDIO_CLIENT_EVENT_PLAYBACK_EOS, NULL);
                }
            }
            else if (client->audio_configured == WICED_FALSE)
            {
                /*
                 * The audio was never configured successfully so the decoder must have shutdown before
                 * we played any audio. That means that HTTP might still be active and we'll never get
                 * a playback complete message from audio render. We need to make sure that we trigger
                 * a proper cleanup now.
                 */

                audio_client_stop(client);
            }
        }

        if (events & AUDIO_CLIENT_EVENT_PLAYBACK_COMPLETE)
        {
            if (client->state == AUDIO_CLIENT_STATE_PLAY)
            {
                wiced_log_msg(WICED_LOG_DEBUG0, "audio client transition to idle in response to PLAYBACK_COMPLETE\n");
                client->state = AUDIO_CLIENT_STATE_IDLE;
                client->params.event_cb(client, client->params.userdata, AUDIO_CLIENT_EVENT_PLAYBACK_EOS, NULL);
            }
        }
    }   /* while */

    wiced_log_msg(WICED_LOG_NOTICE, "Audio client thread end\n");

    WICED_END_OF_CURRENT_THREAD();
}


static void audio_client_shutdown(audio_client_t* client)
{
    if (client == NULL)
    {
        return;
    }

    /*
     * Stop the main thread if it is running.
     */

    client->tag = AUDIO_CLIENT_TAG_INVALID;
    if (client->client_thread_ptr != NULL)
    {
        wiced_rtos_thread_force_awake(&client->client_thread);
        wiced_rtos_thread_join(&client->client_thread);
        wiced_rtos_delete_thread(&client->client_thread);
        client->client_thread_ptr = NULL;
    }

    if (client->audio != NULL)
    {
        audio_render_deinit(client->audio);
        client->audio = NULL;
    }

    if (client->buf_pool)
    {
        bufmgr_pool_destroy(client->buf_pool);
        client->buf_pool = NULL;
    }

    wiced_rtos_deinit_event_flags(&client->events);
    wiced_rtos_deinit_event_flags(&client->http_events);
    wiced_rtos_deinit_event_flags(&client->decoder_events);

    if (client->socket_ptr != NULL)
    {
        wiced_tcp_delete_socket(client->socket_ptr);
        client->socket_ptr = NULL;
    }

    if (client->data_bufs != NULL)
    {
        free(client->data_bufs);
        client->data_bufs = NULL;
    }

    if (client->pending_uri != NULL)
    {
        free(client->pending_uri);
        client->pending_uri = NULL;
    }

    free(client);
}


static audio_client_t* audio_client_initialize(audio_client_params_t* params)
{
    audio_client_t* client;
    audio_render_params_t render_params;
    wiced_result_t result;

    /*
     * Some basic validation of our parameters.
     */

    if (params->enable_playback)
    {
        if (params->device_id == AUDIO_DEVICE_ID_NONE || params->audio_buffer_num == 0 || params->audio_buffer_size == 0)
        {
            return NULL;
        }
    }
    else
    {
        if (params->buffer_get == NULL || params->buffer_release == NULL)
        {
            return NULL;
        }
    }

    if (params->high_threshold_read_inhibit && (params->data_threshold_high > 0 && params->data_buffer_preroll > params->data_threshold_high))
    {
        wiced_log_msg(WICED_LOG_ERR, "HTTP buffer settings will cause read deadlock\n");
        return NULL;
    }

    /*
     * Allocate the main audio client structure.
     */

    if ((client = calloc(1, sizeof(audio_client_t))) == NULL)
    {
        wiced_log_msg(WICED_LOG_ERR, "Unable to allocate audio client structure\n");
        return NULL;
    }

    /*
     * Copy the configuration parameters.
     */

    memcpy(&client->params, params, sizeof(audio_client_params_t));

    /*
     * Allocate the audio data buffers.
     */

    if ((client->data_bufs = malloc(client->params.data_buffer_num * sizeof(data_buf_t))) == NULL)
    {
        wiced_log_msg(WICED_LOG_ERR, "Unable to allocate audio client data buffers\n");
        free(client);
        return NULL;
    }

    /*
     * Create the event flags.
     */

    result  = wiced_rtos_init_event_flags(&client->events);
    result |= wiced_rtos_init_event_flags(&client->http_events);
    result |= wiced_rtos_init_event_flags(&client->decoder_events);
    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Error initializing audio client event flags\n");
        free(client->data_bufs);
        free(client);
        return NULL;
    }

    if (params->enable_playback)
    {
        wiced_log_msg(WICED_LOG_NOTICE, "Audio_client: enabling playback support\n");

        /*
         * Create the buffer pool.
         */

        if (bufmgr_pool_create(&client->buf_pool, "audio_client_bufs", sizeof(audio_buf_t) + params->audio_buffer_size,
                               params->audio_buffer_num, params->audio_buffer_num, 0, sizeof(uint32_t)) != BUFMGR_OK)
        {
            wiced_log_msg(WICED_LOG_ERR, "bufmgr_pool_create failed\n");
            goto _bail;
        }

        /*
         * Create the audio render component.
         */

        memset(&render_params, 0, sizeof(audio_render_params_t));
        render_params.device_id      = params->device_id;
        render_params.buffer_nodes   = params->audio_buffer_num;
        render_params.userdata       = client;
        render_params.buf_release_cb = audio_render_buffer_release;
        render_params.event_cb       = audio_render_event;

        if (params->audio_period_size != 0)
        {
            render_params.period_size = params->audio_period_size;
        }
        else
        {
            render_params.period_size = AUDIO_CLIENT_PERIOD_SIZE;
        }

        client->audio = audio_render_init(&render_params);
        if (client->audio == NULL)
        {
            wiced_log_msg(WICED_LOG_ERR, "Unable to initialize audio render\n");
            goto _bail;
        }
    }

    /*
     * Set up the decoder functions.
     */

    client->decoders[AUDIO_CLIENT_CODEC_NULL].decoder_start = audio_client_null_decoder_start;
    client->decoders[AUDIO_CLIENT_CODEC_NULL].decoder_stop  = audio_client_null_decoder_stop;
    client->decoders[AUDIO_CLIENT_CODEC_NULL].decoder_ioctl = audio_client_null_decoder_ioctl;
    client->decoders[AUDIO_CLIENT_CODEC_FLAC].decoder_start = audio_client_flac_decoder_start;
    client->decoders[AUDIO_CLIENT_CODEC_FLAC].decoder_stop  = audio_client_flac_decoder_stop;
    client->decoders[AUDIO_CLIENT_CODEC_FLAC].decoder_ioctl = audio_client_flac_decoder_ioctl;
    client->decoders[AUDIO_CLIENT_CODEC_WAV].decoder_start  = audio_client_wav_decoder_start;
    client->decoders[AUDIO_CLIENT_CODEC_WAV].decoder_stop   = audio_client_wav_decoder_stop;
    client->decoders[AUDIO_CLIENT_CODEC_WAV].decoder_ioctl  = audio_client_wav_decoder_ioctl;
    client->decoders[AUDIO_CLIENT_CODEC_AAC].decoder_start  = audio_client_aac_decoder_start;
    client->decoders[AUDIO_CLIENT_CODEC_AAC].decoder_stop   = audio_client_aac_decoder_stop;
    client->decoders[AUDIO_CLIENT_CODEC_AAC].decoder_ioctl  = audio_client_aac_decoder_ioctl;
    client->decoders[AUDIO_CLIENT_CODEC_MP3].decoder_start  = audio_client_mp3_decoder_start;
    client->decoders[AUDIO_CLIENT_CODEC_MP3].decoder_stop   = audio_client_mp3_decoder_stop;
    client->decoders[AUDIO_CLIENT_CODEC_MP3].decoder_ioctl  = audio_client_mp3_decoder_ioctl;

    return client;

_bail:
    audio_client_shutdown(client);
    return NULL;
}


/** Initialize the audio client library.
 *
 * @param[in] params      : Pointer to the configuration parameters.
 *
 * @return Pointer to the audio client instance or NULL
 */

audio_client_ref audio_client_init(audio_client_params_t* params)
{
    audio_client_t* client;
    wiced_result_t result;

    /*
     * An event callback is required.
     */

    if (params == NULL || params->event_cb == NULL)
    {
        return NULL;
    }

    if ((client = audio_client_initialize(params)) == NULL)
    {
        return NULL;
    }

    /*
     * Now create the main audio client thread.
     */

    client->tag   = AUDIO_CLIENT_TAG_VALID;
    client->state = AUDIO_CLIENT_STATE_IDLE;

    result = wiced_rtos_create_thread_with_stack(&client->client_thread, AUDIO_CLIENT_THREAD_PRIORITY, "Audio client thread", audio_client_thread_main,
                                                 client->client_thread_stack_buffer, AUDIO_CLIENT_THREAD_STACK_SIZE, client);
    if (result == WICED_SUCCESS)
    {
        client->client_thread_ptr = &client->client_thread;
    }
    else
    {
        wiced_log_msg(WICED_LOG_ERR, "Unable to create audio client thread\n");
        audio_client_shutdown(client);
        client = NULL;
    }

    return client;
}


/** Deinitialize the audio client library.
 *
 * @param[in] apollo_client : Pointer to the audio client instance.
 *
 * @return    Status of the operation.
 */

wiced_result_t audio_client_deinit(audio_client_ref audio_client)
{
    if (audio_client == NULL || audio_client->tag != AUDIO_CLIENT_TAG_VALID)
    {
        return WICED_BADARG;
    }

    audio_client_shutdown(audio_client);

    return WICED_SUCCESS;
}


/** Send an IOCTL to the audio client session.
 *
 * @param[in] audio_client  : Pointer to the audio client instance.
 * @param[in] cmd           : IOCTL command to process.
 * @param[inout] arg        : Pointer to argument for IOTCL.
 *
 * @return    Status of the operation.
 */

wiced_result_t audio_client_ioctl(audio_client_ref audio_client, AUDIO_CLIENT_IOCTL_T cmd, void* arg)
{
    audio_client_track_info_t* track_info;
    wiced_result_t result;

    if (audio_client == NULL || audio_client->tag != AUDIO_CLIENT_TAG_VALID)
    {
        return WICED_BADARG;
    }

    switch (cmd)
    {
        case AUDIO_CLIENT_IOCTL_STOP:
            wiced_rtos_set_event_flags(&audio_client->events, AUDIO_CLIENT_EVENT_STOP);
            break;

        case AUDIO_CLIENT_IOCTL_LOAD_FILE:
        case AUDIO_CLIENT_IOCTL_PLAY:
            if (audio_client->state != AUDIO_CLIENT_STATE_IDLE || audio_client->pending_uri != NULL)
            {
                return WICED_ERROR;
            }
            audio_client->pending_uri = strdup((char*)arg);
            if (audio_client->pending_uri == NULL)
            {
                wiced_log_msg(WICED_LOG_ERR, "Unable to duplicate URI\n");
                return WICED_ERROR;
            }
            audio_client->http_load_file = (cmd == AUDIO_CLIENT_IOCTL_LOAD_FILE ? WICED_TRUE : WICED_FALSE);
            wiced_rtos_set_event_flags(&audio_client->events, AUDIO_CLIENT_EVENT_PLAY);
            break;

        case AUDIO_CLIENT_IOCTL_PAUSE:
            wiced_rtos_set_event_flags(&audio_client->events, AUDIO_CLIENT_EVENT_PAUSE);
            break;

        case AUDIO_CLIENT_IOCTL_RESUME:
            wiced_rtos_set_event_flags(&audio_client->events, AUDIO_CLIENT_EVENT_RESUME);
            break;

        case AUDIO_CLIENT_IOCTL_SET_VOLUME:
            audio_client->new_volume = (int)arg;
            wiced_rtos_set_event_flags(&audio_client->events, AUDIO_CLIENT_EVENT_VOLUME);
            break;

        case AUDIO_CLIENT_IOCTL_TRACK_INFO:
            if (arg != NULL)
            {
                memset(arg, 0, sizeof(audio_client_track_info_t));
                if (audio_client->state != AUDIO_CLIENT_STATE_IDLE && audio_client->audio_codec != AUDIO_CLIENT_CODEC_NULL)
                {
                    audio_client_stream_info_t info;

                    result = audio_client->decoders[audio_client->audio_codec].decoder_ioctl(audio_client, DECODER_IOCTL_INFO, &info);
                    if (result == WICED_SUCCESS)
                    {
                        track_info = (audio_client_track_info_t*)arg;

                        track_info->current_sample = info.stream_current_sample;
                        track_info->total_samples  = info.stream_total_samples;
                        track_info->sample_rate    = info.stream_sample_rate;
                        track_info->channels       = info.stream_channels;
                        track_info->bps            = info.stream_bps;
                        track_info->bitrate        = info.bit_rate;
                    }
                }
            }
            break;

        case AUDIO_CLIENT_IOCTL_SEEK:
            if (audio_client->state == AUDIO_CLIENT_STATE_IDLE)
            {
                return WICED_ERROR;
            }
            if (audio_client->http_range_requests == WICED_FALSE)
            {
                wiced_log_msg(WICED_LOG_INFO, "Range requests not supported\n");
                return WICED_UNSUPPORTED;
            }

            audio_client->seek_request_ms = (uint32_t)arg;
            wiced_rtos_set_event_flags(&audio_client->events, AUDIO_CLIENT_EVENT_SEEK);
            break;

        case AUDIO_CLIENT_IOCTL_EFFECT:
            audio_client->effect_mode = (uint32_t)arg;
            wiced_rtos_set_event_flags(&audio_client->events, AUDIO_CLIENT_EVENT_EFFECT);
            break;

        default:
            wiced_log_msg(WICED_LOG_ERR, "Unrecognized ioctl: %d\n", cmd);
            break;
    }

    return WICED_SUCCESS;
}
