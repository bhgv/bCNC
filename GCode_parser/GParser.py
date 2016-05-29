
import re

import CNC

from Parser_engine.Scanner import Scanner
from Parser_engine.Parser  import Parser, Errors


class GParser:
    CALLBACK_NAMES = ("eol", "init", "fini", "pragma", "default", "set_param", "get_param",
                      "aux_cmd", "no_callback")
    def __init__(self):
        self.gparser = Parser()
        #self.ln_tmp = ""

    def init_parser(self):
        self.gparser.init()
    
    def do_parse(self, text):
        self.self_caller.ln_tmp = ""
        self.gparser.Parse(Scanner(text + "\n"))
        
        if Errors.count == 0:
            lst = self.gparser.get_gcode_out() # get preprocessed internal gcode list
            out = ""
            for ln in lst:
                out += ln + "\n"
            return out
        #else:
        #    if_errors_present_do_something_foo() # do something if errors present

    def set_gcode_name(self, fname):
        Errors.Init(                         # initialise Errors handler
            fname,               # gcode name used in error messages
            "./",                     # directory name used to store log file
            False,
            self.gparser.getParsingPos,     # callback to get the position of error
            self.gparser.errorMessages      # error messages texts list
        )

        
    def set_callbacks(self, self_caller, cb_dict={}):
        self.self_caller = self_caller

        self.gparser.set_callback_dict(      # set callback-foos for executing different g-codes and situations
             cb_dict
        )

    def set_callbacks_compiler(self, self_caller):
        #@staticmethod
        def gcmd_cb(self, cmd, par=""):
            if cmd not in GParser.CALLBACK_NAMES:
                line = " " + cmd + par
                self.ln_tmp += line
                return line

        #@staticmethod
        def pragma_cb(self, cmd, pragma):
            #print pragma
            #self.ln_tmp += pragma
            pat = re.match(
                    r"\(\s*(Block-name:|polygon)\s*([A-Za-z_\-01-9]*)\s*\)", 
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
        
        self.set_callbacks(self_caller, 
            {
                "pragma": pragma_cb,
                "default": gcmd_cb,
                "eol": eol_cb,
                "aux_cmd": aux_cmd_cb,
                "self": self_caller,
            }
        )
    
    def set_callbacks_draw_gcode(self, self_caller):
        def init_cb(self, cmd):
            self.tmp_cmd_buf = []
        
        def gcmd_cb(self, cmd, par=""):
            #self.ln_tmp += cmd + par
            if cmd not in GParser.CALLBACK_NAMES:
                line = cmd + par
                self.tmp_cmd_buf.append(line)
                return line
            else:
                return ""
                
        self.set_callbacks(self_caller, 
            {
                "init": init_cb,
                "default": gcmd_cb,
                "self": self_caller,
            }
        )
    
    def get_cmds_draw(self, self_caller):
        return self_caller.tmp_cmd_buf
    
    
    def set_callbacks_starup_gcode(self, self_caller, add_cb):
        def gcmd_cb(add_cb, cmd, par=""):
            #self.ln_tmp += cmd + par
            if cmd not in GParser.CALLBACK_NAMES:
                line = cmd + par
                add_cb(line, None)
                return line
            else:
                return ""
        
        self.set_callbacks(self_caller, 
            {
                "default": gcmd_cb,
                "self": add_cb,
            }
        )
