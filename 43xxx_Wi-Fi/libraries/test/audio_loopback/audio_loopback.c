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
 * Audio test Application
 *
 * This program tests the board's audio functionality.
 *
 * Application Instructions
 *   Connect a PC terminal to the serial port of the WICED Eval board,
 *   then build and download the application as described in the WICED
 *   Plug in speaker or headphones into the DAC plug
 *   Quick Start Guide
 *
 *   After download, the app initializes audio on platform,
 *   validates if tx or rx, and performs loop iterations
 *
 */

#include "wiced.h"
#include "wiced_audio.h"
#include "platform_audio.h"
#include "audio_loopback.h"

#include <math.h>

/******************************************************
 *                      Macros
 ******************************************************/

#define BYTES_TO_MILLISECONDS(number_of_bytes)     (((MICROSECONDS_PER_SECOND/config.sample_rate) * number_of_bytes)/MILLISECONDS_PER_SECOND)

#if WICED_AUDIO_LOOPBACK_LOG_ENABLED
#define PRINT_INFO(fmt,...)                                             \
    do                                                                  \
    {                                                                   \
        WPRINT_LIB_INFO(("audio loopback: " fmt "\n", ##__VA_ARGS__));  \
    } while ( 0 );
#else
#define PRINT_INFO(fmt,...)
#endif

/******************************************************
 *                    Constants
 ******************************************************/

#define WORKER_THREAD_STACK_SIZE  (1024)
#define WORKER_THREAD_QUEUE_SIZE  (4)

/* Enable data validation when I2S SDIN/SDOUT lines are shorted.
 * This is only useful when using WICED_AUDIO_LOOPBACK_ENABLE_SINE_WAVE_OUTPUT.
*/
#define NO_DATA_VALIDATION      1

#define TEST_TX_AUDIO_DEVICE         PLATFORM_DEFAULT_AUDIO_OUTPUT  /* To specify, change to a valid device for the platform
                                                                     *  See: platforms/<platform>/platform.h
                                                                     *       platforms/<platform>/platform_audio.c
                                                                     *       WICED/platform/include/platform_audio.h
                                                                     *  example: for BCM943909WCD1_3, specify
                                                                     *      AUDIO_DEVICE_ID_AK4954_DAC_LINE
                                                                     *  or
                                                                     *      AUDIO_DEVICE_ID_WM8533_DAC_LINE
                                                                     */
#if (WICED_AUDIO_LOOPBACK_ENABLE_SINE_WAVE_OUTPUT == 0 || NO_DATA_VALIDATION == 0)
#define TEST_RX_AUDIO_DEVICE         PLATFORM_DEFAULT_AUDIO_INPUT   /* To specify, change to a valid device for the platform
                                                                     *  See: platforms/<platform>/platform.h
                                                                     *       platforms/<platform>/platform_audio.c
                                                                     *       WICED/platform/include/platform_audio.h
                                                                     *  example: for BCM943909WCD1_3, specify
                                                                     *      AUDIO_DEVICE_ID_AK4954_ADC_LINE
                                                                     *  or
                                                                     *      AUDIO_DEVICE_ID_SPDIF
                                                                     */
#endif

#define PERIOD_SIZE                 (1*1024)
#define BUFFER_SIZE                 WICED_AUDIO_BUFFER_ARRAY_DIM_SIZEOF(10, PERIOD_SIZE)

#define EXTRA_MILLIS                (10)
#define TX_START_THRESHOLD          (3*PERIOD_SIZE)

#define SAMPLE_FREQUENCY_IN_HZ      (44100)

#define MICROSECONDS_PER_SECOND     (1000*1000)
#define MILLISECONDS_PER_SECOND     (1000)
#define BITS_PER_BYTE               (8)

#define SINE_WAVE_FREQUENCY_IN_HZ   (1000)
#define SINE_WAVE_VOLUME_PERCENTAGE (0.8F)

#define MAX_DATA_SIZE_IN_BYTES      (1024*2)

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

static wiced_result_t initialize_library( void );
static wiced_result_t initialize_audio_device( const platform_audio_device_id_t device_id, wiced_audio_config_t* config, uint8_t* buffer, size_t buffer_length, wiced_audio_session_ref* session );
static wiced_result_t loopback_thread( void* unused );
static wiced_result_t loop_iteration( void );
static wiced_result_t get_audio_data( uint8_t* buffer, uint16_t buffer_length );

#if WICED_AUDIO_LOOPBACK_ENABLE_SINE_WAVE_OUTPUT
static uint16_t initialize_data( float volume, uint tone_frequency_in_hz, const wiced_audio_config_t* config, uint8_t* buffer, uint16_t buffer_length_in_bytes );
static wiced_result_t copy_data( uint8_t* buffer, uint16_t buffer_length );
static wiced_result_t validate_data( uint8_t* buffer, uint16_t buffer_length );
static wiced_result_t validate_rx_data( uint16_t available_bytes );
#endif

/******************************************************
 *               Variables Definitions
 ******************************************************/

static wiced_bool_t          initialized;
static wiced_bool_t          running;
static wiced_bool_t          stop;
static wiced_semaphore_t     sem;
static wiced_worker_thread_t worker_thread;
static wiced_result_t        loop_result;

/* FIXME The audio subsystem should tell us what buffer requirements or
 *       do this allocation for us!
 */
static uint8_t                  tx_buffer[BUFFER_SIZE];
#ifdef TEST_RX_AUDIO_DEVICE
static uint8_t                  rx_buffer[BUFFER_SIZE];
#endif
static wiced_audio_session_ref  tx_session;
static wiced_audio_session_ref  rx_session;
static int                      is_tx_started;

static wiced_audio_config_t config =
{
    .sample_rate        = SAMPLE_FREQUENCY_IN_HZ,
    .channels           = 2,
    .bits_per_sample    = 16,
    .frame_size         = 4,
};

#if WICED_AUDIO_LOOPBACK_ENABLE_SINE_WAVE_OUTPUT
static int16_t  data[MAX_DATA_SIZE_IN_BYTES/2];
static int      number_of_data_samples = 45;
#endif

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_audio_loopback_run( uint32_t iterations )
{
    wiced_result_t result;

    if ( initialize_library() != WICED_SUCCESS )
    {
        PRINT_INFO("library initialization failed");
        return WICED_ERROR;
    }
    PRINT_INFO("library initialization success");

    result = wiced_rtos_get_semaphore( &sem, WICED_WAIT_FOREVER );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("wiced_rtos_get_semaphore failed");
        wiced_assert("wiced_rtos_get_semaphore loopback start",
                     result == WICED_SUCCESS);
        return WICED_ERROR;
    }

    if ( running == WICED_TRUE )
    {
        wiced_rtos_set_semaphore( &sem );
        PRINT_INFO("already running");
        return WICED_SUCCESS;
    }

    stop = WICED_FALSE;
    running = WICED_TRUE;

    /* Schedule the loopback worker thread to run. */
    result = wiced_rtos_send_asynchronous_event( &worker_thread,
                                                 loopback_thread,
                                                 (void*)iterations );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("wiced_rtos_send_asynchronous_event failed");
        wiced_assert("wiced_rtos_send_asynchronous_event loopback start",
                     result == WICED_SUCCESS);
        running = WICED_FALSE;
        wiced_rtos_set_semaphore( &sem );
        return WICED_ERROR;
    }

    if ( iterations > 0 )
    {
        /* Wait until all iterations have finished */
        while ( running == WICED_TRUE )
        {
            wiced_rtos_delay_milliseconds( 50 );
        }
        result = loop_result;
    }

    wiced_rtos_set_semaphore( &sem );

    return result;
}

wiced_result_t wiced_audio_loopback_start( void )
{
    return wiced_audio_loopback_run( 0 );
}

wiced_result_t wiced_audio_loopback_stop( void )
{
    wiced_result_t result;

    if ( initialized == WICED_FALSE )
    {
        /* There is nothing to stop. */
        return WICED_SUCCESS;
    }

    result = wiced_rtos_get_semaphore( &sem, WICED_WAIT_FOREVER );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("wiced_rtos_get_semaphore failed");
        wiced_assert("wiced_rtos_get_semaphore loopback stop",
                     result == WICED_SUCCESS);
        return WICED_ERROR;
    }

    if ( running == WICED_FALSE )
    {
        wiced_rtos_set_semaphore( &sem );
        return WICED_SUCCESS;
    }

    /* Signal the loopback thread to stop and wait for it. */
    stop = WICED_TRUE;
    while ( running == WICED_TRUE )
    {
        wiced_rtos_delay_milliseconds( 50 );
    }

    wiced_rtos_set_semaphore( &sem );

    return WICED_SUCCESS;
}

static wiced_result_t initialize_library( void )
{
    wiced_result_t result;

    if ( initialized == WICED_TRUE )
    {
        return WICED_SUCCESS;
    }

    result = wiced_rtos_init_semaphore( &sem );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("wiced_rtos_init_semaphore failed");
        wiced_assert("wiced_rtos_init_semaphore", result == WICED_SUCCESS);
        return WICED_ERROR;
    }

    wiced_rtos_set_semaphore( &sem );

    /* Create worker thread for audio loopback. */
    result = wiced_rtos_create_worker_thread( &worker_thread,
                                              WICED_DEFAULT_WORKER_PRIORITY,
                                              WORKER_THREAD_STACK_SIZE,
                                              WORKER_THREAD_QUEUE_SIZE );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("wiced_rtos_create_worker_thread failed");
        wiced_assert("wiced_rtos_create_worker_thread", result == WICED_SUCCESS);
        wiced_rtos_deinit_semaphore( &sem );
        return result;
    }

#if WICED_AUDIO_LOOPBACK_ENABLE_SINE_WAVE_OUTPUT
    /* Initialize sample data. */
    number_of_data_samples = initialize_data( SINE_WAVE_VOLUME_PERCENTAGE,
                                              SINE_WAVE_FREQUENCY_IN_HZ,
                                              &config, (uint8_t *)data,
                                              sizeof data );
#endif

    initialized = WICED_TRUE;
    return WICED_SUCCESS;
}

static wiced_result_t initialize_audio_device( const platform_audio_device_id_t device_id, wiced_audio_config_t* config, uint8_t* buffer, size_t buffer_length, wiced_audio_session_ref* session )
{
    wiced_result_t result = WICED_SUCCESS;

    /* Initialize device. */
    result = wiced_audio_init( device_id, session, PERIOD_SIZE );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("wiced_audio_init failed");
        wiced_assert("wiced_audio_init", WICED_SUCCESS == result);
        return result;
    }

    /* Allocate audio buffer. */
    result = wiced_audio_create_buffer( *session, buffer_length,
                                        WICED_AUDIO_BUFFER_ARRAY_PTR( buffer ),
                                        NULL );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("wiced_audio_create_buffer failed");
        wiced_assert("wiced_audio_create_buffer", WICED_SUCCESS == result);
        goto exit_with_error;
    }

    /* Configure session. */
    result = wiced_audio_configure( *session, config );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("wiced_audio_configure failed");
        wiced_assert("wiced_audio_configure", WICED_SUCCESS == result);
        goto exit_with_error;
    }

    return result;

exit_with_error:
    if ( wiced_audio_deinit( *session ) != WICED_SUCCESS )
    {
        PRINT_INFO("wiced_audio_deinit failed");
        wiced_assert("wiced_audio_deinit", 0);
    }

    return result;
}

#if WICED_AUDIO_LOOPBACK_ENABLE_SINE_WAVE_OUTPUT
/* Copy audio buffer from memory to buffer of given size. */
static wiced_result_t get_audio_data( uint8_t* buffer, uint16_t buffer_length )
{
    wiced_result_t result;

    /* Copy predefined data. */
    result = copy_data( buffer, buffer_length );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("copy RX data failed");
        wiced_assert("copy_data", result == WICED_SUCCESS);
        return result;
    }

    /* Validate and eat RX data. */
    if ( rx_session != NULL )
    {
        result = validate_rx_data( buffer_length );
        if ( result != WICED_SUCCESS )
        {
            PRINT_INFO("validate RX data failed");
            wiced_assert("validate_rx_data", result == WICED_SUCCESS);
        }
    }

    return result;
}

static uint16_t initialize_data( float volume, uint tone_frequency_in_hz, const wiced_audio_config_t* config, uint8_t* buffer, uint16_t buffer_length_in_bytes )
{
    int         sample_index             = 0;
    const uint  sample_frequency_in_hz  = config->sample_rate;
    const uint  bytes_per_sample        = config->bits_per_sample / 8;
    int16_t     *buf16                  = (int16_t *) buffer;
    uint16_t    total_samples_written   = 0;

    while (1)
    {
        float rad    = (2.0 * M_PI) * sample_index /
                       (((float)sample_frequency_in_hz / tone_frequency_in_hz));
        float v      = sinf(rad) * volume;
        int16_t data = (int16_t)(v < 0 ? -INT16_MIN * v : INT16_MAX * v) & 0xFFFF;

        buf16[ sample_index ] = data;

        if ( sample_index > 0 && ((tone_frequency_in_hz * (sample_index+1) %
                                   sample_frequency_in_hz) == 0) )
        {
            total_samples_written = sample_index + 1;
            break;
        }

        sample_index++;
        wiced_assert( "frame buffer too small", buffer_length_in_bytes >=
                     (sample_index * bytes_per_sample) );
    }

    return total_samples_written;
}

static wiced_result_t copy_data( uint8_t* buffer, uint16_t buffer_length )
{
    int i;
    static int last_pos;

    for ( i = 0; i < buffer_length / 2; )
    {
        int16_t *buf16 = (int16_t *) buffer;
        buf16[ i++ ] = data[ last_pos ];
        buf16[ i++ ] = data[ last_pos ];

        if ( ++last_pos >= number_of_data_samples )
        {
            last_pos = 0;
        }
    }

    return WICED_SUCCESS;
}

static wiced_result_t validate_rx_data( uint16_t available_bytes )
{
    wiced_result_t  result = WICED_SUCCESS;
    uint16_t        remaining = available_bytes;
    int             do_data_validation;

    do_data_validation = (is_tx_started != 0 && NO_DATA_VALIDATION == 0) ? 1 : 0;

    while ( remaining != 0 && result == WICED_SUCCESS )
    {
        uint8_t* buffer;
        uint16_t avail = remaining;

        result = wiced_audio_get_buffer( rx_session, &buffer, &avail );
        if ( result != WICED_SUCCESS )
        {
            PRINT_INFO("wiced_audio_get_buffer RX failed");
            wiced_assert("wiced_audio_get_buffer", result == WICED_SUCCESS);
            break;
        }
        if ( avail > remaining )
        {
            PRINT_INFO("bad buffer size");
            wiced_assert("bad size", avail <= remaining);
            result = WICED_ERROR;
            break;
        }

        if ( do_data_validation != 0 )
        {
            result = validate_data( buffer, avail );
            if ( result != WICED_SUCCESS )
            {
                PRINT_INFO("validate_data failed");
                wiced_assert("validate_data", result == WICED_SUCCESS);
                break;
            }
        }

        result = wiced_audio_release_buffer( rx_session, avail );
        if ( result != WICED_SUCCESS )
        {
            PRINT_INFO("wiced_audio_release_buffer RX failed");
            wiced_assert("wiced_audio_release_buffer", result == WICED_SUCCESS);
            break;
        }

        remaining -= avail;
    }

    return result;
}

static wiced_result_t validate_data( uint8_t* buffer, uint16_t buffer_length )
{
    int i = 0;
    static int last_pos = -1;
    static int allowed = 10;

    if ( last_pos < 0 )
    {
        int pos = 0;
        for ( i = 0; i < buffer_length / 2; i += 2 )
        {
            int16_t *buf16 = (int16_t *) buffer;
            for ( pos = 0; pos < number_of_data_samples; pos++ )
            {
                if ( buf16[ i ] == data[ pos ] )
                {
                    break;
                }
            }
            if ( number_of_data_samples == pos )
            {
                continue;
            }
            if ( pos != number_of_data_samples && buf16[ i + 1 ] == data[ pos ] )
            {
                last_pos = pos;
                if ( allowed != 0 )
                {
                    PRINT_INFO("last_pos is bad");
                    wiced_assert("last_pos", allowed == 0);
                    return WICED_ERROR;
                }
                break;
            }
        }
    }
    if ( last_pos < 0 )
    {
        --allowed;
        if ( allowed == 0 )
        {
            PRINT_INFO("data pattern not found");
            wiced_assert("data pattern not found", allowed != 0);
            return WICED_ERROR;
        }

        return WICED_SUCCESS;
    }

    while ( i < buffer_length / 2 )
    {
        int16_t *buf16 = (int16_t *) buffer;

        if ( buf16[ i++ ] != data[ last_pos ] )
        {
            PRINT_INFO("invalid data L");
            wiced_assert("invalid data L", 0);
            return WICED_ERROR;
        }
        if ( buf16[ i++ ] != data[ last_pos ] )
        {
            PRINT_INFO("invalid data R");
            wiced_assert("invalid data R", 0);
            return WICED_ERROR;
        }

        if ( ++last_pos >= number_of_data_samples )
        {
            last_pos = 0;
        }
    }

    return WICED_SUCCESS;
}

#else /* !WICED_AUDIO_LOOPBACK_ENABLE_SINE_WAVE_OUTPUT */
/* Copy audio buffer from session to buffer of given size. */
static wiced_result_t get_audio_data( uint8_t* buffer, uint16_t buffer_length )
{
    wiced_result_t          result;
    uint16_t                remaining = buffer_length;
    wiced_audio_session_ref sh        = rx_session;

    result = WICED_SUCCESS;

    while ( 0 != remaining && result == WICED_SUCCESS )
    {
        uint8_t *buf;
        uint16_t avail = remaining;

        result = wiced_audio_get_buffer( sh, &buf, &avail );
        if ( result != WICED_SUCCESS )
        {
            PRINT_INFO("wiced_audio_get_buffer failed");
            wiced_assert("wiced_audio_get_buffer", result == WICED_SUCCESS);
            break;
        }
        if ( avail > remaining )
        {
            PRINT_INFO("bad size");
            wiced_assert("bad size", avail <= remaining);
            result = WICED_ERROR;
            break;
        }

        memcpy( buffer, buf, avail );
        result = wiced_audio_release_buffer( sh, avail );
        if ( result != WICED_SUCCESS )
        {
            PRINT_INFO("wiced_audio_release_buffer failed");
            wiced_assert("wiced_audio_release_buffer", result == WICED_SUCCESS);
            break;
        }
        buffer    += avail;
        remaining -= avail;
    }

    return result;
}
#endif /* !WICED_AUDIO_LOOPBACK_ENABLE_SINE_WAVE_OUTPUT */

static wiced_result_t loop_iteration( void )
{
    wiced_result_t result;
    uint16_t       remaining;
    const uint32_t timeout = BYTES_TO_MILLISECONDS(PERIOD_SIZE) + EXTRA_MILLIS;

    result = WICED_SUCCESS;

    /* Start data transmission. */
    if ( !is_tx_started )
    {
        uint32_t weight;

        /* Determine if we should start TX. */
        result = wiced_audio_get_current_buffer_weight( tx_session, &weight );
        if ( result != WICED_SUCCESS )
        {
            PRINT_INFO("wiced_audio_get_current_buffer_weight failed");
            wiced_assert("wiced_audio_get_current_buffer_weight",
                         result == WICED_SUCCESS);
            return result;
        }
        if ( weight >= TX_START_THRESHOLD )
        {
            result = wiced_audio_start( tx_session );
            if ( result != WICED_SUCCESS )
            {
                PRINT_INFO("wiced_audio_start TX failed");
                wiced_assert("wiced_audio_start TX", result == WICED_SUCCESS);
                return result;
            }

            PRINT_INFO("TX started");
            is_tx_started = 1;
        }
    }

    /* Wait for data that can be transmitted. */
    /* In the case of canned data, this defines the transmit cadence. */
    if ( rx_session != NULL )
    {
        result = wiced_audio_wait_buffer(rx_session, PERIOD_SIZE, timeout);
        if ( result != WICED_SUCCESS )
        {
            PRINT_INFO("wiced_audio_wait_buffer RX failed");
            wiced_assert("wiced_audio_wait_buffer RX", result == WICED_SUCCESS);
            return result;
        }
    }

    /* Wait for slot in transmit buffer. */
    result = wiced_audio_wait_buffer( tx_session, PERIOD_SIZE, timeout );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("wiced_audio_wait_buffer TX failed");
        wiced_assert("wiced_audio_wait_buffer TX", result == WICED_SUCCESS);
        return result;
    }

    /* Copy available data to transmit buffer. */
    remaining = PERIOD_SIZE;
    while ( 0 != remaining && result == WICED_SUCCESS )
    {
        uint8_t *buf;
        uint16_t avail = remaining;

        result = wiced_audio_get_buffer(tx_session, &buf, &avail);
        if ( result != WICED_SUCCESS )
        {
            PRINT_INFO("wiced_audio_get_buffer failed");
            wiced_assert("wiced_audio_get_buffer", result == WICED_SUCCESS);
            return result;
        }
        if ( avail > remaining )
        {
            PRINT_INFO("bad size");
            wiced_assert("bad size", avail <= remaining);
            return WICED_ERROR;
        }

        result = get_audio_data( buf, avail );
        if ( result != WICED_SUCCESS )
        {
            PRINT_INFO("get_data failed");
            wiced_assert("get_data", result == WICED_SUCCESS);
            return result;
        }

        result = wiced_audio_release_buffer( tx_session, avail );
        if ( result != WICED_SUCCESS )
        {
            PRINT_INFO("wiced_audio_release_buffer failed");
            wiced_assert("wiced_audio_release_buffer", result == WICED_SUCCESS);
            return result;
        }

        remaining -= avail;
    }

    return WICED_SUCCESS;
}

static wiced_result_t loopback_thread( void* arg )
{
    static wiced_bool_t audio_inited = WICED_FALSE;
    wiced_result_t      result = WICED_SUCCESS;
    uint32_t            iterations = (uint32_t)arg;
    wiced_bool_t        forever = iterations == 0 ? WICED_TRUE : WICED_FALSE;

    loop_result = WICED_SUCCESS;

    /* Initialize platform audio. */
    if ( audio_inited == WICED_FALSE )
    {
        result = platform_init_audio();
        if ( result != WICED_SUCCESS )
        {
            PRINT_INFO("initialize platform audio failed");
            wiced_assert("platform_init_audio", result == WICED_SUCCESS);
            loop_result = WICED_ERROR;
            return WICED_ERROR;
        }

        PRINT_INFO("initialize platform audio success");
        audio_inited = WICED_TRUE;
    }

    /* Initialize TX device. */
    result = initialize_audio_device( TEST_TX_AUDIO_DEVICE, &config, tx_buffer,
                                      sizeof(tx_buffer), &tx_session );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("initialize TX audio device failed");
        wiced_assert("initialize_audio_device TX", result == WICED_SUCCESS);
        loop_result = WICED_ERROR;
        goto out_deinit_audio;
    }
    PRINT_INFO("initialize TX audio device success");

#ifdef TEST_RX_AUDIO_DEVICE
    /* Initialize RX device. */
    result = initialize_audio_device( TEST_RX_AUDIO_DEVICE, &config, rx_buffer,
                                      sizeof(rx_buffer), &rx_session );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("initialize RX audio device failed");
        wiced_assert("initialize_audio_device RX", result == WICED_SUCCESS);
        loop_result = WICED_ERROR;
        goto out_stop_tx;
    }
    PRINT_INFO("initialize RX audio device success");

    /* Start RX. */
    PRINT_INFO("starting audio");
    result = wiced_audio_start( rx_session );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("wiced_audio_start RX failed");
        wiced_assert("wiced_audio_start RX", result == WICED_SUCCESS);
        loop_result = WICED_ERROR;
        goto out_stop_rx;
    }
    PRINT_INFO("audio started");
#endif

    /* Main loop. */
    PRINT_INFO("starting main loop");
    while ((( forever == WICED_TRUE ) && ( stop == WICED_FALSE )) ||
           (( forever == WICED_FALSE ) && ( iterations-- > 0 )))
    {
        result = loop_iteration();
        if ( result != WICED_SUCCESS )
        {
            PRINT_INFO("loop_iteration failed- bailing!");
            wiced_assert("loop_iteration", result == WICED_SUCCESS);
            loop_result = WICED_ERROR;
            break;
        }
    }

    if ( is_tx_started )
    {
        result = wiced_audio_stop( tx_session );
        if ( result != WICED_SUCCESS )
        {
            PRINT_INFO("wiced_audio_stop TX failed");
            wiced_assert("wiced_audio_stop TX", result == WICED_SUCCESS);
            loop_result = WICED_ERROR;
        }
        is_tx_started = 0;
    }

#ifdef TEST_RX_AUDIO_DEVICE
    result = wiced_audio_stop( rx_session );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("wiced_audio_stop RX failed");
        wiced_assert("wiced_audio_stop RX", result == WICED_SUCCESS);
        loop_result = WICED_ERROR;
    }

out_stop_rx:
    result = wiced_audio_deinit( rx_session );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("wiced_audio_deinit RX failed");
        wiced_assert("wiced_audio_deinit RX", result == WICED_SUCCESS);
        loop_result = WICED_ERROR;
    }

out_stop_tx:
#endif
    result = wiced_audio_deinit( tx_session );
    if ( result != WICED_SUCCESS )
    {
        PRINT_INFO("wiced_audio_deinit TX failed");
        wiced_assert("wiced_audio_deinit TX", result == WICED_SUCCESS);
        loop_result = WICED_ERROR;
    }

out_deinit_audio:
    /*
     * May not support deiniting (43909 does not). In this case keep platform
     * audio initialized. But this is not good as audio including codec is not
     * re-initialized, and other tests (particular I2C test) is talking to
     * codec and may break current test when current test run next time without
     * power-cycling.
     */
    if ( platform_deinit_audio() == WICED_SUCCESS )
    {
        /* Deiniting supported. */
        audio_inited = WICED_FALSE;
    }

    running = WICED_FALSE;

    return WICED_SUCCESS;
}

