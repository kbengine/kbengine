/*
 * Copyright (c) 2007 Hyperic, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Python.h>
#include "sigar.h"
#include "sigar_fileinfo.h"
#include "sigar_format.h"

#define PySigarString_FromNetAddr(a) pysigar_net_address_to_string(&a)

#define PySigarInt_FromChar(c) PyInt_FromLong((int)c)

#define PySigar_ParsePID \
    if (!PyArg_ParseTuple(args, "i", &pid)) return NULL

#define PySigar_ParseName \
    if (!PyArg_ParseTuple(args, "s", &name, &name_len)) return NULL

#define PySIGAR_OBJ ((PySigarObject *)self)

#define PySIGAR ((sigar_t *)PySIGAR_OBJ->ptr)

#define PySigar_new(t) PyType_GenericAlloc(&t, 0)

#define PySigar_TPFLAGS Py_TPFLAGS_DEFAULT

#define PySigar_AddType(name, t) \
    if (PyType_Ready(&t) == 0) { \
        Py_INCREF(&t); \
        PyModule_AddObject(module, name, (PyObject *)&t); \
    }

#define PySigar_Croak() PyErr_SetString(PyExc_ValueError, sigar_strerror(sigar, status))

typedef struct {
    PyObject_HEAD
    void *ptr;
} PySigarObject;

static PyTypeObject pysigar_PySigarType;

static void pysigar_free(PyObject *self)
{
    if (PySIGAR_OBJ->ptr) {
        if (self->ob_type == &pysigar_PySigarType) {
            sigar_close(PySIGAR);
        }
        else {
            free(PySIGAR_OBJ->ptr);
        }
        PySIGAR_OBJ->ptr = NULL;
    }

    self->ob_type->tp_free((PyObject *)self);
}

static PyObject *pysigar_net_address_to_string(sigar_net_address_t *address)
{
    char addr_str[SIGAR_INET6_ADDRSTRLEN];
    sigar_net_address_to_string(NULL, address, addr_str);
    return PyString_FromString(addr_str);
}

#include "_sigar_generated.c"

static PyObject *pysigar_open(PyObject *pyself, PyObject *args)
{
    PyObject *self = PySigar_new(pysigar_PySigarType);
    sigar_open((sigar_t **)&PySIGAR_OBJ->ptr);
    return self;
}

static PyObject *pysigar_close(PyObject *self, PyObject *args)
{
    if (PySIGAR_OBJ->ptr) {
        sigar_close(PySIGAR);
        PySIGAR_OBJ->ptr = NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static int pysigar_parse_uint64(PyObject *args, sigar_uint64_t *val)
{
    PyObject *obj;

    if (!PyArg_ParseTuple(args, "O", &obj)) {
        return !SIGAR_OK;
    }

    if (PyInt_Check(obj)) {
        *val = PyInt_AsUnsignedLongLongMask(obj);
    }
    else if (PyLong_Check(obj)) {
        *val = PyLong_AsUnsignedLongLong(obj);
    }
    else {
        return !SIGAR_OK;
    }
    return SIGAR_OK;
}

static PyObject *pysigar_new_strlist(char **data, unsigned long number)
{
    unsigned long i;
    PyObject *av;

    if (!(av = PyTuple_New(number))) {
        return NULL;
    }

    for (i=0; i<number; i++) {
        PyTuple_SET_ITEM(av, i, PyString_FromString(data[i]));
    }

    return av;
}

static PyObject *pysigar_new_list(char *data, unsigned long number,
                                  int size, PyTypeObject *type)
{
    unsigned long i;
    PyObject *av = PyTuple_New(number);

    for (i=0; i<number; i++, data += size) {
        void *ent = malloc(size);
        PyObject *self = PyType_GenericAlloc(type, 0);
        memcpy(ent, data, size);
        PySIGAR_OBJ->ptr = ent;
        PyTuple_SET_ITEM(av, i, self);
    }

    return av;
}

static PyObject *pysigar_file_system_list(PyObject *self, PyObject *args)
{
    int status;
    sigar_t *sigar = PySIGAR;
    sigar_file_system_list_t fslist;
    PyObject *RETVAL;

    status = sigar_file_system_list_get(sigar, &fslist);
    if (status != SIGAR_OK) {
        PySigar_Croak();
        return NULL;
    }

    RETVAL = pysigar_new_list((char *)&fslist.data[0],
                              fslist.number,
                              sizeof(*fslist.data),
                              &pysigar_PySigarFileSystemType);

    sigar_file_system_list_destroy(sigar, &fslist);

    return RETVAL;
}

static PyObject *pysigar_net_interface_list(PyObject *self, PyObject *args)
{
    int status;
    sigar_t *sigar = PySIGAR;
    sigar_net_interface_list_t iflist;
    PyObject *RETVAL;

    status = sigar_net_interface_list_get(sigar, &iflist);
    if (status != SIGAR_OK) {
        PySigar_Croak();
        return NULL;
    }

    RETVAL = pysigar_new_strlist(iflist.data, iflist.number);

    sigar_net_interface_list_destroy(sigar, &iflist);

    return RETVAL;
}

static PyObject *pysigar_format_size(PyObject *self, PyObject *args)
{
    char buffer[56];
    sigar_uint64_t size;

    if (pysigar_parse_uint64(args, &size) == SIGAR_OK) {
        return PyString_FromString(sigar_format_size(size, buffer));
    }
    else {
        return NULL;
    }
}

static PyMethodDef pysigar_methods[] = {
    { "close", pysigar_close, METH_NOARGS, NULL },
    { "net_interface_list", pysigar_net_interface_list, METH_NOARGS, NULL },
    { "file_system_list", pysigar_file_system_list, METH_NOARGS, NULL },
    PY_SIGAR_METHODS
    {NULL}
};

static PyTypeObject pysigar_PySigarType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "Sigar",                   /*tp_name*/
    sizeof(PySigarObject),     /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    pysigar_free,              /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    PySigar_TPFLAGS,           /*tp_flags*/
    0,                         /*tp_doc*/
    0,                         /*tp_traverse*/
    0,                         /*tp_clear*/
    0,                         /*tp_richcompare*/
    0,                         /*tp_weaklistoffset*/
    0,                         /*tp_iter*/
    0,                         /*tp_iternext*/
    pysigar_methods,           /*tp_methods*/
    0,                         /*tp_members*/
    0,                         /*tp_getset*/
    0,                         /*tp_base*/
    0,                         /*tp_dict*/
    0,                         /*tp_descr_get*/
    0,                         /*tp_descr_set*/
    0,                         /*tp_dictoffset*/
    0,                         /*tp_init*/
    0,                         /*tp_alloc*/
    0                          /*tp_new*/
};

static PyMethodDef pysigar_module_methods[] = {
    { "open", pysigar_open, METH_NOARGS, NULL },
    { "format_size", pysigar_format_size, METH_VARARGS, NULL },
    {NULL}
};

#define PY_SIGAR_CONST_INT(name) \
    PyDict_SetItemString(dict, #name, o=PyInt_FromLong(SIGAR_##name)); Py_DECREF(o)

#define PY_SIGAR_CONST_STR(name) \
    PyDict_SetItemString(dict, #name, o=PyString_FromString(SIGAR_##name)); Py_DECREF(o)

static void init_pysigar_constants(PyObject *dict)
{
    PyObject *o;

    PY_SIGAR_CONST_INT(FIELD_NOTIMPL);

    PY_SIGAR_CONST_INT(IFF_UP);
    PY_SIGAR_CONST_INT(IFF_BROADCAST);
    PY_SIGAR_CONST_INT(IFF_DEBUG);
    PY_SIGAR_CONST_INT(IFF_LOOPBACK);
    PY_SIGAR_CONST_INT(IFF_POINTOPOINT);
    PY_SIGAR_CONST_INT(IFF_NOTRAILERS);
    PY_SIGAR_CONST_INT(IFF_RUNNING);
    PY_SIGAR_CONST_INT(IFF_NOARP);
    PY_SIGAR_CONST_INT(IFF_PROMISC);
    PY_SIGAR_CONST_INT(IFF_ALLMULTI);
    PY_SIGAR_CONST_INT(IFF_MULTICAST);

    PY_SIGAR_CONST_STR(NULL_HWADDR);
}

PyMODINIT_FUNC
init_sigar(void) 
{
    PyObject *module =
        Py_InitModule("_sigar", pysigar_module_methods);

    PySigar_AddType("Sigar", pysigar_PySigarType);

    PY_SIGAR_ADD_TYPES;

    init_pysigar_constants(PyModule_GetDict(module));
}
