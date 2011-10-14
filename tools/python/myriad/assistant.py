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

import os, sys, tempfile, optparse, datetime, time

import myriad.task.bootstrap
import myriad.event

class Assistant(object):
    '''
    classdocs
    '''
    __basePath = None
    __tasks = []
    
    __isSummary = False
    __isTaskHelp = False
    __isTaskExecute = False
    __taskArgs = []
    
    __taskName = None
    
    __events = {}
    
    VERSION = "0.1.0"
    
    def __init__(self, basePath, argv):
        '''
        Constructor
        '''
        
        self.__basePath = basePath
        self.initialize(argv)
    
    def initialize(self, argv):

        try:
            # initialize events
            self.__events["task.summary"] = myriad.event.Event()
            self.__events["task.help"] = myriad.event.Event()
            self.__events["task.execute"] = myriad.event.Event()
            
            self.__registerTask(myriad.task.bootstrap.ProjectTask(self))
            self.__registerTask(myriad.task.bootstrap.RecordTask(self))
            self.__registerTask(myriad.task.bootstrap.GeneratorTask(self))
            
            if len(argv) == 0:
                self.__isSummary = True
            elif len(argv) == 2 and argv[0] == "help":
                self.__isTaskHelp = True
                self.__taskName = argv[1]
            elif len(argv) > 1:
                self.__isTaskExecute = True
                self.__taskName = argv[1]
                self.__taskArgs = argv[2:]

        except:
            e = sys.exc_info()[1]
            self.error("unexpected error: %s" % (str(e)), True)
            raise
            
    def run(self):
        '''
        Run the assistant
        '''

        try:
            print "running assistant"
            
            if (self.__isSummary):
                self.event("task.summary").fire()
                print "render summary"

            elif (self.__isTaskHelp):
                self.event("task.help").fire(taskName=self.__taskName)
                print "render task help"
                
            elif (self.__isTaskExecute):
                self.event("task.execute").fire(taskName=self.__taskName)
                print "render task execute"
                
        except:
            e = sys.exc_info()[1]
            self.error("unexpected error: %s" % (str(e)), True)
            raise
        
    def event(self, name):
        if self.__events.has_key(name):
            return self.__events.get(name)
        else:
            raise myriad.event.UndefinedEventException()

    def error(self, message=None, withUsage = False):
        if (withUsage):
            self.parser.print_usage(sys.stderr)
        if (message != None):
            self.log.error("%s: error: %s", self.parser.get_prog_name(), message)
            
    def __registerTask(self, task):
        self.__tasks.append(task)
