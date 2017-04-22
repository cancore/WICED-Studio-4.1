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

#include "wiced.h"
#include "wiced_log.h"
#include "button_manager.h"
#include "apollo_config.h"
#include "apollo_player.h"
#include "apollo_streamer.h"
#include "apollocore.h"
#include "apollo_cmd.h"
#include "apollo_cmd_sender.h"

#ifdef USE_UPNPAV
#include "upnp_av_render.h"
#include "bufmgr.h"
#endif

#if defined(OTA2_SUPPORT)
#include "apollo_ota2_support.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define APOLLO_TAG_VALID                      (0xCA11AB1E)
#define APOLLO_TAG_INVALID                    (0xDEADBEEF)

#define APOLLO_CONSOLE_COMMAND_MAX_LENGTH     (85)
#define APOLLO_CONSOLE_COMMAND_HISTORY_LENGTH (10)

#define APOLLO_TX_PACKET_BUFFER_COUNT         (128)
#define APOLLO_TX_AUDIO_RENDER_BUFFER_NODES   (200)
#define APOLLO_RX_AUDIO_RENDER_BUFFER_NODES   (128)

#define BUTTON_WORKER_STACK_SIZE              ( 4096 )
#define BUTTON_WORKER_QUEUE_SIZE              ( 4 )

#define NETWORK_INIT_TIMER_PERIOD_MSECS       (15000)

#define PLAYBACK_TIMER_PERIOD_MSECS           (1000)
#define PLAYBACK_TIMER_TIMEOUT_MSECS          (4 * 1000)

#define SECONDS_PER_MINUTE                    (60)
#define SECONDS_PER_HOUR                      (SECONDS_PER_MINUTE * 60)
#define SECONDS_PER_DAY                       (SECONDS_PER_HOUR * 24)   //86400

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct
{
    uint32_t                    tag;
    int                         stop_received;
    wiced_bool_t                initializing;
    wiced_bool_t                playback_active;
    wiced_bool_t                network_logging_active;

    wiced_time_t                playback_ended;

    wiced_event_flags_t         events;
    wiced_timer_t               timer;

    apollo_dct_collection_t     dct_tables;
    apollo_streamer_params_t    streamer_params;

    wiced_ip_address_t          mrtp_ip_address;

    wiced_bool_t                rmc_network_up;
    wiced_timer_t               network_timer;
    wiced_interface_t           interface;
    wiced_interface_t           upstream_interface;
    wiced_bool_t                upstream_interface_valid;
    wiced_udp_socket_t          report_socket;
    wiced_udp_socket_t*         report_socket_ptr;
    wiced_mac_t                 mac_address;                        /* Our mac address */

    apollo_player_ref           player_handle;
    apollo_streamer_ref         streamer_handle;
    void*                       cmd_handle;
    void*                       cmd_sender_handle;

    button_manager_t            button_manager;
    wiced_worker_thread_t       button_worker_thread;
    wiced_bool_t                button_gatt_launch_was_pressed;

    APOLLO_CMD_RTP_TIMING_T     rtp_timing_cmd;
    uint32_t                    rtp_timing_entries;

    uint64_t                    total_rtp_packets_received;
    uint64_t                    total_rtp_packets_dropped;
    uint64_t                    total_audio_frames_played;
    uint64_t                    total_audio_frames_dropped;
    uint64_t                    total_audio_frames_inserted;

#ifdef USE_AUDIO_DISPLAY
    wiced_thread_t display_thread;
    apollo_player_stats_t player_stats;
    char display_name[32];
    char display_info[32]; /* includes channel info */
#endif

#ifdef USE_UPNPAV
    upnpavrender_service_ref    upnpavrender_handle;
    objhandle_t                 upnpavrender_buf_pool;
    wiced_bool_t                upnpavrender_mute_enabled;
    uint8_t                     upnpavrender_volume;
    wiced_bool_t                upnpavrender_paused;
#endif

#if defined(OTA2_SUPPORT)
    apollo_ota2_service_info_t  ota2_info;
#endif

} apollo_app_t;

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif
