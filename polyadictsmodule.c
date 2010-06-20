
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
* and the GNU Lesser Public License along with PerTwisted.  If not, see
** <http://www.gnu.org/licenses/>.
*/

#include "polyadictsmodule.h"
#include "polyadicobjects.h"

/* polyadicts module method defition */
static PyMethodDef polyadicts_methods[] = {
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
    if (PyType_Ready(&PyPolyad_Type) < 0 ||
            PyType_Ready(&PyPolyid_Type) < 0)
        return NULL;

    // Initialize module
    PyObject *module = PyModule_Create(&polyadicts_module);
    if (module != NULL) {
        // Add type object references to the module
        Py_INCREF(&PyPolyad_Type);
        PyModule_AddObject(module, "polyad", (PyObject*)&PyPolyad_Type);
        Py_INCREF(&PyPolyid_Type);
        PyModule_AddObject(module, "polyid", (PyObject*)&PyPolyid_Type);
    }
    return module;
}
