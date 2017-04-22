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
 *  Defines functions for using the WICED Bluetooth Framework
 */

#pragma once

#include "wiced.h"
#include "wiced_bt_types.h"
#include "wiced_bt_dev.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/** WICED Bluetooth Framework mode of operation
 */
typedef enum
{
    WICED_BT_MPAF_MODE, /**< The framework uses Broadcom's proprietary Multi-Profile Application Framework (MPAF). The entire Bluetooth stack runs on the controller. The host controls the controller using remote procedure calls (RPC) */
    WICED_BT_HCI_MODE,  /**< The framework uses Bluetooth's standard Host Controller Interface (HCI). The upper stack runs on the host and the lower stack runs on the controller                                                         */
} wiced_bt_mode_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/** WICED Bluetooth packet.
 *  An opaque data type representing a generic, zero-copy packet used for transporting data to/from the Bluetooth controller.
 */
typedef struct bt_packet wiced_bt_packet_t;

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/*****************************************************************************/
/** @defgroup wicedbt      Bluetooth
 *
 *  WICED Bluetooth Framework Functions
 */
/*****************************************************************************/

/*****************************************************************************/
/** @addtogroup btfwmgmt  Framework
 *  @ingroup wicedbt
 *
 *  Bluetooth Framework Management Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Initialise the WICED Bluetooth Framework
 *
 * This function initialises the local Bluetooth device and the framework core
 * components to operate in the mode specified. Upon return, the device is
 * powered and discoverable.
 *
 * @note To switch mode, invoke @ref wiced_bt_deinit to tear down the current
 *       operating mode, and call this function with the desired mode.
 *
 * @param mode        : The framework mode of operation
 * @param device_name : A user-friendly name of the local Bluetooth device. A
 *                      name longer than 21 characters will be truncated.
 *
 * @return    WICED_SUCCESS : on success;
 *            WICED_ERROR   : if an error occurred
 */

wiced_result_t wiced_bt_init( wiced_bt_mode_t mode, const char* device_name );

/** Deinitialise the WICED Bluetooth Framework
 *
 * This function tears down all active framework components. Depending on the
 * hardware platform used, it may also power down the Bluetooth device.
 *
 * @return    WICED_SUCCESS : on success;
 *            WICED_ERROR   : if an error occurred
 */
wiced_result_t wiced_bt_deinit( void );

/** Initialise the device address of the local Bluetooth device
 *
 * This function provides users with a option to overwrite the default address of
 * the local Bluetooth device with the address provided. Once called, @ref wiced_bt_init()
 * overwrites the default address with the address provided. Users can selectively
 * overwrite bits of the default address by setting the correspondping bits in the
 * 'mask' argument to 1.
 *
 * @warning When used, this function *MUST* be called before wiced_bt_init()
 *
 * @param[in] address : new address
 * @param[in] mask    : masking bits
 *
 * @return    WICED_SUCCESS : on success;
 *            WICED_ERROR   : if an error occurred
 */
wiced_result_t wiced_bt_init_address( const wiced_bt_device_address_t* address, const wiced_bt_device_address_t* mask );


/** Start manufacturing test mode
 *
 * @param[in] config : Configuration of the UART peripheral that connects to the host PC
 *
 * @return    WICED_SUCCESS : on success;
 *            WICED_ERROR   : if an error occurred
 */
wiced_result_t wiced_bt_start_mfgtest_mode( const wiced_uart_config_t* config );

/** @} */

/*****************************************************************************/
/** @addtogroup btdevmgmt  Device
 *  @ingroup wicedbt
 *
 *  Bluetooth Device Management Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Retrieve the device address of the local Bluetooth device
 *
 * @param[out] address : device address
 *
 * @return    WICED_TRUE  : is device address successfully retrieved;
 *            WICED_FALSE : if not or if an error occurred
 */
wiced_result_t wiced_bt_device_get_address( wiced_bt_device_address_t* address );

/** Retrieve the user-friendly name of the local Bluetooth device
 *
 * @return pointer to the device name string
 */
const char*    wiced_bt_device_get_name( void );

/** Check if the local Bluetooth device is powered
 *
 * @return    WICED_TRUE  : if powered on;
 *            WICED_FALSE : if not powered or if an error occurred
 */
wiced_bool_t   wiced_bt_device_is_on( void );

/** Check if the local Bluetooth device is connectable state
 *
 * @return    WICED_TRUE  : is in connectable state;
 *            WICED_FALSE : if not or if an error occurred
 */
wiced_bool_t   wiced_bt_device_is_connectable( void );

/** Check if the local Bluetooth device is discoverable state
 *
 * @return    WICED_TRUE  : is in discoverable state;
 *            WICED_FALSE : if not or if an error occurred
 */
wiced_bool_t   wiced_bt_device_is_discoverable( void );


/** @} */

/*****************************************************************************/
/** @addtogroup btpktmgmt  Packet
 *  @ingroup wicedbt
 *
 *  Bluetooth Packet Management Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Delete a WICED Bluetooth packet
 *
 * This function returns the packet's memory space back to the source, allowing
 * for reuse.
 *
 * @param packet : The pointer to the packet to delete

 * @return    WICED_SUCCESS : on success;
 *            WICED_BADARG  : if bad argument(s) are inserted;
 *            WICED_ERROR   : if an error occurred.
 */
wiced_result_t wiced_bt_packet_delete( wiced_bt_packet_t* packet );

/** Get a pointer to the packet data
 *
 * This function retrieves a pointer to the start of the data section in the
 * packet. It also returns the current data size and the remaining data space
 * in the packet.
 *
 * @param packet            : The pointer to the packet
 * @param data              : A pointer that will receive the pointer to the
 *                            start of the data section in the packet
 * @param current_data_size : A pointer that will receive the size of the data
 *                            in the packet in bytes
 * @param available_space   : A pointer that will receive the available data
 *                            space in the packet in bytes
 *
 * @return    WICED_SUCCESS : on success;
 *            WICED_BADARG  : if bad argument(s) are inserted;
 *            WICED_ERROR   : if an error occurred.
 */
wiced_result_t wiced_bt_packet_get_data( const wiced_bt_packet_t* packet, uint8_t** data, uint32_t* current_data_size, uint32_t* available_space );

/** Set the end of the packet data
 *
 * This function updates the end of the data section and the data size in the
 * packet.
 *
 * @param packet   : The pointer to the packet
 * @param data_end : The pointer to the end of the data section in the packet

 * @return    WICED_SUCCESS : on success;
 *            WICED_BADARG  : if bad argument(s) are inserted;
 *            WICED_ERROR   : if an error occurred
 */
wiced_result_t wiced_bt_packet_set_data_end( wiced_bt_packet_t* packet, const uint8_t* data_end );

/** @} */

#ifdef __cplusplus
} /* extern "C" */
#endif
