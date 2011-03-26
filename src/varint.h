
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

#ifndef _varint_h_DEFINED
#define _varint_h_DEFINED

#include <stdint.h>
#include <unistd.h>

typedef uint64_t varint;
typedef uint8_t vi_size_t;
typedef uint8_t vi_data_t;

static const uint64_t VI_MAX = (1ll << (7 * sizeof(varint))) - 1;

vi_size_t vi_to_uint64  (const varint *, uint64_t *);
vi_size_t vi_to_uint64_2(const varint *, uint64_t *, size_t);
vi_size_t uint64_to_vi  (uint64_t, varint *);

vi_size_t vi_copy(const varint *, varint *);

vi_size_t uint64_log2(uint64_t);
#define uint64_vi_len(x) (uint64_log2(x) / 7 + 1)

static inline vi_data_t vi_rv(const varint *vi, vi_size_t i)
{
    return ((vi_data_t *)vi)[i & (sizeof(varint) - 1)];
}

static inline varint* vi_step(varint *vi, uint64_t *x)
{
    vi_size_t n;
    n = vi_to_uint64(vi,x);
    if (!n) return 0;
    return (varint *)((vi_data_t *)vi + n);
}

#endif /* _varint_h_DEFINED */
