
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

#ifndef _ntuple_h_DEFINED
#define _ntuple_h_DEFINED

#include <stddef.h>

/**
 * ntuple - an n-tuple of natural numbers packed as varints
 */

/**
 * Compute the packed size of an ntuple from an array of natural numbers.
 *
 * @param rank the number of elements in the ntuple
 * @param info the elements to be stored in the ntuple
 * @return the packed size of the ntuple, in bytes
 * @error ERANGE a value would overflow the maximum supported varint
 */
size_t ntuple_size(size_t rank, const size_t *info);

/**
 * Pack an ntuple from an array of natural numbers.
 *
 * @param rank the number of elements in the ntuple
 * @param info the elements to be stored in the ntuple
 * @param data the destination buffer
 * @param size the size of the destination buffer
 * @return the number of bytes written to {@code data}
 * @error ERANGE a value would overflow the maximum supported varint
 * @error EINVAL {@code size} is too small to contain the ntuple
 */
size_t ntuple_pack(size_t rank, const size_t *info, void *data, size_t size);

/**
 * Read the rank of an ntuple from a data buffer.
 *
 * @param data the source buffer
 * @param size the size of the source buffer
 * @param rank the address to store the rank
 * @return the number of bytes read
 * @error ERANGE the rank varint would overflow this architechture's size
 * @error EINVAL {@code size} is too small to read the rank
 */
size_t ntuple_rank(const void *data, size_t size, size_t *rank);

/**
 * Read an ntuple from a data buffer.
 *
 * @param data the source buffer
 * @param size the size of the source buffer
 * @param rank the number of elements in the ntuple
 * @param info an array of size {@code rank} to store the elements
 * @return the number of bytes read
 * @error ERANGE a varint value would overflow this architechture's size
 * @error EINVAL {@code size} is too small to read the ntuple, or {@code rank} is invalid
 */
size_t ntuple_load(const void *data, size_t size, size_t rank, size_t *info);

#endif /* _varint_h_DEFINED */
