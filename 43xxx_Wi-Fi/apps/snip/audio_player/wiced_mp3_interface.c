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

#include "wiced.h"
#include "platform_audio.h"
#include "audio_render.h"
#include "resources.h"

/* FLAC library includes */
#include <FLAC/ordinals.h>
#include <stream_encoder.h>
#include <stream_decoder.h>
#include <metadata.h>
#include "wiced_audio_interface.h"

#include "wiced_mp3_internal.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define MILLISECONDS_PER_SECOND         (uint64_t)(1000)

#define SAMPLES_TO_MILLISECONDS(number_of_samples, sample_rate) ((MILLISECONDS_PER_SECOND * number_of_samples) / sample_rate)

/******************************************************
 *                    Constants
 ******************************************************/
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
 *               Function Declarations
 ******************************************************/
static wiced_result_t  wiced_mp3_get_data_from_input_queue(mp3_internal_t* internal, uint8_t* buffer, uint32_t buffer_size, uint32_t *bytes_copied);
static wiced_result_t  wiced_mp3_push_data_to_audio_render(mp3_internal_t* internal, const FLAC__Frame *frame, const FLAC__int32 * const buffer[]);
/******************************************************
 *               Variables Definitions
 ******************************************************/

/* un-comment to save stream data to the DDR memory to check
 * #define SAVE_FLAC_TO_DDR_DEBUG_MAX
 */
#ifdef SAVE_MP3_TO_DDR_DEBUG_MAX
#define WICED_MP3_BUFFER_DDR_OFFSET    (0)
#define WICED_MP3_BUFFER_DDR_ADDRESS   ((uint8_t *)PLATFORM_DDR_BASE(WICED_MP3_BUFFER_DDR_OFFSET))

uint8_t*   mp3_test_data_output = WICED_MP3_BUFFER_DDR_ADDRESS;
uint32_t    mp3_test_data_output_index = 0;
#endif  /* SAVE_MP3_TO_DDR_DEBUG_MAX */

/* our internal structure */
static mp3_internal_t  *g_internal;

/******************************************************
 *       Internal Function Definitions
 ******************************************************/

/*
 * copy packet(s) of data from queue
 *
 * This is called from the FLAC READ callback
 *
 */
static wiced_result_t  wiced_mp3_get_data_from_input_queue(mp3_internal_t* internal, uint8_t* buffer, uint32_t buffer_size, uint32_t *bytes_copied)
{
    wiced_result_t          result = WICED_SUCCESS;
    uint32_t                chunk_size;
    uint32_t                total_copied;

    wiced_assert("wiced_mp3_get_data_from_input_queue() internal == NULL!", (internal != NULL) && (buffer != NULL) &&
                                                                 (buffer_size > 0) && (bytes_copied != NULL));

    MP3_LIB_PRINT(AUDIO_PLAYER_LOG_DEBUG, ("wiced_mp3_get_data_from_input_queue() stream.data_offset: %ld !\r\n", internal->stream.data_offset));

    *bytes_copied = 0;
    total_copied = 0;
    while (*bytes_copied < buffer_size)
    {
        /* check if user stopped playback */
        if (internal->user_stop == WICED_TRUE)
        {
            /* end of stream */
            wiced_time_t time;
            wiced_time_get_time( &time );
            printf("wiced_mp3_get_data_from_input_queue() user stop %ld\r\n", time);
           return WICED_ERROR;
        }

        /* check for a partially used packet */
        if (internal->stream.packet.packet == NULL)
        {
            /* pop one off the queue if we don't have a partial one waiting */
            result = wiced_rtos_pop_from_queue(&internal->mp3_packet_queue, &internal->stream.packet, MP3_QUEUE_POP_TIMEOUT );
            MP3_LIB_PRINT(AUDIO_PLAYER_LOG_DEBUG, ("FLAC: wiced_rtos_pop_from_queue(%d) returned %d !\n", internal->debug_queue_count, result));
            if (result == WICED_SUCCESS)
            {
                internal->debug_queue_count--;
            }
            else if(result != WICED_TIMEOUT)
            {
                MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("FLAC: wiced_mp3_get_data_from_input_queue(%d) Failed!%d\n", internal->debug_queue_count, result));
                return result;
            }
            else
            {
                continue;
            }
        }

        if (internal->stream.packet.packet != NULL)
        {
            uint8_t* in_data;
            uint16_t avail_data_length;
            uint16_t total_data_length;

            if( wiced_packet_get_data( internal->stream.packet.packet, 0, &in_data,
                                       &avail_data_length, &total_data_length) != WICED_SUCCESS)
            {
                MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("FLAC: wiced_mp3_get_data_from_input_queue() wiced_packet_get_data Failed!%d\n", result));
                return WICED_ERROR;
            }

            /* copy just what is available !!! */
            if (internal->stream.packet.pkt_offset < avail_data_length)
            {
                chunk_size = avail_data_length - internal->stream.packet.pkt_offset;
                if (chunk_size > buffer_size)
                {
                    chunk_size = buffer_size;
                }
                memcpy(buffer, &in_data[internal->stream.packet.pkt_offset], chunk_size);
                total_copied += chunk_size;
                internal->stream.packet.pkt_offset += chunk_size;
            }

            /* update bytes copied */
            *bytes_copied = total_copied;

            /* are we done with this packet? */
            if (internal->stream.packet.pkt_offset >= avail_data_length)
            {
                wiced_packet_delete( internal->stream.packet.packet );
                memset(&internal->stream.packet, 0, sizeof(internal->stream.packet));
                return WICED_SUCCESS;
            }
        }

        if ((internal->stream.end_of_input_stream == WICED_TRUE) &&
            (wiced_rtos_is_queue_empty(&internal->mp3_packet_queue) == WICED_SUCCESS))
        {
            if (*bytes_copied > 0)
            {
                return WICED_SUCCESS;
            }
            else
            {
                /* end of stream */
                wiced_time_t time;
                wiced_time_get_time( &time );
                printf("wiced_mp3_get_data_from_input_queue() end of stream %ld\r\n", time);
                return WICED_ERROR;
            }
        }
    }

    MP3_LIB_PRINT(AUDIO_PLAYER_LOG_DEBUG, ("FLAC: wiced_mp3_get_data_from_input_queue() DONE -- partial offset: %ld!\n", internal->stream.packet.pkt_offset));

    return WICED_SUCCESS;
}

wiced_result_t  wiced_mp3_push_data_to_audio_render(mp3_internal_t* internal, const FLAC__Frame *frame, const FLAC__int32 * const buffer[])
{
//    uint64_t                        sample_number, sample_rate;
//    uint64_t                        pts;

//    uint32_t                        samples_given_to_us;
//    wiced_decoder_buffer_info_t        LPCM_buff_info;


    /* here is where we get the decoded LPCM data - write it to the audio renderer */
    MP3_LIB_PRINT(AUDIO_PLAYER_LOG_DEBUG, ("wiced_mp3_push_data_to_audio_render()\r\n"));
    MP3_LIB_PRINT(AUDIO_PLAYER_LOG_DEBUG, ("         blocksize: %d\r\n", frame->header.blocksize));
    MP3_LIB_PRINT(AUDIO_PLAYER_LOG_DEBUG, ("          channels: %d\r\n", frame->header.channels));
    MP3_LIB_PRINT(AUDIO_PLAYER_LOG_DEBUG, ("      this channel:(%d) %s \r\n", frame->header.channel_assignment, FLAC__ChannelAssignmentString[frame->header.channel_assignment]));
    MP3_LIB_PRINT(AUDIO_PLAYER_LOG_DEBUG, ("   bits per sample: %d\r\n", frame->header.bits_per_sample));
    MP3_LIB_PRINT(AUDIO_PLAYER_LOG_DEBUG, ("              rate: %d\r\n", frame->header.sample_rate));
    MP3_LIB_PRINT(AUDIO_PLAYER_LOG_DEBUG, ("         buffer[0]: %p\r\n", buffer[0]));
    MP3_LIB_PRINT(AUDIO_PLAYER_LOG_DEBUG, ("         buffer[1]: %p\r\n", buffer[1]));
    MP3_LIB_PRINT(AUDIO_PLAYER_LOG_DEBUG, ("         buffer[2]: %p\r\n\r\n", buffer[2]));

    if (internal->buffer_get == NULL)
    {
        MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("Not buffer get function!\r\n"));
        return WICED_ERROR;
    }

#if 0
    samples_given_to_us = frame->header.blocksize;          /* updated in loop if we don't use all the first time */
    sample_number = frame->header.number.sample_number;     /* updated in loop if we don't use all the first time */
    sample_rate   = (uint64_t)frame->header.sample_rate;
    while (samples_given_to_us > 0)
    {
        uint32_t            total_data_length;
        uint32_t            chunk_size, samples_to_copy;
        uint32_t*           in[AUDIO_PLAYER_INPUT_BUFFER_MAX];
        wiced_bool_t        fill_FL_with_zero, fill_FR_with_zero;
        uint32_t            output_bytes_per_sample;


        if (internal->user_stop == WICED_TRUE)
        {
            wiced_time_t time;
            wiced_time_get_time( &time );
            MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("wiced_mp3_push_data_to_audio_render() user stop %ld\r\n", time));
            return WICED_ERROR;
        }

        /* get a buffer to use from our list, also informs us of the channel mapping app wants */
        if (internal->buffer_get(&LPCM_buff_info) != WICED_SUCCESS)
        {
            continue;
        }

        /* all FLAC decoded samples are 32 bits in size */
        total_data_length = (frame->header.blocksize * sizeof(FLAC__int32) * frame->header.channels);
        chunk_size = total_data_length;
        if (chunk_size > LPCM_buff_info.data_buf_size)
        {
            chunk_size = LPCM_buff_info.data_buf_size;
        }
        /* The LPCM buffer is built for 32 bits per sample.
         *   for 16 bits per sample, we designate the buffer as 16 bits
         *   for 24 or 32 bits per sample, we designate the buffer as 32 bits
         */
        output_bytes_per_sample = (frame->header.bits_per_sample == 24 ? sizeof(FLAC__int32) : (frame->header.bits_per_sample/8));
        samples_to_copy = chunk_size / (output_bytes_per_sample * LPCM_buff_info.num_channels);

        if (samples_to_copy > frame->header.blocksize)
        {
            samples_to_copy = frame->header.blocksize;
        }

        /* copy samples over to where audio renderer wants them
         *
         *  FLAC output buffers
         *  #       Buffer
         *  Chan    [0] [1] [2] [3] [4] [5]
         *  1       FL
         *  2       FL  FR
         *  3       FL  FR  FC
         *  4       FL  FR  BL  BR
         *  5       FL  FR  FC  BL  BR
         *  5.1     FL  FR  FC  LFE BL  BR
         *
         */
        /* clear out our input buffer pointers, fill input based on what we want for output
         * cross-correlate the input buffers to the output we want in out render buffer
         * so that in[FLAC_INPUT_BUFFER_XX] always points to the appropriate buffer (may be NULL!)
         */
        memset(in, 0, sizeof(in));
        in[FLAC_INPUT_BUFFER_FL] = buffer[0];
        in[FLAC_INPUT_BUFFER_FR] = buffer[1];
        in[FLAC_INPUT_BUFFER_FC] = buffer[2];
        switch (frame->header.channels)
        {
        case 4:
            in[FLAC_INPUT_BUFFER_FC] = NULL;
            in[FLAC_INPUT_BUFFER_BL] = buffer[2];
            in[FLAC_INPUT_BUFFER_BR] = buffer[3];
            break;
        case 5:
            in[FLAC_INPUT_BUFFER_BL] = buffer[3];
            in[FLAC_INPUT_BUFFER_BR] = buffer[4];
            break;
        case 6: /* 5.1 */
            in[FLAC_INPUT_BUFFER_LFE] = buffer[3];
            in[FLAC_INPUT_BUFFER_BL] = buffer[4];
            in[FLAC_INPUT_BUFFER_BR] = buffer[5];
            break;
        }

        /* if the output buffer is for 2 channels, but FL or FR is not in the channel mask, we need to fill
         * the other part of the buffer with 0x00
         */
        fill_FL_with_zero = (LPCM_buff_info.channel_mask & CHANNEL_MAP_FL) ? WICED_FALSE : WICED_TRUE;
        fill_FR_with_zero = (LPCM_buff_info.channel_mask & CHANNEL_MAP_FR) ? WICED_FALSE : WICED_TRUE;

        /* The array "in" has 1 pointer per input, each being a 32bit buffer.
         * we want 16 or 32 bits per channel per sample interleaved into the output.
         */

        switch (output_bytes_per_sample)
        {
        case 2:
            {
                uint16_t    count = samples_to_copy;
                uint16_t*    out_buff;
                out_buff = (uint16_t*)LPCM_buff_info.data_buf;
                while (count > 0)
                {
                    if (fill_FL_with_zero == WICED_TRUE) *out_buff++ = 0x00;
                    else if (in[FLAC_INPUT_BUFFER_FL] != NULL) *out_buff++ = (uint16_t)(*in[FLAC_INPUT_BUFFER_FL]++);
                    if (fill_FR_with_zero == WICED_TRUE) *out_buff++ = 0x00;
                    else if (in[FLAC_INPUT_BUFFER_FR] != NULL) *out_buff++ = (uint16_t)(*in[FLAC_INPUT_BUFFER_FR]++);
                    if (in[FLAC_INPUT_BUFFER_FC] != NULL) *out_buff++ = (uint16_t)(*in[FLAC_INPUT_BUFFER_FC]++);
                    if (in[FLAC_INPUT_BUFFER_LFE] != NULL) *out_buff++ = (uint16_t)(*in[FLAC_INPUT_BUFFER_LFE]++);
                    if (in[FLAC_INPUT_BUFFER_BL] != NULL) *out_buff++ = (uint16_t)(*in[FLAC_INPUT_BUFFER_BL]++);
                    if (in[FLAC_INPUT_BUFFER_BR] != NULL) *out_buff++ = (uint16_t)(*in[FLAC_INPUT_BUFFER_BR]++);
                    count--;
                }
            }
            break;
        case 4:
            {
                uint16_t    count = samples_to_copy;
                uint32_t*    out_buff;
                out_buff = (uint32_t*)LPCM_buff_info.data_buf;
                while (count > 0)
                {
                    if (fill_FL_with_zero == WICED_TRUE) *out_buff++ = 0x00;
                    else if (in[FLAC_INPUT_BUFFER_FL] != NULL) *out_buff++ = (uint32_t)(*in[FLAC_INPUT_BUFFER_FL]++);
                    if (fill_FR_with_zero == WICED_TRUE) *out_buff++ = 0x00;
                    else if (in[FLAC_INPUT_BUFFER_FR] != NULL) *out_buff++ = (uint32_t)(*in[FLAC_INPUT_BUFFER_FR]++);
                    if (in[FLAC_INPUT_BUFFER_FC] != NULL) *out_buff++ = (uint32_t)(*in[FLAC_INPUT_BUFFER_FC]++);
                    if (in[FLAC_INPUT_BUFFER_LFE] != NULL) *out_buff++ = (uint32_t)(*in[FLAC_INPUT_BUFFER_LFE]++);
                    if (in[FLAC_INPUT_BUFFER_BL] != NULL) *out_buff++ = (uint32_t)(*in[FLAC_INPUT_BUFFER_BL]++);
                    if (in[FLAC_INPUT_BUFFER_BR] != NULL) *out_buff++ = (uint32_t)(*in[FLAC_INPUT_BUFFER_BR]++);
                    count--;
                }
            }
        break;
        default:
            continue;
        }

        /* figure out pts value */
        pts = SAMPLES_TO_MILLISECONDS(sample_number, sample_rate);

        {
            /* debug */
            wiced_time_t time;
            wiced_time_get_time( &time );
            MP3_LIB_PRINT(AUDIO_PLAYER_LOG_INFO, ("flac_test_write_callback() time: %ld\r\n", time));
            MP3_LIB_PRINT(AUDIO_PLAYER_LOG_INFO, ("  sample_number: %ld%ld of %ld%ld \r\n",
                                             (uint32_t)(sample_number / 0x00010000), (uint32_t)(sample_number % 0x00010000),
                                             (uint32_t)(internal->stream.total_samples / 0x00010000),
                                             (uint32_t)(internal->stream.total_samples % 0x00010000)));
            MP3_LIB_PRINT(AUDIO_PLAYER_LOG_INFO, ("    sample_rate: %ld \r\n", (uint32_t)sample_rate));
            MP3_LIB_PRINT(AUDIO_PLAYER_LOG_INFO, ("            pts: %ld%ld \r\n", (uint32_t)(pts / 0x00010000), (uint32_t)(pts % 0x00010000) ));
        }

        /* fill the buffer info */

        LPCM_buff_info.filled_data_offset   = 0;
        LPCM_buff_info.filled_samples       = samples_to_copy;
        LPCM_buff_info.filled_channels      = LPCM_buff_info.num_channels;
        LPCM_buff_info.filled_data_length   = (samples_to_copy * (output_bytes_per_sample * LPCM_buff_info.num_channels));
        LPCM_buff_info.filled_pts           = pts;
        LPCM_buff_info.current_sample       = frame->header.number.sample_number;

        LPCM_buff_info.source_total_samples = internal->stream.total_samples;
        LPCM_buff_info.source_sample_rate   = frame->header.sample_rate;
        LPCM_buff_info.source_channels      = frame->header.channels;
        LPCM_buff_info.source_bps           = frame->header.bits_per_sample;
        LPCM_buff_info.frame_size           = output_bytes_per_sample;

        /* keep info on current track */
        internal->source_info.source_total_samples = internal->stream.total_samples;
        internal->source_info.source_current_sample = frame->header.number.sample_number;
        internal->source_info.source_sample_rate = frame->header.sample_rate;
        internal->source_info.source_channels = frame->header.channels;
        internal->source_info.source_bps = frame->header.bits_per_sample;


#ifdef SAVE_FLAC_TO_DDR_DEBUG_MAX
        /* write decoded data to DDR buffer */
        {
            uint8_t*    buffer      = LPCM_buff_info.data_buf;
            uint16_t    buffer_size = LPCM_buff_info.data_length;
            memcpy(&flac_test_data_output[flac_test_data_output_index], buffer, buffer_size);
            flac_test_data_output_index += buffer_size;
            printf("test_data_output_index %ld\r\n", flac_test_data_output_index);
            wiced_assert("Done capturing FLAC output to DDR\r\n", (flac_test_data_output_index < SAVE_FLAC_TO_DDR_DEBUG_MAX));
        }
#endif


        if ((internal->buffer_push == NULL) || (internal->buffer_push(&LPCM_buff_info) != WICED_SUCCESS))
        {
            MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("FLAC: write_callback() audio_buffer_push() failed\r\n"));
        }

        samples_given_to_us -= samples_to_copy;
        sample_number += samples_to_copy;

    } /* while samples to send */
#endif

    return WICED_SUCCESS;
}

/*
 * FLAC library shim layer thread
 *
 * This allows us to call blocking functions async
 *
 */

void mp3_worker_thread(uint32_t arg)
{
    wiced_result_t        result;
    uint32_t              events;
    mp3_internal_t*      internal;

    wiced_assert("wiced_mp3_worker_thread() ARG == NULL!", (arg != 0));
    internal = (mp3_internal_t*)arg;

    while(1)
    {
        events = 0;
        result = wiced_rtos_wait_for_event_flags(&internal->mp3_events, MP3_EVENT_WORKER_THREAD_EVENTS, &events,
                WICED_TRUE, WAIT_FOR_ANY_EVENT, WICED_NEVER_TIMEOUT);

        if (result != WICED_SUCCESS)
        {
            continue;
        }

        if (events & MP3_EVENT_WORKER_THREAD_SHUTDOWN)
        {
            break;
        }

        if (events & MP3_EVENT_WORKER_DECODE_FRAME)
        {
//            FLAC__bool ret;

            /* empty the queue */
            while ((internal->user_stop == WICED_FALSE) &&
                   (wiced_rtos_is_queue_empty( &internal->mp3_packet_queue ) != WICED_SUCCESS))
            {
//                ret = FLAC__stream_decoder_process_single(internal->flac_decoder);
//                if (ret == false)
//                {
//                    MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("FLAC__stream_decoder_process_single() ret = %d \r\n", ret));
//                }
            }
            /* make sure we finish up */
//            ret = FLAC__stream_decoder_process_single(internal->flac_decoder);
//            if (ret == false)
//            {
//                MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("FLAC__stream_decoder_process_single() ret = %d \r\n", ret));
//            }
        }
    }

    MP3_LIB_PRINT(AUDIO_PLAYER_LOG_INFO, ("FLAC: Worker thread shutting down!\n"));

    /* Signal worker thread we are done */
    internal->mp3_worker_thread_ptr = NULL;
    wiced_rtos_set_event_flags(&internal->mp3_events, MP3_EVENT_WORKER_THREAD_DONE);

    MP3_LIB_PRINT(AUDIO_PLAYER_LOG_INFO, ("FLAC: Worker thread DONE\n"));
}

/******************************************************
 *               External Function Definitions
 ******************************************************/

 void* wiced_mp3_decoder_init(wiced_decoder_params_t* params)
{
    /* sanity checks */
    if ((params == NULL) || (params->buff_get == NULL) || (params->buff_push == NULL))
    {
        WPRINT_LIB_INFO(("wiced_mp3_decoder_init() BAD ARG! %p\r\n", params));
        return NULL;
    }

    /*
     * Allocate the main internal structure.
     */

    g_internal = (mp3_internal_t*)calloc_named("mp3_internal", 1, sizeof(mp3_internal_t));
    if (g_internal == NULL)
    {
        WPRINT_LIB_INFO(("wiced_mp3_decoder_init() Unable to allocate internal structure\r\n"));
        goto _internal_init_error;
    }

    /* application call backs */
    g_internal->buffer_get  = params->buff_get;   /* get a buffer to fill with LPCM and submit  */
    g_internal->buffer_push = params->buff_push;  /* push a buffer with LPCM data in it to the audio render */

    /*
     * Create our event flags.
     */

    if (wiced_rtos_init_event_flags(&g_internal->mp3_events) != WICED_SUCCESS)
    {
        WPRINT_LIB_INFO(("Error initializing flac event flags\r\n"));
        goto _internal_init_error;
    }

    /* initialize a FLAC decoder instance */
//    g_internal->flac_decoder = FLAC__stream_decoder_new();
//    if (g_internal->flac_decoder == NULL)
//    {
//        WPRINT_LIB_INFO(("Unable to create new FLAC decoder\r\n"));
//        goto _internal_init_error;
//    }

    /* initialize packet queue */
    if (wiced_rtos_init_queue( &g_internal->mp3_packet_queue, "FLAC_packet_queue", sizeof( audio_player_packet_info_t),
                               APP_QUEUE_MAX_ENTRIES) != WICED_SUCCESS)
    {
        WPRINT_LIB_INFO(("MP3: wiced_rtos_init_queue() failed !\r\n"));
        goto _internal_init_error;
    }

    /* Start worker thread */
    if (wiced_rtos_create_thread( &g_internal->mp3_worker_thread, MP3_WORKER_THREAD_PRIORITY, "MP3 worker",
                                   mp3_worker_thread, MP3_WORKER_STACK_SIZE, g_internal) != WICED_SUCCESS)
    {
        WPRINT_LIB_INFO(("wiced_rtos_create_thread(mp3_worker) failed!\r\n"));
        goto _internal_init_error;
    }
    else
    {
        g_internal->mp3_worker_thread_ptr = &g_internal->mp3_worker_thread;
    }


    g_internal->tag = MP3_INTERNAL_TAG_VALID;

    return (void *)g_internal;

_internal_init_error:

    WPRINT_LIB_INFO(("mp3_decoder_init() FAILED !\r\n"));

    /* we need to free the stuff we allocated if we error out here */
    if (g_internal != NULL)
    {
        if (g_internal->mp3_decoder != NULL)
        {
            FLAC__stream_decoder_delete(g_internal->mp3_decoder);
        }
        g_internal->mp3_decoder =  NULL;

        wiced_rtos_deinit_queue( &g_internal->mp3_packet_queue );

        wiced_rtos_deinit_event_flags(&g_internal->mp3_events);

        free(g_internal);
        g_internal = NULL;
    }

    return NULL;

}

wiced_result_t wiced_flac_decoder_deinit(void* internal_ptr)
{
//    audio_player_packet_info_t  flac_packet;
    mp3_internal_t*    internal = (mp3_internal_t*)internal_ptr;


    if ((internal == NULL) || (internal->tag != MP3_INTERNAL_TAG_VALID))
    {
        WPRINT_LIB_INFO(("libflac_iface_flac_decoder_deinit() Bad arg!\r\n"));
        return WICED_ERROR;
    }

    g_internal->tag = MP3_INTERNAL_TAG_INVALID;

//    if (internal->flac_decoder != NULL)
//    {
//        FLAC__stream_decoder_delete(internal->flac_decoder);
//
//        /* empty the queue and deinit */
//        while (wiced_rtos_is_queue_empty( &internal->flac_packet_queue ) != WICED_SUCCESS)
//        {
//            wiced_rtos_pop_from_queue(&internal->flac_packet_queue, &flac_packet, MP3_QUEUE_POP_TIMEOUT );
//        }
//        wiced_rtos_deinit_queue( &internal->flac_packet_queue );
//
//    }
//    internal->flac_decoder = NULL;

//    if (internal->flac_metadata != NULL)
//    {
//        free(internal->flac_metadata);
//    }
//    internal->flac_metadata = NULL;

    return WICED_SUCCESS;
}

wiced_result_t wiced_flac_decoder_start(void* internal_ptr)
{
    FLAC__StreamDecoderInitStatus   flac_init_status;
    mp3_internal_t*    internal = (mp3_internal_t*)internal_ptr;

    if ((internal == NULL) || (internal->tag != MP3_INTERNAL_TAG_VALID))
    {
        MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("libflac_iface_decoder_start() Bad ARG!\r\n"));
        return WICED_ERROR;
    }

//    if (internal->flac_decoder == NULL)
//    {
//        MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("libflac_iface_decoder_start() FLAC decoder not initialised!\r\n"));
//        return WICED_ERROR;
//    }

#ifdef SAVE_FLAC_TO_DDR_DEBUG_MAX
    /* clear out the test output */
    flac_test_data_output_index = 0; /* for debug */
#endif /* SAVE_FLAC_TO_DDR_DEBUG_MAX */

    internal->user_stop = WICED_FALSE;

    /* clear out stream info */
    memset(&internal->stream, 0, sizeof(internal->stream));

    /* make sure the decoder is clean */
//    if (FLAC__stream_decoder_flush(internal->flac_decoder) == false)
//    {
//        MP3_LIB_PRINT(AUDIO_PLAYER_LOG_DEBUG, ("FLAC decoder flush failed\r\n"));
//    }
//
//    if (FLAC__stream_decoder_finish(internal->flac_decoder) == false)
//    {
//        MP3_LIB_PRINT(AUDIO_PLAYER_LOG_DEBUG, ("FLAC decoder finish failed\r\n"));
//    }

//    flac_init_status = FLAC__stream_decoder_init_stream(
//            internal->flac_decoder,
//        flac_test_read_callback,
//#ifdef SUPPORT_FLAC_SEEK_CAPABILITY
//        flac_test_seek_callback, flac_test_tell_callback, flac_test_length_callback, flac_test_eof_callback,
//#else
//        NULL, NULL, NULL, NULL,
//#endif
//        flac_test_write_callback,
//        flac_test_metadata_callback,
//        flac_test_error_callback,
//        internal
//    );
//
    if (flac_init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
    {
        MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("Unable to init FLAC stream\r\n"));
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_flac_decoder_submit_packet(void* internal_ptr, wiced_packet_t* packet)
{
    wiced_result_t      result;
     audio_player_packet_info_t  flac_packet;
    mp3_internal_t*    internal = (mp3_internal_t*)internal_ptr;

    if ((internal == NULL) || (internal->tag != MP3_INTERNAL_TAG_VALID) || (packet == NULL))
    {
        MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("wiced_flac_decoder_submit_packet() Bad ARG!\r\n"));
        return WICED_ERROR;
    }

    flac_packet.packet      = packet;
    flac_packet.pkt_offset  = 0;            /* haven't read anything yet */
    flac_packet.pts         = 0;            /* let play back routine set this (until we get actual pts values) */
    result = wiced_rtos_push_to_queue( &internal->mp3_packet_queue, &flac_packet, MP3_QUEUE_PUSH_TIMEOUT );
    if (result != WICED_SUCCESS)
    {
        MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("wiced_rtos_push_to_queue() failed! %d\r\n", result));
        return WICED_ERROR;
    }

    internal->debug_queue_count++;
    if (internal->debug_queue_count > internal->debug_queue_max)
    {
        internal->debug_queue_max = internal->debug_queue_count;
    }

    /* send signal to start decoder */
    wiced_rtos_set_event_flags(&internal->mp3_events, MP3_EVENT_WORKER_DECODE_FRAME);

    return WICED_SUCCESS;
}


wiced_result_t wiced_flac_decoder_stop(void* internal_ptr)
{
    mp3_internal_t*    internal = (mp3_internal_t*)internal_ptr;

    if ((internal == NULL) || (internal->tag != MP3_INTERNAL_TAG_VALID))
    {
        MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("libflac_iface_decoder_stop() Bad ARG!\r\n"));
        return WICED_ERROR;
    }

    internal->user_stop = WICED_TRUE;

    wiced_time_t time;
    wiced_time_get_time( &time );
    printf("wiced_flac_decoder_stop() user stop %ld\r\n", time);

    return WICED_SUCCESS;
}

/** Get simple information about the currently playing source
 *
 * @param internal_ptr  : pointer returned from wiced_flac_decoder_init()
 * @param source_info   : pointer to structure to store information
 *
 * @return  WICED_SUCCESS
 *          WICED_BAD_ARG
 *          WICED_ERROR
 */
wiced_result_t wiced_flac_get_source_info(void* internal_ptr, wiced_audio_player_source_info_t* source_info)
{
    mp3_internal_t* internal = (mp3_internal_t*)internal_ptr;

    if ((internal == NULL) || (internal->tag != MP3_INTERNAL_TAG_VALID) || (source_info == NULL))
    {
        MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("wiced_flac_get_source_info() Bad arg!\r\n"));
        return WICED_ERROR;
    }

    source_info->source_current_sample  = internal->source_info.source_current_sample;
    source_info->source_total_samples   = internal->source_info.source_total_samples;
    source_info->source_sample_rate     = internal->source_info.source_sample_rate;
    source_info->source_channels        = internal->source_info.source_channels;
    source_info->source_bps             = internal->source_info.source_bps;

    return WICED_SUCCESS;

}

/* TODO: return metadata in a WICED format
 * libflac_iface_get_metadata
 *
 * /param internal     - mp3_internal_t pointer
 * /param metadata   - storage for metadata (not used at this time)
 *
 * return WICED_SUCCESS or WICED_ERROR
 */
wiced_result_t wiced_flac_get_metadata(void* internal_ptr, void *metadata)
{
    mp3_internal_t* internal = (mp3_internal_t*)internal_ptr;

    if ((internal == NULL) || (internal->tag != MP3_INTERNAL_TAG_VALID) || (metadata == NULL))
    {
        MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("wiced_flac_get_metadata() Bad arg!\r\n"));
        return WICED_ERROR;
    }

    return WICED_ERROR;
}

wiced_result_t wiced_flac_set_log_level(void* internal_ptr, wiced_flac_log_level_t log_level)
{
    mp3_internal_t* internal = (mp3_internal_t*)internal_ptr;

    if ((internal == NULL) || (internal->tag != MP3_INTERNAL_TAG_VALID))
    {
        MP3_LIB_PRINT(AUDIO_PLAYER_LOG_ERROR, ("wiced_flac_get_metadata() Bad arg!\r\n"));
        return WICED_ERROR;
    }

    internal->log_level = log_level;

    return WICED_ERROR;

}

