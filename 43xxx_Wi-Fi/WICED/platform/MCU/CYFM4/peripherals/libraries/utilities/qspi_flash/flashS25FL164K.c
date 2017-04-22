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
/** \file flashS25FL164K.c
 **
 ** Driver module for HS-SPI with external S25FL164K Flash
 **
 ** History:
 **   - 2014-10-24  1.0  MWi       First version.
 **
 ******************************************************************************/

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include "flashS25FL164K.h"
#include <string.h>     // for memset()

#if (PDL_UTILITY_ENABLE_QSPI_POLLING_S25FL164K == PDL_ON)

/*****************************************************************************/
/* Local variable definitions                                                */
/*****************************************************************************/
static en_result_t  enResult;
static boolean_t    m_bQioModeEnabled = FALSE;

/*****************************************************************************/
/* Global variable definitions                                               */
/*****************************************************************************/

/*****************************************************************************
 ** \brief Wait until BUSY (Embedded operation in progress) bit is released
 **
 ** \param [in]  u8DeviceNr    Chip select number
 ** \param [in]  pstcHsSpi     HS-SPI instance pointer
 **
 ** \return Ok                 External Flash not Busy (No embedded operation in 
 **                            progress)
 ** \return Error              External Flash Status Register 1 Read Failure
 *****************************************************************************/
static en_result_t m_FlashS25FL164K_WaitWipOff(uint8_t u8DeviceNr, volatile stc_hsspin_t* pstcHsSpi)
{
    uint8_t u8TxBuff;
    uint8_t u8RxBuff;
    en_result_t enRetVal;

    do
    {
        PDL_WAIT_LOOP_HOOK();

        u8TxBuff = 0x05u;           // Read Status Register 1 ...
        enRetVal = HsSpi_DirectModeTransferHalfDuplex(pstcHsSpi, u8DeviceNr, &u8TxBuff, 1u, &u8RxBuff, 1u, FALSE);
        if (enRetVal != Ok)
        {
            return Error;
        }
        #ifdef DEBUG
        if (u8RxBuff & 0x01u)
        {
            u32WipActiveCounter++;
        }
        #endif
    } while (u8RxBuff & 0x01u);      // ... until BUSY is reset

    return Ok;
} // m_FlashS25FL164K_WaitWipOff

/*****************************************************************************
 ** \brief Setup Quad-IO SPI Mode for Spansion S25FL164K Serial Flash
 **
 ** \param [in]  u8DeviceNr  Chip select number
 ** \param [in]  pstcHsSpi   HS-SPI instance pointer
 ** \param [in]  bSwitchOn   True  => enable Quad operation
 **                          False => disable Quad operation
 **
 ** \return Ok               Quad-IO SPI mode enable successfull 
 ** \return Error            Quad-IO SPI mode could not enabled due to External
 **                          Flash Status Registers Read/Write Failure 
 *****************************************************************************/
en_result_t FlashS25FL164K_SetQIO(uint8_t u8DeviceNr, volatile stc_hsspin_t* pstcHsSpi, boolean_t bSwitchOn)
{
    en_result_t enRetVal;
    uint8_t au8TxBuff[4u];
    uint8_t au8RxBuff[1u];

    au8TxBuff[0u] = 0x35u;                  // Read Status Register 2 Command
    enRetVal = HsSpi_DirectModeTransferHalfDuplex(pstcHsSpi, u8DeviceNr, au8TxBuff, 1u, au8RxBuff, 1u, FALSE);
    if (enRetVal != Ok)
    {
        return Error;
    }
    
    au8TxBuff[0u] = 0x06u;                  // WREN Write Enable Command
    enRetVal = HsSpi_DirectModeTransferHalfDuplex(pstcHsSpi, u8DeviceNr, au8TxBuff, 1u, NULL, 0u, FALSE);
    if (enRetVal != Ok)
    {
        return Error;
    }

    au8TxBuff[0u] = 0x01u;                  // Write Status Register 1-3 Command
    au8TxBuff[1u] = 0x00u;                  // Status Register 1 (0x00 = default)
    if (bSwitchOn == TRUE)
    { 
        au8TxBuff[2u] = 0x02u;              // Status Register 2 (0x02 = Quad Enable)
    }
    else
    {
        au8TxBuff[2u] = 0x00u;              // Status Register 2 (0x00 = Clear Quad Enable)
    }
    
    au8TxBuff[3u] = 0x70u;                  // Status Register 3 (0x70 = default)
                                                                                
    enRetVal = HsSpi_DirectModeTransferHalfDuplex(pstcHsSpi, u8DeviceNr, au8TxBuff, 4u, NULL, 0u, FALSE);
    if (enRetVal != Ok)
    {
        return Error;
    }

    // Wait until WIP has been reset by the chip
    enRetVal = m_FlashS25FL164K_WaitWipOff(u8DeviceNr, pstcHsSpi);
    if (enRetVal != Ok)
    {
        return Error;
    }
    
    au8TxBuff[0u] = 0x35u;                  // Read Status Register 2 Command
    enRetVal = HsSpi_DirectModeTransferHalfDuplex(pstcHsSpi, u8DeviceNr, au8TxBuff, 1u, au8RxBuff, 1u, FALSE);
    if (enRetVal != Ok)
    {
        return Error;
    }

    if (TRUE == bSwitchOn)
    {
        if ((au8RxBuff[0x00u] & 0x02u) != 0x02u)                                // Quad Enable (QE) not set?
        {
            return Error;
        }
        m_bQioModeEnabled = TRUE;
    }
    else
    {
        if ((au8RxBuff[0x00u] & 0x02u) != 0x00u)                                // Quad Enable (QE) not cleared?
        {
            return Error;
        }
        m_bQioModeEnabled = FALSE;
    }

    return enRetVal;
} // FlashS25FL164K_SetQIO

/*****************************************************************************
 ** \brief Read ID of Spansion S25FL164K Serial Flash
 **
 ** \param [in] u8DeviceNr           Slave which is connected to corresponding
 **                                  signals SSEL0..3
 ** \param [in] pstcHsSpi            HS-SPI module instance
 **                                  (register start address of HS-SPI module).
 ** \param [in] u8IdLength           Length(in bytes) of the ID.
 ** \param [out] pu8IdData           Pointer to store the ID read from flash.
 **
 ** \retval Ok                       ID Read successfull
 ** \retval ErrorInvalidParameter    If one of following conditions are met:    
 **             - pstcHsSpi == NULL
 **             - u8DeviceNumber > 3
 **             - pu8IdData == NULL
 **             - u8IdLength == 0   
 **             - u8IdLength >= 8
 *****************************************************************************/
en_result_t FlashS25FL164K_ReadID(uint8_t u8DeviceNr, volatile stc_hsspin_t* pstcHsSpi,
                                  uint8_t* pu8IdData, uint8_t u8IdLength)
{
    uint8_t au8TxBuff[8u];
 
    if (NULL == pu8IdData ||
         0u == u8IdLength
      )
    {
        return ErrorInvalidParameter;
    }

    if (u8IdLength >= sizeof(au8TxBuff))
    {
        return ErrorInvalidParameter;
    }

    memset(au8TxBuff, 0xFFu, sizeof(au8TxBuff));
    au8TxBuff[0u] = 0x9Fu;                       // Read ID Command (Device OPN)

    enResult = HsSpi_DirectModeTransferHalfDuplex(pstcHsSpi, u8DeviceNr, au8TxBuff, 1u, pu8IdData, u8IdLength, FALSE);

    return enResult;
} // FlashS25FL164K_ReadID

/*****************************************************************************
 ** \brief Erase Spansion S25FL164K Serial Flash
 **
 ** \param [in] u8DeviceNr            Slave which is connected to corresponding
 **                                   signals SSEL0..3
 ** \param [in] pstcHsSpi             HS-SPI module instance
 **                                   (register start address of HS-SPI module).
 ** \param [in] u32Address            24-bit block address of the serial flash 
 **                                   sector to be erased
 ** \param [in] bBulkErase            Flag to indicate the type of erase
 **                                   FALSE => Sector erase
 **                                   TRUE  => Chip erase
 **
 ** \retval Ok                        Erase successfull  
 ** \retval Error                     Erase failed  
 *****************************************************************************/
en_result_t FlashS25FL164K_Erase(uint8_t u8DeviceNr, volatile stc_hsspin_t* pstcHsSpi,
                                 uint32_t u32Address, boolean_t bBulkErase)
{
    uint8_t au8Buff[4u];

    au8Buff[0u] = 0x06u;                          // Write Enable Command
    enResult = HsSpi_DirectModeTransferHalfDuplex(pstcHsSpi, u8DeviceNr, au8Buff, 1u, NULL, 0u, FALSE);
    if (enResult != Ok)
    {
      return Error;
    }

    if (TRUE == bBulkErase)
    {
        au8Buff[0u] = 0x60u;                      // Chip Erase Command
        enResult = HsSpi_DirectModeTransferHalfDuplex(pstcHsSpi, u8DeviceNr, au8Buff, 1u, NULL, 0u, FALSE);
    }
    else
    {
        au8Buff[0u] = 0xD8u;                      // 64kB Block Erase (works on 4/8KB sectors too)
        au8Buff[1u] = (u32Address & 0x00FF0000ul) >> 16ul;                      // 24-bit Address
        au8Buff[2u] = (u32Address & 0x0000FF00ul) >>  8ul;
        au8Buff[3u] = (u32Address & 0x000000FFul) >>  0ul;
        enResult = HsSpi_DirectModeTransferHalfDuplex(pstcHsSpi, u8DeviceNr, au8Buff, 4u, NULL, 0u, FALSE);
    }

    if (enResult != Ok)
    {
      return Error;
    }

    // Wait until BUSY released (Chip erase finished)
    enResult = m_FlashS25FL164K_WaitWipOff(u8DeviceNr, pstcHsSpi);
    if (enResult != Ok)
    {
      return Error;
    }

    au8Buff[0u] = 0x04u;                           // Write Disable Command
    enResult = HsSpi_DirectModeTransferHalfDuplex(pstcHsSpi, u8DeviceNr, au8Buff, 1u, NULL, 0u, FALSE);
    if (enResult != Ok)
    {
      return Error;
    }
    
    return enResult;
} // FlashS25FL164K_Erase

/*****************************************************************************
 ** \brief SPI write support for Spansion S25FL164K Serial Flash
 **
 ** \param [in] u8DeviceNr            Slave which is connected to corresponding
 **                                   signals SSEL0..3
 ** \param [in] pstcHsSpi             HS-SPI module instance
 **                                   (register start address of HS-SPI module).
 ** \param [in] pu8Data               Pointer to the data which will be written 
 **                                   to Serial flash.
 ** \param [in] u32Address            24-bit address of serial flash where the 
 **                                   data will be written.
 ** \param [in] u32NumData            Number of bytes to write to serial Flash
 **
 ** \retval Ok                        Write success  
 ** \retval Error                     Write failed
 *****************************************************************************/
en_result_t FlashS25FL164K_Write(uint8_t u8DeviceNr, volatile stc_hsspin_t* pstcHsSpi,
                                 uint8_t* pu8Data, uint32_t u32Address, uint32_t u32NumData)
{
    uint8_t au8Buff[4u];

    // Set QUAD IO mode
    if (FALSE == m_bQioModeEnabled)
    {
        // Set QUAD IO mode
        enResult = FlashS25FL164K_SetQIO(u8DeviceNr, pstcHsSpi, TRUE);
        if (enResult != Ok)
        {
          return Error;
        }
    }

    au8Buff[0u] = 0x06u;                                                        // Write Enable Command
    enResult = HsSpi_DirectModeTransferHalfDuplex(pstcHsSpi, u8DeviceNr, au8Buff, 1u, NULL, 0u, FALSE);
    if (enResult != Ok)
    {
      return Error;
    }

    // Write some data
    au8Buff[0u] = 0x02u;                                                        // Page Program Command
    au8Buff[1u] = (uint8_t)((u32Address & 0x00FF0000ul) >> 16ul);               // 24-bit Address
    au8Buff[2u] = (uint8_t)((u32Address & 0x0000FF00ul) >>  8ul);
    au8Buff[3u] = (uint8_t)((u32Address & 0x000000FFul) >>  0ul);
    enResult = HsSpi_MultiModeTransfer( pstcHsSpi,                              // SPI instance
                                        u8DeviceNr,                             // Device #
                                        au8Buff,                                // (Instruction + Address)
                                        4u,                                     // # of legacy data
                                        pu8Data,                                // Data to be written to Flash
                                        u32NumData,                             // # of data to be written
                                        NULL,                                   // Read data
                                        0u                                      // # of data to read(SHOULD BE 0 for WRITE OPN)
                                      );
    if (enResult != Ok)
    {
      return Error;
    }

    // Wait until BUSY released (Page Program finished)
    enResult = m_FlashS25FL164K_WaitWipOff(u8DeviceNr, pstcHsSpi);
    if (enResult != Ok)
    {
      return Error;
    }
    
    au8Buff[0u] = 0x04u;                                                        // Write Disable Command
    enResult = HsSpi_DirectModeTransferHalfDuplex(pstcHsSpi, u8DeviceNr, au8Buff, 1u, NULL, 0u, FALSE);
    if (enResult != Ok)
    {
      return Error;
    }
    
    return enResult;
} // FlashS25FL164K_Write

/*****************************************************************************
 ** \brief SPI read support for Spansion S25FL164K Serial Flash
 **
 ** \param [in] u8DeviceNr            Slave which is connected to corresponding
 **                                   signals SSEL0..3
 ** \param [in] pstcHsSpi             HS-SPI module instance
 **                                   (register start address of HS-SPI module).
 ** \param [in] u32Address            24-bit address starting from which the data
 **                                   will be read from Serial Flash.
 ** \param [in] u32NumData            Number of bytes to be read from Serial Flash
 ** \param [out] pu8Data              Pointer to store the data read from Serial flash.
 **
 ** \retval Ok                        Read success  
 ** \retval Error                     Read failed  
 *****************************************************************************/
en_result_t FlashS25FL164K_Read(uint8_t u8DeviceNr, volatile stc_hsspin_t* pstcHsSpi,
                                uint8_t* pu8Data, uint32_t u32Address, uint32_t u32NumData)
{
    uint8_t au8Buff[16u];

    // Set QUAD IO mode
    if (FALSE == m_bQioModeEnabled)
    {
        // Set QUAD IO mode
        enResult = FlashS25FL164K_SetQIO(u8DeviceNr, pstcHsSpi, TRUE);
        if (enResult != Ok)
        {
          return Error;
        }
    }

    // Read back written data
    au8Buff[0u] = 0xEBu;                                                        // Fast Read Quad I/O Command
    au8Buff[1u] = (uint8_t)((u32Address & 0x00FF0000ul) >> 16ul);               // 24-bit Address
    au8Buff[2u] = (uint8_t)((u32Address & 0x0000FF00ul) >>  8ul);
    au8Buff[3u] = (uint8_t)((u32Address & 0x000000FFul) >>  0ul);
    au8Buff[4u] = 0x00u;                                                        // Mode bits (will release QIO after CS# raised)
    au8Buff[5u] = 0xFFu;                                                        // Dummy  
    au8Buff[6u] = 0xFFu;                                                        // Dummy  

    // Update the count of legacy data and QIO data depending on the Read Mode
    enResult = HsSpi_MultiModeTransfer( pstcHsSpi,                              // SPI instance
                                        u8DeviceNr,                             // Device #
                                        au8Buff,                                // Legacy data
                                        1u,                                     // # of legacy data
                                        &au8Buff[1u],                           // QIO data
                                        6u,                                     // # of QIO data
                                        pu8Data,                                // Read data
                                        u32NumData                              // # of data to read
                                      );
    
    if (enResult != Ok)
    {
      return Error;
    }

    return enResult;
} // FlashS25FL164K_Read

/*****************************************************************************
 ** \brief Init Command Sequencer mode for S25FL164K Flash
 **
 ** \param [in]  u8DeviceNr  Chip select number
 ** \param [in]  pstcHsSpi   HS-SPI instance pointer
 ** \param [in]  bSwitchOn   True => Command sequencer mode
                             False => Direct mode
 **
 ** \return Ok               Command sequencer mode enabled
 ** \return Error            Error in enabling Command Sequencer                        
 *****************************************************************************/
en_result_t FlashS25FL164K_EnableCommandSequencer(uint8_t u8DeviceNr,
                                                  volatile stc_hsspin_t* pstcHsSpi,
                                                  boolean_t bSwitchOn)
{
    en_result_t enRetVal;
    uint8_t au8Buff[9u];
    
    stc_hsspi_command_sequencer_control_t stcReadCommandSequence[] =
    {
        { HsSpiCmdTransmitAddress16To23, 0x00u, FALSE, HsSpiQuadBitMode }, // 3 Address Cycles
        { HsSpiCmdTransmitAddress08To15, 0x00u, FALSE, HsSpiQuadBitMode }, // ...
        { HsSpiCmdTransmitAddress07To00, 0x00u, FALSE, HsSpiQuadBitMode }, // ...
        { HsSpiCmdTransmitData,          0xA0u, FALSE, HsSpiQuadBitMode }, // Mode byte Ax: stay in continous HS mode
        { HsSpiCmdTransmitHighZStateByte,0x00u, FALSE, HsSpiQuadBitMode }, // 2 Dummy Clocks
        { HsSpiCmdTransmitHighZStateByte,0x00u, FALSE, HsSpiQuadBitMode }, // 2 Dummy Clocks
        { HsSpiCmdEndOfList,             0x00u, },
    };
    
    stc_hsspi_command_sequencer_control_t stcWriteCommandSequence[] =
    {
        { HsSpiCmdEndOfList, 0x00u }
    };

    // Check for NULL-Pointer
    if (NULL  == pstcHsSpi)
    {
      return Error;
    }

    if (FALSE == bSwitchOn)
    {
        // Switch back to direct mode
        enRetVal = HsSpi_SetMode(pstcHsSpi, HsSpiModeDirect, HsSpiProtocolModeQuad, HsSpiProtocolTxRx);
        return enRetVal;
    }

    // Set QUAD IO mode
    if (FALSE == m_bQioModeEnabled)
    {
        // Set QUAD IO mode
        enRetVal = FlashS25FL164K_SetQIO(u8DeviceNr, pstcHsSpi, TRUE);
        if (enRetVal != Ok)
        {
          return Error;
        }
    }

    // Write some data
    au8Buff[0u] = 0xEBu;        // Fast Read Quad I/O Command
    au8Buff[1u] = 0x00u;        // 24 address bits
    au8Buff[2u] = 0x00u;
    au8Buff[3u] = 0x00u;        
    au8Buff[4u] = 0xA0u;        // Continuous Read mode
    au8Buff[5u] = 0x00u;        // 2 dummy cycles for latency
    au8Buff[6u] = 0x00u;        // 2 dummy cycles for latency
    enRetVal = HsSpi_MultiModeTransfer( pstcHsSpi,        // SPI instance
                                        u8DeviceNr,       // Device #
                                        au8Buff,          // Legacy data
                                        1u,               // # of legacy data
                                        &au8Buff[1u],     // QIO data
                                        6u,               // # of QIO data
                                        &au8Buff[7u],     // Read data
                                        2u                // # of data to read
                                       );

    // Switch to command sequencer mode
    enRetVal = HsSpi_SetMode(pstcHsSpi, HsSpiModeCmdSequencer, HsSpiProtocolModeQuad, HsSpiProtocolTxRx);
    if (enRetVal != Ok)
    {
      return Error;
    }

    enRetVal = HsSpi_SetReadCommandSequence(pstcHsSpi, stcReadCommandSequence);
    if (enRetVal != Ok)
    {
      return Error;
    }

    enRetVal = HsSpi_SetWriteCommandSequence(pstcHsSpi, stcWriteCommandSequence);
    if (enRetVal != Ok)
    {
      return Error;
    }
    
    return enRetVal;
} // FlashS25FL164K_EnableCommandSequencer

/*****************************************************************************
 ** \brief Read Unique ID (Last 8 bytes of 256-byte SFDP register)
 **
 ** \param [in]  u8DeviceNr  Chip select number
 ** \param [in]  pstcHsSpi   HS-SPI instance pointer
 ** \param [out] pu8Id       Read out Unique ID
 **
 ** \return Ok               Read ID success
 ** \return Error            Read ID failed   
 *****************************************************************************/
en_result_t FlashS25FL164K_ReadUniqueId(uint8_t u8DeviceNr,
                                        volatile stc_hsspin_t* pstcHsSpi,
                                        uint8_t* pu8Id)
{
    en_result_t enRetVal;
    uint8_t au8TxBuff[5u];
       
    au8TxBuff[0u] = 0x5Au;      // Read SFDP command
    au8TxBuff[1u] = 0x00u;      // Address 23-16
    au8TxBuff[2u] = 0x00u;      // Address 15-8
    au8TxBuff[3u] = 0xF8u;      // Address 7-0
    au8TxBuff[4u] = 0x00u;      // 8 dummy clocks
    
    enRetVal = HsSpi_DirectModeTransferHalfDuplex(pstcHsSpi, u8DeviceNr, au8TxBuff, 5u, pu8Id, 8u, FALSE);
    if (enRetVal != Ok)
    {
       return Error;
    }
    
    return enResult;
} // FlashS25FL164K_ReadUniqueId

#endif

/*****************************************************************************/
/* EOF (not truncated)                                                       */
/*****************************************************************************/
