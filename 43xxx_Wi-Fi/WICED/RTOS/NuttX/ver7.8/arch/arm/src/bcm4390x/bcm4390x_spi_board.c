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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdbool.h>
#include <debug.h>
#include <errno.h>

#include <nuttx/spi/spi.h>

#include "wiced_platform.h"

#include "bcm4390x_spi.h"

/* This file has the board-specific initialization for SPI
 * BCM943909WCD1_3 and similar boards of the BCM4390X family.
 * Applied to boards which have a SPI flash on one of SPI ports,
 * and where port described in wiced_spi_flash variable.
 * Other boards with different SPI slave devices
 * should have their own file.
 */

#if defined(CONFIG_BCM4390X_SPI1) || defined(CONFIG_BCM4390X_SPI2)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: bcm4390x_spiinitialize
 *
 * Description:
 *   Map SPI port numbers to SPI Slave devices
 *
 ****************************************************************************/

void weak_function bcm4390x_spiinitialize(int port, wiced_spi_device_t **wiced_spi_device)
{
  *wiced_spi_device = NULL;

#ifdef WICED_PLATFORM_INCLUDES_SPI_FLASH
  if (port == wiced_spi_flash.port)
  {
    *wiced_spi_device = (wiced_spi_device_t *)&wiced_spi_flash;
  }
#endif
}

#endif /* (CONFIG_BCM4390X_SPI1) || defined(CONFIG_BCM4390X_SPI2) */
