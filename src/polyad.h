
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
#include <sys/uio.h>
#include "varint.h"

/**
 * polyad - an n-tuple of binary data segments with a polyid header
 */

struct polyad;

/* read a polyad structure from a suppplied data pointer */
struct polyad * polyad_load(size_t size, void *data, bool shared);

/* free any non-shared memory associated with a polyad */
void polyad_free(struct polyad *polyad);

size_t       polyad_rank(struct polyad *p);
struct iovec polyad_data(struct polyad *p);
struct iovec polyad_item(struct polyad *p, size_t i);

/**
 * The following three functions should be used in consecutive order only
 **/
/* initialize a new polyad object prepared to store n entries */
struct polyad* polyad_prepare(size_t nitem);
/* set the i'th item in a polyad to the given data */
int polyad_set(struct polyad *polyad, size_t i, size_t size, void *data, bool shared);
/* allocate a single memory buffer and store the packed items */
int polyad_finish(struct polyad *polyad);

#endif
