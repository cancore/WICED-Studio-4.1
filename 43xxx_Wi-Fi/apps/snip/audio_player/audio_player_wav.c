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

/** @file Audio Player WAV Decode Routines
 *
 */

#include "wiced.h"
#include "audio_player.h"
#include "audio_player_wav.h"
#include "audio_player_util.h"

/******************************************************
 *                      Macros
 ******************************************************/

#ifndef le16toh
#define le16toh(x)      (x)
#endif

#ifndef le32toh
#define le32toh(x)      (x)
#endif

/******************************************************
 *                    Constants
 ******************************************************/

#define WAV_TAG_VALID               0x61EDBA16
#define WAV_TAG_INVALID             0xDEADBEEF

#define WAV_FORMAT_PCM              (0x0001)    /* Microsoft Pulse Code Modulation (PCM) format */
#define WAV_FORMAT_PCM_FLOAT        (0x0003)    /* PCM format, values are floats                */
#define WAV_FORMAT_EXTENSIBLE       (0xFFFE)    /* Extended data format                         */
#define IBM_FORMAT_MULAW            (0x0101)    /* IBM mu-law format                            */
#define IBM_FORMAT_ALAW             (0x0102)    /* IBM a-law format                             */
#define IBM_FORMAT_ADPCM            (0x0103)    /* IBM AVC Adaptive                             */

#define WAV_FMT_CHUNKID_OFFSET      (12)
#define WAV_FMT_CHUNKSIZE_OFFSET    (16)
#define WAV_FMT_DATA_OFFSET         (20)

#define AUDIO_PLAYER_DECODER_WAV_THREAD_PRIORITY       (WICED_DEFAULT_LIBRARY_PRIORITY - 1)
#define AUDIO_PLAYER_DECODER_WAV_STACK_SIZE            (8 * 1024)

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct
{
    uint32_t        tag;

    wiced_bool_t    header_parsed;
    wiced_bool_t    data_chunk_found;
    uint32_t        chunk_remaining;

    uint32_t        data_chunk_size;

    int             leftover_audio_bytes;
    uint8_t         leftover_audio_data[8];

    /*
     *  Audio format information.
     */

    uint16_t        num_channels;
    uint32_t        sample_rate;
    uint32_t        byte_rate;
    uint16_t        block_align;
    uint16_t        bits_per_sample;

    uint16_t        render_bits_per_sample;

    /*
     * Some housekeeping variables.
     */

    uint32_t        audio_frames_total;
    uint32_t        audio_frames_played;

    uint8_t         stack[AUDIO_PLAYER_DECODER_WAV_STACK_SIZE];
} wav_decoder_t;

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

static wiced_result_t find_data_chunk(wav_decoder_t* dec, data_buf_t* dbuf)
{
    uint8_t* ptr;
    int chunk_size;

    while (dbuf->bufused < dbuf->buflen)
    {
        if (dec->chunk_remaining > 0)
        {
            if (dbuf->buflen - dbuf->bufused > dec->chunk_remaining)
            {
                dbuf->bufused += dec->chunk_remaining;
                dec->chunk_remaining = 0;
            }
            else
            {
                dec->chunk_remaining -= (dbuf->buflen - dbuf->bufused);
                dbuf->bufused = dbuf->buflen;
            }
        }

        if (dbuf->bufused < dbuf->buflen)
        {
            if (dbuf->buflen - dbuf->bufused < 8)
            {
                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Parsing chunk header across packet boundaries not implemented\n");
                return WICED_ERROR;
            }

            ptr = &dbuf->buf[dbuf->bufused];
            if (ptr[0] == 'd' && ptr[1] == 'a' && ptr[2] == 't' && ptr[3] == 'a')
            {
                dec->data_chunk_size  = le32toh(*((uint32_t*)&ptr[4]));
                dec->data_chunk_found = WICED_TRUE;
                dbuf->bufused += 8;

                dec->audio_frames_total = dec->data_chunk_size / dec->block_align;

                break;
            }
            else
            {
                chunk_size = le32toh(*((uint32_t*)&ptr[4])) + 8;
                audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "Skipping chunk: %4.4s, size %d\n", ptr, chunk_size - 8);
                if (dbuf->bufused + chunk_size < dbuf->buflen)
                {
                    dbuf->bufused =+ chunk_size;
                }
                else
                {
                    dec->chunk_remaining = chunk_size - (dbuf->buflen - dbuf->bufused);
                    dbuf->bufused = dbuf->buflen;
                }
            }
        }
    }

    return WICED_SUCCESS;
}


static wiced_result_t parse_wav_header(wav_decoder_t* dec, data_buf_t* dbuf)
{
    uint8_t* ptr;
    uint16_t wav_format;
    int fmt_chunk_size;
    int offset;

    ptr = dbuf->buf;

    if (dbuf->buflen < WAV_FMT_DATA_OFFSET + 20)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "First packet not large enough to contain WAV header\n");
        return WICED_ERROR;
    }

    /*
     * Verify the RIFF header bytes.
     */

    if (ptr[0] != 'R' || ptr[1] != 'I' || ptr[2] != 'F' || ptr[3] != 'F')
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "No RIFF header\n");
        return WICED_ERROR;
    }

    /*
     * Verify the WAVE format.
     */

    if (ptr[8] != 'W' || ptr[9] != 'A' || ptr[10] != 'V' || ptr[11] != 'E')
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "No WAVE format\n");
        return WICED_ERROR;
    }

    /*
     * Verify the format chunk and get the chunk size.
     */

    if (ptr[WAV_FMT_CHUNKID_OFFSET]     != 'f' || ptr[WAV_FMT_CHUNKID_OFFSET + 1] != 'm' ||
        ptr[WAV_FMT_CHUNKID_OFFSET + 2] != 't' || ptr[WAV_FMT_CHUNKID_OFFSET + 3] != ' ')
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "No fmt chunk: %4.4s\n", &ptr[WAV_FMT_CHUNKID_OFFSET]);
        return WICED_ERROR;
    }

    fmt_chunk_size = le32toh(*((uint32_t*)&ptr[WAV_FMT_CHUNKSIZE_OFFSET]));

    /*
     * Extract the audio format information.
     */

    wav_format           = le16toh(*((uint16_t*)&ptr[WAV_FMT_DATA_OFFSET]));
    dec->num_channels    = le16toh(*((uint16_t*)&ptr[WAV_FMT_DATA_OFFSET + 2]));
    dec->sample_rate     = le32toh(*((uint16_t*)&ptr[WAV_FMT_DATA_OFFSET + 4]));
    dec->byte_rate       = le32toh(*((uint16_t*)&ptr[WAV_FMT_DATA_OFFSET + 8]));
    dec->block_align     = le16toh(*((uint16_t*)&ptr[WAV_FMT_DATA_OFFSET + 12]));
    dec->bits_per_sample = le16toh(*((uint16_t*)&ptr[WAV_FMT_DATA_OFFSET + 14]));

    if (wav_format != WAV_FORMAT_PCM && wav_format != WAV_FORMAT_EXTENSIBLE)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Unsupported wav file format: 0x%08x\n", wav_format);
        return WICED_ERROR;
    }

    /*
     * Verify that we can handle the format.
     */

    if (dec->num_channels != 2)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "%d channels not supported at this time\n", dec->num_channels);
        return WICED_ERROR;
    }

    if (dec->sample_rate != 44100 && dec->sample_rate != 48000 && dec->sample_rate != 96000 && dec->sample_rate != 192000)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Sample rate of %d not supported at this time\n", dec->sample_rate);
        return WICED_ERROR;
    }

    if (dec->bits_per_sample != 8 && dec->bits_per_sample != 16 && dec->bits_per_sample != 24 && dec->bits_per_sample != 32)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Bits per sample of %d not supported at this time\n", dec->bits_per_sample);
        return WICED_ERROR;
    }

    dec->render_bits_per_sample = dec->bits_per_sample == 24 ? 32 : dec->bits_per_sample;

    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "Wav file format:\n");
    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "    audio_format: 0x%04x\n", wav_format);
    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "    num channels: %u\n", dec->num_channels);
    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, " bits per sample: %u\n", dec->bits_per_sample);
    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "     sample rate: %lu\n", dec->sample_rate);
    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "     block align: %u\n", dec->block_align);

    /*
     * Advance past the fmt chunk.
     */

    offset = WAV_FMT_DATA_OFFSET + fmt_chunk_size;
    if (offset < dbuf->buflen)
    {
        dbuf->bufused = offset;
    }
    else
    {
        dbuf->bufused = dbuf->buflen;
        dec->chunk_remaining = WAV_FMT_DATA_OFFSET + fmt_chunk_size - dbuf->buflen;
    }

    return WICED_SUCCESS;
}


static wiced_result_t process_pcm_output(audio_player_t* player, wav_decoder_t* dec, data_buf_t* dbuf)
{
    audio_decoder_buffer_info_t buff_info;
    wiced_result_t result;
    uint8_t* inptr;
    uint8_t* outptr;
    int frames_to_copy;
    int output_framesize;
    int num_frames;
    int frames;
    int size;
    int old;
    int new;

    inptr = &dbuf->buf[dbuf->bufused];

    num_frames = (dbuf->buflen - dbuf->bufused + dec->leftover_audio_bytes) / dec->block_align;

    while (num_frames > 0 && player->decoder_done == WICED_FALSE)
    {
        result = audio_player_audio_render_buffer_get(player, &buff_info);
        if (result != WICED_SUCCESS)
        {
            /* we probably didn't get a buffer because the buffers are full */
            wiced_rtos_delay_milliseconds(1);
            continue;
        }

        output_framesize = (dec->render_bits_per_sample * dec->num_channels) / 8;
        if (output_framesize * num_frames <= buff_info.data_buf_size)
        {
            frames_to_copy = num_frames;
        }
        else
        {
            frames_to_copy = buff_info.data_buf_size / output_framesize;
        }

        outptr = (uint8_t*)buff_info.data_buf;
        switch (dec->bits_per_sample)
        {
            case 8:
            case 16:
            case 32:
                if (dec->leftover_audio_bytes != 0)
                {
                    memcpy(outptr, dec->leftover_audio_data, dec->leftover_audio_bytes);
                    outptr += dec->leftover_audio_bytes;
                    dec->leftover_audio_bytes = 0;
                }

                size = (frames_to_copy * output_framesize);
                memcpy(outptr, inptr, size);
                outptr        += size;
                inptr         += size;
                dbuf->bufused += size;
                break;

            case 24:
                frames = frames_to_copy;                    /* handle filling the frame after leftover bytes used */
                if (dec->leftover_audio_bytes)
                {
                    *outptr++ = 0;
                    for (old = 0; old < dec->leftover_audio_bytes; old++)
                    {
                        if (old == 3)
                        {
                            *outptr++ = 0;
                        }
                        *outptr++ = dec->leftover_audio_data[old];
                    }

                    for (new = dec->leftover_audio_bytes; new < 6; new++)
                    {
                        if (new == 3)
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

                while (frames > 0)
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
                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Decoder corruption\n");
                return WICED_ERROR;
        }

        dec->audio_frames_played += frames_to_copy;
        num_frames -= frames_to_copy;
        dbuf->bufused = (int)inptr - (int)dbuf->buf;

        buff_info.filled_data_offset = 0;
        buff_info.filled_data_length = frames_to_copy * output_framesize;

        while (player->decoder_done == WICED_FALSE)
        {
            result = audio_player_audio_render_buffer_push(player, &buff_info);
            if (result == WICED_SUCCESS)
            {
                break;
            }
            if (result == WICED_BADARG)
            {
                audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Error pushing audio buffer\n");
                return WICED_ERROR;
            }

            /*
             * Sleep to let the output buffers drain a little...
             */

            wiced_rtos_delay_milliseconds(1);
        }
    }

    if (dbuf->bufused < dbuf->buflen)
    {
        dec->leftover_audio_bytes = dbuf->buflen - dbuf->bufused;
        for (old = 0; old < dec->leftover_audio_bytes; old++)
        {
            dec->leftover_audio_data[old] = *inptr++;
        }

        dbuf->bufused = dbuf->buflen;
    }

    return WICED_SUCCESS;
}


static wiced_result_t process_wav_data(audio_player_t* player)
{
    wiced_audio_config_t audio_config;
    wav_decoder_t* dec;
    data_buf_t*    dbuf;

    dec = (wav_decoder_t*)player->decoder_handle;

    dbuf = &player->dbufs[player->dbuf_read_idx];
    if (!dbuf->inuse)
    {
        return WICED_SUCCESS;
    }

    if (dbuf->buflen == 0)
    {
        /*
         * No more data will be sent to us.
         */

        dbuf->buflen  = 0;
        dbuf->bufused = 0;
        dbuf->inuse   = 0;
        player->decoder_done = WICED_TRUE;
        return WICED_SUCCESS;
    }

    /*
     * Are we looking for the header?
     */

    if (dec->header_parsed == WICED_FALSE)
    {
        if (parse_wav_header(dec, dbuf) != WICED_SUCCESS)
        {
            player->decoder_done = WICED_TRUE;
            return WICED_ERROR;
        }

        dec->header_parsed = WICED_TRUE;
        wiced_time_get_time(&player->play_start_time);

        /*
         * Tell audio render about the audio format.
         */

        audio_config.bits_per_sample = dec->render_bits_per_sample;
        audio_config.channels        = dec->num_channels;
        audio_config.sample_rate     = dec->sample_rate;
        audio_config.frame_size      = dec->render_bits_per_sample * dec->num_channels / 8;
        audio_config.volume          = player->dct_app->volume;

        if (audio_render_configure(player->audio_render, &audio_config)!= WICED_SUCCESS)
        {
            audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "wiced_audio_render_configure() failed\n");
            player->decoder_done = WICED_TRUE;
            return WICED_ERROR;
        }
    }

    /*
     * Have we found the data chunk yet?
     */

    if (dec->data_chunk_found == WICED_FALSE)
    {
        if (find_data_chunk(dec, dbuf) != WICED_SUCCESS)
        {
            player->decoder_done = WICED_TRUE;
            return WICED_ERROR;
        }
    }

    if (dec->data_chunk_found && dbuf->bufused < dbuf->buflen)
    {
        if (process_pcm_output(player, dec, dbuf) != WICED_SUCCESS)
        {
            player->decoder_done = WICED_TRUE;
            return WICED_ERROR;
        }
    }

    /*
     * This buffer is done. Advance to the next one.
     */

    dbuf->buflen  = 0;
    dbuf->bufused = 0;
    dbuf->inuse   = 0;
    player->dbuf_read_idx = (player->dbuf_read_idx + 1) % AUDIO_PLAYER_DBUF_NUM_BUFS;

    /*
     * More data waiting to be processed?
     */

    dbuf = &player->dbufs[player->dbuf_read_idx];
    if (dbuf->inuse)
    {
        wiced_rtos_set_event_flags(&player->decoder_events, DECODER_EVENT_AUDIO_DATA);
    }

    return WICED_SUCCESS;
}


static void audio_player_wav_thread(uint32_t arg)
{
    audio_player_t*     player = (audio_player_t*)arg;
    uint32_t            events;
    wiced_result_t      result;

    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "audio_player wav decoder start\n");

    while (!player->decoder_done)
    {
        events = 0;

        result = wiced_rtos_wait_for_event_flags(&player->decoder_events, DECODER_ALL_EVENTS, &events, WICED_TRUE, WAIT_FOR_ANY_EVENT, WICED_WAIT_FOREVER);
        if (result != WICED_SUCCESS)
        {
            break;
        }

        if (events & DECODER_EVENT_AUDIO_DATA)
        {
            if (process_wav_data(player) != WICED_SUCCESS)
            {
                break;
            }
        }
    }

    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "audio_player wav decoder exit\n");

    wiced_rtos_set_event_flags(&player->player_events, PLAYER_EVENT_DECODER_THREAD_DONE);

    WICED_END_OF_CURRENT_THREAD();
}


wiced_result_t audio_player_wav_decoder_start(struct audio_player_s* player)
{
    wav_decoder_t* dec;
    wiced_result_t result;

    if (player == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_wav_decoder_start: Bad ARG\n");
        return WICED_BADARG;
    }

    /*
     * Allocate the internal decoder structure.
     */

    dec = (wav_decoder_t*)calloc(1, sizeof(wav_decoder_t));
    if (dec == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "Unable to allocate decoder structure\n");
        return WICED_ERROR;
    }

    player->decoder_handle = dec;
    dec->tag = WAV_TAG_VALID;

    /* Start decoder thread */
    player->decoder_done = WICED_FALSE;
    result = wiced_rtos_create_thread_with_stack(&player->decoder_thread, AUDIO_PLAYER_DECODER_WAV_THREAD_PRIORITY, "Wav Decoder",
                                                 audio_player_wav_thread, dec->stack, AUDIO_PLAYER_DECODER_WAV_STACK_SIZE, player);
    if (result != WICED_SUCCESS)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_wav_decoder_start: wiced_rtos_create_thread(wav) failed %d\n", result);
        return WICED_ERROR;
    }
    else
    {
        player->decoder_thread_ptr = &player->decoder_thread;
    }

    return result;
}


wiced_result_t audio_player_wav_decoder_stop(struct audio_player_s* player)
{
    audio_player_log_msg(AUDIO_PLAYER_LOG_INFO, "audio_player_wav_decoder_stop\n");

    if (player == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_wav_decoder_stop: Bad ARG\n");
        return WICED_BADARG;
    }

    player->decoder_done = WICED_TRUE;
    if (player->decoder_thread_ptr != NULL)
    {
        wiced_rtos_thread_force_awake(&player->decoder_thread);
        wiced_rtos_thread_join(&player->decoder_thread);
        wiced_rtos_delete_thread(&player->decoder_thread);
        player->decoder_thread_ptr = NULL;

        if (player->decoder_handle)
        {
            ((wav_decoder_t*)player->decoder_handle)->tag = WAV_TAG_INVALID;
            free(player->decoder_handle);
            player->decoder_handle = NULL;
        }
    }

    return WICED_SUCCESS;
}


wiced_result_t audio_player_wav_decoder_info(struct audio_player_s* player, audio_player_source_info_t* info)
{
    wav_decoder_t* dec;

    if (player == NULL || info == NULL)
    {
        audio_player_log_msg(AUDIO_PLAYER_LOG_ERR, "audio_player_wav_decoder_info: Bad ARG\n");
        return WICED_BADARG;
    }

    dec = (wav_decoder_t*)player->decoder_handle;
    if (dec && dec->tag == WAV_TAG_VALID)
    {
        info->source_total_samples  = dec->audio_frames_total;
        info->source_current_sample = dec->audio_frames_played;
        info->source_sample_rate    = dec->sample_rate;
        info->source_channels       = dec->num_channels;
        info->source_bps            = dec->bits_per_sample;
    }
    else
    {
        memset(info, 0, sizeof(audio_player_source_info_t));
    }

    return WICED_SUCCESS;
}
