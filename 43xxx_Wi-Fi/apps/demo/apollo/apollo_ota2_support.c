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

/** @file Apollo OTA2 support functions
 *
 */

#if defined(OTA2_SUPPORT)

#include <ctype.h>
#include "wiced.h"
#include "wiced_log.h"
#include "apollo_config.h"

#include "apollo_ota2_support.h"

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
 *               Function Declarations
 ******************************************************/

wiced_result_t apollo_ota2_restore_settings_after_update( apollo_dct_collection_t* dct_tables, ota2_boot_type_t boot_type, wiced_bool_t* save_dct_tables );

/******************************************************
 *               Variables Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t apollo_ota2_get_update(apollo_ota2_service_info_t* ota2_info)
{
    wiced_result_t  result = WICED_ERROR;
    wiced_bool_t    deinit_ota2_bg;

    if (ota2_info == NULL)
    {
        return WICED_BADARG;
    }

    /* get the image from the server & save in staging area */
    wiced_ota2_service_uri_split(ota2_info->update_uri, ota2_info->host_name, sizeof(ota2_info->host_name),
            ota2_info->file_path, sizeof(ota2_info->file_path), &ota2_info->port);

    ota2_info->bg_params.host_name = ota2_info->host_name;
    ota2_info->bg_params.file_path = ota2_info->file_path;
    ota2_info->bg_params.port      = ota2_info->port;

    wiced_log_msg(WICED_LOG_ERR, "ota2_update_uri:%s\r\n", ota2_info->update_uri);
    wiced_log_msg(WICED_LOG_ERR, "ota2_host_name :%s \r\n", ota2_info->bg_params.host_name);
    wiced_log_msg(WICED_LOG_ERR, "ota2_file_path :%s \r\n", ota2_info->bg_params.file_path);
    wiced_log_msg(WICED_LOG_ERR, "ota2_port      :%d \r\n", ota2_info->bg_params.port);

    deinit_ota2_bg = WICED_TRUE;
    if (ota2_info->bg_service == NULL)
    {
        /* start a new instance of the ota2 bg service */
        ota2_info->bg_service = wiced_ota2_service_init(&ota2_info->bg_params, ota2_info->cb_opaque);
        wiced_log_msg(WICED_LOG_ERR, "apollo_get_update() wiced_ota2_service_init() bg_service:%p \r\n", ota2_info->bg_service);
    }
    else
    {
        /* bg service already started - this is OK, just don't deinit at the end of this function */
        deinit_ota2_bg = WICED_FALSE;
    }

    /* start getting the update */
    if (ota2_info->bg_service != NULL)
    {
        wiced_ota2_service_set_debug_log_level(ota2_info->bg_service, ota2_info->log_level);

        /* add a callback */
        result = wiced_ota2_service_register_callback(ota2_info->bg_service, ota2_info->cb_func);
        if (result != WICED_SUCCESS)
        {
                wiced_log_msg(WICED_LOG_ERR, "ota2_test_get_update register callback failed! %d \r\n", result);
                wiced_ota2_service_deinit(ota2_info->bg_service);
                ota2_info->bg_service = NULL;
        }
        else
        {
            wiced_log_msg(WICED_LOG_ERR, "Download the OTA Image file - get it NOW!\r\n");
            /* NOTE: This is a blocking call! */
            result = wiced_ota2_service_check_for_updates(ota2_info->bg_service);
            if (result != WICED_SUCCESS)
            {
                    wiced_log_msg(WICED_LOG_ERR, "apollo_get_update wiced_ota2_service_check_for_updates() failed! %d \r\n", result);
            }
        }
    }

    /* only de-init if we inited in this function */
    if (deinit_ota2_bg == WICED_TRUE)
    {
        apollo_ota2_stop_bg_update(ota2_info);
    }
    return result;
}

wiced_result_t apollo_ota2_start_bg_update(apollo_ota2_service_info_t* ota2_info)
{
    wiced_result_t result = WICED_ERROR;

    if (ota2_info == NULL)
    {
        return WICED_BADARG;
    }

    /* get the image from the server & save in staging area in the backgound */
    wiced_ota2_service_uri_split(ota2_info->update_uri, ota2_info->host_name, sizeof(ota2_info->host_name),
            ota2_info->file_path, sizeof(ota2_info->file_path), &ota2_info->port);

    ota2_info->bg_params.host_name = ota2_info->host_name;
    ota2_info->bg_params.file_path = ota2_info->file_path;
    ota2_info->bg_params.port      = ota2_info->port;

    wiced_log_msg(WICED_LOG_ERR, "ota2_update_uri:%s\r\n", ota2_info->update_uri);
    wiced_log_msg(WICED_LOG_ERR, "ota2_host_name :%s \r\n", ota2_info->bg_params.host_name);
    wiced_log_msg(WICED_LOG_ERR, "ota2_file_path :%s \r\n", ota2_info->bg_params.file_path);
    wiced_log_msg(WICED_LOG_ERR, "ota2_port      :%d \r\n", ota2_info->bg_params.port);


    wiced_log_msg(WICED_LOG_ERR, "apollo_start_bg_update() ota2_info->bg_service %p \r\n", ota2_info->bg_service);
    if (ota2_info->bg_service == NULL)
    {
        /* start a new instance of the ota2 bg service */
        ota2_info->bg_service = wiced_ota2_service_init(&ota2_info->bg_params, ota2_info->cb_opaque);
        wiced_log_msg(WICED_LOG_ERR, "apollo_start_bg_update() wiced_ota2_service_init() bg_service:%p \r\n", ota2_info->bg_service);
    }

    /* start getting the update */
    if (ota2_info->bg_service != NULL)
    {
        /* add a callback */
        result = wiced_ota2_service_register_callback(ota2_info->bg_service, ota2_info->cb_func);
        if (result != WICED_SUCCESS)
        {
                wiced_log_msg(WICED_LOG_ERR, "apollo_start_bg_update() ota2_test_get_update register callback failed! %d \r\n", result);
                wiced_ota2_service_deinit(ota2_info->bg_service);
                ota2_info->bg_service = NULL;
        }
        else
        {
            wiced_log_msg(WICED_LOG_ERR, "Download the OTA Image file - start background task!\r\n");
            /* NOTE: This is a non-blocking call (async) */
            result = wiced_ota2_service_start(ota2_info->bg_service);
            if (result != WICED_SUCCESS)
            {
                    wiced_log_msg(WICED_LOG_ERR, "apollo_start_bg_update wiced_ota2_service_start() failed! %d \r\n", result);
            }
        }
    }
    return result;
}

wiced_result_t apollo_ota2_network_is_down(apollo_ota2_service_info_t* ota2_info)
{
    if ((ota2_info == NULL) || (ota2_info->bg_service == NULL))
    {
        return WICED_SUCCESS;
    }
    return wiced_ota2_service_app_network_is_down(ota2_info->bg_service);

}

wiced_result_t apollo_ota2_stop_bg_update(apollo_ota2_service_info_t* ota2_info)
{
    wiced_result_t                          result;

    if (ota2_info == NULL)
    {
        return WICED_BADARG;
    }

    result = wiced_ota2_service_deinit(ota2_info->bg_service);
    if (result != WICED_SUCCESS)
    {
        wiced_log_msg(WICED_LOG_ERR, "wiced_ota2_service_deinit() returned:%d\r\n", result);
    }
    ota2_info->bg_service = NULL;

    return result;
}


wiced_result_t apollo_ota2_check_boot_type_and_restore_DCT(apollo_dct_collection_t* dct_tables, wiced_bool_t* save_dct_tables)
{
    wiced_result_t      result = WICED_SUCCESS;
    ota2_boot_type_t    boot_type;
    WICED_LOG_LEVEL_T   log_level;

    if ((dct_tables == NULL) || (save_dct_tables == NULL))
    {
        return WICED_BADARG;
    }

    /* set the log level so we output the boot type to the console */
    log_level = wiced_log_get_level();
    wiced_log_set_level(WICED_LOG_NOTICE);

    /* determine if this is a first boot, factory reset, or after an update boot */
    boot_type = wiced_ota2_get_boot_type();
    switch( boot_type )
    {
        case OTA2_BOOT_FAILSAFE_FACTORY_RESET:
        case OTA2_BOOT_FAILSAFE_UPDATE:
        default:
            /* We should never get here! */
            wiced_log_msg(WICED_LOG_NOTICE, "Unexpected boot_type %d!\r\n", boot_type);
            /* FALL THROUGH */
        case OTA2_BOOT_NEVER_RUN_BEFORE:
            wiced_log_msg(WICED_LOG_NOTICE, "First boot EVER\r\n");
            /* Set the reboot type back to normal so we don't think we updated next reboot */
            wiced_dct_ota2_save_copy( OTA2_BOOT_NORMAL );
            break;
        case OTA2_BOOT_NORMAL:
            wiced_log_msg(WICED_LOG_NOTICE, "Normal boot.\r\n");
            break;
        case OTA2_BOOT_EXTRACT_FACTORY_RESET:   /* pre-OTA2 failsafe ota2_bootloader designation for OTA2_BOOT_FACTORY_RESET */
        case OTA2_BOOT_FACTORY_RESET:
            wiced_log_msg(WICED_LOG_NOTICE, "Factory Reset Occurred!\r\n");
            apollo_ota2_restore_settings_after_update(dct_tables, boot_type, save_dct_tables);
            /* Set the reboot type back to normal so we don't think we updated next reboot */
            wiced_dct_ota2_save_copy( OTA2_BOOT_NORMAL );
            break;
        case OTA2_BOOT_EXTRACT_UPDATE:   /* pre-OTA2 failsafe ota2_bootloader designation for OTA2_BOOT_UPDATE */
        case OTA2_BOOT_SOFTAP_UPDATE:
        case OTA2_BOOT_UPDATE:
            wiced_log_msg(WICED_LOG_NOTICE, "Update Occurred!\r\n");
            apollo_ota2_restore_settings_after_update(dct_tables, boot_type, save_dct_tables);
            /* Set the reboot type back to normal so we don't think we updated next reboot */
            wiced_dct_ota2_save_copy( OTA2_BOOT_NORMAL );
            break;
        case OTA2_BOOT_LAST_KNOWN_GOOD:
            wiced_log_msg(WICED_LOG_NOTICE, "Last Known Good used!\r\n");
            break;
    }

    /* restore the log level */
    wiced_log_set_level(log_level);

    return result;
}

wiced_result_t apollo_ota2_restore_settings_after_update( apollo_dct_collection_t* dct_tables, ota2_boot_type_t boot_type, wiced_bool_t* save_dct_tables )
{
    platform_dct_network_config_t*   dct_network;
    platform_dct_wifi_config_t*      dct_wifi;
    apollo_dct_t*                    dct_app;
#ifdef WICED_DCT_INCLUDE_BT_CONFIG
    platform_dct_bt_config_t*        dct_bt;
#endif

    /* sanity check */
    if (save_dct_tables == NULL)
    {
        return WICED_BADARG;
    }


    /* read in our configurations from the DCT Save Area and copy to RAM dct_tables */

    /* App */
    if (dct_tables->dct_app != NULL)
    {
        dct_app = (apollo_dct_t *)malloc(sizeof(apollo_dct_t));
        if (dct_app != NULL)
        {
            if (wiced_dct_ota2_read_saved_copy( dct_app, DCT_APP_SECTION, 0, sizeof(apollo_dct_t)) == WICED_SUCCESS)
            {
                uint16_t major, minor;

                /* start with basic app DCT data */
                memcpy(dct_tables->dct_app, dct_app, sizeof(apollo_dct_t));

                /* update the Software version number */
                switch (boot_type)
                {
                    case OTA2_BOOT_FAILSAFE_FACTORY_RESET:
                    case OTA2_BOOT_FAILSAFE_UPDATE:
                    case OTA2_BOOT_NEVER_RUN_BEFORE:
                    case OTA2_BOOT_NORMAL:
                    case OTA2_BOOT_LAST_KNOWN_GOOD: /* unsupported */
                    default:
                        break;
                    case OTA2_BOOT_EXTRACT_FACTORY_RESET:   /* pre-OTA2 failsafe ota2_bootloader designation for OTA2_BOOT_FACTORY_RESET */
                    case OTA2_BOOT_FACTORY_RESET:
                        if (wiced_ota2_image_get_version( WICED_OTA2_IMAGE_TYPE_FACTORY_RESET_APP, &major, &minor) == WICED_SUCCESS)
                        {
                            dct_tables->dct_app->ota2_major_version = major;
                            dct_tables->dct_app->ota2_minor_version = minor;
                        }
                        break;
                    case OTA2_BOOT_EXTRACT_UPDATE:   /* pre-OTA2 failsafe ota2_bootloader designation for OTA2_BOOT_UPDATE */
                    case OTA2_BOOT_SOFTAP_UPDATE:
                    case OTA2_BOOT_UPDATE:
                        if (wiced_ota2_image_get_version( WICED_OTA2_IMAGE_TYPE_STAGED, &major, &minor) == WICED_SUCCESS)
                        {
                            dct_tables->dct_app->ota2_major_version = major;
                            dct_tables->dct_app->ota2_minor_version = minor;
                        }
                        break;
                }
                *save_dct_tables = WICED_TRUE;
            }
            free(dct_app);
            dct_app = NULL;
        }
    }

    /* network */
    if (dct_tables->dct_network != NULL)
    {
        dct_network = (platform_dct_network_config_t *)malloc(sizeof(platform_dct_network_config_t));
        if (dct_network != NULL)
        {
            if (wiced_dct_ota2_read_saved_copy( dct_network, DCT_NETWORK_CONFIG_SECTION, 0, sizeof(platform_dct_network_config_t)) == WICED_SUCCESS)
            {
                memcpy(dct_tables->dct_network, dct_network, sizeof(platform_dct_network_config_t));
                *save_dct_tables = WICED_TRUE;
            }
            free(dct_network);
            dct_network = NULL;
        }
    }

    /* wifi */
    if (dct_tables->dct_wifi != NULL)
    {
        dct_wifi = (platform_dct_wifi_config_t *)malloc(sizeof(platform_dct_wifi_config_t));
        if (dct_wifi != NULL)
        {
            if (wiced_dct_ota2_read_saved_copy( dct_wifi, DCT_WIFI_CONFIG_SECTION, 0, sizeof(platform_dct_wifi_config_t)) == WICED_SUCCESS)
            {
                memcpy(dct_tables->dct_wifi, dct_wifi, sizeof(platform_dct_wifi_config_t));
                *save_dct_tables = WICED_TRUE;
            }
            free(dct_wifi);
            dct_wifi = NULL;
        }
    }

#ifdef WICED_DCT_INCLUDE_BT_CONFIG
    /* Bluetooth */
    if (dct_tables->dct_bt != NULL)
    {
        dct_bt = (platform_dct_bt_config_t *)malloc(sizeof(platform_dct_bt_config_t));
        if (dct_bt != NULL)
        {
            if (wiced_dct_ota2_read_saved_copy( dct_bt, DCT_BT_CONFIG_SECTION, 0, sizeof(platform_dct_bt_config_t)) == WICED_SUCCESS)
            {
                memcpy(dct_tables->dct_bt, dct_bt, sizeof(platform_dct_bt_config_t));
                *save_dct_tables = WICED_TRUE;
            }
            free(dct_bt);
            dct_bt = NULL;
        }
    }
#endif
    return WICED_SUCCESS;
}

wiced_result_t apollo_ota2_status(apollo_ota2_service_info_t* ota2_info)
{
    wiced_ota2_image_status_t status;

    if (ota2_info == NULL)
    {
        return WICED_BADARG;
    }

    if (ota2_info->bg_service == NULL)
    {
        wiced_log_msg(WICED_LOG_ERR, "OTA2 Service not initialized.\r\n");
    }
    else
    {
        wiced_log_msg(WICED_LOG_ERR, "OTA2 Service info:\r\n");
        wiced_log_msg(WICED_LOG_ERR, "                update_uri: %s\r\n", (strlen(ota2_info->update_uri) > 0) ? ota2_info->update_uri : "NO URI SET");
        wiced_log_msg(WICED_LOG_ERR, "                   cb_func: %s\r\n", (ota2_info->cb_func!= NULL) ? "Callback Set" : "NO CALLBACK SET");
        wiced_log_msg(WICED_LOG_ERR, "                 cb_opaque: %p\r\n", ota2_info->cb_opaque);
        wiced_log_msg(WICED_LOG_ERR, "              ota2_ap_info: %p\r\n", ota2_info->bg_params.ota2_ap_info);
        if (ota2_info->bg_params.ota2_ap_info != NULL)
        {
            wiced_log_msg(WICED_LOG_ERR, "              SSID: %.*s\r\n",
                    ota2_info->bg_params.ota2_ap_info->details.SSID.length, ota2_info->bg_params.ota2_ap_info->details.SSID.value);
        }
        wiced_log_msg(WICED_LOG_ERR, "        ota2_ap_list_count: %ld\r\n", ota2_info->bg_params.ota2_ap_list_count);
        wiced_log_msg(WICED_LOG_ERR, "              ota2_ap_list: %ld\r\n", ota2_info->bg_params.ota2_ap_list);
        if ( (ota2_info->bg_params.ota2_ap_list_count > 0 ) && (ota2_info->bg_params.ota2_ap_list !=  NULL))
        {
            int i;
            for (i = 0; i < ota2_info->bg_params.ota2_ap_list_count; i++)
            {
                if (ota2_info->bg_params.ota2_ap_list[i].details.SSID.length > 0)
                {
                    wiced_log_msg(WICED_LOG_ERR, "                   [%d]  SSID: %.*s\r\n", i,
                            ota2_info->bg_params.ota2_ap_list[i].details.SSID.length, ota2_info->bg_params.ota2_ap_list[i].details.SSID.value);
                }
            }
        }
        wiced_log_msg(WICED_LOG_ERR, "           default_ap_info: %ld\r\n", ota2_info->bg_params.default_ap_info);
        if (ota2_info->bg_params.default_ap_info != NULL)
        {
            wiced_log_msg(WICED_LOG_ERR, "              SSID: %.*s\r\n",
                    ota2_info->bg_params.default_ap_info->details.SSID.length, ota2_info->bg_params.default_ap_info->details.SSID.value);
        }


        wiced_ota2_service_set_debug_log_level(ota2_info->bg_service, OTA2_LOG_NOTIFY);
        wiced_ota2_service_status(ota2_info->bg_service);
        wiced_ota2_service_set_debug_log_level(ota2_info->bg_service, ota2_info->log_level);
    }

    if (wiced_ota2_image_get_status(WICED_OTA2_IMAGE_TYPE_STAGED, &status) == WICED_SUCCESS)
    {
        /* Staging Area status */
        wiced_log_msg(WICED_LOG_ERR, "OTA2 Staged Area Status:\n");
        switch(status)
        {
        case WICED_OTA2_IMAGE_INVALID:
        default:
            wiced_log_msg(WICED_LOG_ERR, "    Invalid\n");
            break;
        case WICED_OTA2_IMAGE_DOWNLOAD_IN_PROGRESS:
            wiced_log_msg(WICED_LOG_ERR, "    In Progress\n");
            break;
        case WICED_OTA2_IMAGE_DOWNLOAD_FAILED:
            wiced_log_msg(WICED_LOG_ERR, "    Failed\n");
            break;
        case WICED_OTA2_IMAGE_DOWNLOAD_UNSUPPORTED:
            wiced_log_msg(WICED_LOG_ERR, "    Unsupported\n");
            break;
        case WICED_OTA2_IMAGE_DOWNLOAD_COMPLETE:
            wiced_log_msg(WICED_LOG_ERR, "    Complete\n");
            break;
        case WICED_OTA2_IMAGE_VALID:
            wiced_log_msg(WICED_LOG_ERR, "    Valid\n");
            break;
        case WICED_OTA2_IMAGE_EXTRACT_ON_NEXT_BOOT:
            wiced_log_msg(WICED_LOG_ERR, "    Extract on Boot\n");
            break;
        case WICED_OTA2_IMAGE_DOWNLOAD_EXTRACTED:
            wiced_log_msg(WICED_LOG_ERR, "    Extracted\n");
            break;
        }
    }

    return WICED_SUCCESS;
}

#endif  /* defined(OTA2_SUPPORT) */
