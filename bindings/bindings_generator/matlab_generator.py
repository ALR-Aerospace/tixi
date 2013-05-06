# -*- coding: utf-8 -*-
"""
Created on Sat May  4 19:28:45 2013

@author: Martin Sigel <martin.siggel@dlr.de>
"""

import os
from datetime import date

class MatlabGenerator(object):
    
    def __init__(self, cparser, prefix, includefile):
        self.cparser = cparser
        self.prefix  = prefix
        self.libinclude = includefile
        self.mex_body = '(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])'
        
    def create_m_file(self, fun_dec):
        '''creates an matlab .m file for the function fun_dec'''
        
        name = fun_dec.method_name
        filename = name + '.m'
        fop = open(filename, 'w')
        
        string = 'function '
        # get number of outargs
        outargs = [arg for arg in fun_dec.arguments if arg.is_outarg]
        inargs = [arg for arg in fun_dec.arguments if not arg.is_outarg]
        outstr = ''
        if len(outargs) > 1:
            outstr += '['
            for arg in outargs:
                outstr += arg.name + ', '
            outstr = outstr[0:-2] + '] = '
        elif len(outargs) == 1:
            outstr += outargs[0].name + ' = '
            
        string += outstr + fun_dec.method_name + '('
        
        instr = ''
        for arg in inargs:
            instr += arg.name + ', '
        if len(inargs) > 0:
            instr = instr[0:-2]
        
        string += instr + ')\n'
        
        indent = ' '*4
        string += indent \
                + '%% this file was automatically generated from %s on %s\n\n'\
                % (self.libinclude, date.today())
        
        # make some checks
        for arg in inargs:
            myfun = 'not(ischar' if arg.is_string else '(ischar'
            string += indent + 'if %s(%s))\n' % (myfun, arg.name)
            string += 2*indent + 'error(\'Invalid type of argument "%s"\');\n'\
                    % arg.name
            string += indent + 'end\n\n'
                
        # call the mex interface
        string += indent + 'try\n'
        string += 2*indent + outstr + self.prefix + '_matlab(\'%s\', %s);\n' \
                % (name, instr)
        indent = ' '*4
        string += indent + 'catch err\n'
        string += 2*indent + 'error(err.message)\n'
        string += indent + 'end\n'
        string += 'end\n\n'
        
        fop.write(string)
        fop.close()
        
    def create_mex_function(self, func):
        string = 'void mex_%s %s {\n' % (func.method_name, self.mex_body)
        #declare variables
        inargs = [arg for arg in func.arguments if not arg.is_outarg]
        if len(inargs) > 0:
            string += 4*' ' + '/* input arguments */\n'
            for arg in inargs:
                string += 4*' ' + arg.rawtype + '*'*arg.npointer + ' ' + arg.name
                if arg.npointer > 0:
                    string += ' = NULL'
                string += ';\n'
            string += '\n'
            
        outargs = [arg for arg in func.arguments if arg.is_outarg]
        if len(outargs) > 0:
            string += 4*' ' + '/* output variables */\n'
            for arg in outargs:
                assert(arg.npointer > 0)
                string += 4*' ' + arg.rawtype + '*'*(arg.npointer-1) + ' ' + arg.name
                if arg.npointer > 1:
                    string += ' = NULL'
                string += ';\n'
            string += '\n'
            
        if func.return_value.rawtype != 'void':
            string += 4*' '  + func.return_value.rawtype \
                + '*'*func.return_value.npointer + ' ' + func.return_value.name
            if func.return_value.npointer > 0:
                string += ' = NULL'
            string += ';\n\n'
        
        #check correct number in inargs
        string += 4*' ' + 'if(nrhs != %d) {\n' % (len(inargs) + 1)

        pseudocall = func.method_name + '('
        for arg in inargs:
            pseudocall += arg.name + ', '
        if len(inargs) > 0:
            pseudocall = pseudocall[0:-2]
        pseudocall += ')'
        
        string += 4*' ' + '    mexErrMsgTxt("%s: Wrong number of arguments\\n");\n' % pseudocall
        string += 4*' ' + '}\n'
        
        # function call
        string += 4*' '
        if func.return_value.rawtype != 'void':
            string +=  func.return_value.name + ' = '
                
        string += func.method_name + '();\n'
            
        string += '\n'
        
        string += '}\n\n'
        return string

    def create_mex_dispatcher(self):
        '''crates the main entry point for the mex file'''        
        
        indent = 4*' '   
        string = '''
#ifdef __cplusplus
extern "C" {
#endif


'''
        string += '/** main entry point of MATLAB, deals as a function dispatcher */\n'
        string += 'void mexFunction%s{\n' % self.mex_body
        
        string += '''
    char * functionName = NULL;
    if(nrhs < 1){
        mexErrMsgTxt("No function argument given!");
        return;
    }
    
    mxToString(prhs[0],&functionName);

''' 
        call = ''
        for index, func in enumerate(self.cparser.declarations):
            if index > 0:
                call += 'else '
            call += 'if(strcmp(functionName, "%s") == 0) {\n' % func.method_name
            call += '    mex_%s(nlhs, plhs, nrhs, prhs);\n' % func.method_name
            call += '}\n'
        
        call += '''else {
    // this is only executed if the function could not be identified
    char text[255];
'''
        call += '    snprintf(text, 250, "%%s is not a valid %s function!\\n",functionName);\n' % self.prefix
        call += '''    mxFree(functionName);
    mexErrMsgTxt(text);
}
mxFree(functionName);
''' 
        
        for line in call.splitlines():
            string += indent + line + '\n'
            
        string += '}\n'
        string += '''
#ifdef __cplusplus
} /* extern C */
#endif

'''   
        return string
        
        
    def create_cmex_file(self):
        '''creates the .c file, which then is compiled to the matlab mex library'''        
        
        string = '/* This file was automatically generated from %s on %s */\n\n' % (self.libinclude, date.today())        
        
        incpath = os.path.dirname(os.path.realpath(__file__)) + '/matlab_cheader.in'
        
        #read header
        fop = open(incpath, 'r')
        string += fop.read() + '\n\n'
        fop.close()
        
        string += '#include <%s>\n\n' % self.libinclude
        
        for function in self.cparser.declarations:
            string += self.create_mex_function(function)
        
        string += self.create_mex_dispatcher()
        fop = open('tiximatlab.c', 'w')
        fop.write(string)
        fop.close()
        
    def create_wrapper(self):
        for fun_dec in self.cparser.declarations:
            self.create_m_file(fun_dec)
            
        self.create_cmex_file()
            
            