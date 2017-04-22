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

#include <stdint.h>
#include "wiced_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

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

typedef /*@abstract@*/ /*@immutable@*/ struct
{
    uint8_t*  buffer;
    uint32_t  size;
    volatile uint32_t  head; /* Read from */
    volatile uint32_t  tail; /* Write to */
} wiced_ring_buffer_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/* Ring Buffer API */
wiced_result_t ring_buffer_init       ( /*@out@*/ wiced_ring_buffer_t* ring_buffer, /*@keep@*/ uint8_t* buffer, uint32_t buffer_size );
wiced_result_t ring_buffer_deinit     ( wiced_ring_buffer_t* ring_buffer );
uint32_t       ring_buffer_write      ( wiced_ring_buffer_t* ring_buffer, const uint8_t* data, uint32_t data_length );
uint32_t       ring_buffer_used_space ( wiced_ring_buffer_t* ring_buffer );
uint32_t       ring_buffer_free_space ( wiced_ring_buffer_t* ring_buffer );
wiced_result_t ring_buffer_get_data   ( wiced_ring_buffer_t* ring_buffer, uint8_t** data, uint32_t* contiguous_bytes );
wiced_result_t ring_buffer_consume    ( wiced_ring_buffer_t* ring_buffer, uint32_t bytes_consumed );
wiced_result_t ring_buffer_read       ( wiced_ring_buffer_t* ring_buffer, uint8_t* data, uint32_t data_length, uint32_t* number_of_bytes_read );


#ifdef __cplusplus
} /* extern "C" */
#endif
