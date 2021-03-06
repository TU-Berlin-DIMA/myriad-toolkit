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
 
Created on Feb 25, 2012

@author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
'''

import logging
import os
import re

from myriad.compiler.ast import AbstractCardinalityEstimatorNode
from myriad.compiler.ast import AbstractFunctionNode
from myriad.compiler.ast import AbstractRuntimeComponentNode
from myriad.compiler.ast import ArgumentCollectionNode
from myriad.compiler.ast import CallbackValueProviderNode
from myriad.compiler.ast import DepthFirstNodeFilter
from myriad.compiler.ast import EnumSetNode
from myriad.compiler.ast import LiteralArgumentNode
from myriad.compiler.ast import RandomSequenceNode
from myriad.compiler.ast import ResolvedFieldRefArgumentNode
from myriad.compiler.ast import ResolvedFunctionRefArgumentNode
from myriad.compiler.ast import RecordEnumFieldNode
from myriad.util.stringutil import StringTransformer

class ArgumentTransformer(object):
    
    _log = logging.getLogger("source.transformer.factory")
    _descriptor_pattern = re.compile('^([a-zA-Z_]+)\((.+)\)(\*)?$')
    
    @classmethod
    def createTransformer(cls, transformerDescriptor):
        m = cls._descriptor_pattern.match(transformerDescriptor)
        if (m):
            transformerType = m.group(1)
            argTransformer = None
            argKey = m.group(2)
            argOptional = None 
            argOptional = m.group(3) is not None 
            
            if (transformerType == "Literal"):
                argTransformer = LiteralTransfomer()
            elif (transformerType == "Verbatim"):
                argTransformer = VerbatimTransfomer(verbatimCode=argKey)
                argKey = None
            elif (transformerType == "EnvVariable"):
                argTransformer = EnvVariableTransfomer(varName=argKey)
                argKey = None
            elif (transformerType == "FieldSetterRef"):
                argTransformer = FieldSetterRefTransfomer()
            elif (transformerType == "SequenceInspector"):
                argTransformer = SequenceInspectorTransfomer(recordTypeName=argKey)
                argKey = None
            elif (transformerType == "FunctionRef"):
                argTransformer = FunctionRefTransfomer()
            elif (transformerType == "RuntimeComponentRef"):
                argTransformer = RuntimeComponentRefTransformer()
            else:
                message = "Unknown argument transformer type `%s`" % (transformerType)
                cls._log.error(message)
                raise RuntimeError(message)
            
            return (argTransformer, argKey, argOptional)
        else:
            message = "Bad argument transformer descriptor `%s`" % (transformerDescriptor)
            cls._log.error(message)
            raise RuntimeError(message)
    
    @classmethod
    def compileConstructorArguments(cls, argsContainerNode, env = {}):
        argsCode = []
        
        for transformerDescriptor in argsContainerNode.getConstructorArguments():
            (argTransformer, argKey, argOptional) = cls.createTransformer(transformerDescriptor)
            
            if argKey is None:
                argument = None
            else:
                argument = argsContainerNode.getArgument(argKey)
            
            argsCode.extend(argTransformer.transform(argument, env.get("config", "config"), argOptional))
        
        return filter(None, argsCode)


class FieldTransfomer(object):

    def __init__(self, *args, **kwargs):
        pass
    
    def transform(self, argumentNode = None, configVarName = "config", optional = False):
        raise RuntimeError("Called abstract method FieldTransfomer.transform()")


class LiteralTransfomer(object):
    
    _param_pattern = re.compile('%(\([a-zA-Z0-9]+\))?([\w.\-]+)%')
    _literal_pattern = re.compile('(\([a-zA-Z0-9]+\))?(.+)')
    _expr_pattern = re.compile('^\${(.+)}$')
    
    def __init__(self, *args, **kwargs):
        super(LiteralTransfomer, self).__init__()
    
    def transform(self, argumentNode = None, configVarName = "config", optional = False):
        if optional is True and argumentNode is None:
            return [ None ]
        
        if configVarName is not None:
            configPrefix = configVarName + "."
        else:
            configPrefix = ""
        
        if isinstance(argumentNode, LiteralArgumentNode):
            attributeType = argumentNode.getAttribute("type").strip()
            attributeValue = argumentNode.getAttribute("value").strip()
            
            paramMatch = self._param_pattern.match(attributeValue)
            if (paramMatch):
                return [ '%sparameter<%s>("%s")' % (configPrefix, attributeType, paramMatch.group(2)) ]
            
            exprMatch = self._expr_pattern.match(attributeValue)
            if (exprMatch):
                exprExpandedParams = self._param_pattern.sub(lambda exprMatch: '%sparameter<%s>("%s")' % (configPrefix, attributeType if exprMatch.group(1) == None else exprMatch.group(1)[1:-1], exprMatch.group(2)), attributeValue)
                return [ "static_cast<%s>(%s)" % (attributeType, exprExpandedParams[2:-1]) ]
            
            literalMatch = self._literal_pattern.match(attributeValue)
            if (literalMatch):
                if literalMatch.group(1) is None:
                    if literalMatch.group(2) == "NULL": # explicit case for NULL values
                        return [ 'Myriad::nullValue<%s>()' % (attributeType) ]
                    if attributeType == "String":
                        return [ '"%s"' % (literalMatch.group(2)) ]
                    if attributeType == "Char":
                        return [ "'%s'" % (literalMatch.group(2)) ]
                    if attributeType == "Date":
                        return [ 'Date("%s")' % (literalMatch.group(2)) ]
                    else:
                        return [ '%s' % (literalMatch.group(2)) ]
                else: # explicit cast
                    if literalMatch.group(2) == "NULL": # explicit case for NULL values
                        return [ 'Myriad::nullValue<%s>()' % (literalMatch.group(1)[1:-1]) ]
                    if attributeType == "String":
                        return [ 'static_cast<%s>("%s")' % (literalMatch.group(1)[1:-1], literalMatch.group(2)) ]
                    if attributeType == "Char":
                        return [ "static_cast<%s>(%s)" % (literalMatch.group(1)[1:-1], literalMatch.group(2)) ]
                    if attributeType == "Date":
                        return [ 'Date(static_cast<%s>("%s"))' % (literalMatch.group(1)[1:-1], literalMatch.group(2)) ]
                    else:
                        return [ 'static_cast<%s>(%s)' % (literalMatch.group(1)[1:-1], literalMatch.group(2)) ]
            else:
                raise RuntimeError("Unsupported literal value `%s`" % (attributeValue))
            
        else:
            raise RuntimeError("Unsupported argument `%s` of type `%s`" % (argumentNode.getAttribute("key"), type(argumentNode)))


class FieldSetterRefTransfomer(object):
    
    def __init__(self, *args, **kwargs):
        super(FieldSetterRefTransfomer, self).__init__()
    
    def transform(self, argumentNode = None, configVarName = "config", optional = False):
        if optional is True and argumentNode is None:
            return [ None ]
        
        if not isinstance(argumentNode, ResolvedFieldRefArgumentNode):
            raise RuntimeError("Unsupported argument `%s` of type `%s`" % (argumentNode.getAttribute("name"), type(argumentNode)))
        elif len(argumentNode.getInnerPathRefs()) > 0:
            raise RuntimeError("Field `%s` is not directly attached to the context (i.e. the inner path is not empty)" % (argumentNode.getFieldRef().getAttribute("name")))
        elif not argumentNode.getFieldRef().hasSetter():
            raise RuntimeError("Field `%s` does not have an associated setter" % (argumentNode.getFieldRef().getAttribute("name")))
        else:
            return [ argumentNode.getFieldRef().getSetter().getAttribute("var_name") ]


class SequenceInspectorTransfomer(object):
    
    __recordTypeName = None

    def __init__(self, *args, **kwargs):
        super(SequenceInspectorTransfomer, self).__init__()
        self.__recordTypeName = kwargs.get("recordTypeName")
    
    def transform(self, argumentNode = None, configVarName = "config", optional = False):
        if optional is True and argumentNode is None:
            return [ None ]
        
        if configVarName is not None:
            configPrefix = configVarName + "."
        else:
            configPrefix = ""
            
        return [ '%sgeneratorPool().get<%sGenerator>().inspector()' % (configPrefix, self.__recordTypeName) ]


class FunctionRefTransfomer(object):

    def __init__(self, *args, **kwargs):
        super(FunctionRefTransfomer, self).__init__()
    
    def transform(self, argumentNode = None, configVarName = "config", optional = False):
        if optional is True and argumentNode is None:
            return [ None ]
        
        if configVarName is not None:
            configPrefix = configVarName + "."
        else:
            configPrefix = ""
            
        if isinstance(argumentNode, ResolvedFunctionRefArgumentNode):
            functionType = argumentNode.getAttribute("concrete_type")
            functionName = argumentNode.getAttribute("ref")
            return [ '%sfunction< %s >("%s")' % (configPrefix, functionType, functionName) ]
        else:
            raise RuntimeError("Unsupported argument `%s` of type `%s`" % (argumentNode.getAttribute("key"), type(argumentNode)))


class RuntimeComponentRefTransformer(object):

    def __init__(self, *args, **kwargs):
        super(RuntimeComponentRefTransformer, self).__init__()
    
    def transform(self, argumentNode = None, configVarName = "config", optional = False):
        if optional is True and argumentNode is None:
            return [ None ]
        
        if isinstance(argumentNode, AbstractRuntimeComponentNode):
            return [ '%s' % (argumentNode.getAttribute("var_name")) ]
        elif isinstance(argumentNode, ArgumentCollectionNode):
            result = []
            for childArgument in argumentNode.getAll():
                result.extend(self.transform(childArgument, configVarName, optional)) 
            return result
        else:
            raise RuntimeError("Unsupported argument `%s` of type `%s`" % (argumentNode.getAttribute("key"), type(argumentNode)))


class EnvVariableTransfomer(object):
    
    __varName = None

    def __init__(self, *args, **kwargs):
        super(EnvVariableTransfomer, self).__init__()
        self.__varName = kwargs.get("varName")
    
    def transform(self, argumentNode = None, configVarName = "config", optional = False):
        return [ self.__varName ]


class VerbatimTransfomer(object):
    
    __verbatimCode = None

    def __init__(self, *args, **kwargs):
        super(VerbatimTransfomer, self).__init__()
        self.__verbatimCode = kwargs.get("verbatimCode")
    
    def transform(self, argumentNode = None, configVarName = "config", optional = False):
        return [ self.__verbatimCode ]


class SourceCompiler(object):
    '''
    An abstract compiler type for all C++ source compiler components. 
    '''
    
    BUFFER_SIZE = 512
    PARAM_PREFIX = 'generator.'
    
    _args = None
    _srcPath = None
    _dgenName = None
    
    _log = None
    
    def __init__(self, args):
        '''
        Constructor
        '''
        self._args = args
        self._srcPath = "%s/../../src" % (args.base_path)
        self._dgenName = args.dgen_name
        
        self._log = logging.getLogger("source.compiler")


class FrontendCompiler(SourceCompiler):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        super(FrontendCompiler, self).__init__(*args, **kwargs)
        
    def compileCode(self, astRoot):
        self._log.info("Compiling frontend C++ sources.")
        self.compileMainMethod(astRoot)
            
    def compileMainMethod(self, astRoot):
        try:
            os.makedirs("%s/cpp/core" % (self._srcPath))
        except OSError:
            pass
        
        sourcePath = "%s/cpp/core/main.cpp" % (self._srcPath)
        
        if (not os.path.isfile(sourcePath)):
            wfile = open(sourcePath, "w", SourceCompiler.BUFFER_SIZE)
            
            print >> wfile, '#include "core/constants.h"'
            print >> wfile, '#include "core/Frontend.h"'
            print >> wfile, ''
            print >> wfile, 'namespace Myriad {'
            print >> wfile, ''
            print >> wfile, '/**'
            print >> wfile, ' * Application name.'
            print >> wfile, ' */'
            print >> wfile, 'const String Constant::APP_NAME = "%s - Parallel Data Generator";' % (self._args.dgen_name)
            print >> wfile, ''
            print >> wfile, '/**'
            print >> wfile, ' * Application version.'
            print >> wfile, ' */'
            print >> wfile, 'const String Constant::APP_VERSION = "0.1.0";'
            print >> wfile, ''
            print >> wfile, '} // Myriad namespace'
            print >> wfile, ''
            print >> wfile, '// define the application main method'
            print >> wfile, 'POCO_APP_MAIN(Myriad::Frontend)'
    
            wfile.close()


class GeneratorSubsystemCompiler(SourceCompiler):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        super(GeneratorSubsystemCompiler, self).__init__(*args, **kwargs)
        
    def compileCode(self, astRoot):
        self._log.info("Compiling generator subsystem C++ sources.")
        self.compileBaseGeneratorSubsystem(astRoot)
        self.compileGeneratorSubsystem(astRoot)
            
    def compileBaseGeneratorSubsystem(self, astRoot):
        try:
            os.makedirs("%s/cpp/generator/base" % (self._srcPath))
        except OSError:
            pass
        
        wfile = open("%s/cpp/generator/base/BaseGeneratorSubsystem.h" % (self._srcPath), "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '// auto-generatad base generator config C++ file'
        print >> wfile, ''
        print >> wfile, '#ifndef BASEGENERATORSUBSYSTEM_H_'
        print >> wfile, '#define BASEGENERATORSUBSYSTEM_H_'
        print >> wfile, ''
        print >> wfile, '#include "generator/AbstractGeneratorSubsystem.h"'
        print >> wfile, ''
        print >> wfile, 'namespace Myriad {'
        print >> wfile, ''
        print >> wfile, 'class BaseGeneratorSubsystem: public AbstractGeneratorSubsystem'
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        print >> wfile, '    BaseGeneratorSubsystem(NotificationCenter& notificationCenter, const vector<bool>& executeStages) : '
        print >> wfile, '        AbstractGeneratorSubsystem(notificationCenter, executeStages)'
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    virtual ~BaseGeneratorSubsystem()'
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, 'protected:'
        print >> wfile, ''
        print >> wfile, '    virtual void registerGenerators();'
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '} // namespace Myriad'
        print >> wfile, ''
        print >> wfile, '#endif /* BASEGENERATORSUBSYSTEM_H_ */'
        
        wfile.close()
        
        wfile = open("%s/cpp/generator/base/BaseGeneratorSubsystem.cpp" % (self._srcPath), "w", SourceCompiler.BUFFER_SIZE)

        print >> wfile, '// auto-generatad base generator config C++ file'
        print >> wfile, ''
        print >> wfile, '#include "generator/base/BaseGeneratorSubsystem.h"'
        
        for recordSequence in astRoot.getSpecification().getRecordSequences().getRecordSequences():
            print >> wfile, '#include "generator/%sGenerator.h"' % (StringTransformer.us2ccAll(recordSequence.getAttribute("key")))

        print >> wfile, ''
        print >> wfile, 'using namespace std;'
        print >> wfile, 'using namespace Poco;'
        print >> wfile, ''
        print >> wfile, 'namespace Myriad {'
        print >> wfile, ''
        print >> wfile, '// the initial stage ID should always be zero'
        print >> wfile, 'I32u AbstractSequenceGenerator::Stage::NEXT_STAGE_ID = 0;'
        print >> wfile, ''
        print >> wfile, '// register the valid stages for the Myriad generator extension'
        print >> wfile, 'AbstractSequenceGenerator::StageList initList()'
        print >> wfile, '{'
        print >> wfile, '    AbstractSequenceGenerator::StageList tmp;'
        print >> wfile, ''
        
        for recordSequence in astRoot.getSpecification().getRecordSequences().getRecordSequences():
            print >> wfile, '    tmp.push_back(AbstractSequenceGenerator::Stage("%s"));' % (recordSequence.getAttribute("key"))
        
        print >> wfile, ''
        print >> wfile, '    return tmp;'
        print >> wfile, '}'
        print >> wfile, ''
        print >> wfile, 'const AbstractSequenceGenerator::StageList AbstractSequenceGenerator::STAGES(initList());'
        print >> wfile, ''
        print >> wfile, '// register the record sequence generators'
        print >> wfile, 'void BaseGeneratorSubsystem::registerGenerators()'
        print >> wfile, '{'
        
        for recordSequence in astRoot.getSpecification().getRecordSequences().getRecordSequences():
            print >> wfile, '    registerGenerator<%s::%sGenerator>("%s");' % (self._args.dgen_ns, StringTransformer.us2ccAll(recordSequence.getAttribute("key")), recordSequence.getAttribute("key"))
            
        print >> wfile, '}'
        print >> wfile, ''
        print >> wfile, '} // namespace Myriad'
        
        wfile.close()
            
    def compileGeneratorSubsystem(self, astRoot):
        try:
            os.makedirs("%s/cpp/generator" % (self._srcPath))
        except OSError:
            pass
        
        sourceHeaderPath = "%s/cpp/generator/GeneratorSubsystem.h" % (self._srcPath)
        sourcePath = "%s/cpp/generator/GeneratorSubsystem.cpp" % (self._srcPath)
        
        if (not os.path.isfile(sourceHeaderPath)):
            wfile = open(sourceHeaderPath, "w", SourceCompiler.BUFFER_SIZE)
            
            print >> wfile, '#ifndef GENERATORSUBSYSTEM_H_'
            print >> wfile, '#define GENERATORSUBSYSTEM_H_'
            print >> wfile, ''
            print >> wfile, '#include "generator/base/BaseGeneratorSubsystem.h"'
            print >> wfile, ''
            print >> wfile, 'using namespace std;'
            print >> wfile, 'using namespace Poco;'
            print >> wfile, ''
            print >> wfile, 'namespace Myriad {'
            print >> wfile, ''
            print >> wfile, 'class GeneratorSubsystem : public BaseGeneratorSubsystem'
            print >> wfile, '{'
            print >> wfile, 'public:'
            print >> wfile, ''
            print >> wfile, '    GeneratorSubsystem(NotificationCenter& notificationCenter, const vector<bool>& executeStages) :'
            print >> wfile, '        BaseGeneratorSubsystem(notificationCenter, executeStages)'
            print >> wfile, '    {'
            print >> wfile, '    }'
            print >> wfile, ''
            print >> wfile, '    virtual ~GeneratorSubsystem()'
            print >> wfile, '    {'
            print >> wfile, '    }'
            print >> wfile, ''
            print >> wfile, 'protected:'
            print >> wfile, ''
            print >> wfile, '    virtual void registerGenerators();'
            print >> wfile, '};'
            print >> wfile, ''
            print >> wfile, '} // namespace Myriad'
            print >> wfile, ''
            print >> wfile, '#endif /* GENERATORSUBSYSTEM_H_ */'
    
            wfile.close()
        
        if (not os.path.isfile(sourcePath)):
            wfile = open(sourcePath, "w", SourceCompiler.BUFFER_SIZE)
            
            print >> wfile, '#include "generator/GeneratorSubsystem.h"'
            print >> wfile, ''
            print >> wfile, 'using namespace std;'
            print >> wfile, 'using namespace Poco;'
            print >> wfile, ''
            print >> wfile, 'namespace Myriad {'
            print >> wfile, ''
            print >> wfile, 'void GeneratorSubsystem::registerGenerators()'
            print >> wfile, '{'
            print >> wfile, '    BaseGeneratorSubsystem::registerGenerators();'
            print >> wfile, ''
            print >> wfile, '    // register generators with the autorelease pool'
            print >> wfile, '}'
            print >> wfile, ''
            print >> wfile, '} // namespace Myriad'
    
            wfile.close()    


class ConfigCompiler(SourceCompiler):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        super(ConfigCompiler, self).__init__(*args, **kwargs)
        
    def compileCode(self, astRoot):
        self._log.info("Compiling generator config C++ sources.")
        self.compileParameters(astRoot)
        self.compileBaseConfig(astRoot)
        self.compileConfig(astRoot)
            
    def compileParameters(self, astRoot):
        try:
            os.makedirs("%s/config" % (self._srcPath))
        except OSError:
            pass
        
        # we assume that the node config *.properties file already exists
        rfile = open("%s/config/%s-node.properties" % (self._srcPath, self._dgenName), "r", SourceCompiler.BUFFER_SIZE)
        lines = rfile.readlines()
        rfile.close()
        
        parameters = astRoot.getSpecification().getParameters().getAll()
        
        filteredLines = []

        for l in lines:
            l = l.strip()
            
            if len(l) == 0 or l[0] == '#':
                filteredLines.append(l)
                continue
            
            key = l[0:l.find("=")].strip()
            
            if not key.startswith(SourceCompiler.PARAM_PREFIX):
                filteredLines.append(l)
                continue
            
            key = key[len(SourceCompiler.PARAM_PREFIX):]
            
            if not parameters.has_key(key):
                filteredLines.append(l)
        
        wfile = open("%s/config/%s-node.properties" % (self._srcPath, self._dgenName), "w", SourceCompiler.BUFFER_SIZE)
        
        for l in filteredLines:
            print >> wfile, l
            
        for k, v in parameters.iteritems():
            print >> wfile, '%s%s = %s' % (SourceCompiler.PARAM_PREFIX, k, v)
        
        wfile.close()
            
    def compileBaseConfig(self, astRoot):
        try:
            os.makedirs("%s/cpp/config/base" % (self._srcPath))
        except OSError:
            pass
        
        wfile = open("%s/cpp/config/base/BaseGeneratorConfig.h" % (self._srcPath), "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '// auto-generatad base generator config C++ file'
        print >> wfile, ''
        print >> wfile, '#ifndef BASEGENERATORCONFIG_H_'
        print >> wfile, '#define BASEGENERATORCONFIG_H_'
        print >> wfile, ''
        print >> wfile, '#include "config/AbstractGeneratorConfig.h"'
        print >> wfile, ''
        print >> wfile, 'namespace Myriad {'
        print >> wfile, ''
        print >> wfile, 'class BaseGeneratorConfig: public AbstractGeneratorConfig'
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        print >> wfile, '    BaseGeneratorConfig(GeneratorPool& generatorPool) : AbstractGeneratorConfig(generatorPool)'
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, 'protected:'
        print >> wfile, ''
        print >> wfile, '    virtual void configurePartitioning()'
        print >> wfile, '    {'
        print >> wfile, '        // TODO: this piece of auto-generating code / Config API needs to be rewritten'
        print >> wfile, ''


        literalTransformer = LiteralTransfomer()
        nodeFilter = DepthFirstNodeFilter(filterType=AbstractCardinalityEstimatorNode)
        for cardinalityEstimator in nodeFilter.getAll(astRoot.getSpecification().getRecordSequences()):
            cardinalityEstimatorType = cardinalityEstimator.getAttribute("type")
            
            if cardinalityEstimatorType == 'linear_scale_estimator':
                print >> wfile, '        // setup linear scale estimator for %s' % (cardinalityEstimator.getParent().getAttribute("key"))
                print >> wfile, '        setString("partitioning.%s.base-cardinality", toString<%s>(%s));' % (cardinalityEstimator.getParent().getAttribute("key"), cardinalityEstimator.getArgument("base_cardinality").getAttribute("type").strip(), literalTransformer.transform(cardinalityEstimator.getArgument("base_cardinality"), None).pop())
                print >> wfile, '        computeLinearScalePartitioning("%s");' % (cardinalityEstimator.getParent().getAttribute("key"))
            elif cardinalityEstimatorType == 'const_estimator':
                print >> wfile, '        // setup constant estimator for %s' % (cardinalityEstimator.getParent().getAttribute("key"))
                print >> wfile, '        setString("partitioning.%s.cardinality", toString<%s>(%s));' % (cardinalityEstimator.getParent().getAttribute("key"), cardinalityEstimator.getArgument("cardinality").getAttribute("type").strip(), literalTransformer.transform(cardinalityEstimator.getArgument("cardinality"), None).pop())
                print >> wfile, '        computeFixedPartitioning("%s");' % (cardinalityEstimator.getParent().getAttribute("key"))
        
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    virtual void configureFunctions()'
        print >> wfile, '    {'
        print >> wfile, '        // register prototype functions'
        
        nodeFilter = DepthFirstNodeFilter(filterType=AbstractFunctionNode)
        for function in nodeFilter.getAll(astRoot.getSpecification().getFunctions()):
            argsCode = ArgumentTransformer.compileConstructorArguments(function, {'config': None})
            print >> wfile, '        function(new %(t)s(%(a)s));' % {'t': function.getConcreteType(), 'a': ', '.join(argsCode)}

        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    virtual void configureSets()'
        print >> wfile, '    {'
        print >> wfile, '        // bind string sets to config members with the bindStringSet method'
        
        literalTransformer = LiteralTransfomer()
        nodeFilter = DepthFirstNodeFilter(filterType=EnumSetNode)
        for enumSet in nodeFilter.getAll(astRoot):
            print >> wfile, '        enumSet(new MyriadEnumSet("%(n)s", %(p)s));' % {'n': enumSet.getAttribute("key"), 'p': literalTransformer.transform(enumSet.getArgument("path"), None).pop()}

        print >> wfile, '    }'
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '} // namespace Myriad'
        print >> wfile, ''
        print >> wfile, '#endif /* BASEGENERATORCONFIG_H_ */'
        
        wfile.close()
            
    def compileConfig(self, astRoot):
        try:
            os.makedirs("%s/cpp/config" % (self._srcPath))
        except OSError:
            pass
        
        sourcePath = "%s/cpp/config/GeneratorConfig.h" % (self._srcPath)
        
        if (os.path.isfile(sourcePath)):
            return
        
        wfile = open(sourcePath, "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '#ifndef GENERATORCONFIG_H_'
        print >> wfile, '#define GENERATORCONFIG_H_'
        print >> wfile, ''
        print >> wfile, '#include "config/base/BaseGeneratorConfig.h"'
        print >> wfile, ''
        print >> wfile, 'namespace Myriad {'
        print >> wfile, ''
        print >> wfile, 'class GeneratorConfig: public BaseGeneratorConfig'
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        print >> wfile, '    GeneratorConfig(GeneratorPool& generatorPool) : BaseGeneratorConfig(generatorPool)'
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, 'protected:'
        print >> wfile, ''
        print >> wfile, '    virtual void configurePartitioning()'
        print >> wfile, '    {'
        print >> wfile, '        BaseGeneratorConfig::configurePartitioning();'
        print >> wfile, '        // override or add partitioning config here'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    virtual void configureFunctions()'
        print >> wfile, '    {'
        print >> wfile, '        BaseGeneratorConfig::configureFunctions();'
        print >> wfile, '        // override or add functions here'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    virtual void configureSets()'
        print >> wfile, '    {'
        print >> wfile, '        BaseGeneratorConfig::configureSets();'
        print >> wfile, '        // override or add enumerated sets here'
        print >> wfile, '    }'
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '} // namespace Myriad'
        print >> wfile, ''
        print >> wfile, '#endif /* GENERATORCONFIG_H_ */'

        wfile.close()


class OutputCollectorCompiler(SourceCompiler):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        super(OutputCollectorCompiler, self).__init__(*args, **kwargs)
        
    def compileCode(self, astRoot):
        self._log.info("Compiling output collector C++ sources.")
        self.compileOutputCollector(astRoot)
            
    def compileOutputCollector(self, astRoot):
        try:
            os.makedirs("%s/cpp/io" % (self._srcPath))
        except OSError:
            pass
        
        sourcePath = "%s/cpp/io/OutputCollector.cpp" % (self._srcPath)
        
        if (not os.path.isfile(sourcePath)):
            wfile = open(sourcePath, "w", SourceCompiler.BUFFER_SIZE)
            
            print >> wfile, '#include "io/OutputCollector.h"'
            print >> wfile, '#include "record/AbstractRecord.h"'
            print >> wfile, ''
            print >> wfile, 'namespace Myriad {'
            print >> wfile, ''
            print >> wfile, '}  // namespace Myriad'
    
            wfile.close()


class RecordTypeCompiler(SourceCompiler):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        super(RecordTypeCompiler, self).__init__(*args, **kwargs)
        
    def compileCode(self, recordSequences):
        for recordSequence in recordSequences.getRecordSequences():
            self._log.info("Compiling record C++ sources for `%s`." % (recordSequence.getAttribute("key")))
            self.compileBaseRecordMeta(recordSequence)
            self.compileBaseRecordType(recordSequence)
            self.compileBaseRecordUtil(recordSequence)
            self.compileRecordMeta(recordSequence)
            self.compileRecordType(recordSequence)
            self.compileRecordUtil(recordSequence)
            
    def compileBaseRecordMeta(self, recordSequence):
        try:
            os.makedirs("%s/cpp/record/base" % (self._srcPath))
        except OSError:
            pass
        
        recordType = recordSequence.getRecordType()
        
        typeNameUS = recordType.getAttribute("key")
        typeNameCC = StringTransformer.ucFirst(StringTransformer.us2cc(typeNameUS))
        typeNameUC = StringTransformer.uc(typeNameCC)
        
        wfile = open("%s/cpp/record/base/Base%sMeta.h" % (self._srcPath, typeNameCC), "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '// auto-generatad C++ file for `%s`' % (recordType.getAttribute("key"))
        print >> wfile, ''
        print >> wfile, '#ifndef BASE%sMETA_H_' % (typeNameUC)
        print >> wfile, '#define BASE%sMETA_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#include "record/AbstractRecord.h"'
        print >> wfile, ''
        print >> wfile, 'namespace %s {' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '// forward declarations'
        print >> wfile, 'class %s;' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, 'class Base%(t)sMeta : public Myriad::RecordMeta<%(t)s>' % { 't': typeNameCC }
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        
        if recordType.hasEnumFields():
            print >> wfile, '    Base%(t)sMeta(const Myriad::EnumSetPool& enumSets) : ' % {'t': typeNameCC}
            print >> wfile, '        %s' % ', '.join([ '%(n)s(enumSets.get("%(r)s").values())' % {'n': field.getAttribute("name"), 'r': field.getAttribute("enumref")} for field in recordType.getEnumFields() ])
            print >> wfile, '    {'
            print >> wfile, '    }'
        else:
            print >> wfile, '    Base%(t)sMeta(const Myriad::EnumSetPool& enumSets)' % {'t': typeNameCC}
            print >> wfile, '    {'
            print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    // enum set refecences'
        for field in recordType.getEnumFields():
            print >> wfile, '    const vector<String>& %s;' % field.getAttribute("name")
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '} // namespace %s' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '#endif /* BASE%sMETA_H_ */' % (typeNameUC)

        wfile.close()
            
    def compileRecordMeta(self, recordSequence):
        try:
            os.makedirs("%s/cpp/record" % (self._srcPath))
        except OSError:
            pass
        
        recordType = recordSequence.getRecordType()
        
        typeNameUS = recordType.getAttribute("key")
        typeNameCC = StringTransformer.ucFirst(StringTransformer.us2cc(typeNameUS))
        typeNameUC = StringTransformer.uc(typeNameCC)
        
        sourcePath = "%s/cpp/record/%sMeta.h" % (self._srcPath, typeNameCC)
        
        if (os.path.isfile(sourcePath)):
            return
        
        wfile = open(sourcePath, "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '// auto-generatad C++ file for `%s`' % (recordType.getAttribute("key"))
        print >> wfile, ''
        print >> wfile, '#ifndef %sMETA_H_' % (typeNameUC)
        print >> wfile, '#define %sMETA_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#include "record/base/Base%sMeta.h"' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, 'namespace %s {' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, 'class %(t)sMeta : public Base%(t)sMeta' % {'t': typeNameCC}
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        print >> wfile, '    %(t)sMeta(const Myriad::EnumSetPool& enumSets) :' % {'t': typeNameCC}
        print >> wfile, '        Base%(t)sMeta(enumSets)' % {'t': typeNameCC}
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '} // namespace %s' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '#endif /* %sMETA_H_ */' % (typeNameUC)

        wfile.close()
            
    def compileBaseRecordType(self, recordSequence):
        try:
            os.makedirs("%s/cpp/record/base" % (self._srcPath))
        except OSError:
            pass
        
        recordType = recordSequence.getRecordType()
        
        typeNameUS = recordType.getAttribute("key")
        typeNameCC = StringTransformer.ucFirst(StringTransformer.us2cc(typeNameUS))
        typeNameUC = StringTransformer.uc(typeNameCC)
        
        wfile = open("%s/cpp/record/base/Base%s.h" % (self._srcPath, typeNameCC), "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '// auto-generatad C++ file for `%s`' % (recordType.getAttribute("key"))
        print >> wfile, ''
        print >> wfile, '#ifndef BASE%s_H_' % (typeNameUC)
        print >> wfile, '#define BASE%s_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#include "record/AbstractRecord.h"'
        print >> wfile, '#include "record/%sMeta.h"' % (typeNameCC)
        
        for referenceType in recordType.getReferenceTypes():
            print >> wfile, '#include "record/%s.h"' % (referenceType)
        
        print >> wfile, ''
        print >> wfile, 'namespace %s {' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, '// forward declarations'
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, ''
        print >> wfile, 'class %s;' % (typeNameCC)
        print >> wfile, 'class %sConfig;' % (typeNameCC)
        print >> wfile, 'class %sGenerator;' % (typeNameCC)
        print >> wfile, 'class %sSetterChain;' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, '// base record type'
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, ''
        print >> wfile, 'class Base%s: public Myriad::AbstractRecord' % (typeNameCC)
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        print >> wfile, '    Base%(t)s(const %(t)sMeta& meta) : ' % { 't': typeNameCC }
        for field in filter(lambda f: f.isVectorType(), recordType.getFields()):
            print >> wfile, '        _%s(%s),' % (field.getAttribute("name"), field.vectorTypeSize()) 
        print >> wfile, '        _meta(meta)'
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    const %(t)sMeta& meta() const' % { 't': typeNameCC }
        print >> wfile, '    {'
        print >> wfile, '        return _meta;'
        print >> wfile, '    }'
        print >> wfile, ''
        
        for field in filter(lambda f: not f.isImplicit(), recordType.getFields()):
            if field.isDerived():
                print >> wfile, '    virtual const %s %s() const = 0;' % (field.sourceType(), StringTransformer.us2cc(field.getAttribute("name")))
                print >> wfile, ''
            else:
                print >> wfile, '    void %s(const %s& v);' % (StringTransformer.us2cc(field.getAttribute("name")), field.sourceType())
                print >> wfile, '    const %s& %s() const;' % (field.sourceType(), StringTransformer.us2cc(field.getAttribute("name")))
            
                if field.coreType() == 'Enum':
                    if field.isVectorType():
                        print >> wfile, '    const String& %sEnumValue(size_t i) const;' % (StringTransformer.us2cc(field.getAttribute("name")))
                        print >> wfile, ''
                    else:
                        print >> wfile, '    const String& %sEnumValue() const;' % (StringTransformer.us2cc(field.getAttribute("name")))
                        print >> wfile, ''
                else:
                    print >> wfile, ''
        
        for reference in recordType.getReferences():
            print >> wfile, '    void %s(const AutoPtr<%s>& v);' % (StringTransformer.us2cc(reference.getAttribute("name")), reference.getAttribute("type"))
            print >> wfile, '    const AutoPtr<%s>& %s() const;' % (reference.getAttribute("type"), StringTransformer.us2cc(reference.getAttribute("name")))
            print >> wfile, ''
        
        print >> wfile, 'protected:'

        if recordType.hasFields():
            print >> wfile, ''
            print >> wfile, '    // fields'
        for field in filter(lambda f: not f.isImplicit() and not f.isDerived(), recordType.getFields()):
            print >> wfile, '    %s _%s;' % (field.sourceType(), field.getAttribute("name")) 
        
        if recordType.hasReferences():
            print >> wfile, ''
            print >> wfile, '    // references'
        for reference in recordType.getReferences():
            print >> wfile, '    AutoPtr<%s> _%s;' % (reference.getAttribute("type"), reference.getAttribute("name")) 
        
        print >> wfile, ''
        print >> wfile, '    // meta'
        print >> wfile, '    const %sMeta& _meta;' % (typeNameCC)
        print >> wfile, '};'
        print >> wfile, ''
        
        for field in filter(lambda f: not f.isImplicit() and not f.isDerived(), recordType.getFields()):
            
            if field.isVectorType():
                print >> wfile, 'inline void Base%s::%s(const %s& v)' % (typeNameCC, StringTransformer.us2cc(field.getAttribute("name")), field.sourceType())
                print >> wfile, '{'
                print >> wfile, '    _%s = v;' % (field.getAttribute("name"))
                print >> wfile, '    _%s.resize(v.size());' % (field.getAttribute("name"))
                print >> wfile, '}'
                print >> wfile, ''
            else:
                print >> wfile, 'inline void Base%s::%s(const %s& v)' % (typeNameCC, StringTransformer.us2cc(field.getAttribute("name")), field.sourceType())
                print >> wfile, '{'
                print >> wfile, '    _%s = v;' % (field.getAttribute("name"))
                print >> wfile, '}'
                print >> wfile, ''
            print >> wfile, 'inline const %s& Base%s::%s() const' % (field.sourceType(), typeNameCC, StringTransformer.us2cc(field.getAttribute("name")))
            print >> wfile, '{'
            print >> wfile, '    return _%s;' % (field.getAttribute("name"))
            print >> wfile, '}'
            print >> wfile, ''
            
            if field.coreType() == 'Enum':
                if field.isVectorType():
                    print >> wfile, 'inline const String& Base%s::%sEnumValue(size_t i) const' % (typeNameCC, StringTransformer.us2cc(field.getAttribute("name")))
                    print >> wfile, '{'
                    print >> wfile, '    if (_%(n)s[i] == Myriad::nullValue<Enum>())' % {'n': field.getAttribute("name")}
                    print >> wfile, '    {'
                    print >> wfile, '        return Myriad::nullValue<String>();'
                    print >> wfile, '    }'
                    print >> wfile, '    else'
                    print >> wfile, '    {'
                    print >> wfile, '        return _meta.%(n)s[_%(n)s[i]];' % {'n': field.getAttribute("name")}
                    print >> wfile, '    }'
                    print >> wfile, '}'
                    print >> wfile, ''
                else:
                    print >> wfile, 'inline const String& Base%s::%sEnumValue() const' % (typeNameCC, StringTransformer.us2cc(field.getAttribute("name")))
                    print >> wfile, '{'
                    print >> wfile, '    if (_%(n)s == Myriad::nullValue<Enum>())' % {'n': field.getAttribute("name")}
                    print >> wfile, '    {'
                    print >> wfile, '        return Myriad::nullValue<String>();'
                    print >> wfile, '    }'
                    print >> wfile, '    else'
                    print >> wfile, '    {'
                    print >> wfile, '        return _meta.%(n)s[_%(n)s];' % {'n': field.getAttribute("name")}
                    print >> wfile, '    }'
                    print >> wfile, '}'
                    print >> wfile, ''
        
        for reference in recordType.getReferences():
            print >> wfile, 'inline void Base%s::%s(const AutoPtr<%s>& v)' % (typeNameCC, StringTransformer.us2cc(reference.getAttribute("name")), reference.getAttribute("type"))
            print >> wfile, '{'
            print >> wfile, '    _%s = v;' % (reference.getAttribute("name"))
            print >> wfile, '}'
            print >> wfile, ''
            print >> wfile, 'inline const AutoPtr<%s>& Base%s::%s() const' % (reference.getAttribute("type"), typeNameCC, StringTransformer.us2cc(reference.getAttribute("name")))
            print >> wfile, '{'
            print >> wfile, '    return _%s;' % (reference.getAttribute("name"))
            print >> wfile, '}'
            print >> wfile, ''
        print >> wfile, '} // namespace %s' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, 'namespace Myriad {'
        print >> wfile, ''
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, '// record traits specialization'
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, ''
        
        fieldConstants = []
        fieldConstants.extend([StringTransformer.uc(field.getAttribute("name")) for field in recordType.getFields()])
        fieldConstants.extend([StringTransformer.uc(field.getAttribute("name")) for field in recordType.getReferences()])

        print >> wfile, 'template<>'
        print >> wfile, 'struct RecordTraits<%s::%s>' % (self._args.dgen_ns, typeNameCC)
        print >> wfile, '{'
        print >> wfile, '    typedef %s::%sMeta MetaType;' % (self._args.dgen_ns, typeNameCC)
        print >> wfile, '    typedef %s::%sGenerator GeneratorType;' % (self._args.dgen_ns, typeNameCC)
        print >> wfile, '    typedef %s::%sSetterChain SetterChainType;' % (self._args.dgen_ns, typeNameCC)
        print >> wfile, '    typedef RecordFactory<%s::%s> FactoryType;' % (self._args.dgen_ns, typeNameCC)
        print >> wfile, ''
        print >> wfile, '    enum Field { UNKNOWN, %s };' % ', '.join(fieldConstants)
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, '// serialize method specialization'
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, ''
        print >> wfile, 'template<>' 
        print >> wfile, 'inline void AbstractOutputCollector<%(ns)s::Base%(t)s>::serialize(std::ostream& out, const %(ns)s::Base%(t)s& record)' % {'ns': self._args.dgen_ns, 't': typeNameCC}
        print >> wfile, '{'
        
        outputFormatter = recordSequence.getOutputFormatter()
        
        if not recordSequence.hasOutputFormatter():
            # do nothing if the specification does not provide an output format
            pass
        elif outputFormatter.getAttribute("type") == "empty":
            # do nothing if the specification expects an empty output format
            pass
        elif outputFormatter.getAttribute("type") == "csv":
            # produce the CSV output
            outputFormatDelimiter = outputFormatter.getArgument("delimiter").getAttribute("value")
            outputFormatIsQuoted = str(outputFormatter.getArgument("quoted").getAttribute("value").lower() == 'true').lower()
            outputFormatFields = outputFormatter.getArgument("field")
            for fieldRef in outputFormatFields.getAll():
                field = fieldRef.getFieldRef()
                fieldType = field.getAttribute("type")
                fieldName = field.getAttribute("name")
                
                if fieldType == "Enum":
                    print >> wfile, '    write(out, %s, %s);' % ("record." + StringTransformer.us2cc(fieldName) + "EnumValue()", outputFormatIsQuoted)
                elif fieldType == 'String' or (field.isVectorType() and field.coreType() == 'Char'):
                    print >> wfile, '    write(out, %s, %s);' % ("record." + StringTransformer.us2cc(fieldName) + "()", outputFormatIsQuoted)
                else:
                    print >> wfile, '    write(out, %s, false);' % ("record." + StringTransformer.us2cc(fieldName) + "()")
                print >> wfile, '    out << \'%s\';' % (outputFormatDelimiter)
            print >> wfile, '    out << \'\\n\';' 
            
        print >> wfile, '}'
        print >> wfile, ''
        print >> wfile, '} // namespace Myriad'
        print >> wfile, ''
        print >> wfile, '#endif /* BASE%s_H_ */' % (typeNameUC)

        wfile.close()
    
    def compileRecordType(self, recordSequence):
        try:
            os.makedirs("%s/cpp/record" % (self._srcPath))
        except OSError:
            pass
        
        recordType = recordSequence.getRecordType()
        
        typeNameUS = recordType.getAttribute("key")
        typeNameCC = StringTransformer.ucFirst(StringTransformer.us2cc(typeNameUS))
        typeNameUC = StringTransformer.uc(typeNameCC)
        
        sourcePath = "%s/cpp/record/%s.h" % (self._srcPath, typeNameCC)
        
        if (os.path.isfile(sourcePath)):
            return
        
        wfile = open(sourcePath, "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '#ifndef %s_H_' % (typeNameUC)
        print >> wfile, '#define %s_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#include "record/base/Base%s.h"' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, 'namespace %s {' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, '// record type'
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, ''
        print >> wfile, 'class %(t)s: public Base%(t)s' % {'t': typeNameCC}
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        print >> wfile, '    %(t)s(const %(t)sMeta& meta)' % {'t': typeNameCC}
        print >> wfile, '        : Base%s(meta)' % (typeNameCC)
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        
        for field in filter(lambda f: f.isDerived(), recordType.getFields()):
            fieldType = field.getAttribute("type")
            fieldName = field.getAttribute("name")

            print >> wfile, '    virtual const %s %s() const' % (field.sourceType(), StringTransformer.us2cc(field.getAttribute("name")))
            print >> wfile, '    {'
            print >> wfile, '        return Myriad::nullValue<%s>();' % (field.sourceType())
            print >> wfile, '    }'
            print >> wfile, ''

        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '} // namespace %s' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, 'namespace Myriad {'
        print >> wfile, ''
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, '// record serialize method specialization'
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, ''
        print >> wfile, 'template<>'
        print >> wfile, 'inline void AbstractOutputCollector<%(ns)s::%(t)s>::serialize(std::ostream& out, const %(ns)s::%(t)s& record)' % {'ns': self._args.dgen_ns, 't': typeNameCC}
        print >> wfile, '{'
        print >> wfile, '    AbstractOutputCollector<%(ns)s::Base%(t)s>::serialize(out, record);' % {'ns': self._args.dgen_ns, 't': typeNameCC}
        print >> wfile, '}'
        print >> wfile, ''
        print >> wfile, '} // namespace Myriad'
        print >> wfile, ''
        print >> wfile, '#endif /* %s_H_ */' % (typeNameUC)

        wfile.close()
            
    def compileBaseRecordUtil(self, recordSequence):
        try:
            os.makedirs("%s/cpp/record/base" % (self._srcPath))
        except OSError:
            pass
        
        recordType = recordSequence.getRecordType()
        
        typeNameUS = recordType.getAttribute("key")
        typeNameCC = StringTransformer.ucFirst(StringTransformer.us2cc(typeNameUS))
        typeNameUC = StringTransformer.uc(typeNameCC)
        
        wfile = open("%s/cpp/record/base/Base%sUtil.h" % (self._srcPath, typeNameCC), "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '// auto-generatad C++ file for `%s`' % (recordType.getAttribute("key"))
        print >> wfile, ''
        print >> wfile, '#ifndef BASE%sUTIL_H_' % (typeNameUC)
        print >> wfile, '#define BASE%sUTIL_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#include "record/%s.h"' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, 'namespace Myriad {'
        print >> wfile, ''
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, '// record field inspection structures'
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        
        for field in filter(lambda f: not f.isImplicit(), recordType.getFields()):
            fieldType = field.sourceType()
            fieldName = field.getAttribute("name")
            parameters = {'ns': self._args.dgen_ns, 't': typeNameCC, 'k': StringTransformer.uc(fieldName), 'f': StringTransformer.us2cc(fieldName)}

            print >> wfile, ''
            print >> wfile, '// %s' % (fieldName)
            print >> wfile, 'template<>'
            print >> wfile, 'struct RecordFieldTraits<RecordTraits<%(ns)s::%(t)s>::%(k)s, %(ns)s::%(t)s>' % parameters
            print >> wfile, '{'
            print >> wfile, '    typedef %s FieldType;' % (fieldType)
            if field.isDerived():
                print >> wfile, '    typedef MethodTraits<%(ns)s::%(t)s, FieldType>::ValSetter FieldSetterType;' % parameters
                print >> wfile, '    typedef MethodTraits<%(ns)s::%(t)s, FieldType>::ValGetter FieldGetterType;' % parameters
            else:
                print >> wfile, '    typedef MethodTraits<%(ns)s::%(t)s, FieldType>::RefSetter FieldSetterType;' % parameters
                print >> wfile, '    typedef MethodTraits<%(ns)s::%(t)s, FieldType>::RefGetter FieldGetterType;' % parameters
            print >> wfile, ''
            print >> wfile, '    static inline FieldSetterType setter()'
            print >> wfile, '    {'
            if field.isDerived():
                print >> wfile, '        throw RuntimeException("Derived field `%(f)s` does not have a setter.");' % parameters
            else:
                print >> wfile, '        return static_cast<FieldSetterType>(&%(ns)s::%(t)s::%(f)s);' % parameters
            print >> wfile, '    }'
            print >> wfile, ''
            print >> wfile, '    static inline FieldGetterType getter()'
            print >> wfile, '    {'
            print >> wfile, '        return static_cast<FieldGetterType>(&%(ns)s::%(t)s::%(f)s);' % parameters
            print >> wfile, '    }'
            print >> wfile, '};'
        
        for field in recordType.getReferences():
            fieldType = field.getAttribute("type")
            fieldName = field.getAttribute("name")
            parameters = {'ns': self._args.dgen_ns, 't': typeNameCC, 'k': StringTransformer.uc(fieldName), 'f': StringTransformer.us2cc(fieldName)}
            
            print >> wfile, ''
            print >> wfile, '// %s' % (fieldName)
            print >> wfile, 'template<>'
            print >> wfile, 'struct RecordFieldTraits<RecordTraits<%(ns)s::%(t)s>::%(k)s, %(ns)s::%(t)s>' % parameters
            print >> wfile, '{'
            print >> wfile, '    typedef %s::%s FieldType;' % (self._args.dgen_ns, fieldType)
            print >> wfile, '    // record field getter / setter types'
            print >> wfile, '    typedef MethodTraits<%(ns)s::%(t)s, AutoPtr<FieldType> >::RefSetter FieldSetterType;' % parameters
            print >> wfile, '    typedef MethodTraits<%(ns)s::%(t)s, AutoPtr<FieldType> >::RefGetter FieldGetterType;' % parameters
            print >> wfile, ''
            print >> wfile, '    static inline FieldSetterType setter()'
            print >> wfile, '    {'
            print >> wfile, '        return static_cast<FieldSetterType>(&%(ns)s::%(t)s::%(f)s);' % parameters
            print >> wfile, '    }'
            print >> wfile, ''
            print >> wfile, '    static inline FieldGetterType getter()'
            print >> wfile, '    {'
            print >> wfile, '        return static_cast<FieldGetterType>(&%(ns)s::%(t)s::%(f)s);' % parameters
            print >> wfile, '    }'
            print >> wfile, '};'
        
        print >> wfile, ''
        print >> wfile, '} // namespace Myriad'
        print >> wfile, ''
        print >> wfile, '#endif /* BASE%sUTIL_H_ */' % (typeNameUC)

        wfile.close()
    
    def compileRecordUtil(self, recordSequence):
        try:
            os.makedirs("%s/cpp/record" % (self._srcPath))
        except OSError:
            pass
        
        recordType = recordSequence.getRecordType()
        
        typeNameUS = recordType.getAttribute("key")
        typeNameCC = StringTransformer.ucFirst(StringTransformer.us2cc(typeNameUS))
        typeNameUC = StringTransformer.uc(typeNameCC)
        
        sourcePath = "%s/cpp/record/%sUtil.h" % (self._srcPath, typeNameCC)
        
        if (os.path.isfile(sourcePath)):
            return
        
        wfile = open(sourcePath, "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '#ifndef %sUTIL_H_' % (typeNameUC)
        print >> wfile, '#define %sUTIL_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#include "record/base/Base%sUtil.h"' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, 'namespace Myriad {'
        print >> wfile, ''
        print >> wfile, '// put your extra RecordFieldTraits specializations here'
        print >> wfile, ''
        print >> wfile, '} // namespace Myriad'
        print >> wfile, ''
        print >> wfile, '#endif /* %sUTIL_H_ */' % (typeNameUC)


class SetterChainCompiler(SourceCompiler):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        super(SetterChainCompiler, self).__init__(*args, **kwargs)
        
    def compileCode(self, recordSequences):
        for recordSequence in recordSequences.getRecordSequences():
            self._log.info("Compiling setter chain C++ sources for `%s`." % (recordSequence.getAttribute("key")))
            self.compileBaseSetterChain(recordSequence)
            self.compileSetterChain(recordSequence)
            
    def compileBaseSetterChain(self, recordSequence):
        try:
            os.makedirs("%s/cpp/runtime/setter/base" % (self._srcPath))
        except OSError:
            pass
        
        if isinstance(recordSequence, RandomSequenceNode):
            self.__compileBaseRandomSequenceSetterChain(recordSequence)
        else:
            self._log.warning("unsupported generator type for sequence `%s`" % (recordSequence.getAttribute("key")))

    def __compileBaseRandomSequenceSetterChain(self, recordSequence):
        typeNameUS = recordSequence.getAttribute("key")
        typeNameCC = StringTransformer.ucFirst(StringTransformer.us2cc(typeNameUS))
        typeNameUC = StringTransformer.uc(typeNameCC)
        
        sourcePath = "%s/cpp/runtime/setter/base/Base%sSetterChain.h" % (self._srcPath, typeNameCC)
        
        wfile = open(sourcePath, "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '// auto-generatad C++ setter chain for `%s`' % (typeNameUS)
        print >> wfile, ''
        print >> wfile, '#ifndef BASE%sSETTERCHAIN_H_' % (typeNameUC)
        print >> wfile, '#define BASE%sSETTERCHAIN_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#include "config/GeneratorConfig.h"'
        print >> wfile, '#include "runtime/setter/SetterChain.h"'
        
        for referenceType in recordSequence.getRecordType().getReferenceTypes():
            print >> wfile, '#include "generator/%sGenerator.h"' % (referenceType)
            
        print >> wfile, '#include "record/%s.h"' % (typeNameCC)
        print >> wfile, '#include "record/%sUtil.h"' % (typeNameCC)
        
        for componentPath in recordSequence.getSetterChain().getComponentIncludePaths():
            print >> wfile, '#include "%s"' % (componentPath)
        
        print >> wfile, ''
        print >> wfile, 'namespace %s {' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, '// SetterChain specialization (base class)'
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, ''
        print >> wfile, '/**'
        print >> wfile, ' * SetterChain specialization for User.'
        print >> wfile, ' */'
        print >> wfile, 'class Base%(t)sSetterChain : public Myriad::SetterChain<%(t)s>' % {'t': typeNameCC}
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        
        print >> wfile, '    // runtime component typedefs'
        for setter in recordSequence.getSetterChain().getAll():
            print >> wfile, '    // runtime components for setter `%s`' % (setter.getAttribute('key'))
            nodeFilter = DepthFirstNodeFilter(filterType=AbstractRuntimeComponentNode)
            for node in nodeFilter.getAll(setter):
                print >> wfile, '    typedef %s %s;' % (node.getConcreteType(), node.getAttribute("type_alias"))
            
        print >> wfile, ''
        print >> wfile, '    Base%sSetterChain(Myriad::BaseSetterChain::OperationMode& opMode, Myriad::RandomStream& random, Myriad::GeneratorConfig& config) :' % (typeNameCC)
        print >> wfile, '        Myriad::SetterChain<%s>(opMode, random),' % (typeNameCC)
        
        
        print >> wfile, '        _sequenceCardinality(config.cardinality("%s")),' % (typeNameUS)
        nodeFilter = DepthFirstNodeFilter(filterType=AbstractRuntimeComponentNode)
        for node in nodeFilter.getAll(recordSequence.getSetterChain()):
            argsCode = ArgumentTransformer.compileConstructorArguments(node, {'config': 'config'})
            print >> wfile, '        %s(%s),' % (node.getAttribute("var_name"), ', '.join(argsCode))
        
        print >> wfile, '        _logger(Logger::get("%s.setter.chain"))' % (typeNameUS)
            
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    virtual ~Base%sSetterChain()' % (typeNameCC)
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    /**'
        print >> wfile, '     * Applies the setter chain to the given record instance.'
        print >> wfile, '     */'
        print >> wfile, '    void operator()(AutoPtr<%s> recordPtr) const' % (typeNameCC)
        print >> wfile, '    {'
        print >> wfile, '        ensurePosition(recordPtr->genID());'
        print >> wfile, ''
        print >> wfile, '        Base%(t)sSetterChain* me = const_cast<Base%(t)sSetterChain*>(this);' % {'t': typeNameCC}
        print >> wfile, ''
        print >> wfile, '        // apply setter chain'

        for setter in recordSequence.getSetterChain().getAll():
            print >> wfile, '        me->%s(recordPtr, me->_random);' % (setter.getAttribute("var_name"))
        
        print >> wfile, '    }'
        print >> wfile, ''
        
        print >> wfile, '    /**'
        print >> wfile, '     * Predicate filter function.'
        print >> wfile, '     */'
        print >> wfile, '    virtual Myriad::Interval<I64u> filter(const Myriad::EqualityPredicate<%(t)s>& predicate)' % {'t': typeNameCC}
        print >> wfile, '    {'
        print >> wfile, '        Myriad::Interval<I64u> result(0, _sequenceCardinality);'
        print >> wfile, ''
        print >> wfile, '        // explicitly mimic inverse setter logic for gen_id'
        print >> wfile, '        if (predicate.bound(Myriad::RecordTraits<%(t)s>::GEN_ID))' % {'t': typeNameCC}
        print >> wfile, '        {'
        print >> wfile, '            const AutoPtr<%(t)s>& valueHolder = predicate.valueHolder();' % {'t': typeNameCC}
        print >> wfile, '            result.intersect(Myriad::Interval<I64u>(valueHolder->genID(), valueHolder->genID()+1));' % {'t': typeNameCC}
        print >> wfile, '        }'
        print >> wfile, ''
        print >> wfile, '        // apply inverse setter chain, setters are applied in the same order'
        for setter in recordSequence.getSetterChain().getFieldSetters():
            print >> wfile, '        %s.filterRange(predicate, result);' % (setter.getAttribute("var_name"))
        print >> wfile, ''
        print >> wfile, '        return result;'
        print >> wfile, '    }'
        print >> wfile, ''

        if recordSequence.getSetterChain().settersCount() > 0:
            nodeFilter = DepthFirstNodeFilter(filterType=CallbackValueProviderNode)
            for node in nodeFilter.getAll(recordSequence.getSetterChain()):
                print >> wfile, '    virtual %s %s(const AutoPtr<%s>& recordPtr, Myriad::RandomStream& random) = 0;' % (node.getValueType(), node.getArgument('name').getAttribute('value'), typeNameCC)
                print >> wfile, ''
                                
        print >> wfile, 'protected:'
        print >> wfile, ''
        
        print >> wfile, '    // cardinality'
        print >> wfile, '    I64u _sequenceCardinality;'
        print >> wfile, ''
        for setter in recordSequence.getSetterChain().getAll():
            print >> wfile, '    // runtime components for setter `%s`' % (setter.getAttribute('key'))
            nodeFilter = DepthFirstNodeFilter(filterType=AbstractRuntimeComponentNode)
            for node in nodeFilter.getAll(setter):
                print >> wfile, '    %s %s;' % (node.getAttribute("type_alias"), node.getAttribute("var_name"))
            print >> wfile, ''
        
        print >> wfile, '    // Logger instance.'
        print >> wfile, '    Logger& _logger;'
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '} // namespace %s' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '#endif /* BASE%sSETTERCHAIN_H_ */' % (typeNameUC)

        wfile.close()
            
    def compileSetterChain(self, recordSequence):
        try:
            os.makedirs("%s/cpp/generator" % (self._srcPath))
        except OSError:
            pass
        
        if isinstance(recordSequence, RandomSequenceNode):
            self.__compileRandomSequenceSetterChain(recordSequence)
        else:
            self._log.warning("unsupported generator type for sequence `%s`" % (recordSequence.getAttribute("key")))

    def __compileRandomSequenceSetterChain(self, recordSequence):
        typeNameUS = recordSequence.getAttribute("key")
        typeNameCC = StringTransformer.ucFirst(StringTransformer.us2cc(typeNameUS))
        typeNameUC = StringTransformer.uc(typeNameCC)
        
        sourcePath = "%s/cpp/runtime/setter/%sSetterChain.h" % (self._srcPath, typeNameCC)
        
        if (os.path.isfile(sourcePath)):
            return
        
        wfile = open(sourcePath, "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '#ifndef %sSETTERCHAIN_H_' % (typeNameUC)
        print >> wfile, '#define %sSETTERCHAIN_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#include "runtime/setter/base/Base%sSetterChain.h"' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, 'namespace %s {' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, '// SetterChain specialization'
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, ''
        print >> wfile, 'class %(t)sSetterChain : public Base%(t)sSetterChain' % {'t': typeNameCC}
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        print >> wfile, '    %sSetterChain(Myriad::BaseSetterChain::OperationMode& opMode, Myriad::RandomStream& random, Myriad::GeneratorConfig& config) :' % (typeNameCC)
        print >> wfile, '        Base%sSetterChain(opMode, random, config)' % (typeNameCC)
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    virtual ~%sSetterChain()' % (typeNameCC)
        print >> wfile, '    {'
        print >> wfile, '    }'
        
        if recordSequence.getSetterChain().settersCount() > 0:
            nodeFilter = DepthFirstNodeFilter(filterType=CallbackValueProviderNode)
            for node in nodeFilter.getAll(recordSequence.getSetterChain()):
                print >> wfile, ''
                print >> wfile, '    virtual %s %s(const AutoPtr<%s>& recordPtr, Myriad::RandomStream& random)' % (node.getValueType(), node.getArgument('name').getAttribute('value'), typeNameCC)
                print >> wfile, '    {'
                if (node.getArgument('type').getAttribute('value').lower() == 'string'):
                    print >> wfile, '        return "";'
                else:
                    print >> wfile, '        return Myriad::nullValue<%s>();' % (node.getArgument('type').getAttribute('value'))
                print >> wfile, '    }'
        
        
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '} // namespace %s' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '#endif /* BASE%sGENERATOR_H_ */' % (typeNameUC)

        wfile.close()


class AbstractSequenceGeneratorCompiler(SourceCompiler):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        super(AbstractSequenceGeneratorCompiler, self).__init__(*args, **kwargs)
        
    def compileCode(self, recordSequences):
        for recordSequence in recordSequences.getRecordSequences():
            self._log.info("Compiling generator C++ sources for `%s`." % (recordSequence.getAttribute("key")))
            self.compileBaseSetterChain(recordSequence)
            self.compileSetterChain(recordSequence)
            
    def compileBaseSetterChain(self, recordSequence):
        try:
            os.makedirs("%s/cpp/generator/base" % (self._srcPath))
        except OSError:
            pass
        
        if isinstance(recordSequence, RandomSequenceNode):
            self.__compileBaseRandomSequenceGenerator(recordSequence)
        else:
            self._log.warning("unsupported generator type for sequence `%s`" % (recordSequence.getAttribute("key")))

    def __compileBaseRandomSequenceGenerator(self, recordSequence):
        typeNameUS = recordSequence.getAttribute("key")
        typeNameCC = StringTransformer.ucFirst(StringTransformer.us2cc(typeNameUS))
        typeNameUC = StringTransformer.uc(typeNameCC)
        
        sourcePath = "%s/cpp/generator/base/Base%sGenerator.h" % (self._srcPath, typeNameCC)
        
        wfile = open(sourcePath, "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '// auto-generatad C++ generator for `%s`' % (typeNameUS)
        print >> wfile, ''
        print >> wfile, '#ifndef BASE%sGENERATOR_H_' % (typeNameUC)
        print >> wfile, '#define BASE%sGENERATOR_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#include "generator/RandomSequenceGenerator.h"'
        print >> wfile, '#include "runtime/setter/%sSetterChain.h"' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, 'namespace %s {' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, '// AbstractSequenceGenerator specialization (base class)'
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, ''
        print >> wfile, 'class Base%(t)sGenerator: public Myriad::RandomSequenceGenerator<%(t)s>' % {'t': typeNameCC}
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        print >> wfile, '    Base%sGenerator(const string& name, Myriad::GeneratorConfig& config, NotificationCenter& notificationCenter) :' % (typeNameCC)
        print >> wfile, '        Myriad::RandomSequenceGenerator<%s>(name, config, notificationCenter)' % (typeNameCC)
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    void prepare(Stage stage, const Myriad::GeneratorPool& pool)'
        print >> wfile, '    {'
        print >> wfile, '        // call generator implementation'
        print >> wfile, '        Myriad::RandomSequenceGenerator<%s>::prepare(stage, pool);' % (typeNameCC)
        
        if recordSequence.hasSequenceIterator():
            sequenceIterator = recordSequence.getSequenceIterator()
            sequenceIteratorArgsCode = ArgumentTransformer.compileConstructorArguments(sequenceIterator, {'config': '_config'})
            
            print >> wfile, ''
            print >> wfile, '        if (stage.name() == name())'
            print >> wfile, '        {'
            print >> wfile, '            registerTask(new %s (%s));' % (sequenceIterator.getConcreteType(), ', '.join(sequenceIteratorArgsCode))
            print >> wfile, '        }'
            
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    %(t)sSetterChain setterChain(Myriad::BaseSetterChain::OperationMode opMode, Myriad::RandomStream& random)' % {'t': typeNameCC}
        print >> wfile, '    {'
        print >> wfile, '        return %sSetterChain(opMode, random, _config);' % (typeNameCC)
        print >> wfile, '    }'
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '} // namespace %s' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '#endif /* BASE%sGENERATOR_H_ */' % (typeNameUC)

        wfile.close()
            
    def compileSetterChain(self, recordSequence):
        try:
            os.makedirs("%s/cpp/generator" % (self._srcPath))
        except OSError:
            pass
        
        if isinstance(recordSequence, RandomSequenceNode):
            self.__compileRandomSequenceGenerator(recordSequence)
        else:
            self._log.warning("unsupported generator type for sequence `%s`" % (recordSequence.getAttribute("key")))

    def __compileRandomSequenceGenerator(self, recordSequence):
        typeNameUS = recordSequence.getAttribute("key")
        typeNameCC = StringTransformer.ucFirst(StringTransformer.us2cc(typeNameUS))
        typeNameUC = StringTransformer.uc(typeNameCC)
        
        sourcePath = "%s/cpp/generator/%sGenerator.h" % (self._srcPath, typeNameCC)
        
        if (os.path.isfile(sourcePath)):
            return
        
        wfile = open(sourcePath, "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '#ifndef %sGENERATOR_H_' % (typeNameUC)
        print >> wfile, '#define %sGENERATOR_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#include "generator/base/Base%sGenerator.h"' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, 'namespace %s {' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, '// AbstractSequenceGenerator specialization'
        print >> wfile, '// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~'
        print >> wfile, ''
        print >> wfile, 'class %(t)sGenerator: public Base%(t)sGenerator' % {'t': typeNameCC}
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        print >> wfile, '    typedef Myriad::RecordTraits<%s>::SetterChainType SetterChainType;' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, '    %sGenerator(const string& name, Myriad::GeneratorConfig& config, NotificationCenter& notificationCenter) :' % (typeNameCC)
        print >> wfile, '        Base%sGenerator(name, config, notificationCenter)' % (typeNameCC)
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '} // namespace %s' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '#endif /* BASE%sGENERATOR_H_ */' % (typeNameUC)

        wfile.close()