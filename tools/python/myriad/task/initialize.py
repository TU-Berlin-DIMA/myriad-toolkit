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

@author: Alexander Alexandrov <alexander.s.alexandrov@campus.tu-berlin.de>
'''

import os

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
        parser.add_argument("--name", metavar="NAME", dest="project_name", type="str",
                            help="name of the new project")
        # options
        parser.add_option("--ns", metavar="NS", dest="project_ns", type="str",
                          help="namespace for the C++ generator extensions")

        return parser
        
    def _fixArgs(self, args):
        super(InitializeProjectTask, self)._fixArgs(args)
        
        if args.project_ns == None:
            args.project_ns = "__%s" % (args.project_name)

    def _do(self, args):
        
        skeletonBase = "%s/tools/skeleton/task/%s/%s" % (args.base_path, self.group(), self.name())
        targetBase = "%s/../.." % (args.base_path)

        self._log.info("Processing skeleton for task.")
        skeletonProcessor = myriad.task.common.SkeletonProcessor(skeletonBase)
        skeletonProcessor.process(targetBase, args);

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
        parser.add_argument("--name", metavar="NAME", dest="config", type="str",
                             help="name of the new record")
        # options

        return parser


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
        parser.add_argument("--name", metavar="NAME", dest="config", type="str",
                            help="name of the new generator")
        # options

        return parser

