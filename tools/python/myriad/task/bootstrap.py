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
 
Created on Mar 2, 2011

@author: Alexander Alexandrov <alexander.s.alexandrov@campus.tu-berlin.de>
'''

import myriad.task.common

class ProjectTask(myriad.task.common.AbstractTask):
    '''
    classdocs
    '''

    def __init__(self, dispatcher):
        '''
        Constructor
        '''
        super(ProjectTask, self).__init__(dispatcher)


class RecordTask(myriad.task.common.AbstractTask):
    '''
    classdocs
    '''

    def __init__(self, dispatcher):
        '''
        Constructor
        '''
        super(RecordTask, self).__init__(dispatcher)


class GeneratorTask(myriad.task.common.AbstractTask):
    '''
    classdocs
    '''

    def __init__(self, dispatcher):
        '''
        Constructor
        '''
        super(GeneratorTask, self).__init__(dispatcher)
