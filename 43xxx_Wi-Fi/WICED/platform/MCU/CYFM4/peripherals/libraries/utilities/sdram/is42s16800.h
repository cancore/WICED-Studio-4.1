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
/*****************************************************************************/
/** \file sv6p1615.h
 **
 ** SV6P1615 (PSRAM) driver header file
 **
 ** History:
 ** - 2015-01-31  1.0   EZh  First version for FM universal PDL
 **
 *****************************************************************************/

#ifndef __SV6P1615_H__
#define __SV6P1615_H__

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include "mcu.h"
#include "pdl_user.h"
#include "gpio/gpio.h"
#include "extif/extif.h"


#if (PDL_UTILITY_ENABLE_EXTIF_IS42S16800 == PDL_ON)   
   
/**
 ******************************************************************************
 ** \defgroup Is42s16800Group 
 **
 ** Provided Is42s16800 module functions:
 **
 ** - Is42s16800_Init()
 **
 ** Is42s16800_Init() initializes SDRAM interface and allcates IS42S16800 to 
 ** a certain area.
 **
 ******************************************************************************/
/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
  
//@{  

/*****************************************************************************/
/* Global pre-processor symbols/macros ('define')                            */
/*****************************************************************************/  
 
/******************************************************************************
 * Global definitions
 ******************************************************************************/

/*****************************************************************************/
/* Global type definitions ('typedef')                                       */
/*****************************************************************************/
typedef enum en_sdram_width
{
    SdramWidth16Bit = 0u,
    SdramWidth32Bit = 1u,
    
}en_sdram_width_t;
  

/*****************************************************************************/
/* Global function prototypes ('extern', definition in C source)             */
/*****************************************************************************/
en_result_t Is42s16800_Init(boolean_t bUseGdcSdramIf,
                            uint8_t u8ColumnAddrNum,
                            uint8_t u8RowAddeNum,
                            en_sdram_width_t enWidth);

//@}
#ifdef __cplusplus
}
#endif

#endif

#endif /* __SV6P1615_H__ */
/*****************************************************************************/
/* EOF (not truncated)                                                       */
/*****************************************************************************/
