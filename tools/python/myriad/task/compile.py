'''
Copyright 2010-2011 DIMA Research Group, TU Berlin

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 
Created on Oct 14, 2011

@author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
'''

from myriad.compiler.debug import PrintVisitor
from myriad.compiler.reader import XMLReader
from myriad.task.common import AbstractTask
import os, re


TASK_PREFIX = "compile"

class CompileModelTask(AbstractTask):
    '''
    classdocs
    '''

    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        kwargs.update(group=TASK_PREFIX, name="model", description="Compile generator extensions from XML model specification.")

        super(CompileModelTask, self).__init__(*args, **kwargs)
    
    def argsParser(self):
        parser = super(CompileModelTask, self).argsParser()
        
        # arguments
        parser.add_option("--input-spec", metavar="SPECIFICATION", dest="model_spec_path", type="str", 
                            default=None, help="path to the model specification XML file")

        return parser
        
    def _fixArgs(self, args):
        super(CompileModelTask, self)._fixArgs(args)
        
        if (args.model_spec_path == None):
            args.model_spec_path = "%s-specification.xml" % (args.dgen_name)
                    
        if (not os.path.isabs(args.model_spec_path)):
            args.model_spec_path = "%s/../../specification/%s" % (args.base_path, args.model_spec_path)
            
        args.model_spec_path = os.path.realpath(args.model_spec_path)
        
    def _do(self, args):
        # reed the AST
        reader = XMLReader(args)
        ast = reader.read()
        
        astPrinter = PrintVisitor()
        astPrinter.traverse(ast)
        
        # compile record types
        recordCompiler = RecordTypeCompiler(args=args)
        recordCompiler.compileAll(ast.getSpecification().getRecordSequences())

class FileCompiler(object):
    '''
    classdocs
    '''
    
    BUFFER_SIZE = 512
    
    _args = None
    _srcPath = None
    _cc2us_pattern1 = None
    _cc2us_pattern2 = None
    
    def __init__(self, args):
        '''
        Constructor
        '''
        self._cc2us_pattern1 = re.compile('(.)([A-Z][a-z]+)')
        self._cc2us_pattern2 = re.compile('([a-z0-9])([A-Z])')
        
        self._args = args
        self._srcPath = "%s/../../src/cpp" % (args.base_path)
        
    def _uc(self, s):
        return s.upper()

    def _lc(self, s):
        return s.lower()

    def _cc2us(self, s):
        return self._cc2us_pattern2.sub(r'\1_\2', self._cc2us_pattern1.sub(r'\1_\2', s)).lower()

    def _us2cc(self, s):
        def camelcase(): 
            yield str.lower
            while True:
                yield str.capitalize
    
        c = camelcase()
        return "".join(c.next()(x) if x else '_' for x in s.split("_"))
    
    def _ucFirst(self, s):
        return "%s%s" % (s[0].capitalize(), s[1:])

        
class RecordTypeCompiler(FileCompiler):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        super(RecordTypeCompiler, self).__init__(*args, **kwargs)
        
    def compileAll(self, recordSequences):
        for v in recordSequences.getAll().itervalues():
            self.compileRecordType(v.getRecordType())
            
    def compileRecordType(self, recordType):
    
        try:
            os.makedirs("%s/record/base" % (self._srcPath))
        except OSError:
            pass
        
        typeNameUS = recordType.getAttribute("key")
        typeNameCC = self._ucFirst(self._us2cc(typeNameUS))
        typeNameUC = self._uc(typeNameCC)
        
        wfile = open("%s/record/base/Base%s.h" % (self._srcPath, typeNameCC), "w", RecordTypeCompiler.BUFFER_SIZE)
        
        print >> wfile, '// auto-generatad base C++ type for `%s`' % (recordType.getAttribute("key"))
        print >> wfile, ''
        print >> wfile, '#ifndef BASE%s_H_' % (typeNameUC)
        print >> wfile, '#define BASE%s_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#include "record/Record.h'
        print >> wfile, ''
        print >> wfile, 'using namespace Myriad;'
        print >> wfile, ''
        print >> wfile, 'namespace %s {' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '// forward declarations'
        print >> wfile, 'class %s;' % (typeNameCC)
        print >> wfile, 'class %sConfig;' % (typeNameCC)
        print >> wfile, 'class %sGenerator;' % (typeNameCC)
        print >> wfile, 'class %sHydratorChain;' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, 'class Base%s: public Record' % (typeNameCC)
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        print >> wfile, 'private:'
        print >> wfile, ''
        print >> wfile, '}'
        print >> wfile, ''
        print >> wfile, '} // namespace %s' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, "#endif /* BASE%s_H_ */" % (typeNameUC)

        wfile.close()
        