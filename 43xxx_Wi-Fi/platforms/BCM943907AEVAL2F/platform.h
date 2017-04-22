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
BCM943907AEVAL2F platform pin definitions ...
+-------------------------------------------------------------------------------------------------------+--------------+-------------------------------|
| Enum ID       |Pin  |   Pin Name   |  SIP Pin  Name          | SIP |  Module Pin              | Module| Function     | Board Connection              |
|               | #   |   on 43907   |                         | Pin#|  Name                    | Pin#  |              |                               |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_1  | 141 | GPIO_0       | GPIO_0                  | C7  | GPIO_0                   | J10:1 |  GPIO        | ARD_GPIO0                     |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_2  | 142 | GPIO_1       | GPIO_1_GSPI_MODE        | A34 | GPIO_1_GSPI_MODE         | J10:2 |  GPIO        | ARD_GPIO1                     |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_3  | 95  | GPIO_7       | GPIO_7_WCPU_BOOT_MODE   | B10 | GPIO_7_WCPU_BOOT_MODE    | J10:4 |  GPIO        | ARD_GPIO3                     |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_4  | 134 | GPIO_8       | GPIO_8_TAP_SEL          | B12 | GPIO_8_TAP_SEL           | J12:5 |  GPIO        | ARD_MISO                      |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_5  | 94  | GPIO_9       | GPIO_9_USB_SEL          | A15 | GPIO_9_USB_EN            | J12:6 |  GPIO        | ARD_SCK                       |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_6  | 146 | GPIO_10      | GPIO_10                 | D8  | GPIO_10                  | J12:4 |  GPIO        | ARD_MOSI                      |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_7  | 140 | GPIO_11      | GPIO_11_ACPU_BOOT_MODE  | A21 | GPIO_11_ACPU_BOOT_MODE   | J12:3 |  GPIO        | ARD_SS                        |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_8  | 143 | GPIO_12      | GPIO_12                 | C17 | GPIO_12                  | J12:2 |  GPIO        | ARD_GPIO9                     |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_9  | 131 | GPIO_13      | GPIO_13_SDIO_MODE       | B34 | GPIO_13_SDIO_MODE        | J10:3 |  GPIO        | ARD_GPIO2                     |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_10 | 130 | GPIO_14      | GPIO_14                 | B35 | GPIO_14                  | J10:5 |  GPIO        | ARD_GPIO4                     |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_11 | 145 | GPIO_15      | GPIO_15                 | B30 | GPIO_15                  | J10:7 |  GPIO        | ARD_GPIO6                     |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_12 | 137 | GPIO_16      | GPIO_16                 | B19 | GPIO_16                  | J10:6 |  GPIO        | ARD_GPIO5                     |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_13 | 185 | PWM0         | PWM_0                   | C13 | PWM_0                    | xxxx  |  PWM, GPIO   | uSD Connector/Slot Detect     |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_14 | 186 | PWM1         | PWM_1                   | B15 | PWM_1                    | xxxx  |  PWM, GPIO   | Auth Chip Reset               |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_15 | 190 | PWM2         | PWM_2                   | A30 | PWM_2                    | TP6   |  PWM, GPIO   |                               |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_16 | 189 | PWM3         | PWM_3                   | A29 | PWM_3                    | TP4   |  PWM, GPIO   |                               |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_17 | 187 | PWM4         | PWM_4                   | A35 | PWM_4                    | J6:1  |  PWM, GPIO   | WCD_PWM_4                     |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_18 | 188 | PWM5         | PWM_5                   | D7  | PWM_5                    | J6:2  |  PWM, GPIO   | WCD_PWM_5                     |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_19 | 78  | SPI0_MISO    | SPI_0_MISO              | A11 | SPI_0_MISO               | J6:17 |  SPI, GPIO   | WCD_SPI_0_MISO                |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_20 | 76  | SPI0_CLK     | SPI_0_CLK               | A12 | SPI_0_CLK                | J6:12 |  SPI, GPIO   | WCD_SPI_0_CLK                 |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_21 | 81  | SPI0_SISO    | SPI_0_MOSI              | A14 | SPI_0_MOSI               | J6:14 |  SPI, GPIO   | WCD_SPI_0_MOSI                |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_22 | 82  | SPI0_CS      | SPI_0_CS                | A13 | SPI_0_CS                 | J6:16 |  SPI, GPIO   | WCD_SPI_0_CS                  |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_23 | 139 | GPIO_2       | GPIO_2_JTAG_TCK         | C16 | GPIO_2_JTAG_TCK          | TP11  |  GPIO        | DEBUGGER                      |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_24 | 144 | GPIO_3       | GPIO_3_JTAG_TMS         | D6  | GPIO_3_JTAG_TMS          | TP10  |  GPIO        | DEBUGGER                      |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_25 | 136 | GPIO_4       | GPIO_4_JTAG_TDI         | C6  | GPIO_4_JTAG_TDI          | TP8   |  GPIO        | DEBUGGER                      |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_26 | 132 | GPIO_5       | GPIO_5_JTAG_TDO         | B9  | GPIO_5_JTAG_TDO          | TP9   |  GPIO        | DEBUGGER                      |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_27 | 133 | GPIO_6       | GPIO_6_JTAG_TRST        | C5  | GPIO_6_JTAG_TRST_L       | TP7   |  GPIO        | DEBUGGER                      |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_28 | 153 | I2S_MCLK0    | I2S0_MCK                | B41 | I2S0_MCK                 | J6:3  |  GPIO        | WCD_I2S0_MCK                  |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_29 | 154 | I2S_SCLK0    | I2S0_SCK_BCLK           | A59 | I2S0_SCK_BCLK            | J6:5  |  GPIO        | WCD_I2S0_SCK_BCLK             |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_30 | 163 | I2S_LRCLK0   | I2S0_WS_LRCLK           | A55 | I2S0_WS_LRCLK            | J6:6  |  GPIO        | WCD_I2S0_WS_LRCLK             |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_31 | 164 | I2S_SDATAI0  | I2S0_SD_IN              | A54 | I2S0_SD_IN               | J10:8 |  GPIO        | ARD_GPIO7                     |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_32 | 161 | I2S_SDATAO0  | I2S0_SD_OUT             | A56 | I2S0_SD_OUT              | J6:4  |  GPIO        | WCD_I2S0_SD_OUT               |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_33 | 158 | I2S_MCLK1    | I2S1_MCK                | A61 | I2S1_MCK                 | J6:26 |  GPIO        | WCD_I2S1_MCK                  |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_34 | 160 | I2S_SCLK1    | I2S1_SCK_BCLK           | B43 | I2S1_SCK_BCLK            | J6:29 |  GPIO        | WCD_I2S1_SCK_BCLK             |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_35 | 162 | I2S_LRCLK1   | I2S1_WS_LRCLK           | A53 | I2S1_WS_LRCLK            | J6:27 |  GPIO        | WCD_I2S1_WS_LRCLK             |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_36 | 156 | I2S_SDATAI1  | I2S1_SD_IN              | B42 | I2S1_SD_IN               | J12:1 |  GPIO        | ARD_GPIO8                     |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_37 | 155 | I2S_SDATAO1  | I2S1_SD_OUT             | A60 | I2S1_SD_OUT              | J6:10 |  GPIO        | WCD_I2S1_SD_OUT               |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_38 | 83  | SPI1_CLK     | SPI_1_CLK               | B8  | SPI_1_CLK                | J6:9  |  SPI         | WCD_SPI_1_CLK                 |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_39 | 84  | SPI1_MISO    | SPI_1_MISO              | B4  | SPI_1_MISO               | J6:11 |  SPI         | WCD_SPI1_MISO                 |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_40 | 86  | SPI1_SISO    | SPI_1_MOSI              | B37 | SPI_1_MOSI               | J6:13 |  SPI         | WCD_SPI_1_MOSI                |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_41 | 80  | SPI1_CS      | SPI_1_CS                | B11 | SPI_1_CS                 | J6:15 |  SPI         | WCD_SPI_1_CS                  |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_42 | 152 | SDIO_CLK     | SDIO_CLK                | B29 | SDIO_CLK                 | J6:32 |  SDIO        | WCD_SDIO_CLK                  |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_43 | 151 | SDIO_CMD     | SDIO_CMD                | A41 | SDIO_CMD                 | J6:33 |  SDIO        | WCD_SDIO_CMD                  |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_44 | 150 | SDIO_DATA_0  | SDIO_DATA0              | B27 | SDIO_DATA_0              | J6:31 |  SDIO        | WCD_SDIO_DATA_0               |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_45 | 149 | SDIO_DATA_1  | SDIO_DATA1              | B26 | SDIO_DATA_1              | J6:30 |  SDIO        | WCD_SDIO_DATA_1               |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_46 | 148 | SDIO_DATA_2  | SDIO_DATA2              | A42 | SDIO_DATA_2              | J6:35 |  SDIO        | WCD_SDIO_DATA_2               |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_47 | 147 | SDIO_DATA_3  | SDIO_DATA3              | B31 | SDIO_DATA_3              | J6:34 |  SDIO        | WCD_SDIO_DATA_3               |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_48 | 93  | I2C0_SDATA   | I2C_0_SDA               | A51 | I2C_0_SDA                | J6:25 |  GPIO        | WCD_I2C_0_SDA                 |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_49 | 92  | I2C0_CLK     | I2C_0_SCL               | A58 | I2C_0_SCL                | J6:23 |  GPIO        | WCD_I2C_0_SCL                 |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_50 | 90  | I2C1_SDATA   | I2C_1_SDA               | A52 | I2C_1_SDA                | J12:9 |  GPIO        | ARD_AD4_SDA                   |
|---------------+-----+--------------+-------------------------+-----+--------------------------+-------+--------------+-------------------------------|
| WICED_GPIO_51 | 89  | I2C1_CLK     | I2C_1_SCL               | A57 | I2C_1_SCL                | J12:10|  GPIO        | ARD_AD5_SCL                   |
+------------------------------------------------------------------------------------------------------------------------------------------------------+

+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Enum ID                 |Pin  |   Pin Name on |  SIP Pin Name                            | SIP |  Module Pin                 | Module|  Peripheral |  Peripheral                |
|                         | #   |      43907    |                                          | Pin#|  Name                       | Pin#  |  Available  |    Alias                   |
|-------------------------+-----+---------------+------------------------------------------+-----+-----------------------------+-------+-------------+----------------------------|
| WICED_PERIPHERAL_PIN_1  | 202 | RF_SW_CTRL_6  | RF_SW_CTRL_6_UART1_RX_IN                 | B22 |  RF_SW_CTRL_6_UART1_RXD     | J6:36 |  UART1      | WCD_RF_SW_CTRL_6_UART1_RXD |
|-------------------------+-----+---------------+------------------------------------------+-----+-----------------------------+-------+-------------+----------------------------|
| WICED_PERIPHERAL_PIN_2  | 201 | RF_SW_CTRL_7  | RF_SW_CTRL_7_RSRC_INIT_MODE_UART1_TX_OUT | B16 |  RF_SW_CTRL_7_UART1_TXD     | J6:37 |  UART1      | WCD_UART1_TXD              |
|-------------------------+-----+---------------+------------------------------------------+-----+-----------------------------+-------+-------------+----------------------------|
| WICED_PERIPHERAL_PIN_3  | 88  | UART0_RXD     | UART0_RXD_IN                             | B45 |  UART0_RXD_BT_JTAG_TDO      | J6:18 |  UART2      | WCD_UART0_RXD_BT_JTAG_TDO  |
|-------------------------+-----+---------------+------------------------------------------+-----+-----------------------------+-------+-------------+----------------------------|
| WICED_PERIPHERAL_PIN_4  | 87  | UART0_TXD     | UART0_TXD_OUT                            | B44 |  UART0_TXD_BT_JTAG_TDI      | J6:20 |  UART2      | WCD_UART0_TXD_BT_JTAG_TDI  |
|-------------------------+-----+---------------+------------------------------------------+-----+-----------------------------+-------+-------------+----------------------------|
| WICED_PERIPHERAL_PIN_5  | 85  | UART0_CTS     | UART0_CTS_IN                             | A64 |  UART0_CTS_BT_JTAG_CLK      | J6:22 |  UART2      | WCD_UART0_CTS_BT_JTAG_CLK  |
|-------------------------+-----+---------------+------------------------------------------+-----+-----------------------------+-------+-------------+----------------------------|
| WICED_PERIPHERAL_PIN_6  | 91  | UART0_RTS     | UART0_RTS_OUT                            | A63 |  UART0_RTS_BT_JTAG_TMS      | J6:24 |  UART2      | WCD_UART0_RTS_BT_JTAG_TMS  |
|-------------------------+-----+---------------+------------------------------------------+-----+-----------------------------+-------+-------------+----------------------------|
| WICED_PERIPHERAL_PIN_7  | 200 | RF_SW_CTRL_8  | RF_SW_CTRL_8_BT_SECI_IN                  | A23 |  RF_SW_CTRL_8_BT_GPIO_7     | J6:38 |  UART3      | WCD_RF_SW_CTRL_8_BT_GPIO_7 |
|-------------------------+-----+---------------+------------------------------------------+-----+-----------------------------+-------+-------------+----------------------------|
| WICED_PERIPHERAL_PIN_8  | 196 | RF_SW_CTRL_9  | RF_SW_CTRL_9_HIB_LPO_SEL_BT_SECI_OUT     | C10 |  RF_SW_CTRL_9_BT_GPIO_6     | J6:39 |  UART3      | WCD_RF_SW_CTRL_9_BT_GPIO_6 |
|-------------------------+-----+---------------+------------------------------------------+-----+-----------------------------+-------+-------------+----------------------------+
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
    WICED_GPIO_24,
    WICED_GPIO_25,
    WICED_GPIO_26,
    WICED_GPIO_27,
    WICED_GPIO_28,
    WICED_GPIO_29,
    WICED_GPIO_30,
    WICED_GPIO_31,
    WICED_GPIO_32,
    WICED_GPIO_33,
    WICED_GPIO_34,
    WICED_GPIO_35,
    WICED_GPIO_36,
    WICED_GPIO_37,
    WICED_GPIO_38,
    WICED_GPIO_39,
    WICED_GPIO_40,
    WICED_GPIO_41,
    WICED_GPIO_42,
    WICED_GPIO_43,
    WICED_GPIO_44,
    WICED_GPIO_45,
    WICED_GPIO_46,
    WICED_GPIO_47,
    WICED_GPIO_48,
    WICED_GPIO_49,
    WICED_GPIO_50,
    WICED_GPIO_51,

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
    WICED_SPI_2,
    WICED_SPI_MAX, /* Denotes the total number of SPI port aliases. Not a valid SPI alias */
} wiced_spi_t;

typedef enum
{
    WICED_I2C_1,
    WICED_I2C_2,
    WICED_I2C_MAX, /* Denotes the total number of I2C port aliases. Not a valid I2C alias */
} wiced_i2c_t;

typedef enum
{
    WICED_PWM_1,
    WICED_PWM_2,
    WICED_PWM_3,
    WICED_PWM_4,
    WICED_PWM_5,
    WICED_PWM_6,
    WICED_PWM_MAX, /* Denotes the total number of PWM port aliases. Not a valid PWM alias */
} wiced_pwm_t;

typedef enum
{
#ifdef USING_EXTERNAL_ADC
   WICED_ADC_1,
   WICED_ADC_2,
   WICED_ADC_3,
   WICED_ADC_4,
#ifdef USING_EXTENDED_EXTERNAL_ADC
   WICED_ADC_5,
   WICED_ADC_6,
#endif /* ifdef USING_EXTENDED_EXTERNAL_ADC */
#else
   WICED_ADC_NONE = 0xff, /* No ADCs */
#endif /* ifdef USING_EXTERNAL_ADC */
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
    WICED_I2S_1,
    WICED_I2S_2,
    WICED_I2S_3,
    WICED_I2S_MAX, /* Denotes the total number of I2S port aliases. Not a valid I2S alias */
} wiced_i2s_t;

/* Logical Button-ids which map to phyiscal buttons on the board */
typedef enum
{
    PLATFORM_BUTTON_1,
    PLATFORM_BUTTON_2,
    PLATFORM_BUTTON_MAX, /* Denotes the total number of Buttons on the board. Not a valid Button alias */
} platform_button_t;

#define WICED_PLATFORM_BUTTON_COUNT  ( 2 )

#define ARD_GPIO0  (WICED_GPIO_1)
#define ARD_GPIO1  (WICED_GPIO_2)
#define ARD_GPIO2  (WICED_GPIO_9)
#define ARD_GPIO3  (WICED_GPIO_3)
#define ARD_GPIO4  (WICED_GPIO_10)
#define ARD_GPIO5  (WICED_GPIO_12)
#define ARD_GPIO6  (WICED_GPIO_11)
#define ARD_GPIO7  (WICED_GPIO_31)
#define ARD_GPIO8  (WICED_GPIO_36)
#define ARD_GPIO9  (WICED_GPIO_8)

#define ARD_MISO   (WICED_GPIO_4)
#define ARD_SCK    (WICED_GPIO_5)
#define ARD_MOSI   (WICED_GPIO_6)
#define ARD_SS     (WICED_GPIO_7)

/*No external GPIO LEDS are available*/
#define GPIO_LED_NOT_SUPPORTED

/* Note: The BCM943907AEVAL2F hardware does not have physical LEDs or Buttons so the #defines are mapped to PWM 4,5 and SPI on the expansion header J6 */
#define WICED_LED1       ( WICED_GPIO_17 )
#define WICED_LED2       ( WICED_GPIO_19 )
#define WICED_BUTTON1    ( WICED_GPIO_18)
#define WICED_BUTTON2    ( WICED_GPIO_20 )

#define WICED_GPIO_AUTH_RST  (WICED_GPIO_14)

#define AUTH_IC_I2C_PORT     (WICED_I2C_1)

/* no audio on this platform */
#define PLATFORM_DEFAULT_AUDIO_INPUT    AUDIO_DEVICE_ID_NONE
#define PLATFORM_AUDIO_NUM_INPUTS       (0)
#define PLATFORM_DEFAULT_AUDIO_OUTPUT   AUDIO_DEVICE_ID_OUTPUT_NONE
#define PLATFORM_AUDIO_NUM_OUTPUTS      (0)

/* Bootloader OTA and OTA2 factory reset during settings */
#define PLATFORM_FACTORY_RESET_BUTTON_INDEX     ( PLATFORM_BUTTON_1 )
#define PLATFORM_FACTORY_RESET_TIMEOUT          ( 10000 )

/* Generic button checking defines */
#define PLATFORM_BUTTON_PRESS_CHECK_PERIOD      ( 100 )
#define PLATFORM_BUTTON_PRESSED_STATE           (   0 )

#define PLATFORM_GREEN_LED_INDEX                ( WICED_LED_INDEX_1 )
#define PLATFORM_RED_LED_INDEX                  ( WICED_LED_INDEX_2 )

#ifdef __cplusplus
} /*extern "C" */
#endif
