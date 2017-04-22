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

/** @file Apollo audio application.
 */

#include <unistd.h>
#include <malloc.h>

#include "apollo_context.h"
#include "platform_audio.h"
#include "command_console.h"
#include "console_wl.h"
#ifndef APOLLO_NO_WIFI_COMMANDS
#include "wifi/command_console_wifi.h"
#endif
#include "dct/command_console_dct.h"
#include "apollo_debug.h"
#include "apollo_wl_utils.h"
#include "apollo_tracex.h"
#include "apollo_report.h"

#ifdef USE_UPNPAV
#include "apollo_upnpavrender.h"
#endif

#ifdef USE_AUDIO_DISPLAY
#include "audio_display.h"
#endif

#ifndef APOLLO_NO_BT
#include "apollo_bt_service.h"
#include "apollo_config_gatt_server.h"
#endif

/******************************************************
 *                      Macros
 ******************************************************/

#if defined(OTA2_SUPPORT)
#define OTA2_SUPPORT_COMMANDS   \
    { (char*) "get_update",     apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Get OTA2 Update (extract on reboot)" }, \
    { (char*) "timed_update",   apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Start OTA2 timed Update (extract on reboot)" }, \
    { (char*) "stop_update",    apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Stop  OTA2 timed Update (extract on reboot)" }, \
    { (char*) "ota2_status",    apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Current OTA2 status (log_level > 0)" }, \

#else
#define OTA2_SUPPORT_COMMANDS
#endif

#define APOLLO_CONSOLE_COMMANDS \
    { (char*) "exit",           apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Exit application" }, \
    { (char*) "start",          apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Start source/sink" }, \
    { (char*) "stop",           apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Stop source/sink" }, \
    { (char*) "volume",         apollo_console_command, 1, NULL, NULL, (char *)"", (char *)"Set the audio volume (0-100)" }, \
    { (char*) "config",         apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Display / change config values" }, \
    { (char*) "log",            apollo_console_command, 1, NULL, NULL, (char *)"", (char *)"Set the logging level" }, \
    { (char*) "ascu_time",      apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Display current time" }, \
    { (char*) "stats",          apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Display player stats" }, \
    { (char*) "netlog",         apollo_console_command, 1, NULL, NULL, (char *)"", (char *)"Turn network logging on|off" }, \
    { (char*) "rmc_rate",       apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Get/Set RMC Tx Rate" }, \
    { (char*) "rmc_status",     apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Display rmc_status info" }, \
    { (char*) "reboot",         apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Reboot the platform" }, \
    { (char*) "reset_counters", apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Reset apollo and WiFi persistent counters" }, \
    { (char*) "rx_counters",    apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Print persistent WiFi receive counters" }, \
    { (char*) "tx_counters",    apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Print persitent WiFi transmit counters" }, \
    { (char*) "memory",         apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Display memory stats" }, \
    { (char*) "rmc_down",       apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Bring down RMC Network" },  \
    { (char*) "rmc_up",         apollo_console_command, 0, NULL, NULL, (char *)"", (char *)"Bring up RMC Network" }, \
    OTA2_SUPPORT_COMMANDS

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    APOLLO_CONSOLE_CMD_EXIT = 0,
    APOLLO_CONSOLE_CMD_START,
    APOLLO_CONSOLE_CMD_STOP,
    APOLLO_CONSOLE_CMD_VOLUME,
    APOLLO_CONSOLE_CMD_CONFIG,
    APOLLO_CONSOLE_CMD_LOG,
    APOLLO_CONSOLE_CMD_TIME,
    APOLLO_CONSOLE_CMD_STATS,
    APOLLO_CONSOLE_CMD_NETLOG,
    APOLLO_CONSOLE_CMD_RMC_RATE,
    APOLLO_CONSOLE_CMD_RMC_STATUS,
    APOLLO_CONSOLE_CMD_REBOOT,
    APOLLO_CONSOLE_CMD_RESET_COUNTERS,
    APOLLO_CONSOLE_CMD_GET_RX_COUNTERS,
    APOLLO_CONSOLE_CMD_GET_TX_COUNTERS,
    APOLLO_CONSOLE_CMD_MEMORY,
    APOLLO_CONSOLE_CMD_RMC_DOWN,
    APOLLO_CONSOLE_CMD_RMC_UP,

#if defined(OTA2_SUPPORT)
    APOLLO_CONSOLE_CMD_GET_UPDATE,
    APOLLO_CONSOLE_CMD_START_TIMED_UPDATE,
    APOLLO_CONSOLE_CMD_STOP_TIMED_UPDATE,
    APOLLO_CONSOLE_CMD_OTA2_STATUS,
#endif

    APOLLO_CONSOLE_CMD_MAX,
} APOLLO_CONSOLE_CMDS_T;

typedef enum {
    APOLLO_EVENT_SHUTDOWN           = (1 << 0),
    APOLLO_EVENT_START              = (1 << 1),
    APOLLO_EVENT_STOP               = (1 << 2),
    APOLLO_EVENT_AUTOSTOP           = (1 << 3),
    APOLLO_EVENT_CONFIG_GATT        = (1 << 4),
    APOLLO_EVENT_RTP_TIMING         = (1 << 5),
    APOLLO_EVENT_TIMER              = (1 << 6),
    APOLLO_EVENT_NETWORK_TIMER      = (1 << 7),

    APOLLO_EVENT_RELOAD_DCT_WIFI      = (1 << 8),
    APOLLO_EVENT_RELOAD_DCT_NETWORK   = (1 << 9),
    APOLLO_EVENT_RELOAD_DCT_BLUETOOTH = (1 << 10),

    APOLLO_EVENT_RMC_DOWN             = (1 << 11),
    APOLLO_EVENT_RMC_UP               = (1 << 12),

#if defined(OTA2_SUPPORT)
    APOLLO_EVENT_GET_UPDATE           = (1 << 20),
    APOLLO_EVENT_START_TIMED_UPDATE   = (1 << 21),
    APOLLO_EVENT_STOP_TIMED_UPDATE    = (1 << 22),
    APOLLO_EVENT_OTA2_STATUS          = (1 << 23),
#endif

} APOLLO_EVENTS_T;

#define APOLLO_ALL_EVENTS       (-1)

typedef enum
{
    /* Launch BTLE GATT configuration process */
    ACTION_CONFIG_GATT_LAUNCH
} app_action_t;

typedef enum
{
    CONFIG_GATT_BUTTON
} application_button_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct
{
    char *cmd;
    uint32_t event;
} cmd_lookup_t;

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

static int  apollo_console_command  (int argc, char *argv[]);
static void app_button_event_handler( const button_manager_button_t* button, button_manager_event_t event, button_manager_button_state_t state );

static wiced_result_t apollo_start_network_services(apollo_app_t* apollo);
static wiced_result_t apollo_stop_network_services(apollo_app_t* apollo);

static wiced_result_t pno_app_result_callback(wiced_scan_handler_result_t* malloced_scan_result);

#if defined(OTA2_SUPPORT)
wiced_bool_t apollo_is_service_active(apollo_app_t* apollo);
wiced_result_t apollo_ota2_callback(void* session_id, wiced_ota2_service_status_t status, uint32_t value, void* opaque );
#endif

/******************************************************
 *               Variables Definitions
 ******************************************************/

static wiced_time_t apollo_start_time;

static char apollo_command_buffer[APOLLO_CONSOLE_COMMAND_MAX_LENGTH];
static char apollo_command_history_buffer[APOLLO_CONSOLE_COMMAND_MAX_LENGTH * APOLLO_CONSOLE_COMMAND_HISTORY_LENGTH];

const command_t apollo_command_table[] =
{
    APOLLO_CONSOLE_COMMANDS
    WL_COMMANDS
#ifndef APOLLO_NO_WIFI_COMMANDS
    WIFI_COMMANDS
#endif
    DCT_CONSOLE_COMMANDS
    APOLLO_TRACEX_COMMANDS
    CMD_TABLE_END
};

static cmd_lookup_t command_lookup[APOLLO_CONSOLE_CMD_MAX] =
{
    { "exit",           APOLLO_EVENT_SHUTDOWN   },
    { "start",          APOLLO_EVENT_START      },
    { "stop",           APOLLO_EVENT_STOP       },
    { "volume",         0                       },
    { "config",         0                       },
    { "log",            0                       },
    { "ascu_time",      0                       },
    { "stats",          0                       },
    { "netlog",         0                       },
    { "rmc_rate",       0                       },
    { "rmc_status",     0                       },
    { "reboot",         0                       },
    { "reset_counters", 0                       },
    { "rx_counters",    0                       },
    { "tx_counters",    0                       },
    { "memory",         0                       },
    { "rmc_down",       APOLLO_EVENT_RMC_DOWN   },
    { "rmc_up",         APOLLO_EVENT_RMC_UP     },

#if defined(OTA2_SUPPORT)
    { "get_update",     APOLLO_EVENT_GET_UPDATE         },
    { "timed_update",   APOLLO_EVENT_START_TIMED_UPDATE },
    { "stop_update",    APOLLO_EVENT_STOP_TIMED_UPDATE  },
    { "ota2_status",    APOLLO_EVENT_OTA2_STATUS  },
#endif
};

static const wiced_ip_setting_t ap_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS( 255,255,255,  0 ) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
};

static const wiced_button_manager_configuration_t button_manager_configuration =
{
    .short_hold_duration     = 500  * MILLISECONDS,
    .debounce_duration       = 150  * MILLISECONDS,

    .event_handler           = app_button_event_handler,
};

/* Static button configuration */
static const wiced_button_configuration_t button_configurations[] =
{
#if (WICED_PLATFORM_BUTTON_COUNT > 0)
    [ CONFIG_GATT_BUTTON ] = { PLATFORM_BUTTON_1, BUTTON_CLICK_EVENT , ACTION_CONFIG_GATT_LAUNCH },
#endif
};

/* Button objects for the button manager */
static button_manager_button_t buttons[] =
{
#if (WICED_PLATFORM_BUTTON_COUNT > 0)
    [ CONFIG_GATT_BUTTON ] = { &button_configurations[ CONFIG_GATT_BUTTON ] },
#endif
};

static apollo_app_t *g_apollo;

/******************************************************
 *               Function Definitions
 ******************************************************/

static void apollo_link_up(void)
{
    wiced_log_msg(WICED_LOG_ERR, "STA Link Up\n");
}

static void apollo_link_down(void)
{
    apollo_app_t *apollo= g_apollo;

    if (apollo == NULL || (apollo->tag != APOLLO_TAG_VALID && !apollo->initializing))
    {
        wiced_log_msg(WICED_LOG_ERR, "%s: Bad app structure\r\n", __FUNCTION__);
        return ;
    }

    /* If the STA interface isn't being used directly by RMC/apollo and the link goes down,
     * then start PNO to wait for the AP/SSID, re-establish link, and send us an event.
     */
    if (apollo->interface == WICED_AP_INTERFACE)
    {
        wiced_result_t result;
        wiced_ssid_t ssid;
        wiced_security_t security;

        bzero(&ssid, sizeof(ssid));
        ssid.length = apollo->dct_tables.dct_wifi->stored_ap_list[0].details.SSID.length;
        memcpy( ssid.value, apollo->dct_tables.dct_wifi->stored_ap_list[0].details.SSID.value, ssid.length);
        security = apollo->dct_tables.dct_wifi->stored_ap_list[0].details.security;

        wiced_log_msg(WICED_LOG_ERR, "STA Link Down, Start PNO with SSID %s\n", ssid.value);

        result = wiced_wifi_pno_start( &ssid, security, pno_app_result_callback, NULL );
        if ( result != WICED_SUCCESS ) {
            wiced_log_msg(WICED_LOG_ERR, "Unable to start PNO, error %d\n", result);
        }
    } else {
        wiced_log_msg(WICED_LOG_INFO, "STA Link Down\n");
    }
}

static wiced_result_t pno_app_result_callback( wiced_scan_handler_result_t* malloced_scan_result )
{
    wiced_result_t ret;

    /* must be done prior to free below -- this callback function owns the memory */
    malloc_transfer_to_curr_thread(malloced_scan_result);

    wiced_log_msg(WICED_LOG_INFO, "PNO callback\n");

    /* when network is detected, force a reassoc via wiced_network_up */
    if ( wiced_network_is_up(WICED_STA_INTERFACE) == WICED_FALSE) {
        ret = wiced_network_up(WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL);

        if (ret == WICED_SUCCESS) {
            wiced_log_msg(WICED_LOG_INFO, "%s: Re-join succeeded; clearing pno.\n", __FUNCTION__);
        } else {
            wiced_log_msg(WICED_LOG_ERR, "%s: Re-join failed. PNO still running.\n", __FUNCTION__);
        }
    }

    if (wiced_network_is_up( WICED_STA_INTERFACE) == WICED_TRUE) {
        /* PNO has detected a registered network, disable it now.
        * Note: this also clears event registration
        */
        ret = wiced_wifi_pno_stop();
        if (ret != WICED_SUCCESS) {
            wiced_log_msg(WICED_LOG_ERR, "%s: Can't disable PNO %d\n", ret, __FUNCTION__);
        }
    }

    free( malloced_scan_result );

    wiced_log_msg(WICED_LOG_INFO, "Exit PNO event\n");
    return WICED_SUCCESS;
}


static wiced_interface_t apollo_get_rmc_interface(apollo_dct_collection_t* dct_tables)
{
    wiced_interface_t interface;

    interface = WICED_STA_INTERFACE;

    if ((dct_tables != NULL) && (dct_tables->dct_app != NULL))
    {
        if ( (dct_tables->dct_app->rmc_info.bss_type == WICED_BSS_TYPE_INFRASTRUCTURE) &&
             (dct_tables->dct_app->apollo_role == APOLLO_ROLE_SOURCE) &&
             (dct_tables->dct_app->rmc_info.use_external_ap == 0))
        {
            interface = WICED_AP_INTERFACE;
        }
    }

    return interface;
}

static void network_timer_callback(void *arg)
{
    apollo_app_t* apollo = (apollo_app_t *)arg;

    if (apollo && apollo->tag == APOLLO_TAG_VALID)
    {
        wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_NETWORK_TIMER);
    }
}


static void timer_callback(void *arg)
{
    apollo_app_t* apollo = (apollo_app_t *)arg;

    if (apollo && apollo->tag == APOLLO_TAG_VALID)
    {
        wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_TIMER);
    }
}


static void app_button_event_handler( const button_manager_button_t* button, button_manager_event_t event, button_manager_button_state_t state )
{
    if ( ( g_apollo != NULL ) && ( g_apollo->button_gatt_launch_was_pressed != WICED_TRUE ) &&
         ( button->configuration->application_event == ACTION_CONFIG_GATT_LAUNCH ) &&
         ( event == BUTTON_CLICK_EVENT ) )
    {
        g_apollo->button_gatt_launch_was_pressed = WICED_TRUE;
    }

    return;
}


static wiced_result_t apollo_button_handler_init(apollo_app_t* apollo)
{
    wiced_result_t result;

    result = wiced_rtos_create_worker_thread( &apollo->button_worker_thread, WICED_DEFAULT_WORKER_PRIORITY, BUTTON_WORKER_STACK_SIZE, BUTTON_WORKER_QUEUE_SIZE );
    wiced_action_jump_when_not_true( result == WICED_SUCCESS, _exit, wiced_log_msg(WICED_LOG_ERR, "wiced_rtos_create_worker_thread() failed !\r\n") );
    result = button_manager_init( &apollo->button_manager, &button_manager_configuration, &apollo->button_worker_thread, buttons, ARRAY_SIZE( buttons ) );
    wiced_action_jump_when_not_true( result == WICED_SUCCESS, _exit, wiced_log_msg(WICED_LOG_ERR, "button_manager_init() failed !\r\n") );

 _exit:
    return result;
}


static void apollo_button_handler_deinit(apollo_app_t* apollo)
{
    button_manager_deinit( &apollo->button_manager );
    wiced_rtos_delete_worker_thread( &apollo->button_worker_thread );
}


static int apollo_log_output_handler(WICED_LOG_LEVEL_T level, char *logmsg)
{
    write(STDOUT_FILENO, logmsg, strlen(logmsg));

    return 0;
}


static wiced_result_t apollo_log_get_time(wiced_time_t* time)
{
    uint32_t master_secs;
    uint32_t master_nanosecs;
    uint32_t local_secs;
    uint32_t local_nanosecs;
    wiced_time_t now;
    wiced_result_t result;

    /*
     * Get the current time.
     */

    result = wiced_time_read_8021as(&master_secs, &master_nanosecs, &local_secs, &local_nanosecs);

    if (result == WICED_SUCCESS)
    {
        *time = (master_secs * MILLISECONDS_PER_SECOND) + (master_nanosecs / NANOSECONDS_PER_MILLISECOND);
    }
    else
    {
        result = wiced_time_get_time(&now);
        *time  = now - apollo_start_time;
    }

    return result;
}


int apollo_console_command(int argc, char *argv[])
{
    apollo_player_stats_t stats;
    wiced_udp_socket_t socket;
    wiced_counters_t counters;
    uint32_t master_secs;
    uint32_t master_nanosecs;
    uint32_t local_secs;
    uint32_t local_nanosecs;
    uint32_t event = 0;
    int log_level;
    int result;
    int i;
    apollo_volume_t volume =
    {
        .volume       = 0,
        .caching_mode = (APOLLO_CACHE_TO_MEMORY|APOLLO_STORE_TO_NVRAM),
    };

    wiced_log_msg(WICED_LOG_DEBUG0, "Apollo console received command: %s\r\n", argv[0]);

    if (g_apollo == NULL || (g_apollo->tag != APOLLO_TAG_VALID && !g_apollo->initializing))
    {
        wiced_log_msg(WICED_LOG_ERR, "Bad app structure\r\n");
        return ERR_CMD_OK;
    }

    /*
     * Lookup the command in our table.
     */

    for (i = 0; i < APOLLO_CONSOLE_CMD_MAX; ++i)
    {
        if (strcmp(command_lookup[i].cmd, argv[0]) == 0)
            break;
    }

    if (i >= APOLLO_CONSOLE_CMD_MAX)
    {
        wiced_log_msg(WICED_LOG_ERR, "Unrecognized command: %s\r\n", argv[0]);
        return ERR_CMD_OK;
    }

    switch (i)
    {
        case APOLLO_CONSOLE_CMD_EXIT:
        case APOLLO_CONSOLE_CMD_START:
        case APOLLO_CONSOLE_CMD_STOP:
        case APOLLO_CONSOLE_CMD_RMC_DOWN:
        case APOLLO_CONSOLE_CMD_RMC_UP:
            event = command_lookup[i].event;
            break;

        case APOLLO_CONSOLE_CMD_VOLUME:
            volume.volume = atoi(argv[1]);
            if (g_apollo->player_handle)
            {
                apollo_player_set_volume(g_apollo->player_handle, volume.volume);
            }
            else if (g_apollo->streamer_handle)
            {
                apollo_streamer_set_volume(g_apollo->streamer_handle, volume.volume);

                /*
                 * Since we're the streamer, we want to tell all the sinks about the volume change.
                 */

                if (g_apollo->cmd_sender_handle != NULL)
                {
                    apollo_cmd_sender_command(g_apollo->cmd_sender_handle, APOLLO_CMD_SENDER_COMMAND_VOLUME, WICED_TRUE, NULL, &volume);
                }
            }
            break;

        case APOLLO_CONSOLE_CMD_CONFIG:
            apollo_set_config(&g_apollo->dct_tables, argc, argv);
            break;

        case APOLLO_CONSOLE_CMD_LOG:
            log_level = atoi(argv[1]);
            wiced_log_printf("Setting new log level to %d (0 - off, %d - max debug)\r\n", log_level, WICED_LOG_DEBUG4);
            wiced_log_set_level(log_level);
            break;

        case APOLLO_CONSOLE_CMD_TIME:
            result = wiced_time_read_8021as(&master_secs, &master_nanosecs, &local_secs, &local_nanosecs);

            if (result == WICED_SUCCESS)
            {
                wiced_time_t wtime;

                wiced_time_get_time(&wtime);
                wiced_log_printf("Master time = %u.%09u secs\n", (unsigned int)master_secs, (unsigned int)master_nanosecs);
                wiced_log_printf("Current local time = %u.%09u secs\n", (unsigned int)local_secs, (unsigned int)local_nanosecs);
                wiced_log_printf("wtime is %d\n", (int)wtime);
            }
            else
            {
                wiced_log_printf("Error returned from wiced_time_read_8021as\n");
            }
            break;

        case APOLLO_CONSOLE_CMD_STATS:
            if (g_apollo->player_handle)
            {
                if (apollo_player_ioctl(g_apollo->player_handle, APOLLO_PLAYER_IOCTL_GET_SOCKET, &socket) == WICED_SUCCESS)
                {
                    ULONG packets_sent;
                    ULONG bytes_sent;
                    ULONG packets_received;
                    ULONG bytes_received;
                    ULONG packets_queued;
                    ULONG receive_packets_dropped;
                    ULONG checksum_errors;

                    if (nx_udp_socket_info_get(&socket.socket, &packets_sent, &bytes_sent, &packets_received, &bytes_received,
                                               &packets_queued, &receive_packets_dropped, &checksum_errors) == NX_SUCCESS)
                    {
                        wiced_log_printf("Player UDP: packets sent %u, bytes sent %u, packets received %u, bytes received %u\n", packets_sent, bytes_sent, packets_received, bytes_received);
                        wiced_log_printf("            packets queued %u, receive packets dropped %u, checksum errors %u\n", packets_queued, receive_packets_dropped, checksum_errors);
                    }
                }

                if (apollo_player_ioctl(g_apollo->player_handle, APOLLO_PLAYER_IOCTL_GET_STATS, &stats) == WICED_SUCCESS)
                {
                    wiced_log_printf("Player stats: packets received %lu, packets dropped %lu\n", stats.rtp_packets_received, stats.rtp_packets_dropped);
                    wiced_log_printf("              total bytes received %lu, audio bytes received %lu\n", (uint32_t)stats.total_bytes_received, (uint32_t)stats.audio_bytes_received);
                    wiced_log_printf("              payload size %lu\n", stats.payload_size);
                    wiced_log_printf("              audio frames played %lu, audio frames dropped %lu, audio frames inserted %lu\n",
                                     (uint32_t)stats.audio_frames_played, (uint32_t)stats.audio_frames_dropped, (uint32_t)stats.audio_frames_inserted);
                }
                else
                {
                    memset(&stats, 0, sizeof(stats));
                }

                wiced_log_printf("Apollo stats:  total packets received %lu, total packets dropped %lu\n",
                                 (uint32_t)(g_apollo->total_rtp_packets_received + stats.rtp_packets_received),
                                 (uint32_t)(g_apollo->total_rtp_packets_dropped + stats.rtp_packets_dropped));
                wiced_log_printf("               total audio frames played %lu, total audio dropped %lu, total frames inserted %lu\n",
                                 (uint32_t)(g_apollo->total_audio_frames_played + stats.audio_frames_played),
                                 (uint32_t)(g_apollo->total_audio_frames_dropped + stats.audio_frames_dropped),
                                 (uint32_t)(g_apollo->total_audio_frames_inserted + stats.audio_frames_inserted));
            }
            break;

        case APOLLO_CONSOLE_CMD_NETLOG:
            if (!strcmp(argv[1], "on"))
            {
                if (!g_apollo->network_logging_active)
                {
                    if (apollo_debug_create_tcp_data_socket(NULL, 0) == WICED_SUCCESS)
                    {
                        wiced_log_set_platform_output(apollo_debug_tcp_log_output_handler);
                        g_apollo->network_logging_active = WICED_TRUE;
                        {
                            uint32_t master_secs2;
                            uint32_t master_nanosecs2;

                            wiced_time_read_8021as(&master_secs, &master_nanosecs, &local_secs, &local_nanosecs);
                            wiced_log_msg(WICED_LOG_ERR, "Network logging enabled\n");
                            wiced_time_read_8021as(&master_secs2, &master_nanosecs2, &local_secs, &local_nanosecs);
                            local_secs = master_secs2 - master_secs;
                            if (master_nanosecs2 < master_nanosecs)
                            {
                                local_secs--;
                                local_nanosecs = 1000000000 - master_nanosecs + master_nanosecs2;
                            }
                            else
                            {
                                local_nanosecs = master_nanosecs2 - master_nanosecs;
                            }
                            wiced_log_msg(WICED_LOG_ERR, "Log time delay is %lu.%09lu\n", local_secs, local_nanosecs);
                        }
                    }
                }
                else
                {
                    wiced_log_msg(WICED_LOG_ERR, "Network logging is already active\n");
                }
            }
            else if (g_apollo->network_logging_active)
            {
                wiced_log_set_platform_output(apollo_log_output_handler);
                apollo_debug_close_tcp_data_socket();
                g_apollo->network_logging_active = WICED_FALSE;
            }
            break;

        case APOLLO_CONSOLE_CMD_RMC_RATE:
        {
            uint32_t rspec = -1;
            char mybuf[32];
            /* Get Rate */
            if (!argv[1]) {
                if (apollo_wl_rmc_rate(&rspec, WLC_BAND_5G) < 0) {
                    wiced_log_msg(WICED_LOG_ERR, "RMC_RATE: Cannot get rmc_rate\r\n");
                    break;
                }
                apollo_wl_rate_print(mybuf, sizeof(mybuf), rspec);
                wiced_log_printf("%s\n", mybuf);
            } else {
                /* set rate */
                if (!strncmp("-h", argv[1], 2)) {
                    if (argv[2]) {
                        rspec = WL_RSPEC_ENCODE_HT | atoi(argv[2]);
                    }
                } else {
                    /* Legacy rates are stored as number of .5 Mbit units */
                    rspec = WL_RSPEC_ENCODE_RATE | (2 * atoi(argv[1]));
                }
                if (rspec != -1) {
                    if (apollo_wl_rmc_rate(&rspec, WLC_BAND_5G) < 0) {
                        wiced_log_msg(WICED_LOG_ERR, "RMC_RATE: Cannot set rmc_rate\r\n");
                        break;
                    }
                }
            }

            break;
        }

        case APOLLO_CONSOLE_CMD_RMC_STATUS:
        {
            wl_relmcast_status_t rmc_status;

            if (apollo_wl_get_rmc_status(&rmc_status) < 0)
            {
                wiced_log_msg(WICED_LOG_ERR, "Error returned from apollo_wl_get_rmc_status\r\n");
                break;
            }
            wiced_log_printf("%d peers associated\r\n", rmc_status.num);
            for (i = 0; i < rmc_status.num; i++)
            {
                wiced_log_printf("  %02x:%02x:%02x:%02x:%02x:%02x  %3d  %c %c %c %c\r\n",
                                 rmc_status.clients[i].addr.octet[0], rmc_status.clients[i].addr.octet[1],
                                 rmc_status.clients[i].addr.octet[2], rmc_status.clients[i].addr.octet[3],
                                 rmc_status.clients[i].addr.octet[4], rmc_status.clients[i].addr.octet[5],
                                 rmc_status.clients[i].rssi,
                                 rmc_status.clients[i].flag & WL_RMC_FLAG_ACTIVEACKER ? 'A' : ' ',
                                 rmc_status.clients[i].flag & WL_RMC_FLAG_MASTER_TX   ? 'M' : ' ',
                                 rmc_status.clients[i].flag & WL_RMC_FLAG_RELMCAST    ? 'R' : ' ',
                                 rmc_status.clients[i].flag & WL_RMC_FLAG_INBLACKLIST ? 'B' : ' ');
            }
            break;
        }

        case APOLLO_CONSOLE_CMD_REBOOT:
            wiced_framework_reboot();
            break;

        case APOLLO_CONSOLE_CMD_RESET_COUNTERS:
            if (wwd_reset_statistics_counters() == WWD_SUCCESS)
            {
                g_apollo->total_rtp_packets_dropped = 0;
            }
            else
            {
                wiced_log_printf("%s: command failed).\n", argv[0]);
            }
            break;

        case APOLLO_CONSOLE_CMD_GET_RX_COUNTERS:
            wwd_get_counters(&counters);
            wiced_log_printf("Good frames RX %u, RX errors %u. total RTP dropped %u\n",
                             counters.rxframe, counters.rxerror, (uint32_t)g_apollo->total_rtp_packets_dropped);
            wiced_log_printf("Bad PLCP %u, CRS glitch %u, Bad FCS %u, RX Fifo Overflow %u.\n",
                             counters.rxbadplcp, counters.rxcrsglitch, counters.rxbadfcs, counters.rxf0ovfl);
            break;

        case APOLLO_CONSOLE_CMD_GET_TX_COUNTERS:
            wwd_get_counters(&counters);
            wiced_log_printf("Good frames TX %u, TX retries %u, TX failures %u, TX errors %u.\n",
                             counters.txframe, counters.txretry, counters.txfail, counters.txerror);
            break;

        case APOLLO_CONSOLE_CMD_MEMORY:
            {
                extern unsigned char _heap[];
                extern unsigned char _eheap[];
                extern unsigned char *sbrk_heap_top;
                volatile struct mallinfo mi = mallinfo();

                wiced_log_printf("sbrk heap size:    %7lu\n", (uint32_t)_eheap - (uint32_t)_heap);
                wiced_log_printf("sbrk current free: %7lu \n", (uint32_t)_eheap - (uint32_t)sbrk_heap_top);

                wiced_log_printf("malloc allocated:  %7d\n", mi.uordblks);
                wiced_log_printf("malloc free:       %7d\n", mi.fordblks);

                wiced_log_printf("\ntotal free memory: %7lu\n", mi.fordblks + (uint32_t)_eheap - (uint32_t)sbrk_heap_top);
            }
            break;

#if defined(OTA2_SUPPORT)
        case APOLLO_CONSOLE_CMD_GET_UPDATE:
            if (argc > 1)
            {
                strlcpy(g_apollo->ota2_info.update_uri, argv[1], sizeof(g_apollo->ota2_info.update_uri) );
            }
            else
            {
                wiced_log_printf("Use app_dct->ota2_default_update_uri\r\n");
                strlcpy(g_apollo->ota2_info.update_uri, g_apollo->dct_tables.dct_app->ota2_default_update_uri, sizeof(g_apollo->ota2_info.update_uri) );
            }
            event = command_lookup[i].event;
            break;

        case APOLLO_CONSOLE_CMD_START_TIMED_UPDATE:
            if (argc > 1)
            {
                strlcpy(g_apollo->ota2_info.update_uri, argv[1], sizeof(g_apollo->ota2_info.update_uri) );
            }
            else
            {
                wiced_log_printf("Use app_dct->ota2_default_update_uri\r\n");
                strlcpy(g_apollo->ota2_info.update_uri, g_apollo->dct_tables.dct_app->ota2_default_update_uri, sizeof(g_apollo->ota2_info.update_uri) );
            }
            event = command_lookup[i].event;
            break;

        case APOLLO_CONSOLE_CMD_STOP_TIMED_UPDATE:
        case APOLLO_CONSOLE_CMD_OTA2_STATUS:
            event = command_lookup[i].event;
            break;
#endif

        default:
            wiced_log_printf("%s: command not found.\n", argv[0]);
            break;
    }

    if (event)
    {
        /*
         * Send off the event to the main loop.
         */

        wiced_rtos_set_event_flags(&g_apollo->events, event);
    }

    return ERR_CMD_OK;
}


static int apollo_player_event_callback(apollo_player_ref handle, void* userdata, APOLLO_PLAYER_EVENT_T event, void* arg)
{
    apollo_app_t* apollo = (apollo_app_t*)userdata;
    const platform_audio_device_info_t* audio_device;
    apollo_audio_format_t* format;
    apollo_seq_err_t* seq_err;
    apollo_player_stats_t* stats;
    uint32_t num_lost;

    if (apollo == NULL || apollo->tag != APOLLO_TAG_VALID)
    {
        return -1;
    }

    switch (event)
    {
        case APOLLO_PLAYER_EVENT_PLAYBACK_STARTED:
            format = (apollo_audio_format_t*)arg;
            apollo->playback_active = WICED_TRUE;
            audio_device = platform_audio_device_get_info_by_id(apollo->dct_tables.dct_app->audio_device_tx);
            wiced_log_msg(WICED_LOG_INFO, "Playback started using device %s\r\n", audio_device ? audio_device->device_name : "");
            wiced_log_msg(WICED_LOG_INFO, "Audio format is %u channels, %lu kHz, %u bps\r\n",
                          format->num_channels, format->sample_rate, format->bits_per_sample);
            if (wiced_rtos_is_timer_running(&apollo->timer) != WICED_SUCCESS)
            {
                wiced_rtos_start_timer(&apollo->timer);
            }
            break;

        case APOLLO_PLAYER_EVENT_PLAYBACK_STOPPED:
            stats = (apollo_player_stats_t*)arg;
            apollo->playback_active = WICED_FALSE;
            wiced_time_get_time(&apollo->playback_ended);

            /* Get non-persistent player stat(s) to update persistent Apollo stat(s) */
            apollo->total_rtp_packets_received  += stats->rtp_packets_received;
            apollo->total_rtp_packets_dropped   += stats->rtp_packets_dropped;
            apollo->total_audio_frames_played   += stats->audio_frames_played;
            apollo->total_audio_frames_dropped  += stats->audio_frames_dropped;
            apollo->total_audio_frames_inserted += stats->audio_frames_inserted;
            wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_AUTOSTOP);
            wiced_log_msg(WICED_LOG_NOTICE, "%lu bytes received in %lu packets\r\n", (uint32_t)stats->total_bytes_received, stats->rtp_packets_received);
            break;

        case APOLLO_PLAYER_EVENT_SEQ_ERROR:
            seq_err = (apollo_seq_err_t*)arg;
            if (seq_err->cur_seq > seq_err->last_valid_seq)
            {
                num_lost = seq_err->cur_seq - seq_err->last_valid_seq - 1;
            }
            else
            {
                num_lost = seq_err->cur_seq + (RTP_MAX_SEQ_NUM + 1) - seq_err->last_valid_seq - 1;
            }
            wiced_log_msg(WICED_LOG_WARNING, "SEQ error - cur %u, last %u (lost %u)\r\n", seq_err->cur_seq, seq_err->last_valid_seq, num_lost);

#ifdef USE_AUDIO_DISPLAY
            apollo_player_ioctl(apollo->player_handle, APOLLO_PLAYER_IOCTL_GET_STATS, &apollo->player_stats);
            if (audio_display_get_footer_options() & FOOTER_OPTION_APOLLO_RX)
            {
                audio_display_footer_update_time_info(apollo->player_stats.rtp_packets_received, apollo->player_stats.rtp_packets_dropped);
            }
#endif
            break;

        case APOLLO_PLAYER_EVENT_RTP_TIMING_FULL:
            wiced_log_msg(WICED_LOG_ERR, "RTP timing log is full\n");
            break;
    }

    return 0;
}

static int apollo_streamer_event_notification(apollo_streamer_ref handle, void* user_context, apollo_streamer_event_t event, apollo_streamer_event_data_t* event_data)
{
    int rc = 0;
    apollo_app_t* apollo = (apollo_app_t*)user_context;
    wiced_action_jump_when_not_true((apollo != NULL) && (apollo->tag == APOLLO_TAG_VALID), _exit, rc = -1);

    switch (event)
    {
        case APOLLO_STREAMER_EVENT_CONNECTED:
#ifdef USE_AUDIO_DISPLAY
            audio_display_header_update_options(BATTERY_ICON_IS_VISIBLE | BATTERY_ICON_SHOW_PERCENT | SIGNAL_STRENGTH_IS_VISIBLE | BLUETOOTH_IS_CONNECTED);
#endif
            wiced_log_msg(WICED_LOG_ERR, "[%s] device is connected.\n", apollo_source_type_get_text(apollo->streamer_params.source_type));
            break;

        case APOLLO_STREAMER_EVENT_DISCONNECTED:
#ifdef USE_AUDIO_DISPLAY
            audio_display_header_update_options(BATTERY_ICON_IS_VISIBLE | BATTERY_ICON_SHOW_PERCENT | SIGNAL_STRENGTH_IS_VISIBLE);
#endif
            wiced_log_msg(WICED_LOG_ERR, "[%s] device is disconnected.\n", apollo_source_type_get_text(apollo->streamer_params.source_type));
            break;

        case APOLLO_STREAMER_EVENT_PLAYBACK_STARTED:
            wiced_log_msg(WICED_LOG_INFO, "[%s] playback has started.\n", apollo_source_type_get_text(apollo->streamer_params.source_type));
            break;

        case APOLLO_STREAMER_EVENT_PLAYBACK_STOPPED:
            wiced_log_msg(WICED_LOG_INFO, "[%s] playback has stopped.\n", apollo_source_type_get_text(apollo->streamer_params.source_type));
            break;

        case APOLLO_STREAMER_EVENT_VOLUME_CHANGED:
            break;

        case APOLLO_STREAMER_EVENT_PLAYBACK_STATUS:
        {
            uint32_t position = event_data->playback.position_msecs/1000;
            uint32_t duration = event_data->playback.duration_msecs/1000;

#ifdef USE_AUDIO_DISPLAY
            audio_display_footer_update_time_info(position, duration);
#endif
            wiced_log_msg(WICED_LOG_INFO, "%s track: %lu / %lu secs\n",
                          apollo_source_type_get_text(apollo->streamer_params.source_type), position, duration);
        }
            break;

        case APOLLO_STREAMER_EVENT_TRACK_METADATA:
#ifdef USE_AUDIO_DISPLAY
            audio_display_footer_update_song_info((char *)event_data->metadata.title_utf8_str, (char *)event_data->metadata.artist_utf8_str);
#endif
            wiced_log_msg(WICED_LOG_INFO, "[%s] track: [%s - %s]\n",
                          apollo_source_type_get_text(apollo->streamer_params.source_type),
                          event_data->metadata.artist_utf8_str, event_data->metadata.title_utf8_str);

            if (apollo->cmd_sender_handle != NULL)
            {
                apollo_cmd_metadata_t data_buffer;

                memset(&data_buffer, 0, sizeof(apollo_cmd_metadata_t));

                data_buffer.type                                      = APOLLO_METADATA_TITLE | APOLLO_METADATA_ARTIST;
                data_buffer.data_length[APOLLO_METADATA_TITLE_INDEX]  = 1 + strlen((char *)event_data->metadata.title_utf8_str);
                data_buffer.data[APOLLO_METADATA_TITLE_INDEX]         = (uint8_t *)event_data->metadata.title_utf8_str;
                data_buffer.data_length[APOLLO_METADATA_ARTIST_INDEX] = 1 + strlen((char *)event_data->metadata.artist_utf8_str);
                data_buffer.data[APOLLO_METADATA_ARTIST_INDEX]        = (uint8_t *)event_data->metadata.artist_utf8_str;

                apollo_cmd_sender_command(g_apollo->cmd_sender_handle, APOLLO_CMD_SENDER_COMMAND_METADATA, WICED_TRUE, NULL, &data_buffer);
            }
            break;

        case APOLLO_STREAMER_EVENT_BUFFER_RELEASED:
#ifdef USE_UPNPAV
            apollo_upnpavrender_release_buffer(apollo, event_data);
#endif
            break;

        default:
            break;
    }

 _exit:
    return rc;
}

#ifndef APOLLO_NO_BT
wiced_result_t gatt_event_callback(apollo_config_gatt_event_t event, apollo_config_gatt_server_dct_t *dct,  void *user_context)
{
    size_t         len;
    wiced_result_t result = WICED_SUCCESS;
    apollo_app_t*  apollo = (apollo_app_t *)user_context;

    switch ( event)
    {
        case APOLLO_CONFIG_GATT_EVENT_DCT_READ:
            dct->is_configured   = 1;
            dct->mode            = apollo->dct_tables.dct_app->apollo_role;
            dct->spk_channel_map = apollo->dct_tables.dct_app->speaker_channel;
            dct->spk_vol         = apollo->dct_tables.dct_app->volume;
            dct->src_type        = apollo->dct_tables.dct_app->source_type;
            dct->security        = apollo->dct_tables.dct_app->rmc_info.security;
            strlcpy(dct->nw_ssid_name, (char *)apollo->dct_tables.dct_app->rmc_info.ssid_name, sizeof(dct->nw_ssid_name));
            strlcpy(dct->nw_pass_phrase, (char *)apollo->dct_tables.dct_app->rmc_info.security_key, sizeof(dct->nw_pass_phrase));
            break;

        case APOLLO_CONFIG_GATT_EVENT_DCT_WRITE:
            apollo->dct_tables.dct_app->is_configured       = dct->is_configured;
            apollo->dct_tables.dct_app->apollo_role         = dct->mode;
            apollo->dct_tables.dct_app->speaker_channel     = dct->spk_channel_map;
            apollo->dct_tables.dct_app->volume              = dct->spk_vol;
            apollo->dct_tables.dct_app->source_type         = dct->src_type;
            apollo->dct_tables.dct_app->rmc_info.security   = dct->security;

            len = strlen(dct->nw_ssid_name);
            len = MIN(len, sizeof(dct->nw_ssid_name));
            len = MIN(len, sizeof(apollo->dct_tables.dct_app->rmc_info.ssid_name));
            memcpy(apollo->dct_tables.dct_app->rmc_info.ssid_name, dct->nw_ssid_name, len);
            apollo->dct_tables.dct_app->rmc_info.ssid_length = len;

            len = strlen(dct->nw_pass_phrase);
            len = MIN(len, sizeof(dct->nw_pass_phrase));
            len = MIN(len, sizeof(apollo->dct_tables.dct_app->rmc_info.security_key));
            memcpy(apollo->dct_tables.dct_app->rmc_info.security_key, dct->nw_pass_phrase, len);
            apollo->dct_tables.dct_app->rmc_info.security_key_length = len;

            if(apollo->dct_tables.dct_app->apollo_role == APOLLO_ROLE_SINK)
            {
                strlcpy((char *)apollo->dct_tables.dct_app->speaker_name,
                    dct->spk_name, sizeof(apollo->dct_tables.dct_app->speaker_name));
            }
#ifdef WICED_DCT_INCLUDE_BT_CONFIG
            if(apollo->dct_tables.dct_app->apollo_role == APOLLO_ROLE_SOURCE)
            {
                strlcpy((char*)apollo->dct_tables.dct_bt->bluetooth_device_name, dct->nw_ssid_name, sizeof(apollo->dct_tables.dct_bt->bluetooth_device_name));
            }
            else
            {
                strlcpy((char*)apollo->dct_tables.dct_bt->bluetooth_device_name, dct->spk_name, sizeof(apollo->dct_tables.dct_bt->bluetooth_device_name));
            }
#endif
            break;

        case APOLLO_CONFIG_GATT_EVENT_DCT_WRITE_COMPLETED:
            wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_CONFIG_GATT);
            break;

        default:
            result = WICED_ERROR;
            break;
    }

    return result;
}
#endif


static wiced_result_t apollo_stream_start(apollo_app_t* apollo)
{
    wiced_result_t result;
    int num_pool_packets;

    if (apollo->dct_tables.dct_app->burst_length == 0)
    {
        num_pool_packets = 20;
    }
    else if (apollo->dct_tables.dct_app->burst_length != APOLLO_STREAMER_BURST_AUTO_SLC)
    {
        num_pool_packets = apollo->dct_tables.dct_app->burst_length * 2 * 3;
    }
    else
    {
        num_pool_packets = RTP_AUDIO_BL_MAX_LENGTH * 2 * 2;
    }

    /*
     * Set streaming options (most from DCT)
     */

    memset(&apollo->streamer_params, 0, sizeof(apollo->streamer_params));

    apollo->streamer_params.user_context        = apollo;
    apollo->streamer_params.event_cb            = apollo_streamer_event_notification;
    apollo->streamer_params.source_type         = (apollo_audio_source_type_t)apollo->dct_tables.dct_app->source_type;
    apollo->streamer_params.iface               = apollo_get_rmc_interface(&apollo->dct_tables);
    apollo->streamer_params.port                = apollo->dct_tables.dct_app->rtp_port;
    apollo->streamer_params.num_pool_packets    = num_pool_packets;
    apollo->streamer_params.num_packets         = APOLLO_TX_PACKET_BUFFER_COUNT;
    apollo->streamer_params.max_payload_size    = apollo->dct_tables.dct_app->payload_size;
    apollo->streamer_params.burst_length        = apollo->dct_tables.dct_app->burst_length;
    apollo->streamer_params.shuffle_length      = apollo->dct_tables.dct_app->shuffle_length;
    apollo->streamer_params.audio_device_rx     = apollo->dct_tables.dct_app->audio_device_rx;
    apollo->streamer_params.audio_device_tx     = apollo->dct_tables.dct_app->audio_device_tx;
    apollo->streamer_params.input_sample_rate   = apollo->dct_tables.dct_app->input_sample_rate;
    apollo->streamer_params.input_sample_size   = apollo->dct_tables.dct_app->input_sample_size;
    apollo->streamer_params.input_channel_count = apollo->dct_tables.dct_app->input_channel_count;

    SET_IPV4_ADDRESS(apollo->streamer_params.clientaddr, GET_IPV4_ADDRESS(apollo->dct_tables.dct_app->clientaddr));

    apollo->streamer_params.volume              = apollo->dct_tables.dct_app->volume;
    apollo->streamer_params.buffer_nodes        = APOLLO_TX_AUDIO_RENDER_BUFFER_NODES;
    apollo->streamer_params.buffer_ms           = apollo->dct_tables.dct_app->buffering_ms;
    apollo->streamer_params.threshold_ms        = apollo->dct_tables.dct_app->threshold_ms;
    apollo->streamer_params.clock_enable        = apollo->dct_tables.dct_app->clock_enable;
    apollo->streamer_params.pll_tuning_enable   = apollo->dct_tables.dct_app->pll_tuning_enable;
    apollo->streamer_params.pll_tuning_ppm_max  = apollo->dct_tables.dct_app->pll_tuning_ppm_max;
    apollo->streamer_params.pll_tuning_ppm_min  = apollo->dct_tables.dct_app->pll_tuning_ppm_min;

    /*
     * Fire off the apollo streamer.
     */

    result = apollo_streamer_init(&apollo->streamer_params, &apollo->streamer_handle);

    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Unable to initialize apollo_streamer\r\n");
    }

#ifdef USE_UPNPAV
    if (apollo->streamer_params.source_type == APOLLO_AUDIO_SOURCE_EXTERNAL)
    {
        result = apollo_upnpavrender_start(apollo);
        if (result != WICED_SUCCESS)
        {
            wiced_log_msg(WICED_LOG_ERR, "apollo_upnpavrender_start() failed !\r\n");
        }

    }
#endif

    return result;
}


static void apollo_stream_stop(apollo_app_t* apollo)
{
    if (apollo == NULL || apollo->tag != APOLLO_TAG_VALID || apollo->streamer_handle == NULL)
    {
        return;
    }

#ifdef USE_UPNPAV
    if (apollo->streamer_params.source_type == APOLLO_AUDIO_SOURCE_EXTERNAL)
    {
        apollo_upnpavrender_stop(apollo);
    }
#endif

    apollo_streamer_deinit(apollo->streamer_handle);
    apollo->streamer_handle = NULL;
}


static wiced_result_t apollo_play_start(apollo_app_t* apollo)
{
    apollo_player_params_t params;

    params.event_cb          = apollo_player_event_callback;
    params.userdata          = apollo;
    params.interface         = apollo_get_rmc_interface(&apollo->dct_tables);
    params.rtp_port          = apollo->dct_tables.dct_app->rtp_port;
    params.channel           = apollo->dct_tables.dct_app->speaker_channel;
    params.volume            = apollo->dct_tables.dct_app->volume;
    params.device_id         = apollo->dct_tables.dct_app->audio_device_tx;
    params.buffer_nodes      = APOLLO_RX_AUDIO_RENDER_BUFFER_NODES;
    params.buffer_ms         = apollo->dct_tables.dct_app->buffering_ms;
    params.threshold_ms      = apollo->dct_tables.dct_app->threshold_ms;
    params.clock_enable      = apollo->dct_tables.dct_app->clock_enable;
    params.pll_tuning_enable = apollo->dct_tables.dct_app->pll_tuning_enable;

    /*
     * Fire off the apollo player.
     */

    apollo->player_handle = apollo_player_init(&params);
    if (apollo->player_handle == NULL)
    {
        wiced_log_msg(WICED_LOG_ERR, "Unable to initialize apollo_player\r\n");
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}


static void apollo_play_stop(apollo_app_t* apollo)
{
    if (apollo == NULL || apollo->tag != APOLLO_TAG_VALID || apollo->player_handle == NULL)
    {
        return;
    }

    apollo_player_deinit(apollo->player_handle);
    apollo->player_handle = NULL;
}


static void apollo_service_start(apollo_app_t* apollo)
{
    wiced_result_t result;

    if (apollo->dct_tables.dct_app->apollo_role == APOLLO_ROLE_SINK)
    {
        if (apollo->player_handle == NULL)
        {
            result = apollo_play_start(apollo);
            if (result != WICED_SUCCESS)
            {
                wiced_log_msg(WICED_LOG_ERR, "Error creating apollo player service\r\n");
            }
        }
        else
        {
            wiced_log_msg(WICED_LOG_ERR, "Apollo player service already running\r\n");
        }
    }
    else
    {
        if (apollo->streamer_handle == NULL)
        {
            result = apollo_stream_start(apollo);
            if (result != WICED_SUCCESS)
            {
                wiced_log_msg(WICED_LOG_ERR, "Error creating apollo streamer service\r\n");
            }
        }
        else
        {
            wiced_log_msg(WICED_LOG_ERR, "Apollo streamer service already running\r\n");
        }
    }
}


static void apollo_service_stop(apollo_app_t* apollo)
{
    if (apollo->dct_tables.dct_app->apollo_role == APOLLO_ROLE_SINK)
    {
        apollo_play_stop(apollo);
    }
    else
    {
        apollo_stream_stop(apollo);
    }
}


static void apollo_handle_timer(apollo_app_t* apollo)
{
    apollo_player_stats_t stats;
    apollo_report_stats_t* rstats;
    apollo_report_msg_t* msg;
    wiced_ip_address_t broadcast_addr;
    wiced_time_t curtime;
    wiced_packet_t* packet;
    wiced_result_t result;
    uint16_t available_space;
    uint32_t* reportbuf;
    int32_t rssi;
    int len;

    if (apollo->playback_active == WICED_FALSE || apollo->player_handle == NULL)
    {
        /*
         * See if it's been long enough since playback stopped that
         * we can go ahead and cancel the timer.
         */

        wiced_time_get_time(&curtime);
        if (curtime > apollo->playback_ended + PLAYBACK_TIMER_TIMEOUT_MSECS)
        {
            wiced_rtos_stop_timer(&apollo->timer);
            return;
        }
    }

    if (apollo_player_ioctl(apollo->player_handle, APOLLO_PLAYER_IOCTL_GET_STATS, &stats) != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Error querying player stats\n");
        memset(&stats, 0, sizeof(apollo_player_stats_t));
    }

    /*
     * Get a UDP packet from the packet pool.
     */

    len = ((sizeof(apollo_report_msg_t) + sizeof(apollo_report_stats_t) + 3) / 4) * 4;
    if ((result = wiced_packet_create_udp(&apollo->report_socket, len, &packet, (uint8_t**)&reportbuf, &available_space)) != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Unable to allocate report packet\n");
        return;
    }
    wiced_packet_set_data_end(packet, (uint8_t*)reportbuf + len);

    /*
     * Construct the report message.
     */

    msg = (apollo_report_msg_t*)reportbuf;
    msg->magic       = APOLLO_REPORT_MAGIC_TAG;
    msg->version     = APOLLO_REPORT_VERSION;
    msg->mac_addr[0] = apollo->mac_address.octet[0];
    msg->mac_addr[1] = apollo->mac_address.octet[1];
    msg->mac_addr[2] = apollo->mac_address.octet[2];
    msg->mac_addr[3] = apollo->mac_address.octet[3];
    msg->mac_addr[4] = apollo->mac_address.octet[4];
    msg->mac_addr[5] = apollo->mac_address.octet[5];
    msg->msg_type    = APOLLO_REPORT_MSG_STATS;
    msg->msg_length  = sizeof(apollo_report_stats_t);

    rstats = (apollo_report_stats_t*)msg->msg_data;

    wwd_wifi_get_rssi(&rssi);
    rstats->version              = APOLLO_REPORT_STATS_VERSION;
    rstats->rssi                 = (int16_t)rssi;
    rstats->speaker_channel      = apollo->dct_tables.dct_app->speaker_channel;
    rstats->rtp_packets_received = apollo->total_rtp_packets_received + stats.rtp_packets_received;
    rstats->rtp_packets_dropped  = apollo->total_rtp_packets_dropped  + stats.rtp_packets_dropped;
    rstats->audio_frames_played  = apollo->total_audio_frames_played  + stats.audio_frames_played;
    rstats->audio_frames_dropped = apollo->total_audio_frames_dropped + stats.audio_frames_dropped;

    /*
     * And send it off.
     */

    SET_IPV4_ADDRESS(broadcast_addr, MAKE_IPV4_ADDRESS(255, 255, 255, 255));
    result = wiced_udp_send(&apollo->report_socket, &broadcast_addr, APOLLO_REPORT_PORT, packet);

    if (result != WICED_SUCCESS)
    {
        /*
         * Something went wrong. We need to release the packet back to the pool.
         */

        wiced_log_msg(WICED_LOG_ERR, "Error sending report packet\n");
        wiced_packet_delete(packet);
    }
}


static void apollo_handle_rtp_timing_cmd(apollo_app_t* apollo)
{
    apollo_player_timing_stats_t rtp_timing;
    uint32_t i;

    if (apollo->player_handle == NULL)
    {
        /*
         * RTP timing commands only valid for sink.
         */

        return;
    }

    switch (apollo->rtp_timing_cmd)
    {
        case APOLLO_CMD_RTP_TIMING_INIT:
            if (apollo_player_ioctl(apollo->player_handle, APOLLO_PLAYER_IOCTL_RTP_TIMING_INIT, (void*)apollo->rtp_timing_entries) != WICED_SUCCESS)
            {
                wiced_log_msg(WICED_LOG_ERR, "Error initializing RTP timing log with %u entries\n", apollo->rtp_timing_entries);
            }
            break;

        case APOLLO_CMD_RTP_TIMING_START:
            if (apollo_player_ioctl(apollo->player_handle, APOLLO_PLAYER_IOCTL_RTP_TIMING_START, NULL) != WICED_SUCCESS)
            {
                wiced_log_msg(WICED_LOG_ERR, "Error starting RTP timing log\n");
            }
            break;

        case APOLLO_CMD_RTP_TIMING_STOP:
            if (apollo_player_ioctl(apollo->player_handle, APOLLO_PLAYER_IOCTL_RTP_TIMING_STOP, NULL) != WICED_SUCCESS)
            {
                wiced_log_msg(WICED_LOG_ERR, "Error stopping RTP timing log\n");
            }
            break;

        case APOLLO_CMD_RTP_TIMING_RESET:
            if (apollo_player_ioctl(apollo->player_handle, APOLLO_PLAYER_IOCTL_RTP_TIMING_RESET, NULL) != WICED_SUCCESS)
            {
                wiced_log_msg(WICED_LOG_ERR, "Error resetting RTP timing log\n");
            }
            break;

        case APOLLO_CMD_RTP_TIMING_DUMP:
            if (apollo_player_ioctl(apollo->player_handle, APOLLO_PLAYER_IOCTL_RTP_TIMING_GET, &rtp_timing) != WICED_SUCCESS)
            {
                wiced_log_msg(WICED_LOG_ERR, "Error getting RTP timing log data\n");
                break;
            }

            /*
             * We're going to be outputting a lot of data to the log. Stop playback while we do so.
             */

            apollo_service_stop(apollo);

            wiced_log_printf("RTP Timing Log Dump:\n");
            for (i = 0; i < rtp_timing.number_entries; i++)
            {
                wiced_log_printf("  Seq: %04u  arrival time: %d.%09d\n", rtp_timing.rtp_entries[i].sequence_number,
                                 (int)(rtp_timing.rtp_entries[i].timestamp / NANOSECONDS_PER_SECOND), (int)(rtp_timing.rtp_entries[i].timestamp % NANOSECONDS_PER_SECOND));
            }
            break;

        default:
            wiced_log_msg(WICED_LOG_ERR, "Unknown RTP timing log command: %d\n", apollo->rtp_timing_cmd);
            break;
    }

    apollo->rtp_timing_cmd = APOLLO_CMD_RTP_TIMING_NONE;
}

static void apollo_mainloop(apollo_app_t* apollo)
{
    wiced_result_t result;
    uint32_t events;

    wiced_log_msg(WICED_LOG_INFO, "Begin apollo mainloop\r\n");

    /*
     * If auto start is set and the network is up then begin by sending ourselves a start event.
     */

    if (apollo->dct_tables.dct_app->auto_start && apollo->rmc_network_up)
    {
        wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_START);
    }

    while (apollo->tag == APOLLO_TAG_VALID)
    {
        events = 0;
        result = wiced_rtos_wait_for_event_flags(&apollo->events, APOLLO_ALL_EVENTS, &events,
                                                 WICED_TRUE, WAIT_FOR_ANY_EVENT, WICED_WAIT_FOREVER);
        if (result != WICED_SUCCESS)
        {
            continue;
        }

        if (events & APOLLO_EVENT_SHUTDOWN)
        {
            break;
        }

        if (events & APOLLO_EVENT_START)
        {
            apollo->stop_received = 0;
            apollo_service_start(apollo);
        }

        if (events & (APOLLO_EVENT_STOP | APOLLO_EVENT_AUTOSTOP))
        {
            apollo_service_stop(apollo);

            if (events & APOLLO_EVENT_STOP)
            {
                apollo->stop_received = 1;
            }
            else if (!apollo->stop_received)
            {
                /*
                 * Start playing again automatically.
                 */

                wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_START);
            }
        }

#ifndef APOLLO_NO_BT
        if (events & APOLLO_EVENT_CONFIG_GATT)
        {
            wiced_log_msg(WICED_LOG_WARNING, "Saving new BTLE GATT attributes and rebooting...\r\n");
            apollo_config_save(&apollo->dct_tables);
            apollo_config_deinit(&apollo->dct_tables);
            apollo_config_gatt_server_stop();
            apollo_service_stop(apollo);
            wiced_framework_reboot();
        }
#endif

        if (events & APOLLO_EVENT_RTP_TIMING)
        {
            apollo_handle_rtp_timing_cmd(apollo);
        }

        if (events & APOLLO_EVENT_TIMER)
        {
            apollo_handle_timer(apollo);
        }

        if (events & APOLLO_EVENT_NETWORK_TIMER)
        {
            /*
             * Try and bring up the network.
             */

            if (apollo_start_network_services(apollo) == WICED_SUCCESS)
            {
                wiced_rtos_stop_timer(&apollo->network_timer);
                if (apollo->dct_tables.dct_app->auto_start)
                {
                    wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_START);
                }
            }
        }

        if (events & APOLLO_EVENT_RELOAD_DCT_WIFI)
        {
            apollo_config_reload_dct_wifi(&apollo->dct_tables);
            wiced_log_msg(WICED_LOG_INFO, "WiFi DCT changed\r\n");
        }

        if (events & APOLLO_EVENT_RELOAD_DCT_NETWORK)
        {
            apollo_config_reload_dct_network(&apollo->dct_tables);
            wiced_log_msg(WICED_LOG_INFO, "Network DCT changed\r\n");
        }

        if (events & APOLLO_EVENT_RELOAD_DCT_BLUETOOTH)
        {
            apollo_config_reload_dct_bluetooth(&apollo->dct_tables);
            wiced_log_msg(WICED_LOG_INFO, "Bluetooth DCT changed\r\n");
        }

        if (events & APOLLO_EVENT_RMC_DOWN)
        {
            apollo_service_stop(apollo);
            result = apollo_stop_network_services(apollo);
            wiced_log_msg(WICED_LOG_INFO, "apollo_stop_network_services() result: %d \r\n", result);
        }

        if (events & APOLLO_EVENT_RMC_UP)
        {
            result = apollo_start_network_services(apollo);
            wiced_log_msg(WICED_LOG_INFO, "apollo_start_network_services() result: %d \r\n", result);
        }

#if defined(OTA2_SUPPORT)
        if (events & APOLLO_EVENT_GET_UPDATE)
        {
            wiced_log_msg(WICED_LOG_NOTICE, "GET_UPDATE\r\n");

            apollo_service_stop(apollo);
            result = apollo_stop_network_services(apollo);
            if (result != WICED_SUCCESS)
            {
                wiced_log_msg(WICED_LOG_ERR, "GET_UPDATE apollo_stop_network_services() failed! %d \r\n", result);
            }
            else
            {
                /*
                 * If you use the command console to get an update,
                 *    always stop playback, but remember and restore the setting
                 *    (ota2_stop_playback_to_update is used in the ota2 callback)
                 */
                int  save_ota2_stop_playback_to_update;
                save_ota2_stop_playback_to_update = apollo->dct_tables.dct_app->ota2_stop_playback_to_update;
                apollo->dct_tables.dct_app->ota2_stop_playback_to_update  = 1;

                apollo->ota2_info.cb_func                             = apollo_ota2_callback;
                apollo->ota2_info.cb_opaque                           = apollo;
                apollo->ota2_info.log_level                           = OTA2_LOG_WARNING;

                apollo->ota2_info.bg_params.auto_update               = 1;       /* auto update on reboot (callback overrides)  */
                apollo->ota2_info.bg_params.initial_check_interval    = 1;       /* unused in this call (must be non-zero)      */
                apollo->ota2_info.bg_params.check_interval            = 0;       /* unused in this call                         */
                apollo->ota2_info.bg_params.retry_check_interval      = SECONDS_PER_MINUTE;     /* minimum retry is one minute  */
                apollo->ota2_info.bg_params.default_ap_info           = NULL;    /* don't restart AP using wiced OTA2 routines  */
                apollo->ota2_info.bg_params.ota2_ap_info              = NULL;    /* use the DCT AP list                         */
                apollo->ota2_info.bg_params.ota2_ap_list              = apollo->dct_tables.dct_wifi->stored_ap_list;
                apollo->ota2_info.bg_params.ota2_ap_list_count        = CONFIG_AP_LIST_SIZE;

                /* This is a blocking call */
                result = apollo_ota2_get_update(&apollo->ota2_info);
                if (result != WICED_SUCCESS)
                {
                    wiced_log_msg(WICED_LOG_ERR, "GET_UPDATE apollo_get_update() failed! %d \r\n", result);
                }

                /* restore this setting */
                apollo->dct_tables.dct_app->ota2_stop_playback_to_update = save_ota2_stop_playback_to_update ;
            }

            /* if successful, and setting is to reboot, do it */
            if ((result == WICED_SUCCESS) && (apollo->dct_tables.dct_app->ota2_reboot_after_download != 0 ))
            {
                wiced_log_msg(WICED_LOG_INFO, " Download is complete - reboot! \r\n");
                wiced_framework_reboot();
            }
            else
            {
            }

            /* when we are done, restart the RMC network */
            wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_RMC_UP);
        }

        if (events & APOLLO_EVENT_START_TIMED_UPDATE)
        {
            wiced_log_msg(WICED_LOG_NOTICE, "START_TIMED_UPDATE -- we check if we are streaming when the timer goes off \r\n");

            apollo->ota2_info.cb_func                             = apollo_ota2_callback;
            apollo->ota2_info.cb_opaque                           = apollo;
            apollo->ota2_info.log_level                           = OTA2_LOG_WARNING;

            apollo->ota2_info.bg_params.auto_update               = 1;                      /* auto update on reboot (callback overrides)  */
            apollo->ota2_info.bg_params.initial_check_interval    = SECONDS_PER_MINUTE * 1; /* initial update time from now                */
            apollo->ota2_info.bg_params.check_interval            = SECONDS_PER_DAY;        /* try once a day                              */
            apollo->ota2_info.bg_params.retry_check_interval      = SECONDS_PER_MINUTE;     /* minimum retry is one minute                 */
            apollo->ota2_info.bg_params.default_ap_info           = NULL;                   /* don't restart AP using wiced OTA2           */
            apollo->ota2_info.bg_params.ota2_ap_info              = NULL;                   /* use the DCT AP list                         */
            apollo->ota2_info.bg_params.ota2_ap_list              = apollo->dct_tables.dct_wifi->stored_ap_list;
            apollo->ota2_info.bg_params.ota2_ap_list_count        = CONFIG_AP_LIST_SIZE;

            /* This is NOT a blocking call - it runs asynch */
            result = apollo_ota2_start_bg_update(&apollo->ota2_info);
            if (result != WICED_SUCCESS)
            {
                    wiced_log_msg(WICED_LOG_ERR, "apollo_start_bg_update() failed! %d \r\n", result);
            }
        }

        if (events & APOLLO_EVENT_STOP_TIMED_UPDATE)
        {
            wiced_log_msg(WICED_LOG_NOTICE, "STOP_TIMED_UPDATE\r\n");
            result = apollo_ota2_stop_bg_update(&apollo->ota2_info);
            if (result != WICED_SUCCESS)
            {
                    wiced_log_msg(WICED_LOG_ERR, "apollo_stop_bg_update() failed! %d \r\n", result);
            }
        }

        if (events & APOLLO_EVENT_OTA2_STATUS)
        {
            apollo_ota2_status(&apollo->ota2_info);
        }
#endif
    };

    /*
     * Make sure that playback has been shut down.
     */

    apollo_service_stop(apollo);

    wiced_log_msg(WICED_LOG_INFO, "End apollo mainloop\r\n");
}


static wiced_result_t apollo_cmd_sender_callback(void* handle, void* userdata, APOLLO_CMD_SENDER_EVENT_T event, void* arg)
{
    apollo_app_t* apollo = (apollo_app_t*)userdata;
    apollo_peers_t* peers;
    int i;

    if (apollo == NULL || apollo->tag != APOLLO_TAG_VALID)
    {
        /*
         * Bad player handle so just return.
         */

        return WICED_SUCCESS;
    }

    if (event == APOLLO_CMD_SENDER_EVENT_DISCOVER_RESULTS)
    {
        peers = (apollo_peers_t*)arg;
        wiced_log_msg(WICED_LOG_INFO, "Found %d speakers on our network\r\n", peers->num_speakers);
        for (i = 0; i < peers->num_speakers; i++)
        {
            wiced_log_msg(WICED_LOG_INFO, "  Speaker name: %s, channel 0x%08lx, MAC %02x:%02x:%02x:%02x:%02x:%02x, IP %ld.%ld.%ld.%ld\r\n",
                          peers->speakers[i].config.speaker_name, peers->speakers[i].config.speaker_channel,
                          peers->speakers[i].mac.octet[0], peers->speakers[i].mac.octet[1], peers->speakers[i].mac.octet[2],
                          peers->speakers[i].mac.octet[3], peers->speakers[i].mac.octet[4], peers->speakers[i].mac.octet[5],
                          (peers->speakers[i].ipaddr >> 24) & 0xFF, (peers->speakers[i].ipaddr >> 16) & 0xFF,
                          (peers->speakers[i].ipaddr >>  8) & 0xFF, peers->speakers[i].ipaddr & 0xFF);
            }
    }
    else if (event == APOLLO_CMD_SENDER_EVENT_COMMAND_STATUS)
    {
        wiced_log_msg(WICED_LOG_INFO, "%d peers replied\r\n", (int)arg);
    }

    return WICED_SUCCESS;
}


static wiced_result_t apollo_cmd_callback(void* handle, void* userdata, APOLLO_CMD_EVENT_T event, void* arg)
{
    apollo_app_t* apollo = (apollo_app_t*)userdata;
    apollo_cmd_speaker_t* cmd_speaker;
    wiced_ip_address_t ip_addr;
    apollo_cmd_rtp_timing_t* rtp_timing;
    apollo_volume_t volume;
    apollo_cmd_metadata_t* data_buffer;
    int log_level;
    int len;

    if (apollo == NULL || apollo->tag != APOLLO_TAG_VALID)
    {
        /*
         * Bad player handle so just return.
         */

        return WICED_SUCCESS;
    }

    switch (event)
    {
        case APOLLO_CMD_EVENT_QUERY_SPEAKER:
            cmd_speaker = (apollo_cmd_speaker_t*)arg;
            if (cmd_speaker)
            {
                cmd_speaker->speaker_name     = apollo->dct_tables.dct_app->speaker_name;
                cmd_speaker->speaker_name_len = strlen(apollo->dct_tables.dct_app->speaker_name);
                cmd_speaker->speaker_channel  = apollo->dct_tables.dct_app->speaker_channel;
            }
            break;

        case APOLLO_CMD_EVENT_SET_SPEAKER:
            cmd_speaker = (apollo_cmd_speaker_t*)arg;
            if (cmd_speaker == NULL)
            {
                break;
            }

            len = cmd_speaker->speaker_name_len;
            if (len > sizeof(apollo->dct_tables.dct_app->speaker_name) - 1)
            {
                len = sizeof(apollo->dct_tables.dct_app->speaker_name) - 1;
            }

            /*
             * If the speaker channel or name has changed than update our configuration.
             */

            if (cmd_speaker->speaker_channel != apollo->dct_tables.dct_app->speaker_channel ||
                strncmp(cmd_speaker->speaker_name, apollo->dct_tables.dct_app->speaker_name, len))
            {
                apollo->dct_tables.dct_app->speaker_channel = cmd_speaker->speaker_channel;
                strncpy(apollo->dct_tables.dct_app->speaker_name, cmd_speaker->speaker_name, len);
                apollo->dct_tables.dct_app->speaker_name[len] = '\0';
                wiced_dct_write((void*)apollo->dct_tables.dct_app, DCT_APP_SECTION, 0, sizeof(apollo_dct_t));
            }
            break;

        case APOLLO_CMD_EVENT_SET_VOLUME:
            memcpy(&volume, arg, sizeof(volume));
            if (volume.volume < APOLLO_VOLUME_MIN || volume.volume > APOLLO_VOLUME_MAX)
            {
                volume.volume = apollo->dct_tables.dct_app->volume;
            }

            /*
             * Do we need to adjust the volume?
             */

            if (volume.volume != apollo->dct_tables.dct_app->volume)
            {
                /*
                 * If we're currently playing audio then adjust the volume.
                 */

                if (apollo->player_handle)
                {
                    apollo_player_set_volume(apollo->player_handle, volume.volume);
                }

                /*
                 * And save the new volume information.
                 */

                if (volume.caching_mode & APOLLO_CACHE_TO_MEMORY)
                {
                    apollo->dct_tables.dct_app->volume = volume.volume;
                }
                if (volume.caching_mode & APOLLO_STORE_TO_NVRAM)
                {
                    wiced_dct_write((void*)apollo->dct_tables.dct_app, DCT_APP_SECTION, 0, sizeof(apollo_dct_t));
                }
            }
            break;

        case APOLLO_CMD_EVENT_SET_LOG_SERVER:
            ip_addr.version = WICED_IPV4;
            ip_addr.ip.v4   = (uint32_t)arg;

            if (ip_addr.ip.v4 != 0 && !apollo->network_logging_active)
            {
                if (apollo_debug_create_tcp_data_socket(&ip_addr, 0) == WICED_SUCCESS)
                {
                    wiced_ip_address_t ip_addr;

                    wiced_log_set_platform_output(apollo_debug_tcp_log_output_handler);

                    /*
                     * Output the IP and MAC addresses. This is helpful when processing the log files.
                     */

                    wiced_ip_get_ipv4_address(WICED_STA_INTERFACE, &ip_addr);
                    wiced_log_msg(WICED_LOG_ERR, "Apollo IP address: %d.%d.%d.%d   MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                                  (ip_addr.ip.v4 >> 24) & 0xFF, (ip_addr.ip.v4 >> 16) & 0xFF, (ip_addr.ip.v4 >> 8) & 0xFF, ip_addr.ip.v4 & 0xFF,
                                  apollo->mac_address.octet[0], apollo->mac_address.octet[1], apollo->mac_address.octet[2],
                                  apollo->mac_address.octet[3], apollo->mac_address.octet[4], apollo->mac_address.octet[5]);
                    apollo->network_logging_active = WICED_TRUE;
                }
            }
            else if (ip_addr.ip.v4 == 0 && apollo->network_logging_active)
            {
                wiced_log_set_platform_output(apollo_log_output_handler);
                apollo_debug_close_tcp_data_socket();
                apollo->network_logging_active = WICED_FALSE;
            }
            break;

        case APOLLO_CMD_EVENT_SET_LOG_LEVEL:
            log_level = (int)arg;
            wiced_log_set_level(log_level);
            break;

        case APOLLO_CMD_EVENT_RTP_TIMING:
            rtp_timing = (apollo_cmd_rtp_timing_t*)arg;
            if (rtp_timing != NULL && apollo->player_handle)
            {
                apollo->rtp_timing_cmd     = rtp_timing->cmd;
                apollo->rtp_timing_entries = rtp_timing->num_entries;

                /*
                 * Tell the main loop it has work to do.
                 */

                wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_RTP_TIMING);
            }
            break;

        case APOLLO_CMD_EVENT_METADATA:
            data_buffer = (apollo_cmd_metadata_t*)arg;

            if (data_buffer->type & (APOLLO_METADATA_TITLE | APOLLO_METADATA_ARTIST))
            {
                wiced_log_msg(WICED_LOG_INFO, "BT track: [%.*s - %.*s]\n",
                              data_buffer->data_length[APOLLO_METADATA_ARTIST_INDEX], data_buffer->data[APOLLO_METADATA_ARTIST_INDEX],
                              data_buffer->data_length[APOLLO_METADATA_TITLE_INDEX], data_buffer->data[APOLLO_METADATA_TITLE_INDEX]);
            }
            break;

        default:
            wiced_log_msg(WICED_LOG_ERR, "Unknown command event: %d\r\n", event);
            break;
    }

    return WICED_SUCCESS;
}


static void apollo_shutdown(apollo_app_t* apollo)
{
    /*
     * Mark the app structure as invalid since we are shutting down.
     */

    apollo->tag = APOLLO_TAG_INVALID;

    /*
     * Shutdown the console.
     */

    command_console_deinit();

    /*
     * Shutdown button handler
     */

    apollo_button_handler_deinit(apollo);

    apollo_stop_network_services(apollo);

    wiced_rtos_deinit_timer(&apollo->timer);
    wiced_rtos_deinit_timer(&apollo->network_timer);
    wiced_rtos_deinit_event_flags(&apollo->events);

#ifndef APOLLO_NO_BT
    apollo_config_gatt_server_stop();

    apollo_bt_service_deinit();
#endif

    free(apollo);
}

static void apollo_console_dct_callback(console_dct_struct_type_t struct_changed, void* app_data)
{
    apollo_app_t*           apollo;

    /* sanity check */
    if (app_data == NULL)
    {
        return;
    }

    apollo = (apollo_app_t*)app_data;
    switch(struct_changed)
    {
        case CONSOLE_DCT_STRUCT_TYPE_WIFI:
            /* Get WiFi configuration */
            wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_RELOAD_DCT_WIFI);
            break;
        case CONSOLE_DCT_STRUCT_TYPE_NETWORK:
            /* Get network configuration */
            wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_RELOAD_DCT_NETWORK);
            break;
        case CONSOLE_DCT_STRUCT_TYPE_BLUETOOTH:
#ifdef WICED_DCT_INCLUDE_BT_CONFIG
            /* Get BT configuration */
            wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_RELOAD_DCT_BLUETOOTH);
#endif
            break;
        default:
            break;
    }
}

static wiced_result_t apollo_start_network_services(apollo_app_t* apollo)
{
    wiced_config_ap_entry_t ap_entry;
    wiced_result_t result;
    uint32_t other_ifaces = 0;

    if (apollo->rmc_network_up == WICED_TRUE)
    {
        return WICED_SUCCESS;
    }

    /* Bring up the network interface */
    memset(&ap_entry, 0, sizeof(ap_entry));
    ap_entry.details.SSID.length = apollo->dct_tables.dct_app->rmc_info.ssid_length;
    memcpy(&ap_entry.details.SSID.value, &apollo->dct_tables.dct_app->rmc_info.ssid_name, apollo->dct_tables.dct_app->rmc_info.ssid_length);
    ap_entry.details.security    = apollo->dct_tables.dct_app->rmc_info.security;
    ap_entry.security_key_length = apollo->dct_tables.dct_app->rmc_info.security_key_length;
    memcpy(&ap_entry.security_key, &apollo->dct_tables.dct_app->rmc_info.security_key, apollo->dct_tables.dct_app->rmc_info.security_key_length);
    ap_entry.details.bss_type    = apollo->dct_tables.dct_app->rmc_info.bss_type;
    ap_entry.details.channel     = apollo->dct_tables.dct_app->rmc_info.channel;
    ap_entry.details.band        = apollo->dct_tables.dct_app->rmc_info.band;

    /*
     * for an infrastructure connection where we are the source and not connected to an AP
     * We must be the AP
     *
     * RMC Bss type         Source          Sink
     *  ADHOC               STA + DHCP      STA
     *  INFRA               AP + DHCP       STA
     *  INFRA (external AP) STA             STA
     */

    apollo->interface = apollo_get_rmc_interface(&apollo->dct_tables);

    /* Determine if any other interfaces should be used */
    if ((apollo->interface == WICED_AP_INTERFACE) &&
        (apollo->dct_tables.dct_wifi->stored_ap_list[0].details.SSID.length)) {
            other_ifaces |= (1 << WICED_STA_INTERFACE);
    }

    result = apollo_network_up_default(apollo->interface, &ap_entry, &ap_ip_settings, apollo->dct_tables.dct_app->apollo_role, other_ifaces);
    if (result == WICED_SUCCESS)
    {
        apollo->rmc_network_up = WICED_TRUE;
    }
    else
    {
        /*
         * The network didn't initialize.
         */

        wiced_log_msg(WICED_LOG_ERR, "Bringing up network interface failed (%d)\n", result);
        return result;
    }

    /* If Apollo is using Infra mode and DCT is specifying a non-NULL SSID
     * then try to bring up upstream facing STA side.
     */

    apollo->upstream_interface_valid = WICED_FALSE;
    if (other_ifaces & (1 << WICED_STA_INTERFACE)) {
        wiced_band_list_t           band_list;
        int32_t                     band = 0;

        /* If multi band hw, set join prefs to 5 Ghz since Apollo needs to run on 5Ghz */
        if (wwd_wifi_get_supported_band_list(&band_list) == WWD_SUCCESS) {
            if (band_list.number_of_bands == 2) {
                wwd_wifi_set_preferred_association_band(WLC_BAND_5G);
                if (wwd_wifi_get_preferred_association_band(&band) == WWD_SUCCESS) {
                    wiced_log_msg(WICED_LOG_INFO, "Preferred band for association is %sGHz\n",
                        band == WLC_BAND_2G ? "2.4" : "5");
                }
            }
        }

        /* Join the upstream AP */
        result = wiced_network_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );

        if (result != WICED_SUCCESS) {
            wiced_log_msg(WICED_LOG_INFO, "STA unable to join upstream AP (%d)\n", result);
            apollo_link_down();
        }
        else
        {
            apollo->upstream_interface_valid = WICED_TRUE;
            apollo->upstream_interface       = WICED_STA_INTERFACE;
        }
    } else {
        wiced_log_msg(WICED_LOG_INFO, "NUll upstream SSID, skipping STA interface.\n");
    }

    /*
     * Join the multicast group.
     */

    SET_IPV4_ADDRESS(apollo->mrtp_ip_address, APOLLO_MULTICAST_IPV4_ADDRESS_DEFAULT);
    wiced_log_msg(WICED_LOG_INFO, "Joining multicast group %d.%d.%d.%d\n",
            (int)((apollo->mrtp_ip_address.ip.v4 >> 24) & 0xFF), (int)((apollo->mrtp_ip_address.ip.v4 >> 16) & 0xFF),
            (int)((apollo->mrtp_ip_address.ip.v4 >> 8) & 0xFF),  (int)(apollo->mrtp_ip_address.ip.v4 & 0xFF));
    result = wiced_multicast_join(apollo->interface, &apollo->mrtp_ip_address);
    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "wiced_multicast_join() failed (%d)\n", result);
    }

    /*
     * Create the Apollo command instance.
     */

    wwd_wifi_get_mac_address(&apollo->mac_address, apollo->interface);
    apollo->cmd_handle = apollo_cmd_init(apollo->interface, &apollo->mac_address, apollo, apollo_cmd_callback);
    if (apollo->cmd_handle == NULL)
    {
        wiced_log_msg(WICED_LOG_ERR, "Unable to create Apollo command instance\r\n");
    }

    /*
     * And the command sender instance.
     */

    apollo->cmd_sender_handle = apollo_cmd_sender_init(apollo->interface, apollo, apollo_cmd_sender_callback);

    /*
     * Create the UDP socket for reporting.
     */

    result = wiced_udp_create_socket(&apollo->report_socket, APOLLO_REPORT_PORT + 1, apollo->interface);
    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Unable to create report socket\n");
    }
    else
    {
        apollo->report_socket_ptr = &apollo->report_socket;
    }

    apollo->rmc_network_up = WICED_TRUE;
    return result;
}


static wiced_result_t apollo_stop_network_services(apollo_app_t* apollo)
{
    wiced_result_t      result = WICED_SUCCESS;

    if (apollo->rmc_network_up == WICED_FALSE)
    {
        return WICED_SUCCESS;
    }

    /*
     * Close the UDP reporting socket.
     */

    if (apollo->report_socket_ptr != NULL)
    {
        wiced_udp_delete_socket(&apollo->report_socket);
        apollo->report_socket_ptr = NULL;
    }

    /*
     * Shut down the Command sender
     */

    if (apollo->cmd_sender_handle != NULL)
    {
        apollo_cmd_sender_deinit(apollo->cmd_sender_handle);
        apollo->cmd_sender_handle = NULL;
    }

    /*
     * Shut down the Apollo Command Instance
     */

    if (apollo->cmd_handle != NULL)
    {
        apollo_cmd_deinit(apollo->cmd_handle);
        apollo->cmd_handle = NULL;
    }

    /*
     * Leave multicast session
     */

    wiced_multicast_leave(apollo->interface, &apollo->mrtp_ip_address);

    /*
     * Shutdown the network
     *
     * for an infrastructure connection where we are the source and not connected to an AP
     * We must be the AP
     *
     * RMC Bss type         Source          Sink
     *  ADHOC               STA + DHCP      STA
     *  INFRA               AP + DHCP       STA
     *  INFRA (external AP) STA             STA
     */

    result = apollo_network_down(apollo->interface, apollo->dct_tables.dct_app->rmc_info.bss_type, apollo->dct_tables.dct_app->apollo_role);

    apollo->rmc_network_up = WICED_FALSE;

#if defined(OTA2_SUPPORT)
    /* Let OTA2 know that the network is down */
    apollo_ota2_network_is_down(&apollo->ota2_info);
#endif

    return result;
}

static apollo_app_t* apollo_init(void)
{
    apollo_app_t* apollo;
    wiced_result_t result;
    uint32_t tag = APOLLO_TAG_VALID;
#ifndef APOLLO_NO_BT
    wiced_result_t result_bt_stack;
    apollo_bt_service_init_params_t bt_init_params = {0, };
    apollo_config_gatt_server_params_t gatt_params = {0, };
#endif

    /*
     * Initialize the logging subsystem.
     */

    result = wiced_log_init(WICED_LOG_ERR, apollo_log_output_handler, apollo_log_get_time);
    if (result != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("wiced_log_init() failed !\r\n"));
    }

    /*
     * Temporary - set the device MAC address in the NVRAM.
     */

    apollo_set_nvram_mac();

    /* Initialize the device */
    result = wiced_init();
    if (result != WICED_SUCCESS)
    {
        return NULL;
    }

    /* Register callbacks */
    wiced_network_register_link_callback(apollo_link_up, apollo_link_down, WICED_STA_INTERFACE);

    /*
     * Note what time we started up.
     */

    result = wiced_time_get_time(&apollo_start_time);

    /*
     * Allocate the main application structure.
     */

    apollo = calloc_named("apollo", 1, sizeof(apollo_app_t));
    if (apollo == NULL)
    {
        wiced_log_msg(WICED_LOG_ERR, "Unable to allocate apollo structure\r\n");
        return NULL;
    }
    apollo->initializing = WICED_TRUE;

    /*
     * Create the command console.
     */

    wiced_log_msg(WICED_LOG_INFO, "Start the command console\r\n");
    result = command_console_init(STDIO_UART, sizeof(apollo_command_buffer), apollo_command_buffer, APOLLO_CONSOLE_COMMAND_HISTORY_LENGTH, apollo_command_history_buffer, " ");
    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Error starting the command console\r\n");
        free(apollo);
        return NULL;
    }
    console_add_cmd_table(apollo_command_table);

    /*
     * Create our event flags.
     */

    result = wiced_rtos_init_event_flags(&apollo->events);
    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Error initializing event flags\r\n");
        tag = APOLLO_TAG_INVALID;
    }

    /*
     * Create a timer.
     */

    result = wiced_rtos_init_timer(&apollo->timer, PLAYBACK_TIMER_PERIOD_MSECS, timer_callback, apollo);
    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Error initializing timer\n");
        tag = APOLLO_TAG_INVALID;
    }

    /*
     * Get DCT
     */
    result = apollo_config_init(&apollo->dct_tables);
    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "apollo_config_init() failed !\r\n");
        apollo_config_deinit(&apollo->dct_tables);
        tag = APOLLO_TAG_INVALID;
    }
    console_dct_register_callback(apollo_console_dct_callback, apollo);

    /*
     * Set the global apollo structure pointer. This pointer is used by the command console processing so
     * setting it here allows the user to interact with the console.
     */

    g_apollo = apollo;

    /*
     * Init button handler
     */

    apollo_button_handler_init(apollo);

    /*
     * Set the current logging level.
     */

    wiced_log_set_level(apollo->dct_tables.dct_app->log_level);

    /*
     * Set our hostname to the speaker name - this way DHCP servers will see which unique speaker acquired a lease
     * once the network init runs ...
     */

    result = wiced_network_set_hostname(apollo->dct_tables.dct_app->speaker_name);
    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Can't set hostname for dhcp_client_init!\r\n");
    }

    /*
     * Make sure that 802.1AS time is enabled.
     */

    wiced_time_enable_8021as();

    /*
     * Initialize nanosecond clock for later use
     */

    wiced_init_nanosecond_clock();

    /* Initialize platform audio */
    platform_init_audio();

    /* print out our current configuration */
    apollo_config_print_info(&apollo->dct_tables);

#ifndef APOLLO_NO_BT
    /*
     * Start main BT service
     * Provide offset at the END of APP section; otherwise, BT stack will overwrite valid/current APP section !
     */

    bt_init_params.app_dct_offset = sizeof(apollo_dct_t);
    result_bt_stack = apollo_bt_service_init( &bt_init_params );
    if (result_bt_stack != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Can't start main BT service !\r\n");
    }

    gatt_params.user_context   = apollo;
    gatt_params.gatt_event_cbf = gatt_event_callback;
    result = apollo_config_gatt_server_start( &gatt_params );
    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "Can't start BT GATT service !\r\n");
    }
#endif

    /* Bring up the network interface */
    result = apollo_start_network_services(apollo);
    if (result != WICED_SUCCESS)
    {
        /*
         * The network didn't initialize but we don't want to consider that a fatal error.
         * Start a timer to periodically retry bringing up the network.
         */

        result = wiced_rtos_init_timer(&apollo->network_timer, NETWORK_INIT_TIMER_PERIOD_MSECS, network_timer_callback, apollo);
        if (result == WICED_SUCCESS)
        {
            wiced_rtos_start_timer(&apollo->network_timer);
        }
        else
        {
            wiced_log_msg(WICED_LOG_ERR, "Error initializing network timer\n");
        }
   }

    /*
     * Set the application tag to the correct state.
     */

    apollo->tag = tag;
    apollo->initializing = WICED_FALSE;

#ifdef USE_AUDIO_DISPLAY
    {
        uint32_t wifi_channel;
        apollo_dct_collection_t* dct_tables = &apollo->dct_tables;

        wiced_wifi_get_channel(&wifi_channel);
        audio_display_create_management_thread(&apollo->display_thread);

        memset(apollo->display_info, 0, sizeof(apollo->display_info));

        strlcpy(apollo->display_name, (char*)apollo->dct_tables.dct_app->rmc_info.ssid_name, sizeof(apollo->display_name));
        sprintf(apollo->display_info, "%s - %d/%d %s", (dct_tables->dct_app->apollo_role != APOLLO_ROLE_SOURCE ? "Sink" : "Source"),
                (int)wifi_channel, apollo->dct_tables.dct_app->rmc_info.channel,
                ((apollo->dct_tables.dct_app->rmc_info.band == WICED_802_11_BAND_2_4GHZ) ? "2.4GHz" : "5GHz"));

        audio_display_header_update_options(BATTERY_ICON_IS_VISIBLE |BATTERY_ICON_SHOW_PERCENT |SIGNAL_STRENGTH_IS_VISIBLE);
        audio_display_footer_update_song_info(apollo->display_name, apollo->display_info);
        audio_display_footer_update_options(FOOTER_IS_VISIBLE | FOOTER_CENTER_ALIGN | ((dct_tables->dct_app->apollo_role == APOLLO_ROLE_SOURCE) ? FOOTER_OPTION_APOLLO_TX : FOOTER_OPTION_APOLLO_RX));
#ifndef APOLLO_NO_BT
        if ( result_bt_stack == WICED_SUCCESS )
        {
            audio_display_header_update_options(BATTERY_ICON_IS_VISIBLE | BATTERY_ICON_SHOW_PERCENT | SIGNAL_STRENGTH_IS_VISIBLE | BLUETOOTH_IS_CONNECTED | INVERT_BLUETOOTH_ICON_COLORS);
        }
#endif
    }
#endif

    {
        uint32_t *chipregs = (uint32_t *)PLATFORM_CHIPCOMMON_REGBASE(0x0);
        uint8_t version;

        version = (*chipregs >> 16) & 0xF;
        wiced_log_msg(WICED_LOG_ERR, "**********Chip version is %s\n",
                      version == 1 ? "B0" : (version == 2 ? "B1" : (version == 3 ? "B2" : "Unknown")));
    }

    return apollo;
}

#if defined(OTA2_SUPPORT)
wiced_bool_t apollo_is_service_active(apollo_app_t* apollo)
{
    if ( (apollo->player_handle != NULL) || (apollo->streamer_handle != NULL))
    {
        return WICED_TRUE;
    }

    return WICED_FALSE;
}

wiced_result_t apollo_ota2_callback(void* session_id, wiced_ota2_service_status_t status, uint32_t value, void* opaque )
{
    apollo_app_t*    apollo = (apollo_app_t*)opaque;

    UNUSED_PARAMETER(session_id);
    UNUSED_PARAMETER(apollo);

    switch( status )
    {
    case OTA2_SERVICE_STARTED:      /* Background service has started
                                     * return - None
                                     */
        wiced_log_msg(WICED_LOG_INFO, "----------------------------- OTA2 Service Called : SERVER STARTED -----------------------------\r\n");
        break;
    case OTA2_SERVICE_AP_CONNECT_ERROR:
        wiced_log_msg(WICED_LOG_INFO, "----------------------------- OTA2 Service Called : AP CONNECT_ERROR -----------------------------\r\n");
        wiced_log_msg(WICED_LOG_INFO, "        return SUCCESS (not used by service). This is informational \r\n");

        /* Bring RMC network back up - App DCT value start_playback_after_rmc_up will restart playback */
        wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_RMC_UP);
        break;

    case OTA2_SERVICE_SERVER_CONNECT_ERROR:
        wiced_log_msg(WICED_LOG_INFO, "----------------------------- OTA2 Service Called : SERVER_CONNECT_ERROR -----------------------------\r\n");
        wiced_log_msg(WICED_LOG_INFO, "        return SUCCESS (not used by service). This is informational \r\n");

        /* Bring RMC network back up - App DCT value start_playback_after_rmc_up will restart playback */
        wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_RMC_UP);
        break;

    case OTA2_SERVICE_AP_CONNECTED:
        wiced_log_msg(WICED_LOG_INFO, "----------------------------- OTA2 Service Called : AP_CONNECTED -----------------------------\r\n");
        wiced_log_msg(WICED_LOG_INFO, "        return SUCCESS (not used by service). This is informational \r\n");
        break;

    case OTA2_SERVICE_SERVER_CONNECTED:
        wiced_log_msg(WICED_LOG_INFO, "----------------------------- OTA2 Service Called : SERVER_CONNECTED -----------------------------\r\n");
        wiced_log_msg(WICED_LOG_INFO, "        return SUCCESS (not used by service). This is informational \r\n");
        break;

    case OTA2_SERVICE_CHECK_FOR_UPDATE: /* Time to check for updates.\r 1641
                                         * return - WICED_SUCCESS = Service will check for update availability
                                         *        - WICED_ERROR   = Application will check for update availability   */
        wiced_log_msg(WICED_LOG_INFO, "----------------------------- OTA2 Service Called : CHECK_FOR_UPDATE -----------------------------\r\n");
        wiced_log_msg(WICED_LOG_INFO, "        return SUCCESS, let Service do the checking.\r\n");

        /* Stopping playback  */
        if ((apollo_is_service_active(apollo) == WICED_TRUE) && (apollo->dct_tables.dct_app->ota2_stop_playback_to_update == 0))
        {
            wiced_log_msg(WICED_LOG_ERR, "dct_app->ota2_stop_playback_to_update == 0 (DO NOT stop playback for updates).\r\n");
            /* product may not want to stop playback to get download - skip it */
            return WICED_ERROR; /* DO NOT stop playback to download an update */
        }
        wiced_log_msg(WICED_LOG_ERR, "Bring down RMC network. (stops playback).\r\n");

        /* set a flag to stop playback and stop RMC network */
        wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_RMC_DOWN);

        /* allow other thread to run */
        wiced_rtos_delay_milliseconds(1);

        /* return success so bg will load the update */
        return WICED_SUCCESS;

    case OTA2_SERVICE_UPDATE_AVAILABLE: /* Service has contacted server, update is available
                                         * value   - pointer to the wiced_ota2_image_header_t structure
                                         *           from the file on the server.
                                         *
                                          * return - WICED_SUCCESS = Application indicating that it wants the
                                         *                           OTA Service to perform the download
                                         *        - WICED_ERROR   = Application indicating that it will perform
                                         *                           the download, the OTA Service will do nothing.  */
    {
        /* the OTA2 header for the update is pointed to by the value argument and is only valid for this function call */
        wiced_ota2_image_header_t* ota2_header;

        ota2_header = (wiced_ota2_image_header_t*)value;

        wiced_log_msg(WICED_LOG_INFO, "----------------------------- OTA2 Service Called : UPDATE_AVAILABLE -----------------------------\r\n");

        /*
         * In an actual application, the application would look at the headers information and decide if the
         * file on the update server is a newer version that the currently running application.
         *
         * If the application wants the update to continue, it would return WICED_SUCCESS here
         * If not, return WICED_ERROR
         */
        wiced_log_msg(WICED_LOG_INFO, "Current Version %d.%d\r\n", apollo->dct_tables.dct_app->ota2_major_version,
                                                                      apollo->dct_tables.dct_app->ota2_minor_version);
        wiced_log_msg(WICED_LOG_INFO, "   OTA2 Version %d.%d\r\n", ota2_header->major_version, ota2_header->minor_version);

#if defined(CHECK_OTA2_UPDATE_VERSION)
        if ((apollo->dct_tables.dct_app->ota2_major_version > ota2_header->major_version) ||
            ((apollo->dct_tables.dct_app->ota2_major_version == ota2_header->major_version) &&
             (apollo->dct_tables.dct_app->ota2_minor_version >= ota2_header->minor_version)) )
        {
            wiced_log_msg(WICED_LOG_INFO, "OTA2 Update Version Fail - return ERROR, do not update!\r\n");
            return WICED_ERROR;
        }
#endif

        wiced_log_msg(WICED_LOG_INFO, "        return SUCCESS, let Service perform the download.\r\n");
        wiced_log_msg(WICED_LOG_NOTICE, "starting download:");
        return WICED_SUCCESS;
    }

    case OTA2_SERVICE_DOWNLOAD_STATUS:  /* Download status - value has % complete (0-100)
                                         *   NOTE: This will only occur when Service is performing download
                                         * return - WICED_SUCCESS = Service will continue download
                                         *        - WICED_ERROR   = Service will STOP download and service will
                                         *                          issue OTA2_SERVICE_TIME_TO_UPDATE_ERROR           */
        wiced_log_msg(WICED_LOG_NOTICE, "OTA2_SERVICE_DOWNLOAD_STATUS %ld %%!\r\n", value);
        return WICED_SUCCESS;

    case OTA2_SERVICE_PERFORM_UPDATE:   /* Download is complete
                                         * return - WICED_SUCCESS = Service will inform Bootloader to extract
                                         *                          and update on next power cycle
                                         *        - WICED_ERROR   = Service will inform Bootloader that download
                                         *                          is complete - Bootloader will NOT extract        */
        wiced_log_msg(WICED_LOG_INFO, "----------------------------- OTA2 Service Called : PERFORM_UPDATE -----------------------------\r\n");
        wiced_log_msg(WICED_LOG_INFO, "        return SUCCESS, let Service extract update on next reboot.\r\n");
        return WICED_SUCCESS;

    case OTA2_SERVICE_UPDATE_ERROR:     /* There was an error in transmission
                                         * This will only occur if Error during Service performing data transfer
                                         * return - WICED_SUCCESS = Service will retry immediately
                                         *        - WICED_ERROR   = Service will retry on next check_interval
                                         *            Application can call
                                         *            wiced_ota2_service_check_for_updates()
                                         *            to run another check earlier than next timed update            */
        wiced_log_msg(WICED_LOG_INFO, "----------------------------- OTA2 Service Called : UPDATE_ERROR -----------------------------\r\n");
        wiced_log_msg(WICED_LOG_INFO, "        return SUCCESS, let Service use retry check interval.\r\n");

        /* Bring RMC network back up - App DCT value start_playback_after_rmc_up will restart playback */
        wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_RMC_UP);
        return WICED_SUCCESS;

    case OTA2_SERVICE_UPDATE_ENDED:     /* All update actions for this check are complete.
                                         * value - WICED_SUCCESS - Service completed download successfully
                                         *       - WICED_ERROR   - Service failed OR Application stopped service from completing
                                         *                         download - in this case, the application must decide if
                                         *                         the download was successful
                                         * This callback is to allow the application to take any actions when
                                         *      the service is done checking / downloading an update
                                         *      It is called for all outcomes: successful or error.
                                         * return - None  - informational
                                         */
        wiced_log_msg(WICED_LOG_NOTICE, "\r\n");
        wiced_log_msg(WICED_LOG_INFO, "----------------------------- OTA2 Service Called : UPDATE_ENDED value: %ld-----------------------------\r\n", value);
        wiced_log_msg(WICED_LOG_INFO, "        return SUCCESS (not used by service). This is informational \r\n");

        if ((value == WICED_SUCCESS) && (apollo->dct_tables.dct_app->ota2_reboot_after_download != 0 ))
        {
            wiced_log_msg(WICED_LOG_INFO, " Download is complete - reboot! \r\n");
            wiced_framework_reboot();
        }

        /* Bring RMC network back up - App DCT value start_playback_after_rmc_up will restart playback */
        wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_RMC_UP);
        break;

    case OTA2_SERVICE_STOPPED:
        wiced_log_msg(WICED_LOG_INFO, "----------------------------- OTA2 Service Called : SERVICEENDED -----------------------------\r\n");
        wiced_log_msg(WICED_LOG_INFO, "        return SUCCESS (not used by service). This is informational \r\n");

        /* Bring RMC network back up - App DCT value start_playback_after_rmc_up will restart playback */
        wiced_rtos_set_event_flags(&apollo->events, APOLLO_EVENT_RMC_UP);
        break;

    default:
        wiced_log_msg(WICED_LOG_INFO, "apollo_ota2_callback() UNKNOWN STATUS %d!\r\n", status);
        break;
    }

    return WICED_SUCCESS;
}

#endif



void application_start(void)
{
    apollo_app_t* apollo;

    /*
     * Main initialization.
     */

    if ((apollo = apollo_init()) == NULL)
    {
        return;
    }

    if (apollo->tag != APOLLO_TAG_VALID)
    {
        apollo_shutdown(apollo);
        return;
    }

    /*
     * Drop into our main loop.
     */

    apollo_mainloop(apollo);

    /*
     * Cleanup and exit.
     */

    g_apollo = NULL;
    apollo_shutdown(apollo);
}

