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
/** \file tsdh1188_config.h
 **
 ** Headerfile for tsdh1188 lcd configure parameters
 **  
 ** History:
 **   - 2015-01-29  1.0  DHo   First version for FM uinversal PDL.
 **
 ******************************************************************************/

#ifndef __TSDH1188_CONFIG_H_
#define __TSDH1188_CONFIG_H_

#define PIN_OFF             0
#define PIN_ON              1

/*-   Hardware pins configuration    -*/
/* SEG&COM number */
#define LCD_SEG_NUM           17u
#define LCD_COM_NUM           8u
/* Segment */
#define LCD_SEG0              PIN_ON
#define LCD_SEG1              PIN_ON
#define LCD_SEG2              PIN_ON
#define LCD_SEG3              PIN_ON
#define LCD_SEG4              PIN_ON
#define LCD_SEG5              PIN_ON
#define LCD_SEG6              PIN_ON
#define LCD_SEG7              PIN_ON
#define LCD_SEG8              PIN_ON
#define LCD_SEG9              PIN_ON
#define LCD_SEG10             PIN_ON
#define LCD_SEG11             PIN_OFF
#define LCD_SEG12             PIN_OFF
#define LCD_SEG13             PIN_ON
#define LCD_SEG14             PIN_ON
#define LCD_SEG15             PIN_ON
#define LCD_SEG16             PIN_ON
#define LCD_SEG17             PIN_OFF
#define LCD_SEG18             PIN_OFF
#define LCD_SEG19             PIN_ON
#define LCD_SEG20             PIN_ON
#define LCD_SEG21             PIN_OFF
#define LCD_SEG22             PIN_OFF
#define LCD_SEG23             PIN_OFF
#define LCD_SEG24             PIN_OFF
#define LCD_SEG25             PIN_OFF
#define LCD_SEG26             PIN_OFF
#define LCD_SEG27             PIN_OFF
#define LCD_SEG28             PIN_OFF
#define LCD_SEG29             PIN_OFF
#define LCD_SEG30             PIN_OFF
#define LCD_SEG31             PIN_OFF
#define LCD_SEG32             PIN_OFF
#define LCD_SEG33             PIN_OFF
#define LCD_SEG34             PIN_OFF
#define LCD_SEG35             PIN_OFF
#define LCD_SEG36             PIN_OFF
#define LCD_SEG37             PIN_OFF
#define LCD_SEG38             PIN_OFF
#define LCD_SEG39             PIN_OFF

/* COM */
#define LCD_COM0              PIN_ON
#define LCD_COM1              PIN_ON
#define LCD_COM2              PIN_ON
#define LCD_COM3              PIN_ON
#define LCD_COM4              PIN_ON
#define LCD_COM5              PIN_ON
#define LCD_COM6              PIN_ON
#define LCD_COM7              PIN_ON 

/* VV */
#define LCD_VV0               PIN_OFF
#define LCD_VV1               PIN_OFF
#define LCD_VV2               PIN_OFF
#define LCD_VV3               PIN_OFF
#define LCD_VV4               PIN_ON

/*-      Software     -*/
/* LCD control */
#define LCD_DISP_MODE            Lcd_8Com_1Div8Duty
#define LCD_8COM_BIAS            Lcd1Div3Bias
#define LCD_DRIVE_MODE           LcdcIntRes
#define LCD_INTERNAL_RES_VAL     LcdDivRes10K
#define LCD_INPUT_IO_MODE        LcdInputIoConnect
/* LCD clock */
#define LCD_CLOCK_MODE           LcdClkSelPclk
#define LCD_CLOCK_PRES           (0x8000u)

#define TSDH1188_COMMON_NUM_ARRAY_SIZE      (8u)

/*****************************************************************************
      SEG 0 1 2 3 4 5 6 7 8 9 10 19 20 13 14 15 16
COM 0
    1
    2
    3
    4
    5
    6
    7
******************************************************************************/
/* LCD elements definition */
/* Words */
const uint8_t g_auTsdh1188WordMap[][2] =            
{
/* Line 1 */  
    {7, 0},    /* cos */
    {7, 1},    /* wave */
    {8, 0},    /* 当前 */
    {9, 0},    /* 上月 */
    {13, 0},   /* 组合 */
    {14, 0},   /* 总 */
    {14, 1},   /* 尖 */
    {15, 0},   /* 峰 */
    {16, 0},   /* 平 */
    {16, 1},   /* 谷 */ 
    {0, 0},    /* 剩余 */  
    {1, 0},    /* 常数 */  
/* Line 2 */   
    {6, 0},    /* “1”*/
    {6, 1},    /* “2”*/
    {5, 0},    /* “3”*/
    {6, 2},    /* “4”*/
    {7, 2},    /* phone图*/
    {8, 1},    /* 阶梯 */
    {8, 2},    /* 赊 */
    {9, 2},    /* 欠 */
    {20, 2},   /* 用 */
    {20, 1},   /*电 */
    {13, 1},   /*量 */
    {14, 2},   /*价 */
    {15, 2},   /*户 */
    {15, 1},   /* 时 */
    {16, 2},   /* 间 */
    {0, 1},   /* 段 */
    {1, 1},   /* 金额 */
    {1, 2},   /* 表 */
    {2, 1},   /* 号 */
/* Line 3 */   
    {5, 1},   /*“尖”符号 */
    {5, 2},   /*“峰”符号 */
    {5, 3},   /*《1》三角1 */
    {5, 4},   /* L */
    {5, 5},   /* N */
    {5, 6},   /* 一符号 */
    {5, 7},   /* "平"符号 */
    {7, 7},   /* “谷”符号 */
    {6, 7},   /*《2》三角2*/
    {7, 6},   /* P2 */
    {9, 6},   /* P3 */
    {19,6},   /* P4 */
    {13,6},   /* P5 */
    {15,6},   /* P6 */
    {0,6},    /* P7 */
    {2,6},    /* P8 */
    {0,2},    /* P9 */
    {13,2},   /* P10 */
    {2, 2},   /* 元 */
    {4, 2},   /* V */
    {4, 1},   /* A */
    {4, 6},   /* KW */
    {4, 0},   /* H */
/* Line 4 */  
    {8, 7},   /* <= */
    {9, 7},   /* 1）圆1 */
    {10, 7},  /* 2）圆2 */
    {19, 7},  /*电池符号 */
    {20, 7},  /*两轮符号*/
    {13, 7},  /*锁 符号*/
    {14, 7},  /*读卡*/
    {15, 7},  /*中 */
    {16, 7},  /*成功*/
    {0, 7},   /*失败*/
    {1, 7},   /*请购电*/
    {2, 7},   /*拉闸*/
    {3, 7},   /*透支*/
    {4, 7},   /*囤积*/ 
};

/* Number */
// Number 1
const uint8_t auTsdh1188CommonNum1Map[][2] =            
{
    {6,3},    /* A */
    {7,3},    /* B */
    {7,5},    /* C */
    {6,6},    /* D */
    {6,5},    /* E */
    {6,4},    /* F */
    {7,4},    /* G */
};
// Number 2
const uint8_t auTsdh1188CommonNum2Map[][2] =            
{
    {8,3},    /* A */
    {9,3},    /* B */
    {9,5},    /* C */
    {8,6},    /* D */
    {8,5},    /* E */
    {8,4},    /* F */
    {9,4},    /* G */
};

// Number 3
const uint8_t auTsdh1188CommonNum3Map[][2] =            
{
    {10,3},    /* A */
    {19,3},    /* B */
    {19,5},    /* C */
    {10,6},    /* D */
    {10,5},    /* E */
    {10,4},    /* F */
    {19,4},    /* G */    
};

// Number 4
const uint8_t auTsdh1188CommonNum4Map[][2] =            
{
    {20,3},    /* A */
    {13,3},    /* B */
    {13,5},    /* C */
    {20,6},    /* D */
    {20,5},    /* E */
    {20,4},    /* F */
    {13,4},    /* G */    
};
// Number 5
const uint8_t auTsdh1188CommonNum5Map[][2] =            
{
    {14, 3},    /* A */
    {15, 3},    /* B */
    {15, 5},    /* C */
    {14, 6},    /* D */
    {14, 5},    /* E */
    {14, 4},    /* F */
    {15, 4},    /* G */    
};

// Number 6
const uint8_t auTsdh1188CommonNum6Map[][2] =            
{
    {16,3},     /* A */
    {0,3},      /* B */
    {0,5},      /* C */
    {16,6},     /* D */
    {16,5},     /* E */
    {16,4},     /* F */
    {0,4},      /* G */    
};

// Number 7
const uint8_t auTsdh1188CommonNum7Map[][2] =            
{
    {1, 3},      /* A */
    {2, 3},      /* B */
    {2, 5},      /* C */
    {1, 6},      /* D */
    {1, 5},      /* E */
    {1, 4},      /* F */
    {2, 4},      /* G */    
};

// Number 8
const uint8_t auTsdh1188CommonNum8Map[][2] =            
{
    {3, 3},    /* A */
    {4, 3},    /* B */
    {4, 5},    /* C */
    {3, 6},    /* D */
    {3, 5},    /* E */
    {3, 4},    /* F */
    {4, 4},    /* G */    
};

const uint8_t *g_Tsdh1188CommonNumArray[8] = 
{
    &auTsdh1188CommonNum1Map[0][0],
    &auTsdh1188CommonNum2Map[0][0],
    &auTsdh1188CommonNum3Map[0][0],
    &auTsdh1188CommonNum4Map[0][0],
    &auTsdh1188CommonNum5Map[0][0],
    &auTsdh1188CommonNum6Map[0][0],
    &auTsdh1188CommonNum7Map[0][0],
    &auTsdh1188CommonNum8Map[0][0],    
};

// Element assignment in a nixietube 
/*****************
        A
     ------
    |      |
   F|   G  | B
    |______|
    |      |
   E|      | C
    |______|
        D
******************/

// Bit assignment in a setting number

/*******************************************
Bit 7  6  5  4  3  2  1  0
    -  G  F  E  D  C  B  A
*******************************************/

const uint8_t g_auTsdh1188NumDispVal[] = 
{
    0x3F,   /* 0 */
    0x06,   /* 1 */
    0x5B,   /* 2 */
    0x4F,   /* 3 */
    0x66,   /* 4 */
    0x6D,   /* 5 */
    0x7D,   /* 6 */
    0x07,   /* 7 */
    0x7F,   /* 8 */
    0x6F,   /* 9 */
};

// Bit assignment in a setting number

/*******************************************
Bit 7   6  5  4  3  2  1  0
    P1  G  F  E  D  C  B  A
*******************************************/

// Month Number
const uint8_t g_auTsdh1188MonthNumMap[][2] =            
{
    {19,0},      /* A */
    {20,0},      /* B */
    {19,2},      /* C */
    {10,2},      /* D */
    {10,1},      /* E */
    {10,0},      /* F */
    {19,1},      /* G */    
    {9,1},       /* P1 */
};

const uint8_t g_auTsdh1188MonthNumDispVal[] = 
{
    0x3F,   /* 0 */
    0x06,   /* 1 */
    0x5B,   /* 2 */
    0x4F,   /* 3 */
    0x66,   /* 4 */
    0x6D,   /* 5 */
    0x7D,   /* 6 */
    0x07,   /* 7 */
    0x7F,   /* 8 */
    0x6F,   /* 9 */
    (0x3F|0x80),   /* 10 */
    (0x06|0x80),    /* 11 */
    (0x5B|0x80),    /* 12 */
};


#endif /* __TSDH1188_CONFIG_H_ */

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
