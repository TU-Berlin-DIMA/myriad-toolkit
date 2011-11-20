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

import logging.config
import optparse
import sys

import myriad.event
import myriad.task.compile
import myriad.task.initialize

class UnknownTaskException(Exception):
    '''
    classdocs
    '''
    __qname = None
    
    def __init__(self, qname):
        super(UnknownTaskException, self).__init__()
        self.__qname = qname

    def __str__(self):
        return "Unknown task `%s`" % (self.__qname)

class Assistant(object):
    '''
    classdocs
    '''
    __basePath = None
    __fileName = None
    
    # a flag indicating the `print usage` scenario 
    __isUsage = False
    # a flag indicating the `print task help` scenario 
    __isTaskHelp = False
    # a flag indicating the `execute task` scenario
    __isTaskExecute = False
    # an array of arguments for the task to be executed
    __taskArgv = []
    # the qualified name of the currently processed task
    __taskQName = None
    
    # a list of events supported by the assistant 
    __events = {}
    # a list of task objects supported by the assistant 
    __tasks = {}
    
    # a logger instance
    __log = None
    
    # assistant version
    VERSION = "0.1.0"
    
    def __init__(self, basePath, fileName, argv):
        '''
        Constructor
        '''
        
        self.__basePath = basePath
        self.__fileName = fileName
        
        # initialize the logging subsystem
        logging.config.fileConfig("%s/tools/config/assistant_logging.conf" % (self.__basePath))
        
        self.__log = logging.getLogger("assistant")
        
        self.initialize(argv)
    
    def initialize(self, argv):

        try:
            # register `compile:*` tasks
            self.registerTask(myriad.task.compile.CompileModelTask(self))
            # register `initialize:*` tasks
            self.registerTask(myriad.task.initialize.InitializeProjectTask(self))
            self.registerTask(myriad.task.initialize.InitializeRecordTask(self))
            self.registerTask(myriad.task.initialize.InitializeGeneratorTask(self))
            
            if len(argv) == 2 and argv[0] == "help":
                self.__isTaskHelp = True
                self.__taskQName = argv[1]
            elif len(argv) > 0:
                self.__isTaskExecute = True
                self.__taskQName = argv[0]
                self.__taskArgv = argv[1:]
            else: # len(argv) == 0
                self.__isUsage = True
                
        except:
            e = sys.exc_info()[1]
            self.__printError("unexpected __printError: %s" % (str(e)), True)
            raise
            
    def run(self):
        '''
        Run the assistant
        '''

        try:
            if (self.__isUsage):
                self.__printHeader()
                self.__printUsage()

            elif (self.__isTaskHelp):
                try:
                    task = self.currentTask()
                    
                    self.__printHeader()
                    self.__printTaskHelp(task)
                except UnknownTaskException, e:
                    self.__printHeader(sys.stderr)
                    self.__printErrorLine(str(e), sys.stderr)
                    self.__printUsage(sys.stderr)
                
            elif (self.__isTaskExecute):
                self.__log.info("Running task %s." % (self.__taskQName))
                
                task = None
                
                try:
                    task = self.currentTask()

                    # execute the task
                    task.execute(self.__taskArgv)
                
                # thrown if the task doesn't exist
                except UnknownTaskException, e:
                    self.__printHeader(sys.stderr)
                    self.__printErrorLine(str(e), sys.stderr)
                    self.__printUsage(sys.stderr)
                # thrown by the option parser of the task
                except optparse.OptParseError, e:
                    self.__printHeader()
                    self.__printErrorLine(str(e), sys.stderr)
                    self.__printTaskHelp(task)
                
        except:
            e = sys.exc_info()[1]
            self.__printError("unexpected error: %s" % (str(e)), False)
            raise
        
    def event(self, name):
        if self.__events.has_key(name):
            return self.__events.get(name)
        else:
            raise myriad.event.UndefinedEventException()
            
    def registerTask(self, task):
        self.__tasks[task.qname()] = task
        
    def tasks(self):
        return self.__tasks.values()
    
    def currentTask(self):
        if self.__tasks.has_key(self.__taskQName):
            return self.__tasks[self.__taskQName]
        else:
            raise UnknownTaskException(self.__taskQName)

    def basePath(self):
        return self.__basePath

    def __printHeader(self, out=sys.stdout):
        print >> out, "Myriad Assistant Tool"
        print >> out, "Version %s" % (self.VERSION)
        print >> out, ""
        
    def __printErrorLine(self, errorMessage, out=sys.stderr):
        print >> out, "ERROR: %s " % (errorMessage)
        print >> out, ""

    def __printError(self, out=sys.stderr, errorMessage=None, withUsage = False):
        self.__printHeader(out)
        self.__printErrorLine(errorMessage, out)
        if (withUsage):
            self.__printUsage(out)
        
    def __printUsage(self, out=sys.stdout):
        print >> out, "Usage:"
        print >> out, "  %s                - print this message" % (self.__fileName)
        print >> out, "  %s help [task]    - print information about s specific task" % (self.__fileName)
        print >> out, "  %s task [args]    - execute a specific task" % (self.__fileName)
        print >> out, ""
        print >> out, "Task names follow the {group}:{name} format."
        print >> out, ""
        print >> out, "Available tasks:"
        
        P = list(set([t.group() for t in self.tasks()]))
        P.sort()
        
        for g in P:
            print >> out, "  %s" % (g)
            
            N = [t.name() for t in self.tasks() if t.group() == g]
            D = [t.description() for t in self.tasks() if t.group() == g]
        
            for n, d in zip(N, D):
                print >> out, "    :%-21s - %s" % (n, d)
            
            print >> out, ""
            
    def __printTaskHelp(self, task, out=sys.stdout):

        parser = task.argsParser()

        print >> out, "".join(parser.formatUsage())
        print >> out, "".join(parser.formatDescription())
        print >> out, "".join(parser.formatArgsHelp())
        print >> out, "".join(parser.formatOptionsHelp())
