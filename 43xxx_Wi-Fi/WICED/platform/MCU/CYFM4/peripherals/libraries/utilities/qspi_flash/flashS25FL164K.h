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
/** \file flashS25FL164K.h
 **
 ** Header file of driver module for HS-SPI with external S25FL164K Flash
 **
 ** History:
 ** - 2014-10-24  1.0  MWi  First version
 **
 *****************************************************************************/

#ifndef __FLASHS25FL164K_H__
#define __FLASHS25FL164K_H__

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include "pdl_user.h"
#include "mcu.h"
#include "hsspi/hsspi.h"
#include "gpio/gpio.h"
#include "clk/clk.h"

#if (PDL_UTILITY_ENABLE_QSPI_POLLING_S25FL164K == PDL_ON)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 *****************************************************************************
 ** \defgroup FlashS25FL164KGroup Flash driver
 **
 ** Supporterd functions:
 *****************************************************************************/
//@{

/*****************************************************************************/
/* Global pre-processor symbols/macros ('define')                            */
/*****************************************************************************/

/*****************************************************************************/
/* Global type definitions ('typedef')                                       */
/*****************************************************************************/

/*****************************************************************************/
/* Global variable declarations ('extern', definition in C source)           */
/*****************************************************************************/
  
/*****************************************************************************/
/* Global function prototypes ('extern', definition in C source)             */
/*****************************************************************************/
extern en_result_t FlashS25FL164K_SetQIO(uint8_t u8DeviceNr, volatile stc_hsspin_t* pstcHsSpi, boolean_t bSwitchOn);
extern en_result_t FlashS25FL164K_ReadID(uint8_t u8DeviceNr, volatile stc_hsspin_t* pstcHsSpi,
                                  uint8_t* pu8IdData, uint8_t u8IdLength);
extern en_result_t FlashS25FL164K_Erase(uint8_t u8DeviceNr, volatile stc_hsspin_t* pstcHsSpi,
                                 uint32_t u32Address, boolean_t bBulkErase);
extern en_result_t FlashS25FL164K_Write(uint8_t u8DeviceNr, volatile stc_hsspin_t* pstcHsSpi,
                                 uint8_t* pu8Data, uint32_t u32Address, uint32_t u32NumData);
extern en_result_t FlashS25FL164K_Read(uint8_t u8DeviceNr, volatile stc_hsspin_t* pstcHsSpi,
                                uint8_t* pu8Data, uint32_t u32Address, uint32_t u32NumData);
extern en_result_t FlashS25FL164K_EnableCommandSequencer(uint8_t u8DeviceNr,
                                                  volatile stc_hsspin_t* pstcHsSpi,
                                                  boolean_t bSwitchOn);
extern en_result_t FlashS25FL164K_ReadUniqueId(uint8_t u8DeviceNr,
                                        volatile stc_hsspin_t* pstcHsSpi,
                                        uint8_t* pu8Id);
  
//@} // FlashS25FL164KGroup

#ifdef __cplusplus
}
#endif

#endif

#endif /* __FLASHS25FL164K_H__ */

/*****************************************************************************/
/* EOF (not truncated)                                                       */
/*****************************************************************************/
