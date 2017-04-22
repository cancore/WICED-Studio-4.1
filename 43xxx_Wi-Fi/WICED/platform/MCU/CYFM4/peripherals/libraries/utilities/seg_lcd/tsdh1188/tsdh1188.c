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
/** \file tsdh1188.c
 **
 ** Tsdh1188 lcd display  functions
 **
 ** History:
 **   - 2015-01-29  1.0  DHo   First version for FM uinversal PDL.
 **
 ******************************************************************************/
#include "gpio/gpio.h"
#include "lcd/lcd.h"
#include "tsdh1188.h"
#include "tsdh1188_config.h"

#if (PDL_UTILITY_ENABLE_SEG_LCD_TSDH1188 == PDL_ON)
/*****************************************************************************/
/* Local pre-processor symbols/macros ('#define')                            */
/*****************************************************************************/
#define LcdBlackLightPinInit()  Gpio1pin_InitOut( GPIO1PIN_P35, Gpio1pin_InitVal( 0u ) );     // Init Output P35, Value: 0
#define LcdBlackLightOff()      Gpio1pin_Put( GPIO1PIN_P35, 0u)  // Output P35, Value: 0
#define LcdBlackLightOn()       Gpio1pin_Put( GPIO1PIN_P35, 1u)  // Output P35, Value: 1

#define TSDH1188_DISP_WORD_MAX_INDEX   sizeof(g_auTsdh1188WordMap)/sizeof(g_auTsdh1188WordMap[0])
#define TSDH1188_DISP_VAL_MAX_INDEX    sizeof(g_auTsdh1188NumDispVal)/sizeof(g_auTsdh1188NumDispVal[0])

/*****************************************************************************/
/* Global variable definitions (declared in header file with 'extern')       */
/*****************************************************************************/
uint16_t g_u16Tsdh1188WordMapSize = TSDH1188_DISP_WORD_MAX_INDEX;

/*****************************************************************************/
/* Local type definitions ('typedef')                                        */
/*****************************************************************************/

/*****************************************************************************/
/* Local function prototypes ('static')                                      */
/*****************************************************************************/

/*****************************************************************************/
/* Local variable definitions ('static')                                     */
/*****************************************************************************/

/**
 ******************************************************************************
 ** \brief LCD initialization function
 **
 ** \param  None
 **
 ** \retval  None
 **
 ******************************************************************************
 */
void Tsdh1188_Init(void)
{
    en_result_t enResult = Ok;    
    stc_lcd_config_t stcLcdConfig;

    PDL_ZERO_STRUCT(stcLcdConfig);

    /* LCD clock initialization */
    stcLcdConfig.stcClkConf.enSrcClk = LCD_CLOCK_MODE;
    stcLcdConfig.stcClkConf.u32DivVal = LCD_CLOCK_PRES;
    /* Power driver setting */
    stcLcdConfig.enDivMode = LCD_DRIVE_MODE;
    if(LCD_DRIVE_MODE == LcdcIntRes)
    {        
        stcLcdConfig.enDivResVal = LCD_INTERNAL_RES_VAL;
    }

    /* Configure VVx, COMx, SEGx Pin */
    stcLcdConfig.stcVvSel.u8Vv = (LCD_VV0 << 0) | (LCD_VV1 << 1) | 
                                 (LCD_VV2 << 2) | (LCD_VV3 << 3) | 
                                 (LCD_VV4 << 4);
    
    stcLcdConfig.stcComSel.u8ComSel =  (LCD_COM0 << 0) | (LCD_COM1 << 1) | 
                                        (LCD_COM2 << 2) | (LCD_COM3 << 3) | 
                                        (LCD_COM4 << 4) | (LCD_COM5 << 5) | 
                                        (LCD_COM6 << 6) | (LCD_COM7 << 7) ;
    
    stcLcdConfig.stcSegSel1.u32SegSel1 = (LCD_SEG0 << 0) | (LCD_SEG1 << 1)  | 
                        (LCD_SEG2 << 2)   | (LCD_SEG3 << 3)   | (LCD_SEG4 << 4)   | 
                        (LCD_SEG5 << 5)   | (LCD_SEG6 << 6)   | (LCD_SEG7 << 7)   | 
                        (LCD_SEG8 << 8)   | (LCD_SEG9 << 9)   | (LCD_SEG10 << 10) | 
                        (LCD_SEG11 << 11) | (LCD_SEG12 << 12) | (LCD_SEG13 << 13) | 
                        (LCD_SEG14 << 14) | (LCD_SEG15 << 15) | (LCD_SEG16 << 16) | 
                        (LCD_SEG17 << 17) | (LCD_SEG18 << 18) | (LCD_SEG19 << 19) |
                        (LCD_SEG20 << 20) | (LCD_SEG21 << 21) | (LCD_SEG22 << 22) | 
                        (LCD_SEG23 << 23) | (LCD_SEG24 << 24) | (LCD_SEG25 << 25) | 
                        (LCD_SEG26 << 26) | (LCD_SEG27 << 27) | (LCD_SEG28 << 28) | 
                        (LCD_SEG29 << 29) | (LCD_SEG30 << 30) | (LCD_SEG31 << 31) ;
    
    stcLcdConfig.stcSegSel2.u32SegSel2 = (LCD_SEG32 << 0) | (LCD_SEG33 << 1) | 
                                         (LCD_SEG34 << 2) | (LCD_SEG35 << 3) |
                                         (LCD_SEG36 << 4) | (LCD_SEG37 << 5) | 
                                         (LCD_SEG38 << 6) | (LCD_SEG39 << 7);               

    /* LCD input IO cut-off setting */
    stcLcdConfig.enInputIoMode = LCD_INPUT_IO_MODE;

    /* Disable blank display */
    stcLcdConfig.bEnBlankDisp = FALSE;

    /* LCD mode setting */
    stcLcdConfig.enDispMode = LCD_DISP_MODE;
    stcLcdConfig.en8ComBias = LCD_8COM_BIAS;

    /* Clear RAM */
    LcdBlackLightPinInit();
    Lcd_ClrWholeRam();
    enResult = Lcd_Init(&stcLcdConfig);
    if (Ok != enResult)
    {
        while(1);
    }

    return;   
}

/**
 ******************************************************************************
 ** \brief LCD uninitialization function
 **
 ** \param  None
 **
 ** \retval None
 **
 ******************************************************************************
 */
void Tsdh1188_DeInit(void)
{
    /* Clear RAM */
    Lcd_ClrWholeRam();
    /* LCD mode setting */
    Lcd_SetDispMode(LcdStop);

    return;
}
    
/**
 ******************************************************************************
 ** \brief Display words on the LCD
 **
 ** \param  WordIndex: Refer to tsdh1188_config.h file
 **
 ** \retval None
 **
 ******************************************************************************
 */
en_result_t Tsdh1188_DispLcdWord(uint8_t u8WordIndex)
{
    if(u8WordIndex >= TSDH1188_DISP_WORD_MAX_INDEX)
        return ErrorInvalidParameter;
    
    Lcd_WriteRAMBit(g_auTsdh1188WordMap[u8WordIndex][0], g_auTsdh1188WordMap[u8WordIndex][1], 1);

    return Ok;
}

/**
 ******************************************************************************
 ** \brief Clear words on the LCD
 **
 ** \param  u8WordIndex: Refer to tsdh1188_config.h file
 **
 ** \retval None
 **
 ******************************************************************************
 */
en_result_t Tsdh1188_ClrLcdWord(uint8_t u8WordIndex)
{
    if(u8WordIndex >= TSDH1188_DISP_WORD_MAX_INDEX)
        return ErrorInvalidParameter;
    
    Lcd_WriteRAMBit(g_auTsdh1188WordMap[u8WordIndex][0], g_auTsdh1188WordMap[u8WordIndex][1], 0);

    return Ok;
}

/**
 ******************************************************************************
 ** \brief Display common number (8 numbers) on the LCD 
 **
 ** \param  u8NumIndex: 0~7
 ** \param  u8DispVal: 0~9
 **
 ** \retval  Ok                                      Lcd Common Num is set normally.
 ** \retval  ErrorInvalidParameter          If one of following condition matches:
 **                                                       - NumIndex >= TSDH1188_COMMON_NUM_ARRAY_SIZE
 **                                                       - DispVal >= TSDH1188_DISP_VAL_MAX_INDEX
 **
 ******************************************************************************
 */
en_result_t Tsdh1188_DispLcdCommonNum(uint8_t u8NumIndex, uint8_t u8DispVal)
{
    uint8_t* pnummap = NULL;
    uint8_t val = 0;
    uint8_t cnt = 0;

    if(u8NumIndex >= TSDH1188_COMMON_NUM_ARRAY_SIZE)
        return ErrorInvalidParameter;

    if(u8DispVal >= TSDH1188_DISP_VAL_MAX_INDEX)
        return ErrorInvalidParameter;
    
    pnummap = (uint8_t*)g_Tsdh1188CommonNumArray[u8NumIndex];
    val = g_auTsdh1188NumDispVal[u8DispVal];

    for(cnt = 0 ; cnt < 7; cnt++)
    {
        if((val & (1ul << cnt)) == (1ul << cnt))
        {
            Lcd_WriteRAMBit(*pnummap, *(pnummap + 1), 1);
        }
        pnummap += 2;
    }

    return Ok;
}

/**
 ******************************************************************************
 ** \brief Clear common number (8 numbers) on the LCD 
 **
 ** \param  u8NumIndex: 0~7
 **
 ** \retval  Ok                                      Lcd common number is set normally.
 ** \retval  ErrorInvalidParameter          If one of following condition matches:
 **                                                       - NumIndex >= TSDH1188_COMMON_NUM_ARRAY_SIZE
 **
 ******************************************************************************
 */
en_result_t Tsdh1188_ClrLcdCommonNum(uint8_t u8NumIndex)
{
    uint8_t* pnummap = NULL;
    uint8_t cnt = 0;

    if(u8NumIndex >= TSDH1188_COMMON_NUM_ARRAY_SIZE)
        return ErrorInvalidParameter;
    
    pnummap = (uint8_t*)g_Tsdh1188CommonNumArray[u8NumIndex];
    
    for(cnt = 0; cnt < 7; cnt++)
    {
        Lcd_WriteRAMBit(*pnummap, *(pnummap + 1), 0);
        pnummap += 2;
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Display month number on the LCD 
 **
 ** \param  u8MonthNum: 1~12
 **
 ** \retval  Ok                                      Lcd month number is set normally.
 ** \retval  ErrorInvalidParameter          If one of following condition matches:
 **                                                       - MonthNum > 12
 **
 ******************************************************************************
 */
en_result_t Tsdh1188_DispLcdMonthNum(uint8_t u8MonthNum)
{
    uint8_t* pnummap = NULL;
    uint8_t val = 0;
    uint8_t cnt = 0;

    if(u8MonthNum > 12)
        return ErrorInvalidParameter;
    
    pnummap = (uint8_t*)g_auTsdh1188MonthNumMap;
    val = g_auTsdh1188MonthNumDispVal[u8MonthNum];
    for(cnt = 0; cnt < 7; cnt++)
    {
        if((val & (1ul << cnt)) == (1ul << cnt))
        {
            Lcd_WriteRAMBit(*pnummap, *(pnummap + 1), 1);
        }
          
        pnummap += 2;
    }
    
    if((val & 0x80) == 0x80)
    {
        Lcd_WriteRAMBit(*pnummap, *(pnummap + 1), 1);
    }

    return Ok;
}

/**
 ******************************************************************************
 ** \brief Clear month number on the LCD 
 ** \param  u8MonthNum: 1~12
 **
 ** \retval  Ok                                      Lcd month number is clear normally.
 ** \retval  ErrorInvalidParameter          If one of following condition matches:
 **                                                       - MonthNum > 12
 **
 ******************************************************************************
 */
en_result_t Tsdh1188_ClrLcdMonthNum(uint8_t u8MonthNum)
{
    uint8_t* pnummap = NULL;
    uint8_t cnt = 0;
    
    if(u8MonthNum > 12)
        return ErrorInvalidParameter;

    pnummap = (uint8_t*)g_auTsdh1188MonthNumMap;
    
    for(cnt = 0; cnt < 7; cnt++)
    {
        Lcd_WriteRAMBit(*pnummap, *(pnummap + 1), 0);
        pnummap += 2;
    }
    
    Lcd_WriteRAMBit(*pnummap, *(pnummap + 1), 0);

    return Ok;
}

/**
 ******************************************************************************
 ** \brief Enable LCD back light 
 **
 ** \param  NONE
 **
 ** \retval  None
 **
 ******************************************************************************
 */
void Tsdh1188_EnableLcdBackLight(void)
{
    LcdBlackLightOn();

    return;
}
/**
 ******************************************************************************
 ** \brief Disable LCD back light 
 **
 ** \param  NONE
 **
 ** \retval  None
 **
 ******************************************************************************
 */
void Tsdh1188_DisableLcdBackLight(void)
{
    LcdBlackLightOff();

    return;
}

/**
 ******************************************************************************
 ** \brief Clear LCD screen
 **
 ** \param  NONE
 **
 ** \retval  None
 **
 ******************************************************************************
 */
void Tsdh1188_ClrScreen(void)
{
    /* Clear RAM */
    Lcd_ClrWholeRam();

    return;
}

#endif

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
