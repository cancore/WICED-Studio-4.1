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

/*
 * WICED 4390x SPI Sflash driver
 */
#include "platform_spi_flash.h"
#include <string.h>
#include "crypto_api.h"

#include "wiced_osl.h"
#include <hndsoc.h>
/******************************************************
 *                      Macros
 ******************************************************/
/* Disable Read/Write data via M2M
 *
 * #define INDIRECT_ACCESS 1
 * */

#define TRANSFORM_4BYTES_DATA( data ) (  \
        ( ( data << 24 ) & ( 0xFF000000 ) ) | \
        ( ( data <<  8 ) & ( 0x00FF0000 ) ) | \
        ( ( data >>  8 ) & ( 0x0000FF00 ) ) | \
        ( ( data >> 24 ) & ( 0x000000FF ) ) )

/* QuadAddrMode(bit 24) of SFlashCtrl register only works after Chipcommon Core Revision 55 */
#define CHIP_SUPPORT_QUAD_ADDR_MODE( ccrev ) ( ccrev >= 55  )
#define M2M_START_WRITE ( 0xFFFFFFFF )

#define ALIGNMENT_4BYTES ( 0x03 )

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
 *               Variables Definitions
 ******************************************************/
static uint ccrev;

/* semaphore lock/unlock for sflash */
static host_semaphore_type_t sflash_semaphore;
static wiced_bool_t sflash_semaphore_flag = WICED_FALSE;

/******************************************************
 *               Function Definitions
 ******************************************************/
static int spi_get_actioncode( sflash_command_t command, uint32_t data_length );
static command_purpose_t spi_get_command_purpose( sflash_command_t command );
static int spi_wait_busy_done( busy_type_t busy_type );
#ifndef INDIRECT_ACCESS
static int spi_sflash_read_m2m( bcm43909_sflash_ctrl_reg_t* control_register, uint32_t device_address, void* rx_data, uint32_t data_length, wiced_bool_t read_blocking );
#if ( PLATFORM_NO_SFLASH_WRITE == 0 )
static int spi_sflash_write_m2m( bcm43909_sflash_ctrl_reg_t* control_register, uint32_t device_address, void* tx_data, uint32_t data_length );
#endif /* PLATFORM_NO_SFLASH_WRITE */
#endif /* INDIRECT_ACCESS */

int bcm4390x_sflash_semaphore_init( void )
{
    int result = WICED_SUCCESS;

    /* initialize sflash_semaphore for sflash lock */
    if ( sflash_semaphore_flag == WICED_FALSE )
    {
        result = host_rtos_init_semaphore( &sflash_semaphore );
        if ( result != WICED_SUCCESS )
        {
            WPRINT_APP_DEBUG( ( "semaphore init failed. status=%d\n", result ) );
            return result;
        }
        sflash_semaphore_flag = WICED_TRUE;
        host_rtos_set_semaphore( &sflash_semaphore, WICED_FALSE );
    }
    return result;
}

void bcm4390x_sflash_semaphore_deinit( void )
{
    /* de-initialize sflash_semaphore for sflash lock */
    if ( sflash_semaphore_flag == WICED_TRUE )
    {
        host_rtos_deinit_semaphore( &sflash_semaphore );
        sflash_semaphore_flag = WICED_FALSE;
    }
}

int bcm4390x_sflash_semaphore_get( void )
{
#if defined(TARGETOS_nuttx)
    /* Because the 'long' type ( tv_sec & tv_nsec of timespec ), 0xFFFFFFFF will overflow. */
    return host_rtos_get_semaphore( &sflash_semaphore, ( WICED_NEVER_TIMEOUT / 2 ), WICED_FALSE );
#else
    return host_rtos_get_semaphore( &sflash_semaphore, WICED_NEVER_TIMEOUT, WICED_FALSE );
#endif
}

int bcm4390x_sflash_semaphore_set( void )
{
    return host_rtos_set_semaphore( &sflash_semaphore, WICED_FALSE );
}

void spi_layer_init()
{
    platform_gci_chipcontrol( PMU_CHIPCONTROL_APP_SFLASH_DRIVE_STRENGTH_MASK_REG, 0, PMU_CHIPCONTROL_APP_SFLASH_DRIVE_STRENGTH_MASK );

    /* Get chipc core rev to decide whether 4-bit write supported or not */
    ccrev = osl_get_corerev( CC_CORE_ID );
}

void spi_layer_deinit()
{
    /* To be implement....*/
}

int spi_sflash_send_command( sflash_handle_t* sflash_handle, sflash_command_t command, uint32_t device_address, void* data, uint32_t* data_length )
{
    bcm43909_sflash_ctrl_reg_t control_register;
    uint32_t target_data_length = *data_length;
    uint32_t data_handle_count;
    char* tx_data_pointer;
    char* rx_data_pointer;
    command_purpose_t purpose;

    control_register.raw = 0;

    /* Normal */
    control_register.bits.action_code = spi_get_actioncode( command, target_data_length );
    if ( control_register.bits.action_code == SFLASH_MSG_ERROR )
    {
        return SFLASH_MSG_ERROR;
    }
    purpose = spi_get_command_purpose( command );
    if ( purpose == COMMAND_UNKNOWN_PURPOSE )
    {
        return SFLASH_MSG_ERROR;
    }
    control_register.bits.opcode = command;
    control_register.bits.start_busy = 1;
    PLATFORM_CHIPCOMMON->clock_control.divider.bits.serial_flash_divider = NORMAL_READ_DIVIDER;
    PLATFORM_CHIPCOMMON->sflash.address = device_address & address_masks[ control_register.bits.action_code ];

    switch ( purpose )
    {
        case COMMAND_TXDATA :
            tx_data_pointer = data;

            /* configure High Speed WRITE */
            if ( command == SFLASH_X4IO_WRITE )
            {
                control_register.bits.use_quad_address_mode = ( ( sflash_handle->capabilities->speed_advance->high_speed_write_quad_address_mode == WICED_TRUE ) ? 1 : 0 );
            }

            if ( !CHIP_SUPPORT_QUAD_ADDR_MODE( ccrev ) )
            {
                control_register.bits.use_quad_address_mode = 0;
                command = SFLASH_WRITE;
                WPRINT_APP_DEBUG( ( "!!! Current CORE Revision (0x%x) didn't support Quad mode for Write data !!!\n", ccrev ) );
            }

#if defined(PLATFORM_4390X_OVERCLOCK)
            /* WAR, DO NOT USE sflash Quad mode command !!
             * Issue normal WRITE command(SFLASH_WRITE=0x02) when PLATFORM_4390X_OVERCLOCK enabled,or write data fail. */
            control_register.bits.use_quad_address_mode = 0;
            command = SFLASH_WRITE;
#endif

            /* Write Data to Sflash */
#if ( PLATFORM_NO_SFLASH_WRITE == 0 )
#ifndef INDIRECT_ACCESS
            if ( ( ( control_register.bits.action_code == SFLASH_ACTIONCODE_3ADDRESS_4DATA ) ) &&
                    ( ( target_data_length >= DIRECT_WRITE_BURST_LENGTH ) && ( ( device_address & ALIGNMENT_4BYTES ) == 0x00 ) && ( ( (unsigned long)tx_data_pointer & ALIGNMENT_4BYTES ) == 0x00 ) ) )
            {
                control_register.bits.use_opcode_reg = 1;
                data_handle_count = spi_sflash_write_m2m( &control_register, device_address, tx_data_pointer, target_data_length );
                if ( data_handle_count == SFLASH_MSG_ERROR )
                {
                    return SFLASH_MSG_ERROR;
                }
            }
            else
#endif /* INDIRECT_ACCESS */
#endif /* PLATFORM_NO_SFLASH_WRITE */
            {
                uint32_t data_write = *( ( uint32_t* )tx_data_pointer ) & data_masks[ control_register.bits.action_code ];

                if ( control_register.bits.action_code == SFLASH_ACTIONCODE_3ADDRESS_4DATA )
                {
                    data_write = TRANSFORM_4BYTES_DATA( data_write );
                }
                PLATFORM_CHIPCOMMON->sflash.data = data_write;
                //WPRINT_APP_DEBUG( ( "INDIRECT ACCESS for Write Data\n" ) );

                PLATFORM_CHIPCOMMON->sflash.control.raw = control_register.raw;
                if ( spi_wait_busy_done( SPI_BUSY ) != SFLASH_MSG_OK )
                {
                    return SFLASH_MSG_ERROR;
                }

                data_handle_count = data_bytes[ control_register.bits.action_code ];
            }
            *data_length = data_handle_count;
            break;
        case COMMAND_RXDATA :
            rx_data_pointer = data;

            /* configure FAST / High Speed READ */
            if ( ( command == SFLASH_FAST_READ ) ||( command == SFLASH_QUAD_READ) || ( command == SFLASH_X4IO_READ ) )
            {
                if ( command == SFLASH_FAST_READ )
                {
                    control_register.bits.num_dummy_cycles = sflash_handle->capabilities->speed_advance->fast_read_dummy_cycle;
                }
                else
                {
                    control_register.bits.num_dummy_cycles = sflash_handle->capabilities->speed_advance->high_speed_read_dummy_cycle;
                    control_register.bits.mode_bit_enable = sflash_handle->capabilities->speed_advance->high_speed_read_mode_bit_support;
                }
#if defined(PLATFORM_4390X_OVERCLOCK)
                /* WAR, DO NOT USE sflash Quad mode command !!
                 * Issue normal FAST READ command(SFLASH_FAST_READ=0x0B) when PLATFORM_4390X_OVERCLOCK enabled,or read data fail. */
                command = SFLASH_FAST_READ;
                control_register.bits.num_dummy_cycles = sflash_handle->capabilities->speed_advance->fast_read_dummy_cycle;
#endif
                control_register.bits.high_speed_mode = 1;
                PLATFORM_CHIPCOMMON->clock_control.divider.bits.serial_flash_divider = FAST_READ_DIVIDER;
            }

#ifndef INDIRECT_ACCESS
            control_register.bits.use_opcode_reg = 1;
#endif  /* INDIRECT_ACCESS */
            /* Read Data from Sflash */
            PLATFORM_CHIPCOMMON->sflash.control.raw = control_register.raw;
            if ( spi_wait_busy_done( SPI_BUSY ) != SFLASH_MSG_OK )
            {
                return SFLASH_MSG_ERROR;
            }

#ifndef INDIRECT_ACCESS
            if ( ( control_register.bits.action_code == SFLASH_ACTIONCODE_3ADDRESS_4DATA ) && ( command != SFLASH_READID_JEDEC_ID ) )
            {
                data_handle_count = spi_sflash_read_m2m( &control_register, device_address, rx_data_pointer, target_data_length, sflash_handle->read_blocking );
            }
            else
#endif  /* INDIRECT_ACCESS */
            {
                char* data_point = ( char* ) &PLATFORM_CHIPCOMMON->sflash.data;

                data_handle_count = data_bytes[ control_register.bits.action_code ];
                memcpy( rx_data_pointer, data_point, data_handle_count );
                //WPRINT_APP_DEBUG( ( "INDIRECT ACCESS for Read Data \n" ) );
            }
            *data_length = data_handle_count;
            break;
        case COMMAND_ONLY :
            /* Send command only to Sflash */
            PLATFORM_CHIPCOMMON->sflash.control.raw = control_register.raw;
            if ( spi_wait_busy_done( SPI_BUSY ) != SFLASH_MSG_OK )
            {
                return SFLASH_MSG_ERROR;
            }
            break;
        default :
            WPRINT_APP_DEBUG( ( "!!! Unknown purpose(%d) for command(0x%x)\n", purpose, command ) );
            return SFLASH_MSG_ERROR;
            break;
    }
    return SFLASH_MSG_OK;
}

static int spi_get_actioncode( sflash_command_t command, uint32_t data_length )
{
    if ( ( command == SFLASH_WRITE_STATUS_REGISTER ) ||( command == SFLASH_READ_STATUS_REGISTER) || ( command == SFLASH_READ_STATUS_REGISTER2 ) ||
         ( command == SFLASH_WRITE_DISABLE ) ||( command == SFLASH_WRITE_ENABLE) || ( command == SFLASH_READID_JEDEC_ID ) ||
         ( command == SFLASH_WRITE_ENH_VOLATILE_REGISTER ) || ( command == SFLASH_READ_ENH_VOLATILE_REGISTER ) ||
         ( command == SFLASH_CHIP_ERASE ) ||( command == SFLASH_RESET_ENABLE) || ( command == SFLASH_RESET ) )
    {
         /* - SFLASH_WRITE_STATUS_REGISTER
          * - SFLASH_READ_STATUS_REGISTER
          * - SFLASH_READ_STATUS_REGISTER2
          * - SFLASH_WRITE_DISABLE
          * - SFLASH_WRITE_ENABLE
          * - SFLASH_READID_JEDEC_ID
          * - SFLASH_CHIP_ERASE
          * - SFLASH_RESET_ENABLE
          * - SFLASH_RESET
          * - SFLASH_WRITE_ENH_VOLATILE_REGISTER
          * - SFLASH_READ_ENH_VOLATILE_REGISTER
          * Which will not issue address after command. */
        if ( data_length == 4 )
        {
            return SFLASH_ACTIONCODE_4DATA;
        }
        else if ( data_length == 2 )
        {
            return SFLASH_ACTIONCODE_2DATA;
        }
        else if ( data_length == 1 )
        {
            return SFLASH_ACTIONCODE_1DATA;
        }
        else if ( data_length == 0 )
        {
            return SFLASH_ACTIONCODE_ONLY;
        }
    }
    else if ( ( command == SFLASH_READ ) ||( command == SFLASH_FAST_READ) || ( command == SFLASH_QUAD_READ ) ||
         ( command == SFLASH_X4IO_READ ) ||( command == SFLASH_WRITE) || ( command == SFLASH_QUAD_WRITE ) ||
         ( command == SFLASH_X4IO_WRITE ) ||( command == SFLASH_4K_ERASE) || ( command == SFLASH_64K_ERASE ) )
    {
         /* - SFLASH_READ
          * - SFLASH_FAST_READ
          * - SFLASH_QUAD_READ
          * - SFLASH_X4IO_READ
          * - SFLASH_WRITE
          * - SFLASH_QUAD_WRITE
          * - SFLASH_X4IO_WRITE
          * - SFLASH_4K_ERASE
          * - SFLASH_64K_ERASE
          * Which will issue address after command. */
        if ( data_length >= 4 )
        {
            return SFLASH_ACTIONCODE_3ADDRESS_4DATA;
        }
        else if ( data_length >= 1 )
        {
            return SFLASH_ACTIONCODE_3ADDRESS_1DATA;
        }
        else if ( data_length == 0 )
        {
            return SFLASH_ACTIONCODE_3ADDRESS;
        }
    }
    WPRINT_APP_DEBUG( ( "Not found properly action code for command (0x%x)\n", command ) );
    return SFLASH_MSG_ERROR;
}

static command_purpose_t spi_get_command_purpose( sflash_command_t command )
{
    if ( ( command == SFLASH_WRITE ) || ( command == SFLASH_QUAD_WRITE ) || ( command == SFLASH_X4IO_WRITE) ||
         ( command == SFLASH_WRITE_ENH_VOLATILE_REGISTER ) || ( command == SFLASH_WRITE_STATUS_REGISTER ) )
    {
        return COMMAND_TXDATA;
    }
    else if ( ( command == SFLASH_READ_STATUS_REGISTER ) || ( command == SFLASH_READ_STATUS_REGISTER2 ) ||
         ( command == SFLASH_READ_ENH_VOLATILE_REGISTER) || ( command == SFLASH_READID_JEDEC_ID ) ||
         ( command == SFLASH_READ ) || ( command == SFLASH_FAST_READ ) || ( command == SFLASH_QUAD_READ ) ||
         ( command == SFLASH_X4IO_READ ) )
    {
        return COMMAND_RXDATA;
    }
    else if ( ( command == SFLASH_4K_ERASE ) || ( command == SFLASH_32K_ERASE ) || ( command == SFLASH_64K_ERASE ) ||
         ( command == SFLASH_CHIP_ERASE ) || ( command == SFLASH_RESET_ENABLE ) || ( command == SFLASH_RESET ) ||
         ( command == SFLASH_WRITE_DISABLE) || ( command == SFLASH_WRITE_ENABLE ) )
    {
        return COMMAND_ONLY;
    }
    WPRINT_APP_DEBUG( ( "Not found properly command purpose for command (0x%x)\n", command ) );
    return COMMAND_UNKNOWN_PURPOSE;
}

static int spi_wait_busy_done( busy_type_t busy_type )
{
    uint32_t start_time, end_time;
    uint32_t busy;

    /* Wait for 43909 controller until ready */
    start_time = host_rtos_get_time();
    do
    {
      if ( busy_type == SPI_BUSY )
      {
          busy = PLATFORM_CHIPCOMMON->sflash.control.bits.start_busy;
      } else
      {
          busy = PLATFORM_CHIPCOMMON->sflash.control.bits.backplane_write_dma_busy;
      }

      end_time = host_rtos_get_time();
      if ( end_time - start_time > MAX_TIMEOUT_FOR_43909_BUSY )
      {
          WPRINT_APP_DEBUG( ( "43909 spi controller always busy over %ld ms ! ( default : %d ms)\n", ( end_time - start_time ), MAX_TIMEOUT_FOR_43909_BUSY ) );
          return SFLASH_MSG_ERROR;
      }
    } while ( busy == 1 );

    return SFLASH_MSG_OK;
}

#ifndef INDIRECT_ACCESS
static int spi_sflash_read_m2m( bcm43909_sflash_ctrl_reg_t* control_register, uint32_t device_address, void* rx_data, uint32_t data_length, wiced_bool_t read_blocking )
{
    uint32_t data_handle_count;
    void* direct_address = ( void* ) ( SI_SFLASH + device_address );

    UNUSED_PARAMETER( control_register );
    m2m_unprotected_dma_memcpy( rx_data, direct_address, data_length , read_blocking );
    //WPRINT_APP_DEBUG( ( "M2M for Read Data\n" ) );

    /* Since we can read all data via M2M in one time,
     * so data_length is the amount of handled data. */
    data_handle_count = data_length;

    return data_handle_count;
}

#if (PLATFORM_NO_SFLASH_WRITE == 0)
static int spi_sflash_write_m2m( bcm43909_sflash_ctrl_reg_t* control_register, uint32_t device_address, void* tx_data, uint32_t data_length )
{
    uint8_t num_burst;
    uint32_t i;
    uint32_t data_handle_count = 0;
    uint32_t* src_data_ptr;
    uint32_t* dst_data_ptr;

    num_burst = MIN( MAX_NUM_BURST, data_length / DIRECT_WRITE_BURST_LENGTH );
    data_handle_count = DIRECT_WRITE_BURST_LENGTH * ( 1 << ( num_burst - 1 ) );
    control_register->bits.num_burst = num_burst;
    control_register->bits.start_busy = 0;
    //WPRINT_APP_DEBUG( ( "M2M for Write Data\n" ) );

    PLATFORM_CHIPCOMMON->sflash.control.raw = control_register->raw;

    src_data_ptr = ( uint32_t* ) tx_data;
    dst_data_ptr = ( uint32_t* ) ( SI_SFLASH + device_address );

    for ( i = 0; i < data_handle_count / sizeof(uint32_t); i++ )
    {
        *dst_data_ptr = *src_data_ptr++;
    }

    /* Additional write starts to issue the transaction to the SFLASH */
    *dst_data_ptr = M2M_START_WRITE;

    /* sflash state machine is still running. Do not change any bits in this register while this bit is high */
    if ( spi_wait_busy_done( DMA_BUSY ) != SFLASH_MSG_OK )
    {
        return SFLASH_MSG_ERROR;
    }

    return data_handle_count;
}
#endif /* PLATFORM_NO_SFLASH_WRITE */
#endif /* INDIRECT_ACCESS */
