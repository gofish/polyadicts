
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

#include <stddef.h>

/** We support up to 9 x 7-bit bytes **/
#define VI_MAX_LEN (sizeof(size_t) * 8 / 7)
/** This provides for 63 bits of data **/
#define VI_MAX ((1ULL << (7 * VI_MAX_LEN)) - 1LL)

/**
 * @param src the source buffer (contains varint)
 * @param len the length of the source buffer
 * @param dst the destination buffer (of size at least {@code len})
 * @return the number of bytes copied
 * @error EINVAL if {@code len} is too short to read all of {@code src}
 * @error ERANGE if more than {@code VI_MAX_LEN} would be read
 */
size_t vi_copy(const void *src, size_t len, void *dst);

/**
 * @param src the source buffer (contains varint)
 * @param len the length of the source buffer
 * @param dst the size output variable
 * @return the number of bytes read from {@code src}
 * @error EINVAL if {@code len} is too short to read all of {@code src}
 * @error ERANGE if more than {@code VI_MAX_LEN} would be read
 */
size_t vi_to_size(const void *src, size_t len, size_t *dst);

/**
 * @param src the size source variable
 * @param dst the destination buffer (to contain varint)
 * @param len the length of the destination buffer
 * @return the number of bytes written to {@code dst}
 * @error EINVAL if {@code len} is too short to write all of {@code src}
 * @error ERANGE if more than {@code VI_MAX_LEN} would be written
 */
size_t size_to_vi(size_t src, void *dst, size_t len);

#endif /* _varint_h_DEFINED */
