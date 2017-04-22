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
/** \file s34ml01g.h
 **
 ** S34ML01G (Nand Flash) driver header file
 **
 ** History:
 ** - 2015-01-31  1.0   EZh  First version for FM universal PDL
 **
 *****************************************************************************/
#ifndef __S34ML01G_H__
#define __S34ML01G_H__

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include "mcu.h"
#include "pdl_user.h"
#include "gpio/gpio.h"
#include "extif/extif.h"

#if (PDL_UTILITY_ENABLE_EXTIF_S34ML01G == PDL_ON)   

/**
 ******************************************************************************
 ** \defgroup S34ml01gGroup 
 **
 ** Provided S34ML01G module functions:
 **
 ** - S34ml01g_Init()
 ** - S34ml01g_ReadID()
 ** - S34ml01g_EraseBlock()
 ** - S34ml01g_WritePage()
 ** - S34ml01g_ReadPage()
 **
 ** S34ml01g_Init() initializes Nand Flash interface and allcates S34ML01G to 
 ** a certain area.
 **
 ** S34ml01g_ReadID() reads the product ID information.
 **
 ** S34ml01g_EraseBlock() erases a block of Nand Flash.
 **
 ** S34ml01g_WritePage() writes a page of Nand Flash, the page offset can be 
 ** set with the parameter #u16PageOffset.
 **
 ** S34ml01g_ReadPage() Reads a page of Nand Flash, the page offset can be 
 ** set with the parameter #u16PageOffset.
 **
 ******************************************************************************/
/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
  
//@{  

/*****************************************************************************/
/* Global function prototypes ('extern', definition in C source)             */
/*****************************************************************************/  
en_result_t S34ml01g_Init(uint8_t u8CsArea, uint32_t u32BaseAddress);  
en_result_t S34ml01g_ReadID(uint8_t *pu8aId, uint8_t u8Cnt);
en_result_t S34ml01g_EraseBlock(uint32_t  u32Block);
en_result_t S34ml01g_WritePage(uint32_t u32Block, 
                               uint32_t u32Page, 
                               uint16_t u16PageOffset,
                               uint8_t *pu8Data,
                               uint16_t u16DataSize);
en_result_t S34ml01g_ReadPage(uint32_t u32Block, 
                              uint32_t u32Page, 
                              uint16_t u16PageOffset, 
                              uint8_t *pu8Data,
                              uint16_t u16DataSize);

//@}
#ifdef __cplusplus
}
#endif

#endif

#endif

/*****************************************************************************/
/* EOF (not truncated)                                                       */
/*****************************************************************************/
