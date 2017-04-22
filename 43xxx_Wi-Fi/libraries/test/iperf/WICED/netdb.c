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

#ifdef RTOS_EMBOS
#include "bcmtypes.h"
#endif
#include "compat.h"
#include "stdio.h"

/**
 * Returns an entry containing addresses of address family AF_INET
 * for the host with name name.
 * Due to dns_gethostbyname limitations, only one address is returned.
 *
 * @param name the hostname to resolve
 * @return an entry containing addresses of address family AF_INET
 *         for the host with name name
 */
struct hostent* gethostbyname(const char *name)
{
    unsigned long addr;
    static struct hostent s_hostent;
    static char *s_aliases;
    static unsigned long s_hostent_addr;
    static unsigned long *s_phostent_addr[2];

    int temp[4];
    sscanf( name, "%d.%d.%d.%d", &temp[0], &temp[1], &temp[2], &temp[3] );
    addr = temp[3] << 24 | temp[3] << 16 | temp[1] << 8 | temp[0];
//    if (TX_SUCCESS != nx_dns_host_by_name_get(NULL, (UCHAR*) name, &addr, 250))
//    {
//        return NULL;
//    }

    /* fill hostent */
    s_hostent_addr          = addr;
    s_phostent_addr[0]      = &s_hostent_addr;
    s_phostent_addr[1]      = NULL;
    s_hostent.h_name        = (char*) name;
    s_hostent.h_aliases     = &s_aliases;
    s_hostent.h_addrtype    = AF_INET;
    s_hostent.h_length      = sizeof(unsigned long);
    s_hostent.h_addr_list   = (char**) &s_phostent_addr;

    return &s_hostent;
}
