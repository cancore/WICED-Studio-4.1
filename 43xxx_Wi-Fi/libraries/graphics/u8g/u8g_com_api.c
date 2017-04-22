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

  u8g_com_api.c

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

uint8_t u8g_InitCom(u8g_t *u8g, u8g_dev_t *dev, uint8_t clk_cycle_time)
{
  return dev->com_fn(u8g, U8G_COM_MSG_INIT, clk_cycle_time, NULL);
}

void u8g_StopCom(u8g_t *u8g, u8g_dev_t *dev)
{
  dev->com_fn(u8g, U8G_COM_MSG_STOP, 0, NULL);
}

/* cs contains the chip number, which should be enabled */
void u8g_SetChipSelect(u8g_t *u8g, u8g_dev_t *dev, uint8_t cs)
{
  dev->com_fn(u8g, U8G_COM_MSG_CHIP_SELECT, cs, NULL);
}

void u8g_SetResetLow(u8g_t *u8g, u8g_dev_t *dev)
{
  dev->com_fn(u8g, U8G_COM_MSG_RESET, 0, NULL);
}

void u8g_SetResetHigh(u8g_t *u8g, u8g_dev_t *dev)
{
  dev->com_fn(u8g, U8G_COM_MSG_RESET, 1, NULL);
}


void u8g_SetAddress(u8g_t *u8g, u8g_dev_t *dev, uint8_t address)
{
  dev->com_fn(u8g, U8G_COM_MSG_ADDRESS, address, NULL);
}

uint8_t u8g_WriteByte(u8g_t *u8g, u8g_dev_t *dev, uint8_t val)
{
  return dev->com_fn(u8g, U8G_COM_MSG_WRITE_BYTE, val, NULL);
}

uint8_t u8g_WriteSequence(u8g_t *u8g, u8g_dev_t *dev, uint8_t cnt, uint8_t *seq)
{
  return dev->com_fn(u8g, U8G_COM_MSG_WRITE_SEQ, cnt, seq);
}

uint8_t u8g_WriteSequenceP(u8g_t *u8g, u8g_dev_t *dev, uint8_t cnt, const uint8_t *seq)
{
  return dev->com_fn(u8g, U8G_COM_MSG_WRITE_SEQ_P, cnt, (void *)seq);
}

/*
  sequence := { direct_value | escape_sequence }
  direct_value := 0..254
  escape_sequence := value_255 | sequence_end | delay | adr | cs | not_used 
  value_255 := 255 255
  sequence_end = 255 254
  delay := 255 0..127
  adr := 255 0x0e0 .. 0x0ef 
  cs := 255 0x0d0 .. 0x0df 
  not_used := 255 101..254

#define U8G_ESC_DLY(x) 255, ((x) & 0x7f)
#define U8G_ESC_CS(x) 255, (0xd0 | ((x)&0x0f))
#define U8G_ESC_ADR(x) 255, (0xe0 | ((x)&0x0f))
#define U8G_ESC_VCC(x) 255, (0xbe | ((x)&0x01))
#define U8G_ESC_END 255, 254
#define U8G_ESC_255 255, 255
#define U8G_ESC_RST(x) 255, (0xc0 | ((x)&0x0f))

*/
uint8_t u8g_WriteEscSeqP(u8g_t *u8g, u8g_dev_t *dev, const uint8_t *esc_seq)
{
  uint8_t is_escape = 0;
  uint8_t value;
  for(;;)
  {
    value = u8g_pgm_read(esc_seq);
    if ( is_escape == 0 )
    {
      if ( value != 255 )
      {
        if ( u8g_WriteByte(u8g, dev, value) == 0 )
          return 0;
      }
      else
      {
        is_escape = 1;
      }
    }
    else
    {
      if ( value == 255 )
      {
        if ( u8g_WriteByte(u8g, dev, value) == 0 )
          return 0;
      }
      else if ( value == 254 )
      {
        break;
      }
      else if ( value >= 0x0f0 )
      {
        /* not yet used, do nothing */
      }
      else if ( value >= 0xe0  )
      {
        u8g_SetAddress(u8g, dev, value & 0x0f);
      }
      else if ( value >= 0xd0 )
      {
        u8g_SetChipSelect(u8g, dev, value & 0x0f);
      }
      else if ( value >= 0xc0 )
      {
        u8g_SetResetLow(u8g, dev);
        value &= 0x0f;
        value <<= 4;
        value+=2;
        u8g_Delay(value);
        u8g_SetResetHigh(u8g, dev);
        u8g_Delay(value);
      }
      else if ( value >= 0xbe )
      {
	/* not yet implemented */
        /* u8g_SetVCC(u8g, dev, value & 0x01); */
      }
      else if ( value <= 127 )
      {
        u8g_Delay(value);
      }
      is_escape = 0;
    }
    esc_seq++;
  }
  return 1;
}

