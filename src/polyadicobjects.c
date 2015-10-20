
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

#include "polyadicobjects.h"

/**
 * PyPolyad
 */

void
PyPolyad_dealloc(PyPolyad* self)
{
    if (self->polyad)
        polyad_free(self->polyad);
    if (self->src) {
        PyBuffer_Release(self->src);
        PyMem_Del(self->src);
    }
    self->ob_base.ob_type->tp_free((PyObject*)self);
}

void
PyPolyad_SetErrFromErrno()
{
    switch (errno) {
      case ENOMEM:
        PyErr_SetFromErrno(PyExc_MemoryError);
        break;
      case ERANGE:
        PyErr_SetFromErrno(PyExc_OverflowError);
        break;
      case EINVAL:
        PyErr_SetFromErrno(PyExc_ValueError);
        break;
      default:
        PyErr_SetString(PyExc_RuntimeError, "An unknown error has occurred");
        break;
    }
}

PyObject *
PyPolyad_FromBuffer(Py_buffer *view, size_t off, size_t len)
{
    if (len == 0) {
        len = view->len;
    }
    if (len > view->len || off > len) {
        errno = EINVAL;
        PyPolyad_SetErrFromErrno();
        return NULL;
    }

    /* allocate new polyad object */
    PyPolyad *self;
    self = (PyPolyad*) PyPolyad_Type.tp_alloc(&PyPolyad_Type, 0);
    if (!self)
        return NULL;

    /* allocate Py_buffer to refcount shared memory region */
    self->src = PyMem_Malloc(sizeof(Py_buffer));
    if (!self->src) {
        PyPolyad_Type.tp_free(self);
        return NULL;
    }
    *self->src = *view;

    /* load and initialize polyad pointers from data buffer */
    if (polyad_load(view->buf + off, len, &self->polyad)) {
        return (PyObject*) self;

    } else {
        /* failure */
        PyPolyad_SetErrFromErrno();
        PyMem_Free(self->src);
        PyPolyad_Type.tp_free(self);
        return NULL;
    }
}

PyObject *
PyPolyad_FromSequence(PyObject *src, const char *errmsg)
{
    if (NULL == (src = PySequence_Fast(src, errmsg)))
        return NULL;

    Py_ssize_t rank = PySequence_Fast_GET_SIZE(src);

    Py_ssize_t i;
    Py_buffer view[rank];
    const void *items[rank];
    size_t lens[rank];
    Py_ssize_t utf_len;

    polyad_t polyad = NULL;
    PyPolyad *self = NULL;

    for (i = 0; i < rank; i++) {
        PyObject *const obj = PySequence_Fast_GET_ITEM(src, i);
        if (PyObject_CheckBuffer(obj) &&
                0 == PyObject_GetBuffer(obj, &view[i], PyBUF_SIMPLE)) {
            items[i] = view[i].buf;
            lens[i] = view[i].len;
        } else if (PyUnicode_Check(obj) && 0 == PyUnicode_READY(obj)) {
            items[i] = PyUnicode_AsUTF8AndSize(obj, &utf_len);
            if (items[i]) {
                lens[i] = utf_len;
                view[i].buf = NULL;
            } else {
                break;
            }
        } else {
            break;
        }
    }

    if (i == rank) {
        /* initialize the polyad from the item buffers */
        polyad_init(rank, items, lens, &polyad);
    } else {
        PyErr_SetString(PyExc_TypeError, errmsg);
    }

    /* release all open buffers */
    rank = i;
    for (i = 0; i < rank; i++) {
        if (view[i].buf) {
            PyBuffer_Release(&view[i]);
        }
    }

    /* allocate new PyPolyad object */
    if (polyad) {
        self = (PyPolyad*) PyPolyad_Type.tp_alloc(&PyPolyad_Type, 0);
        if (self) {
            self->polyad = polyad;
            self->src = NULL;
        } else {
            polyad_free(polyad);
        }
    } else if (!PyErr_Occurred()) {
        PyPolyad_SetErrFromErrno();
    }
    return (PyObject*) self;
}

PyObject *
PyPolyad_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *src;
    if (!PyArg_ParseTuple(args, "O", &src))
        return NULL;

    Py_buffer view;
    if (PyObject_CheckBuffer(src) &&
            0 == PyObject_GetBuffer(src, &view, PyBUF_SIMPLE)) {
        PyObject *pack = PyPolyad_FromBuffer(&view, 0, 0);
        if (!pack)
            PyBuffer_Release(&view);
        return pack;
    }

    return PyPolyad_FromSequence(src,
            "expected a sequence (encode) or bufferable (decode)");
}

/* PyPolyad buffer API */
int
PyPolyad_getbuffer(PyPolyad *self, Py_buffer *view, int flags)
{
    return PyBuffer_FillInfo(view, (PyObject*)self, (void *) polyad_data(self->polyad),
            polyad_size(self->polyad), true, PyBUF_SIMPLE);
}

PyBufferProcs PyPolyad_as_buffer = {
    (getbufferproc)PyPolyad_getbuffer,
    NULL,
};

/* PyPolyad sequence API */
Py_ssize_t
PyPolyad_length(PyObject *self)
{
    return polyad_rank(((PyPolyad*)self)->polyad);
}

PyObject*
PyPolyad_item(PyObject *obj_self, Py_ssize_t i)
{
    PyPolyad *self = (PyPolyad*) obj_self;
    if (i >= polyad_rank(self->polyad)) {
        PyErr_SetString(PyExc_IndexError, "pack index out of range");
        return NULL;
    }

    Py_buffer view;
    if (0 == PyObject_GetBuffer(obj_self, &view, PyBUF_SIMPLE)) {
        view.len = polyad_item(self->polyad, i, (const void **) &view.buf);
        return PyMemoryView_FromBuffer(&view);
    }
    return NULL;
}

PySequenceMethods PyPolyad_as_sequence = {
    (lenfunc)PyPolyad_length, /*sq_length*/
    NULL,                       /*sq_concat*/
    NULL,                       /*sq_repeat*/
    (ssizeargfunc)PyPolyad_item,  /*sq_item*/
    NULL,                       /*sq_ass_item*/
    NULL,                       /*sq_contains*/
    NULL,                       /*sq_inplace_concat*/
    NULL,                       /*sq_inplace_repeat*/
};

/* PyPolyad type definition */
PyTypeObject PyPolyad_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "polyadicts.polyad",             /*tp_name*/
    sizeof(PyPolyad),         /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    (destructor)PyPolyad_dealloc, /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    0,                          /*tp_compare*/
    0,                          /*tp_repr*/
    0,                          /*tp_as_number*/
    &PyPolyad_as_sequence,    /*tp_as_sequence*/
    0,                          /*tp_as_mapping*/
    0,                          /*tp_hash */
    0,                          /*tp_call*/
    0,                          /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    &PyPolyad_as_buffer,      /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,         /*tp_flags*/
    "polyad(bufferable | sequence)", /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    0,                          /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    0,                          /* tp_init */
    0,                          /* tp_alloc */
    PyPolyad_tp_new,          /* tp_new */
};
