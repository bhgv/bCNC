
#include <wchar.h>

#include <stdio.h>

#include <Python.h>

#include "Scanner.h"
#include "Parser.h"

#include "uthash/include/uthash.h"

#include "GCode_foos.h"



typedef struct {
    int id;                    /* key */
    wchar_t* param;
    UT_hash_handle hh;         /* makes this structure hashable */
} param_hash_item;

param_hash_item *params_ht = NULL;    /* important! initialize to NULL */


static struct {
    PyObject *cmd, *eol, *init, *fini, *pragma, *def, *set_param, *get_param, *aux_cmd, *no_callback, *self, *O;
} py_callbacks;

const wchar_t* cb_types_strings[] = {
    (const wchar_t*)NULL,
    /*Parser::CMD         =*/ L"cmd",
    /*Parser::EOL         =*/ L"eol",
    /*Parser::INIT        =*/ L"init",
    /*Parser::FINI        =*/ L"fini",
    /*Parser::PRAGMA      =*/ L"pragma",
    /*Parser::AUX_CMD     =*/ L"aux_cmd",
    /*Parser::DEFAULT     =*/ L"default",
    /*Parser::NO_CMD_CB   =*/ L"no_callback",
    (const wchar_t*)NULL
};

wchar_t* _int_cb_call(PyObject *foo, Parser::Cb_Type key, wchar_t *param1, wchar_t *param2);


Parser* gparser = NULL;

PyObject* res_tuple =NULL;
PyObject* res_tuple_itm = NULL;

wchar_t* result = NULL;
wchar_t* res_p = NULL;

int res_len = 0;
int res_len_max = 0;


int out_type = 0;


/*
 int callExecFunction(const char* evalStr)
{
    PyCodeObject* code = (PyCodeObject*)Py_CompileString(evalStr, "pyscript", Py_eval_input);
    PyObject* global_dict = PyModule_GetDict(pModule);
    PyObject* local_dict = PyDict_New();
    PyObject* obj = PyEval_EvalCode(code, global_dict, local_dict);

    PyObject* result = PyObject_Str(obj);
    PyObject_Print(result, stdout, 0);
}
*/


/*
void eval(wchar_t* wcs){
    char* s = coco_string_create_char(wcs);
    
    printf("aux -> %ls\n%s\n", wcs, &s[1]);
    
    PyCodeObject* code = (PyCodeObject*) Py_CompileString(s, "aux", Py_eval_input);
    PyObject* main_module = PyImport_AddModule("__main__");
    PyObject* global_dict = PyModule_GetDict(main_module);
    PyObject* local_dict = PyDict_New();
//    PyObject* obj = PyEval_EvalCode(code, global_dict, local_dict);
    PyObject* obj = PyEval_EvalCode(code, local_dict, local_dict);

//    PyObject* result = PyObject_Str(obj);
//    PyObject_Print(result, stdout, 0);
}
*/


  
  void Parser::set_param(wchar_t* key, wchar_t* param){
    param_hash_item *s = NULL, *so;
    
    s = new param_hash_item();
    s->id = coco_string_hash(key);
    s->param = param;
    
    coco_string_delete(key);
    
    HASH_ADD_INT( params_ht, id, s );  /* id: name of key field */
  }
  
  
  wchar_t* Parser::get_param(wchar_t* key){
    param_hash_item *s = NULL;
    wchar_t* out;
    int id = coco_string_hash(key);
    
    HASH_FIND_INT( params_ht, &id, s );  /* id: name of key field */
    
    if(s == NULL) 
        out = coco_string_create(L"");
    else 
        out = coco_string_create(s->param);
    
    return out;
  }
  
  

static wchar_t* _int_scpy(wchar_t* s){
      int len;
      if(s == NULL) return 0;
      
      len = wcslen(s);
      if(len == 0) return 0;
      
/**/
      if(res_len + len + 1 >= res_len_max){
          wchar_t* res_o = result;
          result = new wchar_t[res_len_max + RES_STR_MEM_STEP];
          res_len_max += RES_STR_MEM_STEP;
          
          if(res_len > 0){
              wcsncpy(result, res_o, res_len);
          }

          res_p = result + res_len;
          delete[] res_o;
      }
/**/
      wcsncpy(res_p, s, len);
      res_len += len;
      res_p = result + res_len;
      *res_p = (wchar_t)0;
      return res_p;
  }
  
  void _int_out_tuple_append_last(){
    PyObject* ostr;
    
    ostr = PyUnicode_FromUnicode(result, res_len);

    if(out_type == 0)
        PyList_Append(res_tuple, ostr);
    else
        PyList_Append(res_tuple_itm, ostr);

    Py_DECREF(ostr);

    res_p = result;
    *result = (wchar_t)0;
    res_len = 0;
  }
  
  void Parser::call(Cb_Type key, wchar_t* param1, wchar_t* param2){
      switch(key){
          case Parser::CMD:
            _int_cb_call(py_callbacks.cmd, Parser::CMD, param1, param2);
            if(out_type == 0)
                _int_scpy((wchar_t*)L" ");
            _int_scpy(param1);
            if(param2 != NULL){ 
                _int_scpy(param2);
            }
            if(out_type != 0)
                _int_out_tuple_append_last();
            break;

          case Parser::EOL:
            if(out_type == 0){
                _int_cb_call(py_callbacks.eol, Parser::EOL, 0, 0);
                _int_out_tuple_append_last();
            }else{
                _int_cb_call(py_callbacks.eol, Parser::EOL, 0, 0);
                PyList_Append(res_tuple, res_tuple_itm);
                Py_DECREF(res_tuple_itm);
                res_tuple_itm = PyList_New(0);
            }
            break;
            
          case Parser::AUX_CMD:
            _int_cb_call(py_callbacks.cmd, Parser::AUX_CMD, param1, param2);
            break;
            
          case Parser::INIT:
            _int_cb_call(py_callbacks.cmd, Parser::INIT, 0, 0);
            break;
            
          case Parser::FINI:
            _int_cb_call(py_callbacks.cmd, Parser::FINI, 0, 0);
            break;
            
/*            
          case Parser::CMD:
            break;
*/            
          default:
            return;
      }
      
      if(param1) coco_string_delete(param1);
      if(param2) coco_string_delete(param2);
  }
/*
   def call(self, key, param=None):
      if self._int_is_callback_defined(key):
         out = self._int_call(key, param)
         if out:
            self.gcode_out_last += str(out)
      elif self._int_is_callback_defined("default"):
         out = self._int_call("default", key, param)
         if out:
            self.gcode_out_last += str(out)
      elif self._int_is_callback_defined("no_callback"):
         self._int_call("no_callback", key, param, self.getParsingPos())
      else:
         if key == "eol":
            if self.out_type == 0:
               if len(self.gcode_out_last) > 0:
                  self.gcode_out.append(self.gcode_out_last)
                  self.gcode_out_last = ""
            else:
               if len(self.gcode_out_array_last) > 0:
                  self.gcode_out.append(self.gcode_out_array_last)
                  self.gcode_out_array_last = []
                  
         elif key and key not in self.callback_names:
            if self.out_type == 0:
               self.gcode_out_last += " " + key
               if param:
                  self.gcode_out_last += param
            else:
               line = key
               if param:
                  line += param
               self.gcode_out_array_last.append(line)
               
         elif key == "aux_cmd" and param:
            pat = self.other_param.match(param)
            if pat:
               var_name = pat.group(1)
               var_val_raw = pat.group(2)
               var_val = eval(var_val_raw)
               self.gcode_params[var_name] = str(var_val)
*/


wchar_t*
_int_cb_call(PyObject *foo, Parser::Cb_Type key, wchar_t *param1, wchar_t *param2){
    if(!foo) {
        foo = py_callbacks.def;
        if(!foo){
            foo = py_callbacks.no_callback;
            if(!foo)
                return NULL;
        }
    }
    if(key < 1 || key >= Parser::CB_LAST) return NULL;
    
    PyObject *arglist;
    PyObject *result;

    if(py_callbacks.self != NULL)
        arglist = Py_BuildValue("(Ouuu)", py_callbacks.self, cb_types_strings[key], param1, param2);
    else
        arglist = Py_BuildValue("(uuu)", cb_types_strings[key], param1, param2);
    result = PyObject_CallObject(foo, arglist);
    Py_DECREF(arglist);
}



PyObject *
_int_set_cb_dict_aux(PyObject *dict, const char *name, PyObject **cb_pptr, bool check_foo){
    PyObject *result = NULL;
    PyObject *cb, *cbo;
    
    cbo = *cb_pptr;
    
    cb = PyDict_GetItemString(dict, name);

    if (cb) {
        if (check_foo && !PyCallable_Check(cb)) {
            *cb_pptr = NULL;
            
//            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }
        Py_XINCREF(cb);      /* Add a reference to new callback */
        *cb_pptr = cb;       /* Remember new callback */
        if(cbo != NULL) Py_XDECREF(cbo);  /* Dispose of previous callback */
        
        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}


PyObject *
_int_set_cb_dict(PyObject *self, PyObject *args){
     PyObject *dict;
     if (!PyArg_ParseTuple(args, "O!", &PyDict_Type, &dict))
            return NULL;
    
    _int_set_cb_dict_aux(dict, "cmd", &py_callbacks.cmd, true);
    _int_set_cb_dict_aux(dict, "init", &py_callbacks.init, true);
    _int_set_cb_dict_aux(dict, "fini", &py_callbacks.fini, true);
    _int_set_cb_dict_aux(dict, "eol", &py_callbacks.eol, true);
    _int_set_cb_dict_aux(dict, "pragma", &py_callbacks.pragma, true);
    _int_set_cb_dict_aux(dict, "default", &py_callbacks.def, true);
    _int_set_cb_dict_aux(dict, "set_param", &py_callbacks.set_param, true);
    _int_set_cb_dict_aux(dict, "get_param", &py_callbacks.get_param, true);
    _int_set_cb_dict_aux(dict, "aux_cmd", &py_callbacks.aux_cmd, true);
    _int_set_cb_dict_aux(dict, "no_callback", &py_callbacks.no_callback, true);
    _int_set_cb_dict_aux(dict, "self", &py_callbacks.self, false);
    
    py_callbacks.O = self;

    /* Boilerplate to return "None" */
    Py_INCREF(Py_None);
   return Py_None;
}



void 
_int_create_parser(){
    gparser = new Parser(NULL);
    
    py_callbacks.O = 
    py_callbacks.eol = 
    py_callbacks.init = 
    py_callbacks.fini = 
    py_callbacks.pragma = 
    py_callbacks.def = 
    py_callbacks.set_param = 
    py_callbacks.get_param = 
    py_callbacks.aux_cmd = 
    py_callbacks.no_callback = NULL;
    
    res_len = 0;
    res_len_max = RES_STR_MEM_STEP; 
    result = new wchar_t[res_len_max + 1];
    res_p = result;
}


PyObject* 
_int_do_parse(char* gcode, int len){
    Scanner* sc, *sc_o;
    
    if(gparser == NULL || gcode == NULL) 
        return NULL;

    sc_o = gparser->scanner;
    sc = new Scanner((const unsigned char*)gcode, len);
    gparser->scanner = sc;

    if(sc_o) delete sc_o;
    
    res_p = result;
    res_len = 0;
    
    if(res_tuple) Py_DECREF(res_tuple);
    
    res_tuple = PyList_New(0);
    res_tuple_itm = PyList_New(0);

    gparser->Parse();
    
    Py_DECREF(res_tuple_itm);
    res_tuple_itm = NULL;

    return res_tuple;
}
