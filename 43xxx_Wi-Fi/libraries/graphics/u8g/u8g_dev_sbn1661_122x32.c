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

  u8g_dev_sbn1661_122x32.c
  
  WG12232 display with 2xSBN1661 / SED1520 controller (122x32 display)
  At the moment only available in the Arduino Environment

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

#define WIDTH 122
#define HEIGHT 32
#define PAGE_HEIGHT 8


static const uint8_t u8g_dev_sbn1661_122x32_init_seq[] PROGMEM = {
  U8G_ESC_CS(0),             /* disable chip */
  U8G_ESC_ADR(0),           /* instruction mode */
  U8G_ESC_RST(15),           /* do reset low pulse with (15*16)+2 milliseconds */
  U8G_ESC_CS(1),             /* enable chip 1 */
  0x0af,				/* display on */
  0x0c0,				/* display start at line 0 */
  0x0a0,				/* a0: ADC forward, a1: ADC reverse */
  0x0a9,				/* a8: 1/16, a9: 1/32 duty */
  U8G_ESC_CS(2),             /* enable chip 2 */
  0x0af,				/* display on */
  0x0c0,				/* display start at line 0 */
  0x0a0,				/* a0: ADC forward, a1: ADC reverse */
  0x0a9,				/* a8: 1/16, a9: 1/32 duty */
  
  U8G_ESC_CS(0),             /* disable chip */
  
  
  U8G_ESC_END                /* end of sequence */
};

uint8_t u8g_dev_sbn1661_122x32_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg)
{
  switch(msg)
  {
    case U8G_DEV_MSG_INIT:
      u8g_InitCom(u8g, dev, U8G_SPI_CLK_CYCLE_NONE);
      u8g_WriteEscSeqP(u8g, dev, u8g_dev_sbn1661_122x32_init_seq);
      break;
    case U8G_DEV_MSG_STOP:
      break;
    case U8G_DEV_MSG_PAGE_NEXT:
      {
        u8g_pb_t *pb = (u8g_pb_t *)(dev->dev_mem);
	
        u8g_SetAddress(u8g, dev, 0);           /* command mode */
        u8g_SetChipSelect(u8g, dev, 1);
        u8g_WriteByte(u8g, dev, 0x0b8 | pb->p.page); /* select current page (SBN1661/SED1520) */
        u8g_WriteByte(u8g, dev, 0x000 ); /* set X address */
        u8g_SetAddress(u8g, dev, 1);           /* data mode */
        u8g_WriteSequence(u8g, dev, WIDTH/2, pb->buf);
	
        u8g_SetAddress(u8g, dev, 0);           /* command mode */
        u8g_SetChipSelect(u8g, dev, 2);
        u8g_WriteByte(u8g, dev, 0x0b8 | pb->p.page); /* select current page (SBN1661/SED1520) */
        u8g_WriteByte(u8g, dev, 0x000 ); /* set X address */
        u8g_SetAddress(u8g, dev, 1);           /* data mode */
        u8g_WriteSequence(u8g, dev, WIDTH/2, WIDTH/2+(uint8_t *)pb->buf);
	
        u8g_SetChipSelect(u8g, dev, 0);
	
      }
      break;
    case U8G_DEV_MSG_CONTRAST:
      break;
  }
  return u8g_dev_pb8v1_base_fn(u8g, dev, msg, arg);
}

/* u8g_com_arduino_sw_spi_fn does not work, too fast??? */
U8G_PB_DEV(u8g_dev_sbn1661_122x32 , WIDTH, HEIGHT, PAGE_HEIGHT, u8g_dev_sbn1661_122x32_fn, u8g_com_arduino_no_en_parallel_fn);
