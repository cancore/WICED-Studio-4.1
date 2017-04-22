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
/** \file s26kl512s.h
 **
 ** S26KL512S (Hyper Flash)driver header file
 **
 ** History:
 ** - 2015-01-09  1.0   EZh  First version for FM universal PDL
 **
 *****************************************************************************/

#ifndef __S26KL512S_H__
#define __S26KL512S_H__

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include "mcu.h"
#include "pdl_user.h"
#include "gpio/gpio.h"
#include "hbif/hbif.h"


#if (PDL_UTILITY_ENABLE_HBIF_S26KL512S == PDL_ON)   
   
/**
 ******************************************************************************
 ** \defgroup S26kl512sGroup 
 **
 ** Provided S26KL512S module functions:
 **
 ** - S26kl512s_Init()
 ** - S26kl512s_ChipErase()
 ** - S26kl512s_BlankCheck()
 ** - S26kl512s_SectorErase()
 ** - S26kl512s_WordProgram()
 ** - S26kl512s_WriteBufferAndProgram()
 ** - S26kl512s_ReadId()
 ** - S26kl512s_ReadCfi()
 **
 ** S26kl512s_Init() initializes Hyper bus and allcates S26KL512S to 
 ** a certain area.
 **
 ** S26kl512s_ChipErase() erase whole chip of S26KL512S.
 **
 ** S26kl512s_SectorErase() erases one sector of S26KL512S and 
 ** S26kl512s_BlankCheck() checks whether the sector of S26KL512S is erased
 ** or not.
 **
 ** S26kl512s_WordProgram() writes one word to S26KL512S and 
 ** S26kl512s_WriteBufferAndProgram() stores the data into internal buffer of 
 ** S26KL512S first and then program.
 **
 ** S26kl512s_ReadId() reads ID information and S26kl512s_ReadCfi() reads
 ** CFI information.
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

/*****************************************************************************/
/* Global variable declarations ('extern', definition in C source)           */
/*****************************************************************************/

/*****************************************************************************/
/* Global function prototypes ('extern', definition in C source)             */
/*****************************************************************************/
en_result_t S26kl512s_Init(uint8_t u8CsArea, uint32_t u32BaseAddress) ;
en_result_t S26kl512s_ChipErase(void);
en_result_t S26kl512s_BlankCheck(uint16_t* pu16SectorAddr);
en_result_t S26kl512s_SectorErase(uint16_t* pu16SectorAddr);
en_result_t S26kl512s_WordProgram(uint16_t* pu16Addr, uint16_t u16Data);
en_result_t S26kl512s_WriteBufferAndProgram(uint16_t* pu16Addr, 
                                            uint16_t* pu16Data, 
                                            uint16_t u16Datalength);
en_result_t S26kl512s_ReadId(uint16_t* pu16Id, 
                             uint8_t u8WordOffset, 
                             uint8_t u8WordCnt);
en_result_t S26kl512s_ReadCfi(uint16_t* pu16Cfi, 
                              uint8_t u8WordOffset, 
                              uint8_t u8WordCnt);

//@}
#ifdef __cplusplus
}
#endif

#endif

#endif /* __S26KL512S_H__ */
/*****************************************************************************/
/* EOF (not truncated)                                                       */
/*****************************************************************************/
