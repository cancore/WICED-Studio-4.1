/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *
 * Generic types
 *
 */
#pragma once
#include "wiced.h"
#include "data_types.h"


#ifdef __cplusplus
extern "C" {
#endif

#define BD_ADDR_LEN     6
typedef uint8_t         wiced_bt_device_address_t[BD_ADDR_LEN]; /**< Device address length */

typedef uint8_t *wiced_bt_device_address_ptr_t;                 /**< Device address Pointer */

#define DEV_CLASS_LEN   3
typedef uint8_t         wiced_bt_dev_class_t[DEV_CLASS_LEN];    /**< Device class */

#define MAX_UUID_SIZE              16  /**< Maximum UUID size - 16 bytes, and structure to hold any type of UUID. */

/** UUID Type */
typedef struct
{
#define LEN_UUID_16     2
#define LEN_UUID_32     4
#define LEN_UUID_128    16

    uint16_t        len;     /**< UUID length */

    union
    {
        uint16_t    uuid16; /**< 16-bit UUID */
        uint32_t    uuid32; /**< 32-bit UUID */
        uint8_t     uuid128[MAX_UUID_SIZE]; /**< 128-bit UUID */
    } uu;

} wiced_bt_uuid_t;

#define BT_OCTET16_LEN    16                /**<  length: 16 */
typedef uint8_t BT_OCTET16[BT_OCTET16_LEN]; /**< octet array: size 16 */

#define BT_OCTET32_LEN    32
typedef uint8_t BT_OCTET32[BT_OCTET32_LEN];   /* octet array: size 32 */

/** Bluetooth QoS defintions */
typedef struct {
    uint8_t         qos_flags;              /**< TBD */
    uint8_t         service_type;           /**< service type (NO_TRAFFIC, BEST_EFFORT, or GUARANTEED) */
    uint32_t        token_rate;             /**< token rate (bytes/second) */
    uint32_t        token_bucket_size;      /**< token bucket size (bytes) */
    uint32_t        peak_bandwidth;         /**< peak bandwidth (bytes/second) */
    uint32_t        latency;                /**< latency (microseconds) */
    uint32_t        delay_variation;        /**< delay variation (microseconds) */
} wiced_bt_flow_spec_t;

/* Values for swiced_bt_flow_spec_t service_type */
#define NO_TRAFFIC      0
#define BEST_EFFORT     1
#define GUARANTEED      2

/**
 * @anchor WICED_BT_TRANSPORT_TYPE
 * @name Transport types
 * @{
 */
#define BT_TRANSPORT_BR_EDR         1       /**< BR/EDR transport */
#define BT_TRANSPORT_LE             2       /**< BLE transport */
typedef uint8_t wiced_bt_transport_t;       /**< Transport type (see @ref WICED_BT_TRANSPORT_TYPE "BT Transport Types") */

/**
 * @anchor WICED_BT_DEVICE_TYPE
 * @name Device Types
 * @{
 */
#define BT_DEVICE_TYPE_BREDR        0x01    /**< BR/EDR device */
#define BT_DEVICE_TYPE_BLE          0x02    /**< LE device */
#define BT_DEVICE_TYPE_BREDR_BLE    0x03    /**< Dual Mode device */
typedef uint8_t wiced_bt_device_type_t;     /**< Bluetooth device type (see @ref WICED_BT_DEVICE_TYPE "BT Device Types") */
/** @} WICED_BT_DEVICE_TYPE */

/**
 * @anchor WICED_BT_ADDR_TYPE
 * @name Address Types
 * @{
 */
#define BLE_ADDR_PUBLIC             0x00        /**< Public address */
#define BLE_ADDR_RANDOM             0x01        /**< Random address */
#define BLE_ADDR_PUBLIC_ID          0x02        /**< Public ID      */
#define BLE_ADDR_RANDOM_ID          0x03        /**< Random ID      */
typedef uint8_t wiced_bt_ble_address_type_t;    /**< BLE device address type (see @ref WICED_BT_ADDR_TYPE "BT Address Types")*/
#define BLE_ADDR_TYPE_MASK          (BLE_ADDR_RANDOM | BLE_ADDR_PUBLIC)
/** @} WICED_BT_ADDR_TYPE */

#ifndef wiced_bt_ble_address_t
typedef struct
{
    wiced_bt_ble_address_type_t type;
    wiced_bt_device_address_t   bda;
} wiced_bt_ble_address_t;
#endif

/* Structure defined for Vendor Specific Command complete callback */
typedef struct
{
    UINT16  opcode;
    UINT16  param_len;
    UINT8   *p_param_buf;
} tBTM_MESH_VSC_CMPL;
/* Callback function for when a vendor specific event occurs.
* The length and array of returned parameter bytes are included.
* This asynchronous event is enabled/disabled by calling wiced_bt_register_vsc_event
*/
typedef void (tBTM_MESH_VS_EVT_CB) (UINT8 len, UINT8 *p);

#ifdef __cplusplus
}
#endif
