
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

#ifndef _polyadic_h_DEFINED
#define _polyadic_h_DEFINED

#include <stdbool.h>
#include <sys/types.h>
#include "varint.h"

typedef unsigned int polyadic_len_t;

typedef struct polyadic_info_st
{
    // length of data pointed to by void* data
    size_t size;
    // memory region for data;
    void* data;
    // do we own the memory, or is it shared?
    bool shared;
} polyadic_info_t;

typedef struct polyadic_st
{
    // total polyadic length and data pointer
    polyadic_info_t self;
    // number and array of stored items
    polyadic_len_t nitem;
    polyadic_info_t *item;
} polyadic_t;


/** The following three functions should be used in consecutive order only **/

/* initialize a new polyadic object prepared to store n entries */
polyadic_t* polyadic_prepare(polyadic_len_t nitem);
/* set the ith item in a polyadic to the given data */
int polyadic_set(polyadic_t *polyadic, polyadic_len_t i, size_t size, void *data, bool shared);
/* allocate a single memory buffer and store the packed items */
int polyadic_finish(polyadic_t *polyadic);

/* read a polyadic structure from a suppplied data pointer */
polyadic_t* polyadic_load(size_t size, void *data, bool shared);

/* free any non-shared memory associated with a polyadic */
void polyadic_free(polyadic_t *polyadic);




typedef struct peanos_st
{
    // an n-tuple of varints, transcoded to unsigned 64-bit integers
    polyadic_len_t n;
    uint64_t *values;
    // the length and location of the backing data buffer, which may be shared
    size_t size;
    void *data;
    bool shared;
} peanos_t;

peanos_t* peanos_new(polyadic_len_t n, uint64_t *values);
peanos_t* peanos_load(void *data, bool shared, size_t maxlen);
void peanos_free(peanos_t *pack);

/** The following three functions should be used in consecutive order only **/
peanos_t* peanos_prepare(polyadic_len_t n);
int peanos_set(peanos_t *pack, polyadic_len_t i, uint64_t value);
int peanos_finish(peanos_t *pack);
/** The previous three functions should be used in consecutive order only **/


typedef struct bytepack_st
{
    // begins with an peanos header
    peanos_t *header;
    // includes several binary data chunks
    void *bv;
    // which may or may not be shared memory regions
    bool *shared;
} bytepack_t;

bytepack_t* bytepack_load(void *data, bool shared, size_t maxlen);
void bytepack_free(bytepack_t *pack);

/** The following three functions should be used in consecutive order only **/
bytepack_t* bytepack_prepare(polyadic_len_t n);
int bytepack_set(bytepack_t *pack, polyadic_len_t i, void *data, size_t len,
        bool shared);
int bytepack_finish(bytepack_t *pack);
/** The previous three functions should be used in consecutive order only **/


#endif
