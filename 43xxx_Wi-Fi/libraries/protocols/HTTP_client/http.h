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
#include "wiced_tcpip.h"
#include "wiced_rtos.h"
#include "linked_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define HTTP_CLRF        "\r\n"
#define HTTP_CRLF_CRLF   "\r\n\r\n"
#define HTTP_SPACE       " "
#define HTTP_COLON       ":"

#define HTTP_VERSION_1_0  "HTTP/1.0"
#define HTTP_VERSION_1_1  "HTTP/1.1"
#define HTTP_VERSION_2    "HTTP/2"

#define HTTP_METHOD_OPTIONS  "OPTIONS"
#define HTTP_METHOD_GET      "GET"
#define HTTP_METHOD_HEAD     "HEAD"
#define HTTP_METHOD_POST     "POST"
#define HTTP_METHOD_PUT      "PUT"
#define HTTP_METHOD_DELETE   "DELETE"
#define HTTP_METHOD_TRACE    "TRACE"
#define HTTP_METHOD_CONNECT  "CONNECT"

#define HTTP_HEADER_HOST            "Host: "
#define HTTP_HEADER_DATE            "Date: "
#define HTTP_HEADER_CONTENT_LENGTH  "Content-Length: "
#define HTTP_HEADER_CONTENT_TYPE    "Content-Type: "
#define HTTP_HEADER_CHUNKED         "Transfer-Encoding: chunked"

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    HTTP_1_0,
    HTTP_1_1,
    HTTP_2,
} http_version_t;

/* rfc2616 */
typedef enum
{
    HTTP_UNKNOWN  = -1,
    HTTP_OPTIONS  =  0,
    HTTP_GET      =  1,
    HTTP_HEAD     =  2,
    HTTP_POST     =  3,
    HTTP_PUT      =  4,
    HTTP_DELETE   =  5,
    HTTP_TRACE    =  6,
    HTTP_CONNECT  =  7,

    HTTP_METHODS_MAX,   /* must be last! */
} http_method_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    char*    field;
    uint16_t field_length;
    char*    value;
    uint16_t value_length;
} http_header_field_t;

typedef struct
{
    http_version_t version;
    uint16_t       code;
} http_status_line_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t http_parse_header( const uint8_t* data, uint16_t length, http_header_field_t* header, uint32_t number_of_header_fields );

wiced_result_t http_get_status_line( const uint8_t* data, uint16_t length, http_status_line_t* status_line );

wiced_result_t http_split_line( const char* line, uint16_t max_length, char** next_line );

wiced_result_t http_get_next_line( const char* line, uint16_t max_length, char** next_line );

wiced_result_t http_get_line_length( const char* line, uint32_t max_line_length, uint32_t* actual_length );

wiced_result_t http_get_next_line_with_length( const char* data, uint16_t data_length, char** next_line, uint32_t* line_length );

wiced_result_t http_get_host( const char* line, uint16_t line_length, char** host, uint16_t* host_length, uint16_t* port );

wiced_result_t http_get_next_string_token( const char* string, uint16_t string_length, char delimiter, char** next_token );

#ifdef __cplusplus
} /* extern "C" */
#endif
