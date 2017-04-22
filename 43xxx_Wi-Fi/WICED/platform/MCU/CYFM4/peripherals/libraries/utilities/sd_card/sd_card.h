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
/** \file sd_cmd.h
 **
 ** Headerfile for SD functions
 **
 ** History:
 **   - 2013-05-16  1.0  QXu  First version.
 **   - 2013-07-24  1.1  RQi  Modification.
 **   - 2015-01-20  1.3  RZh  Modify for FM universal PDL
 **
 ******************************************************************************/

#ifndef __SD_CARD_H__
#define __SD_CARD_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "mcu.h"
#include "pdl_user.h"
#include "sd_card/sd_cmd.h"
#include "gpio/gpio.h"
#include "clk/clk.h"
#include "sdif/sdif.h"

#if (defined(PDL_PERIPHERAL_SD_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

//@{

/**
 ******************************************************************************
 ** \brief SD/eMMC/SDIO card type
 ******************************************************************************/
typedef enum en_sdcmd_cardtype
{
    SdCardTypeNone      = 0u,
    SdCardTypeSD        = 1u,
    SdCardTypeSDIO      = 2u,
    SdCardTypeEMMC      = 3u,
}en_sdcmd_cardtype_t;  
  
/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/
en_result_t Sdcard_Init( stc_sdcard_config_t *pstcCfg );

en_result_t SdCardInitHost( stc_sdcard_config_t *pstcCfg );

en_result_t Sdcard_Read_SingleBlock( stc_sdcard_config_t *pstcCfg,
                                     uint32_t u32BlockIdx,
                                     uint8_t *pu8BuffOut );

en_result_t Sdcard_Read_MultiBlock( stc_sdcard_config_t *pstcCfg,
                                    uint32_t u32BlockIdx,
                                    uint32_t u32Count,
                                    uint8_t *pu8BuffOut );

en_result_t Sdcard_Write_SingleBlock( stc_sdcard_config_t *pstcCfg,
                                      uint32_t u32BlockIdx,
                                      uint8_t *pu8BuffIn );

en_result_t Sdcard_Write_MultiBlock( stc_sdcard_config_t *pstcCfg,
                                     uint32_t u32BlockIdx,
                                     uint32_t u32Count,
                                     uint8_t *pu8BuffIn );

en_sdcmd_cardstatus_t Sdcard_GetStatus( stc_sdcard_config_t *pstcCfg);

en_sdcmd_cardstatus_t Sdcard_GetCardInfo( stc_sdcmd_info_t *pstcSdcardInfo);

//@} // SDGroup
#ifdef __cplusplus
}
#endif

#endif

#endif
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
