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

  u8g_scale.c

  Universal 8bit Graphics Library
  
  Copyright (c) 2012, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  

  Scale screen by some constant factors. Usefull for making bigger fonts wiht less
  memory consumption
    
*/

#include "u8g.h"

uint8_t u8g_dev_scale_2x2_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg);


u8g_dev_t u8g_dev_scale = { u8g_dev_scale_2x2_fn, NULL, NULL };

void u8g_UndoScale(u8g_t *u8g)
{
  if ( u8g->dev != &u8g_dev_scale )
    return;
  u8g->dev = u8g_dev_scale.dev_mem;
  u8g_UpdateDimension(u8g);
}

void u8g_SetScale2x2(u8g_t *u8g)
{
  if ( u8g->dev != &u8g_dev_scale )
  {
    u8g_dev_scale.dev_mem = u8g->dev;
    u8g->dev = &u8g_dev_scale;
  }
  u8g_dev_scale.dev_fn = u8g_dev_scale_2x2_fn;
  u8g_UpdateDimension(u8g);
}


uint8_t u8g_dev_scale_2x2_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg)
{
  u8g_dev_t *chain = (u8g_dev_t *)(dev->dev_mem);
  uint8_t pixel;
  uint16_t scaled_pixel;
  uint8_t i;
  uint8_t dir;
  u8g_uint_t x, y, xx,yy;
  
  switch(msg)
  {
    default:
      return u8g_call_dev_fn(u8g, chain, msg, arg);
    case U8G_DEV_MSG_GET_WIDTH:
      *((u8g_uint_t *)arg) = u8g_GetWidthLL(u8g, chain) / 2;
      break;
    case U8G_DEV_MSG_GET_HEIGHT:
      *((u8g_uint_t *)arg) = u8g_GetHeightLL(u8g, chain) / 2;
      break;
    case U8G_DEV_MSG_GET_PAGE_BOX:
      /* get page size from next device in the chain */
      u8g_call_dev_fn(u8g, chain, msg, arg);
      ((u8g_box_t *)arg)->x0 /= 2;
      ((u8g_box_t *)arg)->x1 /= 2;
      ((u8g_box_t *)arg)->y0 /= 2;
      ((u8g_box_t *)arg)->y1 /= 2;
      return 1;
    case U8G_DEV_MSG_SET_PIXEL:
      x = ((u8g_dev_arg_pixel_t *)arg)->x;
      x *= 2;
      y = ((u8g_dev_arg_pixel_t *)arg)->y;
      y *= 2;
      ((u8g_dev_arg_pixel_t *)arg)->x = x;
      ((u8g_dev_arg_pixel_t *)arg)->y = y;
      u8g_call_dev_fn(u8g, chain, msg, arg);
      x++;
      ((u8g_dev_arg_pixel_t *)arg)->x = x;
      ((u8g_dev_arg_pixel_t *)arg)->y = y;
      u8g_call_dev_fn(u8g, chain, msg, arg);
      y++;
      ((u8g_dev_arg_pixel_t *)arg)->x = x;
      ((u8g_dev_arg_pixel_t *)arg)->y = y;
      u8g_call_dev_fn(u8g, chain, msg, arg);
      x--;
      ((u8g_dev_arg_pixel_t *)arg)->x = x;
      ((u8g_dev_arg_pixel_t *)arg)->y = y;
      u8g_call_dev_fn(u8g, chain, msg, arg);    
      break;
    case U8G_DEV_MSG_SET_8PIXEL:
      pixel = ((u8g_dev_arg_pixel_t *)arg)->pixel;
      dir = ((u8g_dev_arg_pixel_t *)arg)->dir;
      scaled_pixel = 0;
      for( i = 0; i < 8; i++ )
      {
	scaled_pixel<<=2;
	if ( pixel & 128 )
	{
	  scaled_pixel |= 3;
	}
	pixel<<=1;
      }
      x = ((u8g_dev_arg_pixel_t *)arg)->x;
      x *= 2;
      xx = x;
      y = ((u8g_dev_arg_pixel_t *)arg)->y;
      y *= 2;
      yy = y;
      if ( ((u8g_dev_arg_pixel_t *)arg)->dir & 1 )
      {
	xx++;
      }
      else
      {
	yy++;
      }
      
      ((u8g_dev_arg_pixel_t *)arg)->pixel = scaled_pixel>>8;      
      ((u8g_dev_arg_pixel_t *)arg)->x = x;
      ((u8g_dev_arg_pixel_t *)arg)->y = y;
      ((u8g_dev_arg_pixel_t *)arg)->dir = dir;
      u8g_call_dev_fn(u8g, chain, msg, arg);    

      
      ((u8g_dev_arg_pixel_t *)arg)->x = xx;
      ((u8g_dev_arg_pixel_t *)arg)->y = yy;
      ((u8g_dev_arg_pixel_t *)arg)->dir = dir;
      u8g_call_dev_fn(u8g, chain, msg, arg);    
      
      ((u8g_dev_arg_pixel_t *)arg)->pixel = scaled_pixel&255;
      //((u8g_dev_arg_pixel_t *)arg)->pixel = 0x00;
      switch(dir)
      {
	case 0:
 	  x+=8;
	  xx+=8;
	  break;
	case 1:
	  y+=8;
	  yy+=8;
	  break;
	case 2:
	  x-=8;
	  xx-=8;
	  break;
	case 3:
	  y-=8;
	  yy-=8;
	  break;
      }
      ((u8g_dev_arg_pixel_t *)arg)->x = x;
      ((u8g_dev_arg_pixel_t *)arg)->y = y;
      ((u8g_dev_arg_pixel_t *)arg)->dir = dir;
      u8g_call_dev_fn(u8g, chain, msg, arg);    
      
      ((u8g_dev_arg_pixel_t *)arg)->x = xx;
      ((u8g_dev_arg_pixel_t *)arg)->y = yy;
      ((u8g_dev_arg_pixel_t *)arg)->dir = dir;
      u8g_call_dev_fn(u8g, chain, msg, arg);    
      break;
  }
  return 1;
}

