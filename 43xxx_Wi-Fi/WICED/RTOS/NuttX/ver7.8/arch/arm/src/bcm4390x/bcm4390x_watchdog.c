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
#include <nuttx/watchdog.h>

#include "platform_peripheral.h"

#ifndef CONFIG_WATCHDOG_DEVPATH
#define CONFIG_WATCHDOG_DEVPATH  "/dev/watchdog0"
#endif

#ifdef CONFIG_WATCHDOG

struct bcm4390x_watchdog_s
{
  struct watchdog_lowerhalf_s lower; /* must be first */
  uint32_t                    timeout_ms;
};

static inline struct bcm4390x_watchdog_s* bcm4390x_watchdog_lowerhalf_to_priv(struct watchdog_lowerhalf_s *lower)
{
  return (struct bcm4390x_watchdog_s*)lower;
}

static int bcm4390x_watchdog_start(struct watchdog_lowerhalf_s *lower)
{
  int result = OK;

  if (platform_watchdog_init() != PLATFORM_SUCCESS)
  {
    result = -EIO;
  }

  return result;
}

static int bcm4390x_watchdog_stop(struct watchdog_lowerhalf_s *lower)
{
  int result = OK;

  if (platform_watchdog_deinit() != PLATFORM_SUCCESS)
  {
    result = -EIO;
  }

  return result;
}

static int bcm4390x_watchdog_keepalive(struct watchdog_lowerhalf_s *lower)
{
  struct bcm4390x_watchdog_s *priv = bcm4390x_watchdog_lowerhalf_to_priv(lower);
  platform_result_t platform_result;

  if (priv->timeout_ms != 0)
  {
    platform_result = platform_watchdog_kick_milliseconds(priv->timeout_ms);
  }
  else
  {
    platform_result = platform_watchdog_kick();
  }

  return (platform_result == PLATFORM_SUCCESS) ? OK : -EIO;
}

static int bcm4390x_watchdog_settimeout(struct watchdog_lowerhalf_s *lower, uint32_t timeout_ms)
{
  struct bcm4390x_watchdog_s *priv = bcm4390x_watchdog_lowerhalf_to_priv(lower);

  priv->timeout_ms = timeout_ms;

  return lower->ops->keepalive(lower);
}

struct watchdog_ops_s bcm4390x_watchdog_ops =
{
  .start      = bcm4390x_watchdog_start,
  .stop       = bcm4390x_watchdog_stop,
  .keepalive  = bcm4390x_watchdog_keepalive,
  .settimeout = bcm4390x_watchdog_settimeout
};

static struct bcm4390x_watchdog_s bcm4390x_watchdog =
{
  .lower      = { .ops = &bcm4390x_watchdog_ops },
  .timeout_ms = 0
};

#endif /* CONFIG_WATCHDOG */

int up_wdginitialize(void)
{
  /*
   * Watchdog is started during early platform initialization.
   * Idea is watchdog would reset board if initalization hang.
   * Even if watchdog support is compiled-in here, let's deinit watchdog
   * and start it again. This is because of watchdog driver initial state
   * is stopped.
   */
  platform_watchdog_deinit();

#ifdef CONFIG_WATCHDOG

  if (watchdog_register(CONFIG_WATCHDOG_DEVPATH, &bcm4390x_watchdog.lower) == NULL)
  {
    DEBUGASSERT(0);
    return -ENODEV;
  }

#endif /* CONFIG_WATCHDOG */

  return OK;
}
