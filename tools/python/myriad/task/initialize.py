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
    
    def _initArgsParser(self):
        self._argsParser.add_positional_argument("--name", metavar="NAME", dest="config", type="str",
                             help="name of the new project")
        self._argsParser.add_option("--ns", metavar="NS", dest="config", type="str",
                                     help="namespace for the C++ generator extensions")

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
    
    def _initArgsParser(self):
        self._argsParser.add_positional_argument("--name", metavar="NAME", dest="config", type="str",
                             help="name of the new record")


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
    
    def _initArgsParser(self):
        self._argsParser.add_positional_argument("--name", metavar="NAME", dest="config", type="str",
                     help="name of the new generator")

