
/*
** This file is part of varyadicts - addicted to data encapsulation.
**
** varyadicts is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** varyadicts is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** and the GNU Lesser Public License along with varyadicts.  If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef _varyad_h_DEFINED
#define _varyad_h_DEFINED

#include <sys/types.h>
#include "varint.h"

/**
 * varyad - an n-list of binary data segments.
 */
struct varyad;

/**
 * varyads are non-const/mutable
 */
typedef struct varyad * varyad_t;

/** The number of items in a varyad. **/
size_t varyad_rank(varyad_t v);

/** The number of bytes in a varyad. **/
size_t varyad_size(varyad_t v);

/** The data buffer backing the entire varyad. **/
const void * varyad_data(varyad_t v);

/**
 * The data buffer corresponding to a particular item.
 *
 * @param v the varyad
 * @param i the item index ({@code 0 <= i < varyad_rank(v)})
 * @param dst the address of a NULL-initialized memory address
 * @return
 *   The address at {@code item} will contain a pointer to the start of
 *   the varyad item buffer, or NULL on error. On success, the return
 *   value will be the size of this buffer.
 * @error EINVAL {@code i} is greater or equal to the varyad rank
 */
size_t varyad_item(varyad_t v, size_t i, const void **dst);

/**
 * Allocate and initialize a new varyad structure of the given size.
 *
 * @param size the initial data size, in bytes
 * @param dst the address of an uninitialized varyad pointer
 * @return the size of the varyad data buffer, with {@code dst} pointing
 *   to the varyad or NULL on error
 * @error ENOMEM if memory allocation fails
 * @error EINVAL if {@code size} is less than sizeof(size_t)
 */
size_t varyad_init(size_t size, varyad_t *dst);

/**
 * Free the memory associated with a varyad.
 **/
void   varyad_free(varyad_t v);

/**
 * Push a data element onto the end of a varyad.
 *
 * @param v reference to a varyad pointer (which may be reallocated)
 * @param data the data buffer to push onto the varyad
 * @param size the size of the data buffer
 * @param realloc flag enabling reallocation (1 if should reallocate)
 * @return the size of the varyad (always > 0), or 0 on error
 * @error ENOMEM if a rellocation is required and either failed or is disabled
 */
size_t varyad_push(varyad_t *v, void *data, size_t size, int realloc);

#endif
