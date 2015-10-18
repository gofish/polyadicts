
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

#ifndef _polyad_h_DEFINED
#define _polyad_h_DEFINED

#include <stdbool.h>
#include <sys/types.h>
#include "varint.h"

/**
 * polyad - an n-tuple of binary data segments with a polyid header
 */
struct polyad;

typedef const struct polyad * polyad_t;

/** The number of items in a polyad. **/
size_t polyad_rank(polyad_t p);

/** The number of bytes in a polyad. **/
size_t polyad_size(polyad_t p);

/** The data buffer backing the entire polyad. **/
const void * polyad_data(polyad_t p);

/**
 * The data buffer corresponding to a particular item.
 *
 * @param p the polyad
 * @param i the item index ({@code 0 <= i < polyad_rank(p)})
 * @param dst the address of a NULL-initialized memory address
 * @return
 *   The address at {@code item} will contain a pointer to the start of
 *   the polyad item buffer, or NULL on error. On success, the return
 *   value will be the size of this buffer.
 * @error EINVAL {@code i} is greater or equal to the polyad rank
 */
size_t polyad_item(polyad_t p, size_t i, const void **dst);

/**
 * Allocate and initialize a polyad structure from serialized form.
 *
 * The buffer at the {@code data} address WILL be shared -- it must not be
 * freed before the returned polyad struct is freed.
 *
 * @param src a pointer to the read buffer
 * @param len the buffer size (maximum length of polyad)
 * @param dst the address of an uninitialized polyad pointer
 * @return the number of bytes read
 * @error ERANGE a stored varint would overflow the {@code size_t} of this architecture
 * @error EINVAL the buffer {@code size} is too small to read a full polyad
 **/
size_t polyad_load(const void *src, size_t len, polyad_t *dst);

/**
 * Allocate and initialize a new polyad structure from items.
 *
 * The item memory buffers WILL NOT be shared -- a new buffer will be
 * allocated to contain the polyad structure and backing data buffer.
 *
 * @param rank the number of items in the polyad
 * @param items an array of {@code rank} item buffers
 * @param sizes the size of each corresponding buffer in {@code items}
 * @param dst the address of an uninitialized polyad pointer
 * @return the size of the polyad data buffer
 */
size_t polyad_init(size_t rank, const void **items, const size_t *sizes, polyad_t *dst);

/**
 * Copy a polyad into another data buffer.
 *
 * @param src the source polyad
 * @param dst the destination buffer
 * @param len the length of the destination buffer)
 * @return the number of bytes copied
 * @error EINVAL {@code len} is too small to contain {@code src}
 */
size_t polyad_copy(polyad_t src, void *dst, size_t len);

/**
 * Free the memory associated with a polyad.
 **/
void   polyad_free(polyad_t p);

#endif
