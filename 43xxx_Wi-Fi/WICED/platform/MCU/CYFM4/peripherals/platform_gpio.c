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
 * FM4 common GPIO implementation
 */
#include "stdint.h"
#include "string.h"
#include "platform_peripheral.h"
#include "platform_isr.h"
#include "platform_isr_interface.h"
#include "wwd_rtos_isr.h"
#include "wwd_assert.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define DEFINE_GPIO_IRQ_HANDLER_WRAPPER(x) \
static void gpio_irq_handler_wrapper_##x( void ); \
static void gpio_irq_handler_wrapper_##x( void ) \
{ \
    if( gpio_irq_data[x].handler != NULL ) \
    { \
        gpio_irq_data[x].handler( gpio_irq_data[x].arg ); \
    } \
}

#define ARRAY_SIZE(a)                                ( sizeof(a) / sizeof(a[0]) )

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

/* Structure of runtime GPIO IRQ data */
typedef struct
{
    platform_gpio_irq_callback_t handler;    // User callback
    void*                        arg;        // User argument to be passed to the callback
} platform_gpio_irq_data_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

static platform_gpio_irq_data_t gpio_irq_data[PDL_EXINT_INSTANCE_COUNT];

/* FM4 GPIO library doesn't provide mechanism to disable individual interrupt lines
 * So, we store all the currently enabled interrupts in stc_exint_config
 */
static stc_exint_config_t stc_exint_config;

/* FM4 GPIO IRQ callback functions won't take an argument as input parameter
 * But, WICED APIs expect callback functions take one void type argument.
 * So, an array of wrapper functions are created to make WICED APIs work with FM4
 * interrupt callback routines
 */
DEFINE_GPIO_IRQ_HANDLER_WRAPPER(0)
DEFINE_GPIO_IRQ_HANDLER_WRAPPER(1)
func_ptr_t gpio_irq_handler_wrapper_fn[] =
{
    gpio_irq_handler_wrapper_0,
    gpio_irq_handler_wrapper_1
};

/******************************************************
 *            Platform Function Definitions
 ******************************************************/

platform_result_t platform_gpio_init( const platform_gpio_t* gpio, platform_pin_config_t config )
{
    platform_result_t ret;

    wiced_assert("bad argument", ( gpio != NULL ) && ( gpio->PCRx != NULL ) && ( gpio->DDRx != NULL )
            && ( gpio->PFRx != NULL ) && ( gpio->PDORx != NULL ));

    platform_mcu_powersave_disable();

    switch ( config )
    {
        case INPUT_PULL_UP:
            *( gpio->PCRx ) = 1;
            *( gpio->DDRx ) = 0;
            *( gpio->PFRx ) = 0;
            if ( gpio->ADE != NULL )
            {
                *( gpio->ADE )  = 0;
            }
            ret = PLATFORM_SUCCESS;
            break;

        case INPUT_HIGH_IMPEDANCE: /* FM4 GPIOs doesn't support HIGH_IMPEDANCE mode. So, mapping it to PULL_DOWN */
        case INPUT_PULL_DOWN:
            *( gpio->PCRx ) = 0;
            *( gpio->DDRx ) = 0;
            *( gpio->PFRx ) = 0;
            if ( gpio->ADE != NULL )
            {
                *( gpio->ADE )  = 0;
            }
            ret = PLATFORM_SUCCESS;
            break;

        case OUTPUT_OPEN_DRAIN_PULL_UP:
            if ( gpio->PZRx != NULL )
            {
                *( gpio->PZRx )  = 1;
            }
            *( gpio->PDORx ) = 0; /* Output driven low by default */
            *( gpio->DDRx )  = 1;
            *( gpio->PFRx )  = 0;
            if ( gpio->ADE != NULL )
            {
                *( gpio->ADE )   = 0;
            }
            ret = PLATFORM_SUCCESS;
            break;

        case OUTPUT_PUSH_PULL:  /* FM4 GPIOs doesn't support PUSH_PULL mode. So, mapping it to OPEN_DRAIN */
        case OUTPUT_OPEN_DRAIN_NO_PULL:
            if ( gpio->PZRx != NULL )
            {
                *( gpio->PZRx )  = 0;
            }
            *( gpio->PDORx ) = 0; /* Output driven low by default */
            *( gpio->DDRx )  = 1;
            *( gpio->PFRx )  = 0;
            if ( gpio->ADE != NULL )
            {
                *( gpio->ADE )   = 0;
            }
            ret = PLATFORM_SUCCESS;
            break;

        default:
            ret = PLATFORM_UNSUPPORTED;
            break;
    }

    platform_mcu_powersave_enable();
    return ret;
}

platform_result_t platform_gpio_deinit( const platform_gpio_t* gpio )
{
    wiced_assert( "bad argument", ( gpio != NULL ) );
    /* Configure GPIO to input mode if disabled */
    return platform_gpio_init( gpio, INPUT_HIGH_IMPEDANCE );
}

platform_result_t platform_gpio_output_high( const platform_gpio_t* gpio )
{
    wiced_assert("bad argument", ( gpio != NULL ) && ( gpio->PDORx != NULL ));

    platform_mcu_powersave_disable( );

    *( gpio->PDORx ) = 1;

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_output_low( const platform_gpio_t* gpio )
{
    wiced_assert("bad argument", ( gpio != NULL ) && ( gpio->PDORx != NULL ));

    platform_mcu_powersave_disable( );

    *( gpio->PDORx ) = 0;

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

wiced_bool_t platform_gpio_input_get( const platform_gpio_t* gpio )
{
    wiced_bool_t result;

    wiced_assert("bad argument", ( gpio != NULL ) && ( gpio->PDIRx != NULL ));

    platform_mcu_powersave_disable();

    result = ( (*( gpio->PDIRx )) != 0 ) ? WICED_TRUE : WICED_FALSE;

    platform_mcu_powersave_enable();

    return result;
}

platform_result_t platform_gpio_irq_enable( const platform_gpio_t* gpio, platform_gpio_irq_trigger_t trigger, platform_gpio_irq_callback_t handler, void* arg )
{
    wiced_assert( "bad argument", ( gpio != NULL ) && ( gpio->intr_conf_fn != NULL ) && ( gpio->index < ExintInstanceIndexMax ) && ( handler != NULL ) && ( gpio_irq_handler_wrapper_fn[gpio->index] != NULL ));

    /* The internal pull-up resistance can be connected for the external
    interrupt pin with falling edge detection if necessary */
    /* E.g.: Gpio1pin_InitIn ( GPIO1PIN_Pxy, Gpio1pin_InitPullup(1u)); */

    /* Before initialize external interrupt, make sure
     * PDL_PERIPHERAL_ENABLE_EXINTxx is set to PDL_ON in pdl_user.h
     */
    switch ( trigger )
    {
        case IRQ_TRIGGER_RISING_EDGE:
        {
            stc_exint_config.aenLevel[gpio->index] = ExIntRisingEdge;
            break;
        }
        case IRQ_TRIGGER_FALLING_EDGE:
        {
            stc_exint_config.aenLevel[gpio->index] = ExIntFallingEdge;
            break;
        }
        case IRQ_TRIGGER_BOTH_EDGES:
        {
            stc_exint_config.aenLevel[gpio->index] = ExIntBothEdge;
            break;
        }
        case IRQ_TRIGGER_LEVEL_HIGH:
        {
            stc_exint_config.aenLevel[gpio->index] = ExIntHighLevel;
            break;
        }
        case IRQ_TRIGGER_LEVEL_LOW:
        {
            stc_exint_config.aenLevel[gpio->index] = ExIntLowLevel;
            break;
        }
        default:
        {
            return PLATFORM_BADARG;
        }
    }

    stc_exint_config.abEnable[gpio->index] = TRUE;

    platform_mcu_powersave_disable();
    WICED_DISABLE_INTERRUPTS();
    /* FM4 GPIO IRQ callback functions won't take an argument as input parameter
     * But, WICED APIs expect callback functions take one void type argument
     * So, an array of wrapper functions are created to make WICED APIs work with FM4
     * GPIO interrupt callback routines
     */
    gpio_irq_data[gpio->index].handler = handler;
    gpio_irq_data[gpio->index].arg     = arg;
    stc_exint_config.apfnExintCallback[gpio->index] = gpio_irq_handler_wrapper_fn[gpio->index];
    stc_exint_config.bTouchNvic = TRUE;

    /* Configure GPIO pin for interrupt operation */
    gpio->intr_conf_fn();
    Exint_Init(&stc_exint_config);

    WICED_ENABLE_INTERRUPTS();
    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_irq_disable( const platform_gpio_t* gpio )
{
    uint32_t i;

    platform_mcu_powersave_disable();
    WICED_DISABLE_INTERRUPTS();

    stc_exint_config.abEnable[gpio->index] = FALSE;
    stc_exint_config.apfnExintCallback[gpio->index] = NULL;

    /* Check whether any interrupt is enabled. If not, disable NVIC */
    stc_exint_config.bTouchNvic = FALSE;
    for( i = 0; i < ExintInstanceIndexMax; i++)
    {
        if( stc_exint_config.abEnable[i] == TRUE )
        {
            stc_exint_config.bTouchNvic = TRUE;
            break;
        }
    }

    /* Disable all interrupts & re-enable currently active interrupts */
    Exint_DeInit();
    Exint_Init(&stc_exint_config);

    WICED_ENABLE_INTERRUPTS();
    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_irq_manager_init( void )
{
    wiced_assert( "Not enough entries in gpio_irq_handler_wrapper_fn[]", ( ARRAY_SIZE(gpio_irq_handler_wrapper_fn) <= PDL_EXINT_INSTANCE_COUNT ) );

    WICED_DISABLE_INTERRUPTS();

    memset( (void*)gpio_irq_data, 0, sizeof( gpio_irq_data ) );
    PDL_ZERO_STRUCT(stc_exint_config);

    WICED_ENABLE_INTERRUPTS();

    return PLATFORM_SUCCESS;
}

/******************************************************
 *               IRQ Handler Definitions
 ******************************************************/

/******************************************************
 *               IRQ Handler Mapping
 ******************************************************/
