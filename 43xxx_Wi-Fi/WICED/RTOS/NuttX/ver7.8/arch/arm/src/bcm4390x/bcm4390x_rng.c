/*
 * arch/arm/src/bcm4390x/bcm4390x_rng.c
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
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <debug.h>

#include "up_arch.h"
#include "up_internal.h"

#include "wiced_crypto.h"

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static ssize_t rng_read(struct file *filep, char *buffer, size_t);

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct file_operations g_rngops =
{
  . read = rng_read,
};

/****************************************************************************
 * Private functions
 ****************************************************************************/

/****************************************************************************
 * Name: rng_read
 *
 * Description:
 *   This is the standard, NuttX character driver read method
 *
 * Input Parameters:
 *   filep - The VFS file instance
 *   buffer - Buffer in which to return the random samples
 *   buflen - The length of the buffer
 *
 * Returned Value:
 *
 ****************************************************************************/

static ssize_t rng_read(struct file *filep, char *buffer, size_t buflen)
{
  ssize_t retval;
  int ret;
  (void) filep;

  if (buffer == NULL || buflen == 0)
    {
      retval = -EFAULT;
    }
  else
    {
      ret = wiced_crypto_get_random(buffer, buflen);
      if (ret == WICED_SUCCESS)
        {
          retval = buflen;
        }
      else
        {
          retval = -EIO;
        }
    }
  return retval;
}


/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_rnginitialize
 *
 * Description:
 *   Register the /dev/randome driver.
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void up_rnginitialize(void)
{
  int ret;

  ret = register_driver("/dev/random", &g_rngops, 0644, NULL);
  if (ret < 0)
    {
      dbg("ERROR: Failed to register /dev/random\n");
      return;
    }
}






