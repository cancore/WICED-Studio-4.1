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
#include "wwd_constants.h"
#include "wiced_tcpip.h"
#include "wiced_dtls.h"
#include "wiced_utilities.h"
#include "dtls_host_api.h"
#include "wiced_crypto.h"
#include "wwd_buffer_interface.h"
#include "crypto_constants.h"
#include "crypto_structures.h"
#include "wiced_time.h"
#include "wwd_assert.h"
#include "wwd_buffer_interface.h"
#include "besl_host_interface.h"
#include "wiced_security.h"
#include "internal/wiced_internal_api.h"
#include "wiced_udpip_dtls_api.h"
#include "x509.h"


#include "wiced_network.h"
#include "dtls.h"
#include "dtls_types.h"
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define DTLS_EVENT_QUEUE_DEPTH                      (10)
#define DTLS_THREAD_PRIORITY                        (WICED_DEFAULT_LIBRARY_PRIORITY)
#define DTLS_THREAD_STACK_SIZE                      (6200)
#define DTLS_RETRANSMISSION_CHECK_TIMER_INTERVAL    (100)
#define DEFAULT_UDP_PACKET_SIZE                     (0)

/* TODO : This is already there in dtls.h but to pass jenkins added here. Need to correct this  */
#define DTLS_CT_APPLICATION_DATA        23

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

dtls_result_t  dtls_process_request (wiced_udp_socket_t* socket);
wiced_result_t dtls_retransmission_check_timer_callback( void *arg );
wiced_result_t dtls_check_retransmission( void *arg );

/******************************************************
 *               Static Function Declarations
 ******************************************************/
static besl_result_t wiced_dtls_load_key( wiced_dtls_key_t* key, const char* key_string, const uint32_t key_length );
static besl_result_t wiced_dtls_load_certificate( wiced_dtls_certificate_t* certificate, const uint8_t* certificate_data, uint32_t certificate_length, wiced_tls_certificate_format_t certificate_format );
static wiced_result_t dtls_receive_callback( wiced_udp_socket_t* socket, void *args );
static void dtls_event_thread( uint32_t arg );
static void dtls_retransmit_buffered_packets( wiced_dtls_context_t* context, dtls_peer_t *peer);

/******************************************************
 *               Variable Definitions
 ******************************************************/
/**
 * This list of cipher suites should be in order of strength with the strongest first.
 * Do not enable cipher suites unless they meet your security requirements
 */
static const cipher_suite_t* my_ciphers[ ] =
{

#if defined( USE_DTLS_PSK_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_8_CIPHER ) && defined( USE_DTLS_AES_128_CCM_8_MAC )
        &DTLS_PSK_WITH_AES_128_CCM_8,
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_128_CCM_8_CIPHER ) && defined( USE_AES_128_CCM_8_MAC ) */

#if defined( USE_DTLS_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_DTLS_AES_128_CCM_8_CIPHER ) && defined( USE_DTLS_AES_128_CCM_8_MAC )
        &DTLS_ECDHE_ECDSA_WITH_AES_128_CCM_8,
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_128_CCM_8_CIPHER ) && defined( USE_AES_128_CCM_8_MAC ) */

        0 /* List termination */
};

wiced_thread_t  dtls_thread;
wiced_queue_t   dtls_event_queue;
wiced_timed_event_t dtls_timed_event;
/******************************************************
 *               Function Definitions
 ******************************************************/

dtls_result_t dtls_host_get_packet_data( dtls_context_t* dtls, dtls_packet_t* packet, uint32_t offset, uint8_t** data, uint16_t* data_length, uint16_t* available_data_length )
{
    uint16_t temp_length;
    uint16_t temp_available_length;
    wiced_result_t result = wiced_packet_get_data( (wiced_packet_t*) packet, (uint16_t) offset, data, &temp_length, &temp_available_length );
    if ( result != WICED_SUCCESS )
    {
        return (dtls_result_t) result;
    }
    *data_length = temp_length;
    *available_data_length = temp_available_length;
    return DTLS_SUCCESS;
}

dtls_result_t dtls_host_packet_get_info( uint32_t* packet, dtls_session_t* session )
{
    wiced_udp_packet_get_info( (wiced_packet_t*) packet, (wiced_ip_address_t*) &session->ip, &session->port );
    return DTLS_SUCCESS;
}

dtls_result_t dtls_host_create_buffer( dtls_context_t* dtls_context, uint16_t buffer_size, uint8_t** buffer )
{
    wiced_assert("", dtls_context->outgoing_packet == NULL);

    /* Round requested buffer size up to next 64 byte chunk (required if encryption is active) */
    buffer_size = (uint16_t) ROUND_UP(buffer_size, 64);

    /* Check if requested buffer fits within a single MTU */
    if ( ( buffer_size < 1300 ) ) /* TODO: Fix this */
    {
        uint16_t actual_packet_size;

        if ( wiced_packet_create_udp( dtls_context->send_context, buffer_size, (wiced_packet_t**) &dtls_context->outgoing_packet, buffer, &actual_packet_size ) != WICED_SUCCESS )
        {
            *buffer = NULL;
            return DTLS_ERROR;
        }

        *buffer -= sizeof(dtls_record_header_t);
        wiced_packet_set_data_start( (wiced_packet_t*) dtls_context->outgoing_packet, *buffer );
    }
    else
    {
        /* TODO : Handle If requested size is bigger than MTU. */
    }

    return DTLS_SUCCESS;
}

void* dtls_host_malloc( const char* name, uint32_t size )
{
    (void) name;
    return malloc_named( name, size );
}

void dtls_host_free( void* p )
{
    free( p );
}

/*
 * Flush any data not yet written
 */
dtls_result_t dtls_flush_output( dtls_context_t* context, dtls_session_t* session, uint8_t* buffer, uint32_t length )
{
    wiced_packet_set_data_end( (wiced_packet_t*) context->outgoing_packet, buffer + length );

    /* Send the UDP packet */
    if ( wiced_udp_send( context->send_context, (wiced_ip_address_t*) &session->ip, session->port, (wiced_packet_t*) context->outgoing_packet) != WICED_SUCCESS )
    {
        wiced_packet_delete( (wiced_packet_t*) context->outgoing_packet ); /* Delete packet, since the send failed */
        return DTLS_ERROR;
    }

    return DTLS_SUCCESS;

}

uint64_t dtls_host_get_time_ms( void )
{
    uint64_t time_ms;
    wiced_time_get_utc_time_ms( (wiced_utc_time_ms_t*) &time_ms );
    return time_ms;
}

uint32_t dtls_host_get_time( void )
{
    uint32_t current_time;
    wiced_time_get_time( &current_time );
    return current_time;
}

static besl_result_t wiced_dtls_load_key( wiced_dtls_key_t* key, const char* key_string, const uint32_t key_length )
{
    if ( key->type == TLS_ECC_KEY )
    {
        if ( x509parse_key_ecc( (wiced_dtls_ecc_key_t*) key, (unsigned char *) key_string, key_length, NULL, 0 ) != 0 )
        {
            wiced_assert( "Key parse error", 0 != 0 );
            return BESL_KEY_PARSE_FAIL;
        }
    }
    else
    {
        wiced_assert( "Key type not found", 0 != 0 );
        return BESL_KEY_PARSE_FAIL;
    }

    return BESL_SUCCESS;
}

static besl_result_t wiced_dtls_load_certificate( wiced_dtls_certificate_t* certificate, const uint8_t* certificate_data, uint32_t certificate_length, wiced_tls_certificate_format_t certificate_format )
{
    besl_result_t result;
    uint32_t der_certificate_length;
    uint32_t total_der_bytes;
    x509_name* name_iter = NULL;

    /* Allocate space for temporary processing */
    certificate->processed_certificate_data = malloc_named( "cert", sizeof(x509_cert) );
    if ( certificate->processed_certificate_data == NULL )
    {
        return BESL_TLS_ERROR_OUT_OF_MEMORY;
    }

    memset( certificate->processed_certificate_data, 0, sizeof(x509_cert) );

    switch ( certificate_format )
    {
        case TLS_CERTIFICATE_IN_DER_FORMAT:
            if ( x509_parse_certificate_data( certificate->processed_certificate_data, certificate_data, certificate_length ) != 0 )
            {
                result = BESL_CERT_PARSE_FAIL;
                goto end;
            }
            certificate->certificate_data        = certificate_data;
            certificate->certificate_data_length = certificate_length;
            break;

        case TLS_CERTIFICATE_IN_PEM_FORMAT:
            x509_convert_pem_to_der( certificate_data, certificate_length, &certificate->certificate_data, &total_der_bytes );
            certificate->certificate_data_malloced = WICED_TRUE;
            certificate->certificate_data_length = total_der_bytes;

            der_certificate_length = x509_read_cert_length( certificate->certificate_data );
            if ( x509_parse_certificate_data( certificate->processed_certificate_data, certificate->certificate_data, der_certificate_length ) != 0 )
            {
                result = BESL_CERT_PARSE_FAIL;
                goto end;
            }
            break;
    }

    /* Take ownership of the public_key and common_name */
    certificate->public_key  = certificate->processed_certificate_data->public_key;

    name_iter = &certificate->processed_certificate_data->subject;
    for (; name_iter != NULL; name_iter = name_iter->next)
    {
        if (name_iter->val.p[name_iter->val.len] == 0x30)
        {
            char* temp = malloc_named("tls", name_iter->val.len+1);
            if (temp == NULL)
            {
                result = BESL_TLS_ERROR_OUT_OF_MEMORY;
                goto end;
            }
            memcpy(temp, name_iter->val.p, name_iter->val.len);
            temp[name_iter->val.len] = 0;

            certificate->common_name = temp;
            break;
        }
    }
    //certificate->common_name = certificate->processed_certificate_data->subject;

    /* NULL the public_key pointer in the temporary certificate so x509_free() doesn't free the memory */
    certificate->processed_certificate_data->public_key = NULL;

    x509_free( certificate->processed_certificate_data );
    free( certificate->processed_certificate_data );
    certificate->processed_certificate_data = NULL;

    return BESL_SUCCESS;

end:
    x509_free( certificate->processed_certificate_data );
    free( certificate->processed_certificate_data );
    certificate->processed_certificate_data = NULL;

    if ( certificate->certificate_data_malloced == WICED_TRUE )
    {
        free( (void*)certificate->certificate_data );
        certificate->certificate_data = NULL;
    }

    return result;
}

wiced_result_t wiced_dtls_init_context( wiced_dtls_context_t* context, wiced_dtls_identity_t* identity, const char* peer_cn )
{
    memset( context, 0, sizeof(wiced_dtls_context_t) );

    context->identity = identity;
    return WICED_SUCCESS;
}

wiced_result_t wiced_dtls_init_identity( wiced_dtls_identity_t* identity, wiced_dtls_security_type_t type, void* data )
{
    besl_result_t result;
    wiced_result_t ret;

    memset( identity, 0, sizeof( *identity ) );

    if ( type == WICED_DTLS_SECURITY_TYPE_PSK )
    {
        wiced_dtls_psk_info_t* psk_info = (wiced_dtls_psk_info_t*) data;

        linked_list_init( &identity->private_key.psk.psk_identity_list );
        if ( ( ret = wiced_dtls_add_psk_identity( identity, psk_info ) ) != WICED_SUCCESS )
        {
            linked_list_deinit( &identity->private_key.psk.psk_identity_list );
            return ret;
        }
    }
    else if ( type == WICED_DTLS_SECURITY_TYPE_NONPSK )
    {
        wiced_dtls_nonpsk_info_t* cert_info = (wiced_dtls_nonpsk_info_t*) data;

        wiced_assert( "Bad args", (identity != NULL) && (cert_info->private_key != NULL) && (cert_info->certificate_data != NULL) );

        identity->private_key.common.type = TLS_ECC_KEY;
        result = wiced_dtls_load_certificate( &identity->certificate, cert_info->certificate_data, cert_info->certificate_length, TLS_CERTIFICATE_IN_PEM_FORMAT );
        if ( result != BESL_SUCCESS )
        {
            return ( wiced_result_t )result;
        }

        result = wiced_dtls_load_key( (wiced_dtls_key_t*)&identity->private_key, cert_info->private_key, cert_info->key_length );
        if ( result != BESL_SUCCESS )
        {
            return result;
        }
    }
    else
    {
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_dtls_add_psk_identity( wiced_dtls_identity_t* identity, wiced_dtls_psk_info_t* psk_identity )
{
    wiced_result_t result;

    result = linked_list_insert_node_at_rear( &identity->private_key.psk.psk_identity_list, &psk_identity->this_node );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_ERROR(("Error in adding PSK identity-key pair in list\n"));
        return result;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_dtls_remove_psk_identity( wiced_dtls_identity_t* identity, wiced_dtls_psk_info_t* psk_identity )
{
    wiced_result_t result;

    result = linked_list_remove_node ( &identity->private_key.psk.psk_identity_list, &psk_identity->this_node );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_ERROR(("Error in removing PSK identity-key pair from list\n"));
        return result;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_dtls_deinit_identity( wiced_dtls_identity_t* identity, wiced_dtls_security_type_t type )
{
    if ( type == WICED_DTLS_SECURITY_TYPE_PSK )
    {
        linked_list_deinit( &identity->private_key.psk.psk_identity_list );
    }
    else if ( type == WICED_DTLS_SECURITY_TYPE_NONPSK )
    {
        if ( identity->certificate.processed_certificate_data != NULL )
        {
            x509_free( identity->certificate.processed_certificate_data );
            identity->certificate.processed_certificate_data = NULL;
        }

        if ( identity->certificate.public_key != NULL )
        {
            dtls_host_free( identity->certificate.public_key );
            identity->certificate.public_key = NULL;
        }

        if ( identity->certificate.common_name != NULL )
        {
            dtls_host_free( (void*)identity->certificate.common_name );
            identity->certificate.common_name = NULL;
        }

        if ( identity->certificate.certificate_data_malloced == WICED_TRUE )
        {
            dtls_host_free( (void*)identity->certificate.certificate_data );
            identity->certificate.certificate_data = NULL;
        }
    }
    else
    {
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_dtls_reset_context( wiced_dtls_context_t* dtls_context )
{
    dtls_free( (dtls_context_t*)&dtls_context->context, NULL );
    return WICED_SUCCESS;
}

wiced_result_t wiced_udp_enable_dtls( wiced_udp_socket_t* socket, void* context )
{
    socket->dtls_context = context;
    return WICED_SUCCESS;
}

wiced_result_t wiced_dtls_deinit_context( wiced_dtls_context_t* dtls_context )
{
    dtls_event_message_t current_event;

    current_event.event_type = DTLS_STOP_EVENT;
    current_event.data = NULL;

    wiced_rtos_push_to_queue( (wiced_queue_t*)dtls_context->event_queue, &current_event, WICED_NO_WAIT );

    if ( wiced_rtos_is_current_thread((wiced_thread_t*) dtls_context->event_thread ) != WICED_SUCCESS )
    {
        /* Wakeup DTLS event thread */
        wiced_rtos_thread_force_awake((wiced_thread_t*) dtls_context->event_thread );
    }

    /* Wait for the event to completely get processed. */
    wiced_rtos_thread_join((wiced_thread_t*) dtls_context->event_thread );

    /* Delete the threads */
    wiced_rtos_delete_thread((wiced_thread_t*) dtls_context->event_thread );

    memset (dtls_context, 0, sizeof (wiced_dtls_context_t));
    return WICED_SUCCESS;
}

wiced_result_t wiced_dtls_receive_packet( wiced_udp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout )
{
    return WICED_SUCCESS;
}

wiced_result_t wiced_udp_start_dtls( wiced_udp_socket_t* socket, wiced_ip_address_t ip, wiced_dtls_endpoint_type_t type, wiced_dtls_certificate_verification_t verification )
{
    return wiced_generic_start_dtls_with_ciphers( socket->dtls_context, socket, ip, type, ( wiced_tls_certificate_verification_t )verification, my_ciphers, DTLS_UDP_TRANSPORT );
}

static wiced_result_t dtls_receive_callback( wiced_udp_socket_t* socket, void *args )
{
    dtls_event_message_t current_event;
    wiced_result_t result;

    current_event.event_type = DTLS_RECEIVE_EVENT;
    current_event.data = socket;
    if ( ( result =  wiced_rtos_push_to_queue( (wiced_queue_t*)socket->dtls_context->event_queue, &current_event, WICED_NO_WAIT ) ) != WICED_SUCCESS )
    {
        WPRINT_LIB_ERROR((" Error in DTLS_RECEIVE_EVENT push to queue \n"));
        return result;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_generic_start_dtls_with_ciphers( wiced_dtls_context_t* dtls_context, void* send_context, wiced_ip_address_t ip, wiced_dtls_endpoint_type_t type, wiced_dtls_certificate_verification_t verification, const cipher_suite_t* cipher_list[ ], dtls_transport_protocol_t transport_protocol )
{
    wiced_result_t  result;
    microrng_state  rngstate;

    memset( &dtls_context->context, 0, sizeof(wiced_dtls_workspace_t) );
    dtls_context->context.ciphers = cipher_list;

    wiced_crypto_get_random(dtls_context->context.cookie_secret, 32);

    wiced_crypto_get_random( &rngstate.entropy, 4 );
    microrng_init( &rngstate );

    dtls_context->context.f_rng = microrng_rand;
    dtls_context->context.p_rng = &rngstate;

    /* initialize linked list for retransmission and peer list */
    linked_list_init (&dtls_context->context.peer_list);

    dtls_context->context.identity = dtls_context->identity;

    dtls_context->event_queue = &dtls_event_queue;
    WICED_VERIFY( wiced_rtos_init_queue( (wiced_queue_t*)dtls_context->event_queue, NULL, sizeof(dtls_event_message_t), DTLS_EVENT_QUEUE_DEPTH ) );

    dtls_context->event_thread = &dtls_thread;
    if ( ( result = wiced_rtos_create_thread( (wiced_thread_t*)dtls_context->event_thread, DTLS_THREAD_PRIORITY, "DTLS server", dtls_event_thread, DTLS_THREAD_STACK_SIZE, send_context ) ) != WICED_SUCCESS )
    {
        WPRINT_LIB_ERROR(("Error in creation of DTLS thread\n"));
        return result;
    }

    if ( ( result = wiced_udp_register_callbacks( (wiced_udp_socket_t*)send_context, dtls_receive_callback, dtls_context->callback_arg ) ) != WICED_SUCCESS )
    {
        WPRINT_LIB_ERROR(("Error in registering udp callback\n"));
        return result;
    }

    dtls_context->context.retransmission_timer_event = &dtls_timed_event;
    if ( ( result = wiced_rtos_register_timed_event( (wiced_timed_event_t*)dtls_context->context.retransmission_timer_event, WICED_NETWORKING_WORKER_THREAD, &dtls_retransmission_check_timer_callback, DTLS_RETRANSMISSION_CHECK_TIMER_INTERVAL, send_context ) ) != WICED_SUCCESS )
    {
        WPRINT_LIB_ERROR(("Error in registration of timed event \n"));
        return result;
    }

    /* TODO : Add DTLS client code here */

    return WICED_SUCCESS;

}

static void dtls_event_thread( uint32_t arg )
{
    wiced_udp_socket_t* socket = (wiced_udp_socket_t*) arg;
    dtls_event_message_t current_event;
    wiced_result_t result;
    wiced_bool_t quit = WICED_FALSE;

    UNUSED_PARAMETER(result);

    while ( quit != WICED_TRUE )
    {
        result = wiced_rtos_pop_from_queue( (wiced_queue_t*)socket->dtls_context->event_queue, &current_event, WICED_NEVER_TIMEOUT );

        wiced_assert("unable to pop from queue : dtls_server_thread_main", result == WICED_SUCCESS);

        switch ( current_event.event_type )
        {
            case DTLS_RECEIVE_EVENT:
            {
                /* Receive Event */
                dtls_process_request(socket);
            }
            break;

            case DTLS_RETRANSMISSION_CHECK_EVENT:
            {
                dtls_check_retransmission( current_event.data );
            }
            break;

            case DTLS_STOP_EVENT:
            {
                dtls_peer_t *current_peer, *peer;

                wiced_udp_unregister_callbacks(socket);
                wiced_rtos_deinit_timer(socket->dtls_context->context.retransmission_timer_event);

                /* Remove retransmission nodes and peers */
                linked_list_get_front_node( &socket->dtls_context->context.peer_list, (linked_list_node_t**) &current_peer );
                while ( current_peer != NULL )
                {
                    peer = current_peer;
                    dtls_flush_peer_retransmission_list( &socket->dtls_context->context, peer );
                    current_peer = (dtls_peer_t*) peer->this_node.next;
                    dtls_remove_peer( &socket->dtls_context->context, peer);
                }

                linked_list_deinit( &socket->dtls_context->context.peer_list );
                wiced_rtos_deinit_queue((wiced_queue_t*) socket->dtls_context->event_queue );
                quit = WICED_TRUE;
            }
            break;

            default:
            wiced_assert("Wrong Event : DTLS_thread_main",1);
            break;
        }
    }

    WICED_END_OF_CURRENT_THREAD( );
}

wiced_result_t dtls_retransmit( wiced_dtls_context_t* dtls_context, dtls_retransmission_node_t* retransmission_node )
{
    uint8_t* buffer;
    uint16_t actual_packet_size;
    wiced_packet_t* packet;
    dtls_security_parameters_t* security;
    int ret = 0;

    if ( wiced_packet_create_udp( dtls_context->context.send_context, DEFAULT_UDP_PACKET_SIZE, &packet, &buffer, &actual_packet_size ) != WICED_SUCCESS )
    {
        buffer = NULL;
        return DTLS_ERROR;
    }

    buffer -= sizeof(dtls_record_header_t);
    wiced_packet_set_data_start( (wiced_packet_t*) packet, buffer );

    memcpy( buffer + sizeof(dtls_record_header_t), retransmission_node->packet, retransmission_node->length );

    security = dtls_security_params_epoch ( retransmission_node->peer, retransmission_node->epoch );

    if ( ( ret = dtls_prepare_record( retransmission_node->peer, &dtls_context->context, security, retransmission_node->type, (dtls_record_t*) buffer, retransmission_node->length, 0 ) ) < 0 )
    {
        WPRINT_LIB_ERROR ((" problem in preparing record for type : %d", retransmission_node->type));
        wiced_packet_delete(packet);
        return DTLS_ERROR;
    }

    wiced_packet_set_data_end( (wiced_packet_t*) packet, buffer + ret + sizeof(dtls_record_header_t) );

    /* Send the UDP packet */
    if ( wiced_udp_send( dtls_context->context.send_context, (wiced_ip_address_t*) &retransmission_node->peer->session.ip, retransmission_node->peer->session.port, packet) != WICED_SUCCESS )
    {
        wiced_packet_delete( packet ); /* Delete packet, since the send failed */
        return DTLS_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t dtls_check_retransmission( void *arg )
{
    wiced_udp_socket_t* socket = (wiced_udp_socket_t*) arg;
    dtls_peer_t *current_peer, *next_peer;
    dtls_retransmission_node_t *current_retransmission_node, *next_retransmission_node = NULL;
    uint32_t current_time;
    wiced_bool_t is_retransmission_count_exceeded = WICED_FALSE;

    /* Go over all the pees and find out how many retransmission nodes has timer expired and need to retransmit */
    linked_list_get_front_node( &socket->dtls_context->context.peer_list, (linked_list_node_t**) &current_peer );
    while ( current_peer != NULL )
    {
        linked_list_get_front_node( &current_peer->retransmission_list, (linked_list_node_t**) &current_retransmission_node );
        while( current_retransmission_node != NULL )
        {
            /* change cipherspec and finished message included in retransmission list but we should not consider them to retransmit */
            if ( current_retransmission_node->retransmit == WICED_TRUE )
            {
                if ( current_retransmission_node->retransmit_cnt < DTLS_MAX_RETRANSMISSION_COUNT )
                {
                    wiced_time_get_time( &current_time );

                    if ( current_retransmission_node->retransmit_timestamp < current_time )
                    {
                        dtls_retransmit( socket->dtls_context, current_retransmission_node);

                        /* reset retransmit timestamp and retransmit interval */
                        current_retransmission_node->retransmit_cnt++;
                        current_retransmission_node->retransmit_timestamp = current_time + ( current_retransmission_node->retransmit_interval * 2 ); /* double */
                        current_retransmission_node->retransmit_interval = ( current_retransmission_node->retransmit_interval * 2 );
                    }
                    next_retransmission_node = (dtls_retransmission_node_t*) current_retransmission_node->this_node.next;
                    current_retransmission_node = next_retransmission_node;
                }
                else
                {
                    next_retransmission_node = (dtls_retransmission_node_t*) current_retransmission_node->this_node.next;

                    dtls_free_retransmission_buffer( current_peer, current_retransmission_node );

                    /* make sure we only free peer once all retransmission nodes are freed */
                    if ( current_peer->retransmission_list.count == 0)
                    {
                        is_retransmission_count_exceeded = WICED_TRUE;
                    }

                    current_retransmission_node = next_retransmission_node;
                }
            }
            else
            {
                current_retransmission_node = (dtls_retransmission_node_t*) current_retransmission_node->this_node.next;
            }
        }

        next_peer = (dtls_peer_t*) current_peer->this_node.next;

        if ( is_retransmission_count_exceeded == WICED_TRUE )
        {
            dtls_remove_peer( &socket->dtls_context->context, current_peer );
            is_retransmission_count_exceeded = WICED_FALSE;
        }

        current_peer = next_peer;
    }

    return WICED_SUCCESS;
}

wiced_result_t dtls_retransmission_check_timer_callback( void *arg )
{
    wiced_udp_socket_t* socket = (wiced_udp_socket_t*) arg;
    dtls_event_message_t current_event;

    current_event.event_type = DTLS_RETRANSMISSION_CHECK_EVENT;
    current_event.data = socket;
    wiced_rtos_push_to_queue((wiced_queue_t*)socket->dtls_context->event_queue, &current_event, WICED_NO_WAIT );

    return WICED_SUCCESS;
}

wiced_result_t wiced_dtls_close_notify( wiced_udp_socket_t* socket )
{
    dtls_close_notify( &socket->dtls_context->context);
    return WICED_SUCCESS;
}

dtls_result_t dtls_host_free_packet( uint32_t* packet )
{
    wiced_packet_delete( (wiced_packet_t*) packet );
    return DTLS_SUCCESS;
}

dtls_result_t dtls_host_receive_packet( dtls_context_t* context, dtls_packet_t** packet, uint32_t timeout )
{
    dtls_result_t result = DTLS_RECEIVE_FAILED;
    result = (dtls_result_t) network_udp_receive( (wiced_udp_socket_t*) context->send_context, (wiced_packet_t**) packet, timeout );

    return result;
}

dtls_result_t dtls_host_set_packet_start( tls_packet_t* packet, uint8_t* start )
{
    wiced_packet_set_data_start( (wiced_packet_t*) packet, start );
    return DTLS_SUCCESS;
}

/*
 * Calculates the maximium amount of payload that can fit in a given sized buffer
 */
wiced_result_t wiced_dtls_calculate_overhead( wiced_dtls_workspace_t* context, uint16_t available_space, uint16_t* header, uint16_t* footer )
{
    *header = 0;
    *footer = 0;

    *header = sizeof(dtls_record_header_t);

    /* TODO : Add MAC size based on cipher suite currently only support AES CCM so
     * made it constant 8. will change once we add MAC and cipher driver. */
    *footer += 8;

    return WICED_SUCCESS;
}

wiced_result_t wiced_dtls_encrypt_packet( wiced_dtls_workspace_t* workspace, const wiced_ip_address_t* IP, uint16_t port, wiced_packet_t* packet )
{
    uint8_t* data;
    uint16_t length;
    uint16_t available;
    wiced_result_t result;
    dtls_session_t target;
    int res;

    if ( ( workspace == NULL ) || ( packet == NULL ) )
    {
        return WICED_ERROR;
    }

    if ( wiced_packet_get_data( packet, 0, &data, &length, &available ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }

    data -= sizeof(dtls_record_header_t);
    result = (wiced_result_t) dtls_host_set_packet_start( (dtls_packet_t*) packet, data );

    if ( result != WICED_SUCCESS )
    {
        return result;
    }

    memcpy( &target.ip, IP, sizeof(wiced_ip_address_t) );
    target.port = port;

    dtls_peer_t *peer = dtls_get_peer( workspace, &target );

    /* Check if peer connection already exists */
    if ( !peer )
    { /* no ==> create one */

        WPRINT_SECURITY_INFO(( "peer not found : wiced_dtls_encrypt_packet\n" ));
        return WICED_ERROR;
    }
    else
    {
        if ( peer->context.state != DTLS_HANDSHAKE_OVER )
        {
            return WICED_ERROR;
        }
        else
        {
            res = dtls_prepare_record( peer, workspace, dtls_security_params( peer ), DTLS_CT_APPLICATION_DATA, (dtls_record_t*) data, length, 1 );
            if ( res < 0 )
            {
                return WICED_ERROR;
            }
        }

        wiced_packet_set_data_end( packet, data + res + sizeof(dtls_record_header_t) );

    }
    return WICED_SUCCESS;
}

dtls_result_t dtls_process_request (wiced_udp_socket_t* socket )
{
    wiced_packet_t*             packet = NULL;
    dtls_peer_t*                peer = NULL;
    uint8_t*                    data;
    dtls_record_t*              record;
    uint16_t                    record_length;
    uint8_t*                    request_string = NULL;
    uint16_t                    request_length;
    uint16_t                    available_data_length;
    dtls_session_t              peer_session;
    dtls_peer_data              peer_data;
    dtls_result_t               result;
    wiced_result_t              res;
    dtls_handshake_message_t*   handshake_msg = NULL;

    /* Receive Packet from client */
    if ( ( res = wiced_udp_receive (socket, &packet, WICED_NO_WAIT) ) != WICED_SUCCESS )
    {
        WPRINT_SECURITY_ERROR(("UDP Receive failed"));
        return res;
    }

    dtls_host_get_packet_data(&socket->dtls_context->context, (dtls_packet_t*)packet, 0, &request_string, &request_length, &available_data_length);
    if ( request_string == NULL )
    {
        return DTLS_ERROR;
    }

    memset ( &peer_session, 0, sizeof ( peer_session ) );
    dtls_host_packet_get_info( (uint32_t*)packet, &peer_session);

    socket->dtls_context->context.send_context = socket;

    WPRINT_SECURITY_DEBUG ( ("UDP Rx from IP %u.%u.%u.%u:%d\n",
                (unsigned char) ( ( GET_IPV4_ADDRESS(peer_session.ip) >> 24 ) & 0xff ),
                (unsigned char) ( ( GET_IPV4_ADDRESS(peer_session.ip) >> 16 ) & 0xff ),
                (unsigned char) ( ( GET_IPV4_ADDRESS(peer_session.ip) >> 8 ) & 0xff ),
                (unsigned char) ( ( GET_IPV4_ADDRESS(peer_session.ip) >> 0 ) & 0xff ),
                 peer_session.port ));

    /* check if we have DTLS state for addr/port */
    peer = dtls_get_peer(&socket->dtls_context->context, &peer_session);

    while( request_length > 0 )
    {
        record = (dtls_record_t*) request_string;
        record_length = ( record->record_header.length[ 0 ] << 8 ) | ( record->record_header.length[ 1 ] );

        switch ( record->record_header.content_type )
        {
            case DTLS_CT_HANDSHAKE :
            {
                int data_length;
                data_length = record_length;

                if ( peer && peer->context.state != DTLS_HANDSHAKE_OVER )
                {
                   /* check received epoch with expected epoch, if it doesn't match then just go and process the next record */
                   if ( dtls_check_epoch( peer, htobe16(record->record_header.epoch) ) != DTLS_SUCCESS )
                   {
                       goto next_record;
                   }
                }

                if ( peer && peer->context.state == DTLS_CLIENT_FINISHED )
                {
                    data_length = decrypt_verify( &socket->dtls_context->context, &peer_session, record, record_length, &data );
                    if ( data_length < 0 )
                    {
                        wiced_packet_delete (packet);
                        return DTLS_ERROR_DECRYPTION_FAIL;
                    }

                    memcpy( record->data, data, data_length );
                    record->record_header.length[ 0 ] = data_length;
                    record->record_header.length[ 1 ] = data_length >> 8;

                    /* verify MAC, if MAC is matches then continue, or else send alert to peer and remove from list */
                    if ( verify_mac( &socket->dtls_context->context, peer, record, data_length) != DTLS_SUCCESS )
                    {
                        dtls_write_alert_msg( &socket->dtls_context->context, &peer->session, DTLS_ALERT_LEVEL_FATAL, DTLS_ALERT_BAD_RECORD_MAC );
                        dtls_flush_peer_retransmission_list( &socket->dtls_context->context, peer );
                        dtls_remove_peer ( &socket->dtls_context->context, peer );
                        wiced_packet_delete( packet );
                        return DTLS_SUCCESS;
                    }
                }

                if ( peer)
                {
                     handshake_msg = (dtls_handshake_message_t*) ( record->data );

                     /* process handshake packets which match with our expected sequence number */
                     if ( ( htobe16( handshake_msg->handshake_header.message_seq ) ) == ( peer->handshake_params->hs_state.mseq_r ) )
                     {
                         dtls_flush_peer_retransmission_list( &socket->dtls_context->context, peer );
                     }
                     else if (( htobe16(handshake_msg->handshake_header.message_seq) ) < ( peer->handshake_params->hs_state.mseq_r ))
                     {
                         handshake_msg = (dtls_handshake_message_t*) ( record->data );

                         /* if handshake is over with peer but somehow we dont get disconnection (alert) form client. and when client tries to reconnect by sending client hello packet.
                          * then we should free the previous peer entry and start the handshake again with the peer.
                          */
                         if( peer && peer->context.state == DTLS_HANDSHAKE_OVER && handshake_msg->handshake_header.msg_type == DTLS_HT_CLIENT_HELLO )
                         {
                             dtls_flush_peer_retransmission_list(&socket->dtls_context->context, peer);
                             dtls_remove_peer( &socket->dtls_context->context, peer);
                             peer = NULL;
                         }
                         else
                         {
                             /* when client retransmit the previous flight, we should send the packet which are there in retransmission buffer */
                             dtls_retransmit_buffered_packets( socket->dtls_context, peer );
                             wiced_packet_delete (packet);
                             return DTLS_SUCCESS;
                         }
                     }
                     else
                     {
                         /* According to RFC 6347, there is trade off between bandwidth and space for buffering the out of order packets. currently we discarding out of order packets so client needs to retransmit */
                         WPRINT_LIB_DEBUG ((" out of order \n"));
                         goto next_record;
                    }
                 }

                if( ( result = dtls_handshake_server_async (&socket->dtls_context->context, &peer_session, peer, record, data_length + sizeof(dtls_record_header_t)) ) != DTLS_SUCCESS )
                {
                    wiced_packet_delete ( packet );
                    return DTLS_ERROR;
                }
            }

            break;

            case DTLS_CT_CHANGE_CIPHER_SPEC:
            {
                int hdr_len;
                hdr_len = record_length;

                if ( peer )
                {
                    /* check received epoch with expected epoch, if it doesn't match then just go and process the next record */
                    if ( dtls_check_epoch( peer, htobe16(record->record_header.epoch) ) != DTLS_SUCCESS )
                    {
                        goto next_record;
                    }

                    if( ( result = dtls_handshake_server_async (&socket->dtls_context->context, &peer_session, peer, record, hdr_len + sizeof(dtls_record_header_t)) ) != DTLS_SUCCESS )
                    {
                        wiced_packet_delete ( packet );
                        return DTLS_SUCCESS;
                    }
                }

            }
            break;

            case DTLS_CT_ALERT:
            {
                int data_length;
                data_length = record_length;

                if ( peer )
                {
                    /* check received epoch with expected epoch, if it doesn't match then just go and proces the next record */
                     if ( dtls_check_epoch( peer, htobe16(record->record_header.epoch) ) != DTLS_SUCCESS )
                     {
                         goto next_record;
                     }

                    /* flush peer retransmission list */
                    dtls_flush_peer_retransmission_list( &socket->dtls_context->context, peer );

                    /* decrypt the alert record */
                    data_length = decrypt_verify( &socket->dtls_context->context, &peer_session, record, record_length, &data );
                    if ( data_length < 0 )
                    {
                        wiced_packet_delete (packet);
                        return DTLS_ERROR_DECRYPTION_FAIL;
                    }

                    memcpy( record->data, data, data_length );
                    record->record_header.length[ 0 ] = data_length;
                    record->record_header.length[ 1 ] = data_length >> 8;
                }

                /* Received close notify or fatal alert from peer, invalidate peer and remove from list */
                if ( record->data[0] == DTLS_ALERT_LEVEL_FATAL || record->data[1] == DTLS_ALERT_CLOSE_NOTIFY )
                {
                    /* if handshake has been finished then notify event callback with disconnect event */
                    if ( peer->context.state == DTLS_HANDSHAKE_OVER )
                    {
                        peer_data.event = DTLS_EVENT_TYPE_DISCONNECTED;
                        peer_data.callback_args = socket->dtls_context->callback_arg;
                        peer_data.packet = NULL;
                        memcpy(&peer_data.session, &peer_session, sizeof(dtls_session_t));

                        /* Call registered event callback when client is disconnected from server */
                        socket->dtls_context->callback (socket, &peer_data);
                    }

                    dtls_remove_peer( &socket->dtls_context->context, peer );
                    peer = NULL;
                }
            }
            break;
            case DTLS_CT_APPLICATION_DATA :
            {
                if ( peer )
                {
                    /* check received epoch with expected epoch, if it doesn't match then just go and proces the next record */
                    if ( dtls_check_epoch( peer, htobe16(record->record_header.epoch) ) != DTLS_SUCCESS )
                    {
                        goto next_record;
                    }
                }

                if ( peer && peer->context.state == DTLS_HANDSHAKE_OVER )
                {
                    /* flush retransmission list */
                    dtls_flush_peer_retransmission_list( &socket->dtls_context->context, peer );

                    int data_length = decrypt_verify( &socket->dtls_context->context, &peer_session, record, record_length, &data );
                    if ( data_length < 0 )
                    {
                        wiced_packet_delete (packet);
                        return DTLS_ERROR_DECRYPTION_FAIL;
                    }

                    memcpy( record->data, data, data_length );
                    record->record_header.length[ 0 ] = data_length;
                    record->record_header.length[ 1 ] = data_length >> 8;

                    wiced_packet_set_data_start ( (wiced_packet_t*)packet, record->data );
                    wiced_packet_set_data_end( (wiced_packet_t*) packet, record->data + data_length  );

                    peer_data.event = DTLS_EVENT_TYPE_APP_DATA;
                    peer_data.callback_args = socket->dtls_context->callback_arg;
                    peer_data.packet = (tls_packet_t*)packet;

                    memcpy(&peer_data.session, &peer_session, sizeof(dtls_session_t));

                    /* Call application registered event callback with DTLS application data, Its upto application to delete packet */
                    socket->dtls_context->callback (socket, &peer_data);

                }
            }
            break;
            default:
            break;
        }

next_record:
        request_length -= record_length + sizeof(dtls_record_header_t);

        if ( request_length > 0 )
        {
            request_string   +=  record_length + sizeof(dtls_record_header_t);
        }
        else
        {
            /* delete only handshake packets. Don't delete application data packets, since it will be freed by application */
            if ( record->record_header.content_type != DTLS_CT_APPLICATION_DATA )
            {
                wiced_packet_delete(packet);
            }
        }
    }

    return DTLS_SUCCESS;

}

static void dtls_retransmit_buffered_packets( wiced_dtls_context_t* context, dtls_peer_t *peer)
{
    dtls_retransmission_node_t *current_node, *next_node;
    uint32_t current_time;

    linked_list_get_front_node( &peer->retransmission_list, (linked_list_node_t**) &current_node );
    while( current_node != NULL )
    {
        dtls_retransmit( context, (dtls_retransmission_node_t*) current_node );

        /* reset retransmit timestamp */
        wiced_time_get_time( &current_time );
        current_node->retransmit_timestamp = current_time + ( current_node->retransmit_interval ); /* double */

        next_node = (dtls_retransmission_node_t*) current_node->this_node.next;
        current_node = next_node;
    }
}
