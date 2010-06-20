
/*
** This file is part of PerTwisted, a persistant storage framework
** tailored around the Twisted asynchronous network programming framework,
** utilizing the Berkeley Database, Cononical's Storm ORM, implementations
** of various DHTs and hopefully much more.
**
** Pertwisted is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** PerTwisted is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** and the GNU Lesser Public License along with PerTwisted.  If not, see
** <http://www.gnu.org/licenses/>.
*/


/***********************************************************************\
 * This file, though part of PerTwisted, is partially lifted, persuant *
 * to the following copyright, conditions, and disclaimer.             *
 *   @ Jeremy R. Fishman @                                             *
\***********************************************************************/


// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.
//
// This implementation is heavily optimized to make reads and writes
// of small values (especially varints) as fast as possible.  In
// particular, we optimize for the common case that a read or a write
// will not cross the end of the buffer, since we can avoid a lot
// of branching in this case.


// @ Jeremy R. Fishman @
// Actually, this implementation is so heavily optimized that no check is made
// regarding crossing the end of any buffers.  All buffers are assumed to
// contain enough bytes (<=10) to store the varint.

#include <assert.h>
#include "varint.h"

inline vi_size_t
vi_to_uint64(const varint *vi, uint64_t *value)
{
    uint64_t b;
    vi_size_t len;

    // Splitting into 32-bit pieces gives better performance on 32-bit
    // processors.
    uint32_t part0 = 0, part1 = 0, part2 = 0;

    b = *(vi++); part0  = (b & 0x7F)      ;
        if (!(b & 0x80)) { len =  1; goto done; }

    b = *(vi++); part0 |= (b & 0x7F) <<  7;
        if (!(b & 0x80)) { len =  2; goto done; }

    b = *(vi++); part0 |= (b & 0x7F) << 14;
        if (!(b & 0x80)) { len =  3; goto done; }

    b = *(vi++); part0 |= (b & 0x7F) << 21;
        if (!(b & 0x80)) { len =  4; goto done; }

    b = *(vi++); part1  = (b & 0x7F)      ;
        if (!(b & 0x80)) { len =  5; goto done; }

    b = *(vi++); part1 |= (b & 0x7F) <<  7;
        if (!(b & 0x80)) { len =  6; goto done; }

    b = *(vi++); part1 |= (b & 0x7F) << 14;
        if (!(b & 0x80)) { len =  7; goto done; }

    b = *(vi++); part1 |= (b & 0x7F) << 21;
        if (!(b & 0x80)) { len =  8; goto done; }

    b = *(vi++); part2  = (b & 0x7F)      ;
        if (!(b & 0x80)) { len =  9; goto done; }

    /* @jfishman - I believe this will truncate varints > 64 bits,
    **             though I'm not 110% sure on that point. */
    b = *(vi++); part2 |= (b & 0x7F) <<  7;
        if (!(b & 0x80)) { len = 10; goto done; }

    // We have overrun the maximum size of a varint (10 bytes).  The data
    // must be corrupt.
    return 0;

   done:
    *value = (((uint64_t)part0)      ) |
             (((uint64_t)part1) << 28) |
             (((uint64_t)part2) << 56);
    return len;
}

inline vi_size_t
vi_to_uint64_2(const varint *vi, uint64_t *value, size_t maxlen)
{
    uint64_t b;
    vi_size_t len;

    if (!maxlen)
        // must have at least one byte
        return 0;

    // Splitting into 32-bit pieces gives better performance on 32-bit
    // processors.
    uint32_t part0 = 0, part1 = 0, part2 = 0;

    b = *(vi++); part0  = (b & 0x7F)      ;
        if (!(b & 0x80)) { len =  1; goto done; }
        if (maxlen == 1) { goto fail; }

    b = *(vi++); part0 |= (b & 0x7F) <<  7;
        if (!(b & 0x80)) { len =  2; goto done; }
        if (maxlen == 2) { goto fail; }

    b = *(vi++); part0 |= (b & 0x7F) << 14;
        if (!(b & 0x80)) { len =  3; goto done; }
        if (maxlen == 3) { goto fail; }

    b = *(vi++); part0 |= (b & 0x7F) << 21;
        if (!(b & 0x80)) { len =  4; goto done; }
        if (maxlen == 4) { goto fail; }

    b = *(vi++); part1  = (b & 0x7F)      ;
        if (!(b & 0x80)) { len =  5; goto done; }
        if (maxlen == 5) { goto fail; }

    b = *(vi++); part1 |= (b & 0x7F) <<  7;
        if (!(b & 0x80)) { len =  6; goto done; }
        if (maxlen == 6) { goto fail; }

    b = *(vi++); part1 |= (b & 0x7F) << 14;
        if (!(b & 0x80)) { len =  7; goto done; }
        if (maxlen == 7) { goto fail; }

    b = *(vi++); part1 |= (b & 0x7F) << 21;
        if (!(b & 0x80)) { len =  8; goto done; }
        if (maxlen == 8) { goto fail; }

    b = *(vi++); part2  = (b & 0x7F)      ;
        if (!(b & 0x80)) { len =  9; goto done; }
        if (maxlen == 9) { goto fail; }

    /* @jfishman - I believe this will truncate varints > 64 bits,
    **             though I'm not 110% sure on that point. */
    b = *(vi++); part2 |= (b & 0x7F) <<  7;
        if (!(b & 0x80)) { len = 10; goto done; }

  fail:
    // We have overrun the maximum length specified.  The data
    // must be corrupt.
    return 0;

  done:
    *value = (((uint64_t)part0)      ) |
             (((uint64_t)part1) << 28) |
             (((uint64_t)part2) << 56);
    return len;
}

vi_size_t
uint64_log2(uint64_t value)
{
    uint32_t tmp;

    /*
     * Use the Google Protocol Buffers approach - binary search tree
     *   except we need to go deeper for log2 precision
     */

#define uint8_log2(value) ( \
    ((value < 16) \
    ? ((value < 4) \
      ? ((value >> 1)) \
      : ((value >> 3) + 2)) \
    : ((value < 64) \
      ? ((value >> 5) + 4) \
      : ((value >> 7) + 6))))

    tmp = (uint32_t)(value >> 32);
    if (tmp) {
      if (tmp < (1 << 16)) {
        if (tmp < (1 << 8)) {
          return uint8_log2(tmp) + 32;
        }else {
          tmp >>= 8;
          return uint8_log2(tmp) + 40;
        }
      }else {
        if (tmp < (1 << 24)) {
          tmp >>= 16;
          return uint8_log2(tmp) + 48;
        } else {
          tmp >>= 24;
          return uint8_log2(tmp) + 52;
        }
      }
    } else {
      tmp = (uint32_t)value;
      if (tmp < (1 << 16)) {
        if (tmp < (1 << 8)) {
          return uint8_log2(tmp);
        }else {
          tmp >>= 8;
          return uint8_log2(tmp) + 8;
        }
      }else {
        if (tmp < (1 << 24)) {
          tmp >>= 16;
          return uint8_log2(tmp) + 16;
        } else {
          tmp >>= 24;
          return uint8_log2(tmp) + 24;
        }
      }
    }
}

inline vi_size_t
uint64_len(uint64_t value)
{
    /*
     * Use the Google Protocol Buffers approach - binary search tree
     */
    uint32_t part0 = (uint32_t)(value      );
    uint32_t part1 = (uint32_t)(value >> 28);
    uint32_t part2 = (uint32_t)(value >> 56);

    // Here we can't really optimize for small numbers, since the value is
    // split into three parts.  Checking for numbers < 128, for instance,
    // would require three comparisons, since you'd have to make sure part1
    // and part2 are zero.  However, if the caller is using 64-bit integers,
    // it is likely that they expect the numbers to often be very large, so
    // we probably don't want to optimize for small numbers anyway.  Thus,
    // we end up with a hardcoded binary search tree...

    // @ Jeremy R. Fishman @
    // I suppose PerTwisted doesn't *need* 64-bit varints, especially since
    // most Python builds are not 64-bit compatible, but we also don't
    // need speed with varints quite the way Protocol Buffers does.  And I
    // suppose *somebody* might want to store 4GB+ Blobs, and have a 64-bit
    // version of Python, heaven knows why.  Hopefully, then, this would work.
    //  :-D

    if (part2 == 0) {
      if (part1 == 0) {
        if (part0 < (1 << 14)) {
          if (part0 < (1 << 7)) {
            return 1;
          } else {
            return 2;
          }
        } else {
          if (part0 < (1 << 21)) {
            return 3;
          } else {
            return 4;
          }
        }
      } else {
        if (part1 < (1 << 14)) {
          if (part1 < (1 << 7)) {
            return 5;
          } else {
            return 6;
          }
        } else {
          if (part1 < (1 << 21)) {
            return 7;
          } else {
            return 8;
          }
        }
      }
    } else {
      if (part2 < (1 << 7)) {
        return 9;
      } else {
        return 10;
      }
    }
}

vi_size_t
uint64_to_vi(uint64_t value, varint *vi)
{
    vi_size_t len;

    // Splitting into 32-bit pieces gives better performance on 32-bit
    // processors.
    uint32_t part0 = (uint32_t)(value      );
    uint32_t part1 = (uint32_t)(value >> 28);
    uint32_t part2 = (uint32_t)(value >> 56);

    switch ((len = uint64_len(value))) {
      case 10: vi[9] = (uint8_t)((part2 >>  7) | 0x80);
      case 9 : vi[8] = (uint8_t)((part2      ) | 0x80);
      case 8 : vi[7] = (uint8_t)((part1 >> 21) | 0x80);
      case 7 : vi[6] = (uint8_t)((part1 >> 14) | 0x80);
      case 6 : vi[5] = (uint8_t)((part1 >>  7) | 0x80);
      case 5 : vi[4] = (uint8_t)((part1      ) | 0x80);
      case 4 : vi[3] = (uint8_t)((part0 >> 21) | 0x80);
      case 3 : vi[2] = (uint8_t)((part0 >> 14) | 0x80);
      case 2 : vi[1] = (uint8_t)((part0 >>  7) | 0x80);
      case 1 : vi[0] = (uint8_t)((part0      ) | 0x80); break;
      default: assert(0);  /* Can't get here */
    }

    vi[len-1] &= 0x7F;
    return len;
}

/**
 * Copy a varint from one buffer to another, returning the number
 * of bytes copied (the length of the varint)
 */
vi_size_t
vi_copy(const varint *in, varint *out)
{
    vi_size_t len = 0;

    while (len++ < 10) {
        *out++ = *in;
        if (!(*in & 0x80))
            // reached the last byte
            break;
        in++;
    }
    // check for data corruption
    if (*in & 0x80)
        return 0;
    return len;
}

/**
 * Pad the varint to at least the given length.  Varints already of length
 * C{len} or more are not touched, whereas smaller varint are padded with
 * 0x80 bytes until the last, which is 0x0.
 */
vi_size_t
vi_pad(varint *vi, vi_size_t len)
{
    assert(len > 0 && len <= 10);
    vi_size_t i = 0;

    // count varint length
    while (*vi & 0x80) {
        vi++;
        i++;
    }
    // i == length(vi) at this point
    if (i < len) {
        // set continue bit on previous last byte
        *vi++ |= 0x80;
        i++;
        // set zeros and continue (continue bit, zero pad)
        while (++i < len)
            *vi++ = 0x80;
        // set last byte to zero (no continue, zero pad)
        *vi = 0;
        // i == len at this point
    }
    return i;
}
