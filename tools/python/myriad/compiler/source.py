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

import os, re

class FileCompiler(object):
    '''
    classdocs
    '''
    
    BUFFER_SIZE = 512
    
    _args = None
    _srcPath = None
    _cc2us_pattern1 = None
    _cc2us_pattern2 = None
    
    def __init__(self, args):
        '''
        Constructor
        '''
        self._cc2us_pattern1 = re.compile('(.)([A-Z][a-z]+)')
        self._cc2us_pattern2 = re.compile('([a-z0-9])([A-Z])')
        
        self._args = args
        self._srcPath = "%s/../../src/cpp" % (args.base_path)
        
    def _uc(self, s):
        return s.upper()

    def _lc(self, s):
        return s.lower()

    def _cc2us(self, s):
        return self._cc2us_pattern2.sub(r'\1_\2', self._cc2us_pattern1.sub(r'\1_\2', s)).lower()

    def _us2cc(self, s):
        def camelcase(): 
            yield str.lower
            while True:
                yield str.capitalize
    
        c = camelcase()
        return "".join(c.next()(x) if x else '_' for x in s.split("_"))
    
    def _ucFirst(self, s):
        return "%s%s" % (s[0].capitalize(), s[1:])


class EnumTypesCompiler(FileCompiler):
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
        
        wfile = open("%s/record/base/enums.h" % (self._srcPath), "w", RecordTypeCompiler.BUFFER_SIZE)
        
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


class RecordTypeCompiler(FileCompiler):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        super(RecordTypeCompiler, self).__init__(*args, **kwargs)
        
    def compile(self, recordSequences):
        for v in recordSequences.getRecordSequences():
            self.compileBaseRecordType(v.getRecordType())
            self.compileRecordType(v.getRecordType())
            
    def compileBaseRecordType(self, recordType):
    
        try:
            os.makedirs("%s/record/base" % (self._srcPath))
        except OSError:
            pass
        
        typeNameUS = recordType.getAttribute("key")
        typeNameCC = self._ucFirst(self._us2cc(typeNameUS))
        typeNameUC = self._uc(typeNameCC)
        
        wfile = open("%s/record/base/Base%s.h" % (self._srcPath, typeNameCC), "w", RecordTypeCompiler.BUFFER_SIZE)
        
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
            print >> wfile, '    void %s(const %s& v);' % (self._us2cc(field.getAttribute("name")), field.getAttribute("type"))
            print >> wfile, '    const %s& %s() const;' % (field.getAttribute("type"), self._us2cc(field.getAttribute("name")))
            print >> wfile, ''
        
        print >> wfile, 'private:'
        print >> wfile, ''
        
        for field in recordType.getFields():
            print >> wfile, '    %s _%s;' % (field.getAttribute("type"), field.getAttribute("name")) 
        
        print >> wfile, '};'
        print >> wfile, ''
        
        for field in recordType.getFields():
            print >> wfile, 'inline void Base%s::%s(const %s& v)' % (typeNameCC, self._us2cc(field.getAttribute("name")), field.getAttribute("type"))
            print >> wfile, '{'
            print >> wfile, '    _%s = v;' % (field.getAttribute("name"))
            print >> wfile, '}'
            print >> wfile, ''
            print >> wfile, 'inline const %s& Base%s::%s() const' % (field.getAttribute("type"), typeNameCC, self._us2cc(field.getAttribute("name")))
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
        typeNameCC = self._ucFirst(self._us2cc(typeNameUS))
        typeNameUC = self._uc(typeNameCC)
        
        sourcePath = "%s/record/%s.h" % (self._srcPath, typeNameCC)
        
        if (os.path.exists(sourcePath)):
            return
        
        wfile = open(sourcePath, "w", RecordTypeCompiler.BUFFER_SIZE)
        
        print >> wfile, '// auto-generatad C++ file for `%s`' % (recordType.getAttribute("key"))
        print >> wfile, ''
        print >> wfile, '#ifndef %s_H_' % (typeNameUC)
        print >> wfile, '#define %s_H_' % (typeNameUC)
        print >> wfile, ''
        print >> wfile, '#include "record/base/Base%s.h"' % (typeNameCC)
        print >> wfile, ''
        print >> wfile, 'using namespace Myriad;'
        print >> wfile, ''
        print >> wfile, 'namespace %s {' % (self._args.dgen_ns)
        print >> wfile, ''
        print >> wfile, 'class %s: public Record' % (typeNameCC)
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