
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

#ifndef _polyadicobject_h_DEFINED
#define _polyadicobject_h_DEFINED

#include <Python.h>
#include "varint.h"
#include "polyadic.h"

typedef struct PyPolyadic_st
{
    PyObject_HEAD
    /* underlying C pack object */
    polyadic_t *pack;
    /* references to parent buffer object, if used */
    Py_buffer *src;
} PyPolyadic;

PyAPI_FUNC(void) PyPolyadic_dealloc(PyPolyadic* self);
PyAPI_FUNC(PyObject *) PyPolyadic_tp_new(PyTypeObject *type, PyObject *args,
        PyObject *kwds);
PyAPI_FUNC(PyObject *) PyPolyadic_FromBuffer(Py_buffer *view, size_t off,
        size_t len);
PyAPI_FUNC(PyObject *) PyPolyadic_FromSequence(PyObject *seq);

/* PyPolyadic buffer API */
PyAPI_FUNC(int) PyPolyadic_getbuffer(PyPolyadic *self, Py_buffer *view, int flags);

/* PyPolyadic sequence API */
PyAPI_FUNC(Py_ssize_t) PyPolyadic_length(PyObject *self);
PyAPI_FUNC(PyObject *) PyPolyadic_item(PyObject *self, Py_ssize_t i);

/* PyPolyadic type definition */
PyAPI_DATA(PyTypeObject) PyPolyadic_Type;


typedef struct PyPeanos_st
{
    PyObject_HEAD
    /* underlying C pack object */
    peanos_t *pack;
    /* references to parent buffer object, if used */
    Py_buffer *src;
} PyPeanos;

/* PyPeanos API */
PyAPI_DATA(PyTypeObject) PyPeanos_Type;
PyAPI_FUNC(void) PyPeanos_dealloc(PyPeanos *self);
PyAPI_FUNC(PyObject *) PyPeanos_tp_new(PyTypeObject *type, PyObject *args,
        PyObject *kwargs);
/* PyPeanos buffer API */
PyAPI_FUNC(PyObject *) PyPeanos_FromBuffer(Py_buffer *view, size_t off);
PyAPI_FUNC(int) PyPeanos_getbuffer(PyPeanos *self, Py_buffer *view,
        int flags);
/* PyPeanos sequence API */
PyAPI_FUNC(PyObject *) PyPeanos_FromSequence(PyObject *seq);
PyAPI_FUNC(Py_ssize_t) PyPeanos_length(PyObject *self);
PyAPI_FUNC(PyObject *) PyPeanos_item(PyObject *self, Py_ssize_t i);

#endif
