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

#include "up_internal.h"

#include <arch/irq.h>
#include "irq/irq.h"

#include "platform_appscr4.h"

void up_disable_irq(int irq)
{
  platform_irq_disable_irq( irq );
}

void up_enable_irq(int irq)
{
  platform_irq_enable_irq( irq );
}

uint32_t platform_irq_demuxer_hook(uint32_t mask)
{
  int i;

  for (i = 0; mask && (i < NR_IRQS); i++)
  {
    const uint32_t single_irq_mask = IRQN2MASK(i);

    if ((mask & single_irq_mask) && g_irqvector[i] && (g_irqvector[i] != irq_unexpected_isr))
    {
      irq_dispatch(i, (uint32_t *)current_regs);

      /*
       * This makes all registered interrupts be handled here and not propagated to main WICED code.
       * It is possible (by not clearing bit here) to make double handling - here and then in main WICED code.
       */
      mask &= ~single_irq_mask;
    }
  }

  return mask;
}
