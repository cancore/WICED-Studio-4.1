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
 * Audio player types.
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    CHANNEL_MAP_NONE  = 0,          /* None or undefined    */
    CHANNEL_MAP_FL    = (1 << 0),   /* Front Left           */
    CHANNEL_MAP_FR    = (1 << 1),   /* Front Right          */
    CHANNEL_MAP_FC    = (1 << 2),   /* Front Center         */
    CHANNEL_MAP_LFE1  = (1 << 3),   /* LFE-1                */
    CHANNEL_MAP_BL    = (1 << 4),   /* Back Left            */
    CHANNEL_MAP_BR    = (1 << 5),   /* Back Right           */
    CHANNEL_MAP_FLC   = (1 << 6),   /* Front Left Center    */
    CHANNEL_MAP_FRC   = (1 << 7),   /* Front Right Center   */
    CHANNEL_MAP_BC    = (1 << 8),   /* Back Center          */
    CHANNEL_MAP_LFE2  = (1 << 9),   /* LFE-2                */
    CHANNEL_MAP_SIL   = (1 << 10),  /* Side Left            */
    CHANNEL_MAP_SIR   = (1 << 11),  /* Side Right           */
    CHANNEL_MAP_TPFL  = (1 << 12),  /* Top Front Left       */
    CHANNEL_MAP_TPFR  = (1 << 13),  /* Top Front Right      */
    CHANNEL_MAP_TPFC  = (1 << 14),  /* Top Front Center     */
    CHANNEL_MAP_TPC   = (1 << 15),  /* Top Center           */
    CHANNEL_MAP_TPBL  = (1 << 16),  /* Top Back Left        */
    CHANNEL_MAP_TPBR  = (1 << 17),  /* Top Back Right       */
    CHANNEL_MAP_TPSIL = (1 << 18),  /* Top Side Left        */
    CHANNEL_MAP_TPSIR = (1 << 19),  /* Top Side Right       */
    CHANNEL_MAP_TPBC  = (1 << 20),  /* Top Back Center      */
    CHANNEL_MAP_BTFC  = (1 << 21),  /* Bottom Front Center  */
    CHANNEL_MAP_BTFL  = (1 << 22),  /* Bottom Front Left    */
    CHANNEL_MAP_BTFR  = (1 << 23)   /* Bottom Front Right   */
} AUDIO_CHANNEL_MAP_T;

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct audio_player_source_info_s
{
    /* source info */
    uint64_t        source_current_sample;  /* last played sample                   */
    uint64_t        source_total_samples;   /* total samples in stream (if known)   */
    uint32_t        source_sample_rate;     /* sample rate                          */
    uint8_t         source_channels;        /* number of channels in source stream  */
    uint8_t         source_bps;             /* bits per sample in source stream     */
} audio_player_source_info_t;


typedef struct audio_decoder_buffer_info_s
{
    /* filled by Application    */
    void*           data_buf;           /* pointer to the LPCM buffer           */
    uint32_t        data_buf_size;      /* size of buffer (in bytes)            */
    void*           opaque;             /* for app reference                    */

    /* filled by Decoder */
    uint32_t        filled_data_offset;     /* filled offset from start of buffer   */
    uint32_t        filled_data_length;     /* filled length of data (in bytes)     */
} audio_decoder_buffer_info_t;


/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /*extern "C" */
#endif
