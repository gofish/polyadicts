
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

#include "polyadics.h"
#include "varint.h"

polyid_t* polyid_new(uint32_t n, uint64_t *values)
{
    polyid_t *pack;

    pack = malloc(sizeof(polyid_t));
    if (pack) {
        uint32_t i;

        pack->n = n;
        pack->shared = false;
        pack->values = values; // take control of the values array

        // calculate resulting size
        pack->size = uint64_to_vi(n, NULL, -1);
        for (i = 0; i < n; i++)
            pack->size += uint64_to_vi(values[i], NULL, -1);

        pack->data = malloc(pack->size);
        if (pack->data) {
            size_t off;
            uint8_t vi_size;

            off = vi_size = uint64_to_vi(n, pack->data, pack->size);
            if (vi_size) {
                for (i = 0; i < n; i++) {
                    vi_size = uint64_to_vi(values[i], pack->data + off, pack->size - off);
                    if (vi_size)
                        off += vi_size;
                    else
                        break;
                }
                if (i == n)
                    return pack;
            }
        }
        // failure
        free(pack);
    }
    return NULL;
}

polyid_t* polyid_load(void *data, bool shared, size_t maxlen)
{
    polyid_t *pack;

    pack = malloc(sizeof(polyid_t));
    if (pack) {
        uint8_t vi_size;
        uint32_t i;
        uint64_t n;

        pack->size = 0;
        pack->data = data;
        pack->shared = shared;
        pack->values = NULL;

        // read the number of stored values
        vi_size = vi_to_uint64(data, maxlen, &n);
        if (!vi_size) {
            free(pack);
            return NULL;
        }
        maxlen -= vi_size;
        if (!maxlen) {
            errno = EINVAL;
            free(pack);
            return NULL;
        }

        // allocate a lookup table
        pack->n = n;
        data += vi_size;
        pack->values = malloc(pack->n * sizeof(uint64_t));
        if (!pack->values) {
            free(pack);
            return NULL;
        }

        // read values into table
        for (i = 0; i < pack->n && maxlen; i++) {
            vi_size = vi_to_uint64(data, maxlen, &pack->values[i]);
            if (vi_size) {
                data += vi_size;
                maxlen -= vi_size;
            } else {
                break;
            }
        }

        if (i != pack->n) {
            free(pack->values);
            free(pack);
            return NULL;
        }

        // we're done
        pack->size = data - pack->data;
    }
    return pack;
}

void polyid_free(polyid_t *pack)
{
    assert(pack);

    if (!pack->shared)
        free(pack->data);
    free(pack->values);
    free(pack);
}

polyad_t* polyad_load(size_t size, void *data, bool shared)
{
    polyad_t *polyad;

    polyad = malloc(sizeof(polyad_t));
    if (polyad) {
        uint8_t vi_size;
        void *head, *tail;
        uint64_t item_size;

        polyad->self.size = size;
        polyad->self.data = data;
        polyad->self.shared = shared;
        polyad->nitem = 0;
        polyad->item = NULL;

        /* scan header to count items */
        head = data;
        tail = data + size;
        while (head < tail) {
            vi_size = vi_to_uint64(head, tail - head, &item_size);
            if (vi_size) {
                head += vi_size;
                tail -= item_size;
                polyad->nitem++;
            } else {
                break;
            }
        }

        if (head == tail) {
            /* allocate item array and scan header again for item offsets */
            polyad->item = malloc(polyad->nitem * sizeof(polyad_info_t));
            if (polyad->item) {
                uint32_t i;
                head = data;
                for (i = 0; i < polyad->nitem; i++) {
                    vi_size = vi_to_uint64(head, tail - head, &item_size);
                    assert(vi_size); // (tail - head) OK since already validated against overread above
                    polyad->item[i].size = item_size;
                    polyad->item[i].data = tail;
                    polyad->item[i].shared = true;
                    head += vi_size;
                    tail += item_size;
                }
            } else {
                free(polyad);
                polyad = NULL;
            }
        } else {
            free(polyad);
            polyad = NULL;
            errno = EINVAL;
        }
    }
    return polyad;
}

void polyad_free(polyad_t *polyad)
{
    assert(polyad);

    uint32_t i;
    for (i = 0; i < polyad->nitem; i++) {
        if (!polyad->item[i].shared)
            free(polyad->item[i].data);
    }
    if (!polyad->self.shared)
        free(polyad->self.data);
    free(polyad->item);
    free(polyad);
}

polyad_t* polyad_prepare(uint32_t nitem)
{
    polyad_t *polyad;

    polyad = malloc(sizeof(polyad_t));
    if (polyad) {
        polyad->self.size = 0;
        polyad->self.data = NULL;
        polyad->self.shared = true;
        polyad->item = malloc(nitem * sizeof(polyad_info_t));
        if (polyad->item) {
            uint32_t i;
            for (i = 0; i < nitem; i++) {
                polyad->item[i].size = 0;
                polyad->item[i].data = NULL;
                polyad->item[i].shared = true;
            }
            polyad->nitem = nitem;
        }else {
            free(polyad);
            polyad = NULL;
        }
    }
    return polyad;
}

int polyad_set(polyad_t *polyad, uint32_t i, size_t size, void *data, bool shared)
{
    assert(polyad);

    int retval;
    if (i < polyad->nitem) {
        if (polyad->item[i].data && !polyad->item[i].shared)
            free(polyad->item[i].data);
        polyad->item[i].size = size;
        polyad->item[i].data = data;
        polyad->item[i].shared = shared;
        retval = 0;
    } else {
        errno = EINVAL;
        retval = -1;
    }
    return retval;
}

int polyad_finish(polyad_t *polyad)
{
    assert(polyad);

    int retval;
    uint32_t i;
    size_t head_size, tmp;
    /* count header size and total required buffer size */
    head_size = 0;
    polyad->self.size = 0;
    for (i = 0; i < polyad->nitem; i++) {
        /* size of item */
        polyad->self.size += polyad->item[i].size;
        /* size of varint to required to represent item size */
        tmp = uint64_to_vi(polyad->item[i].size, NULL, -1);
        head_size += tmp;
        polyad->self.size += tmp;
    }

    polyad->self.data = malloc(polyad->self.size);
    if (polyad->self.data) {
        void *head, *tail;
        head = polyad->self.data;
        tail = polyad->self.data + head_size;
        polyad->self.shared = false;

        for (i = 0; i < polyad->nitem; i++) {
            head += uint64_to_vi(polyad->item[i].size, head, tail - head);
            /* copy item data to shared buffer */
            memcpy(tail, polyad->item[i].data, polyad->item[i].size);
            /* free old data */
            if (!polyad->item[i].shared)
                free(polyad->item[i].data);
            /* point item data to location in shared buffer */
            polyad->item[i].data = tail;
            polyad->item[i].shared = true;
            tail += polyad->item[i].size;
        }
        assert(head == polyad->self.data + head_size);
        assert(tail == polyad->self.data + polyad->self.size);
        retval = 0;
    } else {
        retval = -1;
    }

    return retval;
}
