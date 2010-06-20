
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

#include "polyadicobject.h"

void
PyPolyadic_dealloc(PyPolyadic* self)
{
    if (self->pack)
        polyadic_free(self->pack);
    if (self->src) {
        PyBuffer_Release(self->src);
        free(self->src);
    }
    self->ob_base.ob_type->tp_free((PyObject*)self);
}

PyObject *
PyPolyadic_FromBuffer(Py_buffer *view, size_t off, size_t len)
{
    if (len == 0)
        len = view->len;
    if (len > view->len || off > len) {
        errno = EINVAL;
        PyErr_SetFromErrno(PyExc_ValueError);
        return NULL;
    }

    /* allocate new pack object */
    PyPolyadic *self;
    self = (PyPolyadic*) PyPolyadic_Type.tp_alloc(&PyPolyadic_Type, 0);
    if (!self)
        return NULL;

    /* allocate Py_buffer to refcount shared memory region */
    self->src = malloc(sizeof(Py_buffer));
    if (!self->src) {
        PyPolyadic_Type.tp_free(self);
        return NULL;
    }
    *self->src = *view;

    /* load and initialize pack pointers from data buffer */
    self->pack = polyadic_load(len, view->buf + off, true);
    if (!self->pack) {
        PyErr_SetFromErrno(PyExc_ValueError);
        free(self->src);
        PyPolyadic_Type.tp_free(self);
        return NULL;
    }
    /* prevent access to the owned buffer reference */
    memset(view, 0, sizeof(Py_buffer));
    return (PyObject*) self;
}

PyObject *
PyPolyadic_FromSequence(PyObject *src)
{
    if (NULL == (src = PySequence_Fast(src, NULL)))
        return NULL;

    Py_ssize_t len = PySequence_Fast_GET_SIZE(src);
    polyadic_t *pack;
    pack = polyadic_prepare(len);
    if (!pack) {
        PyErr_SetFromErrno(PyExc_MemoryError);
        return NULL;
    }

    Py_ssize_t i;
    Py_buffer view[len];
    for (i = 0; i < len; i++) {
        PyObject *obj = PySequence_Fast_GET_ITEM(src, i);
        if (0 != PyObject_GetBuffer(obj, &view[i], PyBUF_SIMPLE))
            break;
        polyadic_set(pack, i, view[i].len, view[i].buf, true);
    }

    if (i != len || 0 != polyadic_finish(pack)) {
        polyadic_free(pack);
        pack = NULL;
    }

    /* release all open buffers */
    len = i;
    for (i = 0; i < len; i++)
        PyBuffer_Release(&view[i]);
    if (!pack)
        return NULL;

    /* allocate new PyPolyadic object */
    PyPolyadic *self;
    self = (PyPolyadic*) PyPolyadic_Type.tp_alloc(&PyPolyadic_Type, 0);
    if (self) {
        self->pack = pack;
        self->src = NULL;
    }
    return (PyObject*) self;
}

PyObject *
PyPolyadic_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *src;
    if (!PyArg_ParseTuple(args, "O", &src))
        return NULL;

    Py_buffer view;
    if (PyObject_CheckBuffer(src) &&
            0 == PyObject_GetBuffer(src, &view, PyBUF_SIMPLE)) {
        PyObject *pack = PyPolyadic_FromBuffer(&view, 0, 0);
        if (!pack)
            PyBuffer_Release(&view);
        return pack;
    }

    return PyPolyadic_FromSequence(src);
}

/* PyPolyadic buffer API */
int
PyPolyadic_getbuffer(PyPolyadic *self, Py_buffer *view, int flags)
{
    return PyBuffer_FillInfo(view, (PyObject*)self, self->pack->self.data,
            self->pack->self.size, true, PyBUF_SIMPLE);
}

PyBufferProcs PyPolyadic_as_buffer = {
    (getbufferproc)PyPolyadic_getbuffer,
    NULL,
};

/* PyPolyadic sequence API */
Py_ssize_t
PyPolyadic_length(PyObject *self)
{
    return ((PyPolyadic*)self)->pack->nitem;
}

PyObject*
PyPolyadic_item(PyObject *obj_self, Py_ssize_t i)
{
    PyPolyadic *self = (PyPolyadic*) obj_self;
    if (i >= self->pack->nitem) {
        PyErr_SetString(PyExc_IndexError, "pack index out of range");
        return NULL;
    }

    Py_buffer view;
    if (0 == PyObject_GetBuffer(obj_self, &view, PyBUF_SIMPLE)) {
        view.buf = self->pack->item[i].data;
        view.len = self->pack->item[i].size;
        return PyMemoryView_FromBuffer(&view);
    }
    return NULL;
}

PySequenceMethods PyPolyadic_as_sequence = {
    (lenfunc)PyPolyadic_length, /*sq_length*/
    NULL,                       /*sq_concat*/
    NULL,                       /*sq_repeat*/
    (ssizeargfunc)PyPolyadic_item,  /*sq_item*/
    NULL,                       /*sq_ass_item*/
    NULL,                       /*sq_contains*/
    NULL,                       /*sq_inplace_concat*/
    NULL,                       /*sq_inplace_repeat*/
};

/* PyPolyadic type definition */
PyTypeObject PyPolyadic_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_pt.polyadic",             /*tp_name*/
    sizeof(PyPolyadic),         /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    (destructor)PyPolyadic_dealloc, /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    0,                          /*tp_compare*/
    0,                          /*tp_repr*/
    0,                          /*tp_as_number*/
    &PyPolyadic_as_sequence,    /*tp_as_sequence*/
    0,                          /*tp_as_mapping*/
    0,                          /*tp_hash */
    0,                          /*tp_call*/
    0,                          /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    &PyPolyadic_as_buffer,      /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,         /*tp_flags*/
    "pack(bufferable | sequence)", /* tp_doc */
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
    PyPolyadic_tp_new,          /* tp_new */
};



void
PyPeanos_dealloc(PyPeanos* self)
{
    if (self->pack)
        peanos_free(self->pack);
    if (self->src) {
        PyBuffer_Release(self->src);
        free(self->src);
    }
    self->ob_base.ob_type->tp_free((PyObject*)self);
}

PyObject *
PyPeanos_FromBuffer(Py_buffer *view, size_t off)
{
    /* allocate new pack object */
    PyPeanos *self;
    self = (PyPeanos*) PyPeanos_Type.tp_alloc(&PyPeanos_Type, 0);
    if (!self)
        return NULL;

    /* allocate Py_buffer to refcount shared memory region */
    self->src = malloc(sizeof(Py_buffer));
    if (!self->src) {
        PyPeanos_Type.tp_free(self);
        return NULL;
    }
    *self->src = *view;

    /* load and initialize pack pointers from data buffer */
    self->pack = peanos_load(view->buf + off, true, view->len - off);
    if (!self->pack) {
        PyErr_SetFromErrno(PyExc_ValueError);
        free(self->src);
        PyPeanos_Type.tp_free(self);
        return NULL;
    }
    /* prevent access to the owned buffer reference */
    memset(view, 0, sizeof(Py_buffer));
    return (PyObject*) self;
}

PyObject *
PyPeanos_FromSequence(PyObject *src)
{
    if (NULL == (src = PySequence_Fast(src, NULL)))
        return NULL;

    polyadic_len_t n = PySequence_Fast_GET_SIZE(src);
    uint64_t *values = malloc(n * sizeof(uint64_t));
    if (!values) {
        PyErr_SetFromErrno(PyExc_MemoryError);
        Py_DECREF(src);
        return NULL;
    }

    // convert to 64-bit unsigned ints
    polyadic_len_t i;
    for (i = 0; i < n; i++) {
        PyObject *obj = PySequence_Fast_GET_ITEM(src, i);
        if (PyLong_Check(obj)) {
            values[i] = PyLong_AsUnsignedLong(obj);
        } else {
            break;
        }
    }
    Py_DECREF(src);
    if (i != n) {
        PyErr_SetString(PyExc_TypeError, "peanos sequence must be numbers");
        free(values);
        return NULL;
    }

    peanos_t *pack;
    pack = peanos_new(n, values);
    if (!pack) {
        PyErr_SetFromErrno(PyExc_MemoryError);
        free(values);
        return NULL;
    }

    /* allocate new PyPeanos object */
    PyPeanos *self;
    self = (PyPeanos*) PyPeanos_Type.tp_alloc(&PyPeanos_Type, 0);
    if (self) {
        self->pack = pack;
        self->src = NULL;
    }
    return (PyObject*) self;
}

PyObject *
PyPeanos_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *src;
    if (!PyArg_ParseTuple(args, "O", &src))
        return NULL;

    Py_buffer view;
    if (PyObject_CheckBuffer(src) &&
            0 == PyObject_GetBuffer(src, &view, PyBUF_SIMPLE)) {
        PyObject *pack = PyPeanos_FromBuffer(&view, 0);
        if (!pack)
            PyBuffer_Release(&view);
        return pack;
    }

    return PyPeanos_FromSequence(src);
}


/* PyPeanos buffer API */
int
PyPeanos_getbuffer(PyPeanos *self, Py_buffer *view, int flags)
{
    return PyBuffer_FillInfo(view, (PyObject*)self, self->pack->data,
            self->pack->size, true, PyBUF_SIMPLE);
}


PyBufferProcs PyPeanos_as_buffer = {
    (getbufferproc)PyPeanos_getbuffer,
    NULL,
};

/* PyPeanos sequence API */
Py_ssize_t
PyPeanos_length(PyObject *self)
{
    return ((PyPeanos*)self)->pack->n;
}

PyObject*
PyPeanos_item(PyObject *obj_self, Py_ssize_t i)
{
    PyPeanos *self = (PyPeanos*) obj_self;
    if (i >= self->pack->n) {
        PyErr_SetString(PyExc_IndexError, "pack index out of range");
        return NULL;
    }

    return PyLong_FromUnsignedLong(self->pack->values[i]);
}

PySequenceMethods PyPeanos_as_sequence = {
    (lenfunc)PyPeanos_length,   /*sq_length*/
    NULL,                       /*sq_concat*/
    NULL,                       /*sq_repeat*/
    (ssizeargfunc)PyPeanos_item, /*sq_item*/
    NULL,                       /*sq_ass_item*/
    NULL,                       /*sq_contains*/
    NULL,                       /*sq_inplace_concat*/
    NULL,                       /*sq_inplace_repeat*/
};

/* PyPeanos type definition */
PyTypeObject PyPeanos_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_pt.peanos",               /*tp_name*/
    sizeof(PyPeanos),           /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    (destructor)PyPeanos_dealloc, /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    0,                          /*tp_compare*/
    0,                          /*tp_repr*/
    0,                          /*tp_as_number*/
    &PyPeanos_as_sequence,      /*tp_as_sequence*/
    0,                          /*tp_as_mapping*/
    0,                          /*tp_hash */
    0,                          /*tp_call*/
    0,                          /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    &PyPeanos_as_buffer,        /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,         /*tp_flags*/
    "peanos(bufferable | sequence)",   /* tp_doc */
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
    PyPeanos_tp_new,            /* tp_new */
};
