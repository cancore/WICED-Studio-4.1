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
#include "tls_types.h"
#include "besl_constants.h"
#include "supplicant_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef void* tls_agent_packet_t;

/******************************************************
 *                Packed Structures
 ******************************************************/

/******************************************************
 *                Unpacked Structures
 ******************************************************/

typedef struct
{
    tls_agent_event_t           event_type;
    union
    {
        tls_agent_packet_t      packet;
        uint32_t                value;
    } data;
} tls_agent_event_message_t;

typedef struct
{
    void*                       tls_agent_host_workspace;
} tls_agent_workspace_t;

typedef struct supplicant_peap_state_s
{
    eap_type_t              eap_type;
    besl_result_t           result;
    supplicant_main_stage_t main_stage;
    uint8_t                 sub_stage;

    uint8_t                 identity[32];
    uint8_t                 identity_length;

    uint8_t                 password[32];
    uint8_t                 password_length;
} supplicant_peap_state_t;

typedef struct supplicant_mschapv2_identity_s
{
    uint8_t*                identity;
    uint8_t                 identity_length;

    /* in Windows password is UNICODE */
    uint8_t*                password;
    uint8_t                 password_length;
}supplicant_mschapv2_identity_t;

typedef struct supplicant_peap_workspace_s* supplicant_peap_workspace_ptr_t;

typedef struct
{
    eap_type_t                  eap_type;
    void*                       supplicant_host_workspace;
    uint32_t                    interface;
    besl_result_t               supplicant_result;

    /* State machine stages */
    supplicant_main_stage_t     current_main_stage;
    uint8_t                     current_sub_stage; /* Either a value from wps_eap_state_machine_stage_t or wps_state_machine_stage_t */

    /* The ID of the last received packet we should use when replying */
    uint8_t                     last_received_id;

    uint8_t                     have_packet;      /* Flags that a packet is already created for TLS records */
    uint32_t                    start_time;

    besl_mac_t                  supplicant_mac_address;
    besl_mac_t                  authenticator_mac_address;
    uint8_t                     outer_eap_identity[32];
    uint8_t                     outer_eap_identity_length;

    wiced_tls_context_t*        tls_context;
    tls_agent_workspace_t       tls_agent;
    uint8_t                     tls_length_overhead;    /* This is a workaround flag for eap_tls_packet lenght parameter */
    uint8_t*                    buffer;    // XXX temporary until we review how the TLS engine is working with EAP transport
    uint32_t                    buffer_size;
    uint8_t*                    data_start;
    uint8_t*                    data_end;

    /* If type is peap */
    supplicant_peap_workspace_ptr_t peap;
} supplicant_workspace_t;

#ifdef __cplusplus
} /*extern "C" */
#endif
