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

  u8g_bitmap.c

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

void u8g_DrawHBitmap(u8g_t *u8g, u8g_uint_t x, u8g_uint_t y, u8g_uint_t cnt, const uint8_t *bitmap)
{
  while( cnt > 0 )
  {
    u8g_Draw8Pixel(u8g, x, y, 0, *bitmap);
    bitmap++;
    cnt--;
    x+=8;
  }
}

void u8g_DrawBitmap(u8g_t *u8g, u8g_uint_t x, u8g_uint_t y, u8g_uint_t cnt, u8g_uint_t h, const uint8_t *bitmap)
{
  if ( u8g_IsBBXIntersection(u8g, x, y, cnt*8, h) == 0 )
    return;
  while( h > 0 )
  {
    u8g_DrawHBitmap(u8g, x, y, cnt, bitmap);
    bitmap += cnt;
    y++;
    h--;
  }
}


void u8g_DrawHBitmapP(u8g_t *u8g, u8g_uint_t x, u8g_uint_t y, u8g_uint_t cnt, const u8g_pgm_uint8_t *bitmap)
{
  while( cnt > 0 )
  {
    u8g_Draw8Pixel(u8g, x, y, 0, u8g_pgm_read(bitmap));
    bitmap++;
    cnt--;
    x+=8;
  }
}

void u8g_DrawBitmapP(u8g_t *u8g, u8g_uint_t x, u8g_uint_t y, u8g_uint_t cnt, u8g_uint_t h, const u8g_pgm_uint8_t *bitmap)
{
  if ( u8g_IsBBXIntersection(u8g, x, y, cnt*8, h) == 0 )
    return;
  while( h > 0 )
  {
    u8g_DrawHBitmapP(u8g, x, y, cnt, bitmap);
    bitmap += cnt;
    y++;
    h--;
  }
}

/*=========================================================================*/

static void u8g_DrawHXBM(u8g_t *u8g, u8g_uint_t x, u8g_uint_t y, u8g_uint_t w, const uint8_t *bitmap)
{
  uint8_t d;
  x+=7;
  while( w >= 8 )
  {
    u8g_Draw8Pixel(u8g, x, y, 2, *bitmap);
    bitmap++;
    w-= 8;
    x+=8;
  }
  if ( w > 0 )
  {
    d = *bitmap;
    x -= 7;
    do
    {
      if ( d & 1 )
        u8g_DrawPixel(u8g, x, y);
      x++;
      w--;
      d >>= 1;      
    } while ( w > 0 );
  }
}

void u8g_DrawXBM(u8g_t *u8g, u8g_uint_t x, u8g_uint_t y, u8g_uint_t w, u8g_uint_t h, const uint8_t *bitmap)
{
  u8g_uint_t b;
  b = w;
  b += 7;
  b >>= 3;
  
  if ( u8g_IsBBXIntersection(u8g, x, y, w, h) == 0 )
    return;
  
  while( h > 0 )
  {
    u8g_DrawHXBM(u8g, x, y, w, bitmap);
    bitmap += b;
    y++;
    h--;
  }
}

static void u8g_DrawHXBMP(u8g_t *u8g, u8g_uint_t x, u8g_uint_t y, u8g_uint_t w, const u8g_pgm_uint8_t *bitmap)
{
  uint8_t d;
  x+=7;
  while( w >= 8 )
  {
    u8g_Draw8Pixel(u8g, x, y, 2, u8g_pgm_read(bitmap));
    bitmap++;
    w-= 8;
    x+=8;
  }
  if ( w > 0 )
  {
    d = u8g_pgm_read(bitmap);
    x -= 7;
    do
    {
      if ( d & 1 )
        u8g_DrawPixel(u8g, x, y);
      x++;
      w--;
      d >>= 1;      
    } while ( w > 0 );
  }
}

void u8g_DrawXBMP(u8g_t *u8g, u8g_uint_t x, u8g_uint_t y, u8g_uint_t w, u8g_uint_t h, const u8g_pgm_uint8_t *bitmap)
{
  u8g_uint_t b;
  b = w;
  b += 7;
  b >>= 3;
  
  if ( u8g_IsBBXIntersection(u8g, x, y, w, h) == 0 )
    return;
  while( h > 0 )
  {
    u8g_DrawHXBMP(u8g, x, y, w, bitmap);
    bitmap += b;
    y++;
    h--;
  }
}
