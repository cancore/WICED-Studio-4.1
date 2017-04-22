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
 *  MQTT session APIs and types.
 *
 *  Internal types not to be included directly by applications.
 */
#pragma once

#include "mqtt_api.h"
#include "mqtt_frame.h"
#include "linked_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define     SESSION_ITEMS_SIZE      (WICED_MQTT_QUEUE_SIZE * 2)

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *              MQTT session type Definitions
 ******************************************************/
typedef union wiced_mqtt_session_item_agrs_u
{
    mqtt_publish_arg_t     publish;
    mqtt_subscribe_arg_t   subscribe;
    mqtt_unsubscribe_arg_t unsubscribe;
    mqtt_pubrec_arg_t      pubrec;
    mqtt_pubrel_arg_t      pubrel;
}wiced_mqtt_session_item_args_t;

typedef struct mqtt_session_item_s
{
    struct list_head                list;
    mqtt_frame_type_t         type;
    wiced_mqtt_session_item_args_t  args;
}wiced_mqtt_session_item_t;

typedef struct mqtt_session_s
{
    struct list_head used_list;
    struct list_head nonused_list;
    wiced_mqtt_session_item_t items[SESSION_ITEMS_SIZE];
}mqtt_session_t;
/******************************************************
 *             Content Frame Type Definitions
 ******************************************************/

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
wiced_result_t mqtt_session_init                 (                                                                      mqtt_session_t *session );
wiced_result_t mqtt_session_add_item             ( mqtt_frame_type_t type, void *args                           , mqtt_session_t *session );
wiced_result_t mqtt_session_remove_item          ( mqtt_frame_type_t type, uint16_t packet_id                   , mqtt_session_t *session );
wiced_result_t mqtt_session_item_exist           ( mqtt_frame_type_t type, uint16_t packet_id                   , mqtt_session_t *session );
wiced_result_t mqtt_session_iterate_through_items( wiced_result_t (*iter_func)(mqtt_frame_type_t type, void *arg , void *p_user ), void* p_user, mqtt_session_t *session);

#ifdef __cplusplus
} /* extern "C" */
#endif
