#include <Python.h>
#include <structmember.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <getdns/getdns.h>
#include <getdns/getdns_ext_libevent.h>
#include <event2/event.h>
#include <pthread.h>
#include "pygetdns.h"


int
result_init(getdns_ResultObject *self, PyObject *args, PyObject *keywds)
{
    PyObject *result_capsule;
    struct getdns_dict *result_dict;
    int  status;
    int  answer_type;
    char *canonical_name;

    if (!PyArg_ParseTuple(args, "|O", &result_capsule))  {
        PyErr_SetString(PyExc_AttributeError, GETDNS_RETURN_INVALID_PARAMETER_TEXT);
        Py_DECREF(self);
        return -1;
    }
    if ((result_dict = PyCapsule_GetPointer(result_capsule, "result")) == NULL)  {
        PyErr_SetString(PyExc_AttributeError, "Unable to initialize result object");
        Py_DECREF(self);
        return -1;
    }
    if ((self->replies_full = getFullResponse(result_dict)) == NULL)  {
        Py_DECREF(self);
        return -1;
    }
    if ((self->replies_tree = get_replies_tree(result_dict)) == NULL)  {
        Py_DECREF(self);
        return -1;
    }
    if ((status = get_status(result_dict)) == 0)  {
        Py_DECREF(self);
        return -1;
    }
    self->status = PyInt_FromLong((long)status);
    if ((answer_type = get_answer_type(result_dict)) == 0)  {
        Py_DECREF(self);
        return -1;
    }
    self->answer_type = PyInt_FromLong((long)answer_type);
    if ((canonical_name = get_canonical_name(result_dict)) == 0)  
        self->canonical_name = Py_None;
    else
        self->canonical_name = PyString_FromString(canonical_name);
    if ((self->just_address_answers = get_just_address_answers(result_dict)) == NULL)  {
        self->just_address_answers = Py_None;
    }
    if ((self->validation_chain = get_validation_chain(result_dict)) == NULL)  
        self->validation_chain = Py_None;
    return 0;
}


PyObject *
result_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    getdns_ResultObject *self;

    self = (getdns_ResultObject *)type->tp_alloc(type, 0);
    if (self != NULL)  {
        self->just_address_answers = Py_None;
        self->answer_type = Py_None;
        self->status = Py_None;
        self->replies_tree = Py_None;
        self->canonical_name = Py_None;
        self->replies_full = Py_None;
        self->validation_chain = Py_None;
    }
    return (PyObject *)self;
}





void
result_dealloc(getdns_ResultObject *self)
{
    Py_XDECREF(self->just_address_answers);
    Py_XDECREF(self->answer_type);
    Py_XDECREF(self->status);
    Py_XDECREF(self->replies_tree);
    Py_XDECREF(self->replies_full);
    Py_XDECREF(self->canonical_name);
    self->ob_type->tp_free((PyObject *)self);
}


PyObject *
result_getattro(PyObject *self, PyObject *nameobj)
{
    Py_RETURN_NONE;
}


PyObject *
result_str(PyObject *self)
{
    getdns_ResultObject *me = (getdns_ResultObject *)self;
    PyObject *cname;

    cname = me->canonical_name;
    Py_INCREF(cname);
    return cname;
}
    


/*
 * package up a getdns response dict and use it to
 * build a new Python result object
 */

PyObject *
result_create(struct getdns_dict *resp)
{
    PyObject *result_capsule;
    PyObject *args;

    result_capsule = PyCapsule_New(resp, "result", 0);
    args = Py_BuildValue("(O)", result_capsule);
    return PyObject_CallObject((PyObject *)&getdns_ResultType, args);
}
