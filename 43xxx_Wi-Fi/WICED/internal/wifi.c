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

/** @file
 *
 */

#include <stdlib.h>
#include "wiced_wifi.h"
#include "wwd_debug.h"
#include "wwd_assert.h"
#include "wiced_network.h"
#include "wwd_events.h"
#include "platform_dct.h"
#include "string.h"
#include "wwd_wifi.h"
#include "wiced_wps.h"
#include "wiced_utilities.h"
#include "internal/wiced_internal_api.h"
#include "wwd_management.h"
#include "wiced_management.h"
#include "wiced_platform.h"
#include "wiced_framework.h"
#include "wiced_crypto.h"
#include "wiced_deep_sleep.h"
#include "internal/wwd_internal.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define CHECK_RETURN( expr )  { wiced_result_t check_res = (wiced_result_t)(expr); if ( check_res != WICED_SUCCESS ) { wiced_assert("Command failed\n", 0); return check_res; } }

/******************************************************
 *                    Constants
 ******************************************************/

#define WLC_EVENT_MSG_LINK      ( 0x0001 )  /** link is up */

#define SCAN_BSSID_LIST_LENGTH  ( 100 )
#define SCAN_LONGEST_WAIT_TIME  ( 10000 )   /* Dual band devices take over 4000 milliseconds to complete a scan. In the associated case, this time could be doubled */
#define HANDSHAKE_TIMEOUT_MS    ( 3000 )

#define RRM_TIMEOUT             (4000)

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    wiced_scan_result_handler_t results_handler;
    void*                       user_data;
    wiced_bool_t                is_pno_scan;
} internal_scan_handler_t;
static internal_scan_handler_t scan_handler;
/******************************************************
 *               Static Function Declarations
 ******************************************************/

static void*          wiced_link_events_handler ( const wwd_event_header_t* event_header, const uint8_t* event_data, void* handler_user_data );
static void           wiced_scan_result_handler ( wiced_scan_result_t** result_ptr, void* user_data, wiced_scan_status_t status );
static void           find_ap_scan_handler      ( wiced_scan_result_t** result_ptr, void* user_data, wiced_scan_status_t status );
static void           handshake_timeout_handler ( void* arg );
static wiced_result_t handshake_error_callback  ( void* arg );
static void           link_up                   ( void );
static void           link_down                 ( void );
static void*          softap_event_handler      ( const wwd_event_header_t* event_header, const uint8_t* event_data, /*@returned@*/void* handler_user_data );
static wiced_result_t wiced_wifi_init           ( void );
static wiced_result_t wiced_wifi_scan_networks_ex( wiced_scan_result_handler_t results_handler, void* user_data, wiced_scan_type_t scan_type, wiced_ssid_t *ssid );
static void           wiced_wifi_scan_cleanup( void );
/******************************************************
 *               Variable Definitions
 ******************************************************/

/* Link management variables */
static const wwd_event_num_t        link_events[]           = { WLC_E_LINK, WLC_E_DEAUTH_IND, WLC_E_DISASSOC_IND, WLC_E_PSK_SUP, WLC_E_NONE };
static const wwd_event_num_t        ap_client_events[]      = { WLC_E_DEAUTH, WLC_E_DEAUTH_IND, WLC_E_DISASSOC, WLC_E_DISASSOC_IND, WLC_E_ASSOC_IND, WLC_E_REASSOC_IND, WLC_E_NONE };
static wiced_bool_t                 WICED_DEEP_SLEEP_SAVED_VAR( wiced_wlan_initialised )  = WICED_FALSE;
static wiced_bool_t                 WICED_DEEP_SLEEP_SAVED_VAR( wiced_sta_link_up )       = WICED_FALSE;
static wiced_security_t             WICED_DEEP_SLEEP_SAVED_VAR( wiced_sta_security_type ) = WICED_SECURITY_UNKNOWN;
static wiced_timer_t                wiced_sta_handshake_timer;
static wiced_bool_t                 link_up_event_received  = WICED_FALSE;

/* Scanning variables */
static wiced_bool_t                 g_scan_semaphore_and_timer_inited;
static wiced_semaphore_t            scan_semaphore;
static wiced_scan_handler_result_t* off_channel_results;
static wiced_scan_handler_result_t* scan_result_ptr;
static wiced_mac_t*                 scan_bssid_list = NULL;
static int                          current_bssid_list_length = 0;

static wiced_wifi_softap_event_handler_t ap_event_handler;

static wiced_country_code_t         WICED_DEEP_SLEEP_SAVED_VAR( country_code ) = WICED_DEFAULT_COUNTRY_CODE;
#ifdef WICED_ENABLE_AUTO_COUNTRY
static wiced_aggregate_code_t WICED_DEEP_SLEEP_SAVED_VAR( aggregate_code ) = WICED_DEFAULT_COUNTRY_AGGREGATE_CODE;
#endif /* WICED_ENABLE_AUTO_COUNTRY */
static wiced_wifi_rrm_event_handler_t rrm_event_handler;

static wiced_ssid_t*                wiced_wifi_pno_ssid     = NULL;
static wiced_security_t             wiced_wifi_pno_security = WICED_SECURITY_UNKNOWN;
/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_init( void )
{
    wiced_result_t status = WICED_SUCCESS;

    WPRINT_WICED_INFO( ("\nStarting WICED v" WICED_VERSION "\n") );

    CHECK_RETURN( wiced_core_init( ) );

    status = wiced_wlan_connectivity_init( );
    if ( status != WICED_SUCCESS )
    {
        wiced_core_deinit( );
    }

    return status;
}

wiced_result_t wiced_resume_after_deep_sleep( void )
{
    WPRINT_WICED_INFO( ("\nResuming WICED v" WICED_VERSION "\n") );

    if ( !WICED_DEEP_SLEEP_IS_ENABLED( ) )
    {
        wiced_assert( "Deep-sleep is not supported", 0 );
        return WICED_UNSUPPORTED;
    }

    CHECK_RETURN( WICED_DEEP_SLEEP_IS_WARMBOOT_HANDLE( ) ? WICED_SUCCESS : WICED_ERROR );

    CHECK_RETURN( wiced_core_init( ) );

    CHECK_RETURN( wiced_wlan_connectivity_resume_after_deep_sleep( ) );

    WICED_DEEP_SLEEP_CALL_EVENT_HANDLERS( WICED_DEEP_SLEEP_IS_WARMBOOT( ), WICED_DEEP_SLEEP_EVENT_WLAN_RESUME );

    return WICED_SUCCESS;
}

wiced_result_t wiced_wlan_connectivity_init( void )
{
    wiced_result_t              result;
    platform_dct_wifi_config_t* wifi_config;
    platform_dct_misc_config_t* dct_misc;
    uint32_t                    wlan_rand;

#ifdef WPRINT_ENABLE_NETWORK_INFO
    wiced_mac_t  mac;
    char         version[200];
#endif

    if ( wiced_wlan_initialised == WICED_TRUE )
    {
        return WICED_SUCCESS;
    }

    CHECK_RETURN( wiced_network_init( ) );

    /* Initialise Wiced */
    CHECK_RETURN( wiced_dct_read_lock( (void**) &wifi_config, WICED_FALSE, DCT_WIFI_CONFIG_SECTION, 0, sizeof(platform_dct_wifi_config_t) ) );
    CHECK_RETURN( wiced_dct_read_lock( (void**) &dct_misc, WICED_FALSE, DCT_MISC_SECTION, 0, sizeof(platform_dct_misc_config_t) ) );

    if (wifi_config->device_configured == WICED_TRUE)
    {
        country_code = wifi_config->country_code;
#ifdef WICED_ENABLE_AUTO_COUNTRY
        aggregate_code = dct_misc->aggregate_code;
#endif /* WICED_ENABLE_AUTO_COUNTRY */
    }

    wiced_dct_read_unlock( wifi_config, WICED_FALSE );
    wiced_dct_read_unlock( dct_misc, WICED_FALSE );
#ifdef WICED_ENABLE_AUTO_COUNTRY
    wwd_set_aggregate( aggregate_code );
#endif /* WICED_ENABLE_AUTO_COUNTRY */

    result = ( wiced_result_t )wwd_management_wifi_on( country_code );

    /* e.g. user requested an abort or other error */
    if ( result != WICED_SUCCESS )
    {
        wiced_network_deinit( );

        WPRINT_NETWORK_INFO( ("Error %d while starting WICED!\n", result) );
        return result;
    }

    /* Get wlan random to seed PRNG. Not all wlan firmware supports this feature. */
    result = ( wiced_result_t )wwd_wifi_get_iovar_value( IOVAR_STR_RAND, &wlan_rand, WWD_STA_INTERFACE );
    if ( result == WICED_SUCCESS )
    {
        wiced_crypto_add_entropy( &wlan_rand, sizeof( wlan_rand ) );
    }

    WPRINT_NETWORK_INFO( ( "WWD " BUS " interface initialised\n") );

    wiced_wifi_up();

    if ( WICED_DEEP_SLEEP_IS_ENABLED() )
    {
        wwd_wifi_get_and_cache_mac_address( WWD_STA_INTERFACE );
    }

#ifdef WPRINT_ENABLE_NETWORK_INFO
    {
        wwd_wifi_get_mac_address( &mac, WWD_STA_INTERFACE );
        WPRINT_NETWORK_INFO(("WLAN MAC Address : %02X:%02X:%02X:%02X:%02X:%02X\r\n", mac.octet[0],mac.octet[1],mac.octet[2],mac.octet[3],mac.octet[4],mac.octet[5]));

        memset( version, 0, sizeof( version ) );
        wwd_wifi_get_wifi_version( version, sizeof( version ) );
        WPRINT_NETWORK_INFO( ("WLAN Firmware    : %s\r\n", version ) );
    }
#endif

    /* Configure roaming parameters */
    wwd_wifi_set_roam_trigger( WICED_WIFI_ROAMING_TRIGGER_MODE );
    wwd_wifi_set_roam_delta( WICED_WIFI_ROAMING_TRIGGER_DELTA_IN_DBM );
    wwd_wifi_set_roam_scan_period( WICED_WIFI_ROAMING_SCAN_PERIOD_IN_SECONDS );

    wiced_wlan_initialised = WICED_TRUE;

    ap_event_handler = NULL;

    rrm_event_handler = NULL;

    return WICED_SUCCESS;
}

wiced_result_t wiced_wlan_connectivity_resume_after_deep_sleep( void )
{
    if ( !WICED_DEEP_SLEEP_IS_ENABLED( ) )
    {
        wiced_assert( "Deep-sleep is not supported", 0 );
        return WICED_UNSUPPORTED;
    }

    CHECK_RETURN( ( wiced_wlan_initialised == WICED_TRUE ) ? WICED_SUCCESS : WICED_NOTUP );

    CHECK_RETURN( wiced_network_init( ) );

    CHECK_RETURN( wwd_management_wifi_plaform_mpc_init( ) );

    CHECK_RETURN( wiced_wifi_init( ) );

    CHECK_RETURN( (wiced_result_t) wwd_management_wifi_platform_init( country_code, WICED_TRUE ) );

    CHECK_RETURN( (wiced_result_t) wwd_management_set_event_handler_locally( link_events, wiced_link_events_handler, NULL, WWD_STA_INTERFACE ) );

    wwd_wlan_status.state = WLAN_UP;

    return WICED_SUCCESS;
}

static wiced_result_t wiced_wifi_init( void )
{
    if( g_scan_semaphore_and_timer_inited == WICED_FALSE)
    {
        CHECK_RETURN( wiced_rtos_init_semaphore( &scan_semaphore ) );
        CHECK_RETURN( wiced_rtos_set_semaphore( &scan_semaphore ) ); /* Semaphore starts at 1 */

        CHECK_RETURN( wiced_rtos_init_timer( &wiced_sta_handshake_timer, HANDSHAKE_TIMEOUT_MS, handshake_timeout_handler, 0 ) );
        g_scan_semaphore_and_timer_inited = WICED_TRUE;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_wifi_up( void )
{
    CHECK_RETURN( wwd_management_wifi_plaform_mpc_init( ) );

    if ( wwd_wlan_status.state == WLAN_DOWN )
    {
        CHECK_RETURN( wwd_wifi_set_up( ) );
    }

    CHECK_RETURN( wiced_wifi_init( ) );

    return WICED_SUCCESS;
}

wiced_result_t wiced_wlan_connectivity_deinit( void )
{
    if ( wiced_wlan_initialised != WICED_TRUE )
    {
        return WICED_SUCCESS;
    }

    wiced_wifi_down();

    wwd_management_wifi_off( );

    wiced_network_deinit( );

    wiced_wlan_initialised = WICED_FALSE;

    return WICED_SUCCESS;
}

wiced_result_t wiced_wifi_down( void )
{
    CHECK_RETURN( wiced_network_down( WICED_AP_INTERFACE ) );
    CHECK_RETURN( wiced_network_down( WICED_STA_INTERFACE ) );
    CHECK_RETURN( wiced_network_down( WICED_P2P_INTERFACE ) );

    if( g_scan_semaphore_and_timer_inited == WICED_TRUE)
    {
        CHECK_RETURN( wiced_rtos_deinit_timer( &wiced_sta_handshake_timer ) );
        CHECK_RETURN( wiced_rtos_deinit_semaphore( &scan_semaphore ) );
        g_scan_semaphore_and_timer_inited = WICED_FALSE;
    }

    if ( wwd_wlan_status.state == WLAN_UP )
    {
        CHECK_RETURN( wwd_wifi_set_down( ) );
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_deinit( void )
{
    wiced_wlan_connectivity_deinit( );
    wiced_core_deinit( );

    return WICED_SUCCESS;
}

wiced_result_t wiced_start_ap( wiced_ssid_t* ssid, wiced_security_t security, const char* key, uint8_t channel )
{
    return (wiced_result_t) wwd_wifi_start_ap(ssid, security, (uint8_t*)key, (uint8_t) strlen(key), channel);
}

wiced_result_t wiced_stop_ap( void )
{
    return (wiced_result_t) wwd_wifi_stop_ap();
}

wiced_result_t wiced_wifi_register_softap_event_handler( wiced_wifi_softap_event_handler_t event_handler )
{
    ap_event_handler = event_handler;
    return (wiced_result_t) wwd_management_set_event_handler( ap_client_events, softap_event_handler, NULL, WWD_AP_INTERFACE );
}

wiced_result_t wiced_wifi_unregister_softap_event_handler( void )
{
    return (wiced_result_t) wwd_management_set_event_handler( ap_client_events, NULL, NULL, WWD_AP_INTERFACE );
}


wiced_result_t wiced_wifi_register_rrm_event_handler( wiced_wifi_rrm_event_handler_t event_handler )
{
    rrm_event_handler = event_handler;
    return WICED_SUCCESS;
}

wiced_result_t wiced_wifi_unregister_rrm_event_handler( void  )
{
    rrm_event_handler = NULL;
    return WICED_SUCCESS;
}

wiced_result_t wiced_enable_powersave( void )
{
    wiced_result_t result;

    /* Enable MCU powersave */
    wiced_platform_mcu_enable_powersave();

    /* Enable Wi-Fi powersave */
    result = wiced_wifi_enable_powersave();
    if (result != WICED_SUCCESS)
    {
        goto done;
    }

    /* Suspend networking timers */
    result = wiced_network_suspend();

    done:
        return result;
}

wiced_result_t wiced_disable_powersave( void )
{
    wiced_network_resume();
    wiced_wifi_disable_powersave();
    wiced_platform_mcu_disable_powersave();
    return WICED_SUCCESS;
}

/* Configure halt of any joins or scans in progress */
wiced_result_t wiced_wifi_join_halt( wiced_bool_t halt )
{
    return wwd_wifi_join_halt( halt );
}

/** Join a Wi-Fi network as a client
 *      The AP SSID/Passphrase credentials used to join the AP must be available in the DCT
 *      Use the low-level wiced_wifi_join() API to directly join a specific AP if required
 *
 * @return WICED_SUCCESS : Connection to the AP was successful
 *         WICED_ERROR   : Connection to the AP was NOT successful
 */
wiced_result_t wiced_join_ap( void )
{
    unsigned int             a;
    int                      retries;
    wiced_config_ap_entry_t* ap;
    wiced_result_t           result = WICED_NO_STORED_AP_IN_DCT;

    for ( retries = WICED_JOIN_RETRY_ATTEMPTS; retries != 0; --retries )
    {
        /* Try all stored APs */
        for ( a = 0; a < CONFIG_AP_LIST_SIZE; ++a )
        {
            /* Check if the entry is valid */
            CHECK_RETURN( wiced_dct_read_lock( (void**) &ap, WICED_FALSE, DCT_WIFI_CONFIG_SECTION, (uint32_t) ( OFFSETOF(platform_dct_wifi_config_t,stored_ap_list) + a * sizeof(wiced_config_ap_entry_t) ), sizeof(wiced_config_ap_entry_t) ) );
            if ( ap->details.SSID.length != 0 )
            {
                result = wiced_join_ap_specific( &ap->details, ap->security_key_length, ap->security_key );

#ifdef FIRMWARE_WITH_PMK_CALC_SUPPORT
                if ( result == WICED_SUCCESS )
                {
                    /* Extract the calculated PMK and store it in the DCT to speed up future associations */
                    if ( ( ( ( ap->details.security & WPA_SECURITY ) || ( ap->details.security & WPA2_SECURITY ) ) &&  ( ( ap->details.security & ENTERPRISE_ENABLED ) == 0 ) ) &&
                            ( ap->security_key_length != WSEC_MAX_PSK_LEN ) && ( ( ap->details.security & IBSS_ENABLED ) == 0 ) )
                    {
                        wiced_config_ap_entry_t ap_temp;
                        memcpy( &ap_temp, ap, sizeof(wiced_config_ap_entry_t) );
                        if ( wwd_wifi_get_pmk( ap_temp.security_key, ap_temp.security_key_length, ap_temp.security_key ) == WWD_SUCCESS )
                        {
                            ap_temp.security_key_length = WSEC_MAX_PSK_LEN;
                            if ( WICED_SUCCESS != wiced_dct_write( &ap_temp, DCT_WIFI_CONFIG_SECTION, (uint32_t) ( OFFSETOF(platform_dct_wifi_config_t,stored_ap_list) + a * sizeof(wiced_config_ap_entry_t) ), sizeof(wiced_config_ap_entry_t) ) )
                            {
                                 WPRINT_WICED_INFO(("Failed to write ap list to DCT: \n"));
                            }

                        }
                    }
                }
#endif /* FIRMWARE_WITH_PMK_CALC_SUPPORT */
            }

            wiced_dct_read_unlock( (wiced_config_ap_entry_t*) ap, WICED_FALSE );

            if ( result == WICED_SUCCESS )
            {
                return result;
            }
        }
    }
    return result;
}

wiced_result_t wiced_join_ap_specific( wiced_ap_info_t* details, uint8_t security_key_length, const char security_key[ 64 ] )
{

    wiced_result_t      join_result = WICED_STA_JOIN_FAILED;
    wiced_scan_result_t temp_scan_result;

    char                ssid_name[SSID_NAME_SIZE + 1];
    memset(ssid_name, 0, sizeof(ssid_name));
    memcpy(ssid_name, details->SSID.value, details->SSID.length);
    WPRINT_WICED_INFO(("Joining : %s\n", ssid_name));

    memcpy( &temp_scan_result, details, sizeof( *details ) );

    /* Try join AP with last know specific details */
    if ( !( NULL_MAC(details->BSSID.octet) ) && details->channel != 0 )
    {
        join_result = (wiced_result_t) wwd_wifi_join_specific( &temp_scan_result, (uint8_t*) security_key, security_key_length, NULL, WWD_STA_INTERFACE );
    }

    if ( join_result != WICED_SUCCESS )
    {
        wiced_security_t security;

        security = details->security;
        if (details->bss_type == WICED_BSS_TYPE_ADHOC)
        {
            WPRINT_WICED_INFO(("%s: Network is ADHOC\n", __FUNCTION__));
            security |= IBSS_ENABLED;
        }

        /* If join-specific failed, try scan and join AP */
        join_result = (wiced_result_t) wwd_wifi_join( &details->SSID, security, (uint8_t*) security_key, security_key_length, NULL, WWD_STA_INTERFACE );
    }

    if ( join_result == WICED_SUCCESS )
    {
        WPRINT_WICED_INFO( ( "Successfully joined : %s\n", ssid_name) );

        wiced_sta_link_up       = WICED_TRUE;
        wiced_sta_security_type = details->security;

        wwd_management_set_event_handler( link_events, wiced_link_events_handler, NULL, WWD_STA_INTERFACE );
        return WICED_SUCCESS;
    }
    else
    {
        WPRINT_WICED_INFO(("Failed to join : %s\n", ssid_name));
    }
    return join_result;
}


wiced_result_t wiced_leave_ap( wiced_interface_t interface )
{
    /* Deregister the link event handler and leave the current AP */
    wwd_management_set_event_handler(link_events, NULL, 0, WICED_TO_WWD_INTERFACE( interface ) );
    wwd_wifi_leave( WICED_TO_WWD_INTERFACE( interface ) );
    if ( interface == WICED_STA_INTERFACE )
    {
        wiced_sta_link_up = WICED_FALSE;
    }
    return WICED_SUCCESS;
}

wiced_result_t wiced_wifi_scan_disable( void )
{
    wiced_result_t result = (wiced_result_t )wwd_wifi_abort_scan( );

    /* If there's a failure; something went wrong. Abort req without active scan looks like a no-op */
    if ( WICED_SUCCESS != result )
    {
        WPRINT_WICED_ERROR(("Failed to abort scan (none active?). Result=%d\n", result));
        return result;
    }

    /* suppression means firmware is prevented from doing scans even user requested */
    return (wiced_result_t)wwd_wifi_set_scan_suppress( WICED_TRUE );
}

wiced_result_t wiced_wifi_scan_enable( void )
{
    /* allow all scans again */
    return (wiced_result_t)wwd_wifi_set_scan_suppress( WICED_FALSE );
}

wiced_result_t wiced_wifi_scan_networks( wiced_scan_result_handler_t results_handler, void* user_data )
{
    return wiced_wifi_scan_networks_ex( results_handler, user_data, WICED_SCAN_TYPE_ACTIVE, NULL );
}

static wiced_result_t wiced_wifi_scan_networks_ex( wiced_scan_result_handler_t results_handler, void* user_data, wiced_scan_type_t scan_type, wiced_ssid_t *ssid )
{
    wiced_result_t result;

    wiced_assert("Bad args", results_handler != NULL);

    /* A requested scan will override the last disable */
    wiced_wifi_scan_enable( );

    result = wiced_rtos_get_semaphore( &scan_semaphore, SCAN_LONGEST_WAIT_TIME );
    if ( result != WICED_SUCCESS )
    {
        /* Return error result, but set the semaphore to work the next time */
        wiced_rtos_set_semaphore( &scan_semaphore );
        return result;
    }

    off_channel_results = NULL;
    current_bssid_list_length = 0;

    /* Prepare space to keep track of seen BSSIDs */
    if (scan_bssid_list != NULL)
    {
        free(scan_bssid_list);
    }
    scan_bssid_list = (wiced_mac_t*) malloc(SCAN_BSSID_LIST_LENGTH * sizeof(wiced_mac_t));
    if (scan_bssid_list == NULL)
    {
        goto exit;
    }
    memset(scan_bssid_list, 0, SCAN_BSSID_LIST_LENGTH * sizeof(wiced_mac_t));

    /* Convert function pointer to object so it can be passed around */
    scan_handler.results_handler  = results_handler;
    scan_handler.user_data        = user_data;
    scan_handler.is_pno_scan      = ( scan_type == WICED_SCAN_TYPE_PNO ) ? WICED_TRUE : WICED_FALSE;

    /* Initiate scan */
    scan_result_ptr = MALLOC_OBJECT("scan result", wiced_scan_handler_result_t);
    if (scan_result_ptr == NULL)
    {
        goto error_with_bssid_list;
    }
    memset( scan_result_ptr, 0, sizeof( wiced_scan_handler_result_t ) );
    scan_result_ptr->status    = WICED_SCAN_INCOMPLETE;
    scan_result_ptr->user_data = user_data;

    if ( scan_type == WICED_SCAN_TYPE_PNO )
    {
        wiced_wifi_pno_ssid = malloc( sizeof( *wiced_wifi_pno_ssid ) );

        if ( wiced_wifi_pno_ssid == NULL )
        {
            goto error_with_result_ptr;

        }
        wiced_wifi_pno_ssid->length = ssid->length;
        memset( wiced_wifi_pno_ssid->value, 0, sizeof( wiced_wifi_pno_ssid->value ) );
        memcpy( wiced_wifi_pno_ssid->value, ssid->value, ssid->length );
    }

    if ( wwd_wifi_scan( scan_type, WICED_BSS_TYPE_ANY, NULL, NULL, NULL, NULL, wiced_scan_result_handler, (wiced_scan_result_t**) &scan_result_ptr, &scan_handler, WWD_STA_INTERFACE ) != WWD_SUCCESS )
    {
        goto error_with_pno_ssid;
    }

    return WICED_SUCCESS;

error_with_pno_ssid:
    /* may be NULL if not pno scan */
    if ( wiced_wifi_pno_ssid != NULL )
    {
        free( wiced_wifi_pno_ssid );
        wiced_wifi_pno_ssid = NULL;
    }

error_with_result_ptr:
    free(scan_result_ptr);
    scan_result_ptr = NULL;

error_with_bssid_list:
    free(scan_bssid_list);
    scan_bssid_list = NULL;
exit:
    wiced_rtos_set_semaphore(&scan_semaphore);
    return WICED_ERROR;
}

wiced_result_t wiced_wifi_add_custom_ie( wiced_interface_t interface, const wiced_custom_ie_info_t* ie_info )
{
    wiced_assert("Bad args", ie_info != NULL);

    return (wiced_result_t) wwd_wifi_manage_custom_ie( WICED_TO_WWD_INTERFACE( interface ), WICED_ADD_CUSTOM_IE, (const uint8_t*)ie_info->oui, ie_info->subtype, (const void*)ie_info->data, ie_info->length, ie_info->which_packets );
}

wiced_result_t wiced_wifi_remove_custom_ie( wiced_interface_t interface, const wiced_custom_ie_info_t* ie_info )
{
    wiced_assert("Bad args", ie_info != NULL);

    return (wiced_result_t) wwd_wifi_manage_custom_ie( WICED_TO_WWD_INTERFACE( interface ), WICED_REMOVE_CUSTOM_IE, (const uint8_t*)ie_info->oui, ie_info->subtype, (const void*)ie_info->data, ie_info->length, ie_info->which_packets );
}

wiced_result_t wiced_wps_enrollee(wiced_wps_mode_t mode, const wiced_wps_device_detail_t* details, const char* password, wiced_wps_credential_t* credentials, uint16_t credential_count)
{
    wiced_result_t result    = WICED_SUCCESS;
    wps_agent_t*   workspace = (wps_agent_t*) calloc_named("wps", 1, sizeof(wps_agent_t));

    if ( workspace == NULL )
    {
        return WICED_OUT_OF_HEAP_SPACE;
    }

    besl_wps_init ( workspace, (besl_wps_device_detail_t*) details, WPS_ENROLLEE_AGENT, WWD_STA_INTERFACE );
    result = (wiced_result_t) besl_wps_start( workspace, (besl_wps_mode_t) mode, password, (besl_wps_credential_t*) credentials, credential_count );
    host_rtos_delay_milliseconds( 10 ); /* Delay required to allow WPS thread to run and initialize */
    if ( result == WICED_SUCCESS )
    {
        besl_wps_wait_till_complete( workspace );
        result = (wiced_result_t) besl_wps_get_result( workspace );
    }

    besl_wps_deinit( workspace );
    free( workspace );
    workspace = NULL;

    return result;
}

wiced_result_t wiced_wps_registrar(wiced_wps_mode_t mode, const wiced_wps_device_detail_t* details, const char* password, wiced_wps_credential_t* credentials, uint16_t credential_count)
{
    wiced_result_t result    = WICED_SUCCESS;
    wps_agent_t*   workspace = (wps_agent_t*) calloc_named("wps", 1, sizeof(wps_agent_t));

    if ( workspace == NULL )
    {
        return WICED_OUT_OF_HEAP_SPACE;
    }

    besl_wps_init ( workspace, (besl_wps_device_detail_t*) details, WPS_REGISTRAR_AGENT, WWD_AP_INTERFACE );
    result = (wiced_result_t) besl_wps_start( workspace, (besl_wps_mode_t) mode, password, (besl_wps_credential_t*) credentials, credential_count );
    if ( result == WICED_SUCCESS )
    {
        besl_wps_wait_till_complete( workspace );
        result = (wiced_result_t) besl_wps_get_result( workspace );
    }

    besl_wps_deinit( workspace );
    free( workspace );
    workspace = NULL;

    return result;
}

/******************************************************
 *                    Static Functions
 ******************************************************/

static void link_up( void )
{
    if ( wiced_sta_link_up == WICED_FALSE )
    {
        wiced_network_notify_link_up( WICED_STA_INTERFACE );
        wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, wiced_wireless_link_up_handler, 0 );
        wiced_sta_link_up = WICED_TRUE;
    }
    else
    {
        wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, wiced_wireless_link_renew_handler, 0 );
    }
}

static void link_down( void )
{
    if ( wiced_sta_link_up == WICED_TRUE )
    {
        /* Notify network stack that the link is down. Further processing will be done in the link down handler */
        wiced_network_notify_link_down( WICED_STA_INTERFACE );

        /* Force awake the networking thread. It might still be blocked on receive or send timeouts */
        wiced_rtos_thread_force_awake( &(WICED_NETWORKING_WORKER_THREAD->thread) );

        wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, wiced_wireless_link_down_handler, 0 );
        wiced_sta_link_up = WICED_FALSE;
    }
}

static wiced_result_t handshake_error_callback( void* arg )
{
    unsigned int   a;
    int   retries;
    wiced_config_ap_entry_t* ap;

    UNUSED_PARAMETER( arg );

    /* Explicitly leave AP and then rejoin */
    wiced_leave_ap( WICED_STA_INTERFACE );

    for ( retries = WICED_JOIN_RETRY_ATTEMPTS; retries != 0; --retries )
    {
        /* Try all stored APs */
        for ( a = 0; a < CONFIG_AP_LIST_SIZE; ++a )
        {
            /* Check if the entry is valid */
            CHECK_RETURN( wiced_dct_read_lock( (void**) &ap, WICED_FALSE, DCT_WIFI_CONFIG_SECTION, (uint32_t) ( OFFSETOF(platform_dct_wifi_config_t,stored_ap_list) + a * sizeof(wiced_config_ap_entry_t) ), sizeof(wiced_config_ap_entry_t) ) );
            if ( ap->details.SSID.length != 0 )
            {
                wiced_result_t      join_result = WICED_ERROR;
                wiced_scan_result_t temp_scan_result;

                memcpy( &temp_scan_result, &ap->details, sizeof( ap->details ) );

                /* Try join AP with last know specific details */
                if ( !( NULL_MAC(ap->details.BSSID.octet) ) && ap->details.channel != 0 )
                {
                    join_result = (wiced_result_t) wwd_wifi_join_specific( &temp_scan_result, (uint8_t*) ap->security_key, ap->security_key_length, NULL, WWD_STA_INTERFACE );
                }

                if ( join_result != WICED_SUCCESS )
                {
                    /* If join-specific failed, try scan and join AP */
                    join_result = (wiced_result_t) wwd_wifi_join( &ap->details.SSID, ap->details.security, (uint8_t*) ap->security_key, ap->security_key_length, NULL, WWD_STA_INTERFACE );
                }

                if ( join_result == WICED_SUCCESS )
                {
                    link_up();
                    wiced_sta_security_type = ap->details.security;

                    wwd_management_set_event_handler( link_events, wiced_link_events_handler, NULL, WWD_STA_INTERFACE );

#ifdef FIRMWARE_WITH_PMK_CALC_SUPPORT
                    /* Extract the calculated PMK and store it in the DCT to speed up future associations */
                    if ( ( ( ( ap->details.security & WPA_SECURITY ) || ( ap->details.security & WPA2_SECURITY ) ) &&  ( ( ap->details.security & ENTERPRISE_ENABLED ) == 0 ) ) &&
                            ( ap->security_key_length != WSEC_MAX_PSK_LEN ) && ( ( ap->details.security & IBSS_ENABLED ) == 0 ) )
                    {
                        wiced_config_ap_entry_t ap_temp;
                        memcpy( &ap_temp, ap, sizeof(wiced_config_ap_entry_t) );
                        if ( wwd_wifi_get_pmk( ap_temp.security_key, ap_temp.security_key_length, ap_temp.security_key ) == WWD_SUCCESS )
                        {
                            ap_temp.security_key_length = WSEC_MAX_PSK_LEN;
                            if ( WICED_SUCCESS != wiced_dct_write( &ap_temp, DCT_WIFI_CONFIG_SECTION, (uint32_t) ( OFFSETOF(platform_dct_wifi_config_t,stored_ap_list) + a * sizeof(wiced_config_ap_entry_t) ), sizeof(wiced_config_ap_entry_t) ) )
                            {
                                 WPRINT_WICED_INFO(("Failed to write ap list to DCT: \n"));
                            }

                        }
                    }
#endif /* FIRMWARE_WITH_PMK_CALC_SUPPORT */

                    wiced_dct_read_unlock( (wiced_config_ap_entry_t*) ap, WICED_FALSE );
                    return WICED_SUCCESS;
                }
            }
            wiced_dct_read_unlock( (wiced_config_ap_entry_t*) ap, WICED_FALSE );
        }
    }
    return WICED_STA_JOIN_FAILED;
}

static void handshake_timeout_handler( void* arg )
{
    UNUSED_PARAMETER( arg );

    wiced_rtos_stop_timer( &wiced_sta_handshake_timer );

    /* This RTOS timer callback runs in interrupt context. Defer event management to WICED_NETWORKING_WORKER_THREAD */
    wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, handshake_error_callback, 0 );
}

static void* wiced_link_events_handler( const wwd_event_header_t* event_header, const uint8_t* event_data, void* handler_user_data )
{
    UNUSED_PARAMETER( event_data );

    if ( event_header->interface != (uint8_t) WICED_STA_INTERFACE )
    {
        return handler_user_data;
    }

    WPRINT_NETWORK_DEBUG( ("Link event (type, status, reason, flags) %u %u %u %u\n", (unsigned int)event_header->event_type, (unsigned int)event_header->status,
            (unsigned int)event_header->reason, (unsigned int)event_header->flags ) );

    switch ( event_header->event_type )
    {
        case WLC_E_LINK:
            if ( ( event_header->flags & WLC_EVENT_MSG_LINK ) != 0 )
            {
                switch ( wiced_sta_security_type )
                {
                    case WICED_SECURITY_OPEN:
                    case WICED_SECURITY_IBSS_OPEN:
                    case WICED_SECURITY_WPS_OPEN:
                    case WICED_SECURITY_WPS_SECURE:
                    case WICED_SECURITY_WEP_PSK:
                    case WICED_SECURITY_WEP_SHARED:
                    {
                        /* Advertise link-up immediately as no EAPOL is required */
                        link_up();
                        break;
                    }

                    case WICED_SECURITY_WPA_TKIP_PSK:
                    case WICED_SECURITY_WPA_AES_PSK:
                    case WICED_SECURITY_WPA_MIXED_PSK:
                    case WICED_SECURITY_WPA2_AES_PSK:
                    case WICED_SECURITY_WPA2_TKIP_PSK:
                    case WICED_SECURITY_WPA2_MIXED_PSK:
                    case WICED_SECURITY_WPA_TKIP_ENT:
                    case WICED_SECURITY_WPA_AES_ENT:
                    case WICED_SECURITY_WPA_MIXED_ENT:
                    case WICED_SECURITY_WPA2_TKIP_ENT:
                    case WICED_SECURITY_WPA2_AES_ENT:
                    case WICED_SECURITY_WPA2_MIXED_ENT:
                    {
                        link_up_event_received = WICED_TRUE;
                        /* Start a timer and wait for WLC_E_PSK_SUP event */
                        wiced_rtos_start_timer( &wiced_sta_handshake_timer );
                        break;
                    }

                    case WICED_SECURITY_UNKNOWN:
                    case WICED_SECURITY_FORCE_32_BIT:
                    default:
                    {
                        wiced_assert( "Bad Security type\r\n", 0 != 0 );
                        break;
                    }
                }
            }
            else
            {
                link_down( );
            }
            break;

        case WLC_E_DEAUTH_IND:
        case WLC_E_DISASSOC_IND:
            link_down( );
            break;

        case WLC_E_PSK_SUP:
            {
                /* WLC_E_PSK_SUP is received. Check for status and reason. */
                if ( event_header->status == WLC_SUP_KEYED && event_header->reason == WLC_E_SUP_OTHER && link_up_event_received == WICED_TRUE )
                {
                    /* Successful WPA key exchange. Stop timer and announce link up event to application */
                    wiced_rtos_stop_timer( &wiced_sta_handshake_timer );
                    link_up();
                    link_up_event_received = WICED_FALSE;
                }
                break;
            }

        /* Note - These are listed to keep gcc pedantic checking happy */
        case WLC_E_NONE:
        case WLC_E_ROAM:
        case WLC_E_SET_SSID:
        case WLC_E_DEAUTH:
        case WLC_E_DISASSOC:
        case WLC_E_JOIN:
        case WLC_E_START:
        case WLC_E_AUTH:
        case WLC_E_AUTH_IND:
        case WLC_E_ASSOC:
        case WLC_E_ASSOC_IND:
        case WLC_E_REASSOC:
        case WLC_E_REASSOC_IND:
        case WLC_E_QUIET_START:
        case WLC_E_QUIET_END:
        case WLC_E_BEACON_RX:
        case WLC_E_MIC_ERROR:
        case WLC_E_NDIS_LINK:
        case WLC_E_TXFAIL:
        case WLC_E_PMKID_CACHE:
        case WLC_E_RETROGRADE_TSF:
        case WLC_E_PRUNE:
        case WLC_E_AUTOAUTH:
        case WLC_E_EAPOL_MSG:
        case WLC_E_SCAN_COMPLETE:
        case WLC_E_ADDTS_IND:
        case WLC_E_DELTS_IND:
        case WLC_E_BCNSENT_IND:
        case WLC_E_BCNRX_MSG:
        case WLC_E_BCNLOST_MSG:
        case WLC_E_ROAM_PREP:
        case WLC_E_PFN_NET_FOUND:
        case WLC_E_PFN_NET_LOST:
        case WLC_E_RESET_COMPLETE:
        case WLC_E_JOIN_START:
        case WLC_E_ROAM_START:
        case WLC_E_ASSOC_START:
        case WLC_E_IBSS_ASSOC:
        case WLC_E_RADIO:
        case WLC_E_PSM_WATCHDOG:
        case WLC_E_CCX_ASSOC_START:
        case WLC_E_CCX_ASSOC_ABORT:
        case WLC_E_PROBREQ_MSG:
        case WLC_E_SCAN_CONFIRM_IND:
        case WLC_E_COUNTRY_CODE_CHANGED:
        case WLC_E_EXCEEDED_MEDIUM_TIME:
        case WLC_E_ICV_ERROR:
        case WLC_E_UNICAST_DECODE_ERROR:
        case WLC_E_MULTICAST_DECODE_ERROR:
        case WLC_E_TRACE:
        case WLC_E_BTA_HCI_EVENT:
        case WLC_E_IF:
        case WLC_E_P2P_DISC_LISTEN_COMPLETE:
        case WLC_E_RSSI:
        case WLC_E_PFN_SCAN_COMPLETE:
        case WLC_E_EXTLOG_MSG:
        case WLC_E_ACTION_FRAME:
        case WLC_E_ACTION_FRAME_COMPLETE:
        case WLC_E_PRE_ASSOC_IND:
        case WLC_E_PRE_REASSOC_IND:
        case WLC_E_CHANNEL_ADOPTED:
        case WLC_E_AP_STARTED:
        case WLC_E_DFS_AP_STOP:
        case WLC_E_DFS_AP_RESUME:
        case WLC_E_WAI_STA_EVENT:
        case WLC_E_WAI_MSG:
        case WLC_E_ESCAN_RESULT:
        case WLC_E_ACTION_FRAME_OFF_CHAN_COMPLETE:
        case WLC_E_PROBRESP_MSG:
        case WLC_E_P2P_PROBREQ_MSG:
        case WLC_E_DCS_REQUEST:
        case WLC_E_FIFO_CREDIT_MAP:
        case WLC_E_ACTION_FRAME_RX:
        case WLC_E_WAKE_EVENT:
        case WLC_E_RM_COMPLETE:
        case WLC_E_HTSFSYNC:
        case WLC_E_OVERLAY_REQ:
        case WLC_E_CSA_COMPLETE_IND:
        case WLC_E_EXCESS_PM_WAKE_EVENT:
        case WLC_E_PFN_SCAN_NONE:
        case WLC_E_PFN_SCAN_ALLGONE:
        case WLC_E_GTK_PLUMBED:
        case WLC_E_ASSOC_IND_NDIS:
        case WLC_E_REASSOC_IND_NDIS:
        case WLC_E_ASSOC_REQ_IE:
        case WLC_E_ASSOC_RESP_IE:
        case WLC_E_ASSOC_RECREATED:
        case WLC_E_ACTION_FRAME_RX_NDIS:
        case WLC_E_AUTH_REQ:
        case WLC_E_TDLS_PEER_EVENT:
        case WLC_E_SPEEDY_RECREATE_FAIL:
        case WLC_E_NATIVE:
        case WLC_E_PKTDELAY_IND:
        case WLC_E_AWDL_AW:
        case WLC_E_AWDL_ROLE:
        case WLC_E_AWDL_EVENT:
        case WLC_E_NIC_AF_TXS:
        case WLC_E_NIC_NIC_REPORT:
        case WLC_E_BEACON_FRAME_RX:
        case WLC_E_SERVICE_FOUND:
        case WLC_E_GAS_FRAGMENT_RX:
        case WLC_E_GAS_COMPLETE:
        case WLC_E_P2PO_ADD_DEVICE:
        case WLC_E_P2PO_DEL_DEVICE:
        case WLC_E_WNM_STA_SLEEP:
        case WLC_E_TXFAIL_THRESH:
        case WLC_E_PROXD:
        case WLC_E_IBSS_COALESCE:
        case WLC_E_AWDL_RX_PRB_RESP:
        case WLC_E_AWDL_RX_ACT_FRAME:
        case WLC_E_AWDL_WOWL_NULLPKT:
        case WLC_E_AWDL_PHYCAL_STATUS:
        case WLC_E_AWDL_OOB_AF_STATUS:
        case WLC_E_AWDL_SCAN_STATUS:
        case WLC_E_AWDL_AW_START:
        case WLC_E_AWDL_AW_END:
        case WLC_E_AWDL_AW_EXT:
        case WLC_E_AWDL_PEER_CACHE_CONTROL:
        case WLC_E_CSA_START_IND:
        case WLC_E_CSA_DONE_IND:
        case WLC_E_CSA_FAILURE_IND:
        case WLC_E_CCA_CHAN_QUAL:
        case WLC_E_BSSID:
        case WLC_E_TX_STAT_ERROR:
        case WLC_E_BCMC_CREDIT_SUPPORT:
        case WLC_E_PSTA_PRIMARY_INTF_IND:
        case WLC_E_RRM:
        case WLC_E_LAST:
        case WLC_E_FORCE_32_BIT:
        case WLC_E_BT_WIFI_HANDOVER_REQ:
        case WLC_E_PFN_BEST_BATCHING:
        case WLC_E_SPW_TXINHIBIT:
        case WLC_E_FBT_AUTH_REQ_IND:
        case WLC_E_RSSI_LQM:
        case WLC_E_PFN_GSCAN_FULL_RESULT:
        case WLC_E_PFN_SWC:
        case WLC_E_AUTHORIZED:
        case WLC_E_PROBREQ_MSG_RX:
        case WLC_E_RMC_EVENT:
        case WLC_E_DPSTA_INTF_IND:
        default:
            wiced_assert( "Received event which was not registered\n", 0 != 0 );
            break;
    }
    return handler_user_data;
}

static void wiced_wifi_scan_cleanup( void )
{
    if ( scan_bssid_list != NULL )
    {
        malloc_transfer_to_curr_thread(scan_bssid_list);
        free(scan_bssid_list);
    }

    if ( wiced_wifi_pno_ssid != NULL )
    {
        malloc_transfer_to_curr_thread( wiced_wifi_pno_ssid );
        free( wiced_wifi_pno_ssid );
    }

    scan_bssid_list     = NULL;
    off_channel_results = NULL;
    wiced_wifi_pno_ssid = NULL;
    wiced_rtos_set_semaphore(&scan_semaphore);
}

/* we're at the end of the scan job, report up the off channel results and the final result (finished or aborted) */
static wiced_result_t wiced_scan_finish_handler( wiced_scan_handler_result_t* malloced_scan_result )
{
    wiced_scan_handler_result_t* result_iter, *tmp;

    /* Go through the list of off-channel results and report them to the application */
    for ( result_iter = off_channel_results; result_iter != NULL ;  )
    {
        /* callee frees memory so remember the next entry */
        tmp = result_iter->next;

        scan_handler.results_handler( result_iter );

        /* on to the next result */
        result_iter = tmp;
    }

    /* report last result to app */
    scan_handler.results_handler( malloced_scan_result );

    /* clean up after the scan */
    wiced_wifi_scan_cleanup( );

    return WICED_SUCCESS;
}

static void wiced_scan_result_handler( wiced_scan_result_t** result_ptr, void* user_data, wiced_scan_status_t status )
{
    wiced_scan_handler_result_t* result_iter;
    wiced_scan_handler_result_t* current_result;
    wiced_mac_t*                 mac_iter     = NULL;
    internal_scan_handler_t*    int_scan_handler = (internal_scan_handler_t*) user_data;

    /* Check if we don't have a scan result to send to the user */
    if ( scan_result_ptr == NULL )
    {
        return;
    }

    current_result = (wiced_scan_handler_result_t*)(*result_ptr);

    /* check and filter pno scan results, to ensure only desired ssids are passed back */
    if ( int_scan_handler->is_pno_scan == WICED_TRUE )
    {
        /* if the result don't match our one and only ssid, then bail out and ignore */
        if ( wiced_wifi_pno_ssid == NULL || wiced_wifi_pno_ssid->length != current_result->ap_details.SSID.length || memcmp(current_result->ap_details.SSID.value, wiced_wifi_pno_ssid->value, wiced_wifi_pno_ssid->length) != 0 )
        {
            return;
        }
        else
        {
            /* fill out the legit security, saved from ealier as FW doesn't supply currently */
            current_result->ap_details.security = wiced_wifi_pno_security;
        }
    }

    /* Check if scan is complete */
    if ( status == WICED_SCAN_COMPLETED_SUCCESSFULLY || status == WICED_SCAN_ABORTED )
    {

        /* Indicate to the scan handler that scanning is complete */
        scan_result_ptr->status = status;

        /* send out a final event, for further processing */
        if ( wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, (event_handler_t)wiced_scan_finish_handler, (void*) ( scan_result_ptr ) ) != WICED_SUCCESS )
        {
            /* Unable to Queue the message so call scan results
                       * handler directly with the scan status.
                       */
            wiced_scan_finish_handler( scan_result_ptr );
        }
        return;
    }

    /* Check if the result is "on channel" */
    if ((*result_ptr)->on_channel == WICED_TRUE)
    {
        /* Check if this result obsoletes any off channel results we've received earlier */
        wiced_scan_handler_result_t* previous_result = NULL;
        for ( result_iter = off_channel_results; result_iter != NULL ; result_iter = result_iter->next )
        {
            /* Check for matching MAC address */
            if ( CMP_MAC( result_iter->ap_details.BSSID.octet, (*result_ptr)->BSSID.octet ) )
            {
                /* Delete the off channel result */
                if ( previous_result != NULL )
                {
                    previous_result->next = result_iter->next;
                }
                else
                {
                    off_channel_results = NULL;
                }
                free( result_iter );
                result_iter = NULL;
                break;
            }
            previous_result = result_iter;
        }

        /* Check if we've seen this AP before, if not then mac_iter will point to the place where we should put the new result */
        for ( mac_iter = scan_bssid_list; ( mac_iter < scan_bssid_list + current_bssid_list_length ); ++mac_iter )
        {
            /* Check for matching MAC address */
            if ( CMP_MAC( mac_iter->octet, current_result->ap_details.BSSID.octet ) )
            {
                /* Ignore this result. Clean up the result and let it be reused */
                memset( *result_ptr, 0, sizeof(wiced_scan_result_t) );
                return;
            }
        }

        /* New BSSID - add it to the list */
        if ( current_bssid_list_length < SCAN_BSSID_LIST_LENGTH )
        {
            memcpy( &mac_iter->octet, current_result->ap_details.BSSID.octet, sizeof(wiced_mac_t) );
            current_bssid_list_length++;
        }

        /* Post event in worker thread */
        if ( wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, (event_handler_t) int_scan_handler->results_handler, (void*) ( scan_result_ptr ) ) != WICED_SUCCESS )
        {
            free( *result_ptr );
            *result_ptr = NULL;
        }
    }
    else
    {

        /* Check if its already in the off-channel list */
        for ( result_iter = off_channel_results; result_iter != NULL ; result_iter = result_iter->next )
        {
            /* Check for matching MAC address */
            if ( CMP_MAC( result_iter->ap_details.BSSID.octet, current_result->ap_details.BSSID.octet ) )
            {
                /* Ignore this result. Clean up the result and let it be reused */
                memset(*result_ptr, 0, sizeof(wiced_scan_result_t));
                return;
            }
        }

        /* Add it to the list of off-channel results */
        current_result->next = off_channel_results;
        off_channel_results = current_result;
    }

    /* Allocate new storage space for next scan result */
    scan_result_ptr = MALLOC_OBJECT("scan result", wiced_scan_handler_result_t);
    if (scan_result_ptr != NULL)
    {
        memset( scan_result_ptr, 0, sizeof(wiced_scan_handler_result_t));
        scan_result_ptr->status    = WICED_SCAN_INCOMPLETE;
        scan_result_ptr->user_data = int_scan_handler->user_data;
    }
    *result_ptr = (wiced_scan_result_t*)scan_result_ptr;
}

static void* softap_event_handler( const wwd_event_header_t* event_header, const uint8_t* event_data, /*@returned@*/ void* handler_user_data )
{
    wiced_wifi_softap_event_t event = WICED_AP_UNKNOWN_EVENT;

    UNUSED_PARAMETER( handler_user_data );
    UNUSED_PARAMETER( event_data );

    if ( ap_event_handler != NULL )
    {
        if ( event_header->interface == WWD_AP_INTERFACE )
        {
            if ( ( event_header->event_type == WLC_E_DEAUTH ) ||
                 ( event_header->event_type == WLC_E_DEAUTH_IND ) ||
                 ( event_header->event_type == WLC_E_DISASSOC ) ||
                 ( event_header->event_type == WLC_E_DISASSOC_IND ) )
            {
                event = WICED_AP_STA_LEAVE_EVENT;
            }
            else if ( event_header->event_type == WLC_E_ASSOC_IND || event_header->event_type == WLC_E_REASSOC_IND )
            {
                event = WICED_AP_STA_JOINED_EVENT;
            }

            ap_event_handler( event, &event_header->addr );
        }
    }

    return NULL;
}

wiced_result_t wiced_wifi_find_ap( const char* ssid, wiced_scan_result_t* ap_info, const uint16_t* optional_channel_list)
{
    wiced_result_t  result;
    wiced_ssid_t    ssid_info;
    wiced_semaphore_t semaphore;

    wiced_rtos_init_semaphore( &semaphore );

    /* Validate input arguments */
    wiced_assert( "Bad args", ((ssid != NULL) && (ap_info != NULL) && (strlen(ssid) < sizeof(ssid_info.value))) );

    memset( ap_info, 0, sizeof( *ap_info ) );

    /* Populate SSID information */
    memset( &ssid_info, 0, sizeof( ssid_info ) );
    ssid_info.length = (uint8_t) strlen( ssid );
    memcpy( ssid_info.value, ssid, ssid_info.length );

    /* Initiate the scan */
    result = ( wiced_result_t ) wwd_wifi_scan( WICED_SCAN_TYPE_ACTIVE, WICED_BSS_TYPE_ANY, &ssid_info, NULL, optional_channel_list, NULL, find_ap_scan_handler, (wiced_scan_result_t**) &ap_info, &semaphore, WWD_STA_INTERFACE );
    if ( result != WICED_SUCCESS )
    {
        goto exit;
    }

    result = wiced_rtos_get_semaphore( &semaphore, SCAN_LONGEST_WAIT_TIME );

    if ( ap_info->channel == 0 )
    {
        result = WICED_WWD_ACCESS_POINT_NOT_FOUND;
    }

exit:
    wiced_rtos_deinit_semaphore( &semaphore );
    memset(&semaphore, 0, sizeof(semaphore));

    return result;
}

wiced_result_t wiced_wifi_pno_start( wiced_ssid_t *ssid, wiced_security_t security, wiced_scan_result_handler_t handler, void *user_data )
{
    wiced_result_t  result;

    /* add the network */
    CHECK_RETURN( wwd_wifi_pno_add_network( ssid, security ) );
    wiced_wifi_pno_security = security;
    /* Initiate the scan */
    result = wiced_wifi_scan_networks_ex( handler, user_data, WICED_SCAN_TYPE_PNO, ssid );
    return result;
}

wiced_result_t wiced_wifi_pno_stop( void )
{
    /* bring pno down and clear out any networks */
    CHECK_RETURN( wwd_wifi_pno_clear( ) );

    /* clean up any memory which may outstanding */
    wiced_wifi_scan_cleanup( );

    return WICED_SUCCESS;
}

static wiced_result_t wiced_xlate_pattern_to_wl_pattern( wiced_packet_pattern_t *pattern_in, wl_wowl_pattern_list_t *pattern_list)
{
    wl_wowl_pattern_t *pattern_out = &pattern_list->pattern[0];

    /* one and only for now */
    pattern_list->count = 1;

    pattern_out->masksize      = pattern_in->mask_size;
    pattern_out->offset        = pattern_in->match_offset;
    pattern_out->patternoffset = pattern_in->mask_size + sizeof( *pattern_list );
    pattern_out->patternsize   = pattern_in->pattern_size;

    if ( 0 != pattern_in->mask_size )
    {
        if ( NULL == pattern_in->mask )
        {
            return WICED_BADARG;
        }

        memcpy( ( uint8_t* )pattern_list + sizeof( *pattern_list ), pattern_in->mask, pattern_in->mask_size );
    }

    if ( 0 != pattern_in->pattern_size )
    {
        if ( NULL == pattern_in->pattern )
        {
            wiced_assert( "Null pattern data", 0 != 0 );
            return WICED_BADARG;
        }

        memcpy( ( uint8_t* )pattern_list + pattern_out->patternoffset, pattern_in->pattern, pattern_in->pattern_size );
    }

    return WICED_SUCCESS;
}

static wiced_result_t wiced_xlate_keep_alive_to_wl_keep_alive( wiced_keep_alive_packet_t *keep_alive_in, wl_mkeep_alive_pkt_t *keep_alive_out )
{
    keep_alive_out->version     = WL_MKEEP_ALIVE_VERSION;
    keep_alive_out->length      = WL_MKEEP_ALIVE_FIXED_LEN;
    keep_alive_out->period_msec = keep_alive_in->period_msec;

    /* the one and only wowl keep alive set to FW (for now) */
    keep_alive_out->keep_alive_id = 0;
    keep_alive_out->len_bytes     = keep_alive_in->packet_length;

    if ( 0 != keep_alive_out->len_bytes )
    {
        if ( NULL == keep_alive_in->packet )
        {
            wiced_assert( "Null keepalive data", 0 != 0 );
            return WICED_BADARG;
        }

        memcpy( keep_alive_out->data, keep_alive_in->packet, keep_alive_out->len_bytes );
    }

    return WICED_SUCCESS;
}

/*
  * Enter ds1 mode with a given offload
  */
wiced_result_t wiced_wifi_enter_ds1( wiced_interface_t interface, wiced_offload_t offload_type, wiced_offload_value_t *offload_value, uint32_t ulp_wait_milliseconds )
{
    uint32_t        wowl_caps = 0;
    wiced_result_t  result    = WICED_SUCCESS;

    /* powersave needs to be on to enter; make it so here */
    result = wwd_wifi_enable_powersave( );

    switch ( offload_type )
    {
        case WICED_OFFLOAD_PATTERN:
        {
            wl_wowl_pattern_list_t *pattern_list      = NULL;
            char                   *pattern_cmd       = NULL;
            uint32_t                pattern_data_size = sizeof( *pattern_list ) + offload_value->pattern.mask_size + offload_value->pattern.pattern_size + strlen("add") + 1;
            /* allocate memory and xlate */
            pattern_cmd  = calloc( pattern_data_size, 1 );

            if ( NULL == pattern_cmd )
            {
                return WICED_OUT_OF_HEAP_SPACE;
            }

            pattern_list = ( wl_wowl_pattern_list_t* )( pattern_cmd + strlen("add") + 1 );
            strncpy( pattern_cmd, "add", strlen("add") );
            result = wiced_xlate_pattern_to_wl_pattern( &offload_value->pattern, pattern_list );

            if ( WICED_SUCCESS == result )
            {
                /* call into driver to set up wowl */
                wowl_caps |= WL_WOWL_NET;
                result = wwd_wifi_wowl_enable( WICED_TO_WWD_INTERFACE( interface ), wowl_caps, wowl_caps, NULL, (uint8_t*)pattern_cmd, pattern_data_size, NULL );
            }

            /* free memory */
            free( pattern_cmd );
        }
        break;
        case WICED_OFFLOAD_KEEP_ALIVE:
        {
            wl_mkeep_alive_pkt_t  *wowl_keepalive_data = NULL;
            /* allocate memory and xlate */
            wowl_keepalive_data = malloc( sizeof( *wowl_keepalive_data ) + offload_value->keep_alive_packet_info.packet_length );

            if ( NULL == wowl_keepalive_data )
            {
                return WICED_OUT_OF_HEAP_SPACE;
            }

            result = wiced_xlate_keep_alive_to_wl_keep_alive( &offload_value->keep_alive_packet_info, wowl_keepalive_data );

            if ( WICED_SUCCESS == result )
            {
                /* call into driver to set up wowl */
                wowl_caps |= WL_WOWL_TRAFFIC;
                result = wwd_wifi_wowl_enable( WICED_TO_WWD_INTERFACE( interface ), wowl_caps, wowl_caps, wowl_keepalive_data, NULL, 0, NULL );
            }

            /* free memory */
            free( wowl_keepalive_data );
        }
        break;
        case WICED_OFFLOAD_ARP_HOSTIP:
        {
            uint32_t arp_host_ip_v4_address;

            if ( WICED_IPV4 != offload_value->ipv4_address.version )
            {
                /* Ipv6 not currently supported */
                return WICED_BADARG;
            }

            memcpy( &arp_host_ip_v4_address, &offload_value->ipv4_address.ip.v4, sizeof( arp_host_ip_v4_address ) );

            wowl_caps |= WL_WOWL_TRAFFIC;
            CHECK_RETURN( wwd_wifi_wowl_enable( WICED_TO_WWD_INTERFACE( interface ), wowl_caps, wowl_caps, NULL, NULL, 0, &arp_host_ip_v4_address ) );
        }
        break;
        default:
            result = WICED_BADARG;
        break;
    }

    if ( WICED_SUCCESS == result )
    {
        CHECK_RETURN( wwd_wifi_enter_ds1( interface, ulp_wait_milliseconds ) );
    }
    return result;
}

wiced_result_t wiced_wifi_exit_ds1( wiced_interface_t interface )
{
    CHECK_RETURN ( wwd_wifi_exit_ds1( interface ) );
    /* Note: powersave and wowl still enabled here: if client wants to exit it can do so; want to avoid unnecessary side-effects */
    return WICED_SUCCESS;
}

static void find_ap_scan_handler( wiced_scan_result_t** result_ptr, void* user_data, wiced_scan_status_t status )
{
    wiced_semaphore_t* semaphore = (wiced_semaphore_t*)user_data;
    UNUSED_PARAMETER( result_ptr );

    /* Check if scan is complete */
    if ( status == WICED_SCAN_COMPLETED_SUCCESSFULLY || status == WICED_SCAN_ABORTED )
    {
        wiced_rtos_set_semaphore( semaphore );
        return;
    }
}

/* This is callback function to handle the events
 * @param1: wwd_event_header_t
 * @param2: event_data
 * @param3: void* (user_data: semaphore)
 */
void* wiced_rrm_report_handler( const wwd_event_header_t* event_header, const uint8_t* event_data, /*@returned@*/ void* handler_user_data )
{

    wiced_semaphore_t* semaphore = (wiced_semaphore_t*)handler_user_data;

   if (rrm_event_handler != NULL ) {
         rrm_event_handler(event_header, event_data );
    }

    wiced_rtos_set_semaphore( semaphore );
    return handler_user_data;
}

/* This function send Radio Resource Request by calling WWD API to send the request
 * maintains the semaphore so that only one call is sent and response is received in the
 * callback function
 * @param1: wwd_rrm_report_type_t
 * @param2: wwd_event_handler_t
 * @param3: radio resource management request structure
 * @param4: wwd_rrm_report_t**
 * @param5: wwd_interface_t
 * @return: wwd_result_t
 */
wwd_result_t wiced_wifi_rrm_request(  wwd_rrm_report_type_t                      report_type,
                                      wwd_event_handler_t                        callback,
                                      void*                                      rrm_req,
                                      wwd_rrm_report_t**                         report_ptr,
                                      wwd_interface_t                            interface
                                     )
{

    wwd_result_t result = WWD_SUCCESS;
    wiced_semaphore_t rrm_req_complete_semaphore;

    wiced_rtos_init_semaphore( &rrm_req_complete_semaphore );

    wwd_wifi_rrm_request( report_type, callback,  rrm_req, report_ptr, &rrm_req_complete_semaphore, interface);

    wiced_rtos_get_semaphore(&rrm_req_complete_semaphore, RRM_TIMEOUT);

    wiced_rtos_deinit_semaphore( &rrm_req_complete_semaphore );
    memset(&rrm_req_complete_semaphore, 0, sizeof(rrm_req_complete_semaphore));

    return result;
}

wiced_result_t wiced_wifi_start_ap_with_custom_ie( wiced_ssid_t* ssid, wiced_security_t security, const char* key, uint8_t channel, const wiced_custom_ie_info_t* ie)
{
    CHECK_RETURN( wwd_wifi_ap_init( ssid, security, (const uint8_t*)key, (uint8_t)strlen(key), channel ) );
    CHECK_RETURN(wwd_wifi_manage_custom_ie(WICED_AP_INTERFACE, WICED_ADD_CUSTOM_IE,
                                           ie->oui, ie->subtype, ie->data, ie->length, ie->which_packets));
    CHECK_RETURN( wwd_wifi_ap_up() );

    return WWD_SUCCESS;
}
