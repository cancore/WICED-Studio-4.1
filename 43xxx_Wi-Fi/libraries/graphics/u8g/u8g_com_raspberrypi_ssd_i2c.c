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

/*  
  Special pin usage:
    U8G_PI_I2C_OPTION	additional options
    U8G_PI_A0_STATE	used to store the last value of the command/data register selection
    U8G_PI_SET_A0		1: Signal request to update I2C device with new A0_STATE, 0: Do nothing, A0_STATE matches I2C device
    U8G_PI_SCL		clock line (NOT USED)
    U8G_PI_SDA		data line (NOT USED)
    
    U8G_PI_RESET		reset line (currently disabled, see below)

  Protocol:
    SLA, Cmd/Data Selection, Arguments
    The command/data register is selected by a special instruction byte, which is sent after SLA
    
    The continue bit is always 0 so that a (re)start is equired for the change from cmd to/data mode
*/

#include "u8g.h"

#if defined(U8G_RASPBERRY_PI)

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define I2C_SLA		0x3c
#define I2C_CMD_MODE	0x000
#define I2C_DATA_MODE	0x040

#if defined(U8G_WITH_PINLIST)

uint8_t u8g_com_raspberrypi_ssd_start_sequence(u8g_t *u8g)
{
  /* are we requested to set the a0 state? */
  if ( u8g->pin_list[U8G_PI_SET_A0] == 0 )
    return 1;	
  
  /* setup bus, might be a repeated start */
  if ( u8g_i2c_start(I2C_SLA) == 0 )
    return 0;
  if ( u8g->pin_list[U8G_PI_A0_STATE] == 0 )
  {
    if ( u8g_i2c_send_mode(I2C_CMD_MODE) == 0 )
      return 0;
  }
  else
  {
    if ( u8g_i2c_send_mode(I2C_DATA_MODE) == 0 )
      return 0;
  }
  
  
  u8g->pin_list[U8G_PI_SET_A0] = 0;
  return 1;
}

uint8_t u8g_com_raspberrypi_ssd_i2c_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr)
{
  switch(msg)
  {
    case U8G_COM_MSG_INIT:
      u8g_i2c_init(u8g->pin_list[U8G_PI_I2C_OPTION]);
      u8g_SetPIOutput(u8g, U8G_PI_RESET);
      u8g_SetPIOutput(u8g, U8G_PI_A0);
      break;
    
    case U8G_COM_MSG_STOP:
      break;

    case U8G_COM_MSG_RESET:
      break;
      
    case U8G_COM_MSG_CHIP_SELECT:
      u8g->pin_list[U8G_PI_A0_STATE] = 0;
      u8g->pin_list[U8G_PI_SET_A0] = 1;		/* force a0 to set again, also forces start condition */
      if ( arg_val == 0 )
      {
        /* disable chip, send stop condition */
	u8g_i2c_stop();
     }
      else
      {
        /* enable, do nothing: any byte writing will trigger the i2c start */
      }
      break;

    case U8G_COM_MSG_WRITE_BYTE:
      //u8g->pin_list[U8G_PI_SET_A0] = 1;
      if ( u8g_com_raspberrypi_ssd_start_sequence(u8g) == 0 )
	return u8g_i2c_stop(), 0;
      if ( u8g_i2c_send_byte(arg_val) == 0 )
	return u8g_i2c_stop(), 0;
      // u8g_i2c_stop();
      break;
    
    case U8G_COM_MSG_WRITE_SEQ:
      //u8g->pin_list[U8G_PI_SET_A0] = 1;
      if ( u8g_com_raspberrypi_ssd_start_sequence(u8g) == 0 )
	return u8g_i2c_stop(), 0;
      {
        register uint8_t *ptr = (uint8_t *)arg_ptr;
        while( arg_val > 0 )
        {
	  if ( u8g_i2c_send_byte(*ptr++) == 0 )
	    return u8g_i2c_stop(), 0;
          arg_val--;
        }
      }
      // u8g_i2c_stop();
      break;

    case U8G_COM_MSG_WRITE_SEQ_P:
      //u8g->pin_list[U8G_PI_SET_A0] = 1;
      if ( u8g_com_raspberrypi_ssd_start_sequence(u8g) == 0 )
	return u8g_i2c_stop(), 0;
      {
        register uint8_t *ptr = (uint8_t *)arg_ptr;
        while( arg_val > 0 )
        {
	  if ( u8g_i2c_send_byte(u8g_pgm_read(ptr)) == 0 )
	    return 0;
          ptr++;
          arg_val--;
        }
      }
      // u8g_i2c_stop();
      break;
      
    case U8G_COM_MSG_ADDRESS:                     /* define cmd (arg_val = 0) or data mode (arg_val = 1) */
      u8g->pin_list[U8G_PI_A0_STATE] = arg_val;
      u8g->pin_list[U8G_PI_SET_A0] = 1;		/* force a0 to set again */
    
#ifdef OLD_CODE    
      if ( i2c_state != 0 )
      {
	u8g_i2c_stop();
	i2c_state = 0;
      }

      if ( u8g_com_raspberrypi_ssd_start_sequence(arg_val) == 0 )
	return 0;
    
      /* setup bus, might be a repeated start */
      /*
      if ( u8g_i2c_start(I2C_SLA) == 0 )
	return 0;
      if ( arg_val == 0 )
      {
	i2c_state = 1;
	
	if ( u8g_i2c_send_byte(I2C_CMD_MODE) == 0 )
	  return 0;
      }
      else
      {
	i2c_state = 2;
	if ( u8g_i2c_send_byte(I2C_DATA_MODE) == 0 )
	  return 0;
      }
      */
#endif
      break;
  }
  return 1;
}

#else	/* defined(U8G_WITH_PINLIST) */

uint8_t u8g_com_raspberrypi_ssd_i2c_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr) {
   return 1;
}

#endif	/* defined(U8G_WITH_PINLIST) */
#endif
