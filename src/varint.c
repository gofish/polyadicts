
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

inline vi_size_t
uint64_log2(const uint64_t x)
{
    if (x < 0x100000000LL)
        return uint32_log2(x);
    return uint32_log2(x >> 32) + 32;
}

static inline uint8_t
vi_rv(const varint *vi, vi_size_t i)
{
    return ((uint8_t *)vi)[i];
}

static inline uint8_t *
vi_lvp(varint *vi, vi_size_t i)
{
    return &(((uint8_t *)vi)[i]);
}

vi_size_t
vi_copy(const varint *const src, varint *const dst)
{
    vi_size_t i;
    i = 0;
    for (;;) {
        if (!((*vi_lvp(dst,i) = vi_rv(src,i)) & 0x80)) break;
        if (++i == sizeof(varint)) return 0;
    }
    return i;
}

vi_size_t
vi_to_uint64(const varint *const v, uint64_t *x, size_t l)
{
    uint64_t y;
    vi_size_t i;
    y = i = 0;
    for (;;) {
        if (i == l) {
            errno = EINVAL;
            return 0;
        }
        if (i == sizeof(varint)) {
            errno = ERANGE;
            return 0;
        }
        y |= (vi_rv(v,i) & 0x7fLL) << (7 * i);
        if (!(vi_rv(v,i++) & 0x80)) break;
    }
    *x = y;
    return i;
}

vi_size_t
uint64_to_vi(uint64_t x, varint *const v, size_t l)
{
    vi_size_t s, i;
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
