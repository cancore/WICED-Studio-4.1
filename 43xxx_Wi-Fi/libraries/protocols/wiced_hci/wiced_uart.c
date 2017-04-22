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

#include <string.h>
#include "wiced_platform.h"
#include "platform_bluetooth.h"
#include "wwd_debug.h"
#include "wiced_rtos.h"

/** @file
 *
 * Fast UART
 *
 */
/******************************************************
 *                      Macros
 ******************************************************/
#define RETURN_IF_FAILURE( x )
#if 0

    do{ \
        wiced_result_t _result = (x); \
        if ( _result != WICED_SUCCESS ) \
        { \
            WICED_APP_INFO(("RETURN IF FAILURE")); \
            return _result; \
        } \
    } while( 0 )
#endif
#define BT_UART  ( WICED_UART_2 )
/******************************************************
 *                    Constants
 ******************************************************/
#define RX_BUFFER_SIZE    2048

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
wiced_uart_config_t uart_config =
{
    .baud_rate    = 115200,
    .data_width   = DATA_WIDTH_8BIT,
    .parity       = NO_PARITY,
    .stop_bits    = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_CTS_RTS,
};
wiced_ring_buffer_t rx_buffer;
uint8_t             rx_data[RX_BUFFER_SIZE];

#if defined(USE_WICED_HCI)
/* TODO: This is only for the prototype that uses WICED_HCI. Remove
 * this when the actual dev platform is available.
 */
extern const platform_gpio_t platform_gpio_pins[];
/* Bluetooth control pins. Used by libraries/bluetooth/internal/bus/UART/bt_bus.c */
const platform_gpio_t* wiced_bt_control_pins[WICED_BT_PIN_MAX] =
{
    [WICED_BT_PIN_POWER]       = NULL,
    [WICED_BT_PIN_RESET]       = &platform_gpio_pins[WICED_GPIO_11],
    [WICED_BT_PIN_HOST_WAKE]   = NULL,
    [WICED_BT_PIN_DEVICE_WAKE] = NULL
};
#endif

wiced_result_t wiced_hci_uart_init(void)
{
    /* Reset the Bluetooth */
    platform_gpio_init(wiced_bt_control_pins[WICED_BT_PIN_RESET],OUTPUT_PUSH_PULL);
    platform_gpio_output_high(wiced_bt_control_pins[WICED_BT_PIN_RESET]);
    platform_gpio_output_low(wiced_bt_control_pins[WICED_BT_PIN_RESET]);
    wiced_rtos_delay_milliseconds(500);
    platform_gpio_output_high(wiced_bt_control_pins[WICED_BT_PIN_RESET]);

    /* Initialise ring buffer */
    ring_buffer_init(&rx_buffer, rx_data, RX_BUFFER_SIZE );
    return ( wiced_uart_init( BT_UART, &uart_config, &rx_buffer ));
}

wiced_result_t wiced_hci_uart_reconfig(wiced_uart_config_t* config)
{

    /* Initialise ring buffer */
    wiced_uart_config_t bt_config;
    bt_config.baud_rate = config->baud_rate;
    bt_config.data_width = uart_config.data_width;
    bt_config.flow_control = uart_config.flow_control;
    bt_config.parity = uart_config.parity;
    bt_config.stop_bits = uart_config.stop_bits;

    ring_buffer_init(&rx_buffer, rx_data, RX_BUFFER_SIZE );
    return ( wiced_uart_init( BT_UART, &bt_config, &rx_buffer ));
}

wiced_result_t wiced_hci_uart_write(uint8_t* data, uint16_t length)
{
    return(wiced_uart_transmit_bytes( BT_UART, data, length ));
}

wiced_result_t wiced_hci_uart_read(uint8_t* data, uint32_t* length, uint32_t timeout_ms)
{
    return(wiced_uart_receive_bytes( BT_UART, data, length, timeout_ms ));
}
