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

#include "wiced_rtos.h"
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

#define PACKET_POOL_ID 0x1A2B3C4D
#define PACKET_ID      0x5F6A7B8D

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    BT_PACKET_OWNER_POOL,
    BT_PACKET_OWNER_STACK,
    BT_PACKET_OWNER_APP,
} bt_packet_owner_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct bt_packet      bt_packet_t;
typedef struct bt_packet_pool bt_packet_pool_t;

/******************************************************
 *                    Structures
 ******************************************************/

#pragma pack(1)
struct bt_packet
{
    uint32_t           packet_id;
    linked_list_node_t node;
    bt_packet_pool_t*  pool;
    bt_packet_owner_t  owner;
    uint8_t*           data_start;
    uint8_t*           data_end;
    uint8_t*           packet_end;
    uint8_t            packet_start[1];
};

struct bt_packet_pool
{
    uint32_t      pool_id;
    linked_list_t pool_list;
    uint8_t*      pool_buffer;
    wiced_mutex_t mutex;
    uint32_t      max_packet_count;
    uint32_t      header_size;
    uint32_t      data_size;
    uint32_t      packet_created;
    uint32_t      packet_deleted;
};
#pragma pack()

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t bt_packet_pool_init( bt_packet_pool_t* pool, uint32_t packet_count, uint32_t header_size, uint32_t data_size );

wiced_result_t bt_packet_pool_deinit( bt_packet_pool_t* pool );

wiced_result_t bt_packet_pool_allocate_packet( bt_packet_pool_t* pool, bt_packet_t** packet );

wiced_result_t bt_packet_pool_dynamic_allocate_packet( bt_packet_t** packet, uint32_t header_size, uint32_t data_size );

wiced_result_t bt_packet_pool_free_packet( bt_packet_t* packet );

#ifdef __cplusplus
} /* extern "C" */
#endif
