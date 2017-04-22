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
/**
 * Most low-end ARM-based MCUs cannot tolerate 50KB code overhead. To eliminate
 * that code you need to define your own, non-throwing versions of global new
 * and delete.
 *
 * @remarks Adopted from http://www.state-machine.com/arm/Building_bare-metal_ARM_with_GNU.pdf
 * @see http://www.state-machine.com/arm/Building_bare-metal_ARM_with_GNU.pdf
 */

#if defined(WICED) && !defined(RTOS_EMBOS)

#include <stdlib.h>
/**
 * The standard version of the operator new throws std::bad_alloc exception.
 * This version explicitly throws no exceptions. This minimal implementation
 * uses the standard malloc().
 */
void *operator new( size_t size )
{
    return malloc( size );
}


/**
 * This minimal implementation uses the standard free().
 */
void operator delete( void *p )
{
    free( p );
}

/**
 * The function __aeabi_atexit() handles the static destructors. In a
 * bare-metal system this function can be empty because application has no
 * operating system to return to, and consequently the static destructors are
 * never called.
 */
extern "C" int __aeabi_atexit( void *object, void (*destructor) (void *), void *dso_handle )
{
    return 0;
}
#endif /* defined(WICED) && !defined(RTOS_EMBOS) */
