/******************************************************************************
*
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
*
*****************************************************************************/

#ifndef WICED_BT_LOGMSG
#define WICED_BT_LOGMSG

#include "wiced.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/
#define ScrLog( trace_set_mask, fmt_str, ... ) LogMsg (trace_set_mask, fmt_str, ##__VA_ARGS__ )

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
 *               Function Declarations
 ******************************************************/

/** \brief Initializes logging data structures
 *
 *   This function initializes the Bluetooth Audio logging infrastructure. Must be invoked
 *   prior to using any of the other logging APIs
 *
 *   \return WICED_SUCCESS : on success;
 *              WICED_ERROR    : if an error occurred
 *
 **/
    wiced_result_t LogMsg_init(void);


/** \brief Shuts down the logging infrastructure
 *
 *      This function tears down the Bluetooth audio logging mechanism
 *
 *   \return WICED_SUCCESS : on success;
 *              WICED_ERROR    : if an error occurred
 **/
    wiced_result_t LogMsg_cleanup(void);


/** \brief Turn on/off Bluetooth tracing
 *
 *    This function allows the caller to enable or disable logging of BT library traces.
 *
 *   \param enable    tracing is enabled when this is set to TRUE, else disabled.
 *
 **/
    void LogMsg_enable_log(uint8_t enable);


/** \brief Log a trace buffer.
 *
 *    This function is called to output the trace data typically to a console
 *
 * \param trace_set_mask    Indicates the trace level one of TRACE_TYPE_WARNING,
 *                                    TRACE_TYPE_API, TRACE_TYPE_ERROR or TRACE_TYPE_DEBUG
 *
 * \param format   Specifies how subsequent arguments are converted.
 *
 */
    void LogMsg(uint32_t trace_set_mask, const char *format, ...);


/** \brief Build a trace buffer.
 *
 *    This function is called to build a trace buffer based on the format string
 *
 *   \param buffer   Trace buffer to be built
 *
 *   \param format   Specifies how subsequent arguments are converted.
 *
 *   \return WICED_SUCCESS : on success;
 *              WICED_ERROR    : if an error occurred
 */
    wiced_result_t LogMsg_build_trace_buffer(char* buffer, char* format, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //#ifndef WICED_BT_LOGMSG
