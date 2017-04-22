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
BCM943907WCD2 platform pin definitions ...
+-------------------------------------------------------------------------------------------------------------------+
| Enum ID       |Pin  |   Pin Name on    |  Module                           |  Conn  |  Peripheral  |  Peripheral  |
|               | #   |      43907       |  Alias                            |  Pin   |  Available   |    Alias     |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_1  | C11 | GPIO_0           | GPIO_0_USB_CLT                    |  U3:19 |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_2  | A30 | GPIO_1           | GPIO_1_GSPI_MODE                  |  U3:21 |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_3  | A8  | GPIO_7           | GPIO_7_WCPU_BOOT_MODE             |  U4:41 |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_4  | C16 | GPIO_8           | GPIO_8_TAP_SEL                    |  U4:43 |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_5  | C15 | GPIO_9           | GPIO_9_USB_SEL                    |  U4:45 |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_6  | A45 | GPIO_10          | GPIO_10_DEEP_RST                  |  U4:49 |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_7  | C4  | GPIO_11          | GPIO_11_ACPU_BOOT_MODE            |  U4:53 |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_8  | C14 | GPIO_12          | GPIO_12_CODEC_PDN                 |  U4:55 |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_9  | B28 | GPIO_13          | GPIO_13_SDIO_MODE                 |  U4:57 |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_10 | D8  | GPIO_14          | GPIO_14                           |  U4:59 |  GPIO        |              | *
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_11 | B26 | GPIO_15          | GPIO_15                           |  U4:63 |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_12 | B19 | GPIO_16          | GPIO_16_USB_EN_AUD_SW             |  U4:65 |  GPIO        |              | *
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_13 | C10 | PWM_0            | PWM_0                             |  U3:60 |  PWM, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_14 | C9  | PWM_1            | PWM_1                             |  U3:62 |  PWM, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_15 | A27 | PWM_2            | PWM_2                             |  U3:64 |  PWM, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_16 | A26 | PWM_3            | PWM_3                             |  U3:66 |  PWM, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_17 | A31 | PWM_4            | PWM_4                             |  U3:68 |  PWM, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_18 | D7  | PWM_5            | PWM_5                             |  U3:70 |  PWM, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_19 | A12 | SPI_0_MISO       | SPI_0_MISO                        |  U3:36 |  SPI, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_20 | A11 | SPI_0_CLK        | SPI_0_CLK                         |  U3:34 |  SPI, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_21 | A9  | SPI_0_MOSI       | SPI_0_MOSI                        |  U3:38 |  SPI, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_22 | A10 | SPI_0_CS         | SPI_0_CS                          |  U3:40 |  SPI, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_23 | C13 | GPIO_2           | GPIO_2_JTAG_TCK                   |  U4:19 |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_24 | B20 | GPIO_3           | GPIO_3_JTAG_TMS                   |  U4:33 |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_25 | C3  | GPIO_4           | GPIO_4_JTAG_TDI                   |  U4:21 |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_26 | B4  | GPIO_5           | GPIO_5_JTAG_TDO                   |  U4:23 |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_27 | C2  | GPIO_6           | GPIO_6_JTAG_TRST_L                |  U4:35 |  GPIO        |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_28 | A46 | I2C0_SDATA       | I2C_0_SDA                         |  U4:68 |  I2C, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_29 | A47 | I2C0_CLK         | I2C_0_SCL                         |  U4:70 |  I2C, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_30 | B36 | I2S_MCLK0        | I2S0_MCK                          |  U4:48 |  I2S, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_31 | B35 | I2S_SCLK0        | I2S0_SCK_BCLK                     |  U4:44 |  I2S, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_32 | A51 | I2S_LRCLK0       | I2S0_WS_LRCLK                     |  U4:46 |  I2S, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_33 | A50 | I2S_SDATAI0      | I2S0_SD_IN                        |  U4:52 |  I2S, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_34 | A48 | I2S_SDATAO0      | I2S0_SD_OUT                       |  U4:50 |  I2S, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_35 | D5  | I2S_MCLK1        | I2S1_MCK                          |  U4:60 |  I2S, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_36 | B1  | I2S_SCLK1        | I2S1_SCK_BCLK                     |  U4:56 |  I2S, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_37 | A49 | I2S_LRCLK1       | I2S1_WS_LRCLK                     |  U4:58 |  I2S, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_38 | B38 | I2S_SDATAI1      | I2S1_SD_IN                        |  U4:64 |  I2S, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|
| WICED_GPIO_39 | B37 | I2S_SDATAO1      | I2S1_SD_OUT                       |  U4:62 |  I2S, GPIO   |              |
|---------------+-----+------------------+-----------------------------------+--------+--------------+--------------|

+--------------------------------------------------------------------------------------------------------------------------------+
| Enum ID                 |Pin  |   Pin Name on |    Board                                 |  Conn  |  Peripheral |  Peripheral  |
|                         | #   |      43907    |  Connection                              |  Pin   |  Available  |    Alias     |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_1  | B18 | RF_SW_CTRL_6  | RF_SW_CTRL_6_UART1_RX_IN                 |  U3:3  |  UART1      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_2  | B15 | RF_SW_CTRL_7  | RF_SW_CTRL_7_RSRC_INIT_MODE_UART1_TX_OUT |  U3:5  |  UART1      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_3  | A52 | UART0_RXD_IN  | UART0_RXD_IN                             |  J8:4  |  UART0      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_4  | A55 | UART0_TXD_OUT | UART0_TXD_OUT                            |  J8:5  |  UART0      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_5  | A54 | UART0_CTS_IN  | UART0_CTS_IN                             |  J8:6  |  UART0      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_6  | A53 | UART0_RTS_OUT | UART0_RTS_OUT                            |  J8:2  |  UART0      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_7  | B14 | RF_SW_CTRL_8  | RF_SW_CTRL_8_UART2_RX                    |  J3:1  |  UART2      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_8  | C7  | RF_SW_CTRL_9  | RF_SW_CTRL_9_HIB_LPO_SEL_UART2_TX        |  J3:2  |  UART2      |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_9  | C1  | I2C1_SDATA    | I2C_1_SDA                                |  U4:74 |  I2C1       |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_10 | B2  | I2C1_CLK      | I2C_1_SCL                                |  U4:72 |  I2C1       |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_11 | B31 | SPI_1_MISO    | SPI_1_MISO                               |  U4:36 |  SPI        |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_12 | B32 | SPI_1_CLK     | SPI_1_CLK                                |  U4:34 |  SPI        |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_13 | B30 | SPI_1_MOSI    | SPI_1_MOSI                               |  U4:38 |  SPI        |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------|
| WICED_PERIPHERAL_PIN_14 | B33 | SPI_1_CS      | SPI_1_CS                                 |  U4:40 |  SPI        |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------+
| WICED_PERIPHERAL_PIN_15 | B25 | SDIO_CLK      | SDIO_CLK                                 |  U3:44 |  SDIO       |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------+
| WICED_PERIPHERAL_PIN_16 | B24 | SDIO_CMD      | SDIO_CMD                                 |  U3:48 |  SDIO       |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------+
| WICED_PERIPHERAL_PIN_17 | B23 | SDIO_DATA0    | SDIO_DATA0                               |  U3:50 |  SDIO       |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------+
| WICED_PERIPHERAL_PIN_18 | B22 | SDIO_DATA1    | SDIO_DATA1                               |  U3:52 |  SDIO       |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------+
| WICED_PERIPHERAL_PIN_19 | B21 | SDIO_DATA2    | SDIO_DATA2                               |  U3:54 |  SDIO       |              |
|-------------------------+-----+---------------+------------------------------------------+--------+-------------+--------------+
| WICED_PERIPHERAL_PIN_20 | B27 | SDIO_DATA3    | SDIO_DATA3                               |  U3:56 |  SDIO       |              |
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
    WICED_PERIPHERAL_PIN_9,
    WICED_PERIPHERAL_PIN_10,
    WICED_PERIPHERAL_PIN_11,
    WICED_PERIPHERAL_PIN_12,
    WICED_PERIPHERAL_PIN_13,
    WICED_PERIPHERAL_PIN_14,
    WICED_PERIPHERAL_PIN_15,
    WICED_PERIPHERAL_PIN_16,
    WICED_PERIPHERAL_PIN_17,
    WICED_PERIPHERAL_PIN_18,
    WICED_PERIPHERAL_PIN_19,
    WICED_PERIPHERAL_PIN_20,
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
    WICED_I2S_1,
    WICED_I2S_2,
    WICED_I2S_3,
    WICED_I2S_4,
    WICED_I2S_MAX, /* Denotes the total number of I2S port aliases. Not a valid I2S alias */
} wiced_i2s_t;

/* Logical Button-ids which map to phyiscal buttons on the board */
typedef enum
{
    PLATFORM_BUTTON_1,
    PLATFORM_BUTTON_2,
    PLATFORM_BUTTON_3,
    PLATFORM_BUTTON_4,
    PLATFORM_BUTTON_5,
    PLATFORM_BUTTON_6,
    PLATFORM_BUTTON_MAX, /* Denotes the total number of Buttons on the board. Not a valid Button alias */
} platform_button_t;

#define WICED_PLATFORM_BUTTON_COUNT  ( 6 )

#define WICED_PLATFORM_INCLUDES_SPI_FLASH
#define WICED_SPI_FLASH_PORT (WICED_SPI_2)
#define WICED_SPI_FLASH_CS   (WICED_GPIO_NONE) /* No GPIO available, must use controller */

#define WICED_GMAC_PHY_RESET (WICED_GPIO_5)

#define WICED_USB_HOST_OVERCURRENT      (WICED_GPIO_1)
#define WICED_USB_HOST_POWER_ENABLE     (WICED_GPIO_12)

/* No external GPIO LEDS are available */
#define GPIO_LED_NOT_SUPPORTED

/* Components connected to external I/Os */
/* Note: The BCM943907WCD2 hardware does not have physical LEDs or buttons so the #defines are mapped to PWM 0,1,2,3 on the expansion header */
#define WICED_LED1       (WICED_GPIO_13)
#define WICED_LED2       (WICED_GPIO_14)
#define WICED_BUTTON1    (WICED_GPIO_15)    /* BACK button on eval board */
#define WICED_BUTTON2    (WICED_GPIO_16)    /* FORWARD button on eval board */

#define WICED_GPIO_AUTH_RST  ( WICED_GPIO_6 )
#define WICED_GPIO_AKM_PDN   ( WICED_GPIO_8 )

#define AUTH_IC_I2C_PORT (WICED_I2C_2)

/* common audio device defines */
#define PLATFORM_DEFAULT_AUDIO_INPUT    AUDIO_DEVICE_ID_AK4954_ADC_LINE
#define PLATFORM_AUDIO_NUM_INPUTS       2
#define PLATFORM_DEFAULT_AUDIO_OUTPUT   AUDIO_DEVICE_ID_AK4954_DAC_LINE
#define PLATFORM_AUDIO_NUM_OUTPUTS      2

#define WICED_BUTTON_MULTI_FUNC         ( WICED_GPIO_5 )
#define WICED_BUTTON_PAUSE_PLAY         ( WICED_GPIO_18 )
#define WICED_BUTTON_VOLUME_UP          ( WICED_GPIO_17 )
#define WICED_BUTTON_VOLUME_DOWN        ( WICED_GPIO_12 )
#define WICED_BUTTON_BACK               ( WICED_GPIO_15 )
#define WICED_BUTTON_FORWARD            ( WICED_GPIO_16 )

/* Logical Button-ids which map to phyiscal buttons on the board (see platform_gpio_buttons[] in platform.c)*/
#define PLATFORM_BUTTON_BACK                    ( PLATFORM_BUTTON_1 )
#define PLATFORM_BUTTON_VOLUME_DOWN             ( PLATFORM_BUTTON_2 )
#define PLATFORM_BUTTON_VOLUME_UP               ( PLATFORM_BUTTON_3 )
#define PLATFORM_BUTTON_PAUSE_PLAY              ( PLATFORM_BUTTON_4 )
#define PLATFORM_BUTTON_MULTI_FUNC              ( PLATFORM_BUTTON_5 )
#define PLATFORM_BUTTON_FORWARD                 ( PLATFORM_BUTTON_6 )

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
