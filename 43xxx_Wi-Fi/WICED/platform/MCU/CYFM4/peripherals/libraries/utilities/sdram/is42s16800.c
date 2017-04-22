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
/** \file is42s16800.c
 **
 ** IS42S16800 driver API functions
 **
 ** History:
 ** - 2015-01-28  1.0   EZh  First version for FM universal PDL
 **
 *****************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "is42s16800.h"

#if (PDL_UTILITY_ENABLE_EXTIF_IS42S16800 == PDL_ON)   
   
/**
 ******************************************************************************
 ** \addtogroup S26kl512sGroup
 ******************************************************************************/
//@{   

/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/

/******************************************************************************/
/* Local function prototypes                                                  */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Initialize SV6P1615
 **
 ** This function maps SV6P1615 to a certain external area
 **
 ** \param [in]  u8CsArea           Chip selection area selected
 ** \param [in]  u32BaseAddress     Base address of SV6P1615
 ** \param [in]  enSize             PSRAM size   
 **
 ** \retval Ok                      SV6P1615 initialized successfully
 ** \retval Error                   SV6P1615 initialization error
 ** \retval ErrorInvalidParameter   Invalid configuration
 ******************************************************************************/
en_result_t Is42s16800_Init(boolean_t bUseGdcSdramIf,
                            uint8_t u8ColumnAddrNum,
                            uint8_t u8RowAddeNum,
                            en_sdram_width_t enWidth)
{
    stc_extif_area_config_t stcMemConfig;
    stc_extif_sdram_config_t stcSdramConfig;
    uint8_t u8Offset;
    
    // Clear structure
    PDL_ZERO_STRUCT(stcMemConfig);
    PDL_ZERO_STRUCT(stcSdramConfig);
    
    // Enable SDRAM
    stcSdramConfig.bSdramEnable = TRUE; 
    
    // Set data width/column address
    switch (enWidth)
    {
        case SdramWidth16Bit:
            u8Offset = 1u;
            stcSdramConfig.enCasel = ExtifCas16Bit;
            break;
        case SdramWidth32Bit:
            u8Offset = 2u;
            stcSdramConfig.enCasel = ExtifCas32Bit;
            break;
        default:
            return ErrorInvalidParameter;
    }
    
    // Set raw address
    switch ((u8ColumnAddrNum + u8Offset))
    {
        case 6u:
            stcSdramConfig.enRasel = ExtifRas_19_6;
            break;
        case 7u:
            stcSdramConfig.enRasel = ExtifRas_20_7;
            break;
        case 8u:
            stcSdramConfig.enRasel = ExtifRas_21_8;
            break;
        case 9u:
            stcSdramConfig.enRasel = ExtifRas_22_9;
            break;
        case 10u:
            stcSdramConfig.enRasel = ExtifRas_23_10;
            break;
        case 11u:
            stcSdramConfig.enRasel = ExtifRas_24_11;
            break;
        case 12u:
            stcSdramConfig.enRasel = ExtifRas_25_12;
            break;
        default:
            return ErrorInvalidParameter;
    }
    
    // Set bank address
    switch ((u8ColumnAddrNum + u8RowAddeNum + u8Offset))
    {
        case 19u:
            stcSdramConfig.enBasel = ExtifBas_20_19;
            break;
        case 20u:
            stcSdramConfig.enBasel = ExtifBas_21_20;
            break;
        case 21u:
            stcSdramConfig.enBasel = ExtifBas_22_21;
            break;
        case 22u:
            stcSdramConfig.enBasel = ExtifBas_23_22;
            break;
        case 23u:
            stcSdramConfig.enBasel = ExtifBas_24_23;
            break;
        case 24u:
            stcSdramConfig.enBasel = ExtifBas_25_24;
            break;
        case 25u:
            stcSdramConfig.enBasel = ExtifBas_26_25;
            break;
        default:
            return ErrorInvalidParameter;
    }
    
    stcSdramConfig.bSdramPowerDownMode = FALSE;
    stcSdramConfig.bSdramRefreshOff = FALSE;
    stcSdramConfig.u16RefreshCount = 0x619u;
    stcSdramConfig.u8RefreshNumber = 1u;
    stcSdramConfig.bPreRefreshEnable = TRUE;
    stcSdramConfig.enSdramCasLatencyCycle = Sdram3Cycle;
    stcSdramConfig.enSdramRasCycle = Sdram7Cycle;
    stcSdramConfig.enSdramRasPrechargeCycle = Sdram2Cycle;
    stcSdramConfig.enSdramRasCasDelayCycle = Sdram2Cycle;
    stcSdramConfig.enSdramRasActiveCycle = Sdram1Cycle;
    stcSdramConfig.enSdramRefreshCycle = Sdram7Cycle;
    stcSdramConfig.enSdramPrechargeCycle = Sdram3Cycle;
    
    if (FALSE == bUseGdcSdramIf)
    {
        stcMemConfig.pExtifSdramConfig = &stcSdramConfig;
    }
    else
    {
    #if (PDL_MCU_TYPE == PDL_FM4_TYPE4)
        stcMemConfig.pGdcSdramConfig = &stcSdramConfig;
    #else
        return ErrorInvalidParameter;
    #endif
    }
    
    stcMemConfig.u8MclkDivision = 10;
    stcMemConfig.bMclkoutEnable = TRUE;
    stcMemConfig.bPrecedReadContinuousWrite = TRUE;
    
    if (Ok != Extif_InitArea(8u, &stcMemConfig))
    {
        return Error;
    }
    
    return Ok;
}


//@} // Sv6p1615Group  

#endif

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
