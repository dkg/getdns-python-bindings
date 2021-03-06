#include <Python.h>
#include <getdns/getdns.h>
#include <arpa/inet.h>
#include "pygetdns.h"


/*
 *  Get the address of the Python function object
 *    being passed in by name to the context
 *    query methods
 */

PyObject *
get_callback(char *py_main, char *callback)
{
    PyObject *main_module;
    PyObject *main_dict;
    PyObject *callback_func;

    if ((main_module = PyImport_AddModule(py_main)) == 0)  {
        PyErr_SetString(getdns_error, "No 'main'");
        return NULL;
    }
    main_dict = PyModule_GetDict(main_module);
    if ((callback_func = PyDict_GetItemString(main_dict, callback)) == 0)  {
        PyErr_SetString(getdns_error, "callback not found\n");
        return NULL;
    }
    if (!PyCallable_Check(callback_func))  {
        PyErr_SetString(getdns_error, "The callback function is not runnable");
        return NULL;
    }
    return callback_func;
}

    
void
callback_shim(struct getdns_context *context,
              getdns_callback_type_t type,
              struct getdns_dict *response,
              void *userarg,
              getdns_transaction_t tid)
{
    PyObject *py_callback_type;
    PyObject *py_result;
    PyObject *py_tid;
    PyObject *py_userarg;

    userarg_blob *u = (userarg_blob *)userarg;
    if ((py_callback_type = PyInt_FromLong((long)type)) == NULL)  {
        PyObject *err_type, *err_value, *err_traceback;
        PyErr_Fetch(&err_type, &err_value, &err_traceback);
        PyErr_Restore(err_type, err_value, err_traceback);
        return;
    }
    if (type == GETDNS_CALLBACK_CANCEL)  {
        py_result = Py_None;
        py_tid = Py_None;
        py_userarg = Py_None;
    }  else  {
        py_result = result_create(response);
        py_tid = PyInt_FromLong((long)tid);
        if (u->userarg)
            py_userarg = PyString_FromString(u->userarg);
        else
            py_userarg = Py_None;
    }
    PyObject_CallFunctionObjArgs(u->callback_func, py_callback_type, py_result, py_userarg, py_tid, NULL);
}
