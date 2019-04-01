#include <Python.h> //有的是#include<Python.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <fcntl.h>  
#include <vector>
#include <algorithm>
#include <iostream>
#include "matrixwriter.h"
#include "matrixreader.h"

using namespace std;

typedef struct{
    PyObject_HEAD
    MatrixWriter *matrix_writer;
}writer;
 
static PyObject *writer_new(PyTypeObject *type, PyObject *self, PyObject *args) {
    writer *m;
    m = (writer *)type->tp_alloc(type, 0);
    m->matrix_writer = new MatrixWriter();
    return (PyObject *)m;
}

static void writer_delete(writer *self) {
    self->matrix_writer->Close();
    PyObject_GC_UnTrack(self);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static int writer_traverse(writer *self, visitproc visit, void *arg) {
    return 0;
}

static int writer_clear(writer *self) {
    return 0;
}

static int writer_init(writer *self, PyObject *args) {
    char *string1;
    char *string2;
    int max;
    if (!PyArg_ParseTuple(args, "ssi", &string1, &string2, &max)) {
        return -1;
    }
    self->matrix_writer->Open(string1, string2, max);
    return 0;
}

static PyObject *writer_append(writer *self, PyObject *args) {
    int id;
    int len;
    char *value;
    if (!PyArg_ParseTuple(args, "isi", &id, &value, &len)) {
        return Py_BuildValue("i", -1);
    }
    self->matrix_writer->Append(id, value, len);
    return Py_BuildValue("i", 0);
}

static PyMethodDef writer_methods[] = {
    {"append", (PyCFunction)writer_append, METH_VARARGS,"append to matrix file"},
    {NULL}  /* Sentinel */
};

static PyTypeObject writer_obj = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "matrixbuilder7.writer",    /* tp_name */
    sizeof(writer),     /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)writer_delete, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE |
        Py_TPFLAGS_HAVE_GC,    /* tp_flags */
    "writer builder v1.0",     /* tp_doc */
    (traverseproc)writer_traverse,/* tp_traverse */
    (inquiry)writer_clear,     /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    writer_methods,            /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)writer_init,     /* tp_init */
    0,                         /* tp_alloc */
    writer_new,                /* tp_new */
};

typedef struct{
    PyObject_HEAD
    MatrixReader *matrix_reader;
}reader;
 
static PyObject *reader_new(PyTypeObject *type, PyObject *self, PyObject *args) {
    reader *m;
    m = (reader *)type->tp_alloc(type, 0);
    m->matrix_reader = new MatrixReader();
    return (PyObject *)m;
}

static void reader_delete(reader *self) {
    self->matrix_reader->Close();
    PyObject_GC_UnTrack(self);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static int reader_traverse(reader *self, visitproc visit, void *arg) {
    return 0;
}

static int reader_clear(reader *self) {
    return 0;
}

static int reader_init(reader *self, PyObject *args) {
    char *string1, *string2;
    if (!PyArg_ParseTuple(args, "ss", &string1, &string2)) {
        return -1;
    }
    self->matrix_reader->Open(string1, string2);
    return 0;
}

static PyObject *reader_get(reader *self, PyObject *args) {
    int uid, len;
    if (!PyArg_ParseTuple(args, "ii", &uid, &len)) {
        return PyList_New(0);
    }
    IndexBody idx = self->matrix_reader->GetIndex((uint32_t)uid);
    uint64_t offset = idx.offset;
    uint64_t size = idx.count;
    if (size == 0 || len <= 0) {
        return PyList_New(0);
    }
    if (size > (uint64_t)len) {
        size = len;
    }
    vector<MatrixBody> *vec = self->matrix_reader->GetData(offset, size);
    int count = 0,length = vec->size();
    PyObject *list = PyList_New(length);
    if (list == NULL) {
        return PyList_New(0);
    }
    for (vector<MatrixBody>::iterator iter = vec->begin();iter != vec->end();iter++) {
        PyObject *dict = PyDict_New();
        PyObject *rid = Py_BuildValue("l", iter->rid);
        PyMapping_SetItemString(dict, "id", rid);
        PyObject *value = Py_BuildValue("l", iter->value);
        PyMapping_SetItemString(dict, "score", value);
        PyList_SET_ITEM(list, count, dict);
        //Py_DECREF(dict);
        Py_DECREF(rid);
        Py_DECREF(value);
        count++;
    }
    for (count = 0;count < length;count++) {
        PyObject *item = PyList_GET_ITEM(list, count);
        if (item == NULL) {
            PyList_SET_ITEM(list, count, Py_None);
        }
    }
    
    return list;
}

static PyObject *reader_get_rev(reader *self, PyObject *args) {
    int uid, head_len, tail_len;
    if (!PyArg_ParseTuple(args, "iii", &uid, &head_len, &tail_len)) {
        return PyList_New(0);
    }
    IndexBody idx = self->matrix_reader->GetIndex((uint32_t)uid);
    uint64_t offset = idx.offset;
    uint64_t size = idx.count;
    uint64_t get_size = (uint64_t)(head_len + tail_len);
    if (size == 0 || get_size == 0) {
        return PyList_New(0);
    }
    vector<MatrixBody> *vec = NULL;
    if (size <= get_size ) {
        vec = self->matrix_reader->GetData(offset, size);
    } else {
        vec = self->matrix_reader->GetDataRev(offset, size,
            head_len, tail_len);
    }
    int count = 0,length = vec->size();
    PyObject *list = PyList_New(length);
    if (list == NULL) {
        return PyList_New(0);
    }
    for (vector<MatrixBody>::iterator iter = vec->begin();iter != vec->end();iter++) {
        PyObject *dict = PyDict_New();
        PyObject *rid = Py_BuildValue("l", iter->rid);
        PyMapping_SetItemString(dict, "id", rid);
        PyObject *value = Py_BuildValue("l", iter->value);
        PyMapping_SetItemString(dict, "score", value);
        PyList_SET_ITEM(list, count, dict);
        //Py_DECREF(dict);
        Py_DECREF(rid);
        Py_DECREF(value);
        count++;
    }
    for (count = 0;count < length;count++) {
        PyObject *item = PyList_GET_ITEM(list, count);
        if (item == NULL) {
            PyList_SET_ITEM(list, count, Py_None);
        }
    }
    
    return list;
}

static PyObject *reader_dict(reader *self, PyObject *args) {
    int uid, len;
    if (!PyArg_ParseTuple(args, "ii", &uid, &len)) {
        return PyDict_New();
    }
    IndexBody idx = self->matrix_reader->GetIndex((uint32_t)uid);
    uint64_t offset = idx.offset;
    uint64_t size = idx.count;
    if (size == 0 || len <= 0) {
        return PyDict_New();
    }
    if (size > (uint64_t)len) {
        size = len;
    }
    vector<MatrixBody> *vec = self->matrix_reader->GetData(offset, size);
    PyObject *dict = PyDict_New();
    for (vector<MatrixBody>::iterator iter = vec->begin();iter != vec->end();iter++) {
        PyObject *rid = Py_BuildValue("l", iter->rid);
        PyObject *value = Py_BuildValue("l", iter->value);
        if (value == NULL || rid == NULL) {
            continue;
        }
        PyDict_SetItem(dict, rid, value);
        Py_DECREF(rid);
        Py_DECREF(value);
    }
    return dict;
}

static PyObject *reader_header(reader *self, PyObject *args) {
    PyObject *list = PyList_New(2);

    uint64_t max_idx = self->matrix_reader->GetMaxIndex();
    PyObject *p_max_idx = Py_BuildValue("l", max_idx);
    PyList_SET_ITEM(list, 0, p_max_idx);

    uint64_t max_data = self->matrix_reader->GetMaxData();
    PyObject *p_max_data = Py_BuildValue("l", max_data);
    PyList_SET_ITEM(list, 1, p_max_data);

    return list;
}

static PyObject *reader_list(reader *self, PyObject *args) {
    int uid, len;
    if (!PyArg_ParseTuple(args, "ii", &uid, &len)) {
        return PyList_New(0);
    }
    IndexBody idx = self->matrix_reader->GetIndex((uint32_t)uid);
    uint64_t offset = idx.offset;
    uint64_t size = idx.count;
    if (size == 0 || len <= 0) {
        return PyList_New(0);
    }
    if (size > (uint64_t)len) {
        size = len;
    }
    vector<MatrixBody> *vec = self->matrix_reader->GetData(offset, size);
    int count = 0,length = vec->size();
    PyObject *list = PyList_New(length);
    if (list == NULL) {
        return PyList_New(0);
    }
    for (vector<MatrixBody>::iterator iter = vec->begin();iter != vec->end();iter++) {
        PyObject *rid = Py_BuildValue("l", iter->rid);
        PyList_SET_ITEM(list, count, rid);
        count++;
    }
    for (count = 0;count < length;count++) {
        PyObject *item = PyList_GET_ITEM(list, count);
        if (item == NULL) {
            PyList_SET_ITEM(list, count, Py_None);
        }
    }
    
    return list;
}

static PyMethodDef reader_methods[] = {
    {"get", (PyCFunction)reader_get, METH_VARARGS,"read idx and data, return id and score"},
    {"getrev", (PyCFunction)reader_get_rev, METH_VARARGS,"read idx and data, return id and score"},
    {"header", (PyCFunction)reader_header, METH_VARARGS,"return idx size and data size"},
    {"lst", (PyCFunction)reader_list, METH_VARARGS,"read idx and data, return id"},
    {"dic", (PyCFunction)reader_dict, METH_VARARGS,"read idx and data, return id and score"},
    {NULL}  /* Sentinel */
};

static PyTypeObject reader_obj = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "matrixbuilder7.reader",    /* tp_name */
    sizeof(reader),     /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)reader_delete, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE |
        Py_TPFLAGS_HAVE_GC,    /* tp_flags */
    "reader builder v1.0",     /* tp_doc */
    (traverseproc)reader_traverse,/* tp_traverse */
    (inquiry)reader_clear,     /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    reader_methods,            /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)reader_init,     /* tp_init */
    0,                         /* tp_alloc */
    reader_new,                /* tp_new */
};


static PyMethodDef module_methods[] = {
    {NULL}
};

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

#if PY_MAJOR_VERSION >= 3
static int matrixbuilder7_traverse(PyObject *m, visitproc visit, void *arg) {
    return 0;
}

static int matrixbuilder7_clear(PyObject *m) {
    return 0;
}

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "matrixbuilder7",
    NULL,
    0,
    module_methods,
    NULL,
    matrixbuilder7_traverse,
    matrixbuilder7_clear,
    NULL
};

PyMODINIT_FUNC PyInit_matrixbuilder7(void) {

#else

PyMODINIT_FUNC initmatrixbuilder7(void) {

#endif
    PyObject *m = NULL;
    
    if (PyType_Ready(&writer_obj) < 0)
        goto END;
    if (PyType_Ready(&reader_obj) < 0)
        goto END;

#if PY_MAJOR_VERSION >= 3
    m = PyModule_Create(&moduledef);
#else
    m = Py_InitModule3("matrixbuilder7", module_methods, "matrix builder module v1.0");
#endif

    if (m == NULL)
        goto END;

    Py_INCREF(&writer_obj);
    PyModule_AddObject(m, "writer", (PyObject *)&writer_obj);
    Py_INCREF(&reader_obj);
    PyModule_AddObject(m, "reader", (PyObject *)&reader_obj);

END:
#if PY_MAJOR_VERSION >= 3
    return m;
#else
    return;
#endif
}

