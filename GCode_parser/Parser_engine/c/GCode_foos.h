
#ifndef _GCODE_FOOS_
#define _GCODE_FOOS_

#define RES_STR_MEM_STEP 0x100
//#define RES_STR_TUPLE_STEP 0x1000

extern "C" {
void 
_int_create_parser();

PyObject* //const wchar_t* 
_int_do_parse(char* gcode, int len);

PyObject *
_int_set_cb_dict(PyObject *self, PyObject *args);

}

extern int out_type;

extern wchar_t* result;
extern int res_len;

#endif
