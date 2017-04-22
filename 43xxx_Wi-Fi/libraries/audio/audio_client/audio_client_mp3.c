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

/** @file Audio Client MP3 Decode Routines
 *
 */
#include <inttypes.h>
#include "wiced_result.h"
#include "wiced_rtos.h"
#include "wiced_platform.h"
#include "wiced_log.h"
#include "wwd_assert.h"

#include "audio_client_private.h"
#include "audio_client_mp3.h"

#include "mpeg_api.h"

/******************************************************
 *                      Macros
 ******************************************************/


/******************************************************
 *                    Constants
 ******************************************************/

#define MP3_TAG_VALID               ( 0x61EDBA19 )
#define MP3_TAG_INVALID             ( 0xDEADBEEF )

/**
 * NOTE: SamplesPerFrame is a function of mpeg_version and layer_level.
 *
 * For MPEG-1.0 Layer-III and Layer-II the samples per frame is 1152 bytes.
 * For MPEG-1.0 Layer-I it's 384 bytes.
 * For MPEG-2.0 and MPEG-2.5, Layer-I is 384 bytes, Layer-II is 1152 bytes, and Layer-III is 576 bytes.
 *
 * see: http://www.mp3-tech.org/programmer/frame_header.html
 *
 * NOTE: we only handle stereo channel on the output
 *      (eventually we will have to do downmix)
 */
#define MP3_MAX_FRAME_SIZE          ( 2048 )
#define MP3_PCM_OUT_CBPS            ( 16 )    /* re request a 16bit PCM out */
#define MP3_PCM_OUT_MAX_CHNUM       ( 2 )     /* stereo files only */

/* coded data buffer size */
#define MP3_CDB_BUF_SIZE            ( 4 * MP3_MAX_FRAME_SIZE )

/*
* PCM output buffer is a circular buffer and size must be power of 2
* and bigger/multiple than MP3_MAX_FRAME_SIZE
* eventually we need more than double buffering to sustain
* a small amount of backpressure from the output render component
* 4 slots should be a good tradeoff.
*/
#define PCM_MAX_FRAME_SIZE          ( MP3_MAX_FRAME_SIZE * (MP3_PCM_OUT_CBPS>>3) *  MP3_PCM_OUT_MAX_CHNUM )
#define PCM_OUTPUT_BUF_SIZE         ( PCM_MAX_FRAME_SIZE  << 2 )



#define AUDIO_CLIENT_DECODER_MP3_THREAD_PRIORITY       ( WICED_DEFAULT_LIBRARY_PRIORITY )
#define AUDIO_CLIENT_DECODER_MP3_STACK_SIZE            ( 8 * 1024 )

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

    void                 *mp3_dec_handle;

    wiced_bool_t         pcm_first_buffer;
    wiced_audio_config_t audio_config;

    audio_client_t*      audio_client;

    wiced_bool_t         id3_check;
    uint32_t             id3_size;
    uint32_t             id3_bytes_skipped;

    /*
     * BRCM Lib MP3 vars
     */
    uint8_t              sync_f;

    /* local pcm output buffer */
    uint32_t             pcm_out_cbps;
    char                 pcm_out[ PCM_OUTPUT_BUF_SIZE ];
    uint32_t             pcm_out_w_idx;
    uint32_t             pcm_out_r_idx;
    /* code data bitstream buffer, double buffering */
    char                 cdb[ MP3_CDB_BUF_SIZE + 1 ];
    uint32_t             cdb_size;
    uint32_t             cdb_w_idx;
    uint32_t             cdb_r_idx;
    mp3frameinfo         frame_info;
    uint8_t              frame_info_f;

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

    uint8_t              stack[ AUDIO_CLIENT_DECODER_MP3_STACK_SIZE ];
    uint8_t              vbr;
    int32_t              avr_bitrate;
} mp3_decoder_t;

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/
static inline wiced_result_t mp3_dec_reset(audio_client_t* client);

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
static uint32_t pcm_output_push( int8_t*            err,
                                 void*              mp3dec,
                                 void*              player,
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


    mp3_decoder_t*             dec = (mp3_decoder_t*) player;

    if ( buf_id == NULL )
    {
        if ( dec->pcm_first_buffer )
        {
            wiced_log_msg( WICED_LOG_INFO, "audio_client_mp3: CONFIGURE audio render\r\n" );

            wiced_time_get_time( &dec->audio_client->play_start_time );

            dec->pcm_first_buffer = WICED_FALSE;

            /*
             * set pcm info from the pcm_info
             */

            if(!(dec->frame_info_f))
            {
                wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: render_configure() missing PCM info\n" );
                dec->audio_client->decoder_done = WICED_TRUE;
                return WICED_ERROR;
            }

            /* make sure we only have a MAX of two channels in the buffer */
            if ( dec->frame_info.numberofchannels != 2 )
            {
                wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: render_configure() needs TWO channels for playback(chnum=%"PRIu32")\n", dec->frame_info.numberofchannels );
                dec->audio_client->decoder_done = WICED_TRUE;
                return WICED_ERROR;
             }

            dec->sample_rate     = dec->frame_info.samplingfrequency;
            dec->num_channels    = dec->frame_info.numberofchannels;
            dec->bits_per_sample = MP3_PCM_OUT_CBPS;
            dec->block_align     = (MP3_PCM_OUT_CBPS >> 3) * dec->num_channels;

            /* MP3 lib does not provide a bitrate api */
            dec->byte_rate       = dec->frame_info.bitrate;

            dec->audio_config.sample_rate     = dec->sample_rate;
            dec->audio_config.channels        = dec->num_channels;
            dec->audio_config.bits_per_sample = dec->bits_per_sample;
            dec->audio_config.frame_size      = dec->block_align;
            dec->audio_config.volume          = dec->audio_client->params.volume;

            if (audio_client_audio_config(dec->audio_client, &dec->audio_config) != WICED_SUCCESS)
            {
                wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: render_configure() failed\n" );
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
        num_frames = ( buf_len   + dec->leftover_audio_bytes ) / dec->block_align;

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
                    wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: decoder corruption\n" );
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
                    /* we consumed data on the mp3_dec component trigger input thread */
                    wiced_rtos_set_event_flags( &dec->audio_client->decoder_events, DECODER_EVENT_AUDIO_DATA );

                    break;
                }
                if ( result == WICED_BADARG )
                {
                    wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: Error pushing audio buffer to the audio_render\n" );
                    return WICED_ERROR;
                }
            }
        }

        if ( ( dec->audio_client->decoder_done == WICED_FALSE ) && ( dec->bufused < ( buf_len ) ) )
        {
            dec->leftover_audio_bytes = (  buf_len ) - dec->bufused;
            if ( dec->leftover_audio_bytes <= sizeof( dec->leftover_audio_data ) )
            {
                for ( old = 0; old < dec->leftover_audio_bytes; old++ )
                {
                    dec->leftover_audio_data[ old ] = *inptr++;
                }

                dec->bufused = (  buf_len );
            }
            else
            {
                /*
                 * This shouldn't happen.
                 */

                wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: Leftover audio bytes out of bounds (%d)\n", dec->leftover_audio_bytes );
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


static inline wiced_result_t mp3_dec_reset(audio_client_t* client)
{
    mp3_decoder_t*      dec;
    audio_client_buf_t  audio_buf;
    int                 errCode = (int) ( BRCM_MP3DEC_NO_ERROR );
    wiced_result_t      result = WICED_SUCCESS;

    dec = (mp3_decoder_t*) client->decoder_handle;

    /* reset MP3 dec, do not touch the pcm output buffer */
    dec->sync_f       = 0;
    dec->cdb_w_idx    = 0;
    dec->cdb_r_idx    = 0;
    //dec->frame_offset = 0;
    dec->frame_info_f = 0;

    errCode = brcm_mp3dec_resetdecoder( dec->mp3_dec_handle, dec->pcm_out_cbps );
    if ( errCode < 0 )
    {
        wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: process_mp3: Unable to reset LIB_MP3 decoder component\r\n" );

        /* stop the decoder thread */
        client->decoder_done = WICED_TRUE;

        /* Tell audio render that no more data is coming for this stream. */
        memset(&audio_buf, 0, sizeof(audio_client_buf_t));
        audio_buf.flags = AUDIO_CLIENT_BUF_FLAG_EOS;
        audio_client_buffer_release(client, &audio_buf);

        result = WICED_ERROR;
    }

    return result;
}

static wiced_result_t process_mp3_data( audio_client_t* client )
{
    mp3_decoder_t*      dec;
    audio_client_buf_t  audio_buf;
    data_buf_t*         dbuf;
    int                 errCode = (int) ( BRCM_MP3DEC_NO_ERROR );
    wiced_result_t      result = WICED_SUCCESS;

    dec = (mp3_decoder_t*) client->decoder_handle;

    dbuf = &client->data_bufs[ client->data_buf_ridx ];
    if ( !dbuf->inuse )
    {
        return WICED_SUCCESS;
    }

    if ( dbuf->buflen == 0 )
    {
        /* ignore this buffer, the end of stream (EOS)       */
        /* comes from the pcm_push callback from the decoder */
        dbuf->buflen  = 0;
        dbuf->bufused = 0;
        dbuf->inuse   = 0;

        client->decoder_done = WICED_TRUE;

        /*
        * Tell audio render that no more data is coming for this stream.
        */

        memset(&audio_buf, 0, sizeof(audio_client_buf_t));
        audio_buf.flags = AUDIO_CLIENT_BUF_FLAG_EOS;
        audio_client_buffer_release(client, &audio_buf);

        return WICED_SUCCESS;
    }

    if (dec->id3_check && dbuf->buflen >= 10)
    {
        /*
         * Check to see if there are ID3 tags in this stream.
         */

        dec->id3_check = WICED_FALSE;

        if (dbuf->buf[0] == 'I' && dbuf->buf[1] == 'D' && dbuf->buf[2] == '3')
        {
            uint8_t major_version;
            uint8_t revision;
            uint8_t flags;

            major_version = dbuf->buf[3];
            revision      = dbuf->buf[4];

            flags = dbuf->buf[5];
            dec->id3_size = ((dbuf->buf[6] & 0x7f) << 21) | ((dbuf->buf[7] & 0x7f) << 14) | ((dbuf->buf[8] & 0x7f) << 7) | (dbuf->buf[9] & 0x7f);

            dec->id3_size += 10;                        /* Add in the ID3 header size */
            if (major_version == 4 && (flags & 0x10))
            {
                /*
                 * There's a footer present.
                 */

                dec->id3_size += 10;
            }

            wiced_log_msg(WICED_LOG_INFO, "ID3 Marker found - ID3v2.%u.%u, size %lu \n", major_version, revision, dec->id3_size);
        }
    }

    /*
     * Are we skipping ID3 bytes?
     */

    if (dec->id3_bytes_skipped < dec->id3_size)
    {
        if (dec->id3_bytes_skipped + dbuf->buflen <= dec->id3_size)
        {
            /*
             * Skip the entire buffer.
             */

            dec->id3_bytes_skipped += dbuf->buflen;
            goto _toss_buffer;
        }

        dbuf->bufused += (dec->id3_size - dec->id3_bytes_skipped);
        dec->id3_bytes_skipped = dec->id3_size;
    }

    /*
     * coalesce into the bitstream buffer and
     * reset if we are overflowing
     */

    if ( dec->cdb_size > ( dec->cdb_w_idx + dbuf->buflen - dbuf->bufused ) )
    {
        /* coalesce copy into the local input buffer */
        memcpy( (dec->cdb + dec->cdb_w_idx), (char*) &dbuf->buf[dbuf->bufused], dbuf->buflen - dbuf->bufused );

        dec->cdb_w_idx += (dbuf->buflen - dbuf->bufused);

        if ( !(dec->sync_f) )
        {
            /*
             * check for frame sync if needed
             */
            errCode = brcm_mp3dec_findsync ( dec->mp3_dec_handle, dec->cdb, &dec->cdb_r_idx, dec->cdb_w_idx );
            if (errCode != BRCM_MP3DEC_BITSTREAM_DECODE_ERROR)
            {
                if (errCode == BRCM_MP3DEC_NO_ERROR)
                {
                    dec->sync_f = 1;

                    wiced_log_msg( WICED_LOG_INFO, "audio_client_mp3: process_mp3: sync FOUND. r_idx=%d\r\n",dec->cdb_r_idx );

                    /* shift down cdb */
                    if (dec->cdb_r_idx > 0)
                    {
                        memcpy( dec->cdb, (dec->cdb + dec->cdb_r_idx),  ( dec->cdb_w_idx - dec->cdb_r_idx ) );

                        dec->cdb_w_idx -= dec->cdb_r_idx;
                        dec->cdb_r_idx = 0;
                    }
                }
            }
            else
            {
                wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: process_mp3: sync search bitstream error\r\n" );

                result = mp3_dec_reset(client);

                return result;
            }
        }

        if( dec->sync_f )
        {
            errCode = (int) ( BRCM_MP3DEC_NO_ERROR );

            uint32_t cdbFrameBytes = 0;

            /* consume data and decode */
            while ( ( errCode != BRCM_MP3DEC_NOT_ENOUGH_DATA_ERROR ) &&
                    ( errCode != BRCM_MP3DEC_BITSTREAM_DECODE_ERROR ) &&
                    ( errCode != BRCM_MP3DEC_ERROR_NO_RESET ) &&
                    ( errCode != BRCM_MP3DEC_DECODER_INTERNAL_ERROR ) &&
                    ( errCode != BRCM_MP3DEC_UNSUPPORTED_BITSTREAM_CONFIG_ERROR ) &&
                    ( dec->cdb_w_idx > ( MP3_MAX_FRAME_SIZE/4 + dec->cdb_r_idx) ) &&
                    ( !client->decoder_done ) )
            {
                /* check if we have to retrieve frame info */
                if(!(dec->frame_info_f))
                {
                    wiced_log_msg( WICED_LOG_INFO, "audio_client_mp3: process_mp3: get frame info\r\n" );

                    errCode = brcm_mp3dec_getframeinfo ( dec->mp3_dec_handle,
                                                         dec->cdb + dec->cdb_r_idx,
                                                         (dec->cdb_w_idx - dec->cdb_r_idx),
                                                         &dec->frame_info );

                    wiced_log_msg( WICED_LOG_INFO,
                                   "audio_client_mp3: process_mp3: nChannels=%d\r\n",
                                   dec->frame_info.numberofchannels );

                    if( errCode == BRCM_MP3DEC_NO_ERROR )
                    {
                        dec->avr_bitrate = dec->frame_info.bitrate;
                        dec->frame_info_f = 1;
                    }
                }

                /* decode frames and shift cdb*/
                if ( errCode == BRCM_MP3DEC_NO_ERROR  )
                {
                    uint32_t srcBytes = 0;
                    uint32_t pcmBytes = 0;

                    /* wrap output buffer if needed */
                    if( PCM_OUTPUT_BUF_SIZE < (dec->pcm_out_w_idx + PCM_MAX_FRAME_SIZE) )
                    {
                        wiced_log_msg( WICED_LOG_DEBUG4,
                                       "audio_client_mp3: process_mp3: output REWIND w_idx= %d, %d %d\r\n",
                                       dec->pcm_out_w_idx,PCM_OUTPUT_BUF_SIZE, MP3_MAX_FRAME_SIZE);

                        dec->pcm_out_w_idx = 0;
                    }

                    wiced_log_msg( WICED_LOG_DEBUG4,
                                   "audio_client_mp3: process_mp3: DECODE-01 src=%p r_idx=%d w_idx=%d pcm=%p out_w_idx=%d\r\n",
                                   dec->cdb + dec->cdb_r_idx,
                                   dec->cdb_r_idx,
                                   dec->cdb_w_idx,
                                   dec->pcm_out, dec->pcm_out_w_idx);

                    /* cdb bytes left to decode */
                    srcBytes = dec->cdb_w_idx - dec->cdb_r_idx;

                    errCode = brcm_mp3dec_decodeframe(
                        dec->mp3_dec_handle,
                        dec->cdb + dec->cdb_r_idx,
                        &srcBytes,
                        dec->pcm_out + dec->pcm_out_w_idx,
                        &pcmBytes );

                    wiced_log_msg( WICED_LOG_DEBUG4,
                                   "audio_client_mp3: process_mp3: DECODE-02 src=%p r_idx=%d w_idx=%d srcBytes=%d pcmBytes%d\r\n",
                                   dec->cdb + dec->cdb_r_idx,
                                   dec->cdb_r_idx,
                                   dec->cdb_w_idx,
                                   srcBytes, pcmBytes);

                    /* update pointers and shift cdb */
                    if( ( srcBytes > 0) &&
                        ( errCode != BRCM_MP3DEC_NOT_ENOUGH_DATA_ERROR ) &&
                        ( errCode != BRCM_MP3DEC_BITSTREAM_DECODE_ERROR ) &&
                        ( errCode != BRCM_MP3DEC_UNSUPPORTED_BITSTREAM_CONFIG_ERROR ) )
                    {
                        uint32_t cur_bitrate = brcm_mp3dec_getbitrate( dec->mp3_dec_handle );
                        uint16_t sample_per_frame = brcm_mp3dec_getsampleperframe( dec->mp3_dec_handle );

                        /* Check CBR or VBR */
                        if ( dec->avr_bitrate != cur_bitrate )
                        {
                            dec->vbr = 1;
                        }

                        /* Recalculate avr_bitrate */
                        if ( dec->vbr == 1 )
                        {
                            dec->avr_bitrate += (int) ( (int) cur_bitrate - (int) dec->avr_bitrate ) / (int) ( dec->audio_frames_played / sample_per_frame + 1 );

                        }

                        cdbFrameBytes += srcBytes;

                        pcm_output_push( NULL,
                                         dec->mp3_dec_handle,
                                         dec,
                                         (uint8_t*)dec->pcm_out + dec->pcm_out_w_idx,
                                         pcmBytes,
                                         NULL);

                        /* update indexes */
                        dec->cdb_r_idx     += srcBytes;
                        dec->pcm_out_w_idx += pcmBytes;
                    }
                    else
                    {
                        if( ( errCode == BRCM_MP3DEC_BITSTREAM_DECODE_ERROR ) ||
                            ( errCode == BRCM_MP3DEC_UNSUPPORTED_BITSTREAM_CONFIG_ERROR ) ||
                            ( errCode == BRCM_MP3DEC_DECODER_INTERNAL_ERROR )  )
                        {
                            wiced_log_msg(WICED_LOG_ERR, "audio_client_mp3: process_mp3: zero/end byte decode errCode=%d "
                                          "r_idx=%d w_idx=%d cdbFrameBytes=%d\r\n",
                                          errCode,
                                          dec->cdb_r_idx,
                                          dec->cdb_w_idx,
                                          cdbFrameBytes );
                        }
                        else
                        {
                            wiced_log_msg(WICED_LOG_DEBUG4, "audio_client_mp3: process_mp3: errCode=%d\r\n",
                                          errCode);
                        }
                    }

                    wiced_log_msg(WICED_LOG_DEBUG4,
                                  "audio_client_mp3: process_mp3: DECODE-03 src=%p "
                                  "r_idx=%d w_idx=%d pcm=%p out_w_idx=%d (errCode=%d)\r\n\n",
                                  dec->cdb + dec->cdb_r_idx,
                                  dec->cdb_r_idx,
                                  dec->cdb_w_idx,
                                  dec->pcm_out, dec->pcm_out_w_idx,
                                  errCode);
                }
            }

            /* shift cdb if needed */
            if( dec->cdb_r_idx > 0 )
            {
                memcpy( dec->cdb, (dec->cdb + dec->cdb_r_idx),  ( dec->cdb_w_idx - dec->cdb_r_idx ) );

                dec->cdb_w_idx -= dec->cdb_r_idx;
                dec->cdb_r_idx = 0;

                wiced_log_msg( WICED_LOG_DEBUG3,
                               "audio_client_mp3: process_mp3: ALL_DATA_DECODED"
                               "src=%p r_idx=%d w_idx=%d pcm=%p out_w_idx=%d\r\n\n",
                               dec->cdb + dec->cdb_r_idx,
                               dec->cdb_r_idx,
                               dec->cdb_w_idx,
                               dec->pcm_out, dec->pcm_out_w_idx);
            }

            /* reset decoder if we had a bitstream failure */
            if ( ( errCode == BRCM_MP3DEC_BITSTREAM_DECODE_ERROR ) ||
                 ( errCode == BRCM_MP3DEC_UNSUPPORTED_BITSTREAM_CONFIG_ERROR ) ||
                 ( errCode == BRCM_MP3DEC_DECODER_INTERNAL_ERROR ) )
            {
                wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: process_mp3: bitstream failure, RESET decoder\r\n" );

                result = mp3_dec_reset(client);
                if (result != WICED_SUCCESS)
                {
                    return result;
                }
            }
        }
    }
    else
    {
        wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: process_mp3: coalesce buffer OVERFLOW cdbsize=%d r_idx=%d w_idx=%d buflen=%d\r\n",
                       dec->cdb_size, dec->cdb_r_idx, dec->cdb_w_idx , dbuf->buflen  );

        result = mp3_dec_reset(client);
        if (result != WICED_SUCCESS)
        {
            return result;
        }

        /*
         * More data waiting to be processed?
         */

        dbuf = &client->data_bufs[ client->data_buf_ridx ];
        if ( dbuf->inuse || dec->cdb_r_idx < dec->cdb_w_idx)
        {
            wiced_rtos_set_event_flags( &client->decoder_events, DECODER_EVENT_AUDIO_DATA );
        }

        return WICED_SUCCESS;
    }

    wiced_log_msg( WICED_LOG_DEBUG3, "audio_client_mp3: process_mp3: NEXT AUDIO BUF src=%p r_idx=%d w_idx=%d pcm=%p out_w_idx=%d\r\n\n",
                   dec->cdb + dec->cdb_r_idx,
                   dec->cdb_r_idx,
                   dec->cdb_w_idx,
                   dec->pcm_out, dec->pcm_out_w_idx);

  _toss_buffer:
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

    return WICED_SUCCESS;
}


static void audio_client_mp3_thread( uint32_t arg )
{
    audio_client_t* client = (audio_client_t*) arg;
    uint32_t        events;
    wiced_result_t  result;

    wiced_log_msg( WICED_LOG_INFO, "audio_client_mp3: mp3_thread start\n" );

    while ( !client->decoder_done )

    {
        events = 0;

        result = wiced_rtos_wait_for_event_flags( &client->decoder_events,
                                                  DECODER_ALL_EVENTS,
                                                  &events,
                                                  WICED_TRUE, WAIT_FOR_ANY_EVENT, WICED_WAIT_FOREVER );

        if ( ( result != WICED_SUCCESS ) || client->decoder_done )
        {
            break;
        }

        if ( events & DECODER_EVENT_AUDIO_DATA )
        {
            if ( process_mp3_data( client ) != WICED_SUCCESS )
            {
                break;
            }
        }
    }

    wiced_log_msg( WICED_LOG_INFO, "audio_client_mp3: mp3_thread exit\n" );

    wiced_rtos_set_event_flags( &client->events, AUDIO_CLIENT_EVENT_DECODER_THREAD_DONE );

    WICED_END_OF_CURRENT_THREAD( );
}


wiced_result_t audio_client_mp3_decoder_start( audio_client_t* client )
{
    mp3_decoder_t* dec = NULL;
    int            errCode = (int) ( BRCM_MP3DEC_NO_ERROR );
    wiced_result_t result = WICED_SUCCESS;

    uint32_t     memsize  = 0;

    if ( client == NULL )
    {
        wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: mp3_decoder_start: Bad ARG\n" );
        return WICED_BADARG;
    }

    /*
     * Allocate the internal decoder structure.
     */
    dec = (mp3_decoder_t*) calloc( 1, sizeof( mp3_decoder_t ) );
    if ( dec == NULL )
    {
        wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: Unable to allocate decoder structure\n" );

        return WICED_ERROR;
    }

    /*
     * Allocate the Lib_MP3 decoder component area.
     */

    dec->pcm_first_buffer = WICED_TRUE;

    /* ref to parent */
    dec->audio_client = client;

    memsize = brcm_mp3dec_getmem_requirement( MP3_PCM_OUT_CBPS );

    if ( NULL == ( dec->mp3_dec_handle = (uint8_t*) calloc( memsize, sizeof( uint8_t ) ) ) )
    {
        free(dec);
        dec = NULL;

        wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: Unable to allocate LIB_MP3 decoder component\n" );
        return WICED_ERROR;
    }

    wiced_log_msg( WICED_LOG_INFO, "audio_client_mp3: LIB_MP3 version %ld \n", brcm_mp3dec_getversion());

    errCode = brcm_mp3dec_createdecoder( dec->mp3_dec_handle, MP3_PCM_OUT_CBPS );
    if ( errCode < 0 )
    {
        wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: Unable to create LIB_MP3  decoder component\n" );

        free(dec->mp3_dec_handle);
        dec->mp3_dec_handle = NULL;

        free(dec);
        dec = NULL;

        return WICED_ERROR;
    }

    errCode = brcm_mp3dec_resetdecoder( dec->mp3_dec_handle, MP3_PCM_OUT_CBPS );
    if ( errCode < 0 )
    {
        wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: Unable to reset LIB_MP3  decoder component\n" );

        free(dec->mp3_dec_handle);
        dec->mp3_dec_handle = NULL;

        free(dec);
        dec = NULL;

        return WICED_ERROR;
    }

    /*
     * complete client configs
     */

    client->decoder_handle = dec;
    dec->tag = MP3_TAG_VALID;

    /* LIB_MP3 settings */
    dec->cdb_size        = MP3_CDB_BUF_SIZE;
    dec->sync_f          = 0;
    dec->cdb_w_idx       = 0;
    dec->cdb_r_idx       = 0;
    //dec->frame_offset    = 0;
    dec->frame_info_f    = 0;
    dec->pcm_out_cbps    = MP3_PCM_OUT_CBPS;
    dec->pcm_out_w_idx   = 0;
    dec->pcm_out_r_idx   = 0;

    dec->id3_check       = WICED_TRUE;

    dec->vbr             = 0;
    dec->avr_bitrate     = 0;

    /* Start decoder thread */
    client->decoder_done = WICED_FALSE;

    result = wiced_rtos_create_thread_with_stack(
        &client->decoder_thread,
        AUDIO_CLIENT_DECODER_MP3_THREAD_PRIORITY,
        "Mp3 Decoder",
        audio_client_mp3_thread,
        dec->stack,
        AUDIO_CLIENT_DECODER_MP3_STACK_SIZE,
        client );

    if ( result != WICED_SUCCESS )
    {
        wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: start: wiced_rtos_create_thread failed %d\n", result );
        return WICED_ERROR;
    }
    else
    {
        wiced_log_msg( WICED_LOG_INFO, "audio_client_mp3: start: audio_client_mp3_tread create SUCCESS.\n" );
        client->decoder_thread_ptr = &client->decoder_thread;
    }

    return result;
}


wiced_result_t audio_client_mp3_decoder_stop( audio_client_t* client )
{
    mp3_decoder_t* dec = NULL;

    if ( client == NULL )
    {
        wiced_log_msg( WICED_LOG_ERR, "audio_client_mp3: stop: Bad ARG\n" );
        return WICED_BADARG;
    }

    dec = client->decoder_handle;

    /* close the mp3_dec component */
    if ( client->decoder_thread_ptr != NULL )
    {
        wiced_log_msg( WICED_LOG_INFO, "audio_client_mp3: join thread\n" );

        client->decoder_done = WICED_TRUE;
        wiced_rtos_thread_force_awake( &client->decoder_thread );
        wiced_rtos_thread_join( &client->decoder_thread );
        wiced_rtos_delete_thread( &client->decoder_thread );
        client->decoder_thread_ptr = NULL;

        if ( dec->mp3_dec_handle != NULL )
        {
            wiced_log_msg( WICED_LOG_INFO, "audio_client_mp3: LIB_MP3 delete decoder\n" );
            brcm_mp3dec_deletedecoder ( dec->mp3_dec_handle );
            free( dec->mp3_dec_handle );
            dec->mp3_dec_handle = NULL;
        }

        if ( client->decoder_handle )
        {
            wiced_log_msg( WICED_LOG_INFO, "audio_client_mp3: invalidate decoder handle\n" );
            ( (mp3_decoder_t*) client->decoder_handle )->tag = MP3_TAG_INVALID;
            free( client->decoder_handle );
            client->decoder_handle = NULL;
        }
    }

    return WICED_SUCCESS;
}


wiced_result_t audio_client_mp3_decoder_ioctl(struct audio_client_s* client, DECODER_IOCTL_T ioctl, void* arg)
{
    mp3_decoder_t* dec;
    audio_client_stream_info_t* info;

    if (client == NULL)
    {
        wiced_log_msg(WICED_LOG_ERR, "audio_client_mp3_decoder_ioctl: Bad handle\n");
        return WICED_BADARG;
    }

    dec = (mp3_decoder_t*)client->decoder_handle;
    if (dec == NULL || dec->tag != MP3_TAG_VALID)
    {
        wiced_log_msg(WICED_LOG_ERR, "audio_client_mp3_decoder_ioctl: Bad decoder handle\n");
        return WICED_BADARG;
    }

    switch (ioctl)
    {
        case DECODER_IOCTL_INFO:
            info = (audio_client_stream_info_t*)arg;
            if (info != NULL)
            {
                dec->audio_frames_total     = 0;
                info->stream_total_samples  = 0;
                info->stream_current_sample = 0;

                if(dec->frame_info_f)
                {
                    if( dec->num_channels != 0 )
                    {
                        info->stream_total_samples  = dec->audio_frames_total / dec->num_channels;
                        info->stream_current_sample = dec->audio_frames_played;
                    }
                    info->stream_sample_rate    = dec->sample_rate;
                    info->stream_channels       = dec->num_channels;
                    info->stream_bps            = dec->bits_per_sample;
                    info->bit_rate              = dec->avr_bitrate;
                }
            }
            break;
        case DECODER_IOCTL_GET_SEEK_POSITION:
        case DECODER_IOCTL_SET_POSITION:
        default:
            return WICED_UNSUPPORTED;
    }

    return WICED_SUCCESS;
}
