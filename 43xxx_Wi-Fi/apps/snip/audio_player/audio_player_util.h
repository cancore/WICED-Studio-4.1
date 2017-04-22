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

#include <stdarg.h>
#include "wiced_result.h"

/******************************************************
 *                      Macros
 ******************************************************/

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
    AUDIO_PLAYER_LOG_OFF = 0,
    AUDIO_PLAYER_LOG_ERR,
    AUDIO_PLAYER_LOG_WARNING,
    AUDIO_PLAYER_LOG_NOTICE,
    AUDIO_PLAYER_LOG_INFO,
    AUDIO_PLAYER_LOG_DEBUG0,
    AUDIO_PLAYER_LOG_DEBUG1,
    AUDIO_PLAYER_LOG_DEBUG2,
    AUDIO_PLAYER_LOG_DEBUG3,
    AUDIO_PLAYER_LOG_DEBUG4,

    AUDIO_PLAYER_LOG_MAX
} AUDIO_PLAYER_LOG_LEVEL_T;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t  audio_player_log_init(AUDIO_PLAYER_LOG_LEVEL_T level);
wiced_result_t  audio_player_log_shutdown(void);
wiced_result_t  audio_player_log_set_level(AUDIO_PLAYER_LOG_LEVEL_T level);
AUDIO_PLAYER_LOG_LEVEL_T audio_player_log_get_level(void);
wiced_result_t  audio_player_log_msg(AUDIO_PLAYER_LOG_LEVEL_T level, const char *fmt, ...);
wiced_result_t  audio_player_printf(const char *fmt, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif
