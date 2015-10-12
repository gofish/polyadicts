
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

static const uint64_t VI_MAX = (1LL << (7 * sizeof(varint))) - 1;

uint8_t vi_copy     (const varint *, varint *);
uint8_t vi_to_uint64(const varint *, size_t, uint64_t *);
uint8_t uint64_to_vi(uint64_t, varint *, size_t);

uint8_t uint64_log2 (uint64_t);

#endif /* _varint_h_DEFINED */
