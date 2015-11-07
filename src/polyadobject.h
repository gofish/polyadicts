
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

#ifndef _polyadobject_h_DEFINED
#define _polyadobject_h_DEFINED

#include <Python.h>
#include "varint.h"
#include "polyad.h"

typedef struct PyPolyad_st
{
    PyObject_HEAD
    /* underlying C polyad object */
    polyad_t polyad;
    /* references to parent buffer object, if used */
    Py_buffer *src;
} PyPolyad;

PyAPI_FUNC(void) PyPolyad_SetErrFromErrno(void);

PyAPI_FUNC(void) PyPolyad_dealloc(PyPolyad* self);
PyAPI_FUNC(PyObject *) PyPolyad_tp_new(PyTypeObject *type, PyObject *args,
        PyObject *kwds);
PyAPI_FUNC(PyObject *) PyPolyad_FromBuffer(Py_buffer *view, size_t off,
        size_t len);
PyAPI_FUNC(PyObject *) PyPolyad_FromSequence(PyObject *seq, const char *errmsg);

/* PyPolyad buffer API */
PyAPI_FUNC(int) PyPolyad_getbuffer(PyPolyad *self, Py_buffer *view, int flags);

/* PyPolyad sequence API */
PyAPI_FUNC(Py_ssize_t) PyPolyad_length(PyObject *self);
PyAPI_FUNC(PyObject *) PyPolyad_item(PyObject *self, Py_ssize_t i);

/* PyPolyad type definition */
PyAPI_DATA(PyTypeObject) PyPolyad_Type;

#endif
