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
from myriad.compiler.source import *
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
        kwargs.update(group=TASK_PREFIX, name="prototype", description="Compile generator extensions from XML prototype specification.")

        super(CompileModelTask, self).__init__(*args, **kwargs)
    
    def argsParser(self):
        parser = super(CompileModelTask, self).argsParser()
        
        # arguments
        parser.add_option("--input-spec", metavar="PROTOTYPE", dest="prototype_path", type="str", 
                            default=None, help="path to the prototype specification XML file")

        return parser
        
    def _fixArgs(self, args):
        super(CompileModelTask, self)._fixArgs(args)
        
        if (args.prototype_path == None):
            args.prototype_path = "%s-prototype.xml" % (args.dgen_name)
                    
        if (not os.path.isabs(args.prototype_path)):
            args.prototype_path = "%s/../../src/config/%s" % (args.base_path, args.prototype_path)
            
        args.prototype_path = os.path.realpath(args.prototype_path)
        
    def _do(self, args):
        # reed the AST
        reader = XMLReader(args)
        ast = reader.read()
        
#        astPrinter = PrintVisitor()
#        astPrinter.traverse(ast)

        # compile output collector
        frontendCompiler = FrontendCompiler(args=args)
        frontendCompiler.compile(ast)
        # compile generator config
        generatorSubsystemCompiler = GeneratorSubsystemCompiler(args=args)
        generatorSubsystemCompiler.compile(ast)
        # compile generator config
        configCompiler = ConfigCompiler(args=args)
        configCompiler.compile(ast)
        # compile output collector
        outputCollectorCompiler = OutputCollectorCompiler(args=args)
        outputCollectorCompiler.compile(ast)
        # compile enum types
        enumCompiler = EnumTypesCompiler(args=args)
        enumCompiler.compile(ast.getSpecification().getEnumSets())
        # compile record types
        recordCompiler = RecordTypeCompiler(args=args)
        recordCompiler.compile(ast.getSpecification().getRecordSequences())
        # compile record generators
        generatorCompiler = RecordGeneratorCompiler(args=args)
        generatorCompiler.compile(ast.getSpecification().getRecordSequences())