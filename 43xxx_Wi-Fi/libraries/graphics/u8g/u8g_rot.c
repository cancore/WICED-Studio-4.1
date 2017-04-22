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

  u8g_rot.c

  Universal 8bit Graphics Library
  
  Copyright (c) 2011, olikraus@gmail.com
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

uint8_t u8g_dev_rot90_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg);
uint8_t u8g_dev_rot180_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg);
uint8_t u8g_dev_rot270_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg);

uint8_t u8g_dev_rot_dummy_fn(void *u8g, void *dev, uint8_t msg, void *arg)
{
  return 0;
}

u8g_dev_t u8g_dev_rot;


void u8g_UndoRotation(u8g_t *u8g)
{
  if ( u8g->dev != &u8g_dev_rot )
    return;
  u8g->dev = u8g_dev_rot.dev_mem;
  u8g_UpdateDimension(u8g);
}

void u8g_SetRot90(u8g_t *u8g)
{
  if ( u8g->dev != &u8g_dev_rot )
  {
    u8g_dev_rot.dev_mem = u8g->dev;
    u8g->dev = &u8g_dev_rot;
  }
  u8g_dev_rot.dev_fn = u8g_dev_rot90_fn;
  u8g_UpdateDimension(u8g);
}

void u8g_SetRot180(u8g_t *u8g)
{
  if ( u8g->dev != &u8g_dev_rot )
  {
    u8g_dev_rot.dev_mem = u8g->dev;
    u8g->dev = &u8g_dev_rot;
  }
  u8g_dev_rot.dev_fn = u8g_dev_rot180_fn;
  u8g_UpdateDimension(u8g);
}

void u8g_SetRot270(u8g_t *u8g)
{
  if ( u8g->dev != &u8g_dev_rot )
  {
    u8g_dev_rot.dev_mem = u8g->dev;
    u8g->dev = &u8g_dev_rot;
  }
  u8g_dev_rot.dev_fn = u8g_dev_rot270_fn;
  u8g_UpdateDimension(u8g);
}

uint8_t u8g_dev_rot90_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg)
{
  u8g_dev_t *rotation_chain = (u8g_dev_t *)(dev->dev_mem);
  switch(msg)
  {
    default:
    /*
    case U8G_DEV_MSG_INIT:
    case U8G_DEV_MSG_STOP:
    case U8G_DEV_MSG_PAGE_FIRST:
    case U8G_DEV_MSG_PAGE_NEXT:
    case U8G_DEV_MSG_SET_COLOR_ENTRY:
    case U8G_DEV_MSG_SET_XY_CB:
    */
      return u8g_call_dev_fn(u8g, rotation_chain, msg, arg);
#ifdef U8G_DEV_MSG_IS_BBX_INTERSECTION
    case U8G_DEV_MSG_IS_BBX_INTERSECTION:
      {
        u8g_dev_arg_bbx_t *bbx = (u8g_dev_arg_bbx_t *)arg;
        u8g_uint_t x, y, tmp;
        
        /* transform the reference point */
        y = bbx->x;
        x = u8g->height;
        /* x = u8g_GetWidthLL(u8g, rotation_chain); */
        x -= bbx->y; 
        x--;
        
        /* adjust point to be the uppler left corner again */
        x -= bbx->h;
        x++;
        
        /* swap box dimensions */        
        tmp = bbx->w;
        bbx->w = bbx->h;
        bbx->h = tmp;
        
        /* store x,y */
        bbx->x = x;
        bbx->y = y;
      }
      return u8g_call_dev_fn(u8g, rotation_chain, msg, arg);
#endif /* U8G_DEV_MSG_IS_BBX_INTERSECTION */
    case U8G_DEV_MSG_GET_PAGE_BOX:
      /* get page size from next device in the chain */
      u8g_call_dev_fn(u8g, rotation_chain, msg, arg);
      //printf("pre x: %3d..%3d y: %3d..%3d   ", ((u8g_box_t *)arg)->x0, ((u8g_box_t *)arg)->x1, ((u8g_box_t *)arg)->y0, ((u8g_box_t *)arg)->y1);
      {
	u8g_box_t new_box;
	//new_box.x0 = u8g_GetHeightLL(u8g,rotation_chain) - ((u8g_box_t *)arg)->y1 - 1;
	//new_box.x1 = u8g_GetHeightLL(u8g,rotation_chain) - ((u8g_box_t *)arg)->y0 - 1;
	
	new_box.x0 = ((u8g_box_t *)arg)->y0;
	new_box.x1 = ((u8g_box_t *)arg)->y1;
	new_box.y0 = ((u8g_box_t *)arg)->x0;
	new_box.y1 = ((u8g_box_t *)arg)->x1;
	*((u8g_box_t *)arg) = new_box;
	//printf("post x: %3d..%3d y: %3d..%3d\n", ((u8g_box_t *)arg)->x0, ((u8g_box_t *)arg)->x1, ((u8g_box_t *)arg)->y0, ((u8g_box_t *)arg)->y1);
      }
      break;
    case U8G_DEV_MSG_GET_WIDTH:
      *((u8g_uint_t *)arg) = u8g_GetHeightLL(u8g,rotation_chain);
      break;
    case U8G_DEV_MSG_GET_HEIGHT:
      *((u8g_uint_t *)arg) = u8g_GetWidthLL(u8g, rotation_chain);
      break;
    case U8G_DEV_MSG_SET_PIXEL:
    case U8G_DEV_MSG_SET_TPIXEL:
      {
        u8g_uint_t x, y;
        y = ((u8g_dev_arg_pixel_t *)arg)->x;
        x = u8g_GetWidthLL(u8g, rotation_chain);
        x -= ((u8g_dev_arg_pixel_t *)arg)->y; 
        x--;
        ((u8g_dev_arg_pixel_t *)arg)->x = x;
        ((u8g_dev_arg_pixel_t *)arg)->y = y;
      }
      u8g_call_dev_fn(u8g, rotation_chain, msg, arg);
      break;
    case U8G_DEV_MSG_SET_8PIXEL:
    case U8G_DEV_MSG_SET_4TPIXEL:
      {
        u8g_uint_t x, y;
	//uint16_t x,y;
        y = ((u8g_dev_arg_pixel_t *)arg)->x;
        x = u8g_GetWidthLL(u8g, rotation_chain);
        x -= ((u8g_dev_arg_pixel_t *)arg)->y; 
        x--;
        ((u8g_dev_arg_pixel_t *)arg)->x = x;
        ((u8g_dev_arg_pixel_t *)arg)->y = y;
        ((u8g_dev_arg_pixel_t *)arg)->dir+=1;
        ((u8g_dev_arg_pixel_t *)arg)->dir &= 3;
      }
      u8g_call_dev_fn(u8g, rotation_chain, msg, arg);
      break;
  }
  return 1;
}

uint8_t u8g_dev_rot180_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg)
{
  u8g_dev_t *rotation_chain = (u8g_dev_t *)(dev->dev_mem);
  switch(msg)
  {
    default:
    /*
    case U8G_DEV_MSG_INIT:
    case U8G_DEV_MSG_STOP:
    case U8G_DEV_MSG_PAGE_FIRST:
    case U8G_DEV_MSG_PAGE_NEXT:
    case U8G_DEV_MSG_SET_COLOR_ENTRY:
    case U8G_DEV_MSG_SET_XY_CB:
    */
      return u8g_call_dev_fn(u8g, rotation_chain, msg, arg);
#ifdef U8G_DEV_MSG_IS_BBX_INTERSECTION
    case U8G_DEV_MSG_IS_BBX_INTERSECTION:
      {
        u8g_dev_arg_bbx_t *bbx = (u8g_dev_arg_bbx_t *)arg;
        u8g_uint_t x, y;
        
        /* transform the reference point */
        //y = u8g_GetHeightLL(u8g, rotation_chain);
        y = u8g->height;
        y -= bbx->y; 
        y--;
        
        //x = u8g_GetWidthLL(u8g, rotation_chain);
        x = u8g->width;
        x -= bbx->x; 
        x--;
        
        /* adjust point to be the uppler left corner again */
        y -= bbx->h;
        y++;
        
        x -= bbx->w;
        x++;
                
        /* store x,y */
        bbx->x = x;
        bbx->y = y;
      }
      return u8g_call_dev_fn(u8g, rotation_chain, msg, arg);
#endif /* U8G_DEV_MSG_IS_BBX_INTERSECTION */
    case U8G_DEV_MSG_GET_PAGE_BOX:
      /* get page size from next device in the chain */
      u8g_call_dev_fn(u8g, rotation_chain, msg, arg);
      //printf("pre x: %3d..%3d y: %3d..%3d   ", ((u8g_box_t *)arg)->x0, ((u8g_box_t *)arg)->x1, ((u8g_box_t *)arg)->y0, ((u8g_box_t *)arg)->y1);
      {
	u8g_box_t new_box;
	
	new_box.x0 = u8g_GetWidthLL(u8g,rotation_chain) - ((u8g_box_t *)arg)->x1 - 1;
	new_box.x1 = u8g_GetWidthLL(u8g,rotation_chain) - ((u8g_box_t *)arg)->x0 - 1;
	new_box.y0 = u8g_GetHeightLL(u8g,rotation_chain) - ((u8g_box_t *)arg)->y1 - 1;
	new_box.y1 = u8g_GetHeightLL(u8g,rotation_chain) - ((u8g_box_t *)arg)->y0 - 1;
	*((u8g_box_t *)arg) = new_box;
	//printf("post x: %3d..%3d y: %3d..%3d\n", ((u8g_box_t *)arg)->x0, ((u8g_box_t *)arg)->x1, ((u8g_box_t *)arg)->y0, ((u8g_box_t *)arg)->y1);
      }
      break;
    case U8G_DEV_MSG_GET_WIDTH:
      *((u8g_uint_t *)arg) = u8g_GetWidthLL(u8g,rotation_chain);
      break;
    case U8G_DEV_MSG_GET_HEIGHT:
      *((u8g_uint_t *)arg) = u8g_GetHeightLL(u8g, rotation_chain);
      break;
    case U8G_DEV_MSG_SET_PIXEL:
    case U8G_DEV_MSG_SET_TPIXEL:
      {
        u8g_uint_t x, y;

        y = u8g_GetHeightLL(u8g, rotation_chain);
        y -= ((u8g_dev_arg_pixel_t *)arg)->y; 
        y--;
        
        x = u8g_GetWidthLL(u8g, rotation_chain);
        x -= ((u8g_dev_arg_pixel_t *)arg)->x; 
        x--;
        
        ((u8g_dev_arg_pixel_t *)arg)->x = x;
        ((u8g_dev_arg_pixel_t *)arg)->y = y;
      }
      u8g_call_dev_fn(u8g, rotation_chain, msg, arg);
      break;
    case U8G_DEV_MSG_SET_8PIXEL:
    case U8G_DEV_MSG_SET_4TPIXEL:
      {
        u8g_uint_t x, y;
        
        y = u8g_GetHeightLL(u8g, rotation_chain);
        y -= ((u8g_dev_arg_pixel_t *)arg)->y; 
        y--;
        
        x = u8g_GetWidthLL(u8g, rotation_chain);
        x -= ((u8g_dev_arg_pixel_t *)arg)->x; 
        x--;
        
        ((u8g_dev_arg_pixel_t *)arg)->x = x;
        ((u8g_dev_arg_pixel_t *)arg)->y = y;
        ((u8g_dev_arg_pixel_t *)arg)->dir+=2;
        ((u8g_dev_arg_pixel_t *)arg)->dir &= 3;
      }
      u8g_call_dev_fn(u8g, rotation_chain, msg, arg);
      break;
  }
  return 1;
}

uint8_t u8g_dev_rot270_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg)
{
  u8g_dev_t *rotation_chain = (u8g_dev_t *)(dev->dev_mem);
  switch(msg)
  {
    default:
    /*
    case U8G_DEV_MSG_INIT:
    case U8G_DEV_MSG_STOP:
    case U8G_DEV_MSG_PAGE_FIRST:
    case U8G_DEV_MSG_PAGE_NEXT:
    case U8G_DEV_MSG_SET_COLOR_ENTRY:
    case U8G_DEV_MSG_SET_XY_CB:
    */
      return u8g_call_dev_fn(u8g, rotation_chain, msg, arg);
#ifdef U8G_DEV_MSG_IS_BBX_INTERSECTION
    case U8G_DEV_MSG_IS_BBX_INTERSECTION:
      {
        u8g_dev_arg_bbx_t *bbx = (u8g_dev_arg_bbx_t *)arg;
        u8g_uint_t x, y, tmp;
        
        /* transform the reference point */
        x = bbx->y;
        
        y = u8g->width;
        /* y = u8g_GetHeightLL(u8g, rotation_chain); */
        y -= bbx->x; 
        y--;
                
        /* adjust point to be the uppler left corner again */
        y -= bbx->w;
        y++;
        
        /* swap box dimensions */        
        tmp = bbx->w;
        bbx->w = bbx->h;
        bbx->h = tmp;
        
        /* store x,y */
        bbx->x = x;
        bbx->y = y;
      }
      return u8g_call_dev_fn(u8g, rotation_chain, msg, arg);
#endif /* U8G_DEV_MSG_IS_BBX_INTERSECTION */
    case U8G_DEV_MSG_GET_PAGE_BOX:
      /* get page size from next device in the chain */
      u8g_call_dev_fn(u8g, rotation_chain, msg, arg);
      //printf("pre x: %3d..%3d y: %3d..%3d   ", ((u8g_box_t *)arg)->x0, ((u8g_box_t *)arg)->x1, ((u8g_box_t *)arg)->y0, ((u8g_box_t *)arg)->y1);
      {
	u8g_box_t new_box;
	
	new_box.x0 = u8g_GetHeightLL(u8g,rotation_chain) - ((u8g_box_t *)arg)->y1 - 1;
	new_box.x1 = u8g_GetHeightLL(u8g,rotation_chain) - ((u8g_box_t *)arg)->y0 - 1;
	new_box.y0 = u8g_GetWidthLL(u8g,rotation_chain) - ((u8g_box_t *)arg)->x1 - 1;
	new_box.y1 = u8g_GetWidthLL(u8g,rotation_chain) - ((u8g_box_t *)arg)->x0 - 1;
	*((u8g_box_t *)arg) = new_box;
	//printf("post x: %3d..%3d y: %3d..%3d\n", ((u8g_box_t *)arg)->x0, ((u8g_box_t *)arg)->x1, ((u8g_box_t *)arg)->y0, ((u8g_box_t *)arg)->y1);
      }
      break;
    case U8G_DEV_MSG_GET_WIDTH:
      *((u8g_uint_t *)arg) = u8g_GetHeightLL(u8g,rotation_chain);
      break;
    case U8G_DEV_MSG_GET_HEIGHT:
      *((u8g_uint_t *)arg) = u8g_GetWidthLL(u8g, rotation_chain);
      break;
    case U8G_DEV_MSG_SET_PIXEL:
    case U8G_DEV_MSG_SET_TPIXEL:
      {
        u8g_uint_t x, y;
        x = ((u8g_dev_arg_pixel_t *)arg)->y;
        
        y = u8g_GetHeightLL(u8g, rotation_chain);
        y -= ((u8g_dev_arg_pixel_t *)arg)->x; 
        y--;
          
        /*
        x = u8g_GetWidthLL(u8g, rotation_chain);
        x -= ((u8g_dev_arg_pixel_t *)arg)->y; 
        x--;
        */
        ((u8g_dev_arg_pixel_t *)arg)->x = x;
        ((u8g_dev_arg_pixel_t *)arg)->y = y;
      }
      u8g_call_dev_fn(u8g, rotation_chain, msg, arg);
      break;
    case U8G_DEV_MSG_SET_8PIXEL:
    case U8G_DEV_MSG_SET_4TPIXEL:
      {
        u8g_uint_t x, y;
        x = ((u8g_dev_arg_pixel_t *)arg)->y;
        
        y = u8g_GetHeightLL(u8g, rotation_chain);
        y -= ((u8g_dev_arg_pixel_t *)arg)->x; 
        y--;
          
        /*
        x = u8g_GetWidthLL(u8g, rotation_chain);
        x -= ((u8g_dev_arg_pixel_t *)arg)->y; 
        x--;
        */
        ((u8g_dev_arg_pixel_t *)arg)->x = x;
        ((u8g_dev_arg_pixel_t *)arg)->y = y;
        ((u8g_dev_arg_pixel_t *)arg)->dir+=3;
        ((u8g_dev_arg_pixel_t *)arg)->dir &= 3;
      }
      u8g_call_dev_fn(u8g, rotation_chain, msg, arg);
      break;
  }
  return 1;
}

