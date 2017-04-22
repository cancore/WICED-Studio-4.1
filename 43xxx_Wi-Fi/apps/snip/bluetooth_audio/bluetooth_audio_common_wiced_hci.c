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
#include "wiced_rtos.h"
#include "wiced_audio.h"
#include "bluetooth_audio.h"

#ifdef USE_AUDIO_PLL
#include "bluetooth_audio_pll_tuning.h"
#endif

/*****************************************************************************
**
**  Name:           bluetooth_audio_common_wiced_hci.c
**
**  Description:
**
*****************************************************************************/

/******************************************************
 *                      Macros
 ******************************************************/
#define VOLUME_CONVERSION(step,level,min)                    ((double)step*(double)level + min)

/******************************************************
 *                   Enumerations
 ******************************************************/
#ifdef USE_AUDIO_PLL
typedef enum
{
    AUDIO_PLL_CONSOLE_CMD_TARGET    = 0,
    AUDIO_PLL_CONSOLE_CMD_THRESHOLD,
    AUDIO_PLL_CONSOLE_CMD_LOG,
    AUDIO_PLL_CONSOLE_CMD_MAX,
} audio_pll_console_cmd_t;
#endif /* USE_AUDIO_PLL */

/******************************************************
 *                    Constants
 ******************************************************/
#define BT_AUDIO_DEFAULT_PERIOD_SIZE        ( 1024 )

/******************************************************
 *                 Type Definitions
 ******************************************************/
/******************************************************
 *                    Structures
 ******************************************************/
#ifdef USE_AUDIO_PLL
    typedef struct
    {
        char *cmd;
    } cmd_lookup_t;
#endif /* USE_AUDIO_PLL */

/******************************************************
 *               Function Definitions
 ******************************************************/
/******************************************************
 *               Variables Definitions
 ******************************************************/
bt_audio_context_t player;
#ifdef USE_AUDIO_PLL
static cmd_lookup_t command_lookup[AUDIO_PLL_CONSOLE_CMD_MAX] =
{
    { "target"    },
    { "threshold" },
    { "log"       },
};
#endif /* USE_AUDIO_PLL */

/******************************************************
 *               Function Definitions
 ******************************************************/
#ifdef USE_AUDIO_PLL
int audio_pll_console_command( int argc, char* argv[] )
{
    int i;
    int percent;
    int log_level;

    for ( i = 0; i < AUDIO_PLL_CONSOLE_CMD_MAX; ++i )
    {
        if ( strcmp(command_lookup[i].cmd, argv[0]) == 0 )
        {
            break;
        }
    }

    if ( i >= AUDIO_PLL_CONSOLE_CMD_MAX )
    {
        WPRINT_APP_ERROR ( ("Unrecognized command: %s\n", argv[0]) );
        return 0;
    }

    switch ( i )
    {
        case AUDIO_PLL_CONSOLE_CMD_TARGET:
            percent = atoi(argv[1]);
            if ( percent < 1 || percent > 100 )
            {
                WPRINT_APP_ERROR ( ("Buffer level target must be between 1% and 100% (inclusive)\n") );
            }
            else
            {
                player.audio_buffer_target_percent = percent;
            }
            break;

        case AUDIO_PLL_CONSOLE_CMD_THRESHOLD:
            percent = atoi(argv[1]);
            if ( percent < 1 || percent > 100 )
            {
                WPRINT_APP_ERROR ( ("Buffer playback threshold must be between 1% and 100% (inclusive)\n") );
            }
            else
            {
                player.audio_buffer_threshold_percent = percent;
            }
            break;

        case AUDIO_PLL_CONSOLE_CMD_LOG:
            log_level = atoi(argv[1]);
            if ( log_level < WICED_LOG_OFF || log_level > WICED_LOG_DEBUG4 )
            {
                WPRINT_APP_ERROR ( ("Log level must be between %d and %d (inclusive)\n", WICED_LOG_OFF, WICED_LOG_DEBUG4) );
            }
            else
            {
                wiced_log_set_level(log_level);
            }
            break;

        default:
            break;
    }

    return 0;
}
#endif /* USE_AUDIO_PLL */

wiced_bool_t is_bt_audio_player_initialized( void )
{
    return (wiced_bool_t)( player.state > BT_AUDIO_DEVICE_STATE_IDLE );
}

/*This is stub function*/
wiced_result_t  bt_audio_init_player( void )
{
    wiced_result_t result = WICED_SUCCESS;

    if(player.state == BT_AUDIO_DEVICE_STATE_UNINITIALIZED)
    {
        player.state = BT_AUDIO_DEVICE_STATE_IDLE;
    }
    WPRINT_APP_INFO ( ("bt_audio_init_player\n") );
    return result;
}

wiced_result_t bt_audio_configure_player( bt_audio_config_t* p_audio_config )
{
    wiced_result_t    result;
    wiced_audio_config_t wiced_audio_conf = {0, };
    //WPRINT_APP_INFO(("bt_audio_configure_player: INIT\n"));

    if(p_audio_config == NULL)
        return WICED_BADARG;

    if(player.state != BT_AUDIO_DEVICE_STATE_IDLE)
        return WICED_ERROR;

    /* Initialize and configure audio framework for needed audio format */
    result = wiced_audio_init(PLATFORM_DEFAULT_AUDIO_OUTPUT, &player.bluetooth_audio_session_handle, BT_AUDIO_DEFAULT_PERIOD_SIZE);
    if( result != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("bt_audio_configure_player:  Error Initializing Wiced Audio Framework[err: %d]\n",result) );
        return result;
    }

    result = wiced_audio_create_buffer(player.bluetooth_audio_session_handle, 4096, NULL, NULL );
    if( result != WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("bt_audio_configure_player: Error Registering Buffer to Wiced Audio Framework [err: %d]\n",result));
        return result;
    }

    memcpy(&player.bluetooth_audio_config, p_audio_config, sizeof(bt_audio_config_t));
    WPRINT_APP_INFO(("bt_audio_configure_player: config sample_rate:%u channels:%d bps:%d\n", (unsigned int) player.bluetooth_audio_config.sample_rate,
                                             (int)player.bluetooth_audio_config.channels, (int)player.bluetooth_audio_config.bits_per_sample));

    wiced_audio_conf.sample_rate     = player.bluetooth_audio_config.sample_rate;
    wiced_audio_conf.channels        = player.bluetooth_audio_config.channels;
    wiced_audio_conf.bits_per_sample = player.bluetooth_audio_config.bits_per_sample;
    wiced_audio_conf.frame_size      = player.bluetooth_audio_config.frame_size;

    result = wiced_audio_configure( player.bluetooth_audio_session_handle, &wiced_audio_conf );
    if( result != WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("bt_audio_configure_player: Error configuring Wiced Audio framework [err: %d]\n",result));
        return result;
    }
    player.state = BT_AUDIO_DEVICE_STATE_CONFIGURED;

    /* Update the Volume */
    result = bt_audio_update_player_volume( player.bluetooth_audio_config.volume );

    return result;
}

wiced_result_t bt_audio_stop_player( void )
{
    if(player.state <= BT_AUDIO_DEVICE_STATE_IDLE)
        return WICED_SUCCESS;

    //Send stop event to reset the player state to IDLE
    wiced_rtos_set_event_flags(&player.events, BT_AUDIO_EVENT_STOP_PLAYER );
    wiced_rtos_get_semaphore(&player.wait_for_cmd_completion, 34); //max time player can wait for buffer @8K

    return WICED_SUCCESS;
}
wiced_result_t bt_audio_update_player_volume( uint8_t level )
{
    wiced_result_t result = WICED_ERROR;
    double min_volume_db = 0.0, max_volume_db = 0.0, volume_db, step_db;

    if(level > BT_AUDIO_VOLUME_MAX)
        level = BT_AUDIO_DEFAULT_VOLUME;

    if ( is_bt_audio_player_initialized() != WICED_TRUE )
    {
        WPRINT_APP_INFO (("bt_audio_update_player_volume: Player not initialized\n"));
         return result;
    }

    result = wiced_audio_get_volume_range( player.bluetooth_audio_session_handle, &min_volume_db, &max_volume_db );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("bt_audio_update_player_volume: wiced_audio_get_volume_range failed\n"));
        return result;
    }

    step_db = ( double )( max_volume_db - min_volume_db ) /( BT_AUDIO_VOLUME_MAX);
    volume_db = ( double ) VOLUME_CONVERSION( step_db, level, min_volume_db );

    return wiced_audio_set_volume( player.bluetooth_audio_session_handle, volume_db );
}

/*This is stub function*/
void bt_audio_player_task(uint32_t args)
{
    UNUSED_PARAMETER(args);
}

/*This is stub function*/
wiced_result_t bt_audio_write_to_decoder_queue(bt_audio_codec_data_t* audio)
{
    UNUSED_PARAMETER(audio);

    return WICED_SUCCESS;
}

/*This is stub function*/
wiced_result_t bt_audio_decoder_context_init( void )
{
    return WICED_SUCCESS;
}

/*This is stub function*/
wiced_result_t bt_audio_reset_decoder_config( void )
{
    return WICED_SUCCESS;
}

/*This is stub function*/
wiced_result_t bt_audio_configure_decoder(wiced_bt_a2dp_codec_info_t* decoder_config)
{
    UNUSED_PARAMETER(decoder_config);
    return WICED_SUCCESS;
}

/*This is stub function*/
void bt_audio_decoder_task( uint32_t arg )
{
    UNUSED_PARAMETER(arg);
}
