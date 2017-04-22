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
/** \file at24cxx_config.h
 **
 ** EEPROM configuration file
 **
 ** History:
 **   - 2014-03-10  1.0  Edison Zhang        First release version.
 **
 ******************************************************************************/
#ifndef _AT24CXX_CONFIG_H_
#define _AT24CXX_CONFIG_H_

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

/**
 ******************************************************************************
 ** EEPROM Device List
 ******************************************************************************/
#define PDL_EEPROM_AT24C01           0
#define PDL_EEPROM_AT24C02           1
#define PDL_EEPROM_AT24C04           2
#define PDL_EEPROM_AT24C08           3
#define PDL_EEPROM_AT24C16           4
#define PDL_EEPROM_AT24C32           5
#define PDL_EEPROM_AT24C64           6

/**
 ******************************************************************************
 ** EEPROM device selection [User setting]
 ******************************************************************************/
#define PDL_EEPROM_DEVICE     PDL_AT24C02


#endif // _AT24CXX_CONFIG_H_
