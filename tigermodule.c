/* Copyright 2008 Corbin Simpson
 * <cds@corbinsimpson.com>
 * This code is provided under the terms of the GNU Public License, version 3.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "tiger.h"

typedef struct {
    PyObject_HEAD
    char* message;
    Py_ssize_t msize;
} tiger_TigerObject;

static PyObject* tiger_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    tiger_TigerObject* self;

    self = (tiger_TigerObject*)type->tp_alloc(type, 0);

    if (self != NULL) {
        self->message = NULL;
        self->msize = 0;
    }

    return (PyObject*)self;
}

static int tiger_init(tiger_TigerObject* self, PyObject* args, PyObject* kwds) {
    char *temp;

    if (!PyArg_ParseTuple(args, "|s#", &temp, &self->msize)) {
        return -1;
    }

    if (temp != NULL) {
        self->message = malloc(self->msize);
        memcpy(self->message, temp, self->msize);
    }

    return 0;
}

static void tiger_dealloc(tiger_TigerObject* self) {
    if (self->message != NULL) {
        free(self->message);
    }

    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* tiger_update(tiger_TigerObject* self, PyObject* args) {
    char *str;
    Py_ssize_t msize;

    if (!PyArg_ParseTuple(args, "s#", &str, &msize)) {
        return NULL;
    }

    /* If this leaks a string, we're going to crash anyway.
     * I regret nothing. */
    self->message = realloc(self->message, self->msize + msize);
    memcpy(self->message + self->msize, str, msize);
    self->msize += msize;

    Py_RETURN_NONE;
}

static PyObject* tiger_digest(tiger_TigerObject* self, PyObject* args) {
    word64 result[3];

    char* message = self->message;
    unsigned long long int msize = self->msize;

    tiger((word64*)message, msize, result);

    return PyBytes_FromStringAndSize((char*)result, 24);
}

static PyObject* tiger_hexdigest(tiger_TigerObject* self, PyObject* args) {
    PyObject* digest = tiger_digest(self, args);
    if (!digest) {
        return NULL;
    }
    return PyObject_CallMethod(digest, "hex", NULL);  // Use hex method on bytes objects in Python 3
}

static PyMethodDef TigerMethods[] = {
    {"digest", (PyCFunction)tiger_digest, METH_NOARGS, "Hash current message and return the result."},
    {"hexdigest", (PyCFunction)tiger_hexdigest, METH_NOARGS, "Hash current message and return the result as a hexadecimal string."},
    {"update", (PyCFunction)tiger_update, METH_VARARGS, "Update current hash string."},
    {NULL, NULL, 0, NULL}  // Sentinel
};

static PyMethodDef TigerModuleMethods[] = {
    {NULL, NULL, 0, NULL}  // Sentinel
};

static PyTypeObject tiger_TigerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "tiger.Tiger",                    /* tp_name */
    sizeof(tiger_TigerObject),         /* tp_basicsize */
    0,                                 /* tp_itemsize */
    (destructor)tiger_dealloc,         /* tp_dealloc */
    0,                                 /* tp_vectorcall_offset */
    0,                                 /* tp_getattr */
    0,                                 /* tp_setattr */
    0,                                 /* tp_as_async */
    0,                                 /* tp_repr */
    0,                                 /* tp_as_number */
    0,                                 /* tp_as_sequence */
    0,                                 /* tp_as_mapping */
    0,                                 /* tp_hash */
    0,                                 /* tp_call */
    0,                                 /* tp_str */
    0,                                 /* tp_getattro */
    0,                                 /* tp_setattro */
    0,                                 /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,    /* tp_flags */
    "Tiger objects",                   /* tp_doc */
    0,                                 /* tp_traverse */
    0,                                 /* tp_clear */
    0,                                 /* tp_richcompare */
    0,                                 /* tp_weaklistoffset */
    0,                                 /* tp_iter */
    0,                                 /* tp_iternext */
    TigerMethods,                      /* tp_methods */
    0,                                 /* tp_members */
    0,                                 /* tp_getset */
    0,                                 /* tp_base */
    0,                                 /* tp_dict */
    0,                                 /* tp_descr_get */
    0,                                 /* tp_descr_set */
    0,                                 /* tp_dictoffset */
    (initproc)tiger_init,              /* tp_init */
    0,                                 /* tp_alloc */
    tiger_new,                         /* tp_new */
};

static struct PyModuleDef tigermodule = {
    PyModuleDef_HEAD_INIT,
    "tiger",
    "Tiger hash module.",
    -1,
    TigerModuleMethods
};

PyMODINIT_FUNC PyInit_tiger(void) {
    PyObject* module;

    if (PyType_Ready(&tiger_TigerType) < 0) {
        return NULL;
    }

    module = PyModule_Create(&tigermodule);
    if (module == NULL) {
        return NULL;
    }

    Py_INCREF(&tiger_TigerType);
    PyModule_AddObject(module, "new", (PyObject*)&tiger_TigerType);
    PyModule_AddObject(module, "tiger", (PyObject*)&tiger_TigerType);
    return module;
}
