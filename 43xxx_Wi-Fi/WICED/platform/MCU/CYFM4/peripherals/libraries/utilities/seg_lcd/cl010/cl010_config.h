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

#ifndef __CL010_CONFIG_H_
#define __CL010_CONFIG_H_

#if (PDL_UTILITY_ENABLE_SEG_LCD_CL0107031 == PDL_ON)

#define PIN_OFF             (0u)
#define PIN_ON              (1u)

/*-   Hardware pins configuration    -*/
/* SEG&COM number */
#define LCD_SEG_NUM           (16u)
#define LCD_COM_NUM           (8u)
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
#define LCD_SEG11             PIN_ON
#define LCD_SEG12             PIN_ON
#define LCD_SEG13             PIN_ON
#define LCD_SEG14             PIN_ON
#define LCD_SEG15             PIN_ON
#define LCD_SEG16             PIN_OFF
#define LCD_SEG17             PIN_OFF
#define LCD_SEG18             PIN_OFF
#define LCD_SEG19             PIN_OFF
#define LCD_SEG20             PIN_OFF
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
#define LCD_VV0               PIN_ON
#define LCD_VV1               PIN_ON
#define LCD_VV2               PIN_ON
#define LCD_VV3               PIN_ON
#define LCD_VV4               PIN_ON

/*-      Software     -*/
/* LCD control */
#define LCD_DISP_MODE            Lcd_8Com_1Div8Duty
#define LCD_8COM_BIAS            Lcd1Div3Bias
#define LCD_DRIVE_MODE           LcdcIntRes
#define LCD_INTERNAL_RES_VAL     LcdDivRes100K
#define LCD_INPUT_IO_MODE        LcdInputIoCutoff
/* LCD BOOSTER */
#define LCD_BOOSTER_ENABLE      FALSE
#define LCD_BOOSTER_BTRC        (0u)
#define LCD_BOOSTER_BTRF        (0x0Cu)
#define LCD_BOOSTER_CENSEL      (1u)
#define LCD_BOOSTER_BSTPD       (0u)
#define LCD_BOOSTER_BSTOPT      (1u)

/* LCD clock */
#define LCD_CLOCK_MODE           LcdClkSelPclk
#define LCD_CLOCK_PRES           (0xFFFFu)


#define CL010_COMMON_NUM_ARRAY_SIZE      (10u)
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
const uint8_t g_auCl010WordMap[][2] =            
{
/* Line 1 */  
    {0, 0},   /* ? */
    {10,6},   /* k1 */
    {10,5},   /* W */
    {10,4},   /* h */
    {10,7},   /* K */
    {11,7},   /* V */
    {11,6},   /* A */
    {11,5},   /* r */
    {11,4},   /* h1 */
    {9, 4},   /* S15_Symbol */
    {9, 3},   /* S16_Symbol */
    {8, 4},   /* S13_Symbol */
    {8, 5},   /* S14_Symbol: Lock */
    {9, 5},   /* R1 */
    {9, 6},   /* R2 */
    {6, 3},   /* COL */
    {12,3},   /* COL1 */
/* Line 2 */   
    {10,3},   /* P1 */
    {12,7},   /* P2 */
    {14,3},   /* P3 */
    {15,7},   /* P4 */
    {7, 4},   /* V  */
    {7, 5},   /* ? */
    {8, 6},   /* S12_Symbol */
    {9, 7},   /* R3 */
    {8, 7},   /* R4 */
/* Line 3 */   
    {0, 4},   /* Rate */
    {2, 4},   /* S2_Symbol */
    {3, 3},   /* S3_Symbol */
    {4, 3},   /* v */
    {4, 4},   /* ?*/
    {4, 5},   /* S6_Symbol */
    {4, 6},   /* S7_Symbol */
    {4, 7},   /* L3_Symbol */
    {5, 4},   /* L1_Symbol */
    {5, 5},   /* L4_Symbol */
    {6, 4},   /* L2_Symbol */
    {6, 5},   /* f */
    {6, 6},   /* ? */
    {7, 6},   /* S11_Symbol */
    {7, 7},   /* R5 */
    {5, 6},    /* S8_Symbol */
    {5, 7},    /* S9_Symbol */
    {6, 7},   /* S10_Symbol */
};

/* Number */
// Number 1
const uint8_t auCl010CommonNum1Map[][2] =            
{
    {2,0},    /* A */
    {3,0},    /* B */
    {3,2},    /* C */
    {1,3},    /* D */
    {0,3},    /* E */
    {0,1},    /* F */
    {0,2},    /* G */
    {3,1},    /* H */
    {1,0},    /* I */
    {1,1},    /* J */
    {2,1},    /* K */
    {1,2},    /* L */
    {2,3},    /* M */
    {2,2},    /* N */
};
// Number 2
const uint8_t auCl010CommonNum2Map[][2] =            
{
    {5,0},    /* A */
    {5,1},    /* B */
    {5,2},    /* C */
    {5,3},    /* D */
    {4,2},    /* E */
    {4,0},    /* F */
    {4,1},    /* G */
    //{6,3},    /* COL */
};

// Number 3
const uint8_t auCl010CommonNum3Map[][2] =            
{
    {8,0},    /* A */
    {9,0},    /* B */
    {9,2},    /* C */
    {7,3},    /* D */
    {6,2},    /* E */
    {6,0},    /* F */
    {6,1},    /* G */
    {9,1},    /* H */
    {7,0},    /* I */
    {7,1},    /* J */
    {8,1},    /* K */
    {7,2},    /* L */
    {8,3},    /* M */
    {8,2},    /* N */
    //{10,3},   /* P1 */
};

// Number 4
const uint8_t auCl010CommonNum4Map[][2] =            
{
    {11,0},    /* A */
    {11,1},    /* B */
    {11,2},    /* C */
    {11,3},    /* D */
    {10,2},    /* E */
    {10,0},    /* F */
    {10,1},    /* G */
    //{12,3},    /* COL1 */
    //{12,7},    /* P2 */
};
// Number 5
const uint8_t auCl010CommonNum5Map[][2] =            
{
    {13,0},    /* A */
    {13,1},    /* B */
    {13,2},    /* C */
    {13,3},    /* D */
    {12,2},    /* E */
    {12,0},    /* F */
    {12,1},    /* G */    
    //{14,3},    /* P3 */
};
// Number 6
const uint8_t auCl010CommonNum6Map[][2] =            
{
    {15,0},    /* A */
    {15,1},    /* B */
    {15,2},    /* C */
    {15,3},    /* D */
    {14,2},    /* E */
    {14,0},    /* F */
    {14,1},    /* G */    
    {15,7},    /* P4 */
};
// Number 7
const uint8_t auCl010CommonNum7Map[][2] =            
{
    {14,4},    /* A */
    {14,5},    /* B */
    {14,6},    /* C */
    {14,7},    /* D */
    {15,6},    /* E */
    {15,4},    /* F */
    {15,5},    /* G */    
};

// Number 8
const uint8_t auCl010CommonNum8Map[][2] =            
{
    {12,4},      /* A */
    {12,5},      /* B */
    {12,6},      /* C */
    {13,7},      /* D */
    {13,6},      /* E */
    {13,4},      /* F */
    {13,5},      /* G */    
};

// Number 9
const uint8_t auCl010CommonNum9Map[][2] =            
{
    {1,4},      /* A */
    {1,5},      /* B */
    {1,6},      /* C */
    {1,7},      /* D */
    {0,7},      /* E */
    {0,5},      /* F */
    {0,6},      /* G */    
};


// Number 10
const uint8_t auCl010CommonNum10Map[][2] =            
{
    {3,4},      /* A */
    {3,5},      /* B */
    {3,6},      /* C */
    {3,7},      /* D */
    {2,7},      /* E */
    {2,5},      /* F */
    {2,6},      /* G */   
};

const uint8_t *g_Cl010CommonNumArray[CL010_COMMON_NUM_ARRAY_SIZE] = 
{
    &auCl010CommonNum1Map[0][0],
    &auCl010CommonNum2Map[0][0],
    &auCl010CommonNum3Map[0][0],
    &auCl010CommonNum4Map[0][0],
    &auCl010CommonNum5Map[0][0],
    &auCl010CommonNum6Map[0][0],
    &auCl010CommonNum7Map[0][0],
    &auCl010CommonNum8Map[0][0],    
    &auCl010CommonNum9Map[0][0],
    &auCl010CommonNum10Map[0][0],
};

/*****************
        A
     _______
    |\  |  /|
   F| \I|J/K| B
    |_G\|/_H|
    |  /|\  |
   E| /L|N\M| C
    |/__|__\|
        D

******************/

// Bit assignment in a setting number

/*********************************************************
Bit 15  14  13  12  11  10  9  8  7  6  5  4  3  2  1  0
    -   -   N   M   L   K   J  I  H  G  F  E  D  C  B  A
*********************************************************/

const uint16_t g_auCl010NumDispVal1[] = 
{
    0x003F,   /* 0 */
    0x0006,   /* 1 */
    0x00DB,   /* 2 */
    0x00CF,   /* 3 */
    0x00E6,   /* 4 */
    0x00ED,   /* 5 */
    0x00FD,   /* 6 */
    0x0007,   /* 7 */
    0x00FF,   /* 8 */
    0x00EF,   /* 9 */
    0x22C0,   /* ?*/
    0x1D00,   /* X */
};

/*******************************************
Bit 7  6  5  4  3  2  1  0
    -  G  F  E  D  C  B  A
*******************************************/

const uint8_t g_auCl010NumDispVal2[] = 
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

#endif /* PDL_UTILITY_ENABLE_SEG_LCD_CL0107031 == PDL_ON */

#endif /* __CL010_CONFIG_H_ */

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
