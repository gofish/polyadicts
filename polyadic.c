
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

#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "polyadic.h"
#include "varint.h"

polyadic_t* polyadic_prepare(polyadic_len_t nitem)
{
    polyadic_t *polyadic;

    polyadic = malloc(sizeof(polyadic_t));
    if (polyadic) {
        polyadic->self.size = 0;
        polyadic->self.data = NULL;
        polyadic->self.shared = true;
        polyadic->item = malloc(nitem * sizeof(polyadic_info_t));
        if (polyadic->item) {
            polyadic_len_t i;
            for (i = 0; i < nitem; i++) {
                polyadic->item[i].size = 0;
                polyadic->item[i].data = NULL;
                polyadic->item[i].shared = true;
            }
            polyadic->nitem = nitem;
        }else {
            free(polyadic);
            polyadic = NULL;
        }
    }
    return polyadic;
}

int polyadic_set(polyadic_t *polyadic, polyadic_len_t i, size_t size, void *data, bool shared)
{
    assert(polyadic);

    int retval;
    if (i < polyadic->nitem) {
        if (polyadic->item[i].data && !polyadic->item[i].shared)
            free(polyadic->item[i].data);
        polyadic->item[i].size = size;
        polyadic->item[i].data = data;
        polyadic->item[i].shared = shared;
        retval = 0;
    } else {
        errno = EINVAL;
        retval = -1;
    }
    return retval;
}

int polyadic_finish(polyadic_t *polyadic)
{
    assert(polyadic);

    int retval;
    polyadic_len_t i;
    size_t head_size, tmp;
    /* count header size and total required buffer size */
    head_size = 0;
    polyadic->self.size = 0;
    for (i = 0; i < polyadic->nitem; i++) {
        /* size of item */
        polyadic->self.size += polyadic->item[i].size;
        /* size of varint to required to represent item size */
        tmp = uint64_len(polyadic->item[i].size);
        head_size += tmp;
        polyadic->self.size += tmp;
    }

    polyadic->self.data = malloc(polyadic->self.size);
    if (polyadic->self.data) {
        void *head, *tail;
        head = polyadic->self.data;
        tail = polyadic->self.data + head_size;
        polyadic->self.shared = false;

        for (i = 0; i < polyadic->nitem; i++) {
            head += uint64_to_vi(polyadic->item[i].size, head);
            /* copy item data to shared buffer */
            memcpy(tail, polyadic->item[i].data, polyadic->item[i].size);
            /* free old data */
            if (!polyadic->item[i].shared)
                free(polyadic->item[i].data);
            /* point item data to location in shared buffer */
            polyadic->item[i].data = tail;
            polyadic->item[i].shared = true;
            tail += polyadic->item[i].size;
        }
        assert(head == polyadic->self.data + head_size);
        assert(tail == polyadic->self.data + polyadic->self.size);
        retval = 0;
    } else {
        retval = -1;
    }

    return retval;
}

polyadic_t* polyadic_load(size_t size, void *data, bool shared)
{
    polyadic_t *polyadic;

    polyadic = malloc(sizeof(polyadic_t));
    if (polyadic) {
        vi_size_t vi_size;
        void *head, *tail;
        uint64_t item_size;

        polyadic->self.size = size;
        polyadic->self.data = data;
        polyadic->self.shared = shared;
        polyadic->nitem = 0;
        polyadic->item = NULL;

        /* scan header to count items */
        head = data;
        tail = data + size;
        while (head < tail) {
            vi_size = vi_to_uint64(head, &item_size);
            if (vi_size < 0)
                break;
            head += vi_size;
            tail -= item_size;
            polyadic->nitem++;
        }

        if (head == tail) {
            /* allocate item array and scan header again for item offsets */
            polyadic->item = malloc(polyadic->nitem * sizeof(polyadic_info_t));
            if (polyadic->item) {
                polyadic_len_t i;
                head = data;
                for (i = 0; i < polyadic->nitem; i++) {
                    vi_size = vi_to_uint64(head, &item_size);
                    assert(vi_size);
                    polyadic->item[i].size = item_size;
                    polyadic->item[i].data = tail;
                    polyadic->item[i].shared = true;
                    head += vi_size;
                    tail += item_size;
                }
            } else {
                free(polyadic);
                polyadic = NULL;
            }
        }else {
            free(polyadic);
            polyadic = NULL;
            errno = EINVAL;
        }
    }
    return polyadic;
}

void polyadic_free(polyadic_t *polyadic)
{
    assert(polyadic);

    polyadic_len_t i;
    for (i = 0; i < polyadic->nitem; i++) {
        if (!polyadic->item[i].shared)
            free(polyadic->item[i].data);
    }
    if (!polyadic->self.shared)
        free(polyadic->self.data);
    free(polyadic->item);
    free(polyadic);
}

#if 0

peanos_t* peanos_prepare(polyadic_len_t nitem)
{
    peanos_t *pack;

    pack = malloc(sizeof(peanos_t));
    if (pack) {
        pack->self.size = 0;
        pack->self.data = NULL;
        pack->self.shared = true;
        pack->item = malloc(nitem * sizeof(pack_info_t));
        if (pack->item) {
            polyadic_len_t i;
            for (i = 0; i < nitem; i++) {
                pack->item[i].size = 0;
                pack->item[i].data = NULL;
                pack->item[i].shared = true;
            }
            pack->nitem = nitem;
        }else {
            free(pack);
            pack = NULL;
        }
    }
    return pack;
}

int peanos_set(peanos_t *pack, polyadic_len_t i, size_t size, void *data, bool shared)
{
    assert(pack);

    int retval;
    if (i < pack->nitem) {
        if (pack->item[i].data && !pack->item[i].shared)
            free(pack->item[i].data);
        pack->item[i].size = size;
        pack->item[i].data = data;
        pack->item[i].shared = shared;
        retval = 0;
    } else {
        errno = EINVAL;
        retval = -1;
    }
    return retval;
}

int peanos_finish(peanos_t *pack)
{
    assert(pack);

    int retval;
    polyadic_len_t i;
    size_t head_size, tmp;
    /* count header size and total required buffer size */
    head_size = 0;
    pack->self.size = 0;
    for (i = 0; i < pack->nitem; i++) {
        /* size of item */
        pack->self.size += pack->item[i].size;
        /* size of varint to required to represent item size */
        tmp = uint64_len(pack->item[i].size);
        head_size += tmp;
        pack->self.size += tmp;
    }

    pack->self.data = malloc(pack->self.size);
    if (pack->self.data) {
        void *head, *tail;
        head = pack->self.data;
        tail = pack->self.data + head_size;
        pack->self.shared = false;

        for (i = 0; i < pack->nitem; i++) {
            head += uint64_to_vi(pack->item[i].size, head);
            /* copy item data to shared buffer */
            memcpy(tail, pack->item[i].data, pack->item[i].size);
            /* free old data */
            if (!pack->item[i].shared)
                free(pack->item[i].data);
            /* point item data to location in shared buffer */
            pack->item[i].data = tail;
            pack->item[i].shared = true;
            tail += pack->item[i].size;
        }
        assert(head == pack->self.data + head_size);
        assert(tail == pack->self.data + pack->self.size);
        retval = 0;
    } else {
        retval = -1;
    }

    return retval;
}

#endif

peanos_t* peanos_new(polyadic_len_t n, uint64_t *values)
{
    peanos_t *pack;

    pack = malloc(sizeof(peanos_t));
    if (pack) {
        polyadic_len_t i;

        pack->n = n;
        pack->shared = false;
        pack->values = values; // take control of the values array

        // calculate resulting size
        pack->size = uint64_len(n);
        for (i = 0; i < n; i++)
            pack->size += uint64_len(values[i]);

        pack->data = malloc(pack->size);
        if (pack->data) {
            size_t off;
            vi_size_t vi_size;

            off = vi_size = uint64_to_vi(n, pack->data);
            if (vi_size) {
                for (i = 0; i < n; i++) {
                    vi_size = uint64_to_vi(values[i], pack->data + off);
                    if (vi_size)
                        off += vi_size;
                    else
                        break;
                }
            }
            if (i == n)
                return pack;
        }
        // failure
        free(pack);
    }
    return NULL;
}

peanos_t* peanos_load(void *data, bool shared, size_t maxlen)
{
    peanos_t *pack;

    pack = malloc(sizeof(peanos_t));
    if (pack) {
        vi_size_t vi_size;
        polyadic_len_t i;
        uint64_t n;

        pack->size = 0;
        pack->data = data;
        pack->shared = shared;
        pack->values = NULL;

        // read the number of stored values
        vi_size = vi_to_uint64_2(data, &n, maxlen);
        if (vi_size == 0) {
            free(pack);
            return NULL;
        }

        // allocate a lookup table
        pack->n = n;
        data += vi_size;
        maxlen -= vi_size;
        pack->values = malloc(pack->n * sizeof(uint64_t));
        if (!pack->values) {
            free(pack);
            return NULL;
        }

        // read values into table
        for (i = 0; i < pack->n && maxlen; i++) {
            vi_size = vi_to_uint64_2(data, &pack->values[i], maxlen);
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

void peanos_free(peanos_t *pack)
{
    assert(pack);

    if (!pack->shared)
        free(pack->data);
    free(pack->values);
    free(pack);
}
