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
/** \file s26kl512s.c
 **
 ** S26KL512S driver API functions
 **
 ** History:
 ** - 2015-01-28  1.0   EZh  First version for FM universal PDL
 **
 *****************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "s26kl512s.h"

#if (PDL_UTILITY_ENABLE_HBIF_S26KL512S == PDL_ON)   
   
/**
 ******************************************************************************
 ** \addtogroup S26kl512sGroup
 ******************************************************************************/
//@{   

#define FLASH_STATUS_DEVICE_READY             0x80u
#define FLASH_STATUS_ERASE_STATUS             0x20u
#define FLASH_STATUS_PGM_STATUS               0x10u
#define FLASH_STATUS_SECTOR_ERASE_STATUS      0x01u

/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/
uint32_t u32FlashBaseAddress;   

/******************************************************************************/
/* Local function prototypes                                                  */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Initialize S26KL512S
 **
 ** This function maps S26KL512S to a certain external area
 **
 ** \param [in]  u8CsArea        Chip selection area selected
 ** \param [in]  u32BaseAddress  Base address of S26KL512S
 **
 ** \retval Ok                    S26KL512S initialized normally
 ** \retval Error                 Invalid configuration
 ******************************************************************************/
en_result_t S26kl512s_Init(uint8_t u8CsArea, uint32_t u32BaseAddress)
{
    stc_hbif_mem_config_t stcMemConfig;
    stc_hbif_config_t stcHbifConfig;
    uint32_t i,j;
    // Clear structure
    PDL_ZERO_STRUCT(stcMemConfig);
    PDL_ZERO_STRUCT(stcHbifConfig);
    
    stcMemConfig.u32MemBaseAddress = u32BaseAddress;
    stcMemConfig.bMergeContinuousRead = FALSE;
    stcMemConfig.bSupportAsymmetryCache = FALSE;
    stcMemConfig.bRegisterSpace = FALSE;
    stcMemConfig.bHyperRamDevice = FALSE;
    stcMemConfig.enWrapBustsize = HbifWrap32Bytes;
    stcMemConfig.enReadCsHighCycle = HbifCshi15Clk;
    stcMemConfig.enWriteCsHighCycle = HbifCshi15Clk;
    stcMemConfig.enReadCsNextClkCycle = HbifCcs2Clk;
    stcMemConfig.enWriteCsNextClkCycle = HbifCcs2Clk;
    stcMemConfig.enReadCsHoldCycle = HbifCsh2Clk;
    stcMemConfig.enWriteCsHoldCycle = HbifCsh2Clk;
    
    if (0u == u8CsArea)
    {
        stcHbifConfig.pstcMem0Config = &stcMemConfig;
    }
    else if(1u == u8CsArea)
    {
        stcHbifConfig.pstcMem1Config = &stcMemConfig;
    }
    else
    {
        return ErrorInvalidParameter;
    }
    
    if (Ok != Hbif_Init(&stcHbifConfig))
    {
        return Error;
    }
    
    u32FlashBaseAddress = u32BaseAddress;
    
	for(i = 0,j = 0;i < 2000000;i++){
        j++;
	}
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Erase whole chip of S26KL512S
 **
 ** This function erases all chip, which means turn all area to 0xFF.
 **
 ** \retval Ok                    S26KL512S erased successful
 ** \retval Error                 Erase error occurs.
 ******************************************************************************/
en_result_t S26kl512s_ChipErase(void)
{
    uint16_t u16Status;
    en_result_t enRet;
  
    // Issue chip erase sequence
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x555u << 1u)) = 0x00AAu;
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x2AAu << 1u)) = 0x0055u;
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x555u << 1u)) = 0x0080u;
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x555u << 1u)) = 0x00AAu;
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x2AAu << 1u)) = 0x0055u;
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x555u << 1u)) = 0x0010u;
    
    while(1)
    {
        // Issue status read sequence
        *(volatile uint16_t*)((uint8_t*)u32FlashBaseAddress + (0x555u << 1u)) = 0x0070u;
        u16Status = *(uint16_t*)u32FlashBaseAddress;
        
        // Check status
        if((u16Status & FLASH_STATUS_DEVICE_READY) == 0x80u)
        {
            if((u16Status & FLASH_STATUS_ERASE_STATUS) == 0u)
            {
                enRet = Ok;
            }
            else
            {
                enRet = Error; 
            }
            
            break;
        }
    }
    
    return enRet;
    
}

/**
 ******************************************************************************
 ** \brief Erase a sector of S26KL512S
 **
 ** This function erases one specified sector.
 **
 ** \param [in]  pu16SectorAddr   Sector base address
 **
 ** \retval Ok                    S26KL512S sector erased successful
 ** \retval Error                 Erase error occurs.
 ******************************************************************************/
en_result_t S26kl512s_SectorErase(uint16_t* pu16SectorAddr)
{
    uint16_t u16Status;
    en_result_t enRet;
  
    // Issue chip erase sequence
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x555u << 1u)) = 0x00AAu;
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x2AAu << 1u)) = 0x0055u;
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x555u << 1u)) = 0x0080u;
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x555u << 1u)) = 0x00AAu;
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x2AAu << 1u)) = 0x0055u;
    *(volatile uint16_t*)pu16SectorAddr = 0x0030u;
    
    while(1)
    {
        // Issue status read sequence
        *(uint16_t*)((uint8_t*)u32FlashBaseAddress + (0x555u << 1u)) = 0x0070u;
        u16Status = *(uint16_t*)u32FlashBaseAddress;
        
        // Check status
        if((u16Status & FLASH_STATUS_DEVICE_READY) == 0x80u)
        {
            if((u16Status & FLASH_STATUS_SECTOR_ERASE_STATUS) == 0u)
            {
                enRet = Ok;
            }
            else
            {
                enRet = Error; 
            }
            
            break;
        }
    }
    
    return enRet;
    
}

/**
 ******************************************************************************
 ** \brief Blank check a sector of S26KL512S
 **
 ** This function checks whether a sector is blank or not
 **
 ** \param [in]  pu16SectorAddr   Sector base address
 **
 ** \retval Ok                    S26KL512S blank check successful
 ** \retval Error                 Blank check error occurs.
 ******************************************************************************/
en_result_t S26kl512s_BlankCheck(uint16_t* pu16SectorAddr)
{
    uint16_t u16Status;
    en_result_t enRet;
    
    // Issue blank check sequence
    *(volatile uint16_t*)(pu16SectorAddr + (0x555u << 1u)) = 0x0033u;
    
    while(1)
    {
        // Issue status read sequence
        *(uint16_t*)((uint8_t*)u32FlashBaseAddress + (0x555u << 1u)) = 0x0070u;
        u16Status = *(uint16_t*)u32FlashBaseAddress;
        
        // Check status
        if((u16Status & FLASH_STATUS_DEVICE_READY) == 0x80u)
        {
            if((u16Status & FLASH_STATUS_SECTOR_ERASE_STATUS) == 0u)
            {
                enRet = Ok;
            }
            else
            {
                enRet = Error; 
            }
            
            break;
        }
    }
    
    return enRet;
}

/**
 ******************************************************************************
 ** \brief Program a word(16-bit) to S26KL512S
 **
 ** This function programs a word(16-bit) to the specified address.
 **
 ** \param [in]  pu16Addr         Data address
 ** \param [in]  u16Data          One word data (16-bit)
 **
 ** \retval Ok                    S26KL512S programmed successful
 ** \retval Error                 Programming error occurs
 ******************************************************************************/
en_result_t S26kl512s_WordProgram(uint16_t* pu16Addr, uint16_t u16Data)
{
    uint16_t u16Status;
    en_result_t enRet;
  
    // Issue chip erase sequence
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x555u << 1u)) = 0x00AAu;
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x2AAu << 1u)) = 0x0055u;
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x555u << 1u)) = 0x00A0u;
    *(volatile uint16_t*)pu16Addr = u16Data;
    
    while(1)
    {
        // Issue status read sequence
        *(uint16_t*)((uint8_t*)u32FlashBaseAddress + (0x555u << 1u)) = 0x0070u;
        u16Status = *(uint16_t*)u32FlashBaseAddress;
        
        // Check status
        if((u16Status & FLASH_STATUS_DEVICE_READY) == 0x80)
        {
            if((u16Status & FLASH_STATUS_PGM_STATUS) == 0u)
            {
                enRet = Ok;
            }
            else
            {
                enRet = Error; 
            }
            
            break;
        }
    }
    
    return enRet;
    
}

/**
 ******************************************************************************
 ** \brief Write the data array to internal buffer of S26KL512S and program 
 **
 ** This function writes the data array to internal buffer first and then 
 ** program the data in buffer into Flash.
 **
 ** \param [in]  pu16Addr         Data address (must align with 256 words(512 bytes))
 ** \param [in]  u16Data          Pointer to data array
 ** \param [in]  u16Datalength    Data length (must less than 256)
 **
 ** \retval Ok                    S26KL512S programmed successfully
 ** \retval Error                 Programming error occurs
 ******************************************************************************/
en_result_t S26kl512s_WriteBufferAndProgram(uint16_t* pu16Addr, 
                                            uint16_t* pu16Data, 
                                            uint16_t u16Datalength)
{
    uint32_t u32SecBaseAddr, u32OffsetAddr, u32DataAddr, i;
    en_result_t enRet;
    uint16_t u16Status;
  
    if (u16Datalength > 256)
    {
        return ErrorInvalidParameter;
    }
    
    u32DataAddr = (uint32_t)pu16Addr;
    
    if(u32DataAddr%256 != 0)
    {
        return ErrorInvalidParameter;
    }
    
    u32SecBaseAddr = (u32DataAddr/0x40000)*0x40000;
    u32OffsetAddr = u32DataAddr%0x40000;
    
    // Issue write to buffer sequence
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x555u << 1u)) = 0x00AAu;
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x2AAu << 1u)) = 0x0055u;
    *(volatile uint16_t*)u32SecBaseAddr = 0x0025u;
    *(volatile uint16_t*)u32SecBaseAddr = u16Datalength - 1;
    
    for (i=0; i<u16Datalength; i++)
    {
        *(volatile uint16_t*)(u32SecBaseAddr + u32OffsetAddr + i*2) = *(pu16Data+i);
    }
    
    // Issue write to buffer conform sequence
    *(volatile uint16_t*)u32SecBaseAddr = 0x0029u;
    
    while(1)
    {
        // Issue status read sequence
        *(uint16_t*)((uint8_t*)u32FlashBaseAddress + (0x555u << 1u)) = 0x0070u;
        u16Status = *(uint16_t*)u32FlashBaseAddress;
        
        // Check status
        if((u16Status & FLASH_STATUS_DEVICE_READY) == 0x80)
        {
            if((u16Status & FLASH_STATUS_PGM_STATUS) == 0u)
            {
                enRet = Ok;
            }
            else
            {
                enRet = Error; 
            }
            
            break;
        }
    }
    
    return enRet;
}

/**
 ******************************************************************************
 ** \brief Read ID array from S26KL512S
 **
 ** This function reads ID information from S26KL512S
 **
 ** \param [output]  pu16Id           Pointer to ID data buffer         
 ** \param [in]      u8WordOffset     ID offset in the ID map
 ** \param [in]      u8WordCnt        ID data count to be read
 **
 ** \retval Ok                    ID array read successfully
 ** \retval Error                 ID array read failed
 ******************************************************************************/
en_result_t S26kl512s_ReadId(uint16_t* pu16Id, 
                             uint8_t u8WordOffset, 
                             uint8_t u8WordCnt)
{
    uint8_t i;
  
    if(pu16Id == NULL)
    {
        return ErrorInvalidParameter;
    }
    
    // Issue ID read entry
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x555u << 1u)) = 0x00AAu;
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x2AAu << 1u)) = 0x0055u;
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x555u << 1u))  = 0x0090u;
    
    // Issue ID/CFI read
    for (i=0; i<u8WordCnt; i++)
    {
        *pu16Id = *((volatile uint16_t*)u32FlashBaseAddress + u8WordOffset + i);
    }
    
    // Exit ASO
    *(volatile uint16_t*)u32FlashBaseAddress = 0x00F0u;
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Read CFI array from S26KL512S
 **
 ** This function reads CFI information from S26KL512S
 **
 ** \param [output]  pu16Id           Pointer to CFI data buffer         
 ** \param [in]      u8WordOffset     CFI offset in the CFI map
 ** \param [in]      u8WordCnt        CFI data count to be read
 **
 ** \retval Ok                    CFI array read successfully
 ** \retval Error                 CFI array read failed
 ******************************************************************************/
en_result_t S26kl512s_ReadCfi(uint16_t* pu16Cfi, 
                              uint8_t u8WordOffset, 
                              uint8_t u8WordCnt)
{
    uint8_t i;
  
    if(pu16Cfi == NULL)
    {
        return ErrorInvalidParameter;
    }
    
    // Issue CFI read entry
    *(volatile uint16_t*)(u32FlashBaseAddress + (0x55u << 1u)) = 0x0098u;
    
    // Issue ID/CFI read
    for (i=0; i<u8WordCnt; i++)
    {
        *pu16Cfi = *((volatile uint16_t*)u32FlashBaseAddress + u8WordOffset + i);
    }
    
    // Exit ASO
    *(volatile uint16_t*)u32FlashBaseAddress = 0x00F0u;
    
    return Ok;
}

//@} // S26kl512sGroup  

#endif

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
