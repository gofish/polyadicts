
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

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "varyad.h"

struct varyad {
    size_t rank;
    size_t size;
};

size_t
varyad_rank(struct varyad *v)
{
    return v->rank;
}

size_t
varyad_size(struct varyad *v)
{
    return v->size;
}

const void *
varyad_data(struct varyad *v)
{
    return v;
}

static inline char *
_head(const struct varyad *v)
{
    return ((char *) v) + sizeof(struct varyad);
}

static inline char *
_foot(const struct varyad *v)
{
    return (((char *) v) + v->size);
}

static inline size_t *
_size_ptr(const struct varyad *v, size_t i)
{
    return ((size_t *) _foot(v)) - i - 1;
}

static inline size_t
_size(const struct varyad *v, size_t i)
{
    return * _size_ptr(v, i);
}

static inline void *
_item_buf(const struct varyad *v, size_t i)
{
    if (i) {
        return _head(v) + _size(v, i - 1);
    } else {
        return _head(v);
    }
}

static inline size_t
_item_len(const struct varyad *v, size_t i)
{
    if (i) {
        return _size(v, i) - _size(v, i - 1);
    } else {
        return _size(v, i);
    }
}

size_t
varyad_item(struct varyad *v, size_t i, const void **item)
{
    if (i < v->rank) {
        *item = _item_buf(v, i);
        return _item_len(v, i);
    } else {
        *item = NULL;
        errno = EINVAL;
        return 0;
    }
}

size_t
varyad_init(size_t size, struct varyad **dst)
{
    struct varyad *v;
    // adjust the size hint to minimum or maximum, if necessary
    if (size < sizeof(struct varyad)) {
        size = sizeof(struct varyad);
    } else if (size > SIZE_MAX - sizeof(size_t)) {
        size = SIZE_MAX - sizeof(size_t);
    }
    // adjust the size to a multiple of sizeof(size_t)
    size = (size + sizeof(size_t) - 1) / sizeof(size_t) * sizeof(size_t);
    // allocate the varyad (zeroed)
    v = calloc(1, size);
    if (v) {
        v->rank = 0;
        v->size = size;
        *dst = v;
        return size;
    } else {
        return 0;
    }
}

void
varyad_free(struct varyad *v)
{
    free(v);
}

static inline size_t
_avail(const struct varyad *v)
{
    size_t size = v->size;
    size -= sizeof(struct varyad);
    if (v->rank) {
        size -= v->rank * sizeof(size_t);
        size -= _size(v, v->rank - 1);
    }
    return size;
}

static inline void
_set_size(const struct varyad *v, size_t i, size_t size)
{
    * _size_ptr(v, i) = size;
}

static struct varyad *
_realloc(struct varyad *v)
{
    v = realloc(v, 2 * v->size);
    if (v) {
        if (v->rank) {
            const size_t *const tmp = _size_ptr(v, v->rank - 1);
            v->size = 2 * v->size;
            memcpy(_size_ptr(v, v->rank - 1), tmp, v->rank * sizeof(size_t));
        } else {
            v->size = 2 * v->size;
        }
        memset(_item_buf(v, v->rank), 0, _avail(v));
    }
    return v;
}

size_t
varyad_push(struct varyad **v, void *data, size_t size, int realloc)
{
    if (_avail(*v) < size + sizeof(size_t)) {
        if (!realloc) {
            errno = ENOMEM;
            return 0;
        } else {
            struct varyad *const tmp = _realloc(*v);
            if (!tmp) {
                return 0;
            } else {
                *v = tmp;
            }
        }
    }
    const size_t off = (*v)->rank ? _size(*v, (*v)->rank-1) : 0;
    memcpy(_head(*v) + off, data, size);
    _set_size(*v, (*v)->rank++, off + size);
    return (*v)->size;
}
