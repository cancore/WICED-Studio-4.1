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
 *  Frame packing and unpacking functions
 */

#include "wiced.h"
#include "mqtt_internal.h"
#include "mqtt_connection.h"
#include "mqtt_frame.h"
#include "mqtt_session.h"

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

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

/******************************************************
 *               Interface functions
 ******************************************************/
wiced_result_t mqtt_session_init( mqtt_session_t *session )
{
    uint8_t index;

    /* Initialize both list heads used and non used */
    INIT_LIST_HEAD( &session->used_list );
    INIT_LIST_HEAD( &session->nonused_list );

    /* Add all items to non used list */
    for ( index = 0; index < sizeof(session->items) / sizeof(session->items[0]); index++ )
    {
        list_add( &session->items[index].list, &session->nonused_list );
    }

    return WICED_SUCCESS;
}

wiced_result_t mqtt_session_add_item( mqtt_frame_type_t type, void *args, mqtt_session_t *session)
{
    wiced_mqtt_session_item_t *item;

    /* Check if there are any non used slots to grab */
    if ( list_empty( &session->nonused_list ) )
    {
        return WICED_ERROR;
    }

    /* Get first item in the empty list */
    item = list_entry( session->nonused_list.next, wiced_mqtt_session_item_t, list );
    list_del( &item->list );

    /* Fill data */
    item->type = type;
    if ( type == MQTT_PACKET_TYPE_PUBLISH )
    {
        item->args.publish = *((mqtt_publish_arg_t *) args);
    }
    else if ( type == MQTT_PACKET_TYPE_SUBSCRIBE )
    {
        item->args.subscribe = *((mqtt_subscribe_arg_t *) args);
    }
    else if ( type == MQTT_PACKET_TYPE_UNSUBSCRIBE )
    {
        item->args.unsubscribe = *((mqtt_unsubscribe_arg_t *) args);
    }
    else if ( type == MQTT_PACKET_TYPE_PUBREC )
    {
        item->args.pubrec = *((mqtt_pubrec_arg_t *) args);
    }
    else if ( type == MQTT_PACKET_TYPE_PUBREL )
    {
        item->args.pubrel = *((mqtt_pubrel_arg_t *) args);
    }
    else
    {
        list_add( &item->list, &session->nonused_list );
        return WICED_ERROR;
    }

    /* Add it to the used list */
    list_add_tail( &item->list, &session->used_list );

    return WICED_SUCCESS;
}

wiced_result_t mqtt_session_remove_item( mqtt_frame_type_t type, uint16_t packet_id, mqtt_session_t *session)
{
    struct list_head *pos, *q;
    wiced_mqtt_session_item_t *item = NULL;

    if ( list_empty( &session->used_list ) )
    {
        return WICED_ERROR;
    }

    list_for_each_safe( pos, q, &session->used_list )
    {
        item = list_entry( pos, wiced_mqtt_session_item_t, list );
        if ( ( ( type == MQTT_PACKET_TYPE_PUBLISH     ) && ( item->args.publish.packet_id     == packet_id ) ) ||
             ( ( type == MQTT_PACKET_TYPE_SUBSCRIBE   ) && ( item->args.subscribe.packet_id   == packet_id ) ) ||
             ( ( type == MQTT_PACKET_TYPE_UNSUBSCRIBE ) && ( item->args.unsubscribe.packet_id == packet_id ) ) ||
             ( ( type == MQTT_PACKET_TYPE_PUBREC      ) && ( item->args.pubrec.packet_id      == packet_id ) ) ||
             ( ( type == MQTT_PACKET_TYPE_PUBREL      ) && ( item->args.pubrel.packet_id      == packet_id ) ) )
        {
            /* Remove item from used lists */
            list_del( &item->list );
            /* Add Item to non used list */
            list_add( &item->list, &session->nonused_list );

            return WICED_SUCCESS;
        }
    }
    /* No match */
    return WICED_ERROR;
}


wiced_result_t mqtt_session_item_exist( mqtt_frame_type_t type, uint16_t packet_id, mqtt_session_t *session)
{
    struct list_head *pos;
    wiced_mqtt_session_item_t *item = NULL;

    if ( list_empty( &session->used_list ) )
    {
        return WICED_ERROR;
    }

    list_for_each( pos, &session->used_list )
    {
        item = list_entry( pos, wiced_mqtt_session_item_t, list );
        if ( ( ( type == MQTT_PACKET_TYPE_PUBLISH     ) && ( item->args.publish.packet_id == packet_id     ) ) ||
             ( ( type == MQTT_PACKET_TYPE_SUBSCRIBE   ) && ( item->args.subscribe.packet_id == packet_id   ) ) ||
             ( ( type == MQTT_PACKET_TYPE_UNSUBSCRIBE ) && ( item->args.unsubscribe.packet_id == packet_id ) ) ||
             ( ( type == MQTT_PACKET_TYPE_PUBREC      ) && ( item->args.pubrec.packet_id == packet_id      ) ) ||
             ( ( type == MQTT_PACKET_TYPE_PUBREL      ) && ( item->args.pubrel.packet_id == packet_id      ) ) )
        {
            return WICED_SUCCESS;
        }
    }
    /* No match */
    return WICED_ERROR;
}

wiced_result_t mqtt_session_iterate_through_items( wiced_result_t (*iter_func)(mqtt_frame_type_t type, void *arg, void *p_user ), void* p_user, mqtt_session_t *session)
{
    struct list_head *pos;
    wiced_mqtt_session_item_t *item = NULL;

    if ( list_empty( &session->used_list ) )
    {
        return WICED_SUCCESS;
    }

    list_for_each( pos, &session->used_list )
    {
        item = list_entry( pos, wiced_mqtt_session_item_t, list );
        if ( iter_func( item->type, &item->args, p_user ) != WICED_SUCCESS )
        {
            return WICED_ERROR;
        }
    }
    /* No match */
    return WICED_SUCCESS;
}
