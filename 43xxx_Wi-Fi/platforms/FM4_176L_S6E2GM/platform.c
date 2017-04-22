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

/** @file
 * Defines board support package for BCM943364WCD1 board
 */
#include "platform.h"
#include "platform_config.h"
#include "platform_init.h"
#include "platform_isr.h"
#include "platform_peripheral.h"
#include "wwd_platform_common.h"
#include "wwd_rtos_isr.h"
#include "wiced_defaults.h"
#include "wiced_platform.h"
#include "platform_mfi.h"
#include "platform_button.h"
#include "gpio_button.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define FM4_GPIO_REG_NAME( type, port, pin )             ( bFM_GPIO_##type##port##_P##pin )
#define FM4_GPIO_GET_REG_ADDRESS( reg_name )             ( ( volatile uint8_t* ) &( reg_name ) )
#define FM4_GPIO_GET_COMMON_REG_ADDRESSES( port, pin )    \
    .PFRx  = FM4_GPIO_GET_REG_ADDRESS( FM4_GPIO_REG_NAME( PFR,  port, pin )), \
    .PCRx  = FM4_GPIO_GET_REG_ADDRESS( FM4_GPIO_REG_NAME( PCR,  port, pin )), \
    .DDRx  = FM4_GPIO_GET_REG_ADDRESS( FM4_GPIO_REG_NAME( DDR,  port, pin )), \
    .PDIRx = FM4_GPIO_GET_REG_ADDRESS( FM4_GPIO_REG_NAME( PDIR, port, pin )), \
    .PDORx = FM4_GPIO_GET_REG_ADDRESS( FM4_GPIO_REG_NAME( PDOR, port, pin )), \
    .PZRx  = FM4_GPIO_GET_REG_ADDRESS( FM4_GPIO_REG_NAME( PZR,  port, pin ))

#define INTR_CONF_WRAPPER_FN( x ) \
static void INTR_CONF_WRAPPER_FN##x( void ); \
static void INTR_CONF_WRAPPER_FN##x( void ) \
{ \
    x(0u); \
} \

#define ASSIGN_INTR_CONF_FN( x ) \
 .intr_conf_fn = INTR_CONF_WRAPPER_FN##x


/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/
INTR_CONF_WRAPPER_FN( SetPinFunc_INT05_1 )
INTR_CONF_WRAPPER_FN( SetPinFunc_INT06_1 )

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* GPIO pin table. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_gpio_t platform_gpio_pins[] =
{
    [WICED_GPIO_1]  = { FM4_GPIO_GET_COMMON_REG_ADDRESSES( B , 2 ), .ADE = FM4_GPIO_GET_REG_ADDRESS( bFM_GPIO_ADE_AN18 ), .intr_conf_fn = NULL, .index = ExintInstanceIndexMax }, /* GPIO1PIN_PB2, PB2/AN18/SCS61_1/TIOA10_1/BIN0_2/INT09_1/TRACED10 (Pin104), RGB LED Green light control */
    [WICED_GPIO_2]  = { FM4_GPIO_GET_COMMON_REG_ADDRESSES( 1 , 8 ), .ADE = FM4_GPIO_GET_REG_ADDRESS( bFM_GPIO_ADE_AN08 ), .intr_conf_fn = NULL, .index = ExintInstanceIndexMax }, /* GPIO1PIN_P18, P18/AN08/SIN2_0/TIOA3_2/INT10_0/TRACED4 (Pin106), RGB LED Blue light control  */
    [WICED_GPIO_3]  = { FM4_GPIO_GET_COMMON_REG_ADDRESSES( 1 , A ), .ADE = FM4_GPIO_GET_REG_ADDRESS( bFM_GPIO_ADE_AN10 ), .intr_conf_fn = NULL, .index = ExintInstanceIndexMax }, /* GPIO1PIN_P1A, P1A/AN10/SCK2_0/TIOA4_2/TRACED6 (Pin108), RGB LED Red light control   */
    [WICED_GPIO_4]  = { FM4_GPIO_GET_COMMON_REG_ADDRESSES( 2 , 0 ), .ADE = NULL, .intr_conf_fn = NULL, .index = ExintInstanceIndexMax }, /* GPIO1PIN_P20, P20/NMIX/WKUP0 (Pin128) , SW2 push button   */
    /* IO pins on Arduino header */
    [WICED_GPIO_5]  = { FM4_GPIO_GET_COMMON_REG_ADDRESSES( F , 3 ), .ADE = NULL, ASSIGN_INTR_CONF_FN( SetPinFunc_INT05_1 ), .index = ExintInstanceIndexExint5 }, /* GPIO1PIN_PF3, PF3/SCS63_0/FRCK1_1/TIOB6_1/INT05_1/IC1_VCC_1 (Pin 79), CN9/Pin8 */
    [WICED_GPIO_6]  = { FM4_GPIO_GET_COMMON_REG_ADDRESSES( F , 4 ), .ADE = NULL, ASSIGN_INTR_CONF_FN( SetPinFunc_INT06_1 ), .index = ExintInstanceIndexExint6 }, /* GPIO1PIN_PF4, PF4/IC10_1/TIOA7_1/INT06_1/IC1_VPEN_1 (Pin 80), CN9/Pin5 */

    /* SDIO Pins */
    /* GPIO1PIN_P34, P34/IC03_0/INT00_1/S_CLK_0 (Pin 28), CN15/Pin3 */
    /* GPIO1PIN_P35, P35/IC02_0/INT01_1/S_CMD_0 (Pin 31), CN15/Pin4 */
    /* GPIO1PIN_P33, P33/FRCK0_0/S_DATA0_0 (Pin 27), CN17/Pin8 */
    /* GPIO1PIN_P32, P32/INT19_0/S_DATA1_0 (Pin 26), CN17/Pin7 */
    /* GPIO1PIN_P37, P37/IC00_0/INT03_1/S_DATA2_0 (Pin 33), CN15/Pin6 */
    /* GPIO1PIN_P36, P36/IC01_0/INT02_1/S_DATA3_0 (Pin 32), CN15/Pin5 */
    /* GPIO1PIN_P39, P39/RTO00_0(PPG00_0)/TIOA0_1/AIN1_0/INT16_1/S_CD_0 (Pin 35) */

    /* PWMs on Arduino header */
    /* GPIO1PIN_P45, P45/SCS72_1/RTO15_0/TIOA5_0/MCSX2_0 (Pin 51), CN9/Pin7 */
    /* GPIO1PIN_P44, P44/SCS71_1/RTO14_0/TIOA4_0/MCSX3_0 (Pin 50), CN9/Pin6 */
};

/* ADC peripherals. Used WICED/platform/MCU/wiced_platform_common.c */
const platform_adc_t platform_adc_peripherals[] =
{
#ifdef TODO
    [WICED_ADC_1] = {ADC1, ADC_Channel_1, RCC_APB2Periph_ADC1, 1, &platform_gpio_pins[WICED_GPIO_2]},
    [WICED_ADC_2] = {ADC1, ADC_Channel_2, RCC_APB2Periph_ADC1, 1, &platform_gpio_pins[WICED_GPIO_3]},
    [WICED_ADC_3] = {ADC1, ADC_Channel_3, RCC_APB2Periph_ADC1, 1, &platform_gpio_pins[WICED_GPIO_4]},
#else
    [WICED_ADC_1]  = { 0 },
#endif
};

/* PWM peripherals. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_pwm_t platform_pwm_peripherals[] =
{
#ifdef TODO
    [WICED_PWM_1]  = {TIM3,  3, RCC_APB1Periph_TIM3,  GPIO_AF_TIM3,  &platform_gpio_pins[WICED_GPIO_11]},
    [WICED_PWM_2]  = {TIM10, 1, RCC_APB2Periph_TIM10, GPIO_AF_TIM10, &platform_gpio_pins[WICED_GPIO_26]},
    [WICED_PWM_3]  = {TIM2,  2, RCC_APB1Periph_TIM2,  GPIO_AF_TIM2,  &platform_gpio_pins[WICED_GPIO_2] }, /* or TIM5/Ch2                       */
    [WICED_PWM_4]  = {TIM2,  3, RCC_APB1Periph_TIM2,  GPIO_AF_TIM2,  &platform_gpio_pins[WICED_GPIO_3] }, /* or TIM5/Ch3, TIM9/Ch1             */
    [WICED_PWM_5]  = {TIM2,  4, RCC_APB1Periph_TIM2,  GPIO_AF_TIM2,  &platform_gpio_pins[WICED_GPIO_4] }, /* or TIM5/Ch4, TIM9/Ch2             */
    [WICED_PWM_6]  = {TIM2,  1, RCC_APB1Periph_TIM2,  GPIO_AF_TIM2,  &platform_gpio_pins[WICED_GPIO_6] }, /* or TIM2_CH1_ETR, TIM8/Ch1N        */
    [WICED_PWM_7]  = {TIM3,  1, RCC_APB1Periph_TIM3,  GPIO_AF_TIM3,  &platform_gpio_pins[WICED_GPIO_7] }, /* or TIM1_BKIN, TIM8_BKIN, TIM13/Ch1*/
    [WICED_PWM_8]  = {TIM3,  2, RCC_APB1Periph_TIM3,  GPIO_AF_TIM3,  &platform_gpio_pins[WICED_GPIO_8] }, /* or TIM8/Ch1N, TIM14/Ch1           */
    [WICED_PWM_9]  = {TIM5,  2, RCC_APB1Periph_TIM5,  GPIO_AF_TIM5,  &platform_gpio_pins[WICED_GPIO_2] }, /* or TIM2/Ch2                       */
#else
    [WICED_PWM_1]  = { 0 },
#endif
};

/* PWM peripherals. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_spi_t platform_spi_peripherals[] =
{
#ifdef TODO
    [WICED_SPI_1]  =
    {
        .port                  = SPI1,
        .gpio_af               = GPIO_AF_SPI1,
        .peripheral_clock_reg  = RCC_APB2Periph_SPI1,
        .peripheral_clock_func = RCC_APB2PeriphClockCmd,
        .pin_mosi              = &platform_gpio_pins[WICED_GPIO_8],
        .pin_miso              = &platform_gpio_pins[WICED_GPIO_7],
        .pin_clock             = &platform_gpio_pins[WICED_GPIO_6],
        .tx_dma =
        {
            .controller        = DMA2,
            .stream            = DMA2_Stream5,
            .channel           = DMA_Channel_3,
            .irq_vector        = DMA2_Stream5_IRQn,
            .complete_flags    = DMA_FLAG_TCIF5,
            .error_flags       = ( DMA_HISR_TEIF5 | DMA_HISR_FEIF5 | DMA_HISR_DMEIF5 ),
        },
        .rx_dma =
        {
            .controller        = DMA2,
            .stream            = DMA2_Stream0,
            .channel           = DMA_Channel_3,
            .irq_vector        = DMA2_Stream0_IRQn,
            .complete_flags    = DMA_FLAG_TCIF0,
            .error_flags       = ( DMA_LISR_TEIF0 | DMA_LISR_FEIF0 | DMA_LISR_DMEIF0 ),
        },
    }
#else
    [WICED_SPI_1]  = { 0 },
#endif
};

const platform_i2c_t platform_i2c_peripherals[] =
{
#ifdef TODO
    [WICED_I2C_1] =
    {
        .port                    = I2C1,
        .pin_scl                 = &platform_gpio_pins[WICED_GPIO_24],
        .pin_sda                 = &platform_gpio_pins[WICED_GPIO_25],
        .peripheral_clock_reg    = RCC_APB1Periph_I2C1,
        .tx_dma                  = DMA1,
        .tx_dma_peripheral_clock = RCC_AHB1Periph_DMA1,
        .tx_dma_stream           = DMA1_Stream7,
        .rx_dma_stream           = DMA1_Stream5,
        .tx_dma_stream_id        = 7,
        .rx_dma_stream_id        = 5,
        .tx_dma_channel          = DMA_Channel_1,
        .rx_dma_channel          = DMA_Channel_1,
        .gpio_af                 = GPIO_AF_I2C1
    },
#else
    [WICED_I2C_1]  = { 0 },
#endif
};

const wiced_i2c_device_t auth_chip_i2c_device =
{
    .port          = WICED_I2C_1,
    .address       = 0x11,
    .address_width = I2C_ADDRESS_WIDTH_7BIT,
    .speed_mode    = I2C_STANDARD_SPEED_MODE,
};

const platform_mfi_auth_chip_t platform_auth_chip =
{
    .i2c_device = &auth_chip_i2c_device,
    .reset_pin  = WICED_GPIO_23
};

/* UART peripherals and runtime drivers. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_uart_t platform_uart_peripherals[] =
{
    [WICED_UART_1] =
    {
        .port         = 0,    /* MFS UART channel 0 */
        .tx_pin_index = 0,    /* Output pin SOT0_0 */
        .rx_pin_index = 0,    /* Input pin SIN0_0 */
        .cts_pin      = NULL,
        .rts_pin      = NULL,
    },
    [WICED_UART_2] =
    {
        .port         = 1,    /* MFS UART channel 1 */
        .tx_pin_index = 0,    /* Output pin SOT1_0 */
        .rx_pin_index = 0,    /* Input pin SIN1_0 */
        .cts_pin      = NULL,
        .rts_pin      = NULL,
    },
    [WICED_UART_3] =
    {
        .port         = 2,    /* MFS UART channel 2 */
        .tx_pin_index = 0,    /* Output pin SOT2_0 */
        .rx_pin_index = 0,    /* Input pin SIN2_0 */
        .cts_pin      = NULL,
        .rts_pin      = NULL,
    },
    [WICED_UART_4] =
    {
        .port         = 3,    /* MFS UART channel 3 */
        .tx_pin_index = 0,    /* Output pin SOT3_0 */
        .rx_pin_index = 0,    /* Input pin SIN3_0 */
        .cts_pin      = NULL,
        .rts_pin      = NULL,
    },
    [WICED_UART_5] =
    {
        .port         = 4,    /* MFS UART channel 4 */
        .tx_pin_index = 0,    /* Output pin SOT4_0 */
        .rx_pin_index = 0,    /* Input pin SIN4_0 */
        .cts_pin      = NULL,
        .rts_pin      = NULL,
    },
    [WICED_UART_6] =
    {
        .port         = 5,    /* MFS UART channel 5 */
        .tx_pin_index = 0,    /* Output pin SOT5_0 */
        .rx_pin_index = 0,    /* Input pin SIN5_0 */
        .cts_pin      = NULL,
        .rts_pin      = NULL,
    },
    [WICED_UART_7] =
    {
        .port         = 6,    /* MFS UART channel 6 */
        .tx_pin_index = 0,    /* Output pin SOT6_0 */
        .rx_pin_index = 0,    /* Input pin SIN6_0 */
        .cts_pin      = NULL,
        .rts_pin      = NULL,
    },
    [WICED_UART_8] =
    {
        .port         = 7,    /* MFS UART channel 7 */
        .tx_pin_index = 0,    /* Output pin SOT7_0 */
        .rx_pin_index = 0,    /* Input pin SIN7_0 */
        .cts_pin      = NULL,
        .rts_pin      = NULL,
    },
    [WICED_UART_9] =
    {
        .port         = 8,    /* MFS UART channel 8 */
        .tx_pin_index = 0,    /* Output pin SOT8_0 */
        .rx_pin_index = 0,    /* Input pin SIN8_0 */
        .cts_pin      = NULL,
        .rts_pin      = NULL,
    },
    [WICED_UART_10] =
    {
        .port         = 9,    /* MFS UART channel 9 */
        .tx_pin_index = 0,    /* Output pin SOT9_0 */
        .rx_pin_index = 0,    /* Input pin SIN9_0 */
        .cts_pin      = NULL,
        .rts_pin      = NULL,
    },
};

platform_uart_driver_t platform_uart_drivers[WICED_UART_MAX];

/* SPI flash. Exposed to the applications through include/wiced_platform.h */
#if defined ( WICED_PLATFORM_INCLUDES_SPI_FLASH )
const wiced_spi_device_t wiced_spi_flash =
{
    .port        = WICED_SPI_1,
    .chip_select = WICED_SPI_FLASH_CS,
    .speed       = 25000000,
    .mode        = (SPI_CLOCK_RISING_EDGE | SPI_CLOCK_IDLE_HIGH | SPI_USE_DMA | SPI_MSB_FIRST),
    .bits        = 8
};
#endif

/* UART standard I/O configuration */
#ifndef WICED_DISABLE_STDIO
static const platform_uart_config_t stdio_config =
{
    .baud_rate    = 115200,
    .data_width   = DATA_WIDTH_8BIT,
    .parity       = NO_PARITY,
    .stop_bits    = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_DISABLED,
};
#endif

/* Wi-Fi control pins. Used by WICED/platform/MCU/wwd_platform_common.c
 * SDIO: WWD_PIN_BOOTSTRAP[1:0] = b'00
 * gSPI: WWD_PIN_BOOTSTRAP[1:0] = b'01
 */
const platform_gpio_t wifi_control_pins[] =
{
    /* GPIO1PIN_P50, P50/SCS72_0/IC01_1/TIOA8_2 (Pin10), CN17/Pin1 */
    [WWD_PIN_RESET      ] = { FM4_GPIO_GET_COMMON_REG_ADDRESSES( 5 , 0 ), .ADE = NULL, .intr_conf_fn = NULL, .index = ExintInstanceIndexMax },

//    [WWD_PIN_POWER      ] = { 0 },
//    [WWD_PIN_32K_CLK    ] = { 0 },
//    [WWD_PIN_BOOTSTRAP_0] = { 0 },
//    [WWD_PIN_BOOTSTRAP_1] = { 0 },
};

/* Wi-Fi SDIO bus pins. Used by WICED/platform/CYFM4/WWD/wwd_SDIO.c */
const platform_gpio_t wifi_sdio_pins[] =
{
    /* Pin reconfiguration is done by the peripheral API SdInitPins() as part of init sequence */
    /* GPIO1PIN_P34, P34/IC03_0/INT00_1/S_CLK_0 (Pin 28), CN15/Pin3 */
    /* GPIO1PIN_P35, P35/IC02_0/INT01_1/S_CMD_0 (Pin 31), CN15/Pin4 */
    /* GPIO1PIN_P33, P33/FRCK0_0/S_DATA0_0 (Pin 27), CN17/Pin8 */
    /* GPIO1PIN_P32, P32/INT19_0/S_DATA1_0 (Pin 26), CN17/Pin7 */
    /* GPIO1PIN_P37, P37/IC00_0/INT03_1/S_DATA2_0 (Pin 33), CN15/Pin6 */
    /* GPIO1PIN_P36, P36/IC01_0/INT02_1/S_DATA3_0 (Pin 32), CN15/Pin5 */
    /* GPIO1PIN_P39, P39/RTO00_0(PPG00_0)/TIOA0_1/AIN1_0/INT16_1/S_CD_0 (Pin 35) */
};

/* Wi-Fi gSPI bus pins. Used by WICED/platform/STM32F2xx/WWD/wwd_SPI.c */
const platform_gpio_t wifi_spi_pins[] =
{
#ifdef TODO
    [WWD_PIN_SPI_IRQ ] = { GPIOC,  9 },
    [WWD_PIN_SPI_CS  ] = { GPIOB, 12 },
    [WWD_PIN_SPI_CLK ] = { GPIOB, 13 },
    [WWD_PIN_SPI_MOSI] = { GPIOB, 15 },
    [WWD_PIN_SPI_MISO] = { GPIOB, 14 },
#else
    [WWD_PIN_SPI_IRQ ] = { 0 },
    [WWD_PIN_SPI_CS  ] = { 0 },
    [WWD_PIN_SPI_CLK ] = { 0 },
    [WWD_PIN_SPI_MOSI] = { 0 },
    [WWD_PIN_SPI_MISO] = { 0 },
#endif
};

gpio_button_t platform_gpio_buttons[] =
{
    [PLATFORM_BUTTON_1] =
    {
        .polarity   = WICED_ACTIVE_HIGH,
        .gpio       = WICED_BUTTON1,
        .trigger    = IRQ_TRIGGER_BOTH_EDGES,
    },
};

const wiced_gpio_t platform_gpio_leds[PLATFORM_LED_COUNT] =
{
     [WICED_LED_INDEX_1] = WICED_LED1,
     [WICED_LED_INDEX_2] = WICED_LED2,
};
/******************************************************
 *               Function Definitions
 ******************************************************/

void platform_init_peripheral_irq_priorities( void )
{
#ifdef TODO
    /* Interrupt priority setup. Called by WICED/platform/MCU/STM32F2xx/platform_init.c */
    NVIC_SetPriority( RTC_WKUP_IRQn    ,  1 ); /* RTC Wake-up event   */
    NVIC_SetPriority( SDIO_IRQn        ,  2 ); /* WLAN SDIO           */
    NVIC_SetPriority( DMA2_Stream3_IRQn,  3 ); /* WLAN SDIO DMA       */
    NVIC_SetPriority( DMA1_Stream3_IRQn,  3 ); /* WLAN SPI DMA        */
    NVIC_SetPriority( USART1_IRQn      ,  6 ); /* WICED_UART_1        */
    NVIC_SetPriority( USART2_IRQn      ,  6 ); /* WICED_UART_2        */
    NVIC_SetPriority( DMA2_Stream7_IRQn,  7 ); /* WICED_UART_1 TX DMA */
    NVIC_SetPriority( DMA2_Stream2_IRQn,  7 ); /* WICED_UART_1 RX DMA */
    NVIC_SetPriority( DMA1_Stream6_IRQn,  7 ); /* WICED_UART_2 TX DMA */
    NVIC_SetPriority( DMA1_Stream5_IRQn,  7 ); /* WICED_UART_2 RX DMA */
#endif
    NVIC_SetPriority( SD_IRQn          ,  2 );
    NVIC_SetPriority( MFS0_TX_IRQn     ,  6 );
    NVIC_SetPriority( MFS0_RX_IRQn     ,  6 );
}

/* LEDs on this platform are active HIGH */
platform_result_t platform_led_set_state(int led_index, int off_on )
{
    if ((led_index >= 0) && (led_index < PLATFORM_LED_COUNT))
    {
        switch (off_on)
        {
            case WICED_LED_OFF:
                platform_gpio_output_low( &platform_gpio_pins[platform_gpio_leds[led_index]] );
                break;
            case WICED_LED_ON:
                platform_gpio_output_high( &platform_gpio_pins[platform_gpio_leds[led_index]] );
                break;
        }
        return PLATFORM_SUCCESS;
    }
    return PLATFORM_BADARG;
}

void platform_led_init( void )
{
    /* Initialise LEDs and turn off by default */
    platform_gpio_init( &platform_gpio_pins[WICED_LED1], OUTPUT_PUSH_PULL );
    platform_gpio_init( &platform_gpio_pins[WICED_LED2], OUTPUT_PUSH_PULL );
    platform_led_set_state(WICED_LED_INDEX_1, WICED_LED_ON);
    platform_led_set_state(WICED_LED_INDEX_2, WICED_LED_OFF);
 }
void platform_init_external_devices( void )
{
    /* Initialise LEDs and turn off by default */
    platform_led_init();

    /* Initialise buttons to input by default */
    platform_gpio_init( &platform_gpio_pins[WICED_BUTTON1], INPUT_PULL_UP );

#ifndef WICED_DISABLE_STDIO
    /* Initialise UART standard I/O */
    platform_stdio_init( &platform_uart_drivers[STDIO_UART], &platform_uart_peripherals[STDIO_UART], &stdio_config );
#endif
}

uint32_t  platform_get_button_press_time ( int button_index, int led_index, uint32_t max_time )
{
    int             button_gpio;
    uint32_t        button_press_timer = 0;
    int             led_state = 0;

    /* Initialize input */
    button_gpio     = platform_gpio_buttons[button_index].gpio;
    platform_gpio_init( &platform_gpio_pins[ button_gpio ], INPUT_PULL_UP );

    while ( (PLATFORM_BUTTON_PRESSED_STATE == platform_gpio_input_get(&platform_gpio_pins[ button_gpio ])) )
    {
        /* wait a bit */
        host_rtos_delay_milliseconds( PLATFORM_BUTTON_PRESS_CHECK_PERIOD );

        /* Toggle LED */
        platform_led_set_state(led_index, (led_state == 0) ? WICED_LED_OFF : WICED_LED_ON);
        led_state ^= 0x01;

        /* keep track of time */
        button_press_timer += PLATFORM_BUTTON_PRESS_CHECK_PERIOD;
        if ((max_time > 0) && (button_press_timer >= max_time))
        {
            break;
        }
    }

     /* turn off the LED */
    platform_led_set_state(led_index, WICED_LED_OFF );

    return button_press_timer;
}

uint32_t  platform_get_factory_reset_button_time ( uint32_t max_time )
{
    return platform_get_button_press_time ( PLATFORM_FACTORY_RESET_BUTTON_INDEX, PLATFORM_RED_LED_INDEX, max_time );
}

wiced_bool_t host_platform_supports_mpc( void )
{
    return WICED_FALSE;
}
/******************************************************
 *           Interrupt Handlers
 ******************************************************/
#if (PDL_PERIPHERAL_ENABLE_EXINT5 == PDL_ON)
extern void EXINT5_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( EXINT5_IRQ )
{
    EXINT5_IRQHandler();
}
WWD_RTOS_MAP_ISR(EXINT5_IRQ, IRQ016_Handler)
#endif /* PDL_INTERRUPT_ENABLE_EXINT5 */

#if (PDL_PERIPHERAL_ENABLE_EXINT6 == PDL_ON)
extern void EXINT6_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( EXINT6_IRQ )
{
    EXINT6_IRQHandler();
}
WWD_RTOS_MAP_ISR(EXINT6_IRQ, IRQ017_Handler)
#endif /* PDL_PERIPHERAL_ENABLE_EXINT6 */

#if ( PDL_INTERRUPT_ENABLE_MFS0 == PDL_ON )
extern void MFS0_RX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS0_RX_IRQ )
{
    MFS0_RX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS0_RX_IRQ, IRQ060_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS0 */

#if ( PDL_INTERRUPT_ENABLE_MFS0 == PDL_ON )
extern void MFS0_TX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS0_TX_IRQ )
{
    MFS0_TX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS0_TX_IRQ, IRQ061_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS0 */

#if ( PDL_INTERRUPT_ENABLE_MFS1 == PDL_ON )
extern void MFS1_RX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS1_RX_IRQ )
{
    MFS1_RX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS1_RX_IRQ, IRQ062_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS1 */

#if ( PDL_INTERRUPT_ENABLE_MFS1 == PDL_ON )
extern void MFS1_TX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS1_TX_IRQ )
{
    MFS1_TX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS1_TX_IRQ, IRQ063_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS1 */

#if ( PDL_INTERRUPT_ENABLE_MFS2 == PDL_ON )
extern void MFS2_RX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS2_RX_IRQ )
{
    MFS2_RX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS2_RX_IRQ, IRQ064_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS2 */

#if ( PDL_INTERRUPT_ENABLE_MFS2 == PDL_ON )
extern void MFS2_TX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS2_TX_IRQ )
{
    MFS2_TX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS2_TX_IRQ, IRQ065_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS2 */

#if ( PDL_INTERRUPT_ENABLE_MFS3 == PDL_ON )
extern void MFS3_RX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS3_RX_IRQ )
{
    MFS3_RX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS3_RX_IRQ, IRQ066_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS3 */

#if ( PDL_INTERRUPT_ENABLE_MFS3 == PDL_ON )
extern void MFS3_TX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS3_TX_IRQ )
{
    MFS3_TX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS3_TX_IRQ, IRQ067_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS3 */

#if ( PDL_INTERRUPT_ENABLE_MFS4 == PDL_ON )
extern void MFS4_RX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS4_RX_IRQ )
{
    MFS4_RX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS4_RX_IRQ, IRQ068_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS4 */

#if ( PDL_INTERRUPT_ENABLE_MFS4 == PDL_ON )
extern void MFS4_TX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS4_TX_IRQ )
{
    MFS4_TX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS4_TX_IRQ, IRQ069_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS4 */

#if ( PDL_INTERRUPT_ENABLE_MFS5 == PDL_ON )
extern void MFS5_RX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS5_RX_IRQ )
{
    MFS5_RX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS5_RX_IRQ, IRQ070_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS5 */

#if ( PDL_INTERRUPT_ENABLE_MFS5 == PDL_ON )
extern void MFS5_TX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS5_TX_IRQ )
{
    MFS5_TX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS5_TX_IRQ, IRQ071_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS5 */

#if ( PDL_INTERRUPT_ENABLE_MFS6 == PDL_ON )
extern void MFS6_RX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS6_RX_IRQ )
{
    MFS6_RX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS6_RX_IRQ, IRQ072_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS6 */

#if ( PDL_INTERRUPT_ENABLE_MFS6 == PDL_ON )
extern void MFS6_TX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS6_TX_IRQ )
{
    MFS6_TX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS6_TX_IRQ, IRQ073_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS6 */

#if ( PDL_INTERRUPT_ENABLE_MFS7 == PDL_ON )
extern void MFS7_RX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS7_RX_IRQ )
{
    MFS7_RX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS7_RX_IRQ, IRQ074_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS7 */

#if ( PDL_INTERRUPT_ENABLE_MFS7 == PDL_ON )
extern void MFS7_TX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS7_TX_IRQ )
{
    MFS7_TX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS7_TX_IRQ, IRQ075_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS7 */

#if ( PDL_INTERRUPT_ENABLE_MFS8 == PDL_ON )
extern void MFS8_RX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS8_RX_IRQ )
{
    MFS8_RX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS8_RX_IRQ, IRQ103_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS8 */

#if ( PDL_INTERRUPT_ENABLE_MFS8 == PDL_ON )
extern void MFS8_TX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS8_TX_IRQ )
{
    MFS8_TX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS8_TX_IRQ, IRQ104_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS8 */

#if ( PDL_INTERRUPT_ENABLE_MFS9 == PDL_ON )
extern void MFS9_RX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS9_RX_IRQ )
{
    MFS9_RX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS9_RX_IRQ, IRQ105_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS9 */

#if ( PDL_INTERRUPT_ENABLE_MFS9 == PDL_ON )
extern void MFS9_TX_IRQHandler(void);
WWD_RTOS_DEFINE_ISR( MFS9_TX_IRQ )
{
    MFS9_TX_IRQHandler();
}
WWD_RTOS_MAP_ISR( MFS9_TX_IRQ, IRQ106_Handler )
#endif /* PDL_INTERRUPT_ENABLE_MFS9 */
