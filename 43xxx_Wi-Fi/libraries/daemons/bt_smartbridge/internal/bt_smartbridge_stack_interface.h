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

/** @file
 *  Smartbridge's Interface Header with Bluetooth Stack
 */

#include "wiced_utilities.h"
#include "wiced_bt_smartbridge.h"
#include "wiced_bt_smart_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define ATT_DEFAULT_MTU           (23)
#define ATT_STANDARD_VALUE_LENGTH (ATT_DEFAULT_MTU - 3)
#define ATT_STANDARD_TIMEOUT      (500)

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
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t smartbridge_bt_interface_initialize( void );
wiced_result_t smartbridge_bt_interface_deinitialize( void );

wiced_result_t  smartbridge_bt_interface_stop_scan( void  );

wiced_result_t  smartbridge_bt_interface_start_scan( const wiced_bt_smart_scan_settings_t* setting, wiced_bt_smart_scan_complete_callback_t complete_callback, wiced_bt_smart_advertising_report_callback_t advertising_report_callback );
wiced_bool_t    smartbridge_bt_interface_is_scanning( void );

wiced_result_t  smartbridge_bt_interface_connect( const wiced_bt_smart_device_t* remote_device, const wiced_bt_smart_connection_settings_t* settings, wiced_bt_smartbridge_disconnection_callback_t disconnection_callback, wiced_bt_smartbridge_notification_callback_t notification_callback );

wiced_result_t  smartbridge_bt_interface_cancel_last_connect( wiced_bt_device_address_t address );

wiced_result_t  smartbridge_bt_interface_disconnect( uint16_t connection_handle );

wiced_result_t  smartbridge_bt_interface_add_device_to_whitelist( const wiced_bt_device_address_t* device_address, wiced_bt_smart_address_type_t address_type );

wiced_result_t  smartbridge_bt_interface_remove_device_from_whitelist( const wiced_bt_device_address_t* device_address, wiced_bt_smart_address_type_t address_type );

wiced_result_t  smartbridge_bt_interface_get_whitelist_size( uint32_t *size );

wiced_result_t  smartbridge_bt_interface_clear_whitelist( void );

wiced_result_t  smartbridge_bt_interface_set_attribute_timeout( uint32_t timeout_seconds );

wiced_result_t  smartbridge_bt_interface_set_connection_tx_power( uint16_t connection_handle, int8_t transmit_power_dbm );

wiced_result_t  smartbridge_bt_interface_set_max_concurrent_connections( uint8_t count );


wiced_result_t smartbridge_bt_interface_discover_all_primary_services( uint16_t connection_handle, wiced_bt_smart_attribute_list_t* service_list );
wiced_result_t smartbridge_bt_interface_discover_all_characteristics_in_a_service( uint16_t connection_handle, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* characteristic_list );
wiced_result_t smartbridge_bt_interface_discover_all_characteristic_descriptors( uint16_t connection_handle, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* no_value_descriptor_list );
wiced_result_t smartbridge_bt_interface_discover_primary_services_by_uuid( uint16_t connection_handle, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_list_t* service_list );
wiced_result_t smartbridge_bt_interface_find_included_services( uint16_t connection_handle, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* include_list );
wiced_result_t smartbridge_bt_interface_discover_characteristic_by_uuid( uint16_t connection_handle, const wiced_bt_uuid_t* uuid, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* characteristic_list );

wiced_result_t smartbridge_bt_interface_read_characteristic_value( uint16_t connection_handle, uint16_t handle, const wiced_bt_uuid_t* type, wiced_bt_smart_attribute_t** characteristic_value );
wiced_result_t smartbridge_bt_interface_read_long_characteristic_value( uint16_t connection_handle, uint16_t handle, const wiced_bt_uuid_t* type, wiced_bt_smart_attribute_t** characteristic_value );
wiced_result_t smartbridge_bt_interface_read_characteristic_descriptor( uint16_t connection_handle, uint16_t handle, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_t** descriptor );
wiced_result_t smartbridge_bt_interface_read_long_characteristic_descriptor( uint16_t connection_handle, uint16_t handle, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_t** descriptor );
wiced_result_t smartbridge_bt_interface_read_characteristic_values_using_uuid( uint16_t connection_handle, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_list_t* characteristic_value_list );

wiced_result_t smartbridge_bt_interface_write_characteristic_value( uint16_t connection_handle, wiced_bt_smart_attribute_t* attribute );
wiced_result_t smartbridge_bt_interface_write_long_characteristic_value( uint16_t connection_handle, wiced_bt_smart_attribute_t* attribute );
wiced_result_t smartbridge_bt_interface_write_long_characteristic_descriptor( uint16_t connection_handle, const wiced_bt_smart_attribute_t* descriptor );
wiced_result_t smartbridge_bt_interface_write_characteristic_descriptor(  uint16_t connection_handle, wiced_bt_smart_attribute_t* attribute );

wiced_result_t smartbridge_bt_interface_enable_pairing( wiced_bt_device_address_t address, wiced_bt_smart_address_type_t type, const wiced_bt_smart_security_settings_t* settings,  const char* passkey );
wiced_result_t smartbridge_bt_interface_disable_pairing( void );
wiced_result_t smartbridge_bt_interface_start_encryption( wiced_bt_device_address_t* address );

#ifdef __cplusplus
} /* extern "C" */
#endif
