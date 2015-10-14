
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
 * polyid - an n-tuple of varint-packed unsigned integers
 */

struct polyid
{
    // tuple-length 'n' and decoded unsigned 64-bit integers
    uint32_t n;
    uint64_t *values;
    // data length and backing buffer
    size_t size;
    void *data;
    // do we own the buffer
    bool shared;
};

struct polyid* polyid_new(uint32_t n, uint64_t *values);
struct polyid* polyid_load(void *data, bool shared, size_t maxlen);
void polyid_free(struct polyid *pack);

/**
 * polyad - an n-tuple of binary data segments with a polyid header
 */

struct polyad_info
{
    // length of data pointed to by void* data
    size_t size;
    // memory region for data;
    void* data;
    // do we own the memory, or is it shared?
    bool shared;
};

struct polyad
{
    // total polyad length and data pointer
    struct polyad_info self;
    // number and array of stored items
    uint32_t nitem;
    struct polyad_info *item;
};

/* read a polyad structure from a suppplied data pointer */
struct polyad* polyad_load(size_t size, void *data, bool shared);
/* free any non-shared memory associated with a polyad */
void polyad_free(struct polyad *polyad);

/**
 * The following three functions should be used in consecutive order only
 **/
/* initialize a new polyad object prepared to store n entries */
struct polyad* polyad_prepare(uint32_t nitem);
/* set the i'th item in a polyad to the given data */
int polyad_set(struct polyad *polyad, uint32_t i, size_t size, void *data, bool shared);
/* allocate a single memory buffer and store the packed items */
int polyad_finish(struct polyad *polyad);

#endif
