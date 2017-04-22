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

#include "wiced_rtos.h"
#include "platform_bluetooth.h"
#include "platform_config.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define RETURN_IF_FAILURE( x ) \
    do \
    { \
        wiced_result_t _result = (x); \
        if ( _result != WICED_SUCCESS ) \
        { \
            return _result; \
        } \
    } while( 0 )

/******************************************************
 *                    Constants
 ******************************************************/

#ifndef BLUETOOTH_CHIP_STABILIZATION_DELAY
#define BLUETOOTH_CHIP_STABILIZATION_DELAY    (500)
#endif

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

wiced_result_t bluetooth_wiced_init_config_uart( const platform_uart_config_t* bt_uart_config );

/******************************************************
 *               Variables Definitions
 ******************************************************/

static volatile wiced_ring_buffer_t rx_ring_buffer;
static volatile uint8_t             rx_data[ UART_RX_FIFO_SIZE ];

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t bluetooth_wiced_init_platform( void )
{
    if ( wiced_bt_control_pins[ WICED_BT_PIN_HOST_WAKE ] != NULL )
    {
        RETURN_IF_FAILURE( platform_gpio_init( wiced_bt_control_pins[WICED_BT_PIN_HOST_WAKE], INPUT_HIGH_IMPEDANCE ) );
    }

    if ( wiced_bt_control_pins[ WICED_BT_PIN_DEVICE_WAKE ] != NULL )
    {
        RETURN_IF_FAILURE( platform_gpio_init( wiced_bt_control_pins[ WICED_BT_PIN_DEVICE_WAKE ], OUTPUT_PUSH_PULL ) );
        RETURN_IF_FAILURE( platform_gpio_output_low( wiced_bt_control_pins[ WICED_BT_PIN_DEVICE_WAKE ] ) );
        wiced_rtos_delay_milliseconds( 100 );
    }

    /* Configure Reg Enable pin to output. Set to HIGH */
    if ( wiced_bt_control_pins[ WICED_BT_PIN_POWER ] != NULL )
    {
        RETURN_IF_FAILURE( platform_gpio_init( wiced_bt_control_pins[ WICED_BT_PIN_POWER ], OUTPUT_OPEN_DRAIN_PULL_UP ) );
        RETURN_IF_FAILURE( platform_gpio_output_high( wiced_bt_control_pins[ WICED_BT_PIN_POWER ] ) );
    }

    if ( wiced_bt_uart_config.flow_control == FLOW_CONTROL_DISABLED )
    {
        /* Configure RTS pin to output. Set to HIGH */
        RETURN_IF_FAILURE( platform_gpio_init( wiced_bt_uart_pins[WICED_BT_PIN_UART_RTS], OUTPUT_OPEN_DRAIN_PULL_UP ) );
        RETURN_IF_FAILURE( platform_gpio_output_high( wiced_bt_uart_pins[WICED_BT_PIN_UART_RTS] ) );

        /* Configure CTS pin to input pull-up */
        RETURN_IF_FAILURE( platform_gpio_init( wiced_bt_uart_pins[WICED_BT_PIN_UART_CTS], INPUT_PULL_UP ) );
    }

    if ( wiced_bt_control_pins[ WICED_BT_PIN_RESET ] != NULL )
    {
        RETURN_IF_FAILURE( platform_gpio_init( wiced_bt_control_pins[ WICED_BT_PIN_RESET ], OUTPUT_PUSH_PULL ) );
        RETURN_IF_FAILURE( platform_gpio_output_high( wiced_bt_control_pins[ WICED_BT_PIN_RESET ] ) );

        /* Configure USART comms */
        RETURN_IF_FAILURE( bluetooth_wiced_init_config_uart( &wiced_bt_uart_config ) );

        /* Reset bluetooth chip */
        RETURN_IF_FAILURE( platform_gpio_output_low( wiced_bt_control_pins[ WICED_BT_PIN_RESET ] ) );
        wiced_rtos_delay_milliseconds( 10 );
        RETURN_IF_FAILURE( platform_gpio_output_high( wiced_bt_control_pins[ WICED_BT_PIN_RESET ] ) );
    }
    else
    {
        /* Configure USART comms */
        RETURN_IF_FAILURE( bluetooth_wiced_init_config_uart( &wiced_bt_uart_config ) );
    }

    wiced_rtos_delay_milliseconds( BLUETOOTH_CHIP_STABILIZATION_DELAY );

    if ( wiced_bt_uart_config.flow_control == FLOW_CONTROL_DISABLED )
    {
        /* Bluetooth chip is ready. Pull host's RTS low */
        RETURN_IF_FAILURE( platform_gpio_output_low( wiced_bt_uart_pins[WICED_BT_PIN_UART_RTS] ) );
    }

    /* Wait for Bluetooth chip to pull its RTS (host's CTS) low. From observation using CRO, it takes the bluetooth chip > 170ms to pull its RTS low after CTS low */
    while ( platform_gpio_input_get( wiced_bt_uart_pins[ WICED_BT_PIN_UART_CTS ] ) == WICED_TRUE )
    {
        wiced_rtos_delay_milliseconds( 10 );
    }
    return WICED_SUCCESS;
}

wiced_result_t bluetooth_wiced_init_config_uart( const platform_uart_config_t* bt_uart_config )
{
    wiced_result_t result;

    ring_buffer_init( (wiced_ring_buffer_t*) &rx_ring_buffer, (uint8_t*) rx_data, sizeof( rx_data ) );
    result = platform_uart_init( wiced_bt_uart_driver, wiced_bt_uart_peripheral, bt_uart_config, (wiced_ring_buffer_t*) &rx_ring_buffer );
    return result;
}
