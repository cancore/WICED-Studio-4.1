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
/** \file i2c_at24cxx.h
 **
 ** AT24CXX driver file header file
 **
 ** The AT24C02 is tested in the example, make following pin connection first.
 **
 ** --------------------------------------------
 ** | MCU             AT24C02                   |
 ** |-------------------------------------------
 ** | Pin Name        Pin Count     Pin Name    |   
 ** | GND             1             A0          |
 ** | GND             2             A1          |   
 ** | GND             3             A2          |
 ** | GND             4             GND         |
 ** | SOT1_1          5             SDA         |
 ** | SCK1_1          6             SCL         |
 ** | -               7             WP          |
 ** | VCC             8             VCC         |
 ** --------------------------------------------
 **
 ** History:
 **   - 2014-03-10  1.0  Edison Zhang        First release version.
 **
 ******************************************************************************/
#ifndef _I2C_AT24CXX_H_
#define _I2C_PAT24CXX_H_

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl_user.h"
#include "at24cxx_config.h"
#include "mfs/mfs.h"
#include "gpio/gpio.h"

#if (PDL_UTILITY_ENABLE_I2C_POLLING_AT24CXX == PDL_ON) || \
    (PDL_UTILITY_ENABLE_I2C_IRQ_AT24CXX == PDL_ON)

/**
 ******************************************************************************
 ** \defgroup I2cAt24cxxGroup I2C AT24CXX Function
 **
 ** Provided functions of AT24CXX driver:
 **
 ** - At24cxx_Init()
 ** - At24cxx_ByteWrite()
 ** - At24cxx_PageWrite()
 ** - At24cxx_CurrentAddrRead()
 ** - At24cxx_RandomRead()
 ** - At24cxx_SequentialRead()
 ** - At24cxx_Delayms()     
 **
 ** At24cxx_Init() initializes the I2C channel to access AT24CXX.
 **
 ** At24cxx_ByteWrite() writes a byte at a ramdon address in the EEPROM memory.
 **
 ** At24cxx_PageWrite() can write up to 1 page bytes. 
 **
 ** At24cxx_CurrentAddrRead() reads the data at the current address.
 **
 ** At24cxx_RandomRead() can read a byte at a ramdon address in the EEPROM 
 ** memory.
 **
 ** At24cxx_SequentialRead() can read as many bytes as whole EEPROM memory,
 ** which should follow At24cxx_CurrentAddrRead() or At24cxx_RandomRead().      
 **
 ******************************************************************************/ 

//@{      
      
/******************************************************************************
 * Global definitions
 ******************************************************************************/
/** AT24CXX device address [A0=A1=A2=0] */
#define AT24CXX_7BIT_DEVICE_ADDR        (0x50u)                     
   
/** AT24CXX page size    */
#if (PDL_EEPROM_DEVICE == PDL_AT24C01) || \
    (PDL_EEPROM_DEVICE == PDL_AT24C02)
#define AT24CXX_PAGE_SIZE                      8
#elif (PDL_EEPROM_DEVICE == PDL_AT24C04) || \
      (PDL_EEPROM_DEVICE == PDL_AT24C08) || \
      (PDL_EEPROM_DEVICE == PDL_AT24C16)
#define AT24CXX_PAGE_SIZE                      16
#elif (PDL_EEPROM_DEVICE == PDL_AT24C32) || \
      (PDL_EEPROM_DEVICE == PDL_AT24C64)
#define AT24CXX_PAGE_SIZE                      32
#endif

/** AT24CXX address byte length  */        
#if (PDL_EEPROM_DEVICE == PDL_AT24C01) || \
    (PDL_EEPROM_DEVICE == PDL_AT24C02) || \
    (PDL_EEPROM_DEVICE == PDL_AT24C04) || \
    (PDL_EEPROM_DEVICE == PDL_AT24C08) || \
    (PDL_EEPROM_DEVICE == PDL_AT24C16) 
#define AT24CXX_ADDR_LENGTH                    1
#else
#define AT24CXX_ADDR_LENGTH                    2
#endif      
        
/** I2C IO function initialization */       
#define InitI2cIo(void)  {SetPinFunc_SOT1_1();SetPinFunc_SCK1_1();}

/** I2C channel used for AT24C02   */
#define EE_I2C_CH        &I2C1  
      
/*---------------------------------------------------------------------------*/
/* function prototypes                                                       */
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
en_result_t At24cxx_Init(void); 
en_result_t At24cxx_ByteWrite(uint8_t u8DevAddr, uint16_t u16Addr, uint8_t u8Data);
en_result_t At24cxx_PageWrite(uint8_t u8DevAddr, uint16_t u16PageAddr, uint8_t* pu8Data, uint8_t u8Size);
en_result_t At24cxx_CurrentAddrRead(uint8_t u8DevAddr, uint8_t* pu8CurData);
en_result_t At24cxx_RandomRead(uint8_t u8DevAddr, uint16_t u16Addr, uint8_t* pu8Data);
en_result_t At24cxx_SequentialRead(uint8_t u8DevAddr, uint8_t* pu8Data, uint8_t u8Size);
void At24cxx_Delayms(uint32_t u32Cnt);
#ifdef __cplusplus
}
#endif

//@} 

#endif // #if (PDL_UTILITY_ENABLE_I2C_POLLING_AT24CXX == PDL_ON)

#endif //  _I2C_AT24CXX_H_

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
