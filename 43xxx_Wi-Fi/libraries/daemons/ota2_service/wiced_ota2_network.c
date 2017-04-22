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
 * WICED Over The Air 2 Background Network interface (OTA2)
 *
 *        ***  PRELIMINARY - SUBJECT TO CHANGE  ***
 *
 *  This API allows for disconnecting from current network
 *      and connecting to an alternate network for accessing
 *      the internet and downloading an OTA2 Image File
 */
#include <ctype.h>
#include "wiced.h"
#include "internal/wwd_sdpcm.h"
#include "../../WICED/internal/wiced_internal_api.h"
#include "wwd_buffer_interface.h"

#include "wiced_ota2_service.h"
#include "wiced_ota2_network.h"

#define OTA2_LIB_PRINTF(arg)  WPRINT_LIB_DEBUG(arg)

/******************************************************
 *                      Macros
 ******************************************************/

#define CHECK_RETURN( expr )  { wwd_result_t check_res = (expr); if ( check_res != WWD_SUCCESS ) { wiced_assert("Command failed\n", 0 == 1); return check_res; } }

/******************************************************
 *                    Constants
 ******************************************************/

#define MAX_AP_CONNECT_RETRIES      3
#define MAX_GET_IP_ADDRESS_RETRIES  3

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                  Enumerations
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/


/******************************************************
 *               Variables Definitions
 ******************************************************/

/****************************************************************
 *  Internal functions
 ****************************************************************/

/****************************************************************
 *  External functions
 ****************************************************************/
/**
 *  Restore APSTA, AP, MPC, rmc_ackreq, powersave
 *  Restore channel
 *
 * @return - WICED_SUCCESS
 *           WICED_ERROR
 */
static wiced_result_t wiced_ota2_network_settings_restore( wiced_ota2_saved_network_params_t* saved_params )
{
    wiced_result_t  result = WICED_SUCCESS;
    wwd_result_t    retval;

    /* sanity check */
    if (saved_params == NULL)
    {
        return WICED_BADARG;
    }

    OTA2_LIB_PRINTF(("Bringing Network down to restore settings\r\n"));
    wiced_wifi_down();

    /* Restore APSTA */
    retval = wwd_wifi_set_iovar_value(IOVAR_STR_APSTA, saved_params->APSTA, WWD_STA_INTERFACE);
    if ((retval != WWD_SUCCESS) && (retval != WWD_UNSUPPORTED))
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_settings_for_ota2: Restore APSTA error: %d\r\n", retval));
        result = WICED_SUCCESS; /* we want to try to continue anyway */
    }

    /* Restore AP */
    CHECK_RETURN(wwd_wifi_set_ioctl_value(WLC_SET_AP, saved_params->AP, WWD_STA_INTERFACE));


    /* Get Minimum Power Consumption */
    CHECK_RETURN(wwd_wifi_set_iovar_value(IOVAR_STR_MPC, saved_params->MPC, WWD_STA_INTERFACE));

    /* Restore Power Management */
    saved_params->powersave_mode = wiced_wifi_get_powersave_mode();
    if ( saved_params->powersave_mode == PM1_POWERSAVE_MODE )
    {
        wiced_wifi_enable_powersave();
    }
    else if ( saved_params->powersave_mode == PM2_POWERSAVE_MODE )
    {
        wiced_wifi_enable_powersave_with_throughput( saved_params->return_to_sleep_delay );
    }

    /* Restore rmc_ackreq */
    retval = wwd_wifi_set_iovar_value(IOVAR_STR_RMC_ACKREQ, saved_params->rmc_ackreq, WWD_STA_INTERFACE);
    if ((retval != WWD_SUCCESS) && (retval != WWD_UNSUPPORTED))
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_settings_for_ota2: Restore rmc_ackreq error: %d\r\n", retval));
        result = WICED_SUCCESS; /* we want to try to continue anyway */
    }

    /*
     * Now bring up the network.
     */
    retval = wiced_wifi_up();
    if (retval != WWD_SUCCESS)
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_settings_restore: wiced_wifi_up() failed: %d\r\n", retval));
        result = WICED_ERROR;
    }

    return result;
}

wiced_result_t wiced_ota2_network_down( wiced_ota2_saved_network_params_t* saved_params )
{
    wiced_result_t result;

    /* sanity check */
    if (saved_params == NULL)
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_down() Bad ARGS! saved_params %p\r\n", saved_params));
        return WICED_BADARG;
    }

    result = wiced_ip_down( WICED_STA_INTERFACE );
    if ( result != WICED_SUCCESS )
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_down: wiced_ip_down() failed! %d \r\n", result));
    }

    /*
     * Bring down the WiFi interface.
     */
    result = wiced_wifi_down();
    if ( result != WICED_SUCCESS )
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_down: wiced_wifi_down() failed! %d \r\n", result));
    }

    return wiced_ota2_network_settings_restore(saved_params);
}

/**
 *
 *  Save current settings and Set APSTA, AP, MPC, powersave off, rmc_ackreq to 0x00
 *  Set channel
 *
 * @param[in]  channel - channel we expect AP to be on
 *
 * @return - WICED_SUCCESS
 *           WICED_ERROR
 */
static wiced_result_t wiced_ota2_network_settings_for_ota2(wiced_ota2_saved_network_params_t* saved_params, uint32_t channel)
{
    wiced_result_t  result = WICED_SUCCESS;
    wwd_result_t    retval;

    /* sanity check */
    if (saved_params == NULL)
    {
        return WICED_BADARG;
    }


    OTA2_LIB_PRINTF(("Bringing Network down to adjust settings for OTA2\r\n"));
    wiced_wifi_down();

    /* get current APSTA */
    saved_params->APSTA = 0;
    retval = wwd_wifi_get_iovar_value(IOVAR_STR_APSTA, &saved_params->APSTA, WWD_STA_INTERFACE );
    if ((retval != WWD_SUCCESS) && (retval != WWD_UNSUPPORTED))
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_settings_for_ota2: Get APSTA error: %d\r\n", retval));
        result = WICED_SUCCESS; /* we want to try to continue anyway */
    }
    /* Turn APSTA off */
    retval = wwd_wifi_set_iovar_value(IOVAR_STR_APSTA, 0, WWD_STA_INTERFACE);
    if ((retval != WWD_SUCCESS) && (retval != WWD_UNSUPPORTED))
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_settings_for_ota2: Turn off APSTA error: %d\r\n", retval));
        result = WICED_SUCCESS; /* we want to try to continue anyway */
    }

    /* Get current AP */
    saved_params->AP = 0;
    retval = wwd_wifi_get_ioctl_value(WLC_GET_AP, &saved_params->AP, WWD_STA_INTERFACE);
    if ((retval != WWD_SUCCESS) && (retval != WWD_UNSUPPORTED))
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_settings_for_ota2: Get AP error: %d\r\n", retval));
        result = WICED_SUCCESS; /* we want to try to continue anyway */
    }
    /* Set AP to 0 */
    CHECK_RETURN(wwd_wifi_set_ioctl_value(WLC_SET_AP, 0, WWD_STA_INTERFACE));


    /* Get Minimum Power Consumption */
    saved_params->MPC = 0;
    retval = wwd_wifi_get_iovar_value(IOVAR_STR_MPC, &saved_params->MPC, WWD_STA_INTERFACE );
    if ((retval != WWD_SUCCESS) && (retval != WWD_UNSUPPORTED))
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_settings_for_ota2: Get MPC error: %d\r\n", retval));
        result = WICED_SUCCESS; /* we want to try to continue anyway */
    }
    saved_params->MPC &= 0x000000ff;
    /* Set MPC to 0 */
    CHECK_RETURN(wwd_wifi_set_iovar_value(IOVAR_STR_MPC, 0, WWD_STA_INTERFACE));


    /* Get Power Management setting */
    saved_params->powersave_mode = wiced_wifi_get_powersave_mode();
    saved_params->return_to_sleep_delay = wiced_wifi_get_return_to_sleep_delay();
    /* Disable Power Management */
    wiced_wifi_disable_powersave();

    /* Set channel if supplied */
    if (channel != 0)
    {
        CHECK_RETURN(wwd_wifi_set_ioctl_value(WLC_SET_CHANNEL, channel, WWD_STA_INTERFACE));

        /* delay a bit - WWD seems to be busy alot when we set rmc_ackreq after setting the channel */
        wiced_rtos_delay_milliseconds(100);
    }

    /* get rmc_ackreq */
    saved_params->rmc_ackreq = 0;
    retval = wwd_wifi_get_iovar_value(IOVAR_STR_RMC_ACKREQ, &saved_params->rmc_ackreq, WWD_STA_INTERFACE );
    if ((retval != WWD_SUCCESS) && (retval != WWD_UNSUPPORTED))
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_settings_for_ota2: Get rmc_ackreq error: %d\r\n", retval));
        result = WICED_SUCCESS; /* we want to try to continue anyway */
    }
    saved_params->rmc_ackreq &= 0x000000ff;
    /* Set rmc_ackreq to 0 - no RMC when downloading update */
    retval = wwd_wifi_set_iovar_value(IOVAR_STR_RMC_ACKREQ, 0, WWD_STA_INTERFACE);
    if ((retval != WWD_SUCCESS) && (retval != WWD_UNSUPPORTED))
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_settings_for_ota2: Turn off rmc_ackreq error: %d\r\n", retval));
        result = WICED_SUCCESS; /* we want to try to continue anyway */
    }

    /*
     * Now bring up the network.
     */
    retval = wiced_wifi_up();
    if (retval != WWD_SUCCESS)
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_settings_for_ota2: wiced_wifi_up() failed: %d\r\n", retval));
        result = WICED_ERROR;
    }

    return result;
}


wiced_result_t wiced_ota2_network_up( wiced_config_ap_entry_t* ap_info, wiced_ota2_saved_network_params_t* saved_params )
{
    wiced_result_t  result = WICED_SUCCESS;
    int             tries;

    /* sanity check */
    if ((ap_info == NULL) || (saved_params == NULL))
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_up() Bad ARGS! ap_info %p saved_params %p\r\n", ap_info, saved_params));
        return WICED_BADARG;
    }

    result = wiced_ota2_network_settings_for_ota2(saved_params, (uint32_t)ap_info->details.channel);
    if (result != WICED_SUCCESS)
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_up() failed! %d\r\n", result));
        wiced_ota2_network_settings_restore( saved_params );
        return result;
    }

    OTA2_LIB_PRINTF(("wiced_ota2_network_up: start %.*s\r\n", ap_info->details.SSID.length, ap_info->details.SSID.value));


    /* connect to OTA2 AP */
    tries = 0;
    do
    {
        result = wiced_join_ap_specific( &ap_info->details, ap_info->security_key_length, ap_info->security_key );
        if (result != WICED_SUCCESS)
        {
            if (result == (wiced_result_t)WWD_NETWORK_NOT_FOUND)
            {
                tries = MAX_AP_CONNECT_RETRIES; /* so we do not retry */
                OTA2_LIB_PRINTF(("wiced_ota2_network_up: wiced_join_ap_specific() failed (NOT Found)! - we will retry %d\r\n", result));
            }
            else
            {
                OTA2_LIB_PRINTF(("wiced_ota2_network_up: wiced_join_ap_specific() failed! result:%d try:%d\r\n", result, tries));
            }
        }
        else
        {
            break;
        }

        /* wait a bit and try again */
        wiced_rtos_delay_milliseconds(200);

    } while ((result != WICED_SUCCESS) && (tries++ < MAX_AP_CONNECT_RETRIES));

    if (result == WICED_SUCCESS)
    {
        tries = 0;
        do
        {
            wiced_ip_address_t ip_addr;

            /* get our IP address */
            result = wiced_ip_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );
            if ( result != WICED_SUCCESS )
            {
                OTA2_LIB_PRINTF(("wiced_ota2_network_up: wiced_ip_up() failed: result %d tries:%d \r\n", result, (tries + 1)));
            }
            else
            {
                wiced_ip_get_ipv4_address(WICED_STA_INTERFACE, &ip_addr);
                OTA2_LIB_PRINTF(("         IP addr: %d.%d.%d.%d\r\n",
                                (int)((ip_addr.ip.v4 >> 24) & 0xFF), (int)((ip_addr.ip.v4 >> 16) & 0xFF),
                                (int)((ip_addr.ip.v4 >> 8) & 0xFF),  (int)(ip_addr.ip.v4 & 0xFF)));
                if ((ip_addr.ip.v4 != 0x0000) && (ip_addr.ip.v4 != 0x0001))
                {
                    result = WICED_SUCCESS;
                }
                else
                {
                    OTA2_LIB_PRINTF(("wiced_ota2_network_up: wiced_ip_get_ipv4_address() failed: tries:%d \r\n", (tries + 1)));
                }
            }

            /* wait a bit and try again */
            wiced_rtos_delay_milliseconds(200);

        } while ((result != WICED_SUCCESS) && (tries++ < MAX_GET_IP_ADDRESS_RETRIES));
    }

    if (result != WICED_SUCCESS)
    {
        OTA2_LIB_PRINTF(("wiced_ota2_network_up() failed: tries:%d \r\n", tries));
        wiced_ota2_network_settings_restore( saved_params );
    }

    return result;
}
