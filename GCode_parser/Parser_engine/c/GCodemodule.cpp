
#include <Python.h>

#include <stdio.h>

#define GCODE_MODULE
#include "GCodemodule.h"

#include "GCode_foos.h"


static PyObject *
PyGCode_do_parse(PyObject *self, PyObject *args)
{
//    wchar_t *gcode;
    char *gcode; 
    PyObject* out;
    Py_ssize_t len;

//    if (!PyArg_ParseTuple(args, "u#", &gcode, &len))
    if (!PyArg_ParseTuple(args, "s#", &gcode, &len))
        return NULL;
    
    out = _int_do_parse(gcode, (int) len);
    
    return out; 
}


static PyObject *
PyGCode_set_o_by_ln(PyObject *self, PyObject *args){
    out_type = 0;
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
PyGCode_set_o_by_cmd(PyObject *self, PyObject *args){
    out_type = 1;
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
PyGCode_set_callback_dict(PyObject *self, PyObject *args)
{
    return _int_set_cb_dict(self, args);
}



static PyMethodDef GCodeMethods[] = {
    {"set_out_type_by_line",  PyGCode_set_o_by_ln, METH_VARARGS, "test GCode module."},
    {"set_out_type_by_cmd",  PyGCode_set_o_by_cmd, METH_VARARGS, "test GCode module."},
    {"do_parse",  PyGCode_do_parse, METH_VARARGS, "do parse gcode."},
    {"set_callback_dict",  PyGCode_set_callback_dict, METH_VARARGS, "set callbacks on parsing gcode."},
    
    {NULL, NULL, 0, NULL}        /* Sentinel */
};



PyMODINIT_FUNC
initGCode(void)
{
    PyObject *m;
    m = Py_InitModule("GCode", GCodeMethods);
    if (m == NULL)
        return;
        
    _int_create_parser();
}
