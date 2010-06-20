
/* 
** This file is part of PerTwisted, a persistant storage framework
** tailored around the Twisted asynchronous network programming framework,
** utilizing the Berkeley Database, Cononical's Storm ORM, implementations
** of various DHTs and hopefully much more.
**
** Pertwisted is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** PerTwisted is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** and the GNU Lesser Public License along with PerTwisted.  If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef _polyad_h_DEFINED
#define _polyad_h_DEFINED

#include <stdbool.h>
#include <sys/types.h>
#include "varint.h"

typedef unsigned int polyad_len_t;

typedef struct polyad_info_st
{
    // length of data pointed to by void* data
    size_t size;
    // memory region for data;
    void* data;
    // do we own the memory, or is it shared?
    bool shared;
} polyad_info_t;

typedef struct polyad_st
{
    // total polyad length and data pointer
    polyad_info_t self;
    // number and array of stored items
    polyad_len_t nitem;
    polyad_info_t *item;
} polyad_t;


/** The following three functions should be used in consecutive order only **/

/* initialize a new polyad object prepared to store n entries */
polyad_t* polyad_prepare(polyad_len_t nitem);
/* set the ith item in a polyad to the given data */
int polyad_set(polyad_t *polyad, polyad_len_t i, size_t size, void *data, bool shared);
/* allocate a single memory buffer and store the packed items */
int polyad_finish(polyad_t *polyad);

/* read a polyad structure from a suppplied data pointer */
polyad_t* polyad_load(size_t size, void *data, bool shared);

/* free any non-shared memory associated with a polyad */
void polyad_free(polyad_t *polyad);




typedef struct polyid_st
{
    // an n-tuple of varints, transcoded to unsigned 64-bit integers
    polyad_len_t n;
    uint64_t *values;
    // the length and location of the backing data buffer, which may be shared
    size_t size;
    void *data;
    bool shared;
} polyid_t;

polyid_t* polyid_new(polyad_len_t n, uint64_t *values);
polyid_t* polyid_load(void *data, bool shared, size_t maxlen);
void polyid_free(polyid_t *pack);

/** The following three functions should be used in consecutive order only **/
polyid_t* polyid_prepare(polyad_len_t n);
int polyid_set(polyid_t *pack, polyad_len_t i, uint64_t value);
int polyid_finish(polyid_t *pack);
/** The previous three functions should be used in consecutive order only **/


typedef struct bytepack_st
{
    // begins with an polyid header
    polyid_t *header;
    // includes several binary data chunks
    void *bv;
    // which may or may not be shared memory regions
    bool *shared;
} bytepack_t;

bytepack_t* bytepack_load(void *data, bool shared, size_t maxlen);
void bytepack_free(bytepack_t *pack);

/** The following three functions should be used in consecutive order only **/
bytepack_t* bytepack_prepare(polyad_len_t n);
int bytepack_set(bytepack_t *pack, polyad_len_t i, void *data, size_t len,
        bool shared);
int bytepack_finish(bytepack_t *pack);
/** The previous three functions should be used in consecutive order only **/


#endif
