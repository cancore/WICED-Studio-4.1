/*
 * Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */
#pragma once

#include "platform_sleep.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/* UART port used for standard I/O */
#ifdef PLATFORM_WL_UART_ENABLED
#define STDIO_UART  ( WICED_UART_3 )
#else
#define STDIO_UART  ( WICED_UART_1 )
#endif /* PLATFORM_WL_UART_ENABLED */

/******************************************************
 *                   Enumerations
 ******************************************************/

/*
BCM943903PS platform pin definitions ...
+-------------------------------------------------------------------------------------------------------------------+
| Enum ID       |Pin  |   Pin Name on    |  Module                           |  Conn  |  Peripheral  |  Peripheral  |
|               | #   |   43903WLBU_1    |  Alias                            |  Pin   |  Available   |    Alias     |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_1  | E1  | GPIO_0           | GPIO_0                            |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_2  | D1  | GPIO_1           | GPIO_1                            |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_3  | E5  | GPIO_7           | GPIO_7_WCPU_BOOT_MODE             |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_4  | D9  | GPIO_8           | GPIO_8_TAP_SEL                    |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_5  | E2  | GPIO_9           | GPIO_9                            |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_6  | C1  | GPIO_10          | GPIO_10                           |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_7  | B1  | GPIO_11          | GPIO_11_ACPU_BOOT_MODE            |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_8  | C6  | GPIO_12          | GPIO_12                           |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_9  | E8  | GPIO_13          | GPIO_13                           |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_10 | A3  | GPIO_14          | GPIO_14                           |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_11 | A2  | GPIO_15          | GPIO_15                           |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_12 | C2  | GPIO_16          | GPIO_16                           |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_13 | C5  | SPI_0_MISO       | SPI0_MISO                         |    -   |  SPI, GPIO   | GPIO_24      |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_14 | B5  | SPI_0_CLK        | SPI0_CLK                          |    -   |  SPI, GPIO   | GPIO_25      |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_15 | B6  | SPI_0_MOSI       | SPI0_SISO                         |    -   |  SPI, GPIO   | GPIO_26      |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_16 | E6  | SPI_0_CS         | SPI0_CS                           |    -   |  SPI, GPIO   | GPIO_27      |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_17 | G10 | GPIO_2           | GPIO_2_JTAG_TCK                   |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_18 | G12 | GPIO_3           | GPIO_3_JTAG_TMS                   |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_19 | F10 | GPIO_4           | GPIO_4_JTAG_TDI                   |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_20 | F11 | GPIO_5           | GPIO_5_JTAG_TDO                   |    -   |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_21 | F12 | GPIO_6           | GPIO_6_JTAG_TRST_L                |    -   |  GPIO        |              |
+-------------------------------------------------------------------------------------------------------------------+
| WICED_GPIO_22 | B9  | I2C_0_SDA        | I2C_0_SDA                         |    -   |  I2C, GPIO   | GPIO_28      |
+-------------------------------------------------------------------------------------------------------------------+
| WICED_GPIO_23 | C8  | I2C_0_SCL        | I2C_0_SCL                         |    -   |  I2C, GPIO   | GPIO_29      |
+-------------------------------------------------------------------------------------------------------------------+

+--------------------------------------------------------------------------------------------------------------------------------+
| Enum ID                 |Pin  |   Pin Name on |    Board                                 |  Conn  |  Peripheral |  Peripheral  |
|                         | #   |      43909    |  Connection                              |  Pin   |  Available  |    Alias     |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_1  | K10 | RF_SW_CTRL_6  | RF_SW_CTRL_6_UART1_RX_IN                 |  -     |  UART1      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_2  | L12 | RF_SW_CTRL_7  | RF_SW_CTRL_7_RSRC_INIT_MODE_UART1_TX_OUT |  -     |  UART1      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_3  | C7  | UART0_RXD     | UART0_RXD_IN                             |  -     |  UART2      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_4  | B8  | UART0_TXD     | UART0_TXD_OUT                            |  -     |  UART2      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_5  | A7  | UART0_CTS     | UART0_CTS_IN                             |  -     |  UART2      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_6  | B7  | UART0_RTS     | UATR0_CTS_OUT                            |  -     |  UART2      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_7  | M12 | RF_SW_CTRL_8  | RF_SW_CTRL_8_BT_SECI_IN                  |  -     |  UART3      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_8  | N11 | RF_SW_CTRL_9  | RF_SW_CTRL_9_HIB_LPO_SEL_BT_SECI_OUT     |  -     |  UART3      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------+
*/

typedef enum
{
    WICED_GPIO_1,
    WICED_GPIO_2,
    WICED_GPIO_3,
    WICED_GPIO_4,
    WICED_GPIO_5,
    WICED_GPIO_6,
    WICED_GPIO_7,
    WICED_GPIO_8,
    WICED_GPIO_9,
    WICED_GPIO_10,
    WICED_GPIO_11,
    WICED_GPIO_12,
    WICED_GPIO_13,
    WICED_GPIO_14,
    WICED_GPIO_15,
    WICED_GPIO_16,
    WICED_GPIO_17,
    WICED_GPIO_18,
    WICED_GPIO_19,
    WICED_GPIO_20,
    WICED_GPIO_21,
    WICED_GPIO_22,
    WICED_GPIO_23,
    WICED_GPIO_MAX, /* Denotes the total number of GPIO port aliases. Not a valid GPIO alias */
} wiced_gpio_t;

typedef enum
{
    WICED_PERIPHERAL_PIN_1 = WICED_GPIO_MAX,
    WICED_PERIPHERAL_PIN_2,
    WICED_PERIPHERAL_PIN_3,
    WICED_PERIPHERAL_PIN_4,
    WICED_PERIPHERAL_PIN_5,
    WICED_PERIPHERAL_PIN_6,
    WICED_PERIPHERAL_PIN_7,
    WICED_PERIPHERAL_PIN_8,
    WICED_PERIPHERAL_PIN_MAX, /* Denotes the total number of GPIO and peripheral pins. Not a valid pin alias */
} wiced_peripheral_pin_t;

typedef enum
{
    WICED_SPI_1,
    WICED_SPI_MAX, /* Denotes the total number of SPI port aliases. Not a valid SPI alias */
} wiced_spi_t;

typedef enum
{
    WICED_I2C_1,
    WICED_I2C_MAX, /* Denotes the total number of I2C port aliases. Not a valid I2C alias */
} wiced_i2c_t;

typedef enum
{
    WICED_PWM_MAX, /* Denotes the total number of PWM port aliases. Not a valid PWM alias */
} wiced_pwm_t;

typedef enum
{
   WICED_ADC_NONE = 0xff, /* No ADCs */
} wiced_adc_t;


typedef enum
{
    WICED_UART_1,   /* ChipCommon Slow UART */
    WICED_UART_2,   /* ChipCommon Fast UART */
    WICED_UART_3,   /* GCI UART */
    WICED_UART_MAX, /* Denotes the total number of UART port aliases. Not a valid UART alias */
} wiced_uart_t;

typedef enum
{
    WICED_I2S_MAX, /* Denotes the total number of I2S port aliases. Not a valid I2S alias */
} wiced_i2s_t;

/* Logical Button-ids which map to physical buttons on the board */
typedef enum
{
    PLATFORM_BUTTON_1,
    PLATFORM_BUTTON_2,
    PLATFORM_BUTTON_MAX, /* Denotes the total number of Buttons on the board. Not a valid Button Alias */
} platform_button_t;

/* Components connected to external I/Os */
#define PLATFORM_LED_COUNT               ( 2 )
#define WICED_LED1         ( WICED_GPIO_8 )
#define WICED_LED2         ( WICED_GPIO_12 )
#define WICED_BUTTON1      ( WICED_GPIO_5 )
#define WICED_BUTTON2      ( WICED_GPIO_6 )

#define WICED_PLATFORM_BUTTON_COUNT  ( 2 )

#define AUTH_IC_I2C_PORT (WICED_I2C_1)
#define WICED_GPIO_AUTH_RST  ( WICED_GPIO_9 )

/* no audio on this platform */
#define PLATFORM_DEFAULT_AUDIO_INPUT    AUDIO_DEVICE_ID_NONE
#define PLATFORM_AUDIO_NUM_INPUTS       (0)
#define PLATFORM_DEFAULT_AUDIO_OUTPUT   AUDIO_DEVICE_ID_OUTPUT_NONE
#define PLATFORM_AUDIO_NUM_OUTPUTS      (0)

#ifdef __cplusplus
} /*extern "C" */
#endif
