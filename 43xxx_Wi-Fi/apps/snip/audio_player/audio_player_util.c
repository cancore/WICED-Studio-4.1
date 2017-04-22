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

/** @file Audio Player Utility Routines
 *
 */

#include "wiced.h"
#include "audio_player_util.h"

/******************************************************
 *                      Macros
 ******************************************************/

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
 *               Static Function Declarations
 ******************************************************/

static wiced_bool_t audio_player_log_initialized;
static AUDIO_PLAYER_LOG_LEVEL_T audio_player_log_level;

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t audio_player_log_init(AUDIO_PLAYER_LOG_LEVEL_T level)
{
    if (audio_player_log_initialized != WICED_FALSE)
    {
        return WICED_ERROR;
    }

    audio_player_log_initialized  = WICED_TRUE;
    audio_player_log_level        = level;

    return WICED_SUCCESS;
}

wiced_result_t audio_player_log_shutdown(void)
{
    if (audio_player_log_initialized == WICED_FALSE)
    {
        return WICED_ERROR;
    }

    audio_player_log_initialized = WICED_FALSE;

    return WICED_SUCCESS;
}

wiced_result_t audio_player_log_set_level(AUDIO_PLAYER_LOG_LEVEL_T level)
{
    if (audio_player_log_initialized == WICED_FALSE)
    {
        return WICED_ERROR;
    }

    if (level >= AUDIO_PLAYER_LOG_OFF && level < AUDIO_PLAYER_LOG_MAX)
    {
        audio_player_log_level = level;
    }

    return WICED_SUCCESS;
}

AUDIO_PLAYER_LOG_LEVEL_T audio_player_log_get_level(void)
{
    if (audio_player_log_initialized == WICED_FALSE)
    {
        return AUDIO_PLAYER_LOG_OFF;
    }

    return audio_player_log_level;
}

wiced_result_t audio_player_log_msg(AUDIO_PLAYER_LOG_LEVEL_T level, const char *fmt, ...)
{
    va_list args;

    if (audio_player_log_initialized == WICED_FALSE)
    {
        return WICED_ERROR;
    }

    if (audio_player_log_level == AUDIO_PLAYER_LOG_OFF || level > audio_player_log_level)
    {
        return WICED_SUCCESS;
    }

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    return WICED_SUCCESS;
}

wiced_result_t audio_player_printf(const char *fmt, ...)
{
    va_list args;

    if (audio_player_log_initialized == WICED_FALSE)
    {
        return WICED_ERROR;
    }

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    return WICED_SUCCESS;
}
