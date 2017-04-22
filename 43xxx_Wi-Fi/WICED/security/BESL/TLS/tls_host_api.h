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

#include "tls_types.h"

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

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *          TLS -> Host Function Declarations
 ******************************************************/

extern tls_result_t tls_host_create_buffer   ( wiced_tls_workspace_t* ssl, uint8_t** buffer, uint16_t buffer_size );
extern tls_result_t tls_host_free_packet     ( tls_packet_t* packet );
extern tls_result_t tls_host_send_tcp_packet ( void* context, tls_packet_t* packet );
extern tls_result_t tls_host_get_packet_data ( ssl_context* ssl, tls_packet_t* packet, uint32_t offset, uint8_t** data, uint16_t* data_length, uint16_t* available_data_length );
extern tls_result_t tls_host_set_packet_start( tls_packet_t* packet, uint8_t* start );
extern tls_result_t tls_calculate_encrypt_buffer_length( ssl_context* context, uint16_t* required_buff_size, uint16_t payload_size);

/*
 * This should wait for a specified amount of time to receive a packet.
 * If the SSL context already has a received packet stored, it should append it to the previous packet either contiguously or via a linked list.
 */
extern tls_result_t tls_host_receive_packet( ssl_context* ssl, tls_packet_t** packet, uint32_t timeout );

extern uint64_t tls_host_get_time_ms( void );

extern void* tls_host_malloc( const char* name, uint32_t size );
extern void  tls_host_free  ( void* p );

extern void* tls_host_get_defragmentation_buffer ( uint16_t size );
extern void  tls_host_free_defragmentation_buffer( void* buffer );
extern tls_result_t ssl_flush_output( ssl_context *ssl, uint8_t* buffer, uint32_t length );

/******************************************************
 *           Host -> TLS Function Declarations
 ******************************************************/

extern tls_result_t tls_get_next_record( ssl_context* ssl, tls_record_t** record, uint32_t timeout, tls_packet_receive_option_t packet_receive_option );
extern int32_t      ssl_cleanup_record(wiced_tls_context_t* ssl, tls_record_t* record);
extern void         tls_cleanup_current_record(ssl_context* ssl);
extern tls_result_t tls_skip_current_record( ssl_context* ssl );


int32_t tls1_prf( unsigned char *secret, int32_t slen, char *label, unsigned char *random, int32_t rlen, unsigned char *dstbuf, int32_t dlen );

#ifdef __cplusplus
} /*extern "C" */
#endif
