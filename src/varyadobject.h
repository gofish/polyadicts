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

#ifndef _varyadobject_h_DEFINED
#define _varyadobject_h_DEFINED

#include <Python.h>
#include "varint.h"
#include "varyad.h"

typedef struct PyVaryad_st
{
    PyObject_HEAD
    /* underlying C varyad object */
    varyad_t varyad;
    /* references to parent buffer object, if used */
    Py_buffer *src;
} PyVaryad;

PyAPI_FUNC(void) PyVaryad_dealloc(PyVaryad* self);
PyAPI_FUNC(PyObject *) PyVaryad_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
PyAPI_FUNC(PyObject *) PyVaryad_FromBuffer(Py_buffer *view, size_t off, size_t len);
PyAPI_FUNC(PyObject *) PyVaryad_FromSequence(PyObject *seq);
PyAPI_FUNC(PyObject *) PyVaryad_FromSize(size_t size);

/* PyVaryad buffer API */
PyAPI_FUNC(int) PyVaryad_getbuffer(PyVaryad *self, Py_buffer *view, int flags);

/* PyVaryad sequence API */
PyAPI_FUNC(Py_ssize_t) PyVaryad_length(PyObject *self);
PyAPI_FUNC(PyObject *) PyVaryad_item(PyObject *self, Py_ssize_t i);

/* PyVaryad type definition */
PyAPI_DATA(PyTypeObject) PyVaryad_Type;

#endif
