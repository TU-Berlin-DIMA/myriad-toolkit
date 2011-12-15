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
import os


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
        astRoot = reader.read()
        
        astPrinter = PrintVisitor()
        astPrinter.traverse(astRoot)