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

import re
from myriad.compiler.visitor import AbstractVisitor
from myriad.util.stringutil import StringTransformer
    

#
# Utility classes
#

class AstUtils(object):
    '''
    classdocs
    '''
    
    # a pattern for argument types
    __argument_type_pattern = re.compile('^((collection)\[([a-zA-Z0-9_]+)\])|([a-zA-Z0-9_]+)$')
    # a list of all incremental types
    __incremental_types = ['Char', 'Date', 'Enum', 'I16', 'I32', 'I64', 'I16u', 'I32u', 'I64u']
    # a list of all ordered types
    __ordered_types = ['Char', 'Date', 'Decimal', 'Enum', 'I16', 'I32', 'I64', 'I16u', 'I32u', 'I64u']
    # a list of all literal types
    __literal_types = ['Bool', 'Char', 'Date', 'Decimal', 'Enum', 'I16', 'I32', 'I64', 'I16u', 'I32u', 'I64u', 'String']
    # a pattern for the literal types
    __literal_type_pattern = re.compile('^(%s)$' % '|'.join(['Bool', 'Char', 'Date', 'Decimal', 'Enum', 'I16', 'I32', 'I64', 'I16u', 'I32u', 'I64u', 'String']))
    # a pattern for the vector types
    __vector_type_pattern = re.compile('^(%s)\[(\d+)\]$' % '|'.join(['Bool', 'Char', 'Date', 'Decimal', 'Enum', 'I16', 'I32', 'I64', 'I16u', 'I32u', 'I64u', 'String']))
    
    @classmethod
    def incrementalTypes(cls):
        return cls.__incremental_types
    
    @classmethod
    def orderedTypes(cls):
        return cls.__ordered_types
    
    @classmethod
    def literalTypes(cls):
        return cls.__literal_types

    @classmethod
    def matchArgumentType(cls, type):
        return cls.__argument_type_pattern.match(type)

    @classmethod
    def matchLiteralType(cls, type):
        return cls.__literal_type_pattern.match(type)
        
    @classmethod
    def matchVectorType(cls, type):
        return cls.__vector_type_pattern.match(type)
    
    @staticmethod
    def iterSubClasses(cls, _seen=None):
        if not isinstance(cls, type):
            raise TypeError('itersubclasses must be called with new-style classes, not %.100r' % cls)
        if _seen is None: _seen = set()
        try:
            subs = cls.__subclasses__()
        except TypeError: # fails only when cls is type
            subs = cls.__subclasses__(cls)
        for sub in subs:
            if sub not in _seen:
                _seen.add(sub)
                yield sub
                for sub in AstUtils.iterSubClasses(sub, _seen):
                    yield sub


#
# Error types
#

class ParseTypeError(TypeError):
    '''
    An error type to be thrown by parseType() AST node methods
    '''
    pass


#
# AST nodes
#

class AbstractNode(object):
    '''
    classdocs
    '''
    
    _attributes = {}
    
    def __init__(self, *args, **kwargs):
        self._attributes = dict(kwargs)
    
    def accept(self, visitor):
        visitor.preVisit(self)
        visitor.postVisit(self)
        
    def setAttribute(self, key, value):
        self._attributes[key] = value
        
    def getAttribute(self, key, default=None):
        return self._attributes.get(key, default)
    
    def hasAttribute(self, key):
        return self._attributes.has_key(key)
    
    def allAttributes(self):
        return self._attributes


class RootNode(AbstractNode):
    '''
    classdocs
    '''
    
    __specification = None
    
    
    def __init__(self, *args, **kwargs):
        super(RootNode, self).__init__(*args, **kwargs)
        self.__specification = SpecificationNode()
    
    def accept(self, visitor):
        visitor.preVisit(self)
        self.__specification.accept(visitor)
        visitor.postVisit(self)

    def getSpecification(self):
        return self.__specification


class SpecificationNode(AbstractNode):
    '''
    classdocs
    '''

    __parameters = None
    __functions = None
    __enumSets = None
    __recordSequences = None
    
    def __init__(self, *args, **kwargs):
        super(SpecificationNode, self).__init__(*args, **kwargs)
        self.__parameters = ParametersNode()
        self.__functions = FunctionsNode()
        self.__enumSets = EnumSetsNode()
        self.__recordSequences = RecordSequencesNode()
    
    def accept(self, visitor):
        visitor.preVisit(self)
        self.__parameters.accept(visitor)
        self.__functions.accept(visitor)
        self.__enumSets.accept(visitor)
        self.__recordSequences.accept(visitor)
        visitor.postVisit(self)
        
    def getParameters(self):
        return self.__parameters
    
    def getFunctions(self):
        return self.__functions
    
    def getEnumSets(self):
        return self.__enumSets
    
    def getRecordSequences(self):
        return self.__recordSequences


class ParametersNode(AbstractNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(ParametersNode, self).__init__(*args, **kwargs)
        
    def setParameter(self, key, value):
        self.setAttribute(key, value)
        
    def getParameter(self, key):
        return self.getAttribute(key)
    
    def hasParameter(self, value):
        return self.hasAttribute(value)

    def getAll(self):
        return self.allAttributes()

#
# Functions
# 

class FunctionsNode(AbstractNode):
    '''
    classdocs
    '''
    
    __functions = {}
    
    def __init__(self, *args, **kwargs):
        super(FunctionsNode, self).__init__(*args, **kwargs)
        self.__functions = {}
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for key, node in sorted(self.__functions.iteritems()):
            node.accept(visitor)
        visitor.postVisit(self)
        
    def setFunction(self, node):
        self.__functions[node.getAttribute('key')] = node
    
    def getFunction(self, key):
        return self.__functions.get(key)
    
    def hasFunction(self, key):
        return self.__functions.has_key(key)

    
class AbstractFunctionNode(AbstractNode):
    '''
    classdocs
    '''
    
    __arguments = {}
    
    orderkey = None
    
    def __init__(self, *args, **kwargs):
        super(AbstractFunctionNode, self).__init__(*args, **kwargs)
        self.__arguments = {}
        self.orderkey = None
        self.setArgument(LiteralArgumentNode(key='key', type='String', value=self.getAttribute("key")))
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in self.__arguments.itervalues():
            node.accept(visitor)
        visitor.postVisit(self)
        
    def setArgument(self, node):
        self.__arguments[node.getAttribute('key')] = node
        node.setParent(self)
    
    def getArgument(self, key, default=None):
        return self.__arguments.get(key, default)
        
    def setOrderKey(self, key):
        self.orderkey = key
    
    def getConcreteType(self):
        return self.getAttribute("concrete_type")
        
    def getDomainType(self):
        return self.getAttribute('domain_type')
    
    def getXMLArguments(self):
        return {}
    
    def getConstructorArguments(self):
        return [ 'Literal(key)' ]

    

class ParetoProbabilityFunctionNode(AbstractFunctionNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^pareto_probability\[(Decimal)\]$')
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="ParetoPrFunction")
        kwargs.update(concrete_type="Myriad::ParetoPrFunction")
        super(ParetoProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getXMLArguments(self):
        return [ { 'key': 'x_min', 'type': self.getDomainType() }, 
                 { 'key': 'alpha', 'type': self.getDomainType() } 
               ]
        
    def getConstructorArguments(self):
        args = super(ParetoProbabilityFunctionNode, self).getConstructorArguments()
        args.extend(['Literal(x_min)',
                     'Literal(alpha)'
                     ])
        return args
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], { 'domain_type': m.group(1) }
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])

    
class NormalProbabilityFunctionNode(AbstractFunctionNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^normal_probability\[(Decimal)\]$')
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="NormalPrFunction")
        kwargs.update(concrete_type="Myriad::NormalPrFunction")
        super(NormalProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getXMLArguments(self):
        return [ { 'key': 'mean', 'type': self.getDomainType() }, 
                 { 'key': 'stddev', 'type': self.getDomainType() } 
               ]
        
    def getConstructorArguments(self):
        args = super(NormalProbabilityFunctionNode, self).getConstructorArguments()
        args.extend(['Literal(mean)',
                     'Literal(stddev)'
                    ])
        return args
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], { 'domain_type': m.group(1) }
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


class UniformProbabilityFunctionNode(AbstractFunctionNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^uniform_probability\[(%s)\]$' % ('|'.join(AstUtils.orderedTypes())))
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="UniformPrFunction")
        kwargs.update(concrete_type="Myriad::UniformPrFunction<%s>" % kwargs["domain_type"])
        super(UniformProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getXMLArguments(self):
        return [ { 'key': 'x_min', 'type': self.getDomainType() }, 
                 { 'key': 'x_max', 'type': self.getDomainType() } 
               ]
        
    def getConstructorArguments(self):
        args = super(UniformProbabilityFunctionNode, self).getConstructorArguments()
        args.extend(['Literal(x_min)',
                     'Literal(x_max)'
                     ])
        return args
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], { 'domain_type': m.group(1) }
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])
    

class CombinedProbabilityFunctionNode(AbstractFunctionNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^combined_probability\[(%s)\]$' % ('|'.join(AstUtils.orderedTypes())))
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="CombinedPrFunction")
        kwargs.update(concrete_type="Myriad::CombinedPrFunction<%s>" % kwargs["domain_type"])
        super(CombinedProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getXMLArguments(self):
        return [ { 'key': 'path', 'type': 'String' } 
               ]
        
    def getConstructorArguments(self):
        args = super(CombinedProbabilityFunctionNode, self).getConstructorArguments()
        args.extend(['Literal(path)'])
        return args
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], { 'domain_type': m.group(1) }
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])
    

class ConditionalCombinedProbabilityFunctionNode(AbstractFunctionNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^conditional_combined_probability\[(%s);(%s)\]$' % ('|'.join(AstUtils.orderedTypes()), '|'.join(AstUtils.orderedTypes())))
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="ConditionalCombinedPrFunction")
        kwargs.update(concrete_type="Myriad::ConditionalCombinedPrFunction<%s, %s>" % (kwargs["domain_type"], kwargs["condition_type"]))
        super(ConditionalCombinedProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getDomainType(self):
        return self.getAttribute('domain_type')
        
    def getXMLArguments(self):
        return [ { 'key': 'path', 'type': 'String' } 
               ]
        
    def getConstructorArguments(self):
        args = super(ConditionalCombinedProbabilityFunctionNode, self).getConstructorArguments()
        args.extend(['Literal(path)'])
        return args
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], { 'domain_type': m.group(1), 'condition_type': m.group(2) }
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


#
# Enum Sets
# 

class EnumSetsNode(AbstractNode):
    '''
    classdocs
    '''
    
    __sets = {}
    
    def __init__(self, *args, **kwargs):
        super(EnumSetsNode, self).__init__(*args, **kwargs)
        self.__sets = {}
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in self.__sets.itervalues():
            node.accept(visitor)
        visitor.postVisit(self)
        
    def setSet(self, node):
        self.__sets[node.getAttribute('key')] = node
    
    def getSet(self, key):
        return self.__sets.get(key)
    
    def hasSet(self, key):
        return self.__sets.has_key(key)
    
    def getSets(self):
        return self.__sets.itervalues()


class EnumSetNode(AbstractNode):
    '''
    classdocs
    '''
    
    __arguments = {}
    
    def __init__(self, *args, **kwargs):
        super(EnumSetNode, self).__init__(*args, **kwargs)
        self.__arguments = {}
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in self.__arguments.itervalues():
            node.accept(visitor)
        visitor.postVisit(self)
        
    def setArgument(self, node):
        self.__arguments[node.getAttribute('key')] = node
        node.setParent(self)
    
    def getArgument(self, key):
        return self.__arguments.get(key)
        
    def getXMLArguments(self):
        return [ { 'key': 'path', 'type': 'String' } 
               ]
        
    def getConstructorArguments(self):
        return [ 'Literal(path)' 
               ]


#
# Record Sequences
# 

class RecordSequencesNode(AbstractNode):
    '''
    classdocs
    '''
    
    __sequences = {}
    __sequenceList = []
    
    def __init__(self, *args, **kwargs):
        super(RecordSequencesNode, self).__init__(*args, **kwargs)
        self.__sequences = {}
        self.__sequenceList = []
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in self.__sequenceList:
            node.accept(visitor)
        visitor.postVisit(self)
        
    def setRecordSequence(self, node):
        if not self.hasRecordSequence(node.getAttribute('key')):
            self.__sequenceList.append(node)
            self.__sequences[node.getAttribute('key')] = node
    
    def getRecordSequence(self, key):
        return self.__sequences.get(key)
    
    def hasRecordSequence(self, key):
        return self.__sequences.has_key(key)
    
    def getRecordSequences(self):
        return self.__sequenceList


class RecordSequenceNode(AbstractNode):
    '''
    classdocs
    '''
    
    _recordType = None
    _outputFormatter = None
    
    def __init__(self, *args, **kwargs):
        super(RecordSequenceNode, self).__init__(*args, **kwargs)
        self._outputFormatter = None
    
    def accept(self, visitor):
        visitor.preVisit(self)
        self._recordType.accept(visitor)
        self._outputFormatter.accept(visitor)
        visitor.postVisit(self)
        
    def setRecordType(self, node):
        self._recordType = node
        
    def getRecordType(self):
        return self._recordType
        
    def setOutputFormatter(self, node):
        self._outputFormatter = node
        node.setParent(self)
        
    def getOutputFormatter(self):
        return self._outputFormatter
        
    def hasOutputFormatter(self):
        return self._outputFormatter is not None


class RandomSequenceNode(RecordSequenceNode):
    '''
    classdocs
    '''
    
    __cardinalityEstimator = None
    __setterChain = None
    __sequenceIterator = None
    
    def __init__(self, *args, **kwargs):
        super(RandomSequenceNode, self).__init__(*args, **kwargs)
        self.__cardinalityEstimator = None
        self.__setterChain = None
        self.__sequenceIterator = None
    
    def accept(self, visitor):
        visitor.preVisit(self)
        self._recordType.accept(visitor)
        if self._outputFormatter is not None:
            self._outputFormatter.accept(visitor)
        self.__setterChain.accept(visitor)
        self.__cardinalityEstimator.accept(visitor)
        if self.__sequenceIterator is not None:
            self.__sequenceIterator.accept(visitor)
        visitor.postVisit(self)
        
    def setSetterChain(self, node):
        self.__setterChain = node
        node.setParent(self)
        
    def getSetterChain(self):
        return self.__setterChain
        
    def setCardinalityEstimator(self, node):
        self.__cardinalityEstimator = node
        node.setParent(self)
        
    def getCardinalityEstimator(self):
        return self.__cardinalityEstimator
        
    def setSequenceIterator(self, node):
        self.__sequenceIterator = node
        node.setParent(self)
        
    def getSequenceIterator(self):
        return self.__sequenceIterator
        
    def hasSequenceIterator(self):
        return self.__sequenceIterator is not None
        

class RecordTypeNode(AbstractNode):
    '''
    classdocs
    '''
    
    _fields = {}
    _references = {}
    _referenceTypes = []
    _enumFields = {}
    _enumFieldNames = []
    
    def __init__(self, *args, **kwargs):
        super(RecordTypeNode, self).__init__(*args, **kwargs)
        self._fields = {}
        self._references = {}
        self._referenceTypes = []
        self._enumFields = {}
        self._enumFieldNames = []
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in self._fields.itervalues():
            node.accept(visitor)
        for node in self._references.itervalues():
            node.accept(visitor)
        visitor.postVisit(self)
        
    def setField(self, node):
        self._fields[node.getAttribute('name')] = node
        node.setParent(self)
    
    def getField(self, key):
        return self._fields.get(key)
    
    def getFields(self):
        return sorted(self._fields.itervalues(), key=lambda f: f.orderkey)

    def hasFields(self):
        return bool(self._fields)
        
    def setReference(self, node):
        self._references[node.getAttribute('name')] = node
        node.setParent(self)

        if isinstance(node, ResolvedRecordReferenceNode):
            t = node.getAttribute('type')
            if not t in self._referenceTypes:
                self._referenceTypes.append(t)
    
    def getReference(self, key):
        return self._references.get(key)
    
    def getReferences(self):
        return sorted(self._references.itervalues(), key=lambda f: f.orderkey)

    def hasReferences(self):
        return bool(self._references)
    
    def getReferenceTypes(self):
        return sorted(self._referenceTypes)
        
    def setEnumField(self, node):
        self._enumFields[node.getAttribute('name')] = node
        self._fields[node.getAttribute('name')] = node
        node.setParent(self)

        t = node.getAttribute('enumref')
        if not t in self._enumFieldNames:
            self._enumFieldNames.append(t)
    
    def getEnumField(self, key):
        return self._enumFields.get(key)
    
    def getEnumFields(self):
        return self._enumFields.itervalues()

    def hasEnumFields(self):
        return bool(self._enumFields)
    
    def getEnumFieldNames(self):
        return self._enumFieldNames


class RecordFieldNode(AbstractNode):
    '''
    classdocs
    '''
    
    orderkey = None
    __parent = None
    __setter = None
    
    def __init__(self, *args, **kwargs):
        super(RecordFieldNode, self).__init__(*args, **kwargs)
        self.orderkey = None
        self.__parent = None
        
    def setOrderKey(self, key):
        self.orderkey = key

    def setParent(self, parent):
        self.__parent = parent

    def getParent(self):
        return self.__parent

    def setSetter(self, setter):
        self.__setter = setter

    def getSetter(self):
        return self.__setter

    def hasSetter(self):
        return self.__setter is not None
    
    def isImplicit(self):
        return self.getAttribute("implicit", False)
    
    def isDerived(self):
        return self.getAttribute("derived", False)
    
    def getID(self):
        return "Myriad::RecordTraits<%s>::%s" % (StringTransformer.us2ccAll(self.getParent().getAttribute("key")), self.getAttribute("name").upper())
        
    def isSimpleType(self):
        r = AstUtils.matchLiteralType(self.getAttribute("type"))
        return r is not None
        
    def isVectorType(self):
        # check vector types
        r = AstUtils.matchVectorType(self.getAttribute("type"))
        return r is not None
        
    def vectorTypeSize(self):
        # check vector types
        r = AstUtils.matchVectorType(self.getAttribute("type"))
        if r is not None:
            return r.group(2)
        
    def sourceType(self):
        # check vector types
        r = AstUtils.matchVectorType(self.getAttribute("type"))
        if r is not None:
            return "vector<%s>" % (r.group(1))
        
        # check simple types
        r = AstUtils.matchLiteralType(self.getAttribute("type"))
        if r is not None:
            return r.group(1)
        
        # otherwise exception
        raise RuntimeError("Unsupported Myriad source type `%s`" % (self.getAttribute("type")))
        
    def coreType(self):
        # check vector types
        r = AstUtils.matchVectorType(self.getAttribute("type"))
        if r is not None:
            return r.group(1)
        
        # check simple types
        r = AstUtils.matchLiteralType(self.getAttribute("type"))
        if r is not None:
            return r.group(1)
        
        # otherwise exception
        raise RuntimeError("Unsupported Myriad core type `%s`" % (self.getAttribute("type")))


class RecordEnumFieldNode(RecordFieldNode):
    '''
    classdocs
    '''
    
    orderkey = None
    __enumSetRef = None
    
    def __init__(self, *args, **kwargs):
        super(RecordEnumFieldNode, self).__init__(*args, **kwargs)
        self.__eumSetRef = None
        
    def getEnumSetRef(self):
        return self.__enumSetRef
        
    def setEnumSetRef(self, enumSetRef):
        self.__enumSetRef = enumSetRef


#FIXME: use a common XML syntax for RecordReferenceNode and RecordEnumFieldNode
class RecordReferenceNode(AbstractNode):
    '''
    classdocs
    '''
    
    orderkey = None
    __parent = None
    
    def __init__(self, *args, **kwargs):
        super(RecordReferenceNode, self).__init__(*args, **kwargs)
        self.orderkey = None
        self.__parent = None
        
    def setOrderKey(self, key):
        self.orderkey = key

    def setParent(self, parent):
        self.__parent = parent

    def getParent(self):
        return self.__parent


class UnresolvedRecordReferenceNode(RecordReferenceNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(UnresolvedRecordReferenceNode, self).__init__(*args, **kwargs)


class ResolvedRecordReferenceNode(RecordReferenceNode):
    '''
    classdocs
    '''
    
    __recordTypeRef = None
    
    def __init__(self, *args, **kwargs):
        super(ResolvedRecordReferenceNode, self).__init__(*args, **kwargs)
        self.__recordTypeRef = None
        
    def setRecordTypeRef(self, recordTypeRef):
        self.__recordTypeRef = recordTypeRef

    def getRecordTypeRef(self):
        return self.__recordTypeRef
    
    def getID(self):
        return "Myriad::RecordTraits<%s>::%s" % (StringTransformer.us2ccAll(self.getParent().getAttribute("key")), self.getAttribute("name").upper())


#
# Setter Chain
# 

class SetterChainNode(AbstractNode):
    '''
    classdocs
    '''
    
    __setters = {}
    __parent = None
    
    def __init__(self, *args, **kwargs):
        super(SetterChainNode, self).__init__(*args, **kwargs)
        self.__setters = {}
        self.__parent = None
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in sorted(self.__setters.itervalues(), key=lambda s: s.orderkey):
            node.accept(visitor)
        visitor.postVisit(self)
        
    def getCxtRecordType(self):
        return StringTransformer.us2ccAll(self.__parent.getAttribute('key'))

    def setParent(self, parent):
        self.__parent = parent

    def getParent(self):
        return self.__parent
        
    def setSetter(self, node):
        self.__setters[node.getAttribute('key')] = node
        node.setParent(self)
    
    def getSetter(self, key):
        return self.__setters.get(key)
    
    def hasSetter(self, key):
        return self.__setters.has_key(key)
    
    def getAll(self):
        return sorted(self.__setters.itervalues(), key=lambda s: s.orderkey)
    
    def getFieldSetters(self):
        return sorted(filter(lambda s: isinstance(s, FieldSetterNode), self.__setters.itervalues()), key=lambda s: s.orderkey)
    
    def settersCount(self):
        return len(self.__setters)
    
    def getComponentIncludePaths(self):
        nodeFilter = DepthFirstNodeFilter(filterType=AbstractRuntimeComponentNode)
        componentPaths = [ node.getIncludePath() for node in nodeFilter.getAll(self) ]
        return sorted(set(componentPaths))

#
# Cardinality Estimators
# 

class AbstractCardinalityEstimatorNode(AbstractNode):
    '''
    classdocs
    '''
    
    __parent = None
    __arguments = {}
    
    def __init__(self, *args, **kwargs):
        super(AbstractCardinalityEstimatorNode, self).__init__(*args, **kwargs)
        self.__parent = None
        self.__arguments = {}
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in self.__arguments.itervalues():
            node.accept(visitor)
        visitor.postVisit(self)
        
    def setArgument(self, node):
        self.__arguments[node.getAttribute('key')] = node
        node.setParent(self)
    
    def getArgument(self, key):
        return self.__arguments.get(key)

    def setParent(self, parent):
        self.__parent = parent

    def getParent(self):
        return self.__parent
        
    def getXMLArguments(self):
        return {}
        
    def getConstructorArguments(self):
        return []


class LinearScaleEstimatorNode(AbstractCardinalityEstimatorNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^linear_scale_estimator$')
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="linear_scale_estimator")
        super(LinearScaleEstimatorNode, self).__init__(*args, **kwargs)
        
    def getXMLArguments(self):
        return [ { 'key': 'base_cardinality', 'type': 'I64u' } 
               ]
        
    def getConstructorArguments(self):
        return [ 'Literal(base_cardinality)' 
               ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


class ConstEstimatorNode(AbstractCardinalityEstimatorNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^const_estimator$')
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="const_estimator")
        super(ConstEstimatorNode, self).__init__(*args, **kwargs)
        
    def getXMLArguments(self):
        return [ { 'key': 'cardinality', 'type': 'I64u' } 
               ]
        
    def getConstructorArguments(self):
        return [ 'Literal(cardinality)' 
               ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


#
# Sequence Iterators
# 
 
class AbstractSequenceIteratorNode(AbstractNode):
    '''
    classdocs
    '''
    
    __arguments = {}
    __parent = None
    
    def __init__(self, *args, **kwargs):
        super(AbstractSequenceIteratorNode, self).__init__(*args, **kwargs)
        self.__arguments = {}
        self.__parent = None
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in self.__arguments.itervalues():
            node.accept(visitor)
        visitor.postVisit(self)
        
    def setArgument(self, node):
        self.__arguments[node.getAttribute('key')] = node
        node.setParent(self)
    
    def getArgument(self, key):
        return self.__arguments.get(key)
    
    def setParent(self, parent):
        self.__parent = parent

    def getParent(self):
        return self.__parent
        
    def getConcreteType(self):
        return "Myriad::SequenceIterator"
        
    def getXMLArguments(self):
        return {}
        
    def getConstructorArguments(self):
        return [ 'EnvVariable(*this)',
                 'EnvVariable(_config)' 
               ]


class PartitionedSequenceIteratorNode(AbstractSequenceIteratorNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^partitioned_iterator$')
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="PartitionedSequenceIteratorTask")
        super(PartitionedSequenceIteratorNode, self).__init__(*args, **kwargs)
        
    def getConcreteType(self):
        recordType = StringTransformer.us2ccAll(self.getParent().getAttribute("key"))

        return "Myriad::PartitionedSequenceIteratorTask< %s >" % (recordType)
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


#
# Output Formatters
# 
 
class AbstractOutputFormatterNode(AbstractNode):
    '''
    classdocs
    '''
    
    __arguments = {}
    __parent = None
    
    def __init__(self, *args, **kwargs):
        super(AbstractOutputFormatterNode, self).__init__(*args, **kwargs)
        self.__arguments = {}
        self.__parent = None
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in self.__arguments.itervalues():
            node.accept(visitor)
        visitor.postVisit(self)
        
    def setArgument(self, node):
        self.__arguments[node.getAttribute('key')] = node
        node.setParent(self)
    
    def getArgument(self, key):
        return self.__arguments.get(key)
    
    def setParent(self, parent):
        self.__parent = parent

    def getParent(self):
        return self.__parent
        
    def getConcreteType(self):
        return "Myriad::OutputFormatter"
        
    def getXMLArguments(self):
        return {}
        
    def getConstructorArguments(self):
        return []


class CsvOutputFormatterNode(AbstractOutputFormatterNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^csv$')
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="csv")
        kwargs.update(template_type="CsvOutputFormatter")
        super(CsvOutputFormatterNode, self).__init__(*args, **kwargs)
        
    def getConcreteType(self):
        recordType = StringTransformer.us2ccAll(self.getParent().getAttribute("key"))

        return "Myriad::CsvOutputFormatter< %s >" % (recordType)
        
    def getXMLArguments(self):
        return [ { 'key': 'delimiter', 'type': 'Char', 'default': '|' },
                 { 'key': 'quoted', 'type': 'Bool', 'default': 'true' },
                 { 'key': 'field', 'type': 'collection[field_ref]' }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'Literal(delimiter)',
                 'FieldSetterRef(field)'
               ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


class EmptyOutputFormatterNode(AbstractOutputFormatterNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^empty$')
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="empty")
        kwargs.update(template_type="EmptyOutputFormatter")
        super(EmptyOutputFormatterNode, self).__init__(*args, **kwargs)
        
    def getConcreteType(self):
        recordType = StringTransformer.us2ccAll(self.getParent().getAttribute("key"))

        return "Myriad::EmptyOutputFormatter< %s >" % (recordType)
        
    def getXMLArguments(self):
        return []
        
    def getConstructorArguments(self):
        return []
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


#
# Arguments
# 
    
class ArgumentNode(AbstractNode):
    '''
    classdocs
    '''
    
    __parent = None
    orderkey = None
    
    def __init__(self, *args, **kwargs):
        super(ArgumentNode, self).__init__(*args, **kwargs)
        self.orderkey = None

    def setParent(self, parent):
        self.__parent = parent

    def getParent(self):
        return self.__parent


class ArgumentCollectionNode(ArgumentNode):
    '''
    classdocs
    '''
    
    __collection = []
    
    def __init__(self, *args, **kwargs):
        super(ArgumentCollectionNode, self).__init__(*args, **kwargs)
        self.__collection = kwargs['collection']
        for i in range(len(self.__collection)):
            self.__collection[i].setParent(self)
            self.__collection[i].orderkey = i 
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in self.__collection:
            node.accept(visitor)
        visitor.postVisit(self)
        
    def setArgument(self, pos, node):
        self.__collection[pos] = node
        node.setParent(self)
        node.orderkey = pos
    
    def getArgument(self, pos):
        return self.__collection[pos]
    
    def getAll(self):
        return self.__collection
    

class LiteralArgumentNode(ArgumentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(LiteralArgumentNode, self).__init__(*args, **kwargs)


class UnresolvedFieldRefArgumentNode(ArgumentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(UnresolvedFieldRefArgumentNode, self).__init__(*args, **kwargs)


class UnresolvedReferenceRefArgumentNode(ArgumentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(UnresolvedReferenceRefArgumentNode, self).__init__(*args, **kwargs)


class UnresolvedFunctionRefArgumentNode(ArgumentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(UnresolvedFunctionRefArgumentNode, self).__init__(*args, **kwargs)


class StringSetRefArgumentNode(ArgumentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(StringSetRefArgumentNode, self).__init__(*args, **kwargs)


class ResolvedFieldRefArgumentNode(ArgumentNode):
    '''
    classdocs
    '''
    
    __fieldRef = None
    __recordTypeRef = None
    __innerPathRefs = None
    
    def __init__(self, *args, **kwargs):
        super(ResolvedFieldRefArgumentNode, self).__init__(*args, **kwargs)
        self.__fieldRef = None
        self.__recordTypeRef = None
        self.__innerPathRefs = []

    def setRecordTypeRef(self, recordTypeRef):
        self.__recordTypeRef = recordTypeRef

    def getRecordTypeRef(self):
        return self.__recordTypeRef

    def setInnerPathRefs(self, innerPathRefs):
        self.__innerPathRefs = innerPathRefs

    def getInnerPathRefs(self):
        return self.__innerPathRefs

    def setFieldRef(self, fieldRef):
        self.__fieldRef = fieldRef

    def getFieldRef(self):
        return self.__fieldRef
    
    def isDirect(self):
        return len(self.getInnerPathRefs()) == 0
    
    def getID(self):
        return self.getFieldRef().getID()


class ResolvedReferenceRefArgumentNode(ArgumentNode):
    '''
    classdocs
    '''
    
    __recordTypeRef = None
    __recordReferenceRef = None
    
    def __init__(self, *args, **kwargs):
        super(ResolvedReferenceRefArgumentNode, self).__init__(*args, **kwargs)
        self.__recordReferenceRef = None
        self.__recordTypeRef = None

    def setRecordTypeRef(self, recordTypeRef):
        self.__recordTypeRef = recordTypeRef

    def getRecordTypeRef(self):
        return self.__recordTypeRef

    def setRecordReferenceRef(self, recordReferenceRef):
        self.__recordReferenceRef = recordReferenceRef

    def getRecordReferenceRef(self):
        return self.__recordReferenceRef
    
    def getID(self):
        return self.getRecordReferenceRef().getID()


class ResolvedFunctionRefArgumentNode(ArgumentNode):
    '''
    classdocs
    '''
    
    __functionRef = None
    
    def __init__(self, *args, **kwargs):
        super(ResolvedFunctionRefArgumentNode, self).__init__(*args, **kwargs)

    def setFunctionRef(self, functionRef):
        self.__functionRef = functionRef

    def getFunctionRef(self):
        return self.__functionRef


#
# Runtime Components
# 

class AbstractRuntimeComponentNode(ArgumentNode):
    '''
    classdocs
    '''
    
    __arguments = {}
    
    def __init__(self, *args, **kwargs):
        super(AbstractRuntimeComponentNode, self).__init__(*args, **kwargs)
        self.__arguments = {}
        
    def accept(self, visitor):
        visitor.preVisit(self)
        availableArgKeys = self.__arguments.keys() 
        for argKey in filter(lambda k: k in availableArgKeys, map(lambda a: a['key'], self.getXMLArguments())):
            self.__arguments[argKey].accept(visitor)
        visitor.postVisit(self)
    
    def getIncludePath(self):
        raise RuntimeError("Calling abstract AbstractRuntimeComponentNode::getIncludePath() method")
    
    def getCxtRecordType(self):
        raise RuntimeError("Calling abstract AbstractRuntimeComponentNode::getCxtRecordType() method")
    
    def getConcreteType(self):
        raise RuntimeError("Calling abstract AbstractRuntimeComponentNode::getConcreteType() method")
        
    def getXMLArguments(self):
        raise RuntimeError("Calling abstract AbstractRuntimeComponentNode::getXMLArguments() method")
        
    def getConstructorArguments(self):
        raise RuntimeError("Calling abstract AbstractRuntimeComponentNode::getXMLArguments() method")
        
    def setArgument(self, node):
        self.__arguments[node.getAttribute('key')] = node
        node.setParent(self)
    
    def getArgument(self, key):
        return self.__arguments.get(key)
    
    def hasArgument(self, key):
        return self.__arguments.has_key(key)
    
    @classmethod
    def parseType(cls, type):
        raise ParseTypeError("Calling abstract AbstractRuntimeComponentNode::parseType() method")

#
# Setters
# 
 
class AbstractSetterNode(AbstractRuntimeComponentNode):
    '''
    classdocs
    '''
    
    orderkey = None
    
    def __init__(self, *args, **kwargs):
        super(AbstractSetterNode, self).__init__(*args, **kwargs)
        self.orderkey = None
        
    def setOrderKey(self, key):
        self.orderkey = key
        
    def isInvertible(self):
        raise RuntimeError("Calling abstract AbstractSetterNode::isInvertible() method")
    
    def getCxtRecordType(self):
        return self.getParent().getCxtRecordType()


class FieldSetterNode(AbstractSetterNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^field_setter$')
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="FieldSetter")
        super(FieldSetterNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/setter/FieldSetter.h"
        
    def isInvertible(self):
        return False # FIXME: implement
    
    def getConcreteType(self):
        # template<class RecordType, I16u fid, class ValueProviderType>
        recordType = self.getCxtRecordType() 
        fid = self.getArgument("field").getID()
        valueProviderType = self.getArgument("value").getAttribute("type_alias")
        
        return "Myriad::FieldSetter< %s, %s, %s >" % (recordType, fid, valueProviderType)
        
    def getXMLArguments(self):
        return [ { 'key': 'field', 'type': 'field_ref' }, 
                 { 'key': 'value', 'type': 'value_provider' }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'RuntimeComponentRef(value)' ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


class ReferenceSetterNode(AbstractSetterNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^reference_setter$')
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ReferenceSetter")
        super(ReferenceSetterNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/setter/ReferenceSetter.h"
        
    def isInvertible(self):
        return False # FIXME: implement
    
    def getConcreteType(self):
        # template<class RecordType, I16u fid, class ReferenceProviderType>
        recordType = self.getCxtRecordType()
        fid = self.getArgument("reference").getID()
        referenceProviderType = self.getArgument("value").getAttribute("type_alias")
        
        return "Myriad::ReferenceSetter< %s, %s, %s >" % (recordType, fid, referenceProviderType)
        
    def getXMLArguments(self):
        return [ { 'key': 'reference', 'type': 'reference_ref' }, 
                 { 'key': 'value', 'type': 'reference_provider' }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'RuntimeComponentRef(value)' ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


#
# Runtime Components: Value Providers 
# 

class AbstractValueProviderNode(AbstractRuntimeComponentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(AbstractValueProviderNode, self).__init__(*args, **kwargs)
    
    def getValueType(self):
#       return self.getArgument("probability").getFunctionRef().getDomainType()
#       return self.getArgument("value").getAttribute("type")
#       return self.getArgument("field").getFieldRef().getAttribute("type")
#       return self.getArgument("probability").getFunctionRef().getDomainType()
        return self.getAttribute("value_type")
    
    def getCxtRecordType(self):
        if isinstance(self.getParent(), ClusteredReferenceProviderNode) and self.getAttribute("key") == "children_count":
            return self.getParent().getRefRecordType()
        else:
            return self.getParent().getCxtRecordType()


class ElementWiseValueProviderNode(AbstractValueProviderNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^element_wise_value_provider\[(%s)\]$' % ('|'.join(AstUtils.literalTypes())))
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ElementWiseValueProvider")
        super(ElementWiseValueProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/value/ElementWiseValueProvider.h"
    
    def getValueType(self):
#       return "vector<%s>" % (self.getArgument("element_value_provider").getValueType())
        return "vector<%s>" % self.getAttribute("range_type")
    
    def getConcreteType(self):
        # template<typename ValueType, class CxtRecordType>
        valueType = self.getArgument("element_value_provider").getValueType()
        valueVectorSize = self.getParent().getArgument("field").getFieldRef().vectorTypeSize()
        cxtRecordType = self.getCxtRecordType()
        
        return "Myriad::ElementWiseValueProvider< %s, %s, %s >" % (valueType, cxtRecordType, valueVectorSize)
    
    def getXMLArguments(self):
        return [ { 'key': 'element_value_provider', 'type': 'value_provider' }, 
                 { 'key': 'element_length_value_provider', 'type': 'value_provider' }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'RuntimeComponentRef(element_value_provider)',
                 'RuntimeComponentRef(element_length_value_provider)'
               ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], { 'value_type': m.group(1) }
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


class CallbackValueProviderNode(AbstractValueProviderNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^callback_value_provider\[(%s)\]$' % ('|'.join(AstUtils.literalTypes())))
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="CallbackValueProvider")
        super(CallbackValueProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/value/CallbackValueProvider.h"
    
    def getConcreteType(self):
        # template<typename ValueType, class CxtRecordType, class CallbackType>
        valueType = self.getValueType()
        cxtRecordType = self.getCxtRecordType()
        callbackType = "Base%sSetterChain" % (cxtRecordType)
        
        return "Myriad::CallbackValueProvider< %s, %s, %s >" % (valueType, cxtRecordType, callbackType)
        
    def getXMLArguments(self):
        return [ { 'key': 'name' , 'type': 'String' }, 
                 { 'key': 'arity', 'type': 'I16u' }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'Verbatim(*this)',
                 'Verbatim(&Base%sSetterChain::%s)' % (self.getCxtRecordType(), self.getArgument("name").getAttribute("value")),
                 'Literal(arity)'
               ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], { 'value_type': m.group(1) }
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


class ClusteredValueProviderNode(AbstractValueProviderNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^clustered_value_provider\[(%s)\]$' % ('|'.join(AstUtils.literalTypes())))
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ClusteredValueProvider")
        super(ClusteredValueProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/value/ClusteredValueProvider.h"
    
    def getConcreteType(self):
        # template<typename ValueType, class CxtRecordType, class PrFunctionType, class RangeProviderType>
        valueType = self.getValueType()
        cxtRecordType = self.getCxtRecordType()
        probabilityType = self.getArgument("probability").getFunctionRef().getAttribute("concrete_type")
        rangeProviderType = self.getArgument("cardinality").getAttribute("type_alias")
        
        return "Myriad::ClusteredValueProvider< %s, %s, %s, %s >" % (valueType, cxtRecordType, probabilityType, rangeProviderType)
        
    def getXMLArguments(self):
        return [ { 'key': 'probability', 'type': 'function_ref' }, 
                 { 'key': 'cardinality', 'type': 'range_provider' }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'FunctionRef(probability)', 
                 'RuntimeComponentRef(cardinality)' 
               ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], { 'value_type': m.group(1) }
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


class ConstValueProviderNode(AbstractValueProviderNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^const_value_provider\[(%s)\]$' % ('|'.join(AstUtils.literalTypes())))
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ConstValueProvider")
        super(ConstValueProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/value/ConstValueProvider.h"
    
    def getConcreteType(self):
        # template<typename ValueType, class CxtRecordType>
        valueType = self.getValueType()
        cxtRecordType = self.getCxtRecordType()
        
        return "Myriad::ConstValueProvider< %s, %s >" % (valueType, cxtRecordType)
        
    def getXMLArguments(self):
        return [ { 'key': 'value', 'type': self.getValueType() } 
               ]
        
    def getConstructorArguments(self):
        return [ 'Literal(value)' 
               ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], { 'value_type': m.group(1) }
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


class ContextFieldValueProviderNode(AbstractValueProviderNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^context_field_value_provider\[(%s)\]$' % ('|'.join(AstUtils.literalTypes())))
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ContextFieldValueProvider")
        super(ContextFieldValueProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/value/ContextFieldValueProvider.h"
    
    def getConcreteType(self):
        # template<typename ValueType, class CxtRecordType, I16u fid>
        valueType = self.getValueType()
        cxtRecordType = self.getCxtRecordType()
        fids = [ x.getID() for x in self.getArgument("field").getInnerPathRefs() ] # inner path
        fids.append(self.getArgument("field").getID()) # field path
        fids.extend(['0'] * (3 - len(fids)))
        
        return "Myriad::ContextFieldValueProvider< %s, %s, %s >" % (valueType, cxtRecordType, ", ".join(fids))
        
    def getXMLArguments(self):
        return [ { 'key': 'field', 'type': 'field_ref' } 
               ]
        
    def getConstructorArguments(self):
        return []
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], { 'value_type': m.group(1) }
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


class RandomValueProviderNode(AbstractValueProviderNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^random_value_provider\[(%s)\]$' % ('|'.join(AstUtils.literalTypes())))
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="RandomValueProvider")
        super(RandomValueProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/value/RandomValueProvider.h"
    
    def getConcreteType(self):
        # template<typename ValueType, class CxtRecordType, class PrFunctionType, I16u conditionFID>
        valueType = self.getValueType()
        cxtRecordType = self.getCxtRecordType()
        probabilityType = self.getArgument("probability").getFunctionRef().getAttribute("concrete_type")
        if self.hasArgument("condition_field"):
            condFieldID = self.getArgument("condition_field").getID() # field path 
        else:
            condFieldID = '0'
        
        return "Myriad::RandomValueProvider< %s, %s, %s, %s >" % (valueType, cxtRecordType, probabilityType, condFieldID)
    
    def getXMLArguments(self):
        return [ { 'key': 'probability'    , 'type': 'function_ref' }, 
                 { 'key': 'condition_field', 'type': 'field_ref', 'optional': True }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'FunctionRef(probability)'
               ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], { 'value_type': m.group(1) }
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


#
# Runtime Components: Range Providers 
# 

class AbstractRangeProviderNode(AbstractRuntimeComponentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(AbstractRangeProviderNode, self).__init__(*args, **kwargs)
    
    def getCxtRecordType(self):
        return self.getParent().getCxtRecordType()
    
    def getRangeType(self):
        return self.getAttribute("range_type")


class ConstRangeProviderNode(AbstractRangeProviderNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^const_range_provider\[(%s)\]$' % ('|'.join(AstUtils.literalTypes())))
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ConstRangeProvider")
        super(ConstRangeProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/range/ConstRangeProvider.h"
    
    def getConcreteType(self):
        # template<typename RangeType, class CxtRecordType>
        rangeType = self.getRangeType()
        cxtRecordType = self.getCxtRecordType()
        
        return "Myriad::ConstRangeProvider< %s, %s >" % (rangeType, cxtRecordType)
        
    def getXMLArguments(self):
        return [ { 'key': 'min', 'type': self.getRangeType() },
                 { 'key': 'max', 'type': self.getRangeType() }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'Literal(min)', 
                 'Literal(max)' 
               ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], { 'range_type': m.group(1) }
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


class ContextFieldRangeProviderNode(AbstractRangeProviderNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^context_field_range_provider\[(%s)\]$' % ('|'.join(AstUtils.literalTypes())))
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ContextFieldRangeProvider")
        super(ContextFieldRangeProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/range/ContextFieldRangeProvider.h"
    
    def getConcreteType(self):
        # template<typename RangeType, class CxtRecordType, class InvertibleFieldSetterType>
        rangeType = self.getRangeType()
        cxtRecordType = self.getCxtRecordType()
        fieldSetterType = self.getArgument("field").getFieldRef().getSetter().getAttribute("type_alias")
        
        return "Myriad::ContextFieldRangeProvider< %s, %s, %s >" % (rangeType, cxtRecordType, fieldSetterType)
        
    def getXMLArguments(self):
        return [ { 'key': 'field', 'type': 'field_ref' }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'FieldSetterRef(field)'
               ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], { 'range_type': m.group(1) }
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


#
# Runtime Components: Predicate Providers 
# 

class AbstractPredicateProviderNode(AbstractRuntimeComponentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(AbstractPredicateProviderNode, self).__init__(*args, **kwargs)
    
    def getCxtRecordType(self):
        return self.getParent().getCxtRecordType()
    
    def getRefRecordType(self):
        return self.getParent().getRefRecordType()


class EqualityPredicateProviderNode(AbstractPredicateProviderNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^equality_predicate_provider$')
    
    def __init__(self, *args, **kwargs):
        super(EqualityPredicateProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/predicate/EqualityPredicateProvider.h"
    
    def getConcreteType(self):
        # template<typename RangeType, class CxtRecordType, class InvertibleFieldSetterType>
        refRecordType = self.getRefRecordType()
        cxtRecordType = self.getCxtRecordType()
        
        return "Myriad::EqualityPredicateProvider< %s, %s >" % (refRecordType, cxtRecordType)
        
    def getXMLArguments(self):
        return [ { 'key': 'binder', 'type': 'collection[binder]' },
               ]
        
    def getConstructorArguments(self):
        return [ 'Verbatim(config.generatorPool().get<%sGenerator>().recordFactory())' % (self.getRefRecordType()),
                 'RuntimeComponentRef(binder)'
               ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


#
# Runtime Components: Binders 
# 

class AbstractFieldBinderNode(AbstractRuntimeComponentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(AbstractFieldBinderNode, self).__init__(*args, **kwargs)
    
    def getCxtRecordType(self):
        parent = self.getParent()
        while parent is not None and not isinstance(parent, AbstractPredicateProviderNode):
            parent = parent.getParent()
        if parent is None:
            raise RuntimeException("Could not determine CxtRecordType of AbstractFieldBinderNode (parent is None)")
        return parent.getCxtRecordType()
    
    def getRefRecordType(self):
        parent = self.getParent()
        while parent is not None and not isinstance(parent, AbstractPredicateProviderNode):
            parent = parent.getParent()
        if parent is None:
            raise RuntimeException("Could not determine RefRecordType of AbstractFieldBinderNode (parent is None)")
        return parent.getRefRecordType()


class EqualityPredicateFieldBinderNode(AbstractFieldBinderNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^predicate_value_binder$')
    
    def __init__(self, *args, **kwargs):
        super(EqualityPredicateFieldBinderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/predicate/EqualityPredicateFieldBinder.h"
    
    def getConcreteType(self):
        # template<class RecordType, I16u fid, class CxtRecordType, class ValueProviderType>
        refRecordType = self.getRefRecordType()
        fid = self.getArgument("field").getID() # direct field
        cxtRecordType = self.getCxtRecordType()
        valueProviderType = self.getArgument("value").getAttribute("type_alias")
        
        return "Myriad::EqualityPredicateFieldBinder< %s, %s, %s, %s >" % (refRecordType, fid, cxtRecordType, valueProviderType)
        
    def getXMLArguments(self):
        return [ { 'key': 'field', 'type': 'field_ref' },
                 { 'key': 'value', 'type': 'value_provider' } 
               ]
        
    def getConstructorArguments(self):
        return [ 'RuntimeComponentRef(value)'
               ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


#
# Runtime Components: Reference Providers 
# 

class AbstractReferenceProviderNode(AbstractRuntimeComponentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(AbstractReferenceProviderNode, self).__init__(*args, **kwargs)
    
    def getRefRecordType(self):
        return self.getParent().getArgument("reference").getAttribute("type")
    
    def getCxtRecordType(self):
        return self.getParent().getCxtRecordType()


class ClusteredReferenceProviderNode(AbstractReferenceProviderNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^clustered_reference_provider$')
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ClusteredReferenceProvider")
        super(ClusteredReferenceProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/reference/ClusteredReferenceProvider.h"
    
    def getConcreteType(self):
        # template<typename RefRecordType, class CxtRecordType, class ChildrenCountValueProviderType, I16u posFieldID = 0>
        refRecordType = self.getRefRecordType()
        cxtRecordType = self.getCxtRecordType()
        childrenCountType = self.getArgument("children_count").getAttribute("type_alias")
        if self.hasArgument("position_field"):
            posFieldID = self.getArgument("position_field").getID() # direct field
        else:
            posFieldID = '0'
        
        return "Myriad::ClusteredReferenceProvider< %s, %s, %s, %s >" % (refRecordType, cxtRecordType, childrenCountType, posFieldID)
        
    def getXMLArguments(self):
        return [ { 'key': 'children_count', 'type': 'value_provider' }, 
                 { 'key': 'children_count_max', 'type': 'value_provider' },
                 { 'key': 'position_field', 'type': 'field_ref', 'optional': True }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'RuntimeComponentRef(children_count_max)', 
                 'RuntimeComponentRef(children_count)', 
                 'SequenceInspector(%s)' % (self.getRefRecordType()), 
               ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


class RandomReferenceProviderNode(AbstractReferenceProviderNode):
    '''
    classdocs
    '''
    
    # the type pattern for this class
    __type_pattern = re.compile('^random_reference_provider$')
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="RandomReferenceProvider")
        super(RandomReferenceProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/reference/RandomReferenceProvider.h"
    
    def getConcreteType(self):
        # template<typename RefRecordType, class CxtRecordType>
        refRecordType = self.getRefRecordType()
        cxtRecordType = self.getCxtRecordType()
        
        return "Myriad::RandomReferenceProvider< %s, %s >" % (refRecordType, cxtRecordType)
        
    def getXMLArguments(self):
        return [ { 'key': 'predicate', 'type': 'equality_predicate_provider' }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'RuntimeComponentRef(predicate)', 
                 'SequenceInspector(%s)' % (self.getRefRecordType()), 
               ]
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-6])


#
# AST traverse trategies
# 

class DepthFirstNodeFilter(AbstractVisitor):
    '''
    classdocs
    '''
    
    __filterType = None
    __filteredNodes = []
    
    def __init__(self, *args, **kwargs):
        super(DepthFirstNodeFilter, self).__init__(*args, **kwargs)
        self.__filterType = kwargs.get("filterType")
    
    def _postVisitAbstractNode(self, node):
        if isinstance(node, self.__filterType):
            self.__filteredNodes.append(node)
        
    def getAll(self, astRoot):
        # empty current list
        del self.__filteredNodes[:]
        # traverse the AST to build the iterator array
        astRoot.accept(self)
        # return the filtered nodes
        return self.__filteredNodes

