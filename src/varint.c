
/*
** This file is part of polyadicts - addicted to data encapsulation.
**
** Polyadicts is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Polyadicts is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** and the GNU Lesser Public License along with polyadicts.  If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include "varint.h"

static const uint8_t LogTable256[256] =
{
#define LT(n) \
    n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
    0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
    LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
};
#define uint8_log2(x) LogTable256[(x) & 0xff]

static inline uint8_t
uint32_log2(const uint32_t x)
{
#if defined(__i386__) || defined(__x86_64__)
    /*
     * Use Intel "bsr" operation for highest bit set
     */
    uint32_t y;
    /* bsr output operand undefined when input is zero */
    if (!x) return 0;
    __asm__ (
        "\tbsr %1, %0\n"
        : "=r"(y)
        : "r" (x)
        );
    return y;
#else
    /*
     * Use binary search tree with lookup table.
     */
    if (x < 0x10000) {
        if (x < 0x100) {
            return uint8_log2(x);
        } else {
            return uint8_log2(x >> 8) + 8;
        }
    } else {
        if (x < 0x1000000) {
            return uint8_log2(x >> 16) + 16;
        } else {
            return uint8_log2(x >> 24) + 24;
        }
    }
#endif
}

static inline uint8_t
uint64_log2(const uint64_t x)
{
    if (x < 0x100000000LL)
        return uint32_log2(x);
    return uint32_log2(x >> 32) + 32;
}

static inline uint8_t
vi_rv(const void *vi, uint8_t i)
{
    return ((uint8_t *)vi)[i];
}

static inline uint8_t *
vi_lvp(void *vi, uint8_t i)
{
    return &(((uint8_t *)vi)[i]);
}

size_t
vi_copy(const void *const src, size_t len, void *const dst)
{
    size_t i;
    i = 0;
    for (;;) {
        if (i == len) {
            errno = EINVAL;
            return 0;
        } else if (i == VI_MAX_LEN) {
            errno = ERANGE;
            return 0;
        } else {
            if (!(vi_rv(src, i++) & 0x80)) {
                memcpy(dst, src, i);
                return i;
            }
        }
    }
}

size_t
vi_to_size(const void *const src, size_t len, size_t *dst)
{
    size_t x, i;
    x = i = 0;
    for (;;) {
        if (i == len) {
            errno = EINVAL;
            return 0;
        } else if (i == VI_MAX_LEN) {
            errno = ERANGE;
            return 0;
        } else {
            x |= (vi_rv(src, i) & 0x7fLL) << (7 * i);
            if (!(vi_rv(src, i++) & 0x80)) {
                *dst = x;
                return i;
            }
        }
    }
}

size_t
size_to_vi(size_t x, void *const v, size_t l)
{
    size_t s, i;
    if (x > VI_MAX) {
        errno = ERANGE;
        return 0;
    }
    s = uint64_log2(x) / 7 + 1;
    if (s > l) {
        errno = EINVAL;
        return 0;
    }
    if (v) {
        for (i = 0; i < s - 1; i++, x >>= 7) {
            *vi_lvp(v,i) = (x & 0x7f) | 0x80;
        }
        *vi_lvp(v,i) = (x & 0x7f);
    }
    return s;
}
