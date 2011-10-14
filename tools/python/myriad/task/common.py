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

import optparse

TASK_PREFIX = "abstract"

class AbstractTask(object):
    '''
    classdocs
    '''
    
    __dispatcher = None
    __group = None
    __name = None
    __description = None
    
    _argsParser = None

    def __init__(self, dispatcher, group="default", name="default", description=""):
        '''
        Constructor
        '''
        self.__dispatcher = dispatcher
        self.__group = group
        self.__name = name
        self.__description = description
        
    def group(self):
        return self.__group
        
    def name(self):
        return self.__name
        
    def qname(self):
        return "%s:%s" % (self.__group, self.__name)
        
    def description(self, *args, **keywargs):
        return self.__description
        
    def argsParser(self):
        if self._argsParser == None:
            self._argsParser = optparse.OptionParser(usage="%prog "+self.qname()+" [args]", version=self.__dispatcher.VERSION, add_help_option=False)
            self.initArgsParser()
        return self._argsParser
    
    def initArgsParser(self):
        pass