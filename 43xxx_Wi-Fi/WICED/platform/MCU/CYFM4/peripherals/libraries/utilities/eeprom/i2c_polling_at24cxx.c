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
/** \file i2c_polling_at24cxx.c
 **
 ** A detailed description is available at 
 ** @link I2cAt24cxxGroup Module description @endlink
 **
 ** History:
 **   - 2014-03-17  0.1  Edison Zhang  First version.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "i2c_at24cxx.h"

#if (PDL_UTILITY_ENABLE_I2C_POLLING_AT24CXX == PDL_ON)

/*****************************************************************************/
/* Local pre-processor symbols/macros ('#define')                            */
/*****************************************************************************/
/*****************************************************************************/
/* Global variable definitions (declared in header file with 'extern')       */
/*****************************************************************************/
/*****************************************************************************/
/* Local type definitions ('typedef')                                        */
/*****************************************************************************/
/*****************************************************************************/
/* Local variable definitions ('static')                                     */
/*****************************************************************************/
/**
 ******************************************************************************
 ** \brief   Check the error status of I2C
 **
 ** \retval  TRUE                       I2C error occurs 
 ** \retval  FALSE                      No error
 **                                     
 ******************************************************************************/
static boolean_t I2cCheckErrorStatus(void)
{    
    if(TRUE == Mfs_I2c_GetStatus(EE_I2C_CH, I2cBusErr))
    {
        return TRUE; /* Bus error occurs? */
    }
    
    if(TRUE == Mfs_I2c_GetStatus(EE_I2C_CH, I2cOverrunError))
    {
        return TRUE; /* Overrun error occurs? */
    }
    
    return FALSE;
}

/**
 ******************************************************************************
 ** \brief   Generate I2C start signal and send device address
 **
 ** \param   [in]  u8DevAddr          Device address
 **
 ** \retval  Ok                       I2C start normally
 ** \retval  ErrorTimeout             I2C timeout 
 ** \retval  Error                    Other errors
 **                                     
 ******************************************************************************/
static en_result_t I2cStart(uint8_t u8DevAddr)
{
    /* Prepare I2C device address */
    Mfs_I2c_SendData(EE_I2C_CH, u8DevAddr);
    
    /* Enable ACK */
    Mfs_I2c_ConfigAck(EE_I2C_CH, I2cAck);
    
    /* Generate I2C start signal */
    if(Ok != Mfs_I2c_GenerateStart(EE_I2C_CH))
    {
        return ErrorTimeout; /* Timeout or other error */
    }

    /* Wait until data transfer finish */
    while(1)
    {
        if(TRUE != Mfs_I2c_GetStatus(EE_I2C_CH, I2cRxTxIrq))
        {
            break;
        }
    }
    
    /* Check ACK */
    if(I2cNAck == Mfs_I2c_GetAck(EE_I2C_CH))
    {
        return Error;   /* NACK */
    }
    
    /* Check error status of I2C */
    if(TRUE == I2cCheckErrorStatus())
    {
        return Error;
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Generate I2C restart signal and send device address
 **
 ** \param   [in]  u8Addr            Device address
 **
 ** \retval  Ok                       I2C restart normally
 ** \retval  ErrorTimeout             I2C timeout 
 ** \retval  Error                    Other errors
 **                                     
 ******************************************************************************/
static en_result_t I2cRestart(uint8_t u8Addr)
{
    /* Prepare I2C device address */
    Mfs_I2c_SendData(EE_I2C_CH, u8Addr);
    
    /* Enable ACK */
    Mfs_I2c_ConfigAck(EE_I2C_CH, I2cAck);
    
    /* Generate I2C start signal */
    if(Ok != Mfs_I2c_GenerateRestart(EE_I2C_CH))
    {
        return ErrorTimeout; /* Timeout or other error */
    }

    /* Wait until data transfer finish */
    while(1)
    {
        if(TRUE != Mfs_I2c_GetStatus(EE_I2C_CH, I2cRxTxIrq))
        {
            break;
        }
    }
    
    /* Check ACK */
    if(I2cNAck == Mfs_I2c_GetAck(EE_I2C_CH))
    {
        return Error;   /* NACK */
    }
    
    /* Check error status of I2C */
    if(TRUE == I2cCheckErrorStatus())
    {
        return Error;
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Sene I2C data
 **
 ** \param   [in]  pu8Data            Pointer to data address
 ** \param   [in]  u8Size             Data size
 **
 ** \retval  Ok                       I2C data send normally
 ** \retval  Error                    I2C NACK, bus error, overrun error
 **                                     
 ******************************************************************************/
static en_result_t I2cSendData(uint8_t* pu8Data, uint8_t u8Size) 
{
    uint8_t i = 0;
    
    for(i=0;i<u8Size;i++)
    {
        /* Wait for end of transmission */
        while(1)
        {
            if(TRUE == Mfs_I2c_GetStatus(EE_I2C_CH, I2cRxTxIrq))
            {
                break;
            }
        }
      
        /* Transmit the data */
        Mfs_I2c_SendData(EE_I2C_CH, pu8Data[i]);
        Mfs_I2c_ClrStatus(EE_I2C_CH, I2cRxTxIrq);
        
        /* Check until TX finish */
        while(1)
        {
            if(TRUE == Mfs_I2c_GetStatus(EE_I2C_CH, I2cTxEmpty))
            {
                break;
            }
        }
       
        /* Check ACK */
        if(I2cNAck == Mfs_I2c_GetAck(EE_I2C_CH))
        {
            return Error;   /* NACK */
        }
        
        if(TRUE == I2cCheckErrorStatus())
        {
            return Error;
        }
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Read I2C data
 **
 ** \param   [out]  pRxData            Pointer to data address
 ** \param   [in]   u8Size             Data size
 **
 ** \retval  Ok                       I2C read normally
 ** \retval  Error                    I2C NACK, bus error, overrun error
 **                                     
 ******************************************************************************/
static en_result_t I2cRead(uint8_t *pRxData, uint8_t u8Size)
{
    uint8_t u8i = 0;
    uint8_t u8j;
       
    while(u8i < u8Size)
    {   
        /* Wait for the receive data */
        while(1)
        {
            if(TRUE == Mfs_I2c_GetStatus(EE_I2C_CH, I2cRxTxIrq))
            {
                break;
            }
        }
        
        u8j = SystemCoreClock/4000000;
        while(u8j--);
        
        
        if(u8i == u8Size-1)
        {
            Mfs_I2c_ConfigAck(EE_I2C_CH, I2cNAck); /* Last byte send a NACK */
        }
        else
        {
            Mfs_I2c_ConfigAck(EE_I2C_CH, I2cAck);
        }
        
        /* Clear interrupt flag and receive data to RX buffer */
        Mfs_I2c_ClrStatus(EE_I2C_CH, I2cRxTxIrq);
        
        /* Wait for the receive data */
        while(1)
        {
            if(TRUE == Mfs_I2c_GetStatus(EE_I2C_CH, I2cRxFull))
            {
                break;
            }
        }
         
        /* Check error status of I2C */
        if(TRUE == I2cCheckErrorStatus())
        {
            return Error;
        }
        
        pRxData[u8i++] = Mfs_I2c_ReceiveData(EE_I2C_CH);
    }
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Generate I2C stop signal
 **
 ** \retval  Ok                       I2C stop normally
 ** \retval  ErrorTimeout             I2C timeout 
 ** \retval  Error                    Other errors
 **                                     
 ******************************************************************************/
static en_result_t I2cStop(void)
{
    /* Generate I2C start signal */
    if(Ok != Mfs_I2c_GenerateStop(EE_I2C_CH))
    {
        return ErrorTimeout; /* Timeout or other error */
    }
    /* Clear Stop condition flag */
    while(1)
    {
        if(TRUE == Mfs_I2c_GetStatus(EE_I2C_CH, I2cStopDetect))
        {
            break;
        }
    }
   
    if(TRUE == Mfs_I2c_GetStatus(EE_I2C_CH, I2cBusErr))
    {
        return Error;
    }
   
    Mfs_I2c_ClrStatus(EE_I2C_CH, I2cStopDetect);
    Mfs_I2c_ClrStatus(EE_I2C_CH, I2cRxTxIrq);
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Initialize AT24CXX
 **
 ** \retval  Ok                       I2C start normally
 ** \retval  Error                    Other errors
 **                                     
 ******************************************************************************/
en_result_t At24cxx_Init(void)
{
    stc_mfs_i2c_config_t stcI2cConfig;
    PDL_ZERO_STRUCT(stcI2cConfig);
    
    InitI2cIo();
    
    stcI2cConfig.enMsMode = I2cMaster;
    stcI2cConfig.u32BaudRate = 100000u;
    stcI2cConfig.bWaitSelection = FALSE;
    stcI2cConfig.bDmaEnable = FALSE;
    stcI2cConfig.pstcFifoConfig = NULL;
    
    if(Ok != Mfs_I2c_Init(EE_I2C_CH, &stcI2cConfig))
    {
        return Error;
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Write byte at a certain address of AT24CXX
 **
 ** \param  [in] u8DevAddr   7-bit device address
 ** \param  [in] u16Addr     Data address
 ** \param  [in] u8Data      Data byte
 **
 ** \retval  Ok                       Write byte normally
 ** \retval  Error                    Write byte failed
 **                                     
 ******************************************************************************/
en_result_t At24cxx_ByteWrite(uint8_t u8DevAddr, uint16_t u16Addr, uint8_t u8Data)
{
    uint8_t au8TempData[3];
  
    if(Ok != I2cStart((u8DevAddr<<1) | 0u))
    {
        return Error;
    }
    
    /* Send page address */
    if(AT24CXX_ADDR_LENGTH == 1)
    {
        au8TempData[0] = u16Addr & 0x00FFu;
    }
    else
    {
        au8TempData[0] = (u16Addr & 0xFF00u) >> 8;
        au8TempData[1] = u16Addr & 0x00FFu;
    }
    
    if(Ok != I2cSendData(au8TempData, AT24CXX_ADDR_LENGTH))
    {
        return Error;
    }
    
    /* Send data */
    if(Ok != I2cSendData(&u8Data, 1))
    {
        return Error;
    }
    
    if(Ok != I2cStop())
    {
        return Error;
    }
    
    return Ok;
}


/**
 ******************************************************************************
 ** \brief   Write a page bytes at a page address of AT24CXX
 **
 ** \param  u8DevAddr        7-bit device address
 ** \param  [in] u16PageAddr Random addess within EEPROM memory
 ** \param  [in] pu8Data     Pointer to data array
 ** \param  [in] u8Size      Data size (shoudl be less than page size)
 **
 ** \retval  Ok                       Write page normally
 ** \retval  Error                    Write page failed
 **                                     
 ******************************************************************************/
en_result_t At24cxx_PageWrite(uint8_t u8DevAddr, uint16_t u16PageAddr, uint8_t* pu8Data, uint8_t u8Size)
{
    uint8_t au8TempData[2];
  
    if(u8Size > AT24CXX_PAGE_SIZE)
    {
        return ErrorInvalidParameter;
    }
    
    if(Ok != I2cStart((u8DevAddr<<1) | 0u))
    {
        return Error;
    }
    
    /* Send page address */
    if(AT24CXX_ADDR_LENGTH == 1)
    {
        au8TempData[0] = u16PageAddr & 0x00FFu;
    }
    else
    {
        au8TempData[0] = (u16PageAddr & 0xFF00u) >> 8;
        au8TempData[1] = u16PageAddr & 0x00FFu;
    }
    
    if(Ok != I2cSendData(au8TempData, AT24CXX_ADDR_LENGTH))
    {
        return Error;
    }
    
    /* Send data */
    if(Ok != I2cSendData(pu8Data, u8Size))
    {
        return Error;
    }
    
    if(Ok != I2cStop())
    {
        return Error;
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Read the data of current data address of AT24CXX
 **
 ** \param  [in]  u8DevAddr  7-bit device address
 ** \param  [out] pu8CurData Pointer to the data read
 **
 ** \retval  Ok                       Read data normally
 ** \retval  Error                    Read data failed
 **                                     
 ******************************************************************************/
en_result_t At24cxx_CurrentAddrRead(uint8_t u8DevAddr, uint8_t* pu8CurData)
{   
    if(Ok != I2cStart((u8DevAddr<<1)  | 1u))
    {
        return Error;
    }
    
    I2cRead(pu8CurData, 1);
    
    if(Ok != I2cStop())
    {
        return Error;
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Read a byte data of ramdon data address of AT24CXX
 **
 ** \param  [in] u8DevAddr  7-bit device address
 ** \param  [in] u16Addr     Data address
 ** \param  [out] pu8Data    Pointer to the data read
 **
 ** \retval  Ok                       Read data normally
 ** \retval  Error                    Read data failed
 **                                     
 ******************************************************************************/
en_result_t At24cxx_RandomRead(uint8_t u8DevAddr, uint16_t u16Addr, uint8_t* pu8Data)
{
    uint8_t au8TempData[2];
    uint32_t u32DelayCnt = SystemCoreClock/10000;
    
    /* Send start with write flag */
    if(Ok != I2cStart((u8DevAddr<<1)  | 0u))
    {
        return Error;
    }
    
    /* Send address */
    if(AT24CXX_ADDR_LENGTH == 1)
    {
        au8TempData[0] = u16Addr & 0x00FFu;
    }
    else
    {
        au8TempData[0] = (u16Addr & 0xFF00u) >> 8;
        au8TempData[1] = u16Addr & 0x00FFu;
    }
    
    if(Ok != I2cSendData(au8TempData, AT24CXX_ADDR_LENGTH))
    {
        return Error;
    }
    /* Dummy delay */
    while(u32DelayCnt--);
    
    /* Send start with read flag */
    if(Ok != I2cRestart((u8DevAddr<<1)  | 1u))
    {
        return Error;
    }
       
    /* Read address */   
    if(Ok != I2cRead(pu8Data, 1))
    {
        return Error;
    }
       
    if(Ok != I2cStop())
    {
        return Error;
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Read data bytes following by random read or current address read
 **
 ** \param  [in]  u8DevAddr   7-bit device address
 ** \param  [out] pu8Data     Pointer to the data read
 ** \param  [in]  u8Size      Data size
 **
 ** \retval  Ok                       Read data normally
 ** \retval  Error                    Read data failed
 **                                     
 ******************************************************************************/
en_result_t At24cxx_SequentialRead(uint8_t u8DevAddr, uint8_t* pu8Data, uint8_t u8Size)
{
    if(Ok != I2cStart((u8DevAddr<<1)  | 1u))
    {
        return Error;
    }
    
    if(Ok != I2cRead(pu8Data, u8Size))
    {
        return Error;
    }
    
    if(Ok != I2cStop())
    {
        return Error;
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief  ms delay function
 **
 ** \param  [in]  u32Cnt   ms count
 **                                     
 ******************************************************************************/
void At24cxx_Delayms(uint32_t u32Cnt)
{
    uint32_t u32i;
    
    for(;u32Cnt;u32Cnt--)
        for(u32i=SystemCoreClock/12000; u32i; u32i--);
}

#endif // if (PDL_UTILITY_ENABLE_I2C_POLLING_AT24CXX == PDL_ON)

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
