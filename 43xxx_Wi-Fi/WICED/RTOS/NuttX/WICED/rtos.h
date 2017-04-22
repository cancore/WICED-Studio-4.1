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

#include "wiced_result.h"
#include "wwd_rtos.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

#define WICED_PRIORITY_TO_NATIVE_PRIORITY(priority) (uint8_t)(RTOS_HIGHEST_PRIORITY - priority)

#define WICED_END_OF_CURRENT_THREAD( )

#define WICED_HARDWARE_IO_WORKER_THREAD             ((wiced_worker_thread_t*)&wiced_hardware_io_worker_thread)
#define WICED_NETWORKING_WORKER_THREAD              ((wiced_worker_thread_t*)&wiced_networking_worker_thread )

#ifndef HARDWARE_IO_WORKER_THREAD_STACK_SIZE
#ifdef DEBUG
#define HARDWARE_IO_WORKER_THREAD_STACK_SIZE        (768) /* debug builds can use larger stack for example because of compiled-in asserts, switched off optimisation, etc */
#else
#define HARDWARE_IO_WORKER_THREAD_STACK_SIZE        (512)
#endif
#endif
#define HARDWARE_IO_WORKER_THREAD_QUEUE_SIZE        (10)

#ifndef NETWORKING_WORKER_THREAD_STACK_SIZE
#define NETWORKING_WORKER_THREAD_STACK_SIZE         (6*1024)
#endif
#define NETWORKING_WORKER_THREAD_QUEUE_SIZE         (15)

#define WICED_GET_THREAD_HANDLE( thread )           (thread)

#define WICED_GET_QUEUE_HANDLE( queue )             (queue)

#define RTOS_NAME                                   "NuttX"
#define RTOS_VERSION                                CONFIG_VERSION_STRING

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef wiced_result_t (*event_handler_t)(void *arg);
typedef void (*timer_handler_t)(void * arg);

typedef host_queue_type_t wiced_queue_t;

typedef host_thread_type_t wiced_thread_t;

typedef host_semaphore_type_t wiced_semaphore_t;

typedef pthread_mutex_t wiced_mutex_t;

typedef uint32_t wiced_event_flags_t; /* TODO: need to implement */

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    WDOG_ID         id;
    uint32_t        time_ms;
    timer_handler_t function;
    void*           arg;
} wiced_timer_t;

typedef struct
{
    wiced_thread_t thread;
    wiced_queue_t  event_queue;
} wiced_worker_thread_t;

typedef struct
{
    event_handler_t        function;
    void*                  arg;
    wiced_timer_t          timer;
    wiced_worker_thread_t* thread;
} wiced_timed_event_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

extern wiced_worker_thread_t wiced_hardware_io_worker_thread;
extern wiced_worker_thread_t wiced_networking_worker_thread;

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif
