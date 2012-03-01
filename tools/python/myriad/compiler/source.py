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
 
Created on Feb 25, 2012

@author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
'''

import logging
import os
import re

from myriad.compiler.ast import RandomSequenceNode
from myriad.compiler.ast import LiteralArgumentNode
from myriad.compiler.ast import ResolvedFunctionRefArgumentNode
from myriad.compiler.ast import ResolvedFieldRefArgumentNode
from myriad.compiler.ast import ResolvedHydratorRefArgumentNode
from myriad.compiler.ast import StringSetRefArgumentNode
from myriad.compiler.ast import DepthFirstNodeFilter
from myriad.compiler.ast import StringSetNode
from myriad.util.stringutil import StringTransformer

class SourceCompiler(object):
    '''
    classdocs
    '''
    
    BUFFER_SIZE = 512
    
    _args = None
    _srcPath = None
    
    _log = None
    
    _expr_pattern = re.compile('%([\w.]+)%')
    _param_pattern = re.compile('^\${(.+)}$')
    
    def __init__(self, args):
        '''
        Constructor
        '''
        self._args = args
        self._srcPath = "%s/../../src/cpp" % (args.base_path)
        
        self._log = logging.getLogger("source.compiler")
        
    def _argumentCode(self, argumentNode):
        if isinstance(argumentNode, LiteralArgumentNode):
            attributeType = argumentNode.getAttribute("type").strip()
            attributeValue = argumentNode.getAttribute("value").strip()
            
            m = self._expr_pattern.match(attributeValue)
            if (m):
                return 'config.parameter<%s>("%s")' % (attributeType, m.group(1))
            
            m = self._param_pattern.match(attributeValue)
            if (m):
                exprExpandedParams = self._expr_pattern.sub(lambda m: 'config.parameter<Decimal>("%s")' % (m.group(1)), attributeValue)
                return "static_cast<%s>(%s)" % (attributeType, exprExpandedParams[3:-2])
            
            else:
                if type == "String":
                    return '"%s"' % (attributeValue)
                else:
                    return '%s' % (attributeValue)
        
        elif isinstance(argumentNode, ResolvedFunctionRefArgumentNode):
            functionType = argumentNode.getAttribute("type")
            functionName = argumentNode.getAttribute("ref")
            return 'config.func<%s> ("%s")' % (functionType, functionName)
        
        elif isinstance(argumentNode, ResolvedFieldRefArgumentNode):
            typeName = StringTransformer.us2ccAll(argumentNode.getRecordTypeRef().getAttribute("key"))
            fieldAccessMethodName = StringTransformer.us2cc(argumentNode.getFieldRef().getAttribute("name"))
            return '&%s::%s' % (typeName, fieldAccessMethodName)
        
        elif isinstance(argumentNode, ResolvedHydratorRefArgumentNode):
            hydratorVarName = StringTransformer.us2cc(argumentNode.getHydratorRef().getAttribute("key"))
            return '_%s' % (hydratorVarName)
        
        elif isinstance(argumentNode, StringSetRefArgumentNode):
            stringSetKey = StringTransformer.us2cc(argumentNode.getAttribute("ref"))
            return 'config.stringSet("%s")' % (stringSetKey)
        
        else:
            
            return "NULL /* unknown */"


class ConfigCompiler(SourceCompiler):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        super(ConfigCompiler, self).__init__(*args, **kwargs)
        
    def compile(self, astRoot):
        self._log.warning("compiling generator config C++ sources")
        self.compileBaseConfig(astRoot)
        self.compileConfig(astRoot)
            
    def compileBaseConfig(self, astRoot):
        try:
            os.makedirs("%s/config/base" % (self._srcPath))
        except OSError:
            pass
        
        wfile = open("%s/config/base/BaseGeneratorConfig.h" % (self._srcPath), "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '// auto-generatad base generatoc config C++ file'
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
        print >> wfile, '    void configureSets(const AutoPtr<XML::Document>& doc)'
        print >> wfile, '    {'
        print >> wfile, '        // bind string sets to config members with the bindStringSet method'
        
        nodeFilter = DepthFirstNodeFilter(filterType=StringSetNode)
        for stringSet in nodeFilter.getAll(astRoot):
            print >> wfile, '        bindStringSet(doc, "%(n)s", _boundStringSets["%(n)s"]);' % {'n': stringSet.getAttribute("key")}
        
        
        print >> wfile, ''
        print >> wfile, '        // bind object sets to config members with the bindObjectSet method'
        print >> wfile, '    }'
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '} // namespace Myriad'
        print >> wfile, ''
        print >> wfile, '#endif /* BASEGENERATORCONFIG_H_ */'
        
        wfile.close()
            
    def compileConfig(self, astRoot):
        try:
            os.makedirs("%s/config" % (self._srcPath))
        except OSError:
            pass
        
        sourcePath = "%s/config/GeneratorConfig.h" % (self._srcPath)
        
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
        print >> wfile, '    void configureSets(const AutoPtr<XML::Document>& doc)'
        print >> wfile, '    {'
        print >> wfile, '        BaseGeneratorConfig::configureSets(doc);'
        print >> wfile, ''
        print >> wfile, '        // bind string sets to config members with the bindStringSet method'
        print >> wfile, '        // bind object sets to config members with the bindObjectSet method'
        print >> wfile, '    }'
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '} // namespace Myriad'
        print >> wfile, ''
        print >> wfile, '#endif /* GENERATORCONFIG_H_ */'

        wfile.close()
        

class EnumTypesCompiler(SourceCompiler):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        super(EnumTypesCompiler, self).__init__(*args, **kwargs)
    
    def compile(self, enumSets):
    
        try:
            os.makedirs("%s/record/base" % (self._srcPath))
        except OSError:
            pass
        
        wfile = open("%s/record/base/enums.h" % (self._srcPath), "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '// auto-generatad C++ enums'
        print >> wfile, ''
        print >> wfile, '#ifndef ENUMS_H_'
        print >> wfile, '#define ENUMS_H_'
        print >> wfile, ''
        print >> wfile, 'namespace %s {' % (self._args.dgen_ns)
        print >> wfile, ''
        
        for set in enumSets.getSets():
            print >> wfile, 'enum %s' % (set.getAttribute("key"))
            print >> wfile, '{'
            for item in set.getItems():
                print >> wfile, '    %s,' % (item.getAttribute("value")) 
            print >> wfile, '};'
            print >> wfile, ''
            
        print >> wfile, '} // namespace %s' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, 'namespace Myriad {'
        print >> wfile, ''
        for set in enumSets.getSets():
            print >> wfile, '// string conversion method for `%s`' % (set.getAttribute("key"))
            print >> wfile, 'inline std::string toString(const %s::%s& t)' % (self._args.dgen_ns, set.getAttribute("key"))
            print >> wfile, '{'
            for item in set.getItems():
                print >> wfile, '    if (t ==  %s::%s)' % (self._args.dgen_ns, item.getAttribute("value"))
                print >> wfile, '    {'
                print >> wfile, '        return "%s";' % (item.getAttribute("value"))
                print >> wfile, '    }'
            print >> wfile, ''
            print >> wfile, '    throw Poco::LogicException("unsupported value for type `%s`");' % (set.getAttribute("key"))
            print >> wfile, '}'
            print >> wfile, ''
            print >> wfile, '// enum conversion method for `%s`' % (set.getAttribute("key"))
            print >> wfile, 'template<> inline %(ns)s::%(t)s toEnum<%(ns)s::%(t)s> (int v)' % {"ns": self._args.dgen_ns, "t": set.getAttribute("key")}
            print >> wfile, '{'
            for i, item in enumerate(set.getItems()):
                print >> wfile, '    if (v == %d)' % (i)
                print >> wfile, '    {'
                print >> wfile, '        return %s::%s;' % (self._args.dgen_ns, item.getAttribute("value"))
                print >> wfile, '    }'
            print >> wfile, ''
            print >> wfile, '    throw Poco::LogicException("unknown value for type `%s`");' % (set.getAttribute("key"))
            print >> wfile, '}'
            print >> wfile, ''
        print >> wfile, '} // namespace Myriad'
        print >> wfile, ''
        print >> wfile, '#endif /* ENUMS_H_ */'

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
        
    def compile(self, recordSequences):
        for recordSequence in recordSequences.getRecordSequences():
            self._log.warning("compiling record C++ sources for `%s`" % (recordSequence.getAttribute("key")))
            self.compileBaseRecordType(recordSequence.getRecordType())
            self.compileRecordType(recordSequence.getRecordType())
            
    def compileBaseRecordType(self, recordType):
        try:
            os.makedirs("%s/record/base" % (self._srcPath))
        except OSError:
            pass
        
        typeNameUS = recordType.getAttribute("key")
        typeNameCC = StringTransformer.ucFirst(StringTransformer.us2cc(typeNameUS))
        typeNameUC = StringTransformer.uc(typeNameCC)
        
        wfile = open("%s/record/base/Base%s.h" % (self._srcPath, typeNameCC), "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '// auto-generatad C++ file for `%s`' % (recordType.getAttribute("key"))
        print >> wfile, ''
        print >> wfile, '#ifndef BASE%s_H_' % (typeNameUC)
        print >> wfile, '#define BASE%s_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#include "record/Record.h"'
        print >> wfile, '#include "record/base/enums.h"'
        print >> wfile, ''
        print >> wfile, 'using namespace Myriad;'
        print >> wfile, ''
        print >> wfile, 'namespace %s {' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '// forward declarations'
        print >> wfile, 'class %s;' % (typeNameCC)
        print >> wfile, 'class %sConfig;' % (typeNameCC)
        print >> wfile, 'class %sGenerator;' % (typeNameCC)
        print >> wfile, 'class %sHydratorChain;' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, 'class Base%s: public Record' % (typeNameCC)
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        print >> wfile, '    Base%s()' % (typeNameCC)
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        
        for field in recordType.getFields():
            print >> wfile, '    void %s(const %s& v);' % (StringTransformer.us2cc(field.getAttribute("name")), StringTransformer.sourceType(field.getAttribute("type")))
            print >> wfile, '    const %s& %s() const;' % (StringTransformer.sourceType(field.getAttribute("type")), StringTransformer.us2cc(field.getAttribute("name")))
            print >> wfile, ''
        
        print >> wfile, 'private:'
        print >> wfile, ''
        
        for field in recordType.getFields():
            print >> wfile, '    %s _%s;' % (StringTransformer.sourceType(field.getAttribute("type")), field.getAttribute("name")) 
        
        print >> wfile, '};'
        print >> wfile, ''
        
        for field in recordType.getFields():
            print >> wfile, 'inline void Base%s::%s(const %s& v)' % (typeNameCC, StringTransformer.us2cc(field.getAttribute("name")), StringTransformer.sourceType(field.getAttribute("type")))
            print >> wfile, '{'
            print >> wfile, '    _%s = v;' % (field.getAttribute("name"))
            print >> wfile, '}'
            print >> wfile, ''
            print >> wfile, 'inline const %s& Base%s::%s() const' % (StringTransformer.sourceType(field.getAttribute("type")), typeNameCC, StringTransformer.us2cc(field.getAttribute("name")))
            print >> wfile, '{'
            print >> wfile, '    return _%s;' % (field.getAttribute("name"))
            print >> wfile, '}'
            print >> wfile, ''
        
        print >> wfile, '} // namespace %s' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, 'namespace Myriad {'
        print >> wfile, ''
        print >> wfile, '// record traits specialization'
        print >> wfile, 'template<> struct RecordTraits<%s::%s>' % (self._args.dgen_ns, typeNameCC)
        print >> wfile, '{'
        print >> wfile, '    typedef %s::%sGenerator GeneratorType;' % (self._args.dgen_ns, typeNameCC)
        print >> wfile, '    typedef %s::%sHydratorChain HydratorChainType;' % (self._args.dgen_ns, typeNameCC)
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '// forward declaration of operator<<'
        print >> wfile, 'template<> void OutputCollector<%(ns)s::%(t)s>::CollectorType::collect(const %(ns)s::%(t)s& record);' % {'ns': self._args.dgen_ns, 't': typeNameCC}
        print >> wfile, ''
        print >> wfile, '} // namespace Myriad'
        print >> wfile, ''
        print >> wfile, '#endif /* BASE%s_H_ */' % (typeNameUC)

        wfile.close()
            
    def compileRecordType(self, recordType):
        try:
            os.makedirs("%s/record" % (self._srcPath))
        except OSError:
            pass
        
        typeNameUS = recordType.getAttribute("key")
        typeNameCC = StringTransformer.ucFirst(StringTransformer.us2cc(typeNameUS))
        typeNameUC = StringTransformer.uc(typeNameCC)
        
        sourcePath = "%s/record/%s.h" % (self._srcPath, typeNameCC)
        
        if (os.path.isfile(sourcePath)):
            return
        
        wfile = open(sourcePath, "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '#ifndef %s_H_' % (typeNameUC)
        print >> wfile, '#define %s_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#include "record/base/Base%s.h"' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, 'using namespace Myriad;'
        print >> wfile, ''
        print >> wfile, 'namespace %s {' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, 'class %(t)s: public Base%(t)s' % {'t': typeNameCC}
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        print >> wfile, '    %s()' % (typeNameCC)
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '} // namespace %s' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '#endif /* %s_H_ */' % (typeNameUC)

        wfile.close()


class RecordGeneratorCompiler(SourceCompiler):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        super(RecordGeneratorCompiler, self).__init__(*args, **kwargs)
        
    def compile(self, recordSequences):
        for recordSequence in recordSequences.getRecordSequences():
            self._log.warning("compiling generator C++ sources for `%s`" % (recordSequence.getAttribute("key")))
            self.compileBaseGenerator(recordSequence)
            self.compileGenerator(recordSequence)
            
    def compileBaseGenerator(self, recordSequence):
        try:
            os.makedirs("%s/generator/base" % (self._srcPath))
        except OSError:
            pass
        
        if isinstance(recordSequence, RandomSequenceNode):
            self.__compileBaseRandomGenerator(recordSequence)
        else:
            self._log.warning("unsupported generator type for sequence `%s`" % (recordSequence.getAttribute("key")))

    def __compileBaseRandomGenerator(self, recordSequence):
        typeNameUS = recordSequence.getAttribute("key")
        typeNameCC = StringTransformer.ucFirst(StringTransformer.us2cc(typeNameUS))
        typeNameUC = StringTransformer.uc(typeNameCC)
        
        sourcePath = "%s/generator/base/Base%sGenerator.h" % (self._srcPath, typeNameCC)
        
        wfile = open(sourcePath, "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '// auto-generatad C++ generator for `%s`' % (typeNameUS)
        print >> wfile, ''
        print >> wfile, '#ifndef BASE%sGENERATOR_H_' % (typeNameUC)
        print >> wfile, '#define BASE%sGENERATOR_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#include "generator/RandomSetGenerator.h"'
        print >> wfile, '#include "record/%s.h"' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, 'using namespace Myriad;'
        print >> wfile, ''
        print >> wfile, 'namespace %s {' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, 'class UserHydratorChain;'
        print >> wfile, ''
        print >> wfile, 'class Base%(t)sGenerator: public RandomSetGenerator<%(t)s>' % {'t': typeNameCC}
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        print >> wfile, '    typedef RecordTraits<%s>::HydratorChainType HydratorChainType;' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, '    Base%sGenerator(const string& name, GeneratorConfig& config, NotificationCenter& notificationCenter) :' % (typeNameCC)
        print >> wfile, '        RandomSetGenerator<%s>(name, config, notificationCenter)' % (typeNameCC)
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    void prepare(Stage stage, const GeneratorPool& pool)'
        print >> wfile, '    {'
        print >> wfile, '        // call generator implementation'
        print >> wfile, '        RandomSetGenerator<%s>::prepare(stage, pool);' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, '        if (stage.name() == "default")'
        print >> wfile, '        {'
        print >> wfile, '            registerTask(new RandomSetDefaultGeneratingTask<%s> (*this, _config));' % (typeNameCC)
        print >> wfile, '        }'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    HydratorChainType hydratorChain(BaseHydratorChain::OperationMode opMode, RandomStream& random);'
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '/**'
        print >> wfile, ' * Hydrator specialization for User.'
        print >> wfile, ' */'
        print >> wfile, 'class Base%(t)sHydratorChain : public HydratorChain<%(t)s>' % {'t': typeNameCC}
        print >> wfile, '{'
        print >> wfile, 'public:'
        print >> wfile, ''
        
        print >> wfile, '    // hydrator typedefs'
        for hydrator in sorted(recordSequence.getHydrators().getAll(), key=lambda h: h.orderkey):
            print >> wfile, '    typedef %s %s;' % (hydrator.getConcreteType(), hydrator.getAttribute("type_alias"))
            
        print >> wfile, ''
        print >> wfile, '    Base%sHydratorChain(OperationMode& opMode, RandomStream& random, GeneratorConfig& config) :' % (typeNameCC)
        print >> wfile, '        HydratorChain<%s>(opMode, random),' % (typeNameCC)
        
        initializers = []
        for hydrator in sorted(recordSequence.getHydrators().getAll(), key=lambda h: h.orderkey):
            argsCode = []
            if hydrator.hasPRNGArgument():
                argsCode.append('random')
            for argKey in hydrator.getConstructorArgumentsOrder():
                argsCode.append(self._argumentCode(hydrator.getArgument(argKey)))
            initializers.append('        _%s(%s)' % (StringTransformer.us2cc(hydrator.getAttribute("key")), ', '.join(argsCode)))
        
        print >> wfile, ',\n'.join(initializers) 
            
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    virtual ~Base%sHydratorChain()' % (typeNameCC)
        print >> wfile, '    {'
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, '    /**'
        print >> wfile, '     * Object hydrating function.'
        print >> wfile, '     */'
        print >> wfile, '    void operator()(AutoPtr<%s> recordPtr) const' % (typeNameCC)
        print >> wfile, '    {'
        print >> wfile, '        ensurePosition(recordPtr->genID());'
        print >> wfile, ''
        
        for hydrator in recordSequence.getHydrationPlan().getAll():
            print >> wfile, '         _%s(recordPtr);' % (StringTransformer.us2cc(hydrator.getAttribute("key")))
        
        print >> wfile, '    }'
        print >> wfile, ''
        print >> wfile, 'protected:'
        print >> wfile, ''
        
        print >> wfile, '    // hydrator members'
        for hydrator in sorted(recordSequence.getHydrators().getAll(), key=lambda h: h.orderkey):
            print >> wfile, '    %s _%s;' % (hydrator.getAttribute("type_alias"), StringTransformer.us2cc(hydrator.getAttribute("key")))
        
        print >> wfile, ''
        print >> wfile, '    /**'
        print >> wfile, '     * Logger instance.'
        print >> wfile, '     */'
        print >> wfile, '    Logger& _logger;'
        print >> wfile, '};'
        print >> wfile, ''
        print >> wfile, '} // namespace %s' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, '#endif /* BASE%sGENERATOR_H_ */' % (typeNameUC)

        wfile.close()
            
    def compileGenerator(self, recordSequence):
        try:
            os.makedirs("%s/generator" % (self._srcPath))
        except OSError:
            pass
        
        if isinstance(recordSequence, RandomSequenceNode):
            self.__compileRandomGenerator(recordSequence)
        else:
            self._log.warning("unsupported generator type for sequence `%s`" % (recordSequence.getAttribute("key")))

    def __compileRandomGenerator(self, recordSequence):
        typeNameUS = recordSequence.getAttribute("key")
        typeNameCC = StringTransformer.ucFirst(StringTransformer.us2cc(typeNameUS))
        typeNameUC = StringTransformer.uc(typeNameCC)
        
        sourcePath = "%s/generator/%sGenerator.h" % (self._srcPath, typeNameCC)
        
        if (os.path.isfile(sourcePath)):
            return
        
        wfile = open(sourcePath, "w", SourceCompiler.BUFFER_SIZE)
        
        print >> wfile, '#ifndef %sGENERATOR_H_' % (typeNameUC)
        print >> wfile, '#define %sGENERATOR_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#endif /* BASE%sGENERATOR_H_ */' % (typeNameUC)

        wfile.close()