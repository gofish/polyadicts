
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

size_t ntuple_size(size_t rank, const size_t *info);
size_t ntuple_pack(size_t rank, const size_t *info, void *data, size_t size);

size_t ntuple_rank(const void *data, size_t size, size_t *rank);
size_t ntuple_load(const void *data, size_t size, size_t rank, size_t *info);

#endif /* _varint_h_DEFINED */
