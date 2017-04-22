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

/************************************************************************************
 * drivers/mtd/macronix.c
 * Driver for SPI-based MACRONIX flash.
 *
 *   Copyright (C) 2009-2013 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *           Petteri Aimonen <jpa@nx.mail.kapsi.fi>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ************************************************************************************/

/************************************************************************************
 * Included Files
 ************************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/kmalloc.h>
#include <nuttx/fs/ioctl.h>
#include <nuttx/spi/spi.h>
#include <nuttx/mtd/mtd.h>

/************************************************************************************
 * Pre-processor Definitions
 ************************************************************************************/
/* Configuration ********************************************************************/

#ifndef CONFIG_MXIC_SPIMODE
#  define CONFIG_MXIC_SPIMODE SPIDEV_MODE0
#endif

#ifndef CONFIG_MXIC_SPIFREQUENCY
#  define CONFIG_MXIC_SPIFREQUENCY 20000000
#endif

/* MXIC Registers *******************************************************************/
/* Indentification register values */

#define MXIC_MANUFACTURER         0xC2
#define MXIC_MEMORY_TYPE_25L      0x20

/*  MX25L6433F capacity is 8,388,608 bytes:
 *  (2048 sectors) * (4096 bytes per sector)
 *  (32768 pages) * (256 bytes per page)
 */

#define MXIC_SECTOR_SHIFT  12    /* Sector size 1 << 12 = 4096 */
#define MXIC_NSECTORS      2048
#define MXIC_PAGE_SHIFT    8     /* Page size 1 << 9 = 512 */
#define MXIC_NPAGES        32768

/* Instructions */
/*      Command        Value      N Description             Addr Dummy Data */
#define MXIC_WREN      0x06    /* 1 Write Enable              0   0     0 */
#define MXIC_WRDI      0x04    /* 1 Write Disable             0   0     0 */
#define MXIC_RDID      0x9f    /* 1 Read Identification       0   0     1-3 */
#define MXIC_RDSR      0x05    /* 1 Read Status Register      0   0     >=1 */
#define MXIC_WRSR      0x01    /* 1 Write Status Register     0   0     1 */
#define MXIC_READ      0x03    /* 1 Read Data Bytes           3   0     >=1 */
#define MXIC_FAST_READ 0x0b    /* 1 Higher speed read         3   1     >=1 */
#define MXIC_PP        0x02    /* 1 Page Program              3   0     1-256 */
#define MXIC_SE        0x20    /* 1 Sector Erase              3   0     0 */
#define MXIC_BE        0xc7    /* 1 Bulk Erase                0   0     0 */

/* Status register bit definitions */

#define MXIC_SR_BUSY       (1 << 0)    /* Bit 0: Ready/Busy Status */
#define MXIC_SR_WEL        (1 << 1)    /* Bit 1: Write enable latch bit */

#define MXIC_SR_UNPROT 0x00    /* Global unprotect command */

#define MXIC_DUMMY     0xa5

/************************************************************************************
 * Private Types
 ************************************************************************************/

/* This type represents the state of the MTD device.  The struct mtd_dev_s
 * must appear at the beginning of the definition so that you can freely
 * cast between pointers to struct mtd_dev_s and struct mxic_dev_s.
 */

struct mxic_dev_s
{
  struct mtd_dev_s mtd;      /* MTD interface */
  FAR struct spi_dev_s *dev; /* Saved SPI interface instance */
  uint8_t  sectorshift;      /* 16 or 18 */
  uint8_t  pageshift;        /* 8 */
  uint16_t nsectors;         /* 128 or 64 */
  uint32_t npages;           /* 32,768 or 65,536 */
};

/************************************************************************************
 * Private Function Prototypes
 ************************************************************************************/

/* Helpers */

static void mxic_lock(FAR struct spi_dev_s *dev);
static void mxic_unlock(FAR struct spi_dev_s *dev);
static int mxic_readid(struct mxic_dev_s *priv);
static void mxic_waitwritecomplete(struct mxic_dev_s *priv);
static void mxic_writeenable(struct mxic_dev_s *priv);
static void mxic_sectorerase(struct mxic_dev_s *priv, off_t offset);
static int  mxic_bulkerase(struct mxic_dev_s *priv);
static void mxic_pagewrite(struct mxic_dev_s *priv, FAR const uint8_t *buffer,
                                  off_t offset);

/* MTD driver methods */

static int mxic_erase(FAR struct mtd_dev_s *dev, off_t startblock, size_t nblocks);
static ssize_t mxic_bread(FAR struct mtd_dev_s *dev, off_t startblock,
                          size_t nblocks, FAR uint8_t *buf);
static ssize_t mxic_bwrite(FAR struct mtd_dev_s *dev, off_t startblock,
                           size_t nblocks, FAR const uint8_t *buf);
static ssize_t mxic_read(FAR struct mtd_dev_s *dev, off_t offset, size_t nbytes,
                         FAR uint8_t *buffer);
static int mxic_ioctl(FAR struct mtd_dev_s *dev, int cmd, unsigned long arg);

/************************************************************************************
 * Private Data
 ************************************************************************************/

/************************************************************************************
 * Private Functions
 ************************************************************************************/

/************************************************************************************
 * Name: mxic_lock
 ************************************************************************************/

static void mxic_lock(FAR struct spi_dev_s *dev)
{
  /* On SPI busses where there are multiple devices, it will be necessary to
   * lock SPI to have exclusive access to the busses for a sequence of
   * transfers.  The bus should be locked before the chip is selected.
   *
   * This is a blocking call and will not return until we have exclusiv access to
   * the SPI buss.  We will retain that exclusive access until the bus is unlocked.
   */

  (void)SPI_LOCK(dev, true);

  /* After locking the SPI bus, the we also need call the setfrequency, setbits, and
   * setmode methods to make sure that the SPI is properly configured for the device.
   * If the SPI buss is being shared, then it may have been left in an incompatible
   * state.
   */

  SPI_SETMODE(dev, CONFIG_MXIC_SPIMODE);
  SPI_SETBITS(dev, 8);
  (void)SPI_SETFREQUENCY(dev, CONFIG_MXIC_SPIFREQUENCY);
}

/************************************************************************************
 * Name: mxic_unlock
 ************************************************************************************/

static void mxic_unlock(FAR struct spi_dev_s *dev)
{
  (void)SPI_LOCK(dev, false);
}

/************************************************************************************
 * Name: mxic_readid
 ************************************************************************************/

static int mxic_readid(struct mxic_dev_s *priv)
{
  uint16_t manufacturer;
  uint16_t memory;

  fvdbg("priv: %p\n", priv);

  /* Lock the SPI bus, configure the bus, and select this FLASH part. */

  mxic_lock(priv->dev);
  SPI_SELECT(priv->dev, SPIDEV_FLASH, true);

  /* Send the "Read ID (RDID)" command and read the first three ID bytes */

  (void)SPI_SEND(priv->dev, MXIC_RDID);
  manufacturer = SPI_SEND(priv->dev, MXIC_DUMMY);
  memory       = SPI_SEND(priv->dev, MXIC_DUMMY);

  /* Deselect the FLASH and unlock the bus */

  SPI_SELECT(priv->dev, SPIDEV_FLASH, false);
  mxic_unlock(priv->dev);

  fvdbg("manufacturer: %02x memory: %02x\n",
        manufacturer, memory);

  /* Check for a valid manufacturer and memory type */

  if (manufacturer == MXIC_MANUFACTURER && memory == MXIC_MEMORY_TYPE_25L)
    {
        priv->sectorshift = MXIC_SECTOR_SHIFT;
        priv->nsectors    = MXIC_NSECTORS;
        priv->pageshift   = MXIC_PAGE_SHIFT;
        priv->npages      = MXIC_NPAGES;
        return OK;
    }

  return -ENODEV;
}

/************************************************************************************
 * Name: mxic_waitwritecomplete
 ************************************************************************************/

static void mxic_waitwritecomplete(struct mxic_dev_s *priv)
{
  uint8_t status;

  /* Are we the only device on the bus? */

#ifdef CONFIG_SPI_OWNBUS

  /* Select this FLASH part */

  SPI_SELECT(priv->dev, SPIDEV_FLASH, true);

  /* Send "Read Status Register (RDSR)" command */

  (void)SPI_SEND(priv->dev, MXIC_RDSR);

  /* Loop as long as the memory is busy with a write cycle */

  do
    {
      /* Send a dummy byte to generate the clock needed to shift out the status */

      status = SPI_SEND(priv->dev, MXIC_DUMMY);
    }
  while ((status & MXIC_SR_BUSY) != 0);

  /* Deselect the FLASH */

  SPI_SELECT(priv->dev, SPIDEV_FLASH, false);

#else

  /* Loop as long as the memory is busy with a write cycle */

  do
    {
      /* Select this FLASH part */

      SPI_SELECT(priv->dev, SPIDEV_FLASH, true);

      /* Send "Read Status Register (RDSR)" command */

      (void)SPI_SEND(priv->dev, MXIC_RDSR);

      /* Send a dummy byte to generate the clock needed to shift out the status */

      status = SPI_SEND(priv->dev, MXIC_DUMMY);

      /* Deselect the FLASH */

      SPI_SELECT(priv->dev, SPIDEV_FLASH, false);

      /* Given that writing could take up to few tens of milliseconds, and erasing
       * could take more.  The following short delay in the "busy" case will allow
       * other peripherals to access the SPI bus.
       */

      if ((status & MXIC_SR_BUSY) != 0)
        {
          mxic_unlock(priv->dev);
          usleep(10000);
          mxic_lock(priv->dev);
        }
    }
  while ((status & MXIC_SR_BUSY) != 0);
#endif

  fvdbg("Complete, status: 0x%02x\n", status);
}

/************************************************************************************
 * Name:  mxic_writeenable
 ************************************************************************************/

static void mxic_writeenable(struct mxic_dev_s *priv)
{
  SPI_SELECT(priv->dev, SPIDEV_FLASH, true);
  (void)SPI_SEND(priv->dev, MXIC_WREN);
  SPI_SELECT(priv->dev, SPIDEV_FLASH, false);
  fvdbg("Enabled\n");
}

/************************************************************************************
 * Name:  mxic_sectorerase
 ************************************************************************************/

static void mxic_sectorerase(struct mxic_dev_s *priv, off_t sector)
{
  off_t offset = sector << priv->sectorshift;

  fvdbg("sector: %08lx\n", (long)sector);

  /* Wait for any preceding write to complete.  We could simplify things by
   * perform this wait at the end of each write operation (rather than at
   * the beginning of ALL operations), but have the wait first will slightly
   * improve performance.
   */

  mxic_waitwritecomplete(priv);

  /* Send write enable instruction */

  mxic_writeenable(priv);

  /* Select this FLASH part */

  SPI_SELECT(priv->dev, SPIDEV_FLASH, true);

  /* Send the "Sector Erase (SE)" instruction */

  (void)SPI_SEND(priv->dev, MXIC_SE);

  /* Send the sector offset high byte first.  For all of the supported
   * parts, the sector number is completely contained in the first byte
   * and the values used in the following two bytes don't really matter.
   */

  (void)SPI_SEND(priv->dev, (offset >> 16) & 0xff);
  (void)SPI_SEND(priv->dev, (offset >> 8) & 0xff);
  (void)SPI_SEND(priv->dev, offset & 0xff);

  /* Deselect the FLASH */

  SPI_SELECT(priv->dev, SPIDEV_FLASH, false);
  fvdbg("Erased\n");
}

/************************************************************************************
 * Name:  mxic_bulkerase
 ************************************************************************************/

static int mxic_bulkerase(struct mxic_dev_s *priv)
{
  fvdbg("priv: %p\n", priv);

  /* Wait for any preceding write to complete.  We could simplify things by
   * perform this wait at the end of each write operation (rather than at
   * the beginning of ALL operations), but have the wait first will slightly
   * improve performance.
   */

  mxic_waitwritecomplete(priv);

  /* Send write enable instruction */

  mxic_writeenable(priv);

  /* Select this FLASH part */

  SPI_SELECT(priv->dev, SPIDEV_FLASH, true);

  /* Send the "Bulk Erase (BE)" instruction */

  (void)SPI_SEND(priv->dev, MXIC_BE);

  /* Deselect the FLASH */

  SPI_SELECT(priv->dev, SPIDEV_FLASH, false);
  fvdbg("Return: OK\n");
  return OK;
}

/************************************************************************************
 * Name:  mxic_pagewrite
 ************************************************************************************/

static void mxic_pagewrite(struct mxic_dev_s *priv, FAR const uint8_t *buffer,
                                  off_t page)
{
  off_t offset = page << 8;

  fvdbg("page: %08lx offset: %08lx\n", (long)page, (long)offset);

  /* Wait for any preceding write to complete.  We could simplify things by
   * perform this wait at the end of each write operation (rather than at
   * the beginning of ALL operations), but have the wait first will slightly
   * improve performance.
   */

  mxic_waitwritecomplete(priv);

  /* Enable the write access to the FLASH */

  mxic_writeenable(priv);

  /* Select this FLASH part */

  SPI_SELECT(priv->dev, SPIDEV_FLASH, true);

  /* Send "Page Program (PP)" command */

  (void)SPI_SEND(priv->dev, MXIC_PP);

  /* Send the page offset high byte first. */

  (void)SPI_SEND(priv->dev, (offset >> 16) & 0xff);
  (void)SPI_SEND(priv->dev, (offset >> 8) & 0xff);
  (void)SPI_SEND(priv->dev, offset & 0xff);

  /* Then write the specified number of bytes */

  SPI_SNDBLOCK(priv->dev, buffer, 256);

  /* Deselect the FLASH: Chip Select high */

  SPI_SELECT(priv->dev, SPIDEV_FLASH, false);
  fvdbg("Written\n");
}

/************************************************************************************
 * Name: mxic_erase
 ************************************************************************************/

static int mxic_erase(FAR struct mtd_dev_s *dev, off_t startblock, size_t nblocks)
{
  FAR struct mxic_dev_s *priv = (FAR struct mxic_dev_s *)dev;
  size_t blocksleft = nblocks;

  fvdbg("startblock: %08lx nblocks: %d\n", (long)startblock, (int)nblocks);

  /* Lock access to the SPI bus until we complete the erase */

  mxic_lock(priv->dev);
  while (blocksleft-- > 0)
    {
      /* Erase each sector */

      mxic_sectorerase(priv, startblock);
      startblock++;
    }

  mxic_unlock(priv->dev);
  return (int)nblocks;
}

/************************************************************************************
 * Name: mxic_bread
 ************************************************************************************/

static ssize_t mxic_bread(FAR struct mtd_dev_s *dev, off_t startblock, size_t nblocks,
                          FAR uint8_t *buffer)
{
  FAR struct mxic_dev_s *priv = (FAR struct mxic_dev_s *)dev;
  ssize_t nbytes;

  fvdbg("startblock: %08lx nblocks: %d\n", (long)startblock, (int)nblocks);

  /* On this device, we can handle the block read just like the byte-oriented read */

  nbytes = mxic_read(dev, startblock << priv->pageshift, nblocks << priv->pageshift, buffer);
  if (nbytes > 0)
    {
        return nbytes >> priv->pageshift;
    }

  return (int)nbytes;
}

/************************************************************************************
 * Name: mxic_bwrite
 ************************************************************************************/

static ssize_t mxic_bwrite(FAR struct mtd_dev_s *dev, off_t startblock, size_t nblocks,
                           FAR const uint8_t *buffer)
{
  FAR struct mxic_dev_s *priv = (FAR struct mxic_dev_s *)dev;
  size_t blocksleft = nblocks;

  fvdbg("startblock: %08lx nblocks: %d\n", (long)startblock, (int)nblocks);

  /* Lock the SPI bus and write each page to FLASH */

  mxic_lock(priv->dev);
  while (blocksleft-- > 0)
    {
      mxic_pagewrite(priv, buffer, startblock);
      buffer += 1 << priv->pageshift;
      startblock++;
   }

  mxic_unlock(priv->dev);
  return nblocks;
}

/************************************************************************************
 * Name: mxic_read
 ************************************************************************************/

static ssize_t mxic_read(FAR struct mtd_dev_s *dev, off_t offset, size_t nbytes,
                         FAR uint8_t *buffer)
{
  FAR struct mxic_dev_s *priv = (FAR struct mxic_dev_s *)dev;

  fvdbg("offset: %08lx nbytes: %d\n", (long)offset, (int)nbytes);

  /* Wait for any preceding write to complete.  We could simplify things by
   * perform this wait at the end of each write operation (rather than at
   * the beginning of ALL operations), but have the wait first will slightly
   * improve performance.
   */

  mxic_waitwritecomplete(priv);

  /* Lock the SPI bus and select this FLASH part */

  mxic_lock(priv->dev);
  SPI_SELECT(priv->dev, SPIDEV_FLASH, true);

  /* Send "Read from Memory " instruction */

  (void)SPI_SEND(priv->dev, MXIC_READ);

  /* Send the page offset high byte first. */

  (void)SPI_SEND(priv->dev, (offset >> 16) & 0xff);
  (void)SPI_SEND(priv->dev, (offset >> 8) & 0xff);
  (void)SPI_SEND(priv->dev, offset & 0xff);

  /* Then read all of the requested bytes */

  SPI_RECVBLOCK(priv->dev, buffer, nbytes);

  /* Deselect the FLASH and unlock the SPI bus */

  SPI_SELECT(priv->dev, SPIDEV_FLASH, false);
  mxic_unlock(priv->dev);

  fvdbg("return nbytes: %d\n", (int)nbytes);
  return nbytes;
}

/************************************************************************************
 * Name: mxic_ioctl
 ************************************************************************************/

static int mxic_ioctl(FAR struct mtd_dev_s *dev, int cmd, unsigned long arg)
{
  FAR struct mxic_dev_s *priv = (FAR struct mxic_dev_s *)dev;
  int ret = -EINVAL; /* Assume good command with bad parameters */

  fvdbg("cmd: %d \n", cmd);

  switch (cmd)
    {
      case MTDIOC_GEOMETRY:
        {
          FAR struct mtd_geometry_s *geo = (FAR struct mtd_geometry_s *)((uintptr_t)arg);
          if (geo)
            {
              /* Populate the geometry structure with information need to know
               * the capacity and how to access the device.
               *
               * NOTE: that the device is treated as though it where just an array
               * of fixed size blocks.  That is most likely not true, but the client
               * will expect the device logic to do whatever is necessary to make it
               * appear so.
               */

              geo->blocksize    = (1 << priv->pageshift);
              geo->erasesize    = (1 << priv->sectorshift);
              geo->neraseblocks = priv->nsectors;
              ret               = OK;

              fvdbg("blocksize: %d erasesize: %d neraseblocks: %d\n",
                    geo->blocksize, geo->erasesize, geo->neraseblocks);
            }
        }
        break;

      case MTDIOC_BULKERASE:
        {
            /* Erase the entire device */

            mxic_lock(priv->dev);
            ret = mxic_bulkerase(priv);
            mxic_unlock(priv->dev);
        }
        break;

      case MTDIOC_XIPBASE:
      default:
        ret = -ENOTTY; /* Bad command */
        break;
    }

  fvdbg("return %d\n", ret);
  return ret;
}

/************************************************************************************
 * Public Functions
 ************************************************************************************/

/************************************************************************************
 * Name: mxic_initialize
 *
 * Description:
 *   Create an initialize MTD device instance.  MTD devices are not registered
 *   in the file system, but are created as instances that can be bound to
 *   other functions (such as a block or character driver front end).
 *
 ************************************************************************************/

FAR struct mtd_dev_s *mxic_initialize(FAR struct spi_dev_s *dev)
{
  FAR struct mxic_dev_s *priv;
  int ret;

  fvdbg("dev: %p\n", dev);

  /* Allocate a state structure (we allocate the structure instead of using
   * a fixed, static allocation so that we can handle multiple FLASH devices.
   * The current implementation would handle only one FLASH part per SPI
   * device (only because of the SPIDEV_FLASH definition) and so would have
   * to be extended to handle multiple FLASH parts on the same SPI bus.
   */

  priv = (FAR struct mxic_dev_s *)kmm_zalloc(sizeof(struct mxic_dev_s));
  if (priv)
    {
      /* Initialize the allocated structure (unsupported methods were
       * nullified by kmm_zalloc).
       */

      priv->mtd.erase  = mxic_erase;
      priv->mtd.bread  = mxic_bread;
      priv->mtd.bwrite = mxic_bwrite;
      priv->mtd.read   = mxic_read;
      priv->mtd.ioctl  = mxic_ioctl;
      priv->dev        = dev;

      /* Deselect the FLASH */

      SPI_SELECT(dev, SPIDEV_FLASH, false);

      /* Identify the FLASH chip and get its capacity */

      ret = mxic_readid(priv);
      if (ret != OK)
        {
          /* Unrecognized! Discard all of that work we just did and return NULL */

          fdbg("ERROR: Unrecognized\n");
          kmm_free(priv);
          priv = NULL;
        }
      else
        {
          /* Unprotect all sectors */

          mxic_writeenable(priv);
          SPI_SELECT(priv->dev, SPIDEV_FLASH, true);
          (void)SPI_SEND(priv->dev, MXIC_WRSR);
          (void)SPI_SEND(priv->dev, MXIC_SR_UNPROT);
          SPI_SELECT(priv->dev, SPIDEV_FLASH, false);
        }
    }

  /* Register the MTD with the procfs system if enabled */

#ifdef CONFIG_MTD_REGISTRATION
  mtd_register(&priv->mtd, "mxic");
#endif

  /* Return the implementation-specific state structure as the MTD device */

  fvdbg("Return %p\n", priv);
  return (FAR struct mtd_dev_s *)priv;
}
