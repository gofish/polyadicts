
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

#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "polyad.h"
#include "ntuple.h"

struct polyad {
    size_t rank;
    void * data;
    size_t item[];
};

size_t
polyad_rank(const struct polyad *p)
{
    return p->rank;
}

size_t
polyad_size(const struct polyad *p)
{
    return p->item[p->rank];
}

const void *
polyad_data(const struct polyad *p)
{
    return p->data;
}

static inline const void *
_item_buf(const struct polyad *p, size_t i)
{
    return ((const char *) p->data) + p->item[i];
}

static inline size_t
_item_len(const struct polyad *p, size_t i)
{
    return p->item[i + 1] - p->item[i];
}

size_t
polyad_item(const struct polyad *p, size_t i, const void **item)
{
    if (i < p->rank) {
        *item = _item_buf(p, i);
        return _item_len(p, i);
    } else {
        *item = NULL;
        errno = EINVAL;
        return 0;
    }
}

#define SIZEOF_POLYAD(rank) (sizeof(struct polyad) + sizeof(size_t) * ((rank) + 1))

size_t
polyad_load(const void *data, size_t size, const struct polyad **dst)
{
    size_t rank, off, n, i;
    struct polyad *p;
    off = 0;
    *dst = NULL;
    /* read the ntuple/polyad rank and allocate */
    n = vi_to_size(data, size, &rank);
    if (n) {
        p = malloc(SIZEOF_POLYAD(rank));
        if (p) {
            /* read the item sizes */
            p->rank = rank;
            p->data = (void *) data;
            off = n;
            for (i = 0; i < rank; i++) {
                n = vi_to_size(data + off, size - off, &p->item[i]);
                if (n) {
                    off += n;
                } else {
                    break;
                }
            }
            if (i == rank) {
                /* convert the sizes to data offsets */
                for (i = 0; i < rank; i++) {
                    n = p->item[i];
                    p->item[i] = off;
                    off += n;
                }
                p->item[rank] = off;
                /* store the result in destination address */
                *dst = p;
            } else {
                free(p);
                off = 0;
            }
        }
    }
    return off;
}

size_t
polyad_init(size_t rank, const void **items, const size_t *sizes, const struct polyad **dst)
{
    size_t off, i;
    struct polyad *p;
    *dst = NULL;
    /* calculate total header and item size */
    off = ntuple_size(rank, sizes);
    if (off) {
        for (i = 0; i < rank; i++) {
            off += sizes[i];
        }
        /* allocate polyad and data buffer */
        p = malloc(off + SIZEOF_POLYAD(rank));
        if (p) {
            p->rank = rank;
            p->data = ((char *) p) + SIZEOF_POLYAD(rank);
            off = ntuple_pack(rank, sizes, (void *)p->data, off);
            if (off) {
                for (i = 0; i < rank; i++) {
                    memcpy((void *)p->data + off, items[i], sizes[i]);
                    p->item[i] = off;
                    off += sizes[i];
                }
                p->item[rank] = off;
                *dst = p;
            } else {
                free(p);
            }
        } else {
            off = 0;
        }
    }
    return off;
}

size_t
polyad_copy(const struct polyad *src, void *dst, size_t len)
{
    const size_t size = polyad_size(src);
    if (len >= size) {
        memcpy(dst, polyad_data(src), polyad_size(src));
        return polyad_size(src);
    } else {
        errno = EINVAL;
        return 0;
    }
}

void
polyad_free(const struct polyad *p)
{
    free((void *) p);
}
