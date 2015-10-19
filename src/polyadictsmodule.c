
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

#include "polyadictsmodule.h"
#include "polyadicobjects.h"
#include "ntuple.h"

static PyObject*
_ntuple_frombuffer(PyObject *src)
{
    Py_buffer view;
    size_t rank, size, i;
    uint64_t *info;
    PyObject *ret;

    ret = NULL;
    if (0 == PyObject_GetBuffer(src, &view, PyBUF_SIMPLE)) {
        if (ntuple_rank(view.buf, view.len, &rank)) {
            info = malloc(rank * sizeof(uint64_t));
            if (info) {
                size = ntuple_load(view.buf, view.len, rank, info);
                if (size) {
                    if (size == view.len) {
                        ret = PyTuple_New(rank);
                        if (ret) {
                            for (i = 0; i < rank; i++) {
                                PyTuple_SET_ITEM(ret, i, PyLong_FromUnsignedLongLong(info[i]));
                            }
                        }
                    } else {
                        PyErr_SetString(PyExc_ValueError, "buffer contains trailing data");
                    }
                }
                free(info);
            }
        }
    }
    if (!ret && !PyErr_Occurred()) {
        PyPolyad_SetErrFromErrno();
    }
    return ret;
}

static PyObject *
_ntuple_fromsequence(PyObject *src)
{
    uint64_t *info;
    size_t i, rank, size;
    void *data;
    PyObject *ret, *obj;

    src = PySequence_Fast(src, "expected a sequence of natural numbers");
    if (!src) {
        return NULL;
    }

    rank = PySequence_Fast_GET_SIZE(src);
    if (rank == 0) {
        Py_DECREF(src);
        return PyBytes_FromStringAndSize("\x00", 1);
    }

    ret = NULL;
    info = malloc(rank * sizeof(uint64_t));
    if (info) {
        for (i = 0; i < rank; i++) {
            obj = PySequence_Fast_GET_ITEM(src, i);
            info[i] = PyLong_AsUnsignedLongLong(obj);
            if (PyErr_Occurred()) {
                break;
            }
        }
        if (i == rank) {
            size = ntuple_size(rank, info);
            if (size) {
                data = malloc(size);
                if (data) {
                    ntuple_pack(rank, info, data, size);
                    ret = PyBytes_FromStringAndSize(data, size);
                    free(data);
                }
            }
        }
        free(info);
    }
    Py_DECREF(src);
    return ret;
}

static PyObject *
polyadicts_ntuple(PyObject *self, PyObject *args)
{
    Py_ssize_t rank;
    PyObject *ret, *arg;

    ret = NULL;
    rank = PyTuple_GET_SIZE(args);
    if (rank == 1) {
        arg = PyTuple_GET_ITEM(args, 0);
        if (PyObject_CheckBuffer(arg)) {
            ret = _ntuple_frombuffer(arg);
        } else if (PySequence_Check(arg)) {
            ret = _ntuple_fromsequence(arg);
        } else {
            ret = _ntuple_fromsequence(args);
        }
    } else {
        ret = _ntuple_fromsequence(args);
    }
    return ret;
}

/* polyadicts module method defition */
static PyMethodDef polyadicts_methods[] = {
    {"ntuple", polyadicts_ntuple, METH_VARARGS,
        "Pack or load a sequence of natural numbers"},

    {NULL} // Sentinel
};

/* polyadicts module definition */
static struct PyModuleDef polyadicts_module = {
    PyModuleDef_HEAD_INIT,
    "polyadicts",
    NULL,
    0,
    polyadicts_methods,
    NULL,
    NULL,
    NULL,
    NULL
};

/* polyadicts module initialization function */
PyMODINIT_FUNC PyInit_polyadicts (void)
{
    // Initialize the type objects
    if (PyType_Ready(&PyPolyad_Type) < 0)
        return NULL;

    // Initialize module
    PyObject *module = PyModule_Create(&polyadicts_module);
    if (module != NULL) {
        // Add type object references to the module
        Py_INCREF(&PyPolyad_Type);
        PyModule_AddObject(module, "polyad", (PyObject*)&PyPolyad_Type);
    }
    return module;
}
