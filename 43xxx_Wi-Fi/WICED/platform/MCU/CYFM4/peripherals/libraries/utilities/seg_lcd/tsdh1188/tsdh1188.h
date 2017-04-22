/*************************************************************************************
* Copyright (C) 2013-2015, Cypress Semiconductor Corporation. All rights reserved.    
*                                                                                     
* This software, including source code, documentation and related                     
* materials ( "Software" ), is owned by Cypress Semiconductor                         
* Corporation ( "Cypress" ) and is protected by and subject to worldwide              
* patent protection (United States and foreign), United States copyright              
* laws and international treaty provisions. Therefore, you may use this               
* Software only as provided in the license agreement accompanying the                 
* software package from which you obtained this Software ( "EULA" ).                  
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,             
* non-transferable license to copy, modify, and compile the                           
* Software source code solely for use in connection with Cypress's                    
* integrated circuit products. Any reproduction, modification, translation,           
* compilation, or representation of this Software except as specified                 
* above is prohibited without the express written permission of Cypress.              
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO                                
* WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING,                                
* BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED                                        
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A                                     
* PARTICULAR PURPOSE. Cypress reserves the right to make                              
* changes to the Software without notice. Cypress does not assume any                 
* liability arising out of the application or use of the Software or any              
* product or circuit described in the Software. Cypress does not                      
* authorize its products for use in any products where a malfunction or               
* failure of the Cypress product may reasonably be expected to result in              
* significant property damage, injury or death ( "High Risk Product" ). By            
* including Cypress's product in a High Risk Product, the manufacturer                
* of such system or application assumes all risk of such use and in doing             
* so agrees to indemnify Cypress against all liability.                               
*/
/******************************************************************************/
/** \file tsdh1188.h
 **
 ** Headerfile for  tsdh1188 lcd display  functions
 **  
 ** History:
 **   - 2015-01-29  1.0  DHo   First version for FM uinversal PDL.
 **
 ******************************************************************************/

#ifndef __TSDH1188__
#define __TSDH1188__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "mcu.h"
#include "pdl_user.h"

#if (PDL_UTILITY_ENABLE_SEG_LCD_TSDH1188 == PDL_ON)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup LcdGroup Lcd display Function
 **
 ** Provided functions of LcdGroup module:
 **
 ** - Tsdh1188_Init()
 ** - Tsdh1188_DeInit()
 ** - Tsdh1188_DispLcdWord()
 ** - Tsdh1188_ClrLcdWord()
 ** - Tsdh1188_DispLcdCommonNum()
 ** - Tsdh1188_ClrLcdCommonNum()
 ** - Tsdh1188_DispLcdMonthNum()
 ** - Tsdh1188_ClrLcdMonthNum()
 ** - Tsdh1188_EnableLcdBackLight()
 ** - Tsdh1188_DisableLcdBackLight()
 ** - Tsdh1188_ClrScreen()
 **
 ** Tsdh1188_Init() initializes tsdh2288 LCD.
 **
 ** Tsdh1188_DeInit() re-initializes tsdh2288 LCD.
 **
 ** Tsdh1188_DispLcdWord() display a word or symbol on LCD by u8WordIndex.
 **
 ** Tsdh1188_ClrLcdWord() clear a word or symbol on LCD by u8WordIndex.
 **
 ** Tsdh1188_DispLcdCommonNum() display a common number on LCD by u8NumIndex and u8DispVal.
 **
 ** Tsdh1188_ClrLcdCommonNum() clear a common number on LCD by u8NumIndex.
 **
 ** Tsdh1188_DispLcdMonthNum() clear month on LCD by u8MonthNum.
 **
 ** Tsdh1188_ClrLcdMonthNum() clear month on LCD by u8MonthNum.
 **
 ** Tsdh1188_EnableLcdBackLight() enable LCD back light. 
 **
 ** Tsdh1188_DisableLcdBackLight() disable LCD back light. 
 **
 ** Tsdh1188_ClrScreen() clear screen.
 **
 ******************************************************************************/
//@{

/******************************************************************************/
/* Global pre-processor symbols/macros ('#define')                            */
/******************************************************************************/
 
/******************************************************************************
 * Global type definitions
 ******************************************************************************/

/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/
extern uint16_t g_u16Tsdh1188WordMapSize;

/*****************************************************************************/
/* Global function prototypes (definition in C source)                        */
/*****************************************************************************/
void Tsdh1188_Init(void);
void Tsdh1188_DeInit(void);
en_result_t Tsdh1188_DispLcdWord(uint8_t u8WordIndex);
en_result_t Tsdh1188_ClrLcdWord(uint8_t u8WordIndex);
en_result_t Tsdh1188_DispLcdCommonNum(uint8_t u8NumIndex, uint8_t u8DispVal);
en_result_t Tsdh1188_ClrLcdCommonNum(uint8_t u8NumIndex);
en_result_t Tsdh1188_DispLcdMonthNum(uint8_t u8MonthNum);
en_result_t Tsdh1188_ClrLcdMonthNum(uint8_t u8MonthNum);
void Tsdh1188_EnableLcdBackLight(void);
void Tsdh1188_DisableLcdBackLight(void);
void Tsdh1188_ClrScreen(void);

#ifdef __cplusplus
}
#endif

#endif /* #if (PDL_UTILITY_ENABLE_SEG_LCD_TSDH1188 == PDL_ON) */

#endif /* __TSDH1188__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
