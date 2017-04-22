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
 * Audio Client Utility Routines
 *
 */

#include "wiced_result.h"
#include "wiced_utilities.h"

#include "audio_client_utils.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define HTTP_RESPONSE_LEN       13  /* 'HTTP/x.x xxx ' */

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

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t audio_client_http_get_response_code(char* data, uint16_t data_length, uint16_t *code)
{
    char* ptr;

    /* Check we have enough data to identify the response number */
    if (data_length < HTTP_RESPONSE_LEN)
    {
        return WICED_ERROR;
    }

    /* Find the HTTP/x.x part*/
    ptr = strnstr((const char*)data, data_length, HTTP_HEADER_STR, sizeof(HTTP_HEADER_STR) - 1);
    if (ptr == NULL)
    {
        return WICED_ERROR;
    }

    /* Check that we have enough data before dereferencing it */
    if (((data + data_length) - ptr) < HTTP_RESPONSE_LEN)
    {
       return WICED_ERROR;
    }

    /* Skip the "HTTP/" and the version "x.x"*/
    ptr += 5 + 3;

    /* Verify next character is a space */
    if (*ptr++ != ' ')
    {
        return WICED_ERROR;
    }

    /* Verify response is 3 characters followed by a space */
    if (ptr[3] != ' ')
    {
        return WICED_ERROR;
    }

    *code = (uint16_t)atoi(ptr);

    return WICED_SUCCESS;
}
