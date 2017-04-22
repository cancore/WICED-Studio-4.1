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
#include <nuttx/clock.h>

#include "wdog/wdog.h"

#include <stdio.h>
#include <sched.h>

#include "cr4.h"
#include "platform_stdio.h"
#include "platform_appscr4.h"
#include "platform_peripheral.h"
#include "platform_sleep.h"

#define MILLISECONDS_TO_MICROSECONDS(x) ((x)*1000)

void up_assert(const uint8_t *filename, int lineno)
{
  dbg("Assertion failed at file:%s line: %d\n", filename, lineno);

#ifdef CONFIG_DEBUG
  asm volatile ("bkpt");
#endif

  platform_mcu_reset();
}

void up_idle(void)
{
#if !defined(CONFIG_SCHED_IDLE_TICKLESS) || defined(WICED_DISABLE_MCU_POWERSAVE)
  cpu_wait_for_interrupt();
#else
  irqstate_t flags;

  flags = irqsave();

  if (platform_power_down_permission())
  {
    uint32_t next_expiration_ticks = wd_timer(0);
    uint32_t adjust_ticks;

    if (next_expiration_ticks == 0)
    {
      /* No scheduled timers. Try maximum time. */
      next_expiration_ticks = UINT32_MAX;
    }
    else
    {
      /* Power-down hook expects number of ticks after the nearest periodic tick. */
      next_expiration_ticks--;
    }

    adjust_ticks = platform_power_down_hook(next_expiration_ticks);
    if (adjust_ticks)
    {
      sched_lock();
      (void)wd_timer(adjust_ticks);
      sched_unlock();
    }
  }

  irqrestore(flags);
#endif
}

void up_allocate_heap(FAR void **heap_start, size_t *heap_size)
{
  /*
   * 4390x BSP supports SOCSRAM banks powering down.
   * If WICED SDK is built with PLATFORM_CORES_POWERSAVE and without PLATFORM_NO_SOCSRAM_POWERDOWN
   * then startup code power down heap banks, and it is up to heap implementation
   * to power them back when memory is allocated.
   * So either implement powering up, or compile WICED SDK with powering down disabled.
   */

  extern unsigned char _heap[];
  extern unsigned char _eheap[];

  *heap_start = &_heap[0];
  *heap_size  = &_eheap[0] - &_heap[0];
}

uint32_t clock_systimer_proxy(void)
{
  return clock_systimer();
}

void clock_systimer_set(uint32_t ticks)
{
#ifdef CONFIG_SCHED_TICKLESS
# error "Not implemented"
#else
  g_system_timer = ticks;
#endif
}

void up_mdelay(unsigned int milliseconds)
{
  platform_udelay(MILLISECONDS_TO_MICROSECONDS(milliseconds));
}

#ifdef putchar
/*
 * GCC during optimization can replace calls to printf() with calls to putchar().
 * NuttX defines putchar() as macro, so no 'putchar' symbol generated and linking fail.
 * Let's make sure we have such symbol.
 */
#undef putchar
int putchar(int c)
{
  return fputc(c, stdout);
}
#endif /* putchar */
