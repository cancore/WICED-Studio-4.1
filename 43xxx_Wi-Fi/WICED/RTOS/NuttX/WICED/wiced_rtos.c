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
#include "wiced_time.h"

#include "wwd_assert.h"

#include "rtos.h"

#include <stdint.h>
#include <unistd.h>

/******************************************************
 *                      Macros
 ******************************************************/

#ifndef WICED_RTOS_THREAD_FORCE_AWAKE_SIGNAL
#define WICED_RTOS_THREAD_FORCE_AWAKE_SIGNAL SIGUSR1
#endif

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

wiced_worker_thread_t wiced_hardware_io_worker_thread;
wiced_worker_thread_t wiced_networking_worker_thread;

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_rtos_create_thread( wiced_thread_t* thread, uint8_t priority, const char* name, wiced_thread_function_t function, uint32_t stack_size, void* arg )
{
#if RTOS_HIGHEST_PRIORITY < UINT8_MAX
    /* Limit priority to maximum priority */
    if ( priority > RTOS_HIGHEST_PRIORITY )
    {
        priority = RTOS_HIGHEST_PRIORITY;
    }
#endif

    return host_rtos_create_thread_with_arg( WICED_GET_THREAD_HANDLE( thread ), function, name, NULL, stack_size, WICED_PRIORITY_TO_NATIVE_PRIORITY( priority ), (wiced_thread_arg_t) arg );
}

wiced_result_t wiced_rtos_delete_thread( wiced_thread_t* thread )
{
    wwd_result_t result;

    result = host_rtos_finish_thread( WICED_GET_THREAD_HANDLE( thread ) );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    result = host_rtos_delete_terminated_thread( WICED_GET_THREAD_HANDLE( thread ) );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_rtos_is_current_thread( wiced_thread_t* thread )
{
    if ( pthread_equal( thread->handle, pthread_self() ) == 0 )
    {
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_rtos_check_stack( void )
{
    // TODO: Add stack checking here.

    return WICED_SUCCESS;
}

wiced_result_t wiced_rtos_thread_force_awake( wiced_thread_t* thread )
{
    return ( pthread_kill( thread->handle, WICED_RTOS_THREAD_FORCE_AWAKE_SIGNAL ) == OK ) ? WICED_SUCCESS : WICED_ERROR;
}

wiced_result_t wiced_time_get_time( wiced_time_t* time_ptr )
{
    *time_ptr = (wiced_time_t) ( clock_systimer_proxy( ) * ( 1000 / SYSTICK_FREQUENCY ) );
    return WICED_SUCCESS;
}

wiced_result_t wiced_time_set_time( const wiced_time_t* time_ptr )
{
    clock_systimer_set( ( *time_ptr ) / ( 1000 / SYSTICK_FREQUENCY ) );
    return WICED_SUCCESS;
}

static wiced_result_t wiced_rtos_init_mutex_with_attr( wiced_mutex_t* mutex, pthread_mutexattr_t* attr )
{
    int result;

#ifdef CONFIG_MUTEX_TYPES
    if ( attr )
    {
        result = pthread_mutexattr_settype( attr, PTHREAD_MUTEX_RECURSIVE );
        if ( result != OK )
        {
            return WICED_ERROR;
        }
    }
#endif

    result = pthread_mutex_init( mutex, attr );
    if ( result != OK )
    {
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_rtos_init_mutex( wiced_mutex_t* mutex )
{
#ifdef CONFIG_MUTEX_TYPES
    int                 result;
    wiced_result_t      wiced_result;
    pthread_mutexattr_t attr;

    result = pthread_mutexattr_init( &attr );
    if ( result != OK )
    {
        return WICED_ERROR;
    }

    wiced_result = wiced_rtos_init_mutex_with_attr( mutex, &attr );

    result = pthread_mutexattr_destroy( &attr );
    if ( result != OK )
    {
        wiced_assert( "attr destroying failed", 0 );
        return WICED_ERROR;
    }

    return wiced_result;
#else
    return wiced_rtos_init_mutex_with_attr( mutex, NULL );
#endif
}

wiced_result_t wiced_rtos_lock_mutex( wiced_mutex_t* mutex )
{
    int result;

    result = pthread_mutex_lock( mutex );
    if ( result != OK )
    {
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_rtos_unlock_mutex( wiced_mutex_t* mutex )
{
    int result;

    result = pthread_mutex_unlock( mutex );
    if ( result != OK )
    {
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_rtos_deinit_mutex( wiced_mutex_t* mutex )
{
    int result;

    result = pthread_mutex_destroy( mutex );
    if ( result != OK )
    {
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_rtos_init_queue( wiced_queue_t* queue, const char* name, uint32_t message_size, uint32_t number_of_messages )
{
    uint32_t     buffer_size = number_of_messages * message_size;
    uint8_t*     buffer;
    wwd_result_t result;

    UNUSED_PARAMETER( name );

    buffer = malloc( buffer_size );
    if ( !buffer )
    {
        return WICED_OUT_OF_HEAP_SPACE;
    }

    result = host_rtos_init_queue( WICED_GET_QUEUE_HANDLE( queue ), buffer, buffer_size, message_size );
    if ( result != WWD_SUCCESS )
    {
        free( buffer );
        return result;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_rtos_deinit_queue( wiced_queue_t* queue )
{
    uint8_t*     buffer = WICED_GET_QUEUE_HANDLE( queue )->buffer;
    wwd_result_t result;

    result = host_rtos_deinit_queue( WICED_GET_QUEUE_HANDLE( queue ) );

    free( buffer );

    return result;
}

wiced_result_t wiced_rtos_get_queue_occupancy( wiced_queue_t* queue, uint32_t* count )
{
    *count = WICED_GET_QUEUE_HANDLE( queue )->occupancy;
    return WICED_SUCCESS;
}

wiced_result_t wiced_rtos_is_queue_empty( wiced_queue_t* queue )
{
    if ( WICED_GET_QUEUE_HANDLE( queue )->occupancy == 0 )
    {
        return WICED_SUCCESS;
    }
    return WICED_ERROR;
}

wiced_result_t wiced_rtos_is_queue_full( wiced_queue_t* queue )
{
    if ( WICED_GET_QUEUE_HANDLE( queue )->occupancy == WICED_GET_QUEUE_HANDLE( queue )->message_num )
    {
        return WICED_SUCCESS;
    }
    return WICED_ERROR;
}

static void wiced_rtos_timer_wdentry( int argc, wiced_timer_t* timer )
{
    UNUSED_PARAMETER( argc );

    wiced_rtos_start_timer( timer );

    ( *timer->function )( timer->arg );
}

wiced_result_t wiced_rtos_init_timer( wiced_timer_t* timer, uint32_t time_ms, timer_handler_t function, void* arg )
{
    /*
     * WDOG NuttX is working in timer interrupt context.
     * I.e. 'function' will be called in interrupt context.
     */

    timer->id = wd_create( );
    if ( timer->id == NULL )
    {
        return WICED_ERROR;
    }

    timer->time_ms  = time_ms;
    timer->function = function;
    timer->arg      = arg;

    return WICED_SUCCESS;
}

wiced_result_t wiced_rtos_start_timer( wiced_timer_t* timer )
{
    int delay = (int)( timer->time_ms * SYSTICK_FREQUENCY / 1000 );
    return ( wd_start( timer->id, delay, (wdentry_t)wiced_rtos_timer_wdentry, 1, timer ) == OK ) ? WICED_SUCCESS : WICED_ERROR;
}

wiced_result_t wiced_rtos_stop_timer( wiced_timer_t* timer )
{
    return ( wd_cancel( timer->id ) == OK ) ? WICED_SUCCESS : WICED_ERROR;
}

wiced_result_t wiced_rtos_deinit_timer( wiced_timer_t* timer )
{
    return ( wd_delete( timer->id ) == OK ) ? WICED_SUCCESS : WICED_ERROR;
}

wiced_result_t wiced_rtos_is_timer_running( wiced_timer_t* timer )
{
    return WDOG_ISACTIVE( timer->id ) ? WICED_SUCCESS : WICED_ERROR;
}

wiced_result_t wiced_rtos_init_event_flags( wiced_event_flags_t* event_flags )
{
    UNUSED_PARAMETER( event_flags );
    wiced_assert( "not yet implemented", 0 );
    return WICED_SUCCESS;
}

wiced_result_t wiced_rtos_wait_for_event_flags( wiced_event_flags_t* event_flags, uint32_t flags_to_wait_for, uint32_t* flags_set, wiced_bool_t clear_set_flags, wiced_event_flags_wait_option_t wait_option, uint32_t timeout_ms )
{
    UNUSED_PARAMETER( event_flags );
    UNUSED_PARAMETER( flags_to_wait_for );
    UNUSED_PARAMETER( flags_set );
    UNUSED_PARAMETER( clear_set_flags );
    UNUSED_PARAMETER( wait_option );
    UNUSED_PARAMETER( timeout_ms );
    wiced_assert( "not yet implemented", 0 ); /* Need to implement it the way can be called from interrupt context if timeout_ms is zero */
    return WICED_SUCCESS;
}

wiced_result_t wiced_rtos_set_event_flags( wiced_event_flags_t* event_flags, uint32_t flags_to_set )
{
    UNUSED_PARAMETER( event_flags );
    UNUSED_PARAMETER( flags_to_set );
    wiced_assert( "not yet implemented", 0 ); /* Need to implement the way it can be called from interrupt context */
    return WICED_SUCCESS;
}

wiced_result_t wiced_rtos_deinit_event_flags( wiced_event_flags_t* event_flags )
{
    UNUSED_PARAMETER( event_flags );
    wiced_assert( "not yet implemented", 0 );
    return WICED_SUCCESS;
}
