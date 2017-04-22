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

  u8g_cursor.c

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

void u8g_SetCursorFont(u8g_t *u8g, const u8g_pgm_uint8_t *cursor_font)
{
  u8g->cursor_font = cursor_font;
}

void u8g_SetCursorStyle(u8g_t *u8g, uint8_t encoding)
{
  u8g->cursor_encoding = encoding;
}

void u8g_SetCursorColor(u8g_t *u8g, uint8_t fg, uint8_t bg)
{
  u8g->cursor_bg_color = bg;
  u8g->cursor_fg_color = fg;
}

void u8g_SetCursorPos(u8g_t *u8g, u8g_uint_t cursor_x, u8g_uint_t cursor_y)
{
  u8g->cursor_x = cursor_x;
  u8g->cursor_y = cursor_y;
}

void u8g_EnableCursor(u8g_t *u8g)
{
    u8g->cursor_fn = u8g_DrawCursor;
}

void u8g_DisableCursor(u8g_t *u8g)
{
    u8g->cursor_fn = (u8g_draw_cursor_fn)0;
}

void u8g_DrawCursor(u8g_t *u8g)
{
  const u8g_pgm_uint8_t *font;
  uint8_t color;
  uint8_t encoding = u8g->cursor_encoding;
  
  /* get current values */
  color = u8g_GetColorIndex(u8g);
  font = u8g->font;
  
  /* draw cursor */
  u8g->font = u8g->cursor_font;  
  encoding++;
  u8g_SetColorIndex(u8g, u8g->cursor_bg_color); 
  /* 27. Jan 2013: replaced call to u8g_DrawGlyph with call to u8g_draw_glyph */
  /* required, because y adjustment should not happen to the cursor fonts */
  u8g_draw_glyph(u8g, u8g->cursor_x, u8g->cursor_y, encoding);
  encoding--;
  u8g_SetColorIndex(u8g, u8g->cursor_fg_color); 
  /* 27. Jan 2013: replaced call to u8g_DrawGlyph with call to u8g_draw_glyph */
  /* required, because y adjustment should not happen to the cursor fonts */
  /* u8g_DrawGlyph(u8g, u8g->cursor_x, u8g->cursor_y, encoding); */
  u8g_draw_glyph(u8g, u8g->cursor_x, u8g->cursor_y, encoding);
  
  /* restore previous values */
  u8g->font = font;
  u8g_SetColorIndex(u8g, color); 
}

