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
#        for node in self.__functions.itervalues():
#            node.accept(visitor)
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
    
    def __init__(self, *args, **kwargs):
        if not kwargs.has_key("concrete_type"):
            kwargs.update(concrete_type=kwargs["type"])
        super(FunctionNode, self).__init__(*args, **kwargs)
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
        
    def getConstructorArgumentsOrder(self):
        return []
    
    def getConcreteType(self):
        return self.getAttribute("concrete_type")

    

class ParetoProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="ParetoPrFunction")
        super(ParetoProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getConstructorArgumentsOrder(self):
        return ["x_min", "alpha"]

    
class NormalProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="NormalPrFunction")
        super(NormalProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getConstructorArgumentsOrder(self):
        return ["mean", "stddev"]
    

class CustomDiscreteProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="CustomDiscreteProbability")
        super(CustomDiscreteProbabilityFunctionNode, self).__init__(*args, **kwargs)

    
class UniformProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="UniformPrFunction")
        super(UniformProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getConstructorArgumentsOrder(self):
        return ["x_min", "x_max"]
    

class CombinedProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="CombinedPrFunction")
        kwargs.update(concrete_type="CombinedPrFunction<%s>" % kwargs.get("domainType", "I64u"))
        super(CombinedProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getConstructorArgumentsOrder(self):
        return ["path"]
    

class ConditionalCombinedProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="ConditionalCombinedPrFunction")
        kwargs.update(concrete_type="ConditionalCombinedPrFunction<%s, %s>" % (kwargs.get("domainType1", "I64u"), kwargs.get("domainType2", "I64u")))
        super(ConditionalCombinedProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getConstructorArgumentsOrder(self):
        return ["path"]


class QHistogramProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="QHistogramPrFunction")
        super(QHistogramProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getConstructorArgumentsOrder(self):
        return ["path"]
    

class ConditionalQHistogramProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="ConditionalQHistogramPrFunction")
        super(ConditionalQHistogramProbabilityFunctionNode, self).__init__(*args, **kwargs)
        
    def getConstructorArgumentsOrder(self):
        return ["path"]
    

class CustomDiscreteProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="CustomDiscreteProbability")
        super(CustomDiscreteProbabilityFunctionNode, self).__init__(*args, **kwargs)
    

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
    __hydrators = None
    __hydrationPlan = None
    __sequenceIterator = None
    
    def __init__(self, *args, **kwargs):
        super(RandomSequenceNode, self).__init__(*args, **kwargs)
        self.__cardinalityEstimator = None
        self.__hydrators = None
        self.__hydrationPlan = None
        self.__sequenceIterator = None
    
    def accept(self, visitor):
        visitor.preVisit(self)
        self._recordType.accept(visitor)
        self.__hydrators.accept(visitor)
        self.__hydrationPlan.accept(visitor)
        self.__cardinalityEstimator.accept(visitor)
        if self.__sequenceIterator is not None:
            self.__sequenceIterator.accept(visitor)
        visitor.postVisit(self)
        
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


class HydratorsNode(RecordSequenceNode):
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
        if sorted:
            return self.__hydrators.itervalues()
        else:
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
        
    def hasPRNGArgument(self):
        return False
    
    def hasNewArgsSupport(self):
        return False
        
    def isInvertible(self):
        return False
        
    def getConstructorArgumentsOrder(self):
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
        
    def hasPRNGArgument(self):
        return True
    
    def hasNewArgsSupport(self):
        return True
        
    def getXMLArguments(self):
        return { 'field'              : { 'type': 'field_ref' }, 
                 'position_field'     : { 'type': 'position_field', 'optional': True }, 
                 'count_field'        : { 'type': 'field_ref' }, 
                 'nested_cardinality' : { 'type': 'literal' } 
               }
        
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
        
    def hasPRNGArgument(self):
        return True
    
    def hasNewArgsSupport(self):
        return True
        
    def getXMLArguments(self):
        return { 'field'          : { 'type': 'field_ref' }, 
                 'condition_field': { 'type': 'field_ref' }, 
                 'probability'    : { 'type': 'function_ref' } 
               }
        
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
        
    def hasPRNGArgument(self):
        return False
    
    def hasNewArgsSupport(self):
        return True
        
    def getXMLArguments(self):
        return { 'field'              : { 'type': 'field_ref' }, 
                 'const_value'        : { 'type': 'literal' }
               }
        
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
        
    def hasPRNGArgument(self):
        return True
    
    def hasNewArgsSupport(self):
        return True
        
    def getXMLArguments(self):
        return { 'field'      : { 'type': 'field_ref' }, 
                 'pivot_field': { 'type': 'field_ref' }, 
                 'probability': { 'type': 'function_ref' } 
               }
        
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
        
    def hasPRNGArgument(self):
        return True
    
    def hasNewArgsSupport(self):
        return True
        
    def getXMLArguments(self):
        return { 'field'      : { 'type': 'field_ref' }, 
                 'pivot_field': { 'type': 'field_ref' }, 
                 'pivot_value': { 'type': 'field_ref' } 
               }
        
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
        
    def hasPRNGArgument(self):
        return False
        
    def isInvertible(self):
        return True
        
    def getConstructorArgumentsOrder(self):
        return ['field', 'probability', 'sequence_cardinality']


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
        
    def hasPRNGArgument(self):
        return True
        
    def getConstructorArgumentsOrder(self):
        return ['field', 'probability']


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
        
    def getConstructorArgumentsOrder(self):
        return []


class LinearScaleEstimatorNode(CardinalityEstimatorNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        kwargs.update(type="linear_scale_estimator")
        super(LinearScaleEstimatorNode, self).__init__(*args, **kwargs)
        
    def getConstructorArgumentsOrder(self):
        return ["base_cardinality"]


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


#class NestedSequenceIteratorNode(SequenceIteratorNode):
#    '''
#    classdocs
#    '''
#    
#    def __init__(self, *args, **kwargs):
#        kwargs.update(template_type="PartitionedSequenceIterator")
#        super(NestedSequenceIteratorNode, self).__init__(*args, **kwargs)
#        
#    def getConcreteType(self):
#        recordType = StringTransformer.us2ccAll(self.getArgument("parent_field").getRecordTypeRef().getAttribute("key"))
#        refRecordType = StringTransformer.us2ccAll(self.getArgument("parent_field").getRecordReferenceRef().getRecordTypeRef().getAttribute("key"))
#        
#        return "NestedSequenceIterator< %s, %s >" % (recordType, refRecordType)
#        
#    def getXMLArguments(self):
#        return { 'parent_field' : { 'type': 'field_ref' }, 
#                 'children_count_field' : { 'type': 'field_ref' }
#               }
#        
#    def getConstructorArguments(self):
#        return [ 'EnvVariable(_config)'
#               ]

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


class ResolvedRecordReferenceRefArgumentNode(ResolvedFieldRefArgumentNode):
    '''
    classdocs
    '''
    
    __recordTypeRef = None
    __recordReferenceRef = None
    
    def __init__(self, *args, **kwargs):
        super(ResolvedRecordReferenceRefArgumentNode, self).__init__(*args, **kwargs)
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