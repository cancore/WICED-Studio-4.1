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

#include <nuttx/config.h>
#include <nuttx/arch.h>
#include <nuttx/init.h>
#include <nuttx/watchdog.h>
#include <nuttx/syslog/ramlog.h>
#include <nuttx/net/loopback.h>

#include "up_internal.h"

#include "platform_appscr4.h"

#include "wiced_management.h"

void _start_low(void);
void _start_platform_init(void);

void _start(void)
{
  _start_low();

  os_start();
}

void up_initialize(void)
{
  _start_platform_init();

  platform_tick_start();

#ifdef CONFIG_RAMLOG_SYSLOG
  ramlog_sysloginit();
#endif

#ifdef CONFIG_DEV_NULL
  devnull_register();
#endif

#ifdef CONFIG_DEV_RANDOM
  up_rnginitialize();
#endif

  up_serialinit();

  up_wdginitialize();

  irqenable();
}

#ifdef CONFIG_BOARD_INITIALIZE
#ifndef CONFIG_BOARD_INITTHREAD
#  error Network initialization required to be in separate thread after the whole NuttX initialization done
#endif
void board_initialize(void)
{
  wiced_core_init();
  up_netinitialize();
#ifdef CONFIG_NETDEV_LOOPBACK
/* Initialize the local loopback device */
(void)localhost_initialize();
#endif

}
#endif
