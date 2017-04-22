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
/** \file s34ml01g.c
 **
 ** S34ML01G (Nand Flash) driver API functions
 **
 ** History:
 ** - 2015-01-28  1.0   EZh  First version for FM universal PDL
 **
 *****************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "s34ml01g.h"

#if (PDL_UTILITY_ENABLE_EXTIF_S34ML01G == PDL_ON)      
   
/**
 ******************************************************************************
 ** \addtogroup S34ml101gGroup
 ******************************************************************************/
//@{      
   
/*****************************************************************************/
/* Local pre-processor symbols/macros ('#define')                            */
/*****************************************************************************/
// NAND command 
#define NAND_CMD_READ                  0x00u
#define NAND_CMD_READ_CONFIRM          0x30u
#define NAND_CMD_PAGEPROG              0x10u
#define NAND_CMD_READOOB               0x50u
#define NAND_CMD_ERASE1                0x60u
#define NAND_CMD_STATUS                0x70u
#define NAND_CMD_SEQIN                 0x80u
#define NAND_CMD_RANDOM_IN             0x85u
#define NAND_CMD_RANDOM_OUT            0x05u
#define NAND_CMD_RANDOM_OUT_CONFIRM    0xE0u
#define NAND_CMD_READID                0x90u
#define NAND_CMD_READID1               0x91u
#define NAND_CMD_ERASE2                0xd0u
#define NAND_CMD_RESET                 0xFFu

/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/
static uint32_t u32NandBaseAddr;

/**
 ******************************************************************************
 ** \brief  Time delay
 **
 ** \param  u32Cnt   Delay count
 **
 ******************************************************************************/
static void Wait(uint32_t u32Cnt)
{
	volatile uint32_t u32delay = u32Cnt;
	
    while(u32delay--);
}

/**
 ******************************************************************************
 ** \brief  Read status of operation
 **
 ** \retval Ok             The operation passed
 ** \retval ErrorTimeout   The operation is time out
 ** \retval Error          The operation failed.
 ******************************************************************************/
static en_result_t ReadStatus(void)
{
    uint32_t u32TimeOut = 0;
    
    Extif_Nand_SetCommand(u32NandBaseAddr, NAND_CMD_STATUS);
    
    while(!(Extif_Nand_ReadData(u32NandBaseAddr) & 0x40u))
    {
        u32TimeOut++;
        if(u32TimeOut == 0x00100000)
        {
            return ErrorTimeout;
        }
    }
    
    if(Extif_Nand_ReadData(u32NandBaseAddr) & 0x01u)
    {
        return Error;
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief  Initialize S34M101G with external bus
 **
 ** \param  u8CsArea        Chip selection area 
 ** \param  u32BaseAddress  Base address of Nand Flash
 **
 ** \retval Ok      S34M101G initialized successfully
 ** \retval Error   S34M101G initialization failed
 ******************************************************************************/
en_result_t S34ml01g_Init(uint8_t u8CsArea, uint32_t u32BaseAddress)
{    
    stc_extif_area_config_t stcMemConfig;
    
    // Clear structure
    PDL_ZERO_STRUCT(stcMemConfig);
    
    stcMemConfig.enWidth = Extif8Bit;     // 8-bit width
    stcMemConfig.bReadByteMask = TRUE;    // Use read byte mask
    stcMemConfig.bWriteEnableOff = FALSE; // Don't enable write enable off
    stcMemConfig.bNandFlash  = TRUE;      // Enable Nand Flash mode
    stcMemConfig.bPageAccess = FALSE;     // Disable page access 
    stcMemConfig.bRdyOn = FALSE;          // Don't use ready signal
    stcMemConfig.bStopDataOutAtFirstIdle = TRUE;  // Stop data output at first idle cycle
    stcMemConfig.bMultiplexMode = FALSE;          // Single mode
    stcMemConfig.bAleInvert = FALSE;              // Don't invert ALE
    stcMemConfig.bAddrOnDataLinesOff = TRUE;      // Don't output address on data lines
    stcMemConfig.bMpxcsOff = FALSE;               // Assert MCSX in ALC cycle period
    stcMemConfig.bMoexWidthAsFradc = FALSE;       // MOEX width is set with RACC-RADC 
    
    stcMemConfig.enReadAccessCycle          = Extif4Cycle;  // Read Access Cycle timing
    stcMemConfig.enReadAddressSetupCycle    = Extif0Cycle;  // Read Address Set-up Cycle timing
    stcMemConfig.enFirstReadAddressCycle    = Extif1Cycle;  // First Read Address Cycle timing
    stcMemConfig.enReadIdleCycle            = Extif4Cycle;  // Read Idle Cycle timing
    stcMemConfig.enWriteAccessCycle         = Extif5Cycle;  // Write Access Cycle timing
    stcMemConfig.enWriteAddressSetupCycle   = Extif1Cycle;  // Write Address Set-up Cycle timing
    stcMemConfig.enWriteEnableCycle         = Extif4Cycle;  // Write Enable Cycle timing
    stcMemConfig.enWriteIdleCycle           = Extif1Cycle;  // Write Idle Cycle timing
    
    stcMemConfig.u8AreaAddress = ((u32BaseAddress >> 20u) & 0x00FFu);
    stcMemConfig.enAreaMask = Extif16MB; // Dummy setting
    stcMemConfig.enAddressLatchCycle = Extif8Cycle;
    stcMemConfig.enAddressLatchSetupCycle = Extif5Cycle;
    stcMemConfig.enAddressLatchWidthCycle = Extif5Cycle;
    stcMemConfig.bMclkoutEnable = FALSE;
    if (SystemCoreClock <= 160000000)
    {
        stcMemConfig.u8MclkDivision = 1u;
    }
    else
    {
        stcMemConfig.u8MclkDivision = 2u;
    }
    
    if (Ok != Extif_InitArea(u8CsArea, &stcMemConfig))
    {
        return Error;
    }
    
    u32NandBaseAddr = u32BaseAddress;
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief  Read page from S34M101G
 **
 ** \param  u32Block        Block index
 ** \param  u32Page         Page index
 ** \param  u16PageOffset   Page offset in a page (main area: 0~2047, spare area: 2048 ~ 2011)
 ** \param  pu8Data         Pointer to data read
 ** \param  u16DataSize     Data size to be read
 **
 ** \retval Ok      Data read successfully
 ** \retval Error   Data read error
 ******************************************************************************/
en_result_t S34ml01g_ReadPage(uint32_t u32Block, 
                              uint32_t u32Page, 
                              uint16_t u16PageOffset, 
                              uint8_t *pu8Data,
                              uint16_t u16DataSize)
{
    uint32_t u32BlockPage, i;
    
    u32BlockPage = (u32Block << 6u) + u32Page;                     /* 1 block=64 page */
    
    Extif_Nand_SetCommand(u32NandBaseAddr, NAND_CMD_READ);                       /* send read data */
    
    Extif_Nand_SetAddress(u32NandBaseAddr, (u16PageOffset & 0xFFu)); 
    Extif_Nand_SetAddress(u32NandBaseAddr, (u16PageOffset >> 8u)); 
    Extif_Nand_SetAddress(u32NandBaseAddr, (u32BlockPage & 0xFFu));                           
    Extif_Nand_SetAddress(u32NandBaseAddr, ((u32BlockPage >> 8u) & 0xFFu));              /* send 3 byte address */
    Extif_Nand_ClearAle(u32NandBaseAddr);
    
    Extif_Nand_SetCommand(u32NandBaseAddr, NAND_CMD_READ_CONFIRM);                       /* send read data */
    
    Wait(1000u);
    
    if(u16PageOffset < 2112u)
    {
        for(i=u16PageOffset; i<(u16PageOffset+u16DataSize); i++)
        {
            *pu8Data++ = Extif_Nand_ReadData(u32NandBaseAddr);                  /* read data */
        }
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief  Erase block of S34M101G
 **
 ** \param  u32Block        Block index
 **
 ** \retval Ok      Block erased successfully
 ** \retval Error   Block erase failed
 ******************************************************************************/
en_result_t S34ml01g_EraseBlock(uint32_t  u32Block)
{
    uint32_t u32BlockPage;

    u32BlockPage = (u32Block << 6u);

    Extif_Nand_SetCommand(u32NandBaseAddr ,NAND_CMD_ERASE1);  // send erase command 
    Extif_Nand_SetAddress(u32NandBaseAddr, (u32BlockPage & 0xFFu));
    Extif_Nand_SetAddress(u32NandBaseAddr, ((u32BlockPage >> 8u) & 0xFFu));
    Extif_Nand_ClearAle(u32NandBaseAddr);
    Extif_Nand_SetCommand(u32NandBaseAddr, NAND_CMD_ERASE2);  // start erase 
    
    Wait(500);
    
    if(Ok != ReadStatus()) 
    {
       return Error;
    }    
    
    return Ok;                          
}

/**
 ******************************************************************************
 ** \brief  Write page from S34M101G
 **
 ** \param  u32Block        Block index
 ** \param  u32Page         Page index
 ** \param  u16PageOffset   Page offset in a page (main area: 0~2047, spare area: 2048 ~ 2011)
 ** \param  pu8Data         Pointer to write read
 ** \param  u16DataSize     Data size to be written
 **
 ** \retval Ok      Data written successfully
 ** \retval Error   Data written error
 ******************************************************************************/
en_result_t S34ml01g_WritePage(uint32_t u32Block, 
                               uint32_t u32Page, 
                               uint16_t u16PageOffset,
                               uint8_t *pu8Data,
                               uint16_t u16DataSize)
{
    uint32_t u32BlockPage,i;
    
    u32BlockPage = (u32Block << 6) + u32Page;

    Extif_Nand_SetCommand(u32NandBaseAddr, NAND_CMD_SEQIN);                          /* send write command */
    Extif_Nand_SetAddress(u32NandBaseAddr, (u16PageOffset & 0xFFu));
    Extif_Nand_SetAddress(u32NandBaseAddr, (u16PageOffset >> 8u)); 
    Extif_Nand_SetAddress(u32NandBaseAddr, (u32BlockPage & 0xFFu));
    Extif_Nand_SetAddress(u32NandBaseAddr, ((u32BlockPage >> 8u ) & 0xFFu));
    Extif_Nand_ClearAle(u32NandBaseAddr);
    Wait(500);
    
    if(u16PageOffset < 2112) // Set PageOffset to 0~2047 to program main area, set PageOffset to 2048~21112 to program spare area
    {     
        for(i=u16PageOffset; i <(u16PageOffset+u16DataSize); i++) 
        {
            Extif_Nand_WriteData(u32NandBaseAddr, *pu8Data++); /* write data */
        }
        
        Wait(500);
            
    }
   
    Extif_Nand_SetCommand(u32NandBaseAddr, NAND_CMD_PAGEPROG);  /* start programming */
    Wait(3000);
    
    if(Ok != ReadStatus()) 
    {
       return Error;
    }   
    
    return Ok;                         
}

/**
 ******************************************************************************
 ** \brief  Read ID from S34M101G
 **
 ** \param  pu8aId  Pointer to ID array
 ** \param  u8Cnt   ID count
 **
 ** \retval Ok      ID read successfully
 ******************************************************************************/
en_result_t S34ml01g_ReadID(uint8_t *pu8aId, uint8_t u8Cnt)
{
    uint8_t i;

    Extif_Nand_SetCommand(u32NandBaseAddr, NAND_CMD_READID);
    Extif_Nand_SetAddress(u32NandBaseAddr, 0x00);
    Extif_Nand_ClearAle(u32NandBaseAddr);
    Wait(10);
    for(i=0; i<u8Cnt; i++)
    {
        *pu8aId++ = Extif_Nand_ReadData(u32NandBaseAddr);
    }

    return Ok;
}

//@}

#endif

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
