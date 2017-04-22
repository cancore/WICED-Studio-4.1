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

  u8g_virtual_screen.c

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
  
  
*/

#include "u8g.h"

struct _u8g_vs_t
{
  u8g_uint_t x;
  u8g_uint_t y;
  u8g_t *u8g;
};
typedef struct _u8g_vs_t u8g_vs_t;

#define U8g_VS_MAX 4
uint8_t u8g_vs_cnt = 0;
u8g_vs_t u8g_vs_list[U8g_VS_MAX]; 
uint8_t u8g_vs_current;
u8g_uint_t u8g_vs_width;
u8g_uint_t u8g_vs_height;

uint8_t u8g_dev_vs_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg)
{
  switch(msg)
  {
    default:
      {
	uint8_t i;
	for( i = 0; i < u8g_vs_cnt; i++ )
	{
	  u8g_call_dev_fn(u8g_vs_list[i].u8g, u8g_vs_list[i].u8g->dev, msg, arg);
	}
      }
      return 1;
    case U8G_DEV_MSG_PAGE_FIRST:
      u8g_vs_current = 0;
      if ( u8g_vs_cnt != 0 )
	return u8g_call_dev_fn(u8g_vs_list[u8g_vs_current].u8g, u8g_vs_list[u8g_vs_current].u8g->dev, msg, arg);
      return 0;
    case U8G_DEV_MSG_PAGE_NEXT:
      {	
	uint8_t ret = 0;
	if ( u8g_vs_cnt != 0 )
	  ret = u8g_call_dev_fn(u8g_vs_list[u8g_vs_current].u8g, u8g_vs_list[u8g_vs_current].u8g->dev, msg, arg);
	if ( ret != 0 )
	  return ret;
	u8g_vs_current++;	/* next device */
	if ( u8g_vs_current >= u8g_vs_cnt )  /* reached end? */
	  return 0;
	return u8g_call_dev_fn(u8g_vs_list[u8g_vs_current].u8g, u8g_vs_list[u8g_vs_current].u8g->dev, U8G_DEV_MSG_PAGE_FIRST, arg);	
      }
      return 0;
    case U8G_DEV_MSG_GET_WIDTH:
      *((u8g_uint_t *)arg) = u8g_vs_width;
      break;
    case U8G_DEV_MSG_GET_HEIGHT:
      *((u8g_uint_t *)arg) = u8g_vs_height;
      break;
    case U8G_DEV_MSG_GET_PAGE_BOX:
      if ( u8g_vs_current < u8g_vs_cnt )
      {
	u8g_call_dev_fn(u8g_vs_list[u8g_vs_current].u8g, u8g_vs_list[u8g_vs_current].u8g->dev, msg, arg);
	((u8g_box_t *)arg)->x0 += u8g_vs_list[u8g_vs_current].x;
	((u8g_box_t *)arg)->x1 += u8g_vs_list[u8g_vs_current].x;
	((u8g_box_t *)arg)->y0 += u8g_vs_list[u8g_vs_current].y;
	((u8g_box_t *)arg)->y1 += u8g_vs_list[u8g_vs_current].y;
      }
      else
      {
	((u8g_box_t *)arg)->x0 = 0;
	((u8g_box_t *)arg)->x1 = 0;
	((u8g_box_t *)arg)->y0 = 0;
	((u8g_box_t *)arg)->y1 = 0;
      }
      return 1;
    case U8G_DEV_MSG_SET_PIXEL:
    case U8G_DEV_MSG_SET_8PIXEL:
      if ( u8g_vs_current < u8g_vs_cnt )
      {
        ((u8g_dev_arg_pixel_t *)arg)->x -= u8g_vs_list[u8g_vs_current].x;
        ((u8g_dev_arg_pixel_t *)arg)->y -= u8g_vs_list[u8g_vs_current].y;
	return u8g_call_dev_fn(u8g_vs_list[u8g_vs_current].u8g, u8g_vs_list[u8g_vs_current].u8g->dev, msg, arg);
      }
      break;
  }
  return 1;
}



u8g_dev_t u8g_dev_vs = { u8g_dev_vs_fn, NULL, NULL };

void u8g_SetVirtualScreenDimension(u8g_t *vs_u8g, u8g_uint_t width, u8g_uint_t height)
{
  if ( vs_u8g->dev != &u8g_dev_vs )
    return; 	/* abort if there is no a virtual screen device */
  u8g_vs_width = width;
  u8g_vs_height = height;  
}

uint8_t u8g_AddToVirtualScreen(u8g_t *vs_u8g, u8g_uint_t x, u8g_uint_t y, u8g_t *child_u8g)
{
  if ( vs_u8g->dev != &u8g_dev_vs )
    return 0; 	/* abort if there is no a virtual screen device */
  if ( u8g_vs_cnt >= U8g_VS_MAX )
    return 0;  	/* maximum number of  child u8g's reached */
  u8g_vs_list[u8g_vs_cnt].u8g = child_u8g;
  u8g_vs_list[u8g_vs_cnt].x = x;
  u8g_vs_list[u8g_vs_cnt].y = y;
  u8g_vs_cnt++;
  return 1;
}

