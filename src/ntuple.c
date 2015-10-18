
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
#include <limits.h>
#include <stdlib.h>
#include "ntuple.h"
#include "varint.h"

size_t
ntuple_size(size_t rank, const size_t *info)
{
    size_t size, n, i;
    size = 0;
    n = size_to_vi(rank, NULL, -1);
    if (n) {
        size = n;
        for (i = 0; i < rank; i++) {
            n = size_to_vi(info[i], NULL, -1);
            if (n) {
                size += n;
            } else {
                size = 0;
                break;
            }
        }
    }
    return size;
}

size_t
ntuple_pack(size_t rank, const size_t *info, void *data, size_t size)
{
    size_t off, n, i;
    off = 0;
    n = size_to_vi(rank, data, size);
    if (n) {
        off = n;
        for (i = 0; i < rank; i++) {
            n = size_to_vi(info[i], data + off, size - off);
            if (n) {
                off += n;
            } else {
                off = 0;
                break;
            }
        }
    }
    return off;
}

size_t
ntuple_rank(const void *data, size_t size, size_t *rank)
{
    return vi_to_size(data, size, rank);
}

size_t
ntuple_load(const void *data, size_t size, size_t rank, size_t *info)
{
    size_t off, n, i, x;
    off = 0;
    n = vi_to_size(data, size, &x);
    if (n) {
        if (x == rank) {
            off = n;
            for (i = 0; i < rank; i++) {
                n = vi_to_size(data + off, size - off, &info[i]);
                if (n) {
                    off += n;
                } else {
                    off = 0;
                    break;
                }
            }
        } else {
            errno = EINVAL;
        }
    }
    return off;
}
