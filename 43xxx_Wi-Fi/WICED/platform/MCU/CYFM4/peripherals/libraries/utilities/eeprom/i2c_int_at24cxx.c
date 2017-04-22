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
/** \file i2c_int_at24cxx.c
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
#include <string.h>

#if (PDL_UTILITY_ENABLE_I2C_IRQ_AT24CXX == PDL_ON)

/*****************************************************************************/
/* Local pre-processor symbols/macros ('#define')                            */
/*****************************************************************************/
/*****************************************************************************/
/* Global variable definitions (declared in header file with 'extern')       */
/*****************************************************************************/
/*****************************************************************************/
/* Local type definitions ('typedef')                                        */
/*****************************************************************************/
/**
 ******************************************************************************
 ** \brief I2C read and write mode
 ******************************************************************************/
typedef enum en_i2c_rw_mode
{
    I2cTxMode = 0,          ///< I2C TX only 
    I2cRxMode,              ///< I2C RX only 
    I2cTxRxMode,            ///< I2C TX first and RX at the following
    
}en_i2c_rw_mode_t;

/**
 ******************************************************************************
 ** \brief I2C transfer sequence
 ******************************************************************************/
typedef enum en_i2c_seq
{
    I2cSeqStart = 0u,    ///< I2C Start process
    I2cSeqWrite,         ///< I2C Write process
    I2cSeqRestart,       ///< I2C Restart process
    I2cSeqRead,          ///< I2C Read process
    I2cSeqStop,          ///< I2C Stop process
    
}en_i2c_seq_t;

/**
 ******************************************************************************
 ** \brief I2C transfer information structure
 ******************************************************************************/
typedef struct stc_i2c_info
{
    boolean_t bI2cErrorFlag;      ///< Error flag
    uint8_t*  pu8TxBuf;           ///< Pointer to TX buffer
    uint32_t  u32TxSize;          ///< TX buffer size 
    uint8_t*  pu8RxBuf;           ///< Pointer to RX buffer
    uint32_t  u32RxSize;          ///< RX buffer size   
    uint32_t  u32Count;           ///< Transfer count 
    en_i2c_seq_t     enSequence;  ///< Transfer Sequence 
    uint8_t          u8DevAddr;   ///< Device address
    en_i2c_rw_mode_t enRwMode;    ///< R/W Flag
    uint32_t  u32TxRxGap;         ///< The period from end of TX and begin of restart 
    
}stc_i2c_info_t;

/*****************************************************************************/
/* Local variable definitions ('static')                                     */
/*****************************************************************************/
stc_i2c_irq_cb_t     stcI2cIntCb;
stc_i2c_info_t stcI2cInfo;

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
 ** \brief  Delay function
 **
 ** \param  [in]  u32Cnt   Time count
 **                                     
 ******************************************************************************/
static void Delay(uint32_t u32Cnt)
{
    for(;u32Cnt;u32Cnt--);
}


/*!
 ******************************************************************************
 ** \brief  I2C TX callback function 
 ******************************************************************************
 */
static void I2cTxCallback(void)
{
    if(stcI2cInfo.enSequence == I2cSeqStart)
    {
        if((stcI2cInfo.enRwMode == I2cTxMode) || (stcI2cInfo.enRwMode == I2cTxRxMode))
        {
            /* Prepare I2C device address with write flag */
            Mfs_I2c_SendData(EE_I2C_CH, ((stcI2cInfo.u8DevAddr<<1)|0u));
            
            stcI2cInfo.enSequence = I2cSeqWrite;
        }
        else
        {
            /* Prepare I2C device address with read flag */
            Mfs_I2c_SendData(EE_I2C_CH, ((stcI2cInfo.u8DevAddr<<1)|1u));
            
            Mfs_I2c_DisableIrq(EE_I2C_CH, I2cTxIrq); // Disable TX interrupt when read
        }
      
        /* Enable ACK */
        Mfs_I2c_ConfigAck(EE_I2C_CH, I2cAck);
        
        /* Generate I2C start signal */
        if(Ok != Mfs_I2c_GenerateStart(EE_I2C_CH))
        {
            stcI2cInfo.bI2cErrorFlag = 1;
            return; /* Timeout or other error */
        }
        
        
    }
    else if(stcI2cInfo.enSequence == I2cSeqRestart)
    {
        /* Sometimes a short time delay is needed before generate restart */
        Delay(stcI2cInfo.u32TxRxGap);
        /* Prepare I2C device address */
        Mfs_I2c_SendData(EE_I2C_CH, ((stcI2cInfo.u8DevAddr<<1)|1u));
        
        /* Enable ACK */
        Mfs_I2c_ConfigAck(EE_I2C_CH, I2cAck);
        
        /* Generate I2C start signal */
        if(Ok != Mfs_I2c_GenerateRestart(EE_I2C_CH))
        {
            stcI2cInfo.bI2cErrorFlag = 1;
            return ; /* Timeout or other error */
        }
        
        if(stcI2cInfo.enRwMode == I2cTxRxMode)
        {
            Mfs_I2c_DisableIrq(EE_I2C_CH, I2cTxIrq); // Disable TX interrupt when read
        }
        
    }
    else if(stcI2cInfo.enSequence == I2cSeqWrite)
    {
        if(stcI2cInfo.u32Count == stcI2cInfo.u32TxSize)
        {
            if(stcI2cInfo.enRwMode != I2cTxRxMode) 
            {
                /* Generate I2C stop signal */
                if(Ok != Mfs_I2c_GenerateStop(EE_I2C_CH))
                {
                    stcI2cInfo.bI2cErrorFlag = 1;
                    return; /* Timeout or other error */
                }
            }
            else // Don't generate stop signal if read continues after write
            {
                stcI2cInfo.enSequence = I2cSeqRestart;      
                stcI2cInfo.u32Count = 0;
            }
            return;
        }
    
        Mfs_I2c_SendData(EE_I2C_CH, *stcI2cInfo.pu8TxBuf++);
        Mfs_I2c_ClrStatus(EE_I2C_CH, I2cRxTxIrq);
        
        stcI2cInfo.u32Count++;

    }
}

/*!
 ******************************************************************************
 ** \brief  I2C RX callback function 
 ******************************************************************************
 */
static void I2cRxCallback(void)
{
    uint32_t u32Cnt = SystemCoreClock/4000000;
    
    if(stcI2cInfo.enSequence == I2cSeqRead)
    {
        *stcI2cInfo.pu8RxBuf++ = Mfs_I2c_ReceiveData(EE_I2C_CH);
        
        if(stcI2cInfo.u32Count == stcI2cInfo.u32RxSize)
        {
            // Generate stop signal
            if(Ok != Mfs_I2c_GenerateStop(EE_I2C_CH))
            {
                stcI2cInfo.bI2cErrorFlag = 1;
                return; /* Timeout or other error */
            }
            return;
        }
        
        if(stcI2cInfo.u32Count == stcI2cInfo.u32RxSize-1)   
        {
           Mfs_I2c_ConfigAck(EE_I2C_CH, I2cNAck); /* Last byte send a NACK */
        }
        
        /* wait some cycles for slave to prepare the data */
        while(u32Cnt--);
        
        Mfs_I2c_ClrStatus(EE_I2C_CH, I2cRxTxIrq);
        
        stcI2cInfo.u32Count++;
    }
}

/*!
 ******************************************************************************
 ** \brief  I2C interrupt callback function 
 ******************************************************************************
 */
static void I2cIntCallback(void)
{ 
    uint32_t u32Cnt = SystemCoreClock/4000000;
    
    if(TRUE == Mfs_I2c_GetStatus(EE_I2C_CH, I2cRxTxIrq)) 
    {  
        /* Check error status of I2C */
        if(TRUE == I2cCheckErrorStatus())
        {
            stcI2cInfo.bI2cErrorFlag = 1;
            return;
        }
      
        if((stcI2cInfo.enSequence != I2cSeqStart) && (stcI2cInfo.enSequence != I2cSeqRestart)) // In read or write sequence, clear INT at TX/RX callback
        {
            return;
        }
        
        /* Check ACK */
        if(I2cNAck == Mfs_I2c_GetAck(EE_I2C_CH))
        {
            stcI2cInfo.bI2cErrorFlag = 1;
            return;   /* NACK */
        }
        
        if(stcI2cInfo.enSequence == I2cSeqStart) 
        {
            if(stcI2cInfo.enRwMode == I2cRxMode)
            {   
                if(stcI2cInfo.u32Count == stcI2cInfo.u32RxSize-1)   
                {
                    Mfs_I2c_ConfigAck(EE_I2C_CH, I2cNAck); /* Last byte send a NACK */
                }
                
                /* wait some cycles for slave to prepare the data */
                while(u32Cnt--);
                
                /* Receive first data here and receive following data at RX callback */
                Mfs_I2c_ClrStatus(EE_I2C_CH, I2cRxTxIrq);
                
                stcI2cInfo.u32Count++;
                
                stcI2cInfo.enSequence = I2cSeqRead; // Start read data
            }
            else
            {
                stcI2cInfo.enSequence = I2cSeqWrite; // Start write data
            }
        }
        
        if(stcI2cInfo.enSequence == I2cSeqRestart) 
        {
            if(stcI2cInfo.u32Count == stcI2cInfo.u32RxSize-1)   
            {
                Mfs_I2c_ConfigAck(EE_I2C_CH, I2cNAck); /* Last byte send a NACK */
            }
            /* Receive first data here and receive following data at RX callback */
            Mfs_I2c_ClrStatus(EE_I2C_CH, I2cRxTxIrq);
            
            stcI2cInfo.u32Count++;
            
            stcI2cInfo.enSequence = I2cSeqRead; // Start read data
        }
    }
}

/*!
 ******************************************************************************
 ** \brief  I2C stop detection callback function 
 ******************************************************************************
 */
static void I2cStopDetectCallback(void)
{
    if(TRUE == Mfs_I2c_GetStatus(EE_I2C_CH, I2cStopDetect))
    {
        Mfs_I2c_ClrStatus(EE_I2C_CH, I2cStopDetect);
        Mfs_I2c_ClrStatus(EE_I2C_CH, I2cRxTxIrq);
        
        stcI2cInfo.enSequence = I2cSeqStop;
        
        Mfs_I2c_DisableIrq(EE_I2C_CH, I2cTxIrq); 
        Mfs_I2c_DisableIrq(EE_I2C_CH, I2cRxIrq); 
        Mfs_I2c_DisableIrq(EE_I2C_CH, I2cTxRxIrq); 
        Mfs_I2c_DisableIrq(EE_I2C_CH, I2cStopDetectIrq); 
    }
}

/*!
 ******************************************************************************
 ** \brief  Write I2C data
 **
 ** \param  u8DevAddr  7-bit device address
 ** \param  pu8Data  Pointer to data buffer
 ** \param  u32Size  Write buffer size
 **
 ** \retval Ok      Data are transferred normally
 ** \retval Error   Data transferred are ended with error
 ******************************************************************************
 */
static en_result_t I2cTxData(uint8_t u8DevAddr, uint8_t* pu8Data, uint32_t u32Size)
{    
    PDL_ZERO_STRUCT(stcI2cIntCb);
    PDL_ZERO_STRUCT(stcI2cInfo);
    
    stcI2cInfo.enSequence = I2cSeqStart;
    stcI2cInfo.enRwMode = I2cTxMode;
    stcI2cInfo.u8DevAddr = u8DevAddr;
    stcI2cInfo.pu8TxBuf = pu8Data;
    stcI2cInfo.u32TxSize = u32Size;
    stcI2cInfo.u32Count = 0;
    stcI2cInfo.bI2cErrorFlag = 0;
        
    Mfs_I2c_EnableIrq(EE_I2C_CH, I2cTxIrq);
    Mfs_I2c_EnableIrq(EE_I2C_CH, I2cTxRxIrq);
    Mfs_I2c_EnableIrq(EE_I2C_CH, I2cStopDetectIrq);
    
    while(stcI2cInfo.enSequence < I2cSeqStop)
    {
        if(stcI2cInfo.bI2cErrorFlag == 1)
        {
            return Error;
        }
    }
   
    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  Perform a dummy write and read at the following
 **
 ** \param  u8DevAddr  7-bit device address
 ** \param  pu8TxData  Pointer to write data buffer
 ** \param  u32TxSize  Write buffer size
 ** \param  pu8RxData  Pointer to read data buffer
 ** \param  u32RxSize  Read buffer size
 **
 ** \retval Ok      Data are transfer normally
 ** \retval Error   Data transfer are ended with error
 ******************************************************************************
 */
static en_result_t I2cTxRxData(uint8_t u8DevAddr, 
                               uint8_t* pu8TxData, 
                               uint32_t u32TxSize, 
                               uint8_t* pu8RxData, 
                               uint32_t u32RxSize)
{
    PDL_ZERO_STRUCT(stcI2cIntCb);
    PDL_ZERO_STRUCT(stcI2cInfo);
  
    stcI2cInfo.enSequence = I2cSeqStart;
    stcI2cInfo.enRwMode = I2cTxRxMode;
    stcI2cInfo.u8DevAddr = u8DevAddr;
    stcI2cInfo.pu8TxBuf = pu8TxData;
    stcI2cInfo.u32TxSize = u32TxSize;
    stcI2cInfo.pu8RxBuf = pu8RxData;
    stcI2cInfo.u32RxSize = u32RxSize;
    stcI2cInfo.u32Count = 0;
    stcI2cInfo.bI2cErrorFlag = 0;
    stcI2cInfo.u32TxRxGap = SystemCoreClock/(16000);
    
    Mfs_I2c_EnableIrq(EE_I2C_CH, I2cRxIrq);
    Mfs_I2c_EnableIrq(EE_I2C_CH, I2cTxRxIrq);
    Mfs_I2c_EnableIrq(EE_I2C_CH, I2cStopDetectIrq);
    Mfs_I2c_EnableIrq(EE_I2C_CH, I2cTxIrq);
    
    while(stcI2cInfo.enSequence < I2cSeqStop)
    {
        if(stcI2cInfo.bI2cErrorFlag == 1)
        {
            return Error;
        }
    }
    
    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  Read I2C data
 **
 ** \param  u8DevAddr  7-bit device address
 ** \param  pu8RxData Pointer to data buffer
 ** \param  u32RxSize  Read buffer size
 **
 ** \retval Ok      Data are read normally
 ** \retval Error   Data read are ended with error
 ******************************************************************************
 */
static en_result_t I2cRxData(uint8_t u8DevAddr, uint8_t* pu8RxData, uint32_t u32RxSize)
{  
    PDL_ZERO_STRUCT(stcI2cIntCb);
    PDL_ZERO_STRUCT(stcI2cInfo);
  
    stcI2cInfo.enSequence = I2cSeqStart;
    stcI2cInfo.enRwMode = I2cRxMode;
    stcI2cInfo.u8DevAddr = u8DevAddr;
    stcI2cInfo.pu8RxBuf = pu8RxData;
    stcI2cInfo.u32RxSize = u32RxSize;
    stcI2cInfo.u32Count = 0;
    stcI2cInfo.bI2cErrorFlag = 0;

    Mfs_I2c_EnableIrq(EE_I2C_CH, I2cRxIrq);
    Mfs_I2c_EnableIrq(EE_I2C_CH, I2cTxRxIrq);
    Mfs_I2c_EnableIrq(EE_I2C_CH, I2cStopDetectIrq);
    Mfs_I2c_EnableIrq(EE_I2C_CH, I2cTxIrq);
  
    while(stcI2cInfo.enSequence < I2cSeqStop)
    {
        if(stcI2cInfo.bI2cErrorFlag == 1)
        {
            return Error;
        }
    }
    
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
    stc_i2c_irq_cb_t stcI2cIrqCb;
    
    PDL_ZERO_STRUCT(stcI2cConfig);
    PDL_ZERO_STRUCT(stcI2cIrqCb);
    
    InitI2cIo();
    
    /* Initialize interrupt callback functions */
    stcI2cIrqCb.pfnTxIrqCb = I2cTxCallback;
    stcI2cIrqCb.pfnRxIrqCb = I2cRxCallback;
    stcI2cIrqCb.pfnTxRxIrqCb = I2cIntCallback;
    stcI2cIrqCb.pfnStopDetectIrqCb = I2cStopDetectCallback;
    
    /* Initialize configuration structure */
    stcI2cConfig.enMsMode = I2cMaster;
    stcI2cConfig.u32BaudRate = 100000u;
    stcI2cConfig.bWaitSelection = FALSE;
    stcI2cConfig.bDmaEnable = FALSE;
    stcI2cConfig.pstcFifoConfig = NULL;
    stcI2cConfig.pstcIrqEn = NULL;
    stcI2cConfig.pstcIrqCb = &stcI2cIrqCb;
    stcI2cConfig.bTouchNvic = TRUE;
    
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
  
    /* Send data address */
    if(AT24CXX_ADDR_LENGTH == 1)
    {
        au8TempData[0] = u16Addr & 0x00FFu;
        au8TempData[1] = u8Data;
    }
    else
    {
        au8TempData[0] = (u16Addr & 0xFF00u) >> 8;
        au8TempData[1] = u16Addr & 0x00FFu;
        au8TempData[2] = u8Data;
    }
  
    I2cTxData(u8DevAddr, au8TempData, (AT24CXX_ADDR_LENGTH+1));
  
    return Ok;
}


/**
 ******************************************************************************
 ** \brief   Write a page bytes at a page address of AT24CXX
 **
 ** \param  [in] u8DevAddr   7-bit device address
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
    uint8_t au8DataBuf[2+AT24CXX_PAGE_SIZE];
  
    if(u8Size > AT24CXX_PAGE_SIZE)
    {
        return ErrorInvalidParameter;
    }
        
    if(AT24CXX_ADDR_LENGTH == 1)
    {
        au8DataBuf[0] = u16PageAddr & 0x00FFu;
        memcpy(&au8DataBuf[1], pu8Data, u8Size);
    }
    else
    {
        au8DataBuf[0] = (u16PageAddr & 0xFF00u) >> 8;
        au8DataBuf[1] = u16PageAddr & 0x00FFu;
        memcpy(&au8DataBuf[2], pu8Data, u8Size);
    }
    
    if(Ok != I2cTxData(u8DevAddr, au8DataBuf, (AT24CXX_ADDR_LENGTH+u8Size)))
    {
        return Error;
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Read the data of current data address of AT24CXX
 **
 ** \param  [in]  u8DevAddr   7-bit device address
 ** \param  [out] pu8CurData  Pointer to the data read
 **
 ** \retval  Ok                       Read data normally
 ** \retval  Error                    Read data failed
 **                                     
 ******************************************************************************/
en_result_t At24cxx_CurrentAddrRead(uint8_t u8DevAddr, uint8_t* pu8CurData)
{   
    if(Ok != I2cRxData(u8DevAddr, pu8CurData, 1))
    {
        return Error;
    }
  
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Read a byte data of ramdon data address of AT24CXX
 **
 ** \param  [in] u8DevAddr   7-bit device address
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
    
    I2cTxRxData(u8DevAddr, au8TempData, AT24CXX_ADDR_LENGTH, pu8Data, 1);
  
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
    if(Ok != I2cRxData(u8DevAddr, pu8Data, u8Size))
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


#endif // if (PDL_UTILITY_ENABLE_I2C_INT_AT24CXX == PDL_ON)

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
