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

from myriad.compiler.visitor import AbstractVisitor
from myriad.util.stringutil import StringTransformer
from decimal import Decimal

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

    
class FunctionNode(AbstractNode):
    '''
    classdocs
    '''
    
    __arguments = {}
    
    orderkey = None
    
    def __init__(self, *args, **kwargs):
        if not kwargs.has_key("concrete_type"):
            kwargs.update(concrete_type=kwargs["type"])
        super(FunctionNode, self).__init__(*args, **kwargs)
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
    
    def getArgument(self, key):
        return self.__arguments.get(key)
        
    def setOrderKey(self, key):
        self.orderkey = key
    
    def getConcreteType(self):
        return self.getAttribute("concrete_type")
        
    def getDomainType(self):
        raise RuntimeError("Calling abstract FunctionNode::getDomainType() method")
    
    def getXMLArguments(self):
        return {}
    
    def getConstructorArguments(self):
        return [ 'Literal(key)' ]

    

class ParetoProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="ParetoPrFunction")
        super(ParetoProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getDomainType(self):
        return 'Decimal'
        
    def getXMLArguments(self):
        return [ { 'key': 'x_min', 'type': 'literal' }, 
                 { 'key': 'alpha', 'type': 'literal' } 
               ]
        
    def getConstructorArguments(self):
        args = super(ParetoProbabilityFunctionNode, self).getConstructorArguments()
        args.extend(['Literal(x_min)',
                     'Literal(alpha)'
                     ])
        return args

    
class NormalProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="NormalPrFunction")
        super(NormalProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getDomainType(self):
        return self.getArgument('mean').getAttribute('type')
        
    def getXMLArguments(self):
        return [ { 'key': 'mean', 'type': 'literal' }, 
                 { 'key': 'stddev', 'type': 'literal' } 
               ]
        
    def getConstructorArguments(self):
        args = super(NormalProbabilityFunctionNode, self).getConstructorArguments()
        args.extend(['Literal(mean)',
                     'Literal(stddev)'
                     ])
        return args


class UniformProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="UniformPrFunction")
        super(UniformProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getDomainType(self):
        return self.getArgument('x_min').getAttribute('type')
        
    def getXMLArguments(self):
        return [ { 'key': 'x_min', 'type': 'literal' }, 
                 { 'key': 'x_max', 'type': 'literal' } 
               ]
        
    def getConstructorArguments(self):
        args = super(UniformProbabilityFunctionNode, self).getConstructorArguments()
        args.extend(['Literal(x_min)',
                     'Literal(x_max)'
                     ])
        return args
    

class CombinedProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="CombinedPrFunction")
        kwargs.update(concrete_type="CombinedPrFunction<%s>" % kwargs["domain_type"])
        super(CombinedProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getDomainType(self):
        return self.getAttribute('domain_type')
        
    def getXMLArguments(self):
        return [ { 'key': 'path', 'type': 'literal' } 
               ]
        
    def getConstructorArguments(self):
        args = super(CombinedProbabilityFunctionNode, self).getConstructorArguments()
        args.extend(['Literal(path)'])
        return args
    

class ConditionalCombinedProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="ConditionalCombinedPrFunction")
        kwargs.update(concrete_type="ConditionalCombinedPrFunction<%s, %s>" % (kwargs["domain_type1"], kwargs["domain_type2"]))
        super(ConditionalCombinedProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getDomainType(self):
        return self.getAttribute('domain_type1')
        
    def getXMLArguments(self):
        return [ { 'key': 'path', 'type': 'literal' } 
               ]
        
    def getConstructorArguments(self):
        args = super(ConditionalCombinedProbabilityFunctionNode, self).getConstructorArguments()
        args.extend(['Literal(path)'])
        return args


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
        return [ { 'key': 'path', 'type': 'literal' } 
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
    
    def __init__(self, *args, **kwargs):
        super(RecordSequenceNode, self).__init__(*args, **kwargs)
    
    def accept(self, visitor):
        visitor.preVisit(self)
        self._recordType.accept(visitor)
        visitor.postVisit(self)
        
    def setRecordType(self, node):
        self._recordType = node
        
    def getRecordType(self):
        return self._recordType


class RandomSequenceNode(RecordSequenceNode):
    '''
    classdocs
    '''
    
    __cardinalityEstimator = None
    __setterChain = None
    __hydrators = None
    __hydrationPlan = None
    __sequenceIterator = None
    
    def __init__(self, *args, **kwargs):
        super(RandomSequenceNode, self).__init__(*args, **kwargs)
        self.__cardinalityEstimator = None
        self.__setterChain = None
        self.__hydrators = None
        self.__hydrationPlan = None
        self.__sequenceIterator = None
    
    def accept(self, visitor):
        visitor.preVisit(self)
        self._recordType.accept(visitor)
        if self.__setterChain is not None:
            self.__setterChain.accept(visitor)
        if self.__hydrators is not None:
            self.__hydrators.accept(visitor)
            self.__hydrationPlan.accept(visitor)
        self.__cardinalityEstimator.accept(visitor)
        if self.__sequenceIterator is not None:
            self.__sequenceIterator.accept(visitor)
        visitor.postVisit(self)
        
    def setSetterChain(self, node):
        self.__setterChain = node
        node.setParent(self)
        
    def getSetterChain(self):
        return self.__setterChain
        
    def setHydrators(self, node):
        self.__hydrators = node
        
    def getHydrators(self):
        return self.__hydrators
        
    def setHydrationPlan(self, node):
        self.__hydrationPlan = node
        
    def getHydrationPlan(self):
        return self.__hydrationPlan
        
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
    
    def getField(self, key):
        return self._fields.get(key)
    
    def getFields(self):
        return self._fields.itervalues()

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
        return self._references.itervalues()

    def hasReferences(self):
        return bool(self._references)
    
    def getReferenceTypes(self):
        return self._referenceTypes
        
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


class HydratorsNode(AbstractNode):
    '''
    classdocs
    '''
    
    __hydrators = {}
    
    def __init__(self, *args, **kwargs):
        super(HydratorsNode, self).__init__(*args, **kwargs)
        self.__hydrators = {}
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in self.__hydrators.itervalues():
            node.accept(visitor)
        visitor.postVisit(self)
        
    def setHydrator(self, node):
        self.__hydrators[node.getAttribute('key')] = node
    
    def getHydrator(self, key):
        return self.__hydrators.get(key)
    
    def hasHydrator(self, key):
        return self.__hydrators.has_key(key)
    
    def getAll(self):
        return self.__hydrators.itervalues()


class HydrationPlanNode(AbstractNode):
    '''
    classdocs
    '''
    
    _hydrators = []
    
    def __init__(self, *args, **kwargs):
        super(HydrationPlanNode, self).__init__(*args, **kwargs)
        self._hydrators = []
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in self._hydrators:
            node.accept(visitor)
        visitor.postVisit(self)
        
    def addHydrator(self, node):
        self._hydrators.append(node)
        
    def getAll(self):
        return self._hydrators
    

#
# Hydrators
# 
 
class HydratorNode(AbstractNode):
    '''
    classdocs
    '''
    
    __arguments = {}
    
    orderkey = None
    
    def __init__(self, *args, **kwargs):
        super(HydratorNode, self).__init__(*args, **kwargs)
        self.__arguments = {}
        self.orderkey = None
    
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
        
    def setOrderKey(self, key):
        self.orderkey = key
        
    def getConcreteType(self):
        return "RecordHydrator"
        
    def isInvertible(self):
        return False
    
    def getXMLArguments(self):
        return {}
    
    def getConstructorArguments(self):
        return []


class ClusteredReferenceHydratorNode(HydratorNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ClusteredReferenceHydrator")
        super(ClusteredReferenceHydratorNode, self).__init__(*args, **kwargs)
    
    def getConcreteType(self):
        recordType = StringTransformer.us2ccAll(self.getArgument("field").getRecordTypeRef().getAttribute("key"))
        refRecordType = StringTransformer.us2ccAll(self.getArgument("field").getRecordReferenceRef().getRecordTypeRef().getAttribute("key"))
        
        return "ClusteredReferenceHydrator< %s, %s >" % (recordType, refRecordType)
        
    def getXMLArguments(self):
        return [ { 'key': 'field', 'type': 'field_ref' }, 
                 { 'key': 'position_field', 'type': 'field_ref', 'optional': True }, 
                 { 'key': 'count_field', 'type': 'field_ref' }, 
                 { 'key': 'nested_cardinality', 'type': 'literal' } 
               ]
        
    def getConstructorArguments(self):
        return [ 'FieldSetter(field)',
                 'FieldSetter(position_field)*',
                 'FieldGetter(count_field)',
                 'RandomSetInspector(count_field)',
                 'Literal(nested_cardinality)' 
               ]


class ConditionalRandomizedHydratorNode(HydratorNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ConditionalRandomizedHydrator")
        super(ConditionalRandomizedHydratorNode, self).__init__(*args, **kwargs)
    
    def getConcreteType(self):
        recordType = StringTransformer.us2ccAll(self.getArgument("field").getRecordTypeRef().getAttribute("key"))
        fieldType = self.getArgument("field").getFieldRef().getAttribute("type")
        conditionFieldType = self.getArgument("condition_field").getFieldRef().getAttribute("type")
        probabilityType = self.getArgument("probability").getFunctionRef().getAttribute("concrete_type")
        
        return "ConditionalRandomizedHydrator< %s, %s, %s, %s >" % (recordType, fieldType, conditionFieldType, probabilityType)
        
    def getXMLArguments(self):
        return [ { 'key': 'field', 'type': 'field_ref' }, 
                 { 'key': 'condition_field', 'type': 'field_ref' }, 
                 { 'key': 'probability', 'type': 'function_ref' } 
               ]
        
    def getConstructorArguments(self):
        return [ 'EnvVariable(random)',
                 'FieldSetter(field)',
                 'FieldGetter(condition_field)',
                 'FunctionRef(probability)' 
               ]


class ConstValueHydratorNode(HydratorNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ConstValueHydrator")
        super(ConstValueHydratorNode, self).__init__(*args, **kwargs)
    
    def getConcreteType(self):
        recordType = StringTransformer.us2ccAll(self.getArgument("field").getRecordTypeRef().getAttribute("key"))
        fieldType = self.getArgument("field").getFieldRef().getAttribute("type")
        
        return "ConstValueHydrator< %s, %s >" % (recordType, fieldType)
        
    def getXMLArguments(self):
        return [ { 'key': 'field', 'type': 'field_ref' }, 
                 { 'key': 'const_value', 'type': 'literal' }
               ]
        
    def getConstructorArguments(self):
        return [ 'FieldSetter(field)',
                 'Literal(const_value)'
               ]
        
    def getConstructorArgumentsOrder(self):
        return ['field', 'const_value']


class ReferencedRecordHydratorNode(HydratorNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ReferencedRecordHydrator")
        super(ReferencedRecordHydratorNode, self).__init__(*args, **kwargs)
    
    def getConcreteType(self):
        recordType = StringTransformer.us2ccAll(self.getArgument("field").getRecordTypeRef().getAttribute("key"))
        refRecordType = StringTransformer.us2ccAll(self.getArgument("field").getRecordReferenceRef().getRecordTypeRef().getAttribute("key"))
        fieldType = self.getArgument("pivot_field").getFieldRef().getAttribute("type")
        probabilityType = self.getArgument("probability").getFunctionRef().getAttribute("type")
        
        return "ReferencedRecordHydrator< %s, %s, %s, %s >" % (recordType, refRecordType, fieldType, probabilityType)
        
    def getXMLArguments(self):
        return [ { 'key': 'field', 'type': 'field_ref' }, 
                 { 'key': 'pivot_field', 'type': 'field_ref' }, 
                 { 'key': 'probability', 'type': 'function_ref' } 
               ]
        
    def getConstructorArguments(self):
        return [ 'EnvVariable(random)',
                 'FieldSetter(field)',
                 'FieldSetter(pivot_field)',
                 'RandomSetInspector(pivot_field)',
                 'FunctionRef(probability)' 
               ]


class ReferenceHydratorNode(HydratorNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ReferenceHydrator")
        super(ReferenceHydratorNode, self).__init__(*args, **kwargs)
    
    def getConcreteType(self):
        recordType = StringTransformer.us2ccAll(self.getArgument("field").getRecordTypeRef().getAttribute("key"))
        refRecordType = StringTransformer.us2ccAll(self.getArgument("field").getRecordReferenceRef().getRecordTypeRef().getAttribute("key"))
        # TODO: this is a quick and dirty hack, a better solution is needed here
        if isinstance(self.getArgument("pivot_field"), ResolvedDirectFieldRefArgumentNode):
            fieldType = self.getArgument("pivot_field").getFieldRef().getAttribute("type")
        elif isinstance(self.getArgument("pivot_field"), ResolvedRecordReferenceRefArgumentNode):
            fieldType = 'I64u'
        
        return "ReferenceHydrator< %s, %s, %s >" % (recordType, refRecordType, fieldType)
        
    def getXMLArguments(self):
        return [ { 'key': 'field', 'type': 'field_ref' }, 
                 { 'key': 'pivot_field', 'type': 'field_ref' }, 
                 { 'key': 'pivot_value', 'type': 'field_ref' } 
               ]
        
    def getConstructorArguments(self):
        return [ 'EnvVariable(random)',
                 'FieldSetter(field)',
                 'FieldSetter(pivot_field)',
                 'RandomSetInspector(pivot_field)',
                 'FieldGetter(pivot_value)' 
               ]


class SimpleClusteredHydratorNode(HydratorNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="SimpleClusteredHydrator")
        super(SimpleClusteredHydratorNode, self).__init__(*args, **kwargs)
    
    def getConcreteType(self):
        recordType = StringTransformer.us2ccAll(self.getArgument("field").getRecordTypeRef().getAttribute("key"))
        fieldType = self.getArgument("field").getFieldRef().getAttribute("type")
        probabilityType = self.getArgument("probability").getFunctionRef().getAttribute("concrete_type")
        
        return "SimpleClusteredHydrator< %s, %s, %s >" % (recordType, fieldType, probabilityType)
        
    def isInvertible(self):
        return True
        
    def getXMLArguments(self):
        return [ { 'key': 'field', 'type': 'field_ref' }, 
                 { 'key': 'probability', 'type': 'function_ref' }, 
                 { 'key': 'sequence_cardinality', 'type': 'literal' } 
               ]
        
    def getConstructorArguments(self):
        return [ 'FieldSetter(field)',
                 'FunctionRef(probability)',
                 'Literal(sequence_cardinality)' 
               ]


class SimpleRandomizedHydratorNode(HydratorNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="SimpleRandomizedHydrator")
        super(SimpleRandomizedHydratorNode, self).__init__(*args, **kwargs)
    
    def getConcreteType(self):
        recordType = StringTransformer.us2ccAll(self.getArgument("field").getRecordTypeRef().getAttribute("key"))
        fieldType = self.getArgument("field").getFieldRef().getAttribute("type")
        probabilityType = self.getArgument("probability").getFunctionRef().getAttribute("concrete_type")
        
        return "SimpleRandomizedHydrator< %s, %s, %s >" % (recordType, fieldType, probabilityType)
        
    def getXMLArguments(self):
        return [ { 'key': 'field', 'type': 'field_ref' }, 
                 { 'key': 'probability', 'type': 'function_ref' }
               ]
        
    def getConstructorArguments(self):
        return [ 'EnvVariable(random)',
                 'FieldSetter(field)',
                 'FunctionRef(probability)'
               ]


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
        comopnentPaths = [ node.getIncludePath() for node in nodeFilter.getAll(self) ]
        return sorted(set(comopnentPaths))

#
# Cardinality Estimators
# 

class CardinalityEstimatorNode(AbstractNode):
    '''
    classdocs
    '''
    
    __parent = None
    __arguments = {}
    
    def __init__(self, *args, **kwargs):
        super(CardinalityEstimatorNode, self).__init__(*args, **kwargs)
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


class LinearScaleEstimatorNode(CardinalityEstimatorNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="linear_scale_estimator")
        super(LinearScaleEstimatorNode, self).__init__(*args, **kwargs)
        
    def getXMLArguments(self):
        return [ { 'key': 'base_cardinality', 'type': 'literal' } 
               ]
        
    def getConstructorArguments(self):
        return [ 'Literal(base_cardinality)' 
               ]


#
# Sequence Iterators
# 
 
class SequenceIteratorNode(AbstractNode):
    '''
    classdocs
    '''
    
    __arguments = {}
    __parent = None
    
    def __init__(self, *args, **kwargs):
        super(SequenceIteratorNode, self).__init__(*args, **kwargs)
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
        return "SequenceIterator"
        
    def getXMLArguments(self):
        return {}
        
    def getConstructorArguments(self):
        return [ 'EnvVariable(*this)',
                 'EnvVariable(_config)' 
               ]


class PartitionedSequenceIteratorNode(SequenceIteratorNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="RandomSetDefaultGeneratingTask")
        super(PartitionedSequenceIteratorNode, self).__init__(*args, **kwargs)
        
    def getConcreteType(self):
        recordType = StringTransformer.us2ccAll(self.getParent().getAttribute("key"))

        return "RandomSetDefaultGeneratingTask< %s >" % (recordType)


#
# Arguments
# 
    
class ArgumentNode(AbstractNode):
    '''
    classdocs
    '''
    
    __parent = None
    
    def __init__(self, *args, **kwargs):
        super(ArgumentNode, self).__init__(*args, **kwargs)

    def setParent(self, parent):
        self.__parent = parent

    def getParent(self):
        return self.__parent


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


class UnresolvedFunctionRefArgumentNode(ArgumentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(UnresolvedFunctionRefArgumentNode, self).__init__(*args, **kwargs)


class UnresolvedHydratorRefArgumentNode(ArgumentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(UnresolvedHydratorRefArgumentNode, self).__init__(*args, **kwargs)


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
    
    def __init__(self, *args, **kwargs):
        super(ResolvedFieldRefArgumentNode, self).__init__(*args, **kwargs)


class ResolvedDirectFieldRefArgumentNode(ResolvedFieldRefArgumentNode):
    '''
    classdocs
    '''
    
    __fieldRef = None
    __recordTypeRef = None
    
    def __init__(self, *args, **kwargs):
        super(ResolvedDirectFieldRefArgumentNode, self).__init__(*args, **kwargs)
        self.__fieldRef = None
        self.__recordTypeRef = None

    def setRecordTypeRef(self, recordTypeRef):
        self.__recordTypeRef = recordTypeRef

    def getRecordTypeRef(self):
        return self.__recordTypeRef

    def setFieldRef(self, fieldRef):
        self.__fieldRef = fieldRef

    def getFieldRef(self):
        return self.__fieldRef
    
    def getFieldID(self):
        return "RecordTraits<%s>::%s" % (StringTransformer.us2ccAll(self.getRecordTypeRef().getAttribute("key")), self.getFieldRef().getAttribute("name").upper())


class ResolvedRecordReferenceRefArgumentNode(ResolvedFieldRefArgumentNode):
    '''
    classdocs
    '''
    
    __recordTypeRef = None
    __recordReferenceRef = None
    
    def __init__(self, *args, **kwargs):
        super(ResolvedRecordReferenceRefArgumentNode, self).__init__(*args, **kwargs)
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
    
    def getFieldID(self):
        referenceKey = self.getAttribute('ref')
        if referenceKey.find(":") > -1:
            referenceKey = referenceKey[referenceKey.find(":")+1:]
        return "RecordTraits<%s>::%s" % (StringTransformer.us2ccAll(self.getRecordTypeRef().getAttribute("key")), referenceKey.upper())


class ResolvedReferencedFieldRefArgumentNode(ResolvedFieldRefArgumentNode):
    '''
    classdocs
    '''
    
    __recordTypeRef = None
    __recordReferenceRef = None
    __fieldRef = None
    
    def __init__(self, *args, **kwargs):
        super(ResolvedReferencedFieldRefArgumentNode, self).__init__(*args, **kwargs)
        self.__fieldRef = None
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

    def setFieldRef(self, fieldRef):
        self.__fieldRef = fieldRef

    def getFieldRef(self):
        return self.__fieldRef


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


class ResolvedHydratorRefArgumentNode(ArgumentNode):
    '''
    classdocs
    '''
    
    __hydratorRef = None
    
    def __init__(self, *args, **kwargs):
        super(ResolvedHydratorRefArgumentNode, self).__init__(*args, **kwargs)

    def setHydratorRef(self, hydratorRef):
        self.__hydratorRef = hydratorRef

    def getHydratorRef(self):
        return self.__hydratorRef


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
        fieldID = self.getArgument("field").getFieldID()
        valueProviderType = self.getArgument("value").getAttribute("type_alias")
        
        return "FieldSetter< %s, %s, %s >" % (recordType, fieldID, valueProviderType)
        
    def getXMLArguments(self):
        return [ { 'key': 'field', 'type': 'field_ref' }, 
                 { 'key': 'value', 'type': 'value_provider' }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'RuntimeComponentRef(value)' ]


class ReferenceSetterNode(AbstractSetterNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ReferenceSetter")
        super(ReferenceSetterNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/setter/ReferenceSetter.h"
        
    def isInvertible(self):
        return False # FIXME: implement
    
    def getConcreteType(self):
        # template<class RecordType, I16u fid, class ValueProviderType>
        recordType = self.getCxtRecordType()
        fieldID = self.getArgument("reference").getFieldID()
        referenceProviderType = self.getArgument("value").getAttribute("type_alias")
        
        return "ReferenceSetter< %s, %s, %s >" % (recordType, fieldID, referenceProviderType)
        
    def getXMLArguments(self):
        return [ { 'key': 'reference', 'type': 'reference_ref' }, 
                 { 'key': 'value', 'type': 'reference_provider' }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'RuntimeComponentRef(value)' ]


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
        raise RuntimeError("Calling abstract AbstractValueProviderNode::getValueType() method")
    
    def getCxtRecordType(self):
        if isinstance(self.getParent(), ClusteredReferenceProviderNode) and self.getAttribute("key") == "children_count":
            return self.getParent().getRefRecordType()
        else:
            return self.getParent().getCxtRecordType()


class ClusteredValueProviderNode(AbstractValueProviderNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ClusteredValueProvider")
        super(ClusteredValueProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/value/ClusteredValueProvider.h"
    
    def getValueType(self):
        return self.getArgument("probability").getFunctionRef().getDomainType()
    
    def getConcreteType(self):
        # template<typename ValueType, class CxtRecordType, class PrFunctionType, class RangeProviderType>
        valueType = self.getValueType()
        cxtRecordType = self.getCxtRecordType()
        probabilityType = self.getArgument("probability").getFunctionRef().getAttribute("concrete_type")
        rangeProviderType = self.getArgument("cardinality").getAttribute("type_alias")
        
        return "ClusteredValueProvider< %s, %s, %s, %s >" % (valueType, cxtRecordType, probabilityType, rangeProviderType)
        
    def getXMLArguments(self):
        return [ { 'key': 'probability', 'type': 'function_ref' }, 
                 { 'key': 'cardinality', 'type': 'range_provider' }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'FunctionRef(probability)', 
                 'RuntimeComponentRef(cardinality)' 
               ]


class ConstValueProviderNode(AbstractValueProviderNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ConstValueProvider")
        super(ConstValueProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/value/ConstValueProvider.h"
    
    def getValueType(self):
        return self.getArgument("value").getAttribute("type")
    
    def getConcreteType(self):
        # template<typename ValueType, class CxtRecordType>
        valueType = self.getValueType()
        cxtRecordType = self.getCxtRecordType()
        
        return "ConstValueProvider< %s, %s >" % (valueType, cxtRecordType)
        
    def getXMLArguments(self):
        return [ { 'key': 'value', 'type': 'literal' } 
               ]
        
    def getConstructorArguments(self):
        return [ 'Literal(value)' 
               ]


class ContextFieldValueProviderNode(AbstractValueProviderNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ContextFieldValueProvider")
        super(ContextFieldValueProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/value/ContextFieldValueProvider.h"
    
    def getValueType(self):
        return self.getArgument("field").getFieldRef().getAttribute("type")
    
    def getConcreteType(self):
        # template<typename ValueType, class CxtRecordType, I16u fid>
        valueType = self.getValueType()
        cxtRecordType = self.getCxtRecordType()
        fid = self.getArgument("field").getFieldID()
        
        return "ContextFieldValueProvider< %s, %s, %s >" % (valueType, cxtRecordType, fid)
        
    def getXMLArguments(self):
        return [ { 'key': 'field', 'type': 'field_ref' } 
               ]
        
    def getConstructorArguments(self):
        return []


class RandomValueProviderNode(AbstractValueProviderNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="RandomValueProvider")
        super(RandomValueProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/value/RandomValueProvider.h"
    
    def getValueType(self):
        return self.getArgument("probability").getFunctionRef().getDomainType()
    
    def getConcreteType(self):
        # template<typename ValueType, class CxtRecordType, class PrFunctionType, I16u conditionFID>
        valueType = self.getValueType()
        cxtRecordType = self.getCxtRecordType()
        probabilityType = self.getArgument("probability").getFunctionRef().getAttribute("concrete_type")
        if self.hasArgument("condition_field"):
            condFieldID = self.getArgument("condition_field").getFieldID()
        else:
            condFieldID = '0'
        
        return "RandomValueProvider< %s, %s, %s, %s >" % (valueType, cxtRecordType, probabilityType, condFieldID)
    
    def getXMLArguments(self):
        return [ { 'key': 'probability'    , 'type': 'function_ref' }, 
                 { 'key': 'condition_field', 'type': 'field_ref', 'optional': True }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'FunctionRef(probability)'
               ]


#
# Runtime Components: Range Providers 
# 

class AbstractRangeProviderNode(AbstractRuntimeComponentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(AbstractRangeProviderNode, self).__init__(*args, **kwargs)
    
    def getRangeType(self):
        raise RuntimeError("Calling abstract AbstractRuntimeComponentNode::getRangeType() method")
    
    def getCxtRecordType(self):
        return self.getParent().getCxtRecordType()


class ConstRangeProviderNode(AbstractRangeProviderNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ConstRangeProvider")
        super(ConstRangeProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/range/ConstRangeProvider.h"
    
    def getRangeType(self):
        return self.getArgument('min').getAttribute('type')
    
    def getConcreteType(self):
        # template<typename RangeType, class CxtRecordType>
        rangeType = self.getRangeType()
        cxtRecordType = self.getCxtRecordType()
        
        return "ConstRangeProvider< %s, %s >" % (rangeType, cxtRecordType)
        
    def getXMLArguments(self):
        return [ { 'key': 'min', 'type': 'literal' }, 
                 { 'key': 'max', 'type': 'literal' }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'Literal(min)', 
                 'Literal(max)' 
               ]


class ContextFieldRangeProviderNode(AbstractRangeProviderNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ContextFieldRangeProvider")
        super(ContextFieldRangeProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/range/ContextFieldRangeProvider.h"
    
    def getRangeType(self): 
        return self.getArgument('field').getAttribute('type')
    
    def getConcreteType(self):
        # template<typename RangeType, class CxtRecordType, class InvertibleFieldSetterType>
        rangeType = self.getRangeType()
        cxtRecordType = self.getCxtRecordType()
        fieldSetterType = self.getArgument("field").getFieldRef().getSetter().getAttribute("type_alias")
        
        return "ContextFieldRangeProvider< %s, %s, %s >" % (rangeType, cxtRecordType, fieldSetterType)
        
    def getXMLArguments(self):
        return [ { 'key': 'field', 'type': 'field_ref' }, 
               ]
        
    def getConstructorArguments(self):
        return [ 'FieldSetterRef(field)'
               ]


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
        raise RuntimeError("Calling abstract AbstractReferenceProviderNode::getRefRecordType() method")
    
    def getCxtRecordType(self):
        return self.getParent().getCxtRecordType()


class ClusteredReferenceProviderNode(AbstractReferenceProviderNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(template_type="ClusteredReferenceProvider")
        super(ClusteredReferenceProviderNode, self).__init__(*args, **kwargs)
    
    def getIncludePath(self):
        return "runtime/provider/reference/ClusteredReferenceProvider.h"
    
    def getRefRecordType(self):
        return StringTransformer.us2ccAll(self.getParent().getArgument("reference").getAttribute("type"))
    
    def getConcreteType(self):
        # template<typename RefRecordType, class CxtRecordType, class ChildrenCountValueProviderType, I16u posFieldID = 0>
        refRecordType = self.getRefRecordType()
        cxtRecordType = self.getCxtRecordType()
        childrenCountType = self.getArgument("children_count").getAttribute("type_alias")
        if self.hasArgument("position_field"):
            posFieldID = self.getArgument("position_field").getFieldID()
        else:
            posFieldID = '0'
        
        return "ClusteredReferenceProvider< %s, %s, %s, %s >" % (refRecordType, cxtRecordType, childrenCountType, posFieldID)
        
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