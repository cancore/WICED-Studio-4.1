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

/** @file Audio Client AAC Decode Routines
 *
 */

#include "wiced_result.h"
#include "wiced_rtos.h"
#include "wiced_platform.h"
#include "wiced_log.h"
#include "wwd_assert.h"

#include "audio_client_private.h"
#include "audio_client_aac.h"

#include "aacdec.h"

/******************************************************
 *                      Macros
 ******************************************************/


/******************************************************
 *                    Constants
 ******************************************************/

#define AAC_TAG_VALID               ( 0x61EDBA17 )
#define AAC_TAG_INVALID             ( 0xDEADBEEF )


#define AUDIO_CLIENT_DECODER_AAC_THREAD_PRIORITY       ( WICED_DEFAULT_LIBRARY_PRIORITY )
#define AUDIO_CLIENT_DECODER_AAC_STACK_SIZE            ( 8 * 1024 )

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct
{
    uint32_t             tag;
    int                  bufused;

    int                  leftover_audio_bytes;
    uint8_t              leftover_audio_data[ 8 ];

    AACDEC_HANDLE        aac_dec_handle;
    AACDEC_ERR           aac_dec_err;

    wiced_bool_t         pcm_first_buffer;
    wiced_audio_config_t audio_config;

    audio_client_t*      audio_client;

    /*
     *  Audio format information.
     */

    uint16_t             num_channels;
    uint32_t             sample_rate;
    uint32_t             byte_rate;
    uint16_t             block_align;
    uint16_t             bits_per_sample;

    /*
     * Some housekeeping variables.
     */

    uint32_t             audio_frames_total;
    uint32_t             audio_frames_played;

    uint8_t              stack[ AUDIO_CLIENT_DECODER_AAC_STACK_SIZE ];
} aac_decoder_t;

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
static uint32_t pcm_output_push( int8_t*            err,
                                 void*              aacdec,
                                 void*              player,
                                 aacdec_pcm_info_t* pcm_info,
                                 uint8_t*           buf,
                                 uint32_t           buf_len,
                                 uint32_t*          buf_id )
{
    audio_client_buf_t         audio_buf;
    wiced_result_t             result;
    uint8_t*                   inptr;
    uint8_t*                   outptr;
    int                        frames_to_copy;
    int                        output_framesize;
    int                        num_frames;
    int                        frames;
    int                        size;
    int                        old;
    int                        new;


    aac_decoder_t*             dec = (aac_decoder_t*) player;


    if ( buf_id == NULL )
    {
        if ( dec->pcm_first_buffer )
        {
            wiced_log_msg( WICED_LOG_INFO, "audio_client_aac: CONFIGURE audio render\n" );

            wiced_time_get_time( &dec->audio_client->play_start_time );

            dec->pcm_first_buffer = WICED_FALSE;

            /*
             * set pcm info from the pcm_info
             */

            if ( NULL == pcm_info )
            {
                wiced_log_msg( WICED_LOG_ERR, "audio_client_aac: render_configure() missing PCM info\n" );
                dec->audio_client->decoder_done = WICED_TRUE;
                return WICED_ERROR;
            }

            /* make sure we only have a MAX of two channels in the buffer */
            if ( pcm_info->chnum != 2 )
            {
                wiced_log_msg( WICED_LOG_ERR, "audio_client_aac: render_configure() needs TWO channels for playback(chnum=%" PRIu32 ")\n", pcm_info->chnum );
                dec->audio_client->decoder_done = WICED_TRUE;
                return WICED_ERROR;
            }

            dec->sample_rate     = pcm_info->sr;
            dec->num_channels    = pcm_info->chnum;
            dec->bits_per_sample = pcm_info->cbps;
            dec->block_align     = ( pcm_info->cbps >> 3 ) * pcm_info->chnum;

            /* TBD: should we get the instantaneous bitrate from aac decoder? */
            // dec->byte_rate     = le32toh(*((uint16_t*)&ptr[WAV_FMT_DATA_OFFSET + 8]));

            dec->audio_config.sample_rate     = dec->sample_rate;
            dec->audio_config.channels        = dec->num_channels;
            dec->audio_config.bits_per_sample = dec->bits_per_sample;
            dec->audio_config.frame_size      = dec->block_align;
            dec->audio_config.volume          = dec->audio_client->params.volume;

            if (audio_client_audio_config(dec->audio_client, &dec->audio_config) != WICED_SUCCESS)
            {
                wiced_log_msg( WICED_LOG_ERR, "audio_client_aac: render_configure() failed\n" );
                dec->audio_client->decoder_done = WICED_TRUE;
                return WICED_ERROR;
            }
        }

        /*
         * OUTPUT copy in chunks (if needed)
         */
        dec->bufused = 0;
        dec->leftover_audio_bytes = 0;

        inptr = buf;
        num_frames = ( buf_len * 2  + dec->leftover_audio_bytes ) / dec->block_align;

        while ( ( num_frames > 0 ) && ( dec->audio_client->decoder_done == WICED_FALSE ) )
        {
            result = audio_client_buffer_get(dec->audio_client, &audio_buf, AUDIO_CLIENT_AUDIO_BUF_TIMEOUT);
            if ( result != WICED_SUCCESS )
            {
                /* we probably didn't get a buffer because the buffers are full */
                continue;
            }

            output_framesize = ( dec->bits_per_sample * dec->num_channels ) / 8;
            if ( output_framesize * num_frames <= audio_buf.buflen )
            {
                frames_to_copy = num_frames;
            }
            else
            {
                frames_to_copy = audio_buf.buflen / output_framesize;
            }

            outptr = (uint8_t*) audio_buf.buf;
            switch ( dec->bits_per_sample )
            {
                case 8:
                case 16:
                case 32:
                    if ( dec->leftover_audio_bytes != 0 )
                    {
                        memcpy( outptr, dec->leftover_audio_data, dec->leftover_audio_bytes );
                        outptr += dec->leftover_audio_bytes;
                        dec->leftover_audio_bytes = 0;
                    }


                    size = ( frames_to_copy * output_framesize );
                    memcpy( outptr, inptr, size );
                    outptr        += size;
                    inptr         += size;
                    dec->bufused  += size;
                    break;

                case 24:
                    /* handle filling the frame after leftover bytes used */
                    frames = frames_to_copy;
                    if ( dec->leftover_audio_bytes )
                    {
                        *outptr++ = 0;
                        for ( old = 0; old < dec->leftover_audio_bytes; old++ )
                        {
                            if ( old == 3 )
                            {
                                *outptr++ = 0;
                            }
                            *outptr++ = dec->leftover_audio_data[ old ];
                        }

                        for ( new = dec->leftover_audio_bytes; new < 6; new++ )
                        {
                            if ( new == 3 )
                            {
                                *outptr++ = 0;
                            }
                            *outptr++ = *inptr++;
                        }

                        dec->leftover_audio_bytes = 0;
                        frames--;
                    }

                    /* We have handled the leftover bytes and finished a frame (both channels) with them.
                     * Now copy over the rest of the audio.
                     */

                    while ( frames > 0 )
                    {
                        *outptr++ = 0;
                        *outptr++ = *inptr++;
                        *outptr++ = *inptr++;
                        *outptr++ = *inptr++;

                        *outptr++ = 0;
                        *outptr++ = *inptr++;
                        *outptr++ = *inptr++;
                        *outptr++ = *inptr++;

                        frames--;
                    }
                    break;

                default:
                    wiced_log_msg( WICED_LOG_ERR, "audio_client_aac: decoder corruption\n" );
                    return WICED_ERROR;
            }

            dec->audio_frames_played += frames_to_copy;
            num_frames -= frames_to_copy;

            dec->bufused = (int) inptr - (int) buf;

            audio_buf.offset = 0;
            audio_buf.curlen = frames_to_copy * output_framesize;

            while ( dec->audio_client->decoder_done == WICED_FALSE )
            {
                result = audio_client_buffer_release(dec->audio_client, &audio_buf);
                if ( result == WICED_SUCCESS )
                {
                    /* we consumed data on the aac_dec component trigger input thread */
                    wiced_rtos_set_event_flags( &dec->audio_client->decoder_events, DECODER_EVENT_AUDIO_DATA );

                    break;
                }
                if ( result == WICED_BADARG )
                {
                    wiced_log_msg( WICED_LOG_ERR, "audio_client_aac: Error pushing audio buffer\n" );
                    return WICED_ERROR;
                }
            }
        }

        if ( ( dec->audio_client->decoder_done == WICED_FALSE ) && ( dec->bufused < ( 2 * buf_len ) ) )
        {
            dec->leftover_audio_bytes = ( 2 * buf_len ) - dec->bufused;
            if ( dec->leftover_audio_bytes <= sizeof( dec->leftover_audio_data ) )
            {
                for ( old = 0; old < dec->leftover_audio_bytes; old++ )
                {
                    dec->leftover_audio_data[ old ] = *inptr++;
                }

                dec->bufused = ( 2 * buf_len );
            }
            else
            {
                /*
                 * This shouldn't happen.
                 */

                wiced_log_msg( WICED_LOG_ERR, "audio_client_aac: Leftover audio bytes out of bounds (%d)\n", dec->leftover_audio_bytes );
            }
        }
    }


    /*
     *  COMMAND PCM PACKET
     */
    if ( buf_id != NULL )
    {
        if ( *buf_id == 0 )
        {
            dec->audio_client->decoder_done = WICED_TRUE;

            /*
             * Tell audio render that no more data is coming for this stream.
             */

            memset(&audio_buf, 0, sizeof(audio_client_buf_t));
            audio_buf.flags = AUDIO_CLIENT_BUF_FLAG_EOS;
            audio_client_buffer_release(dec->audio_client, &audio_buf);
        }
    }

    return WICED_SUCCESS;
}


static wiced_result_t process_aac_data( audio_client_t* client )
{
    aac_decoder_t* dec;
    data_buf_t*    dbuf;
    int8_t         rv = AACDEC_ERR_NONE ;

    dec = (aac_decoder_t*) client->decoder_handle;

    dbuf = &client->data_bufs[ client->data_buf_ridx ];
    if ( !dbuf->inuse )
    {
        return WICED_SUCCESS;
    }

    /*
     * command buffer
     */
    if ( dbuf->buflen == 0 )
    {
        /* this is an EOS buffer, we ignore the content  */
        /* and send a proper command to the aacdec library */
        dbuf->buflen  = 0;
        dbuf->bufused = 0;
        dbuf->inuse   = 0;

        uint8_t tmp_buf = 0;
        aacdec_input_push_buf( &rv , dec->aac_dec_handle, (char*) &tmp_buf , (uint32_t) sizeof(tmp_buf), AACDEC_BUFFER_TYPE_CMD_EOS );


        return ((rv == AACDEC_ERR_NONE) ? WICED_SUCCESS : WICED_ERROR);
    }

    /*
     * data buffer
     */
    aacdec_input_push_buf( &rv, dec->aac_dec_handle, (char*) dbuf->buf, (uint32_t) dbuf->buflen, AACDEC_BUFFER_TYPE_DATA );

    /* check for overflow or error */
    if ( ( rv == AACDEC_ERR_NEED_MORE_BITS ) ||
         ( rv == AACDEC_ERR_INPUT_QUEUE_OVERFLOW ) ||
         ( rv == AACDEC_ERR_NONE ) )
    {
        if ( rv != AACDEC_ERR_INPUT_QUEUE_OVERFLOW )
        {
            /*
             * This buffer is done. Advance to the next one.
             */

            dbuf->buflen  = 0;
            dbuf->bufused = 0;
            dbuf->inuse   = 0;
            client->data_buf_ridx = ( client->data_buf_ridx + 1 ) % client->params.data_buffer_num;

            CHECK_FOR_THRESHOLD_LOW_EVENT( client );

            /*
             * More data waiting to be processed?
             */

            dbuf = &client->data_bufs[ client->data_buf_ridx ];
            if ( dbuf->inuse )
            {
                wiced_rtos_set_event_flags( &client->decoder_events, DECODER_EVENT_AUDIO_DATA );
            }
        }
    }
    else
    {
        wiced_log_msg( WICED_LOG_ERR, "audio_client_aac: process_aac: error from mp4parse push\n" );

        /* system error from aac_dec_ push */
        client->decoder_done = WICED_TRUE;
        return WICED_ERROR;
    }


    return WICED_SUCCESS;
}


static void audio_client_aac_thread( uint32_t arg )
{
    audio_client_t* client = (audio_client_t*) arg;
    uint32_t        events;
    wiced_result_t  result;

    wiced_log_msg( WICED_LOG_INFO, "audio_client_aac: aac_thread start\n" );

    while ( !client->decoder_done )

    {
        events = 0;

        result = wiced_rtos_wait_for_event_flags( &client->decoder_events, DECODER_ALL_EVENTS, &events, WICED_TRUE, WAIT_FOR_ANY_EVENT, WICED_WAIT_FOREVER );
        if ( ( result != WICED_SUCCESS ) || client->decoder_done )
        {
            break;
        }

        if ( events & DECODER_EVENT_AUDIO_DATA )
        {
            if ( process_aac_data( client ) != WICED_SUCCESS )
            {
                break;
            }
        }
    }

    wiced_log_msg( WICED_LOG_INFO, "audio_client_aac: aac_thread exit\n" );

    wiced_rtos_set_event_flags( &client->events, AUDIO_CLIENT_EVENT_DECODER_THREAD_DONE );

    WICED_END_OF_CURRENT_THREAD( );
}


wiced_result_t audio_client_aac_decoder_start( audio_client_t* client )
{
    aac_decoder_t* dec;
    wiced_result_t result;

    aacdec_stream_t stream_type = AACDEC_STREAM_UNKNOWN;

    if ( client == NULL )
    {
        wiced_log_msg( WICED_LOG_ERR, "audio_client_aac: aac_decoder_start: Bad ARG\n" );
        return WICED_BADARG;
    }

    /*
     * select the proper STREAM_TYPE if the codec/mime is known.
     * the default case is UNKNOWN.
     */

    /* AAC framed stream, MPEG-2 or MPEG-4 but NO transport stream (.ts) */
    if( (strstr(client->pending_uri, ".aac") != NULL) ||
        (strncasecmp("audio/aac", client->http_content_type, strlen("audio/aac")) == 0) ||
        (strncasecmp("audio/x-aac", client->http_content_type, strlen("audio/x-aac")) == 0) )
    {
        stream_type = AACDEC_STREAM_ADTS;
    }

    /* MPEG4FF */
    if( (strstr(client->pending_uri, ".m4a") != NULL) ||
        (strncasecmp("audio/m4a", client->http_content_type, strlen("audio/m4a")) == 0) ||
        (strncasecmp("audio/x-m4a", client->http_content_type, strlen("audio/x-m4a")) == 0) )
    {
        stream_type = AACDEC_STREAM_M4A;
    }

    /* sanity check: we don't have an "auto select" internal feature yet,
     * so we return an error if the stream type is not properly initialized
     */
    if( stream_type == AACDEC_STREAM_UNKNOWN )
    {
        return WICED_ERROR;
    }

    /*
     * Allocate the internal decoder structure.
     */
    dec = (aac_decoder_t*) calloc( 1, sizeof( aac_decoder_t ) );
    if ( dec == NULL )
    {
        wiced_log_msg( WICED_LOG_ERR, "audio_client_aac: Unable to allocate decoder structure\n" );
        return WICED_ERROR;
    }

    /*
     * START the aac_dec component
     */
    if ( NULL != dec )
    {
        aacdec_new( NULL,
                    &dec->aac_dec_handle,
                    dec,
                    0,                        /* LOG_NONE */
                    stream_type,
                    NULL,                     /* PCM_TARGET_FMT   */
                    AACDEC_CHANNEL_MAP_NONE,  /* PCM_TARGET_CHMAP */
                    0 );                      /* DOWNMIX_FLAG */

        /*
         * CONFIG the aac_dec component
         */
        if ( NULL != dec->aac_dec_handle )
        {
            dec->pcm_first_buffer = WICED_TRUE;

            /* ref to parent */
            dec->audio_client = client;


            int8_t rv = 0;

            /* set pcm_callback for PCM rendering */
            aacdec_set_output_push_callback(
                &rv,
                dec->aac_dec_handle,
                &pcm_output_push );

            /* start aac_dec processing */
            aacdec_start(
                &rv,
                dec->aac_dec_handle );

            if ( rv != 0 )
            {
                return WICED_ERROR;
            }
        }
        else
        {
            wiced_log_msg( WICED_LOG_ERR, "audio_client_aac: Unable to create the aac_dec component\n" );
            return WICED_ERROR;
        }
    }


    /*
     * complete client configs
     */
    client->decoder_handle = dec;
    dec->tag = AAC_TAG_VALID;

    /* Start decoder thread */
    client->decoder_done = WICED_FALSE;

    result = wiced_rtos_create_thread_with_stack(
        &client->decoder_thread,
        AUDIO_CLIENT_DECODER_AAC_THREAD_PRIORITY,
        "Aac Decoder",
        audio_client_aac_thread,
        dec->stack,
        AUDIO_CLIENT_DECODER_AAC_STACK_SIZE,
        client );

    if ( result != WICED_SUCCESS )
    {
        wiced_log_msg( WICED_LOG_ERR, "audio_client_aac: start: wiced_rtos_create_thread failed %d\n", result );
        return WICED_ERROR;
    }
    else
    {
        client->decoder_thread_ptr = &client->decoder_thread;
    }

    return result;
}


wiced_result_t audio_client_aac_decoder_stop( audio_client_t* client )
{
    aac_decoder_t* dec = NULL;

    if ( client == NULL )
    {
        wiced_log_msg( WICED_LOG_ERR, "audio_client_aac: stop: Bad ARG\n" );
        return WICED_BADARG;
    }

    dec = client->decoder_handle;

    /* close the aac_dec component */
    if ( dec->aac_dec_handle != NULL )
    {
        aacdec_stop( NULL,
                     dec->aac_dec_handle,
                     1 /*FLUSH*/ );

        /* flushig is done on stop, so we force */
        aacdec_destroy( NULL,
                        &dec->aac_dec_handle,
                        1 /*FORCE QUIT*/ );
    }

    if ( client->decoder_thread_ptr != NULL )
    {
        wiced_log_msg( WICED_LOG_INFO, "audio_client_aac_decoder_stop\n" );

        client->decoder_done = WICED_TRUE;
        wiced_rtos_thread_force_awake( &client->decoder_thread );
        wiced_rtos_thread_join( &client->decoder_thread );
        wiced_rtos_delete_thread( &client->decoder_thread );
        client->decoder_thread_ptr = NULL;

        if ( client->decoder_handle )
        {
            ( (aac_decoder_t*) client->decoder_handle )->tag = AAC_TAG_INVALID;
            free( client->decoder_handle );
            client->decoder_handle = NULL;
        }
    }

    return WICED_SUCCESS;
}


wiced_result_t audio_client_aac_decoder_ioctl(struct audio_client_s* client, DECODER_IOCTL_T ioctl, void* arg)
{
    aac_decoder_t* dec;
    audio_client_stream_info_t* info;

    if (client == NULL)
    {
        wiced_log_msg(WICED_LOG_ERR, "audio_client_aac_decoder_ioctl: Bad handle\n");
        return WICED_BADARG;
    }

    dec = (aac_decoder_t*)client->decoder_handle;
    if (dec == NULL || dec->tag != AAC_TAG_VALID)
    {
        wiced_log_msg(WICED_LOG_ERR, "audio_client_aac_decoder_ioctl: Bad decoder handle\n");
        return WICED_BADARG;
    }

    switch (ioctl)
    {
        case DECODER_IOCTL_INFO:
            info = (audio_client_stream_info_t*)arg;
            if (info != NULL)
            {
                aacdec_track_info_t  audio_track_info;

                if( 0 == dec->audio_frames_total )
                {
                    aacdec_get_trackInfo( NULL,
                                          dec->aac_dec_handle,
                                          &audio_track_info);

                    if( ( 0 != audio_track_info.duration ) &&
                        ( 0 != audio_track_info.timescale ) )
                    {
                        /* duration time approx based on timescale */
                        uint64_t duration_msec  = (audio_track_info.duration * 1024 + (audio_track_info.timescale/2)) / audio_track_info.timescale;
                        dec->audio_frames_total = (duration_msec * (dec->bits_per_sample * dec->num_channels * dec->sample_rate)) /1024;
                    }
                }

                info->stream_total_samples  = 0;
                info->stream_current_sample = 0;
                if( dec->num_channels != 0 )
                {
                    info->stream_total_samples  = dec->audio_frames_total / dec->num_channels;
                    info->stream_current_sample = dec->audio_frames_played / dec->num_channels;
                }
                info->stream_sample_rate    = dec->sample_rate;
                info->stream_channels       = dec->num_channels;
                info->stream_bps            = dec->bits_per_sample;
            }
            break;

        case DECODER_IOCTL_GET_SEEK_POSITION:
        case DECODER_IOCTL_SET_POSITION:
        default:
            return WICED_UNSUPPORTED;
    }

    return WICED_SUCCESS;
}
