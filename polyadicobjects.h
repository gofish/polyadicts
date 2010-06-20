
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

#ifndef _polyadobject_h_DEFINED
#define _polyadobject_h_DEFINED

#include <Python.h>
#include "varint.h"
#include "polyadics.h"

typedef struct PyPolyad_st
{
    PyObject_HEAD
    /* underlying C pack object */
    polyad_t *pack;
    /* references to parent buffer object, if used */
    Py_buffer *src;
} PyPolyad;

PyAPI_FUNC(void) PyPolyad_dealloc(PyPolyad* self);
PyAPI_FUNC(PyObject *) PyPolyad_tp_new(PyTypeObject *type, PyObject *args,
        PyObject *kwds);
PyAPI_FUNC(PyObject *) PyPolyad_FromBuffer(Py_buffer *view, size_t off,
        size_t len);
PyAPI_FUNC(PyObject *) PyPolyad_FromSequence(PyObject *seq);

/* PyPolyad buffer API */
PyAPI_FUNC(int) PyPolyad_getbuffer(PyPolyad *self, Py_buffer *view, int flags);

/* PyPolyad sequence API */
PyAPI_FUNC(Py_ssize_t) PyPolyad_length(PyObject *self);
PyAPI_FUNC(PyObject *) PyPolyad_item(PyObject *self, Py_ssize_t i);

/* PyPolyad type definition */
PyAPI_DATA(PyTypeObject) PyPolyad_Type;


typedef struct PyPolyid_st
{
    PyObject_HEAD
    /* underlying C pack object */
    polyid_t *pack;
    /* references to parent buffer object, if used */
    Py_buffer *src;
} PyPolyid;

/* PyPolyid API */
PyAPI_DATA(PyTypeObject) PyPolyid_Type;
PyAPI_FUNC(void) PyPolyid_dealloc(PyPolyid *self);
PyAPI_FUNC(PyObject *) PyPolyid_tp_new(PyTypeObject *type, PyObject *args,
        PyObject *kwargs);
/* PyPolyid buffer API */
PyAPI_FUNC(PyObject *) PyPolyid_FromBuffer(Py_buffer *view, size_t off);
PyAPI_FUNC(int) PyPolyid_getbuffer(PyPolyid *self, Py_buffer *view,
        int flags);
/* PyPolyid sequence API */
PyAPI_FUNC(PyObject *) PyPolyid_FromSequence(PyObject *seq);
PyAPI_FUNC(Py_ssize_t) PyPolyid_length(PyObject *self);
PyAPI_FUNC(PyObject *) PyPolyid_item(PyObject *self, Py_ssize_t i);

#endif
