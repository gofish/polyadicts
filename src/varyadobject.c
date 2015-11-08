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

#include "polyadobject.h"
#include "varyadobject.h"

/**
 * PyVaryad
 */

void
PyVaryad_dealloc(PyVaryad* self)
{
    if (self->varyad)
        varyad_free(self->varyad);
    if (self->src) {
        PyBuffer_Release(self->src);
        PyMem_Del(self->src);
    }
    self->ob_base.ob_type->tp_free((PyObject*)self);
}

PyObject *
PyVaryad_FromBuffer(Py_buffer *view, size_t off, size_t len)
{
    PyErr_SetString(PyExc_NotImplementedError, "varyad(buffer) not implemented");
    return NULL;
}

PyObject *
PyVaryad_FromSequence(PyObject *seq)
{
    PyErr_SetString(PyExc_NotImplementedError, "varyad(sequence) not implemented");
    return NULL;
}

PyObject *
PyVaryad_FromSize(size_t size)
{
    /* allocate new polyad object */
    PyVaryad *self = (PyVaryad*) PyVaryad_Type.tp_alloc(&PyVaryad_Type, 0);
    if (self) {
        if (!varyad_init(size, &self->varyad)) {
            PyPolyad_SetErrFromErrno();
            Py_DECREF(self);
            self = NULL;
        }
    }
    return (PyObject *) self;

}

PyObject *
PyVaryad_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *arg = NULL;
    if (!PyArg_ParseTuple(args, "|O", &arg)) {
        return NULL;

    } else if (arg == NULL) {
        return PyVaryad_FromSize(512);

    } else {
        const unsigned PY_LONG_LONG size = PyLong_AsUnsignedLongLong(arg);
        if (!PyErr_Occurred()) {
            return PyVaryad_FromSize(size);

        } else {
            Py_buffer view;
            if (PyObject_CheckBuffer(arg) &&
                    0 == PyObject_GetBuffer(arg, &view, PyBUF_SIMPLE)) {
                PyObject *const v = PyVaryad_FromBuffer(&view, 0, 0);
                PyBuffer_Release(&view);
                return v;

            } else if (PySequence_Check(arg)) {
                return PyVaryad_FromSequence(arg);

            } else {
                PyErr_SetString(PyExc_TypeError,
                        "expected a positive integer, a bufferable, or a sequence");
                return NULL;
            }
        }
    }
}

/* PyVaryad buffer API */
int
PyVaryad_getbuffer(PyVaryad *self, Py_buffer *view, int flags)
{
    return PyBuffer_FillInfo(view, (PyObject*)self, (void *) varyad_data(self->varyad),
            varyad_size(self->varyad), 1, PyBUF_SIMPLE);
}

PyBufferProcs PyVaryad_as_buffer = {
    (getbufferproc)PyVaryad_getbuffer,
    NULL,
};

/* PyVaryad sequence API */
Py_ssize_t
PyVaryad_length(PyObject *self)
{
    return varyad_rank(((PyVaryad*)self)->varyad);
}

PyObject*
PyVaryad_item(PyObject *obj_self, Py_ssize_t i)
{
    PyVaryad *self = (PyVaryad*) obj_self;
    if (i >= varyad_rank(self->varyad)) {
        PyErr_SetString(PyExc_IndexError, "pack index out of range");
        return NULL;
    }

    Py_buffer view;
    if (0 == PyObject_GetBuffer(obj_self, &view, PyBUF_SIMPLE)) {
        view.len = varyad_item(self->varyad, i, (const void **) &view.buf);
        return PyMemoryView_FromBuffer(&view);
    }
    return NULL;
}

PySequenceMethods PyVaryad_as_sequence = {
    (lenfunc)PyVaryad_length,   /*sq_length*/
    NULL,                       /*sq_concat*/
    NULL,                       /*sq_repeat*/
    (ssizeargfunc)PyVaryad_item,/*sq_item*/
    NULL,                       /*sq_ass_item*/
    NULL,                       /*sq_contains*/
    NULL,                       /*sq_inplace_concat*/
    NULL,                       /*sq_inplace_repeat*/
};


static PyObject *
PyVaryad_push(PyVaryad *self, PyObject *obj)
{
    Py_buffer view;
    if (PyObject_CheckBuffer(obj)) {
        if (0 == PyObject_GetBuffer(obj, &view, PyBUF_SIMPLE)) {
            if (varyad_push(&self->varyad, view.buf, view.len, 1)) {
                Py_INCREF(Py_None);
                return Py_None;
            } else {
                PyPolyad_SetErrFromErrno();
            }
        }
    }
    return NULL;
}

static PyMethodDef PyVaryad_methods[] = {
    {"push", (PyCFunction)PyVaryad_push, METH_O, "Push a data element onto the end of a varyad" },
    {NULL}  /* Sentinel */
};

/* PyVaryad type definition */
PyTypeObject PyVaryad_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "polyadicts.varyad",        /*tp_name*/
    sizeof(PyVaryad),           /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    (destructor)PyVaryad_dealloc, /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    0,                          /*tp_compare*/
    0,                          /*tp_repr*/
    0,                          /*tp_as_number*/
    &PyVaryad_as_sequence,      /*tp_as_sequence*/
    0,                          /*tp_as_mapping*/
    0,                          /*tp_hash */
    0,                          /*tp_call*/
    0,                          /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    &PyVaryad_as_buffer,        /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,         /*tp_flags*/
    "varyad(bufferable | sequence)", /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    PyVaryad_methods,           /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    0,                          /* tp_init */
    0,                          /* tp_alloc */
    PyVaryad_tp_new,            /* tp_new */
};
