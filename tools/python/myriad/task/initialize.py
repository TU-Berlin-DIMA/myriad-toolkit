'''
Copyright 2010-2013 DIMA Research Group, TU Berlin

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

import myriad.task.common

TASK_PREFIX = "initialize"

class InitializeProjectTask(myriad.task.common.AbstractTask):
    '''
    classdocs
    '''

    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        kwargs.update(group=TASK_PREFIX, name="project", description="Bootstrap a new project.")
        super(InitializeProjectTask, self).__init__(*args, **kwargs)
    
    def argsParser(self):
        parser = super(InitializeProjectTask, self).argsParser()
        
        # arguments
        parser.add_argument("--name", metavar="NAME", dest="dgen_name", type="str",
                            help="name of the new project")
        # options
        parser.add_option("--ns", metavar="NS", dest="dgen_ns", type="str",
                          default=None, help="namespace for the C++ generator extensions")
        parser.add_option("--oligos-cp", metavar="CP", dest="oligos_cp", type="str",
                          default=None, help="extra class path entries to be used by the `compile:oligos` task")

        return parser
        
    def _fixArgs(self, args):
        super(InitializeProjectTask, self)._fixArgs(args)
        
        if args.dgen_ns is None:
            args.dgen_ns = "__%s" % (args.dgen_name)
        
        if args.oligos_cp is None:
            args.oligos_cp = ""
            
    def _requiresMyriadSettings(self):
        return False

    def _do(self, args):
        skeletonBase = "%s/tools/skeleton/task/%s/%s" % (args.base_path, self.group(), self.name())
        projectBase = "%s/../.." % (args.base_path)

        self._log.info("Processing skeleton for task.")
        skeletonProcessor = myriad.task.common.SkeletonProcessor(skeletonBase)
        skeletonProcessor.process(projectBase, args)


class InitializePrototypeTask(myriad.task.common.AbstractTask):
    '''
    classdocs
    '''

    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        kwargs.update(group=TASK_PREFIX, name="prototype", description="Initialize an XML prototype for this project.")
        super(InitializePrototypeTask, self).__init__(*args, **kwargs)
    
    def argsParser(self):
        parser = super(InitializePrototypeTask, self).argsParser()
        
        # arguments
        parser.add_argument("--name", metavar="NAME", dest="prototype_name", type="str",
                            help="name of the prototype example to initialize (currently only `customer` is supported)")

        return parser
        
    def _fixArgs(self, args):
        super(InitializePrototypeTask, self)._fixArgs(args)
        
        if args.prototype_name not in ['customer']:
            args.prototype_name = 'customer'

    def _do(self, args):
        skeletonBase = "%s/tools/skeleton/task/%s/%s/%s" % (args.base_path, self.group(), self.name(), args.prototype_name)
        projectBase = "%s/../.." % (args.base_path)

        self._log.info("Processing skeleton for task.")
        skeletonProcessor = myriad.task.common.SkeletonProcessor(skeletonBase)
        skeletonProcessor.process(projectBase, args)


class InitializeRecordTask(myriad.task.common.AbstractTask):
    '''
    classdocs
    '''

    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        kwargs.update(group=TASK_PREFIX, name="record", description="Generate extensions for a new record.")
        super(InitializeRecordTask, self).__init__(*args, **kwargs)
    
    def argsParser(self):
        parser = super(InitializeRecordTask, self).argsParser()
        
        # arguments
        parser.add_argument("--record_name", metavar="NAME", dest="record_name", type="str",
                            help="name of the new record (CamelCase)")
        # options
        parser.add_option("--dgen_name", metavar="NAME", dest="dgen_name", type="str",
                          default=None, help="name of the new project")
        parser.add_option("--ns", metavar="NS", dest="dgen_ns", type="str",
                          default=None, help="namespace for the C++ generator extensions")

        return parser
        
    def _fixArgs(self, args):
        super(InitializeRecordTask, self)._fixArgs(args)
        
        if args.dgen_ns == None:
            args.dgen_ns = "__%s" % (args.dgen_name)

    def _do(self, args):
        skeletonBase = "%s/tools/skeleton/task/%s/%s" % (args.base_path, self.group(), self.name())
        targetBase = "%s/../.." % (args.base_path)

        self._log.info("Processing skeleton for task.")
        skeletonProcessor = myriad.task.common.SkeletonProcessor(skeletonBase)
        skeletonProcessor.process(targetBase, args, myriad.task.common.SkeletonProcessor.PROCESS_FILES)


class InitializeGeneratorTask(myriad.task.common.AbstractTask):
    '''
    classdocs
    '''

    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        kwargs.update(group=TASK_PREFIX, name="generator", description="Generate extensions for a new generator.")
        super(InitializeGeneratorTask, self).__init__(*args, **kwargs)
    
    def argsParser(self):
        parser = super(InitializeGeneratorTask, self).argsParser()
        
        # arguments
        parser.add_argument("--record_name", metavar="NAME", dest="record_name", type="str",
                            help="name of the record for the new generator (CamelCase)")
        # options
        parser.add_option("--dgen_name", metavar="NAME", dest="dgen_name", type="str",
                            help="name of the new project")
        parser.add_option("--ns", metavar="NS", dest="dgen_ns", type="str",
                          help="namespace for the C++ generator extensions")
        parser.add_option("--type", metavar="TYPE", dest="generator_type", type="choice",
                          choices=["static", "deterministic", "random"], default="random",
                          help="generator type -- one of `static`, `deterministic` or `random` (default)")

        return parser
        
    def _fixArgs(self, args):
        super(InitializeGeneratorTask, self)._fixArgs(args)
        
        if args.dgen_ns == None:
            args.dgen_ns = "__%s" % (args.dgen_name)

    def _do(self, args):
        skeletonBase = "%s/tools/skeleton/task/%s/%s/%s" % (args.base_path, self.group(), self.name(), args.generator_type)
        targetBase = "%s/../.." % (args.base_path)

        self._log.info("Processing skeleton for task.")
        skeletonProcessor = myriad.task.common.SkeletonProcessor(skeletonBase)
        skeletonProcessor.process(targetBase, args, myriad.task.common.SkeletonProcessor.PROCESS_FILES)

