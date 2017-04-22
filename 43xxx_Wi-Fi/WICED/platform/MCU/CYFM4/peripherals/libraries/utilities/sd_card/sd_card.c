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
/** \file sdcard.c
 **
 ** History:
 **   - 2013-05-16  1.0  QXu    First version.
 **   - 2013-07-24  1.1  RQian  Modification.
 **   - 2014-01-28  1.2  MWi    Some comment and code beautifying
 **   - 2015-01-20  1.3  RZh    Modify for FM universal PDL
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include <string.h>
#include "sd_cmd.h"
#include "sd_card.h"
#include "sdif/sdif.h"

#if (defined(PDL_PERIPHERAL_SD_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup SdGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/******************************************************************************/
/* Local function prototypes ('static')                                       */
/******************************************************************************/
static en_sdcmd_cardtype_t SdCardTypeDetect( stc_sdcard_config_t *pstcCfg );
static en_result_t SdCardInitSd( stc_sdcard_config_t *pstcCfg );
static en_result_t SdCardIniteMMC( stc_sdcard_config_t *pstcCfg );
static en_result_t SdInitPins( boolean_t bNoCardDetectPin );
/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/

/******************************************************************************/
/* Function implementation - global ('extern') and local ('static')           */
/******************************************************************************/

/******************************************************************************/
/* Local Functions                                                            */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Initialize SDIO pins
 **
 ** \retval Ok   SDIO pins initialized successfully    
 ** 
 ******************************************************************************/

static en_result_t SdInitPins( boolean_t bNoCardDetectPin )
{
#if (defined(SetPinFunc_S_CMD_0))
    SetPinFunc_S_CMD_0();
#else
    SetPinFunc_S_CMD();
#endif

#if (defined(SetPinFunc_S_CLK_0)) 
  SetPinFunc_S_CLK_0();
#else
  SetPinFunc_S_CLK();
#endif

if (bNoCardDetectPin == TRUE) {
/* FIXME Perhaps better to wrap this up into SetPinFunc_S_CD_0 and make
 * it configurable from pdl_user.h.
 */
#define _Gpio1pin_InitOut(a,b)              Gpio1pin_InitOut(a, b)
#define SetPinFunc_S_CD_0_to_GPIO(dummy)   _Gpio1pin_InitOut(GPIO1PIN_S_CD_0, Gpio1pin_InitVal(0u));
    SetPinFunc_S_CD_0_to_GPIO();
} else {
#if (defined(SetPinFunc_S_CD_0)) 
  SetPinFunc_S_CD_0();
#else
  SetPinFunc_S_CD();
#endif
}

#if (defined(SetPinFunc_S_WP_0)) 
  SetPinFunc_S_WP_0();
#else
  SetPinFunc_S_WP();
#endif

#if (defined(SetPinFunc_S_DATA0_0)) 
  SetPinFunc_S_DATA0_0();
#else
  SetPinFunc_S_DATA0();
#endif
    
#if (defined(SetPinFunc_S_DATA1_0)) 
  SetPinFunc_S_DATA1_0();
#else
  SetPinFunc_S_DATA1();
#endif
  
  #if (defined(SetPinFunc_S_DATA2_0)) 
  SetPinFunc_S_DATA2_0();
#else
  SetPinFunc_S_DATA2();
#endif
  
  #if (defined(SetPinFunc_S_DATA3_0)) 
  SetPinFunc_S_DATA3_0();
#else
  SetPinFunc_S_DATA3();
#endif
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Initialize SDIO Host
 **
 ** \param pstcCfg Pointer to SD card configuration
 **
 ** \retval Ok                      SDIO host initialized successfully   
 ** \retval ErrorInvalidParameter   If one of following conditions macths:
 **                                 - NULL == pstcCfg
 **                                 - Other invalid configuration
 ** 
 ******************************************************************************/
en_result_t SdCardInitHost( stc_sdcard_config_t *pstcCfg )
{
    en_result_t              enRet          = Error;
    stc_sdif_config_t        stcConfig      = { 0 };
    stc_sdif_err_irq_cb_t    stcErrIrqCb    = { 0 };
    stc_sdif_err_irq_en_t    stcErrIrqEn    = {{ 0 }};
    stc_sdif_normal_irq_en_t stcNormalIrqEn = { 0 };
    stc_sdif_normal_irq_cb_t stcNormalIrqCb = { 0 };
    stc_sdcmd_config_t       stcCmdCfg;
    
    if (NULL == pstcCfg)
    {
        return ErrorInvalidParameter;
    }

    PDL_ZERO_STRUCT(stcCmdCfg);
        
    // IO init.
    SdInitPins(pstcCfg->bNoCardDetectPin);
    
    // Enable SD clock supply
    Clk_PeripheralClockEnable(ClkGateSd);
    
    while (( FALSE == pstcCfg->bDisableCardDetect ) && ( FALSE == Sdif_PollCardInsert( SDIF )))
    {
        ;
    }

    // Initialize interrupts
    stcErrIrqEn.u16ErrStatus = 0;
    stcErrIrqEn.CmdTimeoutErr = TRUE;
    stcErrIrqCb.pfnCmdTimeoutErrIrqCb = SdcmdEventCmdTimeOutCbk;
    
    stcNormalIrqEn.BufferReadReady = TRUE;
    stcNormalIrqEn.BufferWriteReady = TRUE;
    stcNormalIrqEn.CommandComplete = TRUE;
    stcNormalIrqEn.TransferComplete = TRUE;
    stcNormalIrqEn.CardRemoval = TRUE;
    stcNormalIrqEn.CardIrq = TRUE;

    stcNormalIrqCb.pfnBufferReadReadyIrqCb = SdcmdEventBufferReadRdyCbk;
    stcNormalIrqCb.pfnBufferWriteReadyIrqCb = SdcmdEventBufferWriteRdyCbk;
    stcNormalIrqCb.pfnCommandCompleteIrqCb = SdcmdEventCmdCompleteCbk;
    stcNormalIrqCb.pfnTransferCompleteIrqCb = SdcmdEventTransCompleteCbk;
    stcNormalIrqCb.pfnCardRemovalIrqCb = SdcmdEventCardRemoveCbk;
    stcNormalIrqCb.pfnCardIrqCb = pstcCfg->pfnCardIrqCb;

    // Initialize configuration
    stcConfig.bEnable4BitMode = FALSE;
    stcConfig.bEnableHighSpeed = FALSE;
    stcConfig.bSelAdma = pstcCfg->bUsingADMA;
    stcConfig.pstcErrIrqCb = &stcErrIrqCb;
    stcConfig.pstcErrIrqEn = &stcErrIrqEn;
    stcConfig.pstcNormalIrqCb = &stcNormalIrqCb;
    stcConfig.pstcNormalIrqEn = &stcNormalIrqEn;
    stcConfig.bTouchNvic = pstcCfg->bTouchNvic;
    
    enRet = Sdif_Init(SDIF, &stcConfig);

    Sdif_SelBusVoltage(SDIF, SdifVoltage33v);
    Sdif_BusPowerOn(SDIF);

    enRet = SdCardChangeClock(SdClk400K);
    
    if (Ok != enRet)
    {
        return enRet;
    }

    //change later.
    stcCmdCfg.bHighSpeedMode = pstcCfg->bHighSpeedMode;
    stcCmdCfg.bUsingADMA = pstcCfg->bUsingADMA;
    stcCmdCfg.enBusWidth = pstcCfg->enBusWidth;
    stcCmdCfg.u32Clock = pstcCfg->u32Clock;

    enRet = Sdcmd_Init(&stcCmdCfg);

    return enRet;
}

/**
 ******************************************************************************
 ** \brief Detect SD card types
 **
 ** \param pstcCfg Pointer to SD card configuration
 **
 ** \retval SdCardTypeNone          No card is detected   
 ** \retval SdCardTypeSD            SD card is detected
 ** \retval SdCardTypeSDIO          SDID card is detected
 ** \retval SdCardTypeEMMC          EMMC card is detected
 ** 
 ******************************************************************************/
static en_sdcmd_cardtype_t SdCardTypeDetect( stc_sdcard_config_t *pstcCfg )
{

    en_result_t         enRet      = Error;
    en_sdcmd_cardtype_t enCardType = SdCardTypeNone;
    stc_sdcmd_info_t    *pstcInfo  = NULL;

    if (NULL == pstcCfg)
    {
        return SdCardTypeNone;
    }

    pstcInfo = &pstcCfg->stcInfo;

    // 1. Reset Card
    enRet = Sdcmd_Ops_GoIdle();
    if (Ok != enRet)
    {
        return SdCardTypeNone;
    }

    // 2. Check SDIO type. (CMD5 NOT IMPL SKIP.)

    // 3. check MMC/SD
    enRet = Sdcmd_Ops_SdSendOpCond(pstcInfo, 0x0);
    if (ErrorTimeout == enRet)
    {
        enCardType = SdCardTypeEMMC;
    }
    else if (Ok == enRet)
    {
        enCardType = SdCardTypeSD;
    }
    else
    {
        enCardType = SdCardTypeNone;
    }

    return enCardType;
}

/**
 ******************************************************************************
 ** \brief Initialize SD card 
 **
 ** \param pstcCfg Pointer to SD card configuration
 **
 ** \retval Ok                      SD card initialized successfully   
 ** \retval ErrorInvalidParameter   If one of following conditions macths:
 **                                 - NULL == pstcCfg
 **                                 - Other invalid configuration
 ** 
 ******************************************************************************/
static en_result_t SdCardInitSd( stc_sdcard_config_t *pstcCfg )
{
    en_result_t      enRet       = Error;
    stc_sdcmd_info_t *pstcSdInfo = NULL;
    uint8_t          u8F8        = 0;
    uint32_t         u32Retry    = 0;
    uint32_t         u32Arg      = 0;

    pstcSdInfo = &pstcCfg->stcInfo;

    // 1. Reset Card
    enRet = Sdcmd_Ops_GoIdle();
    if (Ok != enRet)
    {
        return enRet;
    }

    // 2. volatage check
    enRet = Sdcmd_Ops_SendIfCond(0x1AA);
    if (ErrorTimeout == enRet)
    {
        u8F8 = 0;
    }
    else
    {
        u8F8 = 1;
    }

    // 3.ACMD41 Get OCR
    enRet = Sdcmd_Ops_SdSendOpCond(pstcSdInfo, 0x0);

    u32Arg = ACMD41_VOLTAGE;
    if (1 == u8F8)
    {
        u32Arg |= ACMD41_HCS;
    }

    // 5. ACMD41 Set OCR
    u32Retry = 500;
    while (u32Retry-- > 0)
    {
        enRet = Sdcmd_Ops_SdSendOpCond(pstcSdInfo, u32Arg);
        if (Ok == enRet)
        {
            break;
        }
    }

    // start from CMD2
    // CMD2.Get CID, CID is about manufactor information. ignore.

    enRet = Sdcmd_Ops_AllSendCid(pstcSdInfo, 0x0);

    if (Ok != enRet)
    {
        return enRet;
    }

    // CMD3,     Get RCA

    usdelay(1000); //why need a big delay.

    enRet = Sdcmd_Ops_SendRelativeAddr(pstcSdInfo, 0x0);

    if (Ok != enRet)
    {
        return enRet;
    }

    // CMD9.Get CSD
    enRet = Sdcmd_Ops_SendCSD(pstcSdInfo);
    if (Ok != enRet)
    {
        return enRet;
    }

    // CMD7 .Select Card
    // usdelay(1000);
    enRet = Sdcmd_Ops_SelectCard(pstcSdInfo->RCA);
    if (Ok != enRet)
    {
        return enRet;
    }

    // may should check cmd7 response to unlock
    // cant do it no condition.
    // result = Sdcmd_SendCmd(SD_CMD42, 0, u32Buf, NULL, 0);

    if (SdBusWidth4 == pstcCfg->enBusWidth)
    {
        enRet = Sdcmd_SDChangeBusWidth(pstcSdInfo, SdBusWidth4);
        if (Ok != enRet)
        {
            return enRet;
        }
    }

    if (TRUE == pstcCfg->bHighSpeedMode)
    {
        enRet = Sdcmd_SDEnableHighSpeed(TRUE, SdClk20M);
        if (Ok != enRet)
        {
            return enRet;
        }
    }

    usdelay(1000);

    return Ok;

}

/**
 ******************************************************************************
 ** \brief Initialize eMMC card 
 **
 ** \param pstcCfg Pointer to SD card configuration
 **
 ** \retval Ok                      eMMC card initialized successfully   
 ** \retval ErrorInvalidParameter   If one of following conditions macths:
 **                                 - NULL == pstcCfg
 **                                 - Other invalid configuration
 ** 
 ******************************************************************************/
static en_result_t SdCardIniteMMC( stc_sdcard_config_t *pstcCfg )
{
    en_result_t      enRet        = Ok;
    stc_sdcmd_info_t *pstcMMCInfo = NULL;

    pstcMMCInfo = &pstcCfg->stcInfo;

    if (NULL == pstcCfg)
    {
        return ErrorInvalidParameter;
    }

    SDIF->SSRST_f.SWRSTCMDLN = 1u;
    while (SDIF->SSRST_f.SWRSTCMDLN != 0u);


    // 1. Reset Card  card CMD 0
    enRet = Sdcmd_Ops_GoIdle();
    if (Ok != enRet)
    {
        return enRet;
    }

    enRet = Error;
    while (Ok != enRet)
    {
        enRet = MMCcmd_Ops_SendOpCond(pstcMMCInfo, 0x40FF8000);
    }

    // CMD2.Get CID, CID is about manufactor information. ignore.
    enRet = Sdcmd_Ops_AllSendCid(pstcMMCInfo, 0x0);
    if (Ok != enRet)
    {
        return enRet;
    }

    usdelay(1000); //why need a big delay.

    // step .Get RCA
    enRet = Sdcmd_Ops_SendRelativeAddr(pstcMMCInfo, 0x0);
    if (Ok != enRet)
    {
        return enRet;
    }

    // step   Select Card
    enRet = Sdcmd_Ops_SelectCard(pstcMMCInfo->RCA);
    if (Ok != enRet)
    {
        return enRet;
    }

    enRet = MMCcmd_Ops_SendExtCsd(pstcMMCInfo);
    if (Ok != enRet)
    {
        return enRet;
    }

    if (SdBusWidth4 == pstcCfg->enBusWidth)
    {
        enRet = Sdcmd_MMCChangeBusWidth(pstcMMCInfo, SdBusWidth4);

        if (Ok != enRet)
        {
            return enRet;
        }

        usdelay(5000);
    }

    if (TRUE == pstcCfg->bHighSpeedMode)
    {
        enRet = Sdcmd_MMCEnableHighSpeed(TRUE, pstcCfg->u32Clock);
        if (Ok != enRet)
        {
            return enRet;
        }
    }

    usdelay(10000);

    return enRet;
}

/******************************************************************************/
/* Global Functions                                                           */
/******************************************************************************/
/**
 ******************************************************************************
 ** \brief Initialize card according to the card card detected 
 **
 ** \param pstcCfg Pointer to SD card configuration
 **
 ** \retval Ok                      Card initialized successfully   
 ** \retval ErrorInvalidParameter   If one of following conditions macths:
 **                                 - NULL == pstcCfg
 **                                 - Other invalid configuration
 ** 
 ******************************************************************************/
en_result_t Sdcard_Init( stc_sdcard_config_t *pstcCfg )
{
    en_sdcmd_cardtype_t enType = SdCardTypeNone;
    en_result_t         enRet  = Error;

    enRet = SdCardInitHost(pstcCfg);

    enType = SdCardTypeDetect(pstcCfg);

    switch ( enType )
    {
        case SdCardTypeSD:
            enRet = SdCardInitSd(pstcCfg);
            break;
        case SdCardTypeEMMC:
            enRet = SdCardIniteMMC(pstcCfg);
            break;
        default:
            enRet = Error;
            break;
    }
	
	//for some compile optimization, here needs long enough delay.
    usdelay(8000);
    return enRet;
}

/**
 ******************************************************************************
 ** \brief Read a single block data from SD card
 **
 ** \param pstcCfg Pointer to SD card configuration
 ** \param u32BlockIdx  Block index
 ** \param pu8BuffOut   Pointer to the buffer containing read data
 **
 ** \retval Ok                      A single block is read successfully   
 ** \retval ErrorInvalidParameter   If one of following conditions macths:
 **                                 - NULL == pstcCfg
 **                                 - Other invalid configuration
 ** 
 ******************************************************************************/
en_result_t Sdcard_Read_SingleBlock( stc_sdcard_config_t *pstcCfg,
                                     uint32_t u32BlockIdx,
                                     uint8_t *pu8BuffOut )
{
    en_result_t     enRet = Error;

    if (NULL == pstcCfg || NULL == pu8BuffOut)
    {
        return ErrorInvalidParameter;
    }

    enRet = Sdcmd_Ops_ReadSingleBlock(&pstcCfg->stcInfo, u32BlockIdx, pu8BuffOut);

    return enRet;
}

/**
 ******************************************************************************
 ** \brief Read multi-blocks data of SD card
 **
 ** \param pstcCfg Pointer to SD card configuration
 ** \param u32BlockIdx  Block index
 ** \param u32Count     Block count
 ** \param pu8BuffOut   Pointer to the buffer containing read data
 **
 ** \retval Ok                      A single block is read successfully   
 ** \retval ErrorInvalidParameter   If one of following conditions macths:
 **                                 - NULL == pstcCfg
 **                                 - Other invalid configuration
 ** 
 ******************************************************************************/
en_result_t Sdcard_Read_MultiBlock( stc_sdcard_config_t *pstcCfg,
                                    uint32_t u32BlockIdx,
                                    uint32_t u32Count,
                                    uint8_t *pu8BuffOut )
{
    en_result_t     enRet = Error;

    if (NULL == pstcCfg || NULL == pu8BuffOut)
    {
        return ErrorInvalidParameter;
    }

    enRet = Sdcmd_Ops_ReadMultiBlock(&pstcCfg->stcInfo, u32BlockIdx, u32Count, pu8BuffOut);

    return enRet;
}

/**
 ******************************************************************************
 ** \brief Write a single block data of SD card
 **
 ** \param pstcCfg Pointer to SD card configuration
 ** \param u32BlockIdx  Block index
 ** \param pu8BuffOut   Pointer to the buffer containing read data
 **
 ** \retval Ok                      A single block is written successfully   
 ** \retval ErrorInvalidParameter   If one of following conditions macths:
 **                                 - NULL == pstcCfg
 **                                 - Other invalid configuration
 ** 
 ******************************************************************************/
en_result_t Sdcard_Write_SingleBlock( stc_sdcard_config_t *pstcCfg,
                                      uint32_t u32BlockIdx,
                                      uint8_t *pu8BuffIn )
{
    en_result_t     enRet = Error;

    if (NULL == pstcCfg || NULL == pu8BuffIn)
    {
        return ErrorInvalidParameter;
    }

    enRet = Sdcmd_Ops_WriteSingleBlock(&pstcCfg->stcInfo, u32BlockIdx, pu8BuffIn);

    return enRet;

}

/**
 ******************************************************************************
 ** \brief Write multi-blocks data into SD card
 **
 ** \param pstcCfg Pointer to SD card configuration
 ** \param u32BlockIdx  Block index
 ** \param pu8BuffOut   Pointer to the buffer containing read data
 **
 ** \retval Ok                      Multi-block data is written successfully   
 ** \retval ErrorInvalidParameter   If one of following conditions macths:
 **                                 - NULL == pstcCfg
 **                                 - Other invalid configuration
 ** 
 ******************************************************************************/
en_result_t Sdcard_Write_MultiBlock( stc_sdcard_config_t *pstcCfg,
                                     uint32_t u32BlockIdx,
                                     uint32_t u32Count,
                                     uint8_t *pu8BuffIn )
{
    en_result_t     enRet = Error;

    if (NULL == pstcCfg || NULL == pu8BuffIn)
    {
        return ErrorInvalidParameter;
    }

    enRet = Sdcmd_Ops_WriteMultiBlock(&pstcCfg->stcInfo, u32BlockIdx, u32Count, pu8BuffIn);

    return enRet;
}

/**
 * \brief       Sdcard_GetStatus
 *
 * \param [In]  pstcCfg
 *
 * \retval      en_sdcmd_cardstatus_t
 */
/**
 ******************************************************************************
 ** \brief Get the status of SD card
 **
 ** \param pstcCfg Pointer to SD card configuration
 **
 ** \retval SD card status see the enumeration type #en_sdcmd_cardstatus_t
 ** 
 ******************************************************************************/
en_sdcmd_cardstatus_t Sdcard_GetStatus( stc_sdcard_config_t *pstcCfg )
{
    en_sdcmd_cardstatus_t enState = SdCardStateRev;

    if (NULL == pstcCfg)
    {
        return enState;
    }

    enState = Sdcmd_Ops_SendStatus(&pstcCfg->stcInfo);

    return enState;
}

#endif

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
