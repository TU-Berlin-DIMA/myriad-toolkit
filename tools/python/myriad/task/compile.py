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

from myriad.compiler.debug import PrintVisitor #@UnusedImport
from myriad.compiler.reader import PrototypeSpecificationReader
from myriad.compiler.source import * #@UnusedWildImport
from myriad.task.common import AbstractTask
from subprocess import Popen, PIPE

TASK_PREFIX = "compile"

class CompilePrototypeTask(AbstractTask):
    '''
    classdocs
    '''

    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        kwargs.update(group=TASK_PREFIX, name="prototype", description="Compile data generator C++ extensions from an XML prototype.")

        super(CompilePrototypeTask, self).__init__(*args, **kwargs)
    
    def argsParser(self):
        parser = super(CompilePrototypeTask, self).argsParser()
        
        # arguments
        parser.add_option("--prototype-file", metavar="PROTOTYPE", dest="prototype_path", type="str", 
                          default=None, help="path to the compiled XML prototype file (defaults to `${config-dir}/${dgen-name}-prototype.xml`)")

        return parser
        
    def _fixArgs(self, args):
        super(CompilePrototypeTask, self)._fixArgs(args)
        
        if (args.prototype_path == None):
            args.prototype_path = "%s-prototype.xml" % (args.dgen_name)
                    
        if (not os.path.isabs(args.prototype_path)):
            args.prototype_path = "%s/../../src/config/%s" % (args.base_path, args.prototype_path)
            
        args.prototype_path = os.path.realpath(args.prototype_path)
        
    def _do(self, args):
        # reed the AST
        reader = PrototypeSpecificationReader(args)
        ast = reader.read()

        # compile output collector
        frontendCompiler = FrontendCompiler(args=args)
        frontendCompiler.compileCode(ast)
        # compile generator config
        generatorSubsystemCompiler = GeneratorSubsystemCompiler(args=args)
        generatorSubsystemCompiler.compileCode(ast)
        # compile generator config
        configCompiler = ConfigCompiler(args=args)
        configCompiler.compileCode(ast)
        # compile record types
        recordCompiler = RecordTypeCompiler(args=args)
        recordCompiler.compileCode(ast.getSpecification().getRecordSequences())
        # compile record setter chains
        setterChainCompiler = SetterChainCompiler(args=args)
        setterChainCompiler.compileCode(ast.getSpecification().getRecordSequences())
        # compile record generators
        generatorCompiler = AbstractSequenceGeneratorCompiler(args=args)
        generatorCompiler.compileCode(ast.getSpecification().getRecordSequences())


class CompileOligosTask(AbstractTask):
    '''
    classdocs
    '''

    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        kwargs.update(group=TASK_PREFIX, name="oligos", description="Compile an XML prototype form a reference DB using the Oligos tool.")
        super(CompileOligosTask, self).__init__(*args, **kwargs)
    
    def argsParser(self):
        parser = super(CompileOligosTask, self).argsParser()
        
        # arguments
        parser.add_argument("--schema", metavar="SCHEMA", dest="schema", type="str",
                            help="schema pattern to be profiled")
        # options
        parser.add_option("-h", "--host", metavar="HOST", dest="db_host", type="str",
                          default="localhost", help="hostname to use for the DB connection (defaults to `localhost`)")
        parser.add_option("-u", "--user", metavar="USER", dest="db_user", type="str",
                          default="DB2INST1", help="username to use for the DB connection (defaults to `DB2INST2`)")
        parser.add_option("-p", "--password", metavar="PASS", dest="db_pass", type="str",
                          default=None, help="password to use for the DB connection")
        parser.add_option("-D", "--database", metavar="NAME", dest="db_name", type="str",
                          default=None, help="database to use for the DB connection")
        parser.add_option("-P", "--port", metavar="PORT", dest="db_port", type="int",
                          default=60000, help="port to use for the DB connection (defaults to `60000`)")

        return parser

    def _do(self, args):

        command = []
        command.append('java')
        # add optional custom java classpath
        command.append('-cp')
        command.append(':'.join(filter(lambda x: x, ['%s/bin/oligos.jar' % args.base_path, args.oligos_cp, os.environ.get('CLASSPATH', '')])).strip(':'))
        # add frontend class
        command.append('de.tu_berlin.dima.oligos.Oligos')
        # add DB connection params
        if (args.db_user):
            command.append('-u%s' % args.db_user)
        if (args.db_pass):
            command.append('-p%s' % args.db_pass)
        if (args.db_host):
            command.append('-h%s' % args.db_host)
        if (args.db_name):
            command.append('-D%s' % args.db_name)
        if (args.db_port):
            command.append('-P%s' % args.db_port)
        command.append('-g%s' % args.dgen_name)
        command.append('-o%s/../../src/config' % args.base_path)
        # add SCHEMA argument
        command.append('%s' % args.schema)

        self._log.info("Running Oligos data profiler with command `%s`." % ' '.join(command))

        # invoke Oligos executable and redirect the output 
        proc = Popen(command, stdout=PIPE)
        (out, err) = proc.communicate()
        if (out):
            self._log.info("Forwarding Oligos standard output:")
            self._log.info("")
            for line in out.split('\n'):
                self._log.info(line)
        if (err):
            self._log.info("Forwarding Oligos standard error:")
            self._log.info("")
            for line in err.split('\n'):
                self._log.info(line)
