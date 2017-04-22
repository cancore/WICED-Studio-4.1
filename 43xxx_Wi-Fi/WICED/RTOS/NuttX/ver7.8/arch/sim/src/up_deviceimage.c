/****************************************************************************
 * arch/sim/src/up_deviceimage.c
 *
 *   Copyright (C) 2007, 2009, 2014-2015 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
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
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#ifdef VFAT_STANDALONE
# include <stdio.h>
# include <stdlib.h>
# include <zlib.h>
#else
# include <nuttx/config.h>

# include <stdlib.h>
# include <debug.h>
# include <zlib.h>

# include <nuttx/kmalloc.h>

# include "up_internal.h"
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef VFAT_STANDALONE
# define sdbg(format, ...) printf(format, ##__VA_ARGS__)
# define kmm_malloc(size)   malloc(size)
# define kmm_free(mem)     free(mem)
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* This array holds a compressed VFAT file system created with:
 *
 *   /sbin/mkdosfs -C -F 32 -I -n "NuttXTestVol" -S 512 -v nuttx-test.vfat 1024
 *   sudo mkdir /mnt/loop
 *   sudo mount -o loop nuttx-test.vfat /mnt/loop
 *   mkdir /mnt/loop/TestDir
 *   echo "This is a test" >/mnt/loop/TestDir/TestFile.txt
 *   sudo umount /mnt/loop
 *   gzip nuttx-test.vfat
 *   xxd -g 1 nuttx-test.vfat.gz >some-file
 *
 * Then manually massaged from the gzip xxd output to zlib format.  See
 * http://www.faqs.org/rfcs/rfc1952.html.  This amounts to:
 *
 *   Remove all of the leading bytes through the null terminator of the file name
 *   Remove the last 8 bytes
 *   Add 0x08, 0x1d to the beginning.
 */

static const unsigned char g_vfatdata[] =
{
    0x08, 0x1d, 0xed, 0xdd, 0xcf, 0x6a, 0x13, 0x41, 0x18, 0x00, 0xf0, 0xa9, 0x16, 0x5b, 0x2a, 0x6d,
    0x73, 0x12, 0x3c, 0x39, 0x78, 0xf3, 0xb2, 0x87, 0x0a, 0x3d, 0x78, 0x32, 0xd0, 0x06, 0x0a, 0xa2,
    0xa5, 0xa6, 0xa5, 0x17, 0x95, 0x2d, 0xd9, 0x6a, 0x68, 0x4c, 0xda, 0xec, 0x8a, 0x2d, 0x78, 0xf3,
    0x01, 0xf4, 0x39, 0x8a, 0xb7, 0x7a, 0x13, 0xc5, 0x17, 0xe8, 0x5b, 0x78, 0xcb, 0xc5, 0x63, 0x4f,
    0xc6, 0x8d, 0x69, 0x8b, 0x45, 0xc4, 0x3f, 0x20, 0x51, 0xfc, 0xfd, 0xd8, 0xe1, 0xdb, 0xd9, 0x8f,
    0x81, 0x59, 0xe6, 0x63, 0x59, 0xd8, 0x85, 0xe9, 0xad, 0xbf, 0x7c, 0xbc, 0xd5, 0xe8, 0xe4, 0x9b,
    0x79, 0x08, 0xe7, 0xc6, 0x62, 0x38, 0x17, 0x42, 0x98, 0x3c, 0x0a, 0x21, 0x86, 0x9b, 0xe1, 0x44,
    0xe5, 0x38, 0x0e, 0x72, 0x63, 0xe1, 0x42, 0x38, 0xeb, 0xda, 0xf2, 0xbd, 0x3b, 0xb5, 0xdb, 0x4f,
    0x8a, 0x62, 0xbd, 0x9e, 0xe5, 0xc5, 0x5a, 0xa7, 0x56, 0xad, 0x5f, 0x9f, 0x8b, 0x31, 0xce, 0x5c,
    0x79, 0xf7, 0xf4, 0xd9, 0xab, 0xab, 0xef, 0x8b, 0x8b, 0x6b, 0xaf, 0x67, 0xde, 0x4c, 0x84, 0xc3,
    0xca, 0xfd, 0xde, 0xc7, 0xb9, 0x0f, 0x87, 0x97, 0x0e, 0x2f, 0xf7, 0x3e, 0xd5, 0x1f, 0x35, 0xf3,
    0x58, 0x1e, 0xed, 0x4e, 0x11, 0xd3, 0xb8, 0xd1, 0xe9, 0x14, 0xe9, 0x46, 0x2b, 0x8b, 0x8d, 0x66,
    0xbe, 0x95, 0xc4, 0xb8, 0xdc, 0xca, 0xd2, 0x3c, 0x8b, 0xcd, 0x76, 0x9e, 0x75, 0xcf, 0xe4, 0x37,
    0x5b, 0x9d, 0xed, 0xed, 0xbd, 0x98, 0xb6, 0x1b, 0xd3, 0x53, 0xdb, 0xdd, 0x2c, 0xcf, 0xcb, 0xd3,
    0xbd, 0xb8, 0x95, 0xed, 0xc5, 0xa2, 0x13, 0x8b, 0x6e, 0x99, 0x79, 0x98, 0x36, 0xdb, 0x31, 0x49,
    0x92, 0x38, 0x3d, 0x15, 0xf8, 0x91, 0xd5, 0xfd, 0x95, 0x95, 0xb4, 0x3a, 0xea, 0x59, 0xf0, 0x67,
    0x75, 0xbb, 0xd5, 0xf4, 0xed, 0x44, 0x08, 0xe3, 0xdf, 0x64, 0x56, 0xf7, 0x47, 0x30, 0x1d, 0x00,
    0x60, 0xc4, 0x7a, 0xde, 0xff, 0xff, 0x63, 0xde, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x76, 0x47, 0xfd, 0xfe, 0x6c, 0xbf, 0x6c, 0x27, 0x71, 0xd0,
    0x46, 0x3d, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xd7, 0xf8, 0xfe, 0x0f, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xbe, 0xaf, 0x36, 0xee, 0x9c, 0x0c, 0xe1, 0xc6, 0xf3,
    0x83, 0xf9, 0x83, 0xf9, 0x61, 0x1c, 0xe6, 0xab, 0xf5, 0x90, 0x85, 0x3c, 0x14, 0x61, 0x21, 0xcc,
    0x86, 0x07, 0xcd, 0xd0, 0x2d, 0xaf, 0xf5, 0xbf, 0x18, 0xc6, 0xfa, 0xe2, 0xdd, 0xfa, 0xc2, 0xd2,
    0x4a, 0x2c, 0x55, 0x42, 0xe3, 0xc5, 0xf1, 0xf8, 0x41, 0x3c, 0x3f, 0xd2, 0xfb, 0xe2, 0xe7, 0x24,
    0xf1, 0x54, 0x25, 0x84, 0x9d, 0xe3, 0xf5, 0xdb, 0x39, 0x5d, 0xbf, 0x24, 0xf9, 0x5e, 0x7e, 0x38,
    0xfe, 0xb4, 0x3e, 0x6a, 0x65, 0x7d, 0xec, 0x36, 0x43, 0xab, 0xec, 0x26, 0x65, 0x77, 0xb7, 0xcc,
    0x15, 0x65, 0x1b, 0xd4, 0x47, 0x6d, 0xe9, 0xd6, 0x62, 0x7d, 0xbd, 0x1e, 0xcf, 0xd6, 0xc7, 0x78,
    0xf0, 0xa3, 0xc9, 0xe8, 0x9d, 0x6c, 0xc4, 0x9b, 0xc6, 0xa2, 0x7c, 0x08, 0xd8, 0x33, 0x17, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x77, 0x7d, 0x06
};


/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_deviceimage
 *
 * Description: Return an allocated buffer representing an in-memory VFAT
 *   file system.
 *
 ****************************************************************************/

char *up_deviceimage(void)
{
  char    *pbuffer;
  int      bufsize = 1024*1024;
  int      offset  = 0;
  z_stream strm;
  int      ret;

  /* Initialize inflate state */

  strm.zalloc   = Z_NULL;
  strm.zfree    = Z_NULL;
  strm.opaque   = Z_NULL;
  strm.avail_in = 0;
  strm.next_in  = Z_NULL;
  ret           = inflateInit(&strm);
  if (ret != Z_OK)
    {
      sdbg("inflateInit FAILED: ret=%d msg=\"%s\"\n",
           ret, strm.msg ? strm.msg : "No message" );
      return NULL;
    }

  /* Allocate a buffer to hold the decompressed buffer.  We may have to
   * reallocate this a few times to get the size right.
   */

  pbuffer = (char*)kmm_malloc(bufsize);

  /* Set up the input buffer */

  strm.avail_in = sizeof(g_vfatdata);
  strm.next_in = (Bytef*)g_vfatdata;

  /* Run inflate() on input until output buffer not full */

  do
    {
      /* Set up to catch the next output chunk in the output buffer */

      strm.avail_out = bufsize - offset;
      strm.next_out  = (Bytef*)&pbuffer[offset];

      /* inflate */

      ret = inflate(&strm, Z_NO_FLUSH);

      /* Handle inflate() error return values */

      switch (ret)
        {
          case Z_NEED_DICT:
          case Z_DATA_ERROR:
          case Z_MEM_ERROR:
          case Z_STREAM_ERROR:
              sdbg("inflate FAILED: ret=%d msg=\"%s\"\n",
                    ret, strm.msg ? strm.msg : "No message" );
              (void)inflateEnd(&strm);
              kmm_free(pbuffer);
              return NULL;
        }

      /* If avail_out is zero, then inflate() returned only because it is
       * out of buffer space.  In this case, we will have to reallocate
       * the buffer and try again.
       */

      if (strm.avail_out == 0)
        {
          int newbufsize = bufsize + 128*1024;
          char *newbuffer = kmm_realloc(pbuffer, newbufsize);
          if (!newbuffer)
            {
              kmm_free(pbuffer);
              return NULL;
            }
          else
            {
              pbuffer = newbuffer;
              offset  = bufsize;
              bufsize = newbufsize;
            }
        }
      else
        {
          /* There are unused bytes in the buffer, reallocate to
           * correct size.
           */

          int newbufsize = bufsize - strm.avail_out;
          char *newbuffer = kmm_realloc(pbuffer, newbufsize);
          if (!newbuffer)
            {
              kmm_free(pbuffer);
              return NULL;
            }
          else
            {
              pbuffer = newbuffer;
              bufsize = newbufsize;
            }
        }
    }
  while (strm.avail_out == 0 && ret != Z_STREAM_END);

  (void)inflateEnd(&strm);
  return pbuffer;
}

/****************************************************************************
 * Name: main
 *
 * Description: Used to test decompression logic
 *
 *  gcc -g -Wall -DVFAT_STANDALONE -lz -o vfat-test up_deviceimage.c
 *
 ****************************************************************************/

#ifdef VFAT_STANDALONE
int main(int argc, char **argv, char **envp)
{
    char *deviceimage;
    int cmf;
    int fdict;
    int flg;
    int check;

    cmf = g_vfatdata[0];
    printf("CMF=%02x: CM=%d CINFO=%d\n", cmf, cmf &0x0f, cmf >> 4);

    flg   = g_vfatdata[1];
    fdict = (flg >> 5) & 1;

    printf("FLG=%02x: FCHECK=%d FDICT=%d FLEVEL=%d\n", flg, flg &0x1f, fdict, flg >> 6);

    /* The FCHECK value must be such that CMF and FLG, when viewed as
     * a 16-bit unsigned integer stored in MSB order (CMF*256 + FLG),
     * is a multiple of 31.
     */

    check = cmf*256 + flg;
    if (check % 31 != 0)
    {
        printf("Fails check: %04x is not a multiple of 31\n", check);
    }

    deviceimage = up_deviceimage();
    if (deviceimage)
    {
        printf("Inflate SUCCEEDED\n");
        kmm_free(deviceimage);
        return 0;
    }
    else
    {
        printf("Inflate FAILED\n");
        return 1;
    }
}
#endif
