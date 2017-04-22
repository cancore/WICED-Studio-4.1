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
 * CY FM4 UART Implementation
 */

#include <stdint.h>
#include <string.h>
#include "wiced_platform.h"
#include "platform_config.h"
#include "platform_peripheral.h"
#include "platform_sleep.h"
#include "platform_assert.h"
#include "wwd_assert.h"
#include "wiced_rtos.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define PLATFORM_UART_MFS_CHANNEL_MAX 10
#define PLATFORM_UART_MFS_PIN_INDEX_MAX 2
#define MFSN_UART_TX_BUFFER_SIZE 8
#define MFSN_UART_RX_BUFFER_SIZE 8
#define MFSN_UART_TX_MAX_WAIT_TIME_MS (100)
#define MFSN_UART_RX_MAX_WAIT_TIME_MS (200)

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct mfsn_uart_buffer
{
    wiced_ring_buffer_t tx_buffer;
    uint8_t tx_data[MFSN_UART_TX_BUFFER_SIZE];
    wiced_ring_buffer_t rx_buffer;
    uint8_t rx_data[MFSN_UART_RX_BUFFER_SIZE];
} mfsn_uart_buffer_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* RX ring buffers for MFS UART channels */
static mfsn_uart_buffer_t mfs_uart_buffer[PLATFORM_UART_MFS_CHANNEL_MAX];

#ifndef CYFM4_MFS_UART_POLL_MODE
/* Extern'ed from platforms/<CYFM4_Platform>/platform.c */
extern platform_uart_driver_t platform_uart_drivers[];

/* IRQ callbacks for MFS UART channels */
void mfs_uart0_tx_irq_cb(void);
void mfs_uart0_rx_irq_cb(void);
void mfs_uart1_tx_irq_cb(void);
void mfs_uart1_rx_irq_cb(void);
void mfs_uart2_tx_irq_cb(void);
void mfs_uart2_rx_irq_cb(void);
void mfs_uart3_tx_irq_cb(void);
void mfs_uart3_rx_irq_cb(void);
void mfs_uart4_tx_irq_cb(void);
void mfs_uart4_rx_irq_cb(void);
void mfs_uart5_tx_irq_cb(void);
void mfs_uart5_rx_irq_cb(void);
void mfs_uart6_tx_irq_cb(void);
void mfs_uart6_rx_irq_cb(void);
void mfs_uart7_tx_irq_cb(void);
void mfs_uart7_rx_irq_cb(void);
void mfs_uart8_tx_irq_cb(void);
void mfs_uart8_rx_irq_cb(void);
void mfs_uart9_tx_irq_cb(void);
void mfs_uart9_rx_irq_cb(void);

static stc_uart_irq_cb_t mfs_uart_irq_cb[PLATFORM_UART_MFS_CHANNEL_MAX] =
{
    [0] =
    {
        .pfnTxIrqCb = mfs_uart0_tx_irq_cb,
        .pfnRxIrqCb = mfs_uart0_rx_irq_cb,
        .pfnTxIdleCb = NULL,
        .pfnTxFifoIrqCb = NULL,
    },
    [1] =
    {
        .pfnTxIrqCb = mfs_uart1_tx_irq_cb,
        .pfnRxIrqCb = mfs_uart1_rx_irq_cb,
        .pfnTxIdleCb = NULL,
        .pfnTxFifoIrqCb = NULL,
    },
    [2] =
    {
        .pfnTxIrqCb = mfs_uart2_tx_irq_cb,
        .pfnRxIrqCb = mfs_uart2_rx_irq_cb,
        .pfnTxIdleCb = NULL,
        .pfnTxFifoIrqCb = NULL,
    },
    [3] =
    {
        .pfnTxIrqCb = mfs_uart3_tx_irq_cb,
        .pfnRxIrqCb = mfs_uart3_rx_irq_cb,
        .pfnTxIdleCb = NULL,
        .pfnTxFifoIrqCb = NULL,
    },
    [4] =
    {
        .pfnTxIrqCb = mfs_uart4_tx_irq_cb,
        .pfnRxIrqCb = mfs_uart4_rx_irq_cb,
        .pfnTxIdleCb = NULL,
        .pfnTxFifoIrqCb = NULL,
    },
    [5] =
    {
        .pfnTxIrqCb = mfs_uart5_tx_irq_cb,
        .pfnRxIrqCb = mfs_uart5_rx_irq_cb,
        .pfnTxIdleCb = NULL,
        .pfnTxFifoIrqCb = NULL,
    },
    [6] =
    {
        .pfnTxIrqCb = mfs_uart6_tx_irq_cb,
        .pfnRxIrqCb = mfs_uart6_rx_irq_cb,
        .pfnTxIdleCb = NULL,
        .pfnTxFifoIrqCb = NULL,
    },
    [7] =
    {
        .pfnTxIrqCb = mfs_uart7_tx_irq_cb,
        .pfnRxIrqCb = mfs_uart7_rx_irq_cb,
        .pfnTxIdleCb = NULL,
        .pfnTxFifoIrqCb = NULL,
    },
    [8] =
    {
        .pfnTxIrqCb = mfs_uart8_tx_irq_cb,
        .pfnRxIrqCb = mfs_uart8_rx_irq_cb,
        .pfnTxIdleCb = NULL,
        .pfnTxFifoIrqCb = NULL,
    },
    [9] =
    {
        .pfnTxIrqCb = mfs_uart9_tx_irq_cb,
        .pfnRxIrqCb = mfs_uart9_rx_irq_cb,
        .pfnTxIdleCb = NULL,
        .pfnTxFifoIrqCb = NULL,
    },
};

typedef host_semaphore_type_t uart_semaphore_type_t;

static host_semaphore_type_t mfs_uart_tx_semaphore[PLATFORM_UART_MFS_CHANNEL_MAX];
static host_semaphore_type_t mfs_uart_rx_semaphore[PLATFORM_UART_MFS_CHANNEL_MAX];
#endif /* !CYFM4_MFS_UART_POLL_MODE */

/******************************************************
 *               Function Definitions
 ******************************************************/

/* Mapping of MFS UART port to MFS UART channel instance */
static volatile stc_mfsn_uart_t* platform_uart_get_mfs_channel( uint32_t port )
{
    volatile stc_mfsn_uart_t* mfs_uart_channel;

    if ( port >= PLATFORM_UART_MFS_CHANNEL_MAX )
    {
        return NULL;
    }

    switch ( port )
    {
        case 0:
            mfs_uart_channel = &UART0;
            break;

        case 1:
            mfs_uart_channel = &UART1;
            break;

        case 2:
            mfs_uart_channel = &UART2;
            break;

        case 3:
            mfs_uart_channel = &UART3;
            break;

        case 4:
            mfs_uart_channel = &UART4;
            break;

        case 5:
            mfs_uart_channel = &UART5;
            break;

        case 6:
            mfs_uart_channel = &UART6;
            break;

        case 7:
            mfs_uart_channel = &UART7;
            break;

        case 8:
            mfs_uart_channel = &UART8;
            break;

        case 9:
            mfs_uart_channel = &UART9;
            break;

        default:
            mfs_uart_channel = NULL;
            break;
    }

    return mfs_uart_channel;
}

/* Mapping of MFS UART port and input pin index to MFS UART RX pin function */
static platform_result_t platform_uart_set_input_pin(uint32_t port, uint32_t pin_index)
{
    if ( (port >= PLATFORM_UART_MFS_CHANNEL_MAX) || ( pin_index >= PLATFORM_UART_MFS_PIN_INDEX_MAX) )
    {
        return PLATFORM_UNSUPPORTED;
    }

    switch ( port )
    {
        case 0:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SIN0_0();
                    break;

                case 1:
                    SetPinFunc_SIN0_1();
                    break;
            }
            break;

        case 1:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SIN1_0();
                    break;

                case 1:
                    SetPinFunc_SIN1_1();
                    break;
            }
            break;

        case 2:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SIN2_0();
                    break;

                case 1:
                    SetPinFunc_SIN2_1();
                    break;
            }
            break;

        case 3:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SIN3_0();
                    break;

                case 1:
                    SetPinFunc_SIN3_1();
                    break;
            }
            break;

        case 4:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SIN4_0();
                    break;

                case 1:
                    SetPinFunc_SIN4_1();
                    break;
            }
            break;

        case 5:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SIN5_0();
                    break;

                case 1:
                    SetPinFunc_SIN5_1();
                    break;
            }
            break;

        case 6:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SIN6_0();
                    break;

                case 1:
                    SetPinFunc_SIN6_1();
                    break;
            }
            break;

        case 7:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SIN7_0();
                    break;

                case 1:
                    SetPinFunc_SIN7_1();
                    break;
            }
            break;

        case 8:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SIN8_0();
                    break;

                case 1:
                    SetPinFunc_SIN8_1();
                    break;
            }
            break;

        case 9:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SIN9_0();
                    break;

                case 1:
                    SetPinFunc_SIN9_1();
                    break;
            }
            break;
    }

    return PLATFORM_SUCCESS;
}

/* Mapping of MFS UART port and output pin index to MFS UART TX pin function */
static platform_result_t platform_uart_set_output_pin(uint32_t port, uint32_t pin_index)
{
    if ( (port >= PLATFORM_UART_MFS_CHANNEL_MAX) || ( pin_index >= PLATFORM_UART_MFS_PIN_INDEX_MAX) )
    {
        return PLATFORM_UNSUPPORTED;
    }

    switch ( port )
    {
        case 0:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SOT0_0();
                    break;

                case 1:
                    SetPinFunc_SOT0_1();
                    break;
            }
            break;

        case 1:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SOT1_0();
                    break;

                case 1:
                    SetPinFunc_SOT1_1();
                    break;
            }
            break;

        case 2:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SOT2_0();
                    break;

                case 1:
                    SetPinFunc_SOT2_1();
                    break;
            }
            break;

        case 3:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SOT3_0();
                    break;

                case 1:
                    SetPinFunc_SOT3_1();
                    break;
            }
            break;

        case 4:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SOT4_0();
                    break;

                case 1:
                    SetPinFunc_SOT4_1();
                    break;
            }
            break;

        case 5:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SOT5_0();
                    break;

                case 1:
                    SetPinFunc_SOT5_1();
                    break;
            }
            break;

        case 6:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SOT6_0();
                    break;

                case 1:
                    SetPinFunc_SOT6_1();
                    break;
            }
            break;

        case 7:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SOT7_0();
                    break;

                case 1:
                    SetPinFunc_SOT7_1();
                    break;
            }
            break;

        case 8:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SOT8_0();
                    break;

                case 1:
                    SetPinFunc_SOT8_1();
                    break;
            }
            break;

        case 9:
            switch ( pin_index )
            {
                case 0:
                    SetPinFunc_SOT9_0();
                    break;

                case 1:
                    SetPinFunc_SOT9_1();
                    break;
            }
            break;
    }

    return PLATFORM_SUCCESS;
}

#ifndef CYFM4_MFS_UART_POLL_MODE
static platform_result_t platform_uart_transmit_bytes_irq( platform_uart_driver_t* driver, const uint8_t* data_out, uint32_t size )
{
    platform_result_t result = PLATFORM_ERROR;

    if ( ( driver == NULL ) || ( data_out == NULL ) || ( size == 0 ) )
    {
        wiced_assert( "bad argument", 0 );
        return PLATFORM_BADARG;
    }

    wiced_assert( "not inited", ( driver->tx_buffer != NULL ) );

    if ( driver->tx_buffer != NULL )
    {
        uint32_t write_index = 0;
        uint32_t data_size  = 0;
        uint32_t free_size  = 0;

        result = PLATFORM_SUCCESS;

        while ( size > 0 )
        {
            wwd_result_t sem_result;
            /* Get the semaphore whenever data needs to be produced into the ring buffer */
            sem_result = host_rtos_get_semaphore( &mfs_uart_tx_semaphore[driver->peripheral->port], MFSN_UART_TX_MAX_WAIT_TIME_MS, WICED_TRUE );
            if ( sem_result != WWD_SUCCESS )
            {
                /* Can't get the semaphore */
                result = ( sem_result == WWD_TIMEOUT ) ? PLATFORM_TIMEOUT : PLATFORM_ERROR;
                break;
            }

            WICED_DISABLE_INTERRUPTS();

            free_size = ring_buffer_free_space( driver->tx_buffer );
            data_size = MIN( size, free_size );

            /* Write the data into the ring buffer */
            while ( data_size != 0 )
            {
                driver->tx_buffer->buffer[driver->tx_buffer->tail] = data_out[write_index];
                driver->tx_buffer->tail = ( driver->tx_buffer->tail + 1 ) % driver->tx_buffer->size;
                write_index++;
                size--;
                data_size--;
            }

            free_size = ring_buffer_free_space( driver->tx_buffer );

            /* Make sure MFS UART TX interrupts are re-enabled, they could have
             * been disabled by the ISR if ring buffer was about to underflow */
            Mfs_Uart_EnableIrq( driver->mfsn_uart, UartTxIrq );

            WICED_ENABLE_INTERRUPTS();

            if ( free_size > 0 )
            {
                /* Set the semaphore to indicate the ring buffer is not full */
                host_rtos_set_semaphore( &mfs_uart_tx_semaphore[driver->peripheral->port], WICED_FALSE );
            }
        }
    }

    return result;
}

static platform_result_t platform_uart_receive_bytes_irq( platform_uart_driver_t* driver, uint8_t* data_in, uint32_t* data_size_left_to_read, uint32_t timeout_ms )
{
    platform_result_t result = PLATFORM_ERROR;

    if ( ( driver == NULL ) || ( data_in == NULL ) || ( data_size_left_to_read == NULL ) || ( *data_size_left_to_read == 0 ) )
    {
        wiced_assert( "bad argument", 0 );
        return PLATFORM_BADARG;
    }

    wiced_assert( "not inited", ( driver->rx_buffer != NULL ) );

    if ( driver->rx_buffer != NULL )
    {
        uint32_t bytes_read = 0;
        uint32_t read_index = 0;
        uint32_t data_size  = 0;
        uint32_t ring_size  = 0;

        result = PLATFORM_SUCCESS;

        while ( *data_size_left_to_read > 0 )
        {
            wwd_result_t sem_result;

            /* Get the semaphore whenever data needs to be consumed from the ring buffer */
            sem_result = host_rtos_get_semaphore( &mfs_uart_rx_semaphore[driver->peripheral->port], timeout_ms, WICED_TRUE );
            if ( sem_result != WWD_SUCCESS )
            {
                /* Can't get the semaphore */
                result = ( sem_result == WWD_TIMEOUT ) ? PLATFORM_TIMEOUT : PLATFORM_ERROR;
                break;
            }

            WICED_DISABLE_INTERRUPTS();

            /* Read the data from the ring buffer */
            ring_size = ring_buffer_used_space( driver->rx_buffer );
            data_size = MIN( *data_size_left_to_read, ring_size );
            ring_buffer_read( driver->rx_buffer, &data_in[read_index], data_size, &bytes_read );
            read_index              += bytes_read;
            *data_size_left_to_read -= bytes_read;
            ring_size = ring_buffer_used_space( driver->rx_buffer );

            /* Make sure MFS UART RX interrupts are re-enabled, they could have
             * been disabled by the ISR if ring buffer was about to overflow */
            Mfs_Uart_EnableIrq( driver->mfsn_uart, UartRxIrq );

            WICED_ENABLE_INTERRUPTS();

            if ( ring_size > 0 )
            {
                /* Set the semaphore to indicate the ring buffer is not empty */
                host_rtos_set_semaphore( &mfs_uart_rx_semaphore[driver->peripheral->port], WICED_FALSE );
            }
        }
    }

    return result;
}
#else
static platform_result_t platform_uart_transmit_bytes_poll( platform_uart_driver_t* driver, const uint8_t* data_out, uint32_t size )
{
    do
    {
        wwd_time_t   start_time   = host_rtos_get_time();
        wwd_time_t   elapsed_time = 0;

        /* Polling for TX buffer empty */
        while ( (Mfs_Uart_GetStatus( driver->mfsn_uart, UartTxEmpty ) != TRUE) && (elapsed_time < MFSN_UART_TX_MAX_WAIT_TIME_MS) )
        {
            elapsed_time = host_rtos_get_time( ) - start_time;
        }

        if ( Mfs_Uart_GetStatus( driver->mfsn_uart, UartTxEmpty ) != TRUE )
        {
            return PLATFORM_TIMEOUT;
        }

        /* Transmit data over MFS UART interface */
        Mfs_Uart_SendData( driver->mfsn_uart, *data_out );

        data_out++;
        size--;
    }
    while ( size != 0 );

    return PLATFORM_SUCCESS;
}

static platform_result_t platform_uart_receive_bytes_poll( platform_uart_driver_t* driver, uint8_t* data_in, uint32_t* data_size_left_to_read, uint32_t timeout_ms )
{
    wwd_time_t   total_start_time   = host_rtos_get_time( );
    wwd_time_t   total_elapsed_time = 0;

    do
    {
        wwd_time_t read_start_time   = host_rtos_get_time( );
        wwd_time_t read_elapsed_time = 0;
        wwd_time_t read_timeout_ms   = timeout_ms;

        /* Polling for RX buffer full */
        while ( (Mfs_Uart_GetStatus( driver->mfsn_uart, UartRxFull ) != TRUE) && (read_elapsed_time < read_timeout_ms) )
        {
            read_elapsed_time = host_rtos_get_time() - read_start_time;
        }

        if ( Mfs_Uart_GetStatus( driver->mfsn_uart, UartRxFull ) != TRUE )
        {
            break;
        }

        /* Receive data over MFS UART interface */
        *data_in = Mfs_Uart_ReceiveData( driver->mfsn_uart );

        data_in++;
        (*data_size_left_to_read)--;
        total_elapsed_time = host_rtos_get_time() - total_start_time;
    }
    while ( (*data_size_left_to_read != 0) && (total_elapsed_time < timeout_ms) );

    return ( *data_size_left_to_read == 0 ) ? PLATFORM_SUCCESS : PLATFORM_TIMEOUT;
}
#endif /* !CYFM4_MFS_UART_POLL_MODE */

platform_result_t platform_uart_init( platform_uart_driver_t* driver, const platform_uart_t* peripheral, const platform_uart_config_t* config, wiced_ring_buffer_t* optional_ring_buffer )
{
    volatile stc_mfsn_uart_t* mfsn_uart;
    stc_mfs_uart_config_t mfs_uart_config;

    if ( (driver == NULL) || (peripheral == NULL) || (config == NULL) )
    {
        wiced_assert( "Bad argument", 0 );
        return PLATFORM_ERROR;
    }

    if ( optional_ring_buffer != NULL )
    {
        if ( (optional_ring_buffer->buffer == NULL) || (optional_ring_buffer->size == 0) )
        {
            wiced_assert("Bad ring buffer", 0 );
            return PLATFORM_ERROR;
        }
    }

    platform_mcu_powersave_disable();

    /* Lookup the MFS UART channel instance for this port */
    mfsn_uart = platform_uart_get_mfs_channel( peripheral->port );

    if ( mfsn_uart == NULL )
    {
        return PLATFORM_UNSUPPORTED;
    }

    /* Initialize the MFS UART driver parameters */
    driver->rx_size              = 0;
    driver->tx_size              = 0;
    driver->rx_overflow          = 0;
    driver->tx_underflow         = 0;
    driver->mfsn_uart            = mfsn_uart;
    driver->peripheral           = (platform_uart_t*)peripheral;

#ifndef CYFM4_MFS_UART_POLL_MODE
    host_rtos_init_semaphore( &mfs_uart_tx_semaphore[peripheral->port] );
    host_rtos_init_semaphore( &mfs_uart_rx_semaphore[peripheral->port] );
    host_rtos_set_semaphore( &mfs_uart_tx_semaphore[peripheral->port], WICED_FALSE );
#endif /* !CYFM4_MFS_UART_POLL_MODE */

    /* Configure MFS UART RX and TX pin mapping */
    if ( platform_uart_set_input_pin( peripheral->port, peripheral->rx_pin_index ) != PLATFORM_SUCCESS )
    {
        return PLATFORM_UNSUPPORTED;
    }

    if ( platform_uart_set_output_pin( peripheral->port, peripheral->tx_pin_index ) != PLATFORM_SUCCESS )
    {
        return PLATFORM_UNSUPPORTED;
    }

    /* Setup the MFS UART channel configuration */
    PDL_ZERO_STRUCT(mfs_uart_config);
    mfs_uart_config.enMode = UartNormal;
    mfs_uart_config.u32BaudRate = config->baud_rate;
    mfs_uart_config.enDataLength = ( ( config->data_width == DATA_WIDTH_9BIT ) || ( ( config->data_width == DATA_WIDTH_8BIT ) && ( config->parity != NO_PARITY ) ) ) ? UartNineBits : UartEightBits;
    mfs_uart_config.enStopBit = ( config->stop_bits == STOP_BITS_1 ) ? UartOneStopBit : UartTwoStopBits;
    mfs_uart_config.enBitDirection = UartDataLsbFirst;
    mfs_uart_config.bInvertData = FALSE;
    mfs_uart_config.bUseExtClk = FALSE;
    mfs_uart_config.pstcFifoConfig = NULL;
#ifndef CYFM4_MFS_UART_POLL_MODE
    mfs_uart_config.pstcIrqEn = NULL;
    mfs_uart_config.pstcIrqCb = &mfs_uart_irq_cb[peripheral->port];
    mfs_uart_config.bTouchNvic = FALSE;
#endif /* !CYFM4_MFS_UART_POLL_MODE */

    switch ( config->parity )
    {
        case NO_PARITY:
            mfs_uart_config.enParity = UartParityNone;
            break;

        case EVEN_PARITY:
            mfs_uart_config.enParity = UartParityEven;
            break;

        case ODD_PARITY:
            mfs_uart_config.enParity = UartParityOdd;
            break;

        default:
            return PLATFORM_BADARG;
    }

    switch ( config->flow_control )
    {
        case FLOW_CONTROL_DISABLED:
            mfs_uart_config.bHwFlow = FALSE;
            break;

        case FLOW_CONTROL_CTS:
            mfs_uart_config.bHwFlow = TRUE;
            break;

        case FLOW_CONTROL_RTS:
            mfs_uart_config.bHwFlow = TRUE;
            break;

        case FLOW_CONTROL_CTS_RTS:
            mfs_uart_config.bHwFlow = TRUE;
            break;

        default:
            return PLATFORM_BADARG;
    }

    /* Setup RX ring buffer and related parameters */
    if ( optional_ring_buffer != NULL )
    {
        driver->rx_buffer = optional_ring_buffer;
    }
    else
    {
        ring_buffer_init( &mfs_uart_buffer[peripheral->port].rx_buffer, mfs_uart_buffer[peripheral->port].rx_data, MFSN_UART_RX_BUFFER_SIZE );
        driver->rx_buffer = &mfs_uart_buffer[peripheral->port].rx_buffer;
    }

    /* Setup TX ring buffer and related parameters */
    ring_buffer_init( &mfs_uart_buffer[peripheral->port].tx_buffer, mfs_uart_buffer[peripheral->port].tx_data, MFSN_UART_TX_BUFFER_SIZE );
    driver->tx_buffer = &mfs_uart_buffer[peripheral->port].tx_buffer;

    /* Initialize the MFS UART channel instance */
    if ( Mfs_Uart_Init( mfsn_uart, &mfs_uart_config ) != Ok )
    {
        return PLATFORM_ERROR;
    }

    /* Enable RX and TX function of MFS UART channel */
    Mfs_Uart_EnableFunc( mfsn_uart, UartRx );
    Mfs_Uart_EnableFunc( mfsn_uart, UartTx );

#ifndef CYFM4_MFS_UART_POLL_MODE
    NVIC_EnableIRQ(MFS0_TX_IRQn);
    NVIC_EnableIRQ(MFS0_RX_IRQn);
    /* Enable interrupts from MFS UART channel */
    Mfs_Uart_EnableIrq(mfsn_uart, UartRxIrq);
    Mfs_Uart_EnableIrq(mfsn_uart, UartTxIrq);
#endif /* !CYFM4_MFS_UART_POLL_MODE */

    return PLATFORM_SUCCESS;
}

platform_result_t platform_uart_deinit( platform_uart_driver_t* driver )
{
    boolean_t touch_NVIC;

    if ( driver == NULL )
    {
        wiced_assert( "Bad argument", 0 );
        return PLATFORM_ERROR;
    }

#ifndef CYFM4_MFS_UART_POLL_MODE
    touch_NVIC = TRUE;
    Mfs_Uart_DisableIrq(driver->mfsn_uart, UartTxIrq);
    Mfs_Uart_DisableIrq(driver->mfsn_uart, UartRxIrq);
#else
    touch_NVIC = FALSE;
#endif /* !CYFM4_MFS_UART_POLL_MODE */

    if ( Mfs_Uart_DeInit( driver->mfsn_uart, touch_NVIC ) != Ok )
    {
        return PLATFORM_ERROR;
    }

    return PLATFORM_SUCCESS;
}

platform_result_t platform_uart_transmit_bytes( platform_uart_driver_t* driver, const uint8_t* data_out, uint32_t size )
{
    platform_result_t result = PLATFORM_ERROR;

    if ( ( driver == NULL ) || ( data_out == NULL ) || ( size == 0 ) )
    {
        wiced_assert( "bad argument", 0 );
        return PLATFORM_ERROR;
    }

#ifndef CYFM4_MFS_UART_POLL_MODE
    result = platform_uart_transmit_bytes_irq( driver, data_out, size );
#else
    result = platform_uart_transmit_bytes_poll( driver, data_out, size );
#endif /* !CYFM4_MFS_UART_POLL_MODE */

    return result;
}

platform_result_t platform_uart_receive_bytes( platform_uart_driver_t* driver, uint8_t* data_in, uint32_t* expected_data_size, uint32_t timeout_ms )
{
    platform_result_t result = PLATFORM_ERROR;
    uint32_t          data_size_left_to_read;

    if ( ( driver == NULL ) || ( data_in == NULL ) || ( expected_data_size == NULL ) || ( *expected_data_size == 0 ) )
    {
        wiced_assert( "bad argument", 0 );
        return PLATFORM_ERROR;
    }

    data_size_left_to_read = *expected_data_size;

#ifndef CYFM4_MFS_UART_POLL_MODE
    result = platform_uart_receive_bytes_irq( driver, data_in, &data_size_left_to_read, timeout_ms );
#else
    result = platform_uart_receive_bytes_poll( driver, data_in, &data_size_left_to_read, timeout_ms );
#endif /* !CYFM4_MFS_UART_POLL_MODE */

    *expected_data_size -= data_size_left_to_read;

    return result;
}

/******************************************************
 *            IRQ Handlers Definition
 ******************************************************/


#ifndef CYFM4_MFS_UART_POLL_MODE
static wiced_bool_t platform_uart_process_tx_irq( platform_uart_driver_t* driver )
{
    wiced_bool_t result = WICED_FALSE;

    if ( driver->tx_buffer == NULL )
    {
        /* MFS UART TX interrupts remain turned off */
        result = WICED_FALSE;
    }
    else
    {
        /*
         * Check whether the ring buffer is already about to underflow.
         * This condition cannot happen during normal operation of the driver, but checked here only as precaution
         * to protect against ring buffer underflows for e.g. if UART interrupts got inadvertently enabled elsewhere.
         */
        if ( ring_buffer_used_space( driver->tx_buffer ) == 0 )
        {
            /* TX underflow error counter */
            driver->tx_underflow++;

            /* MFS UART TX interrupts remain turned off */
            result = WICED_FALSE;
        }
        else
        {
            uint8_t data = 0;
            uint32_t size = 1;
            uint32_t read = 0;

            /* Transfer data from the ring buffer into MFS UART */
            ring_buffer_read( driver->tx_buffer, &data, size, &read );
            Mfs_Uart_SendData( driver->mfsn_uart, data );

            /* Check whether the ring buffer is about to underflow */
            if ( ring_buffer_used_space( driver->tx_buffer ) == 0 )
            {
                /* MFS UART TX interrupts remain turned off */
                result = WICED_FALSE;
            }
            else
            {
                result = WICED_TRUE;
            }

            if ( ring_buffer_free_space( driver->tx_buffer ) == 1 )
            {
                /* Set the semaphore to indicate the ring buffer is not full */
                host_rtos_set_semaphore( &mfs_uart_tx_semaphore[driver->peripheral->port], WICED_TRUE );
            }
        }
    }

    return result;
}

static wiced_bool_t platform_uart_process_rx_irq( platform_uart_driver_t* driver )
{
    wiced_bool_t result = WICED_FALSE;

    if ( driver->rx_buffer == NULL )
    {
        /* MFS UART RX interrupts remain turned off */
        result = WICED_FALSE;
    }
    else
    {
        /*
         * Check whether the ring buffer is already about to overflow.
         * This condition cannot happen during correct operation of the driver, but checked here only as precaution
         * to protect against ring buffer overflows for e.g. if UART interrupts got inadvertently enabled elsewhere.
         */
        if ( ring_buffer_free_space( driver->rx_buffer ) == 0 )
        {
            /* RX overflow error counter */
            driver->rx_overflow++;

            /* MFS UART RX interrupts remain turned off */
            result = WICED_FALSE;
        }
        else
        {
            /* Transfer data from MFS UART into the ring buffer */
            driver->rx_buffer->buffer[driver->rx_buffer->tail] = Mfs_Uart_ReceiveData( driver->mfsn_uart );
            driver->rx_buffer->tail = ( driver->rx_buffer->tail + 1 ) % driver->rx_buffer->size;

            /* Check whether the ring buffer is about to overflow */
            if ( ring_buffer_free_space( driver->rx_buffer ) == 0 )
            {
                /* MFS UART RX interrupts remain turned off */
                result = WICED_FALSE;
            }
            else
            {
                result = WICED_TRUE;
            }

            if ( ring_buffer_used_space( driver->rx_buffer ) == 1 )
            {
                /* Set the semaphore to indicate the ring buffer is not empty */
                host_rtos_set_semaphore( &mfs_uart_rx_semaphore[driver->peripheral->port], WICED_TRUE );
            }
        }
    }

    return result;
}

void platform_uart_tx_irq( platform_uart_driver_t* driver )
{
    WICED_DISABLE_INTERRUPTS();

    /* Turn off MFS UART TX interrupts */
    Mfs_Uart_DisableIrq( driver->mfsn_uart, UartTxIrq );

    if ( Mfs_Uart_GetStatus( driver->mfsn_uart, UartTxEmpty ) == TRUE )
    {
        /* Drain the MFS UART TX data */
        while ( Mfs_Uart_GetStatus( driver->mfsn_uart, UartTxEmpty ) == TRUE )
        {
            if ( platform_uart_process_tx_irq( driver ) != WICED_TRUE )
            {
                /* MFS UART TX interrupts remain turned off */
                WICED_ENABLE_INTERRUPTS();
                return;
            }
        }
    }

    /* Turn on MFS UART TX interrupts */
    Mfs_Uart_EnableIrq( driver->mfsn_uart, UartTxIrq );

    WICED_ENABLE_INTERRUPTS();
}

void platform_uart_rx_irq( platform_uart_driver_t* driver )
{
    WICED_DISABLE_INTERRUPTS();

    /* Turn off MFS UART RX interrupts */
    Mfs_Uart_DisableIrq( driver->mfsn_uart, UartRxIrq );

    if ( Mfs_Uart_GetStatus( driver->mfsn_uart, UartRxFull ) == TRUE )
    {
        /* Drain the MFS UART RX data */
        while ( Mfs_Uart_GetStatus( driver->mfsn_uart, UartRxFull ) == TRUE )
        {
            if ( platform_uart_process_rx_irq( driver ) != WICED_TRUE )
            {
                /* MFS UART RX interrupts remain turned off */
                WICED_ENABLE_INTERRUPTS();
                return;
            }
        }
    }

    /* Turn on MFS UART RX interrupts */
    Mfs_Uart_EnableIrq( driver->mfsn_uart, UartRxIrq );

    WICED_ENABLE_INTERRUPTS();
}

void mfs_uart0_tx_irq_cb(void)
{
    platform_uart_tx_irq( &platform_uart_drivers[WICED_UART_1] );
}

void mfs_uart0_rx_irq_cb(void)
{
    platform_uart_rx_irq( &platform_uart_drivers[WICED_UART_1] );
}

void mfs_uart1_tx_irq_cb(void)
{
    platform_uart_tx_irq( &platform_uart_drivers[WICED_UART_2] );
}

void mfs_uart1_rx_irq_cb(void)
{
    platform_uart_rx_irq( &platform_uart_drivers[WICED_UART_2] );
}

void mfs_uart2_tx_irq_cb(void)
{
    platform_uart_tx_irq( &platform_uart_drivers[WICED_UART_3] );
}

void mfs_uart2_rx_irq_cb(void)
{
    platform_uart_rx_irq( &platform_uart_drivers[WICED_UART_3] );
}

void mfs_uart3_tx_irq_cb(void)
{
    platform_uart_tx_irq( &platform_uart_drivers[WICED_UART_4] );
}

void mfs_uart3_rx_irq_cb(void)
{
    platform_uart_rx_irq( &platform_uart_drivers[WICED_UART_4] );
}

void mfs_uart4_tx_irq_cb(void)
{
    platform_uart_tx_irq( &platform_uart_drivers[WICED_UART_5] );
}

void mfs_uart4_rx_irq_cb(void)
{
    platform_uart_rx_irq( &platform_uart_drivers[WICED_UART_5] );
}

void mfs_uart5_tx_irq_cb(void)
{
    platform_uart_tx_irq( &platform_uart_drivers[WICED_UART_6] );
}

void mfs_uart5_rx_irq_cb(void)
{
    platform_uart_rx_irq( &platform_uart_drivers[WICED_UART_6] );
}

void mfs_uart6_tx_irq_cb(void)
{
    platform_uart_tx_irq( &platform_uart_drivers[WICED_UART_7] );
}

void mfs_uart6_rx_irq_cb(void)
{
    platform_uart_rx_irq( &platform_uart_drivers[WICED_UART_7] );
}

void mfs_uart7_tx_irq_cb(void)
{
    platform_uart_tx_irq( &platform_uart_drivers[WICED_UART_8] );
}

void mfs_uart7_rx_irq_cb(void)
{
    platform_uart_rx_irq( &platform_uart_drivers[WICED_UART_8] );
}

void mfs_uart8_tx_irq_cb(void)
{
    platform_uart_tx_irq( &platform_uart_drivers[WICED_UART_9] );
}

void mfs_uart8_rx_irq_cb(void)
{
    platform_uart_rx_irq( &platform_uart_drivers[WICED_UART_9] );
}

void mfs_uart9_tx_irq_cb(void)
{
    platform_uart_tx_irq( &platform_uart_drivers[WICED_UART_10] );
}

void mfs_uart9_rx_irq_cb(void)
{
    platform_uart_rx_irq( &platform_uart_drivers[WICED_UART_10] );
}
#endif /* !CYFM4_MFS_UART_POLL_MODE */
