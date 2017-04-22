/****************************************************************************
 * include/unistd.h
 *
 *   Copyright (C) 2007-2009, 2013-2014 Gregory Nutt. All rights reserved.
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

#ifndef __INCLUDE_BYTESWAP_H
#define __INCLUDE_BYTESWAP_H

#include <stdint.h>

static __inline uint64_t __bswap64(uint64_t _x)
   {
     return ((_x >> 56) | ((_x >> 40) & 0xff00) | ((_x >> 24) & 0xff0000) |
            ((_x >> 8) & 0xff000000) | ((_x << 8) & ((uint64_t)0xff << 32)) |
            ((_x << 24) & ((uint64_t)0xff << 40)) |
            ((_x << 40) & ((uint64_t)0xff << 48)) | ((_x << 56)));
   }

static __inline uint32_t __bswap32_var(uint32_t v)
   {
     uint32_t t1;

     __asm __volatile("eor %1, %0, %0, ror #16\n"
                      "bic %1, %1, #0x00ff0000\n"
                      "mov %0, %0, ror #8\n"
                      "eor %0, %0, %1, lsr #8\n"
                      : "+r" (v), "=r" (t1));
     return (v);
   }

static __inline uint16_t __bswap16_var(uint16_t v)
  {
    uint32_t ret = v & 0xffff;

    __asm __volatile(
                     "mov    %0, %0, ror #8\n"
                     "orr    %0, %0, %0, lsr #16\n"
                     "bic    %0, %0, %0, lsl #16"
                   : "+r" (ret));

    return ((uint16_t)ret);
  }

#ifdef __OPTIMIZE__

#define __bswap32_constant(x)   \
  ((((x) & 0xff000000U) >> 24) |      \
   (((x) & 0x00ff0000U) >>  8) |      \
   (((x) & 0x0000ff00U) <<  8) |      \
   (((x) & 0x000000ffU) << 24))

#define __bswap16_constant(x)   \
  ((((x) & 0xff00) >> 8) |            \
   (((x) & 0x00ff) << 8))

#define __bswap16(x)    \
  ((uint16_t)(__builtin_constant_p(x) ?     \
   __bswap16_constant(x) :                    \
   __bswap16_var(x)))

#define __bswap32(x)    \
  ((uint32_t)(__builtin_constant_p(x) ?     \
   __bswap32_constant(x) :                    \
   __bswap32_var(x)))

#else
#define __bswap16(x)    __bswap16_var(x)
#define __bswap32(x)    __bswap32_var(x)

#endif /* __OPTIMIZE__ */
#endif /* __INCLUDE_BYTESWAP_H */
