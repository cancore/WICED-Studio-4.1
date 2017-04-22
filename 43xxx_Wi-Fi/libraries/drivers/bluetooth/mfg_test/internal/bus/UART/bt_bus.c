/**
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
#include "bt_bus.h"
#include "platform_config.h"
#include "wwd_assert.h"
#include "wiced.h"
#include "wiced_rtos.h"
#include "wiced_utilities.h"
#include "platform_bluetooth.h"

/******************************************************
 *                      Macros
 ******************************************************/

/* Verify if WICED Platform API returns success.
 * Otherwise, returns the error code immediately.
 * Assert in DEBUG build.
 */
#define RETURN_IF_FAILURE( x ) \
    do \
    { \
        wiced_result_t _result = (x); \
        if ( _result != WICED_SUCCESS ) \
        { \
            return _result; \
        } \
    } while( 0 )


/* Macro for checking of bus is initialised */
#define IS_BUS_INITIALISED( ) \
do \
{ \
    if ( bus_initialised == WICED_FALSE ) \
    { \
        wiced_assert( "bus uninitialised", 0!=0 ); \
        return WICED_ERROR; \
    } \
}while ( 0 )

/******************************************************
 *                    Constants
 ******************************************************/

/* Should be overridden by application. If undefined, set to 512 bytes. */
#ifndef BT_BUS_RX_FIFO_SIZE
#define BT_BUS_RX_FIFO_SIZE (512)
#endif

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
 *               Static Function Declarations
 ******************************************************/

static wiced_result_t bluetooth_wiced_init_platform   ( void );
wiced_result_t bluetooth_wiced_init_config_uart( const platform_uart_config_t* bt_uart_config );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static volatile wiced_bool_t bus_initialised = WICED_FALSE;
static volatile wiced_bool_t device_powered  = WICED_FALSE;

/* RX ring buffer. Bluetooth chip UART receive can be asynchronous, therefore a ring buffer is required */
static volatile wiced_ring_buffer_t rx_ring_buffer;
static volatile uint8_t             rx_data[BT_BUS_RX_FIFO_SIZE];

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t bt_bus_init( void )
{
    wiced_result_t init_result = WICED_SUCCESS;
    if ( bus_initialised == WICED_FALSE )
    {
        init_result = bluetooth_wiced_init_platform( );
        if ( init_result == WICED_SUCCESS )
        {
            bus_initialised = WICED_TRUE;
        }
    }
    return init_result;
}

wiced_result_t bt_bus_deinit( void )
{
    if ( bus_initialised == WICED_FALSE )
    {
        return WICED_BT_SUCCESS;
    }

    if ( wiced_bt_control_pins[ WICED_BT_PIN_RESET ] != NULL )
    {
        RETURN_IF_FAILURE( platform_gpio_output_low( wiced_bt_control_pins[WICED_BT_PIN_RESET] ) );
    }

    if ( wiced_bt_uart_config.flow_control == FLOW_CONTROL_DISABLED )
    {
        RETURN_IF_FAILURE( platform_gpio_output_high( wiced_bt_uart_pins[WICED_BT_PIN_UART_RTS] ) ); // RTS deasserted
    }

    if ( wiced_bt_control_pins[ WICED_BT_PIN_POWER ] != NULL )
    {
        RETURN_IF_FAILURE( platform_gpio_output_low ( wiced_bt_control_pins[WICED_BT_PIN_POWER] ) ); // Bluetooth chip regulator off
    }

    device_powered = WICED_FALSE;

    /* Deinitialise UART */
    RETURN_IF_FAILURE( platform_uart_deinit( wiced_bt_uart_driver ) );
    bus_initialised = WICED_FALSE;

    return WICED_BT_SUCCESS;
}

wiced_result_t bt_bus_transmit( const uint8_t* data_out, uint32_t size )
{
    IS_BUS_INITIALISED();

    BT_BUS_WAIT_UNTIL_READY();

    RETURN_IF_FAILURE( platform_uart_transmit_bytes( wiced_bt_uart_driver, data_out, size ) );

    return WICED_BT_SUCCESS;
}

wiced_result_t bt_bus_receive( uint8_t* data_in, uint32_t size, uint32_t timeout_ms )
{
    platform_result_t platform_result;

    IS_BUS_INITIALISED();

    platform_result = platform_uart_receive_bytes( wiced_bt_uart_driver, (void*)data_in, &size, timeout_ms );
    if (platform_result != PLATFORM_SUCCESS)
    {
        return WICED_BT_ERROR;
    }
    return WICED_BT_SUCCESS;
}

wiced_bool_t bt_bus_is_ready( void )
{
    return ( bus_initialised == WICED_FALSE ) ? WICED_FALSE : ( ( platform_gpio_input_get( wiced_bt_uart_pins[WICED_BT_PIN_UART_CTS] ) == WICED_TRUE ) ? WICED_FALSE : WICED_TRUE );
}

wiced_bool_t bt_bus_is_on( void )
{
    return device_powered;
}

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
