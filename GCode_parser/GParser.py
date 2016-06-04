
import re

import CNC

from conf.parser import py_parser

if py_parser:
    from Parser_engine.py.Scanner import Scanner
    from Parser_engine.py.Parser  import Parser, Errors
else:
#if not py_parser:
    from Parser_engine.c import GCode



PRAGMA_BLOCK = re.compile(r"\(\s*(Block-name:|polygon)\s*([A-Za-z_\-01-9]*)\s*\)")

class GParser:
    CALLBACK_NAMES = ("eol", "init", "fini", "pragma", "default", "set_param", "get_param",
                      "aux_cmd", "no_callback")
    def __init__(self):
        if py_parser:
            self.gparser = Parser()
        #else:
        #    self.gparser = GCode

    def init_parser(self):
        if py_parser:
            self.gparser.init()
        
    
    def clear_output():
        if py_parser:
            self.gparser.clear_out()
    
    def do_parse(self, text):
        if py_parser:
            self.gparser.Parse(Scanner( text + "\n" ))
            if Errors.count == 0:
                lst = self.gparser.get_gcode_out() # get preprocessed internal gcode list
            else:
                lst = []
        else:
        #if not py_parser:
            lst = GCode.do_parse(text + "\n")
        
        return lst 
        

    def set_gcode_name(self, fname):
        if py_parser:
            Errors.Init(                         # initialise Errors handler
                fname,               # gcode name used in error messages
                "./",                     # directory name used to store log file
                False,
                self.gparser.getParsingPos,     # callback to get the position of error
                self.gparser.errorMessages      # error messages texts list
            )

        
    def set_callbacks(self, self_caller, cb_dict={}):
        self.self_caller = self_caller

        if py_parser:
            self.gparser.set_callback_dict(      # set callback-foos for executing different g-codes and situations
                cb_dict
            )
        else:
        #if not py_parser:
            GCode.set_callback_dict(      # set callback-foos for executing different g-codes and situations
                cb_dict
            )

    def set_callbacks_compiler(self, self_caller):
        if py_parser:
            self.gparser.set_out_type_by_line()
            self.gparser.init()
        else:
        #if not py_parser:
            GCode.set_out_type_by_line()
        
        #@staticmethod
        def gcmd_cb(self, cmd, par=""):
            if cmd not in GParser.CALLBACK_NAMES:
                line = " " + cmd + par
                self.ln_tmp += line
                return line

        #@staticmethod
        def pragma_cb(self, cmd, pragma):
            #print pragma
            pat = PRAGMA_BLOCK.match(
            #       re.match(
            #        r"\(\s*(Block-name:|polygon)\s*([A-Za-z_\-01-9]*)\s*\)", 
                    pragma
            )
            if pat:
                value = pat.group(2)
                if not self.blocks or len(self.blocks[-1]):
                    self.blocks.append(CNC.Block(value))
                else:
                    self.blocks[-1]._name = value
        
        #@staticmethod
        def eol_cb(self, cmd):
            line = self.ln_tmp
            self.ln_tmp = ""
            
            if not self.blocks:
                self.blocks.append(CNC.Block("Header"))
            #print "%s" % line
            if len(line) > 0:
                self.cnc.motionStart([line])
                self.blocks[-1].append(line)
                self.cnc.motionEnd()
                return "\n"
            else:
                return ""
        
        #@staticmethod
        def aux_cmd_cb(self, cmd):
            print "aux_cmd = " + cmd
        
        #self.set_callbacks(self_caller, 
        #    {
        #        #"pragma": pragma_cb,
        #        #"default": gcmd_cb,
        #        #"eol": eol_cb,
        #        #"aux_cmd": aux_cmd_cb,
        #        "self": self_caller,
        #    }
        #)
    
    def set_callbacks_draw_gcode(self, self_caller):
        if py_parser:
            self.gparser.set_out_type_by_cmd()
            self.gparser.clear_out()
        else:
        #if not py_parser:
            GCode.set_out_type_by_cmd()
        
        def init_cb(self, cmd):
            self.tmp_cmd_buf = []
        
        def gcmd_cb(self, cmd, par=""):
            if cmd not in GParser.CALLBACK_NAMES:
                line = cmd + par
                self.tmp_cmd_buf.append(line)
                return line
            else:
                return ""
                
        #self.set_callbacks(self_caller, 
        #    {
        #        #"init": init_cb,
        #        #"default": gcmd_cb,
        #       "self": self_caller,
        #    }
        #)
    
#    def get_cmds_draw(self, self_caller):
#        return self_caller.tmp_cmd_buf
    
    
    def set_callbacks_starup_gcode(self, self_caller, add_cb):
        if py_parser:
            self.gparser.set_out_type_by_cmd()
            self.gparser.clear_out()
        else:
        #if not py_parser:
            GCode.set_out_type_by_cmd()
        
        def gcmd_cb(add_cb, cmd, par=""):
            if cmd not in GParser.CALLBACK_NAMES:
                line = cmd + par
                add_cb(line, None)
                return line
            else:
                return ""
        
        #self.set_callbacks(self_caller, 
        #    {
#                "default": gcmd_cb,
        #        "self": add_cb,
        #    }
        #)

