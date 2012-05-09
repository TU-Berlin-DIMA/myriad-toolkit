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

import logging
import optparse
import types

import ConfigParser
import os.path
import re
import myriad.error
import myriad.util.properties

TASK_PREFIX = "abstract"

class TaskExecutionException(Exception):
    
    value = None
    
    def __init__(self, value):
        self.value = value
        
    def __str__(self):
        return repr(self.value)

class TaskOptions(optparse.OptionParser):
    '''
    classdocs
    '''
    
    def __init__(self, qname, assistant, description=None):
        optparse.OptionParser.__init__(self, usage="%prog "+qname+" [opts] [args]", description=description, version=assistant.VERSION, add_help_option=False)
        self.args_list = []

    def formatUsage(self):
        formatter = optparse.IndentedHelpFormatter(width=80)
        formatter.store_option_strings(self)

        formatter.indent()
        lines = [ formatter._format_text(self.expand_prog_name(self.usage)) + "\n" ]
        formatter.dedent()

        return [ formatter.format_heading("Usage") ] + lines

    def formatDescription(self):
        formatter = optparse.IndentedHelpFormatter(width=80)
        formatter.store_option_strings(self)

        formatter.indent()
        lines = [ formatter.format_description(self.description) ]
        formatter.dedent()

        return [ formatter.format_heading("Description") ] + lines

    def formatArgsHelp(self):
        formatter = optparse.IndentedHelpFormatter(width=80)

        formatter.indent()
        for opt in self.args_list:
            metavar = opt.metavar or opt.dest.upper()
            strings = metavar.upper()
            formatter.option_strings[opt] = strings
        formatter.dedent()
        
        formatter.help_position = 31
        formatter.help_width = formatter.width - formatter.help_position

        formatter.indent()
        lines = [formatter.format_option(opt) for opt in self.args_list]
        formatter.dedent()
        
        return [ formatter.format_heading("Arguments") ] + lines

    def formatOptionsHelp(self):
        formatter = optparse.IndentedHelpFormatter(width=80)

        formatter.indent()
        for opt in self.option_list:
            strings = formatter.format_option_strings(opt)
            formatter.option_strings[opt] = strings
        formatter.dedent()
        
        formatter.help_position = 31
        formatter.help_width = formatter.width - formatter.help_position

        formatter.indent()
        lines = [formatter.format_option(opt) for opt in self.option_list]
        formatter.dedent()

        if len(lines) > 0:
            return [ formatter.format_heading("Options") ] + lines
        else:
            return []

    def add_argument(self, *args, **kwargs):
        
        if type(args[0]) in types.StringTypes:
            option = self.option_class(*args, **kwargs)
        elif len(args) == 1 and not kwargs:
            option = args[0]
            if not isinstance(option, optparse.Option):
                raise TypeError, "not an Option instance: %r" % option
        else:
            raise TypeError, "invalid arguments"

        self._check_conflict(option)

        self.args_list.append(option)
        option.container = self

        if option.dest is not None:     # option has a dest, we need a default
            if option.default is not ("NO", "DEFAULT"):
                self.defaults[option.dest] = option.default
            elif option.dest not in self.defaults:
                self.defaults[option.dest] = None

        return option

    def check_values(self, values, args):
        
        if (len(args) < len(self.args_list)):
            raise self.error("Not enough arguments supplied for this task")
        if (len(args) > len(self.args_list)):
            raise self.error("Too much arguments supplied for this task")
        
        
        try:
            i = 0
            for (option, value) in zip(self.args_list, args):
                # process a single long option (possibly with value(s))
                option.process("--arg%02d" % (i), value, values, self)
                i += 1
                
        except (optparse.BadOptionError, optparse.OptionValueError), err:
            self.error(str(err))
        
        return (values, args)


class AbstractTask(object):
    '''
    classdocs
    '''
    
    __assistant = None
    __basePath = None
    __group = None
    __name = None
    __description = None
    
    _log = None
    
    def __init__(self, assistant, group="default", name="default", description=""):
        '''
        Constructor
        '''
        self.__assistant = assistant
        self.__basePath = assistant.basePath()
        self.__group = group
        self.__name = name
        self.__description = description
        
        self._log = logging.getLogger(self.qname())
        
    def group(self):
        return self.__group
        
    def name(self):
        return self.__name
        
    def qname(self):
        return "%s:%s" % (self.__group, self.__name)
    
    def description(self):
        return self.__description
        
    def argsParser(self):
        parser = TaskOptions(qname=self.qname(), assistant=self.__assistant, description=self.__description)
        parser.remove_option("--version")
        return parser
    
    def execute(self, argv):
        parser = self.argsParser()
        
        self._log.info("Parsing task arguments.")
        args = parser.parse_args(argv)[0]
        
        # fix the parsed arguments
        self._log.info("Fixing parsed values.")
        self._fixArgs(args)
        
        # perform the task, throws a TaskExecutionException on error 
        try:
            self._log.info("Executing task logic.")
            self._do(args)
            self._log.info("Task execution finished successfully.")
                
        except (Exception), e:
            e = TaskExecutionException("A error occurred during task execution: %s" % (e))
            self._log.error(e)
            raise e
        
    def _fixArgs(self, args):
        args.base_path = self.__basePath
        
        if (self._requiresMyriadSettings()):
            
            # try to add the .myriad-settings contents to the list of args
            myriadProjectPath = os.path.realpath("%s/../.." % (args.base_path))
            myriadSettingsPath = "%s/.myriad-settings" % (myriadProjectPath)
            
            if (os.path.exists(myriadSettingsPath)):
                p = myriad.util.properties.Properties();
                p.load(open(myriadSettingsPath))
                
                # add extra args from the .myriad-settings file
                args.dgen_name = p.getProperty("MYRIAD_DGEN_NAME");
                args.dgen_ns = p.getProperty("MYRIAD_DGEN_NS");
                args.dgen_project_path = myriadProjectPath
            else:
                raise myriad.error.UninitializedProjectError(myriadProjectPath)
                
            
    def _requiresMyriadSettings(self):
        return True

    def _do(self, args):
        print args


class ParametersProcessor(object):
    '''
    classdocs
    '''
    class Snippet(object):
        name = None
        pos = None
        match = None
        contents = None
        
        def __init__(self, m):
            self.name = m.group(1)
            self.pos = m.group(2)
            self.match = m.group(3)
            self.contents = []
    
    __patterns = None
    __values = None
    __log = None
    
    def __init__(self, params):
        '''
        Constructor
        '''
        self.__cc2us_pattern1 = re.compile('(.)([A-Z][a-z]+)')
        self.__cc2us_pattern2 = re.compile('([a-z0-9])([A-Z])')
        
        self.__snippetStart = re.compile(r".*`(.+)` (before|after):`(.+)`.*")
        self.__snippetEnd = re.compile(r".*`(.+)`.*")
        self.__patterns = [ re.compile(r"(\${(uc|lc|cc2us)?{%s}})" % (p)) for p in params.__dict__.keys() ]
        self.__values = [ p for p in params.__dict__.values() ]
        self.__log = logging.getLogger("template.processor")
    
    def processString(self, s):
        for (p, v) in zip(self.__patterns, self.__values):
            for m in p.finditer(s):
                expr = m.group(1)
                func = m.group(2)
                
                if func == "uc": 
                    s = s.replace(expr, self.__uc(v))
                elif func == "lc": 
                    s = s.replace(expr, self.__lc(v))
                elif func == "cc2us": 
                    s = s.replace(expr, self.__cc2us(v))
                else:
                    s = s.replace(expr, v)
        return s
    
    def processTemplate(self, sourcePath, targetPath):
        # open file descriptors to source and target
        s = open(sourcePath, 'r')
        t = open(targetPath, 'w')
        
        # process the contents
        for l in s:
            t.write(self.processString(l))
        
        # close file descriptors
        s.close()
        t.close()
        
        # fix file mode
        stat = os.stat(sourcePath)
        os.chmod(targetPath, stat.st_mode)
        
    def processSnippets(self, sourcePath, targetPath):

        # snippets list
        snippets = []
        # current snippet
        snippet = None
        # open file descriptors to source and target
        state = 0

        # open source file descriptor
        f = open(sourcePath, 'r')
        # phase 1: fetch snippets
        for l in f:
            # expand variables in the current line
            l = self.processString(l)
            # awaiting opening snippet
            if state == 0:
                m = self.__snippetStart.match(l)
                if m == None:
                    pass # ignore this line
                else:
                    snippet = ParametersProcessor.Snippet(m)
                    state = 1
            # construct open snippet
            elif state == 1:
                m = self.__snippetEnd.match(l)
                if m == None:
                    snippet.contents.append(l)
                else:
                    # make sure that snippet name matches
                    if m.group(1) != snippet.name:
                        raise SyntaxError, "closing snippet does not match name `%s`" % (snippet.name)
                    snippets.append(snippet)
                    state = 0
        # close file descriptor
        f.close()
        
        # initialize target buffer
        buf = []
        # open target file descriptor
        f = open(targetPath, 'r')
        # phase 2: expand snippets
        for l in f:
            # expansion lines before and after the current line
            pre = []
            post = []
            for c in snippets:
                if c.match in l:
                    if c.pos == "before":
                        pre += c.contents
                    elif c.pos == "after":
                        post += c.contents
            buf += pre + [l] + post
        # close file descriptor
        f.close()
        
        # save expanded target contents
        f = open(targetPath, 'w')
        f.writelines(buf)
        f.close()
        
    def __uc(self, s):
        return s.upper()

    def __lc(self, s):
        return s.lower()

    def __cc2us(self, s):
        return self.__cc2us_pattern2.sub(r'\1_\2', self.__cc2us_pattern1.sub(r'\1_\2', s)).lower()

class SkeletonProcessor(object):
    '''
    classdocs
    '''
    
    PROCESS_DIRS  = 0x1
    PROCESS_FILES = 0x2
    
    __skeletonBase = None
    __log = None
    
    def __init__(self, skeletonBase):
        '''
        Constructor
        '''
        self.__skeletonBase = skeletonBase
        self.__log = logging.getLogger("skeleton.processor")
        
    def process(self, targetBase, params, flags = PROCESS_FILES | PROCESS_DIRS):
        '''
        Process a skeleton. Initializes the skeleton directory structure at the 
        specified target location and process all skeleton files using the 
        provided set of parameters.
        '''
        
        # create a parameters processor
        paramsProcessor = ParametersProcessor(params)
        
        # create directories
        if (flags & SkeletonProcessor.PROCESS_DIRS == SkeletonProcessor.PROCESS_DIRS):
            self.__createDirs(targetBase)
        
        # create files
        if (flags & SkeletonProcessor.PROCESS_FILES == SkeletonProcessor.PROCESS_FILES):
            self.__createFiles(targetBase, paramsProcessor)
        
    def __createDirs(self, targetBase):
        '''
        Create the skeleton's directory structure at the specified target 
        location. 
        '''
        for t in os.walk(self.__skeletonBase):
            prefix = t[0].replace(self.__skeletonBase, "").lstrip("/")
            
            for d in t[1]:
                if (len(prefix) > 0):
                    dirPath = "%s/%s/%s" % (targetBase, prefix, d)
                else:
                    dirPath = "%s/%s" % (targetBase, d)
                
                if (not os.path.isdir(dirPath)):
                    self.__log.info("Create dir: %s" % (dirPath))
                    os.mkdir(dirPath)
                else:
                    self.__log.info("Skip existing dir: %s" % (dirPath))
                    
    def __createFiles(self, targetBase, paramsProcessor):
        '''
        Instantiate the skeleton's files at the specified target location using
        the set of provided parameters. 
        '''
        for t in os.walk(self.__skeletonBase):
            prefix = t[0].replace(self.__skeletonBase, "").lstrip("/")
            
            metaConfig = self.__loadMeta(t[0], paramsProcessor)
            
            for sourceName in t[2]:
                # skip meta sources
                if self.__isMeta(sourceName):
                    continue
                
                # compute source path
                sourcePath = "%s/%s" % (t[0], sourceName)
            
                # compute target path
                if (metaConfig.has_option("filenames", sourceName)):
                    targetName = metaConfig.get("filenames", sourceName)
                else:
                    targetName = sourceName
                
                # fix target name of *.snippets files
                if (self.__isSnippets(sourceName)):
                    targetName = targetName.replace(".snippets", "")
                
                if (len(prefix) > 0):
                    targetPath = "%s/%s/%s" % (targetBase, prefix, targetName)
                else:
                    targetPath = "%s/%s" % (targetBase, targetName)
                
                # process *.snippets file
                if (self.__isSnippets(sourceName)):
                    self.__log.info("Process snippets for: %s" % (targetPath))
                    paramsProcessor.processSnippets(sourcePath, targetPath)
                # process template file
                elif (not os.path.isfile(targetPath)):
                    self.__log.info("Create file: %s" % (targetPath))
                    paramsProcessor.processTemplate(sourcePath, targetPath)
                else:
                    self.__log.info("Skip existing file: %s" % (targetPath))
            
    def __loadMeta(self, path, paramsProcessor):
        '''
        Load the skeleton meta information from the specified path.  
        '''
        metaConfig = ConfigParser.RawConfigParser()
        
        metaPath = "%s/.meta" % (path)
        if os.path.exists(metaPath):
            metaConfig.readfp(open(metaPath))
        
        if metaConfig.has_section("filenames"):
            for (n, v) in metaConfig.items("filenames"):
                metaConfig.set("filenames", n, paramsProcessor.processString(v))

        return metaConfig
    
    def __isMeta(self, fileName):
        return fileName == ".meta"
    
    def __isSnippets(self, fileName):
        return fileName.endswith(".snippets")