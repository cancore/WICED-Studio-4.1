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

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <semaphore.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/spi/spi.h>

#include <arch/board/board.h>

#include "up_internal.h"
#include "up_arch.h"

#include "chip.h"
#include "arch/bcm4390x/direct_sflash.h"
#include "peripherals/platform_spi_flash.h"


/************************************************************************************
 * Definitions
 ************************************************************************************/
#define BCM4390X_WREN      0x06    /* 1 Write Enable              0   0     0 */
#define BCM4390X_WRDI      0x04    /* 1 Write Disable             0   0     0 */
#define BCM4390X_RDID      0x9f    /* 1 Read Identification       0   0     1-3 */
#define BCM4390X_RDSR      0x05    /* 1 Read Status Register      0   0     >=1 */
#define BCM4390X_WRSR      0x01    /* 1 Write Status Register     0   0     1 */
#define BCM4390X_READ      0x03    /* 1 Read Data Bytes           3   0     >=1 */
#define BCM4390X_FAST_READ 0x0b    /* 1 Higher speed read         3   1     >=1 */
#define BCM4390X_PP        0x02    /* 1 Page Program              3   0     1-256 */
#define BCM4390X_SE        0x20    /* 1 Sector Erase              3   0     0 */
#define BCM4390X_BE        0xc7    /* 1 Bulk Erase                0   0     0 */
#define BCM4390X_DUMMY     0xa5
#define BCM4390X_NOCOMMAND 0x00    /* no command */

#define DATA_1BYTE_MASK    0xFF
#define STATUS_REGISTER_BUSY (1 << 0)
/************************************************************************************
 * Private Functions
 ************************************************************************************/
static void bcm4390x_spi_sflash_select(FAR struct spi_dev_s *dev, enum spi_dev_e devid, bool selected);
static uint8_t bcm4390x_spi_sflash_status(FAR struct spi_dev_s *dev, enum spi_dev_e devid);

/* SPI methods */
#ifndef CONFIG_SPI_OWNBUS
static int         spi_lock(FAR struct spi_dev_s *dev, bool lock);
#endif
static uint32_t    spi_setfrequency(FAR struct spi_dev_s *dev, uint32_t frequency);
static void        spi_setmode(FAR struct spi_dev_s *dev, enum spi_mode_e mode);
static void        spi_setbits(FAR struct spi_dev_s *dev, int nbits);
static uint16_t    spi_send(FAR struct spi_dev_s *dev, uint16_t wd);
static void        spi_exchange(FAR struct spi_dev_s *dev, FAR const void *txbuffer,
                                FAR void *rxbuffer, size_t nwords);
#ifndef CONFIG_SPI_EXCHANGE
static void        spi_sndblock(FAR struct spi_dev_s *dev, FAR const void *txbuffer,
                                size_t nwords);
static void        spi_recvblock(FAR struct spi_dev_s *dev, FAR void *rxbuffer,
                                 size_t nwords);
#endif

/************************************************************************************
 * Private Types
 ************************************************************************************/
struct bcm4390x_spidev_s
{
  struct spi_dev_s spidev;  /* Externally visible part of the SPI interface */
  uint8_t command;
  uint32_t device_address;
  uint32_t data;
  uint8_t steps;
};

/************************************************************************************
 * Private Data
 ************************************************************************************/
static const struct spi_ops_s g_spi_sflash_ops =
{
#ifndef CONFIG_SPI_OWNBUS
  .lock              = spi_lock,
#endif
  .select            = bcm4390x_spi_sflash_select,
  .setfrequency      = spi_setfrequency,
  .setmode           = spi_setmode,
  .setbits           = spi_setbits,
  .status            = bcm4390x_spi_sflash_status,
  .send              = spi_send,
#ifdef CONFIG_SPI_EXCHANGE
  .exchange          = spi_exchange,
#else
  .sndblock          = spi_sndblock,
  .recvblock         = spi_recvblock,
#endif
  .registercallback  = 0,
};

static struct bcm4390x_spidev_s g_spi_sflash_dev =
{
  .spidev   = { &g_spi_sflash_ops },
};

/* Used for SPI Sflash layer of bcm4390x */
static sflash_handle_t sflash_handle;
static sflash_capabilities_t sflash_capability;
static sflash_speed_advance_t sflash_speed_advance;

/************************************************************************************
 * Name: bcm4390x_spi_sflash_select
 *
 * Description:
 *   Used for init & de-init SPI Sflash layer of bcm4390x.
 *
 * Input Parameters:
 *   dev  - Device-specific state data
 *   devid - Device ID
 *   selected - True for initialize device; False for de-initialize device
 *
 * Returned Value:
 *   None
 *
 ************************************************************************************/

void bcm4390x_spi_sflash_select(FAR struct spi_dev_s *dev, enum spi_dev_e devid, bool selected)
{
    UNUSED( dev );
    UNUSED( devid );

    if ( selected )
    {
        spi_layer_init();
        sflash_capability.speed_advance = &sflash_speed_advance;
        sflash_handle.capabilities = &sflash_capability;
    }
    else
    {
        spi_layer_deinit();
        g_spi_sflash_dev.command = 0;
        g_spi_sflash_dev.data = 0;
        g_spi_sflash_dev.device_address = 0;
        g_spi_sflash_dev.steps = 0;

        memset(&sflash_handle, 0, sizeof(sflash_handle_t));
        memset(&sflash_capability, 0, sizeof(sflash_capabilities_t));
        memset(&sflash_speed_advance, 0, sizeof(sflash_speed_advance_t));
    }
}

uint8_t bcm4390x_spi_sflash_status(FAR struct spi_dev_s *dev, enum spi_dev_e devid)
{
  return 0;
}

/************************************************************************************
 * Name: spi_lock
 *
 * Description:
 *   On SPI busses where there are multiple devices, it will be necessary to
 *   lock SPI to have exclusive access to the busses for a sequence of
 *   transfers.  The bus should be locked before the chip is selected. After
 *   locking the SPI bus, the caller should then also call the setfrequency,
 *   setbits, and setmode methods to make sure that the SPI is properly
 *   configured for the device.  If the SPI buss is being shared, then it
 *   may have been left in an incompatible state.
 *
 * Input Parameters:
 *   dev  - Device-specific state data
 *   lock - true: Lock spi bus, false: unlock SPI bus
 *
 * Returned Value:
 *   None
 *
 ************************************************************************************/

#ifndef CONFIG_SPI_OWNBUS
static int spi_lock(FAR struct spi_dev_s *dev, bool lock)
{
    UNUSED( dev );

    if (lock)
      {
        /* Take the semaphore (perhaps waiting) */
        bcm4390x_sflash_semaphore_get();
      }
    else
      {
        bcm4390x_sflash_semaphore_set();
      }
    return OK;
}
#endif

/************************************************************************************
 * Name: spi_setfrequency
 *
 * Description:
 *   Set the SPI frequency.
 *
 * Input Parameters:
 *   dev -       Device-specific state data
 *   frequency - The SPI frequency requested
 *
 * Returned Value:
 *   Returns the actual frequency selected
 *
 ************************************************************************************/

static uint32_t spi_setfrequency(FAR struct spi_dev_s *dev, uint32_t frequency)
{
   /* SPI Sflash layer of bcm4390x won't need to do it ... */
   return 0;
}

/************************************************************************************
 * Name: spi_setmode
 *
 * Description:
 *   Set the SPI mode.  see enum spi_mode_e for mode definitions
 *
 * Input Parameters:
 *   dev  - Device-specific state data
 *   mode - The SPI mode requested
 *
 * Returned Value:
 *   Returns the actual frequency selected
 *
 ************************************************************************************/

static void spi_setmode(FAR struct spi_dev_s *dev, enum spi_mode_e mode)
{
    /* SPI Sflash layer of bcm4390x won't need to do it ... */
}

/************************************************************************************
 * Name: spi_setbits
 *
 * Description:
 *   Set the number of bits per word.
 *
 * Input Parameters:
 *   dev   - Device-specific state data
 *   nbits - The number of bits requested
 *
 * Returned Value:
 *   None
 *
 ************************************************************************************/

static void spi_setbits(FAR struct spi_dev_s *dev, int nbits)
{
    /* SPI Sflash layer of bcm4390x won't need to do it ... */
}

/************************************************************************************
 * Name: spi_send
 *
 * Description:
 *   Exchange one word on SPI
 *
 * Input Parameters:
 *   dev - Device-specific state data
 *   wd  - The word to send.  the size of the data is determined by the
 *         number of bits selected for the SPI interface.
 *
 * Returned Value:
 *   response
 *
 ************************************************************************************/

static uint16_t spi_send(FAR struct spi_dev_s *dev, uint16_t wd)
{
    bool command_stage = FALSE;
    uint32_t id_result = 0;

    UNUSED( dev );

    if ( g_spi_sflash_dev.command == BCM4390X_NOCOMMAND )
    {
        g_spi_sflash_dev.command = wd;
        command_stage = TRUE;
    }

    if ( ( g_spi_sflash_dev.command == BCM4390X_WREN ) ||
         ( g_spi_sflash_dev.command == BCM4390X_WRDI ) ||
         ( g_spi_sflash_dev.command == BCM4390X_BE ) )
    {
        /* issue command right away !!! */
        spi_exchange(dev, NULL, NULL, 0);
    }
    else if ( g_spi_sflash_dev.command == BCM4390X_RDID )
    {
        if ( command_stage == TRUE )
        {
            /* issue command right away and get sflash id !!! */
            spi_exchange(dev, NULL, &g_spi_sflash_dev.data, sizeof(size_t));
        }
        else
        {
            /* pop out sflash id byte by byte. */
            id_result = ( g_spi_sflash_dev.data >> ( g_spi_sflash_dev.steps * 8) ) & 0xFF;

            g_spi_sflash_dev.steps++;
        }
        return id_result;
    }
    else if ( g_spi_sflash_dev.command == BCM4390X_RDSR )
    {
        if ( command_stage == TRUE )
        {
            /* issue command right away and get value of status register !!! */
            spi_exchange(dev, NULL, &g_spi_sflash_dev.data, sizeof(size_t));
        }
        else
        {
            /* pop out value of status register. */
            return (g_spi_sflash_dev.data & DATA_1BYTE_MASK);
        }
    }
    else if ( g_spi_sflash_dev.command == BCM4390X_WRSR )
    {
        if ( command_stage != TRUE )
        {
            g_spi_sflash_dev.data = wd;
            /* wait for data (wd) and issue command !!! */
            spi_exchange(dev, &g_spi_sflash_dev.data, NULL, sizeof(size_t));
        }
    }
    else if ( ( g_spi_sflash_dev.command == BCM4390X_FAST_READ ) ||
              ( g_spi_sflash_dev.command == BCM4390X_READ ) ||
              ( g_spi_sflash_dev.command == BCM4390X_PP ) ||
              ( g_spi_sflash_dev.command == BCM4390X_SE ) )
    {
        if ( command_stage != TRUE )
        {
            g_spi_sflash_dev.device_address |= ( ( wd & 0xFF ) << ( 16 - ( g_spi_sflash_dev.steps * 8 ) ) );

            if ( ( g_spi_sflash_dev.steps == 2 ) && ( g_spi_sflash_dev.command == BCM4390X_SE ) )
            {
                spi_exchange(dev, NULL, NULL, 0);
            }

            if ( ( g_spi_sflash_dev.steps == 0 ) && ( g_spi_sflash_dev.command == BCM4390X_FAST_READ ) )
            {
                sflash_speed_advance.fast_read_dummy_cycle = 8;
            }

            g_spi_sflash_dev.steps++;
        }
    }

    return 0;
}

/************************************************************************************
 * Name: spi_exchange
 *
 * Description:
 *   Exchange a block of data on SPI
 *
 * Input Parameters:
 *   dev      - Device-specific state data
 *   txbuffer - A pointer to the buffer of data to be sent
 *   rxbuffer - A pointer to a buffer in which to receive data
 *   nwords   - the length of data to be exchaned in units of words.
 *              The wordsize is determined by the number of bits-per-word
 *              selected for the SPI interface.  If nbits <= 8, the data is
 *              packed into uint8_t's; if nbits >8, the data is packed into uint16_t's
 *
 * Returned Value:
 *   None
 *
 ************************************************************************************/

static void spi_exchange(FAR struct spi_dev_s *dev, FAR const void *txbuffer,
                         FAR void *rxbuffer, size_t nwords)
{
    UNUSED( dev );
    uint32_t data_len = nwords;

    do
    {
        if ( txbuffer == NULL )
        {
            spi_sflash_send_command( &sflash_handle, g_spi_sflash_dev.command, g_spi_sflash_dev.device_address, rxbuffer, &nwords );
            rxbuffer += nwords;
        }
        else
        {
            spi_sflash_send_command( &sflash_handle, g_spi_sflash_dev.command, g_spi_sflash_dev.device_address, ( void* )txbuffer, &nwords );
            txbuffer += nwords;
        }
        data_len -= nwords;
        g_spi_sflash_dev.device_address += nwords;


        /* if data_len > 0, it means we still have data need to write to sflash. */
        if ( ( data_len > 0 ) && ( txbuffer != NULL ) )
        {
            uint8_t status_register;
            uint32_t tmp_len;

            tmp_len = 1;
            do
            {
                spi_sflash_send_command( NULL, BCM4390X_RDSR, 0, &status_register, &tmp_len );
            } while ((status_register & STATUS_REGISTER_BUSY) != 0);

            tmp_len = 0;
            spi_sflash_send_command( NULL, BCM4390X_WREN, 0, NULL, &tmp_len );
        }
    } while ( data_len > 0);
}

/*************************************************************************
 * Name: spi_sndblock
 *
 * Description:
 *   Send a block of data on SPI
 *
 * Input Parameters:
 *   dev      - Device-specific state data
 *   txbuffer - A pointer to the buffer of data to be sent
 *   nwords   - the length of data to send from the buffer in number of words.
 *              The wordsize is determined by the number of bits-per-word
 *              selected for the SPI interface.  If nbits <= 8, the data is
 *              packed into uint8_t's; if nbits >8, the data is packed into uint16_t's
 *
 * Returned Value:
 *   None
 *
 ************************************************************************************/

#ifndef CONFIG_SPI_EXCHANGE
static void spi_sndblock(FAR struct spi_dev_s *dev, FAR const void *txbuffer, size_t nwords)
{
  return spi_exchange(dev, txbuffer, NULL, nwords);
}
#endif

/************************************************************************************
 * Name: spi_recvblock
 *
 * Description:
 *   Receive a block of data from SPI
 *
 * Input Parameters:
 *   dev      - Device-specific state data
 *   rxbuffer - A pointer to the buffer in which to recieve data
 *   nwords   - the length of data that can be received in the buffer in number
 *              of words.  The wordsize is determined by the number of bits-per-word
 *              selected for the SPI interface.  If nbits <= 8, the data is
 *              packed into uint8_t's; if nbits >8, the data is packed into uint16_t's
 *
 * Returned Value:
 *   None
 *
 ************************************************************************************/

#ifndef CONFIG_SPI_EXCHANGE
static void spi_recvblock(FAR struct spi_dev_s *dev, FAR void *rxbuffer, size_t nwords)
{
  return spi_exchange(dev, NULL, rxbuffer, nwords);
}
#endif

/************************************************************************************
 * Public Functions
 ************************************************************************************/

/************************************************************************************
 * Name: up_bcm4390x_sflash_spiinitialize
 *
 * Description:
 *   Initialize the specific sflash interface of BCM4390x
 *
 * Returned Value:
 *   Valid SPI device structure reference on succcess; a NULL on failure
 *
 ************************************************************************************/

struct spi_dev_s *up_bcm4390x_sflash_spiinitialize( void )
{
  FAR struct bcm4390x_spidev_s *priv = NULL;

  priv = &g_spi_sflash_dev;

#ifndef CONFIG_SPI_OWNBUS
  /* Initialize the SPI semaphore that enforces mutually exclusive
   * access to the SPI registers.
   */
  bcm4390x_sflash_semaphore_init();
#endif

  return (FAR struct spi_dev_s *)priv;
}
