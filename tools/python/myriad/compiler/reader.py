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
 
Created on Dec 15, 2011

@author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
'''

import re
import logging
import libxml2
from myriad.compiler.ast import * #@UnusedWildImport
from myriad.util.stringutil import StringTransformer

class AbstractReader(object):
    
    __NAMESPACE = "http://www.dima.tu-berlin.de/myriad/prototype"
    
    def __init__(self, *args, **kwargs):
        pass
        
    def _createXPathContext(xmlNode):
        context = xmlNode.get_doc().xpathNewContext()
        context.xpathRegisterNs("m", AbstractReader.__NAMESPACE)
        context.setContextNode(xmlNode)
        return context
        
    # static methods
    _createXPathContext = staticmethod(_createXPathContext)


class ArgumentReader(AbstractReader):
    
    _type_pattern = re.compile('^((collection)<([a-zA-Z_]+)>)|([a-zA-Z_]+)$')
    
    _descriptor = {}
    _log = logging.getLogger("argument.reader")

    def __init__(self, *args, **kwargs):
        super(ArgumentReader, self).__init__(*args, **kwargs)
        self._descriptor = kwargs["descriptor"]
    
    def read(self, argContainerXMLNode, argsContainerNode):
        raise RuntimeError("Called abstract method ArgumentReader.read()")
    
    def parse(self, argXMLNode, argsContainerNode):
        raise RuntimeError("Called abstract method ArgumentReader.parse()")
    
    def createReader(argDescriptor):
        # set default values in the descriptor
        argDescriptor['optional'] = argDescriptor.get('optional', False)
        
        argType = argDescriptor['type']
        
        # argument reader factory logic
        m = ArgumentReader._type_pattern.match(argDescriptor['type'])
        if (m):
            if m.group(1) is not None:
                argType = m.group(3)
                encType = m.group(2)
            else:
                argType = m.group(4)
                encType = None
        else:
            message = "Unknown argument reader type `%s`" % (argType)
            ArgumentReader._log.error(message)
            raise RuntimeError(message)
        
        argReader = None
        if (argType == "literal"):
            argReader = LiteralArgumentReader(descriptor=argDescriptor)
        elif (argType == "field_ref"):
            argReader = FieldRefArgumentReader(descriptor=argDescriptor)
        elif (argType == "reference_ref"):
            argReader = ReferenceRefArgumentReader(descriptor=argDescriptor)
        elif (argType == "function_ref"):
            argReader = FunctionRefArgumentReader(descriptor=argDescriptor)
        elif (argType == "value_provider"):
            argReader = ValueProviderArgumentReader(descriptor=argDescriptor)
        elif (argType == "range_provider"):
            argReader = RangeProviderArgumentReader(descriptor=argDescriptor)
        elif (argType == "reference_provider"):
            argReader = ReferenceProviderArgumentReader(descriptor=argDescriptor)
        elif (argType == "equality_predicate_provider"):
            argReader = EqualityPredicateProviderReader(descriptor=argDescriptor)
        elif (argType == "binder"):
            argReader = BinderReader(descriptor=argDescriptor)
        else:
            message = "Unknown argument reader type `%s`" % (argType)
            ArgumentReader._log.error(message)
            raise RuntimeError(message)
        
        if (encType is None):
            return argReader
        elif (encType == "collection"):
            return CollectionArgumentReader(descriptor=argDescriptor,childReader=argReader)
        else:
            message = "Unknown eclosing reader type `%s`" % (encType)
            ArgumentReader._log.error(message)
            raise RuntimeError(message)
        
    def readArguments(argsContainerXMLNode, argsContainerNode):
        for argDescriptor in argsContainerNode.getXMLArguments():
            argReader = ArgumentReader.createReader(argDescriptor)
            argReader.read(argsContainerXMLNode, argsContainerNode)
        
    # static methods
    createReader = staticmethod(createReader)
    readArguments = staticmethod(readArguments)


class SingleArgumentReader(ArgumentReader):
    
    def __init__(self, *args, **kwargs):
        super(SingleArgumentReader, self).__init__(*args, **kwargs)
    
    def read(self, argContainerXMLNode, argsContainerNode):
        # create XML context and grap argument XML node
        childContext = AbstractReader._createXPathContext(argContainerXMLNode)
        argXMLNodes = childContext.xpathEval("./m:argument[@key='%s']" % self._descriptor['key'])
        
        # check if argument exists
        if len(argXMLNodes) < 1:
            # argument does not exist, check if argument is optional
            if not self._descriptor['optional']:
                # argument is not optional 
                message = "Cannot find required argument `%s` in container `%s` of type `%s`" % (self._descriptor['key'], argsContainerNode.getAttribute("key"), argsContainerNode.getAttribute("type"))
                ArgumentReader._log.error(message)
                raise RuntimeError(message)
            else:
                # argument is optional
                return
        elif len(argXMLNodes) > 1:
            # argument is not unique
            message = "Argument `%s` is not unique in container `%s` of type `%s`" % (self._descriptor['key'], argsContainerNode.getAttribute("key"), argsContainerNode.getAttribute("type"))
            ArgumentReader._log.error(message)
            raise RuntimeError(message)
        
        # argument exists, pare it and attach it to the parent container
        argsContainerNode.setArgument(self.parse(argXMLNodes.pop(), argsContainerNode))


class CollectionArgumentReader(ArgumentReader):
    
    _childReader = None
    
    def __init__(self, *args, **kwargs):
        super(CollectionArgumentReader, self).__init__(*args, **kwargs)
        self._childReader = kwargs['childReader']
        self._descriptor = self._childReader._descriptor
    
    def read(self, argContainerXMLNode, argsContainerNode):
        # create XML context and grap argument XML node
        childContext = AbstractReader._createXPathContext(argContainerXMLNode)
        argXMLNodes = childContext.xpathEval("./m:argument[@key='%s']" % self._descriptor['key'])
        
        # check if argument exists
        if len(argXMLNodes) < 1:
            # argument does not exist, check if argument is optional
            if not self._descriptor['optional']:
                # argument is not optional 
                message = "Cannot find required argument `%s` in container `%s` of type `%s`" % (self._descriptor['key'], argsContainerNode.getAttribute("key"), argsContainerNode.getAttribute("type"))
                ArgumentReader._log.error(message)
                raise RuntimeError(message)
            else:
                # argument is optional
                return
        
        # arguments exist, parse them and attach them to the parent container
        argsContainerNode.setArgument(ArgumentCollectionNode(key=self._descriptor['key'],collection=[ self._childReader.parse(argXMLNode, argsContainerNode) for argXMLNode in argXMLNodes ]))


class LiteralArgumentReader(SingleArgumentReader):
    
    def __init__(self, *args, **kwargs):
        super(LiteralArgumentReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        argValue = argXMLNode.prop("value")
        
        return LiteralArgumentNode(key=argKey, type=argType, value=argValue)


class FieldRefArgumentReader(SingleArgumentReader):
    
    def __init__(self, *args, **kwargs):
        super(FieldRefArgumentReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        argRef = argXMLNode.prop("ref")
        
        if argType != 'field_ref':
            raise RuntimeError("Unexpected argument type `%s` for argument `%s` (expected `field_ref`)" % (argType, argKey))
        
        if not argRef:
            raise RuntimeError("Missing required attribute `ref` for `field_ref` argument `%s`" % (argKey))
        
        # TODO: check format {record_key}.{field_key}
        
        return UnresolvedFieldRefArgumentNode(key=argKey, ref=argRef)


class ReferenceRefArgumentReader(SingleArgumentReader):
    
    def __init__(self, *args, **kwargs):
        super(ReferenceRefArgumentReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        argRef = argXMLNode.prop("ref")
        
        if argType != 'reference_ref':
            raise RuntimeError("Unexpected argument type `%s` for argument `%s` (expected `reference_ref`)" % (argType, argKey))
        
        if not argRef:
            raise RuntimeError("Missing required attribute `ref` for `reference_ref` argument `%s`" % (argKey))
        
        # TODO: check format {record_key}.{field_key}
        
        return UnresolvedReferenceRefArgumentNode(key=argKey, ref=argRef)


class FunctionRefArgumentReader(SingleArgumentReader):
    
    def __init__(self, *args, **kwargs):
        super(FunctionRefArgumentReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        argRef = argXMLNode.prop("ref")
        
        if argType != 'function_ref':
            raise RuntimeError("Unexpected argument type `%s` for argument `%s` (expected `function_ref`)" % (argType, argKey))
        
        if not argRef:
            raise RuntimeError("Missing required attribute `ref` for `function_ref` argument `%s`" % (argKey))

        return UnresolvedFunctionRefArgumentNode(key=argKey, ref=argRef)


class ValueProviderArgumentReader(SingleArgumentReader):
    
    def __init__(self, *args, **kwargs):
        super(ValueProviderArgumentReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        
        valueProviderNode = None
        if argType == 'callback_value_provider':
            valueProviderNode = CallbackValueProviderNode(key=argKey)
        elif argType == 'element_wise_value_provider':
            valueProviderNode = ElementWiseValueProviderNode(key=argKey)
        elif argType == 'clustered_value_provider':
            valueProviderNode = ClusteredValueProviderNode(key=argKey)
        elif argType == 'const_value_provider':
            valueProviderNode = ConstValueProviderNode(key=argKey)
        elif argType == 'context_field_value_provider':
            valueProviderNode = ContextFieldValueProviderNode(key=argKey)
        elif argType == 'random_value_provider':
            valueProviderNode = RandomValueProviderNode(key=argKey)
        else:
            raise RuntimeError("Unexpected argument type `%s` for argument `%s` (expected `(callback|clustered|const|context_field|random)_value_provider`)" % (argType, argKey))
        
        # recursively read value provider arguments
        ArgumentReader.readArguments(argXMLNode, valueProviderNode)
        
        return valueProviderNode


class RangeProviderArgumentReader(SingleArgumentReader):
    
    def __init__(self, *args, **kwargs):
        super(RangeProviderArgumentReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        
        rangeProviderNode = None
        if argType == 'const_range_provider':
            rangeProviderNode = ConstRangeProviderNode(key=argKey)
        elif argType == 'context_field_range_provider':
            rangeProviderNode = ContextFieldRangeProviderNode(key=argKey)
        else:
            raise RuntimeError("Unexpected argument type `%s` for argument `%s` (expected `(const|context_field)_range_provider`)" % (argType, argKey))
        
        # recursively read range provider arguments
        ArgumentReader.readArguments(argXMLNode, rangeProviderNode)
        
        return rangeProviderNode


class ReferenceProviderArgumentReader(SingleArgumentReader):
    
    def __init__(self, *args, **kwargs):
        super(ReferenceProviderArgumentReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        
        referenceProviderNode = None
        if argType == 'clustered_reference_provider':
            referenceProviderNode = ClusteredReferenceProviderNode(key=argKey)
        elif argType == 'random_reference_provider':
            referenceProviderNode = RandomReferenceProviderNode(key=argKey)
        else:
            raise RuntimeError("Unexpected argument type `%s` for argument `%s` (expected `(clustered|random)_reference_provider`)" % (argType, argKey))
        
        # recursively read range provider arguments
        ArgumentReader.readArguments(argXMLNode, referenceProviderNode)
        
        return referenceProviderNode


class EqualityPredicateProviderReader(SingleArgumentReader):
    
    def __init__(self, *args, **kwargs):
        super(EqualityPredicateProviderReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        
        predicateProviderNode = None
        if argType == 'equality_predicate_provider':
            predicateProviderNode = EqualityPredicateProviderNode(key=argKey)
        else:
            raise RuntimeError("Unexpected argument type `%s` for argument `%s` (expected `equality_predicate_provider`)" % (argType, argKey))
        
        # recursively read value provider arguments
        ArgumentReader.readArguments(argXMLNode, predicateProviderNode)
        
        return predicateProviderNode


class BinderReader(SingleArgumentReader):
    
    def __init__(self, *args, **kwargs):
        super(BinderReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        
        binderNode = None
        if argType == 'predicate_value_binder':
            binderNode = EqualityPredicateFieldBinderNode(key=argKey)
        else:
            raise RuntimeError("Unexpected argument type `%s` for argument `%s` (expected `predicate_value_binder`)" % (argType, argKey))
        
        # recursively read value provider arguments
        ArgumentReader.readArguments(argXMLNode, binderNode)
        
        return binderNode


class XMLReader(object):
    '''
    Reads the AST from an XML file.
    '''
    
    __args = None
    __log = None
    
    __astRoot = None
    
    def __init__(self, args):
        '''
        Constructor
        '''
        super(XMLReader, self).__init__()
        self.__args = args
        self.__log = logging.getLogger("prototype.reader")
    
    def read(self):
        # assemble the initial AST 
        self.__astRoot = RootNode()
        self.__astRoot.getSpecification().setAttribute("generatorName", self.__args.dgen_name)
        self.__astRoot.getSpecification().setAttribute("generatorNameSpace", self.__args.dgen_ns)
        self.__astRoot.getSpecification().setAttribute("path", self.__args.prototype_path)
        
        # load the XML
        self.__log.info("Reading prototype specification from: `%s`." % (self.__args.prototype_path))
    
        # open the model specification XML
        xmlDoc = libxml2.parseFile(self.__astRoot.getSpecification().getAttribute("path"))
        # construct the remainder of the AST
        self.__readSpecification(self.__astRoot.getSpecification(), xmlDoc)
        
        # resolve argument refs
        self.__resolveRecordReferenceNodes()
        self.__resolveFieldRefArguments()
        self.__resolveReferenceRefArguments()
        self.__resolveFunctionRefArguments()
        
        # set setter references
        self.__setFieldSetterReferences()
        
        # return the final version AST
        return self.__astRoot
            
    def __resolveRecordReferenceNodes(self):
        nodeFilter = DepthFirstNodeFilter(filterType=UnresolvedRecordReferenceNode)
        for unresolvedRecordReferenceNode in nodeFilter.getAll(self.__astRoot):
            parent = unresolvedRecordReferenceNode.getParent()
            fqName = unresolvedRecordReferenceNode.getAttribute("type")
            
            recordSequenceNode = self.__astRoot.getSpecification().getRecordSequences().getRecordSequence(fqName)
            
            if not recordSequenceNode:
                message = "Cannot resolve record reference for record of type `%s`" % (fqName)
                self.__log.error(message)
                raise RuntimeError(message)
            
            recordTypeNode = recordSequenceNode.getRecordType()
            
            resolvedRecordReferenceNode = ResolvedRecordReferenceNode()
            resolvedRecordReferenceNode.setAttribute('name', unresolvedRecordReferenceNode.getAttribute("name"))
            resolvedRecordReferenceNode.setAttribute('type', StringTransformer.ucFirst(StringTransformer.us2cc(unresolvedRecordReferenceNode.getAttribute("type"))))
            resolvedRecordReferenceNode.setRecordTypeRef(recordTypeNode)
            resolvedRecordReferenceNode.orderkey = unresolvedRecordReferenceNode.orderkey
            parent.setReference(resolvedRecordReferenceNode)
    
    def __resolveFieldRefArguments(self):
        nodeFilter = DepthFirstNodeFilter(filterType=UnresolvedFieldRefArgumentNode)
        for unresolvedFieldRefArgumentNode in nodeFilter.getAll(self.__astRoot):
            parent = unresolvedFieldRefArgumentNode.getParent()
            fqName = unresolvedFieldRefArgumentNode.getAttribute("ref")

            if fqName.find(":") < 0:
                message = "Cannot resolve field path `%s`" % (fqName)
                self.__log.error(message)
                raise RuntimeError(message)
            
            rootRecordKey = fqName[:fqName.find(":")]
            innerPath = fqName[fqName.find(":")+1:fqName.rfind(":")]
            if len(innerPath) == 0:
                innerPath = []
            else:
                innerPath = innerPath.split(":")
            fieldKey = fqName[fqName.rfind(":")+1:]
            
            rootRecordSequenceNode = self.__astRoot.getSpecification().getRecordSequences().getRecordSequence(rootRecordKey)
            
            if not rootRecordSequenceNode:
                message = "Cannot resolve field path `%s` (non-existing root type `%s`)" % (fqName, rootRecordKey)
                self.__log.error(message)
                raise RuntimeError(message)
            
            currentParentRecordTypeNode = rootRecordSequenceNode.getRecordType()
            
            for i in range(len(innerPath)):
                referenceKey = innerPath[i]
                referenceNode = currentParentRecordTypeNode.getReference(referenceKey)
                if not referenceNode:
                    message = "Cannot resolve field path `%s` (non-existing reference `%s`)" % (fqName, referenceKey)
                    self.__log.error(message)
                    raise RuntimeError(message)
                innerPath[i] = referenceNode
                currentParentRecordTypeNode = referenceNode.getRecordTypeRef()
            
            fieldNode = currentParentRecordTypeNode.getField(fieldKey)
            
            if not fieldNode:
                message = "Cannot resolve field path `%s` (non-existing record field `%s`)" % (fqName, fieldKey)
                self.__log.error(message)
                raise RuntimeError(message)
            
            resolvedFieldRefArgumentNode = ResolvedFieldRefArgumentNode()
            resolvedFieldRefArgumentNode.setAttribute('key', unresolvedFieldRefArgumentNode.getAttribute("key"))
            resolvedFieldRefArgumentNode.setAttribute('ref', fqName)
            resolvedFieldRefArgumentNode.setAttribute('type', fieldNode.getAttribute("type"))
            resolvedFieldRefArgumentNode.setRecordTypeRef(rootRecordSequenceNode.getRecordType())
            resolvedFieldRefArgumentNode.setInnerPathRefs(innerPath)
            resolvedFieldRefArgumentNode.setFieldRef(fieldNode)
            if isinstance(parent, ArgumentCollectionNode):
                parent.setArgument(unresolvedFieldRefArgumentNode.orderkey, resolvedFieldRefArgumentNode)
            else:
                parent.setArgument(resolvedFieldRefArgumentNode)
    
    def __resolveReferenceRefArguments(self):
        nodeFilter = DepthFirstNodeFilter(filterType=UnresolvedReferenceRefArgumentNode)
        for unresolvedReferenceRefArgumentNode in nodeFilter.getAll(self.__astRoot):
            parent = unresolvedReferenceRefArgumentNode.getParent()
            fqName = unresolvedReferenceRefArgumentNode.getAttribute("ref")

            if fqName.find(":") > -1:
                recordKey = fqName[:fqName.find(":")]
                referencePath = fqName[fqName.find(":")+1:]
            else:
                message = "Cannot resolve referenced record reference for `%s`" % (fqName)
                self.__log.error(message)
                raise RuntimeError(message)
            
            recordSequenceNode = self.__astRoot.getSpecification().getRecordSequences().getRecordSequence(recordKey)
            
            if not recordSequenceNode:
                message = "Cannot resolve referenced record reference for `%s` (non-existing record type `%s`)" % (fqName, recordKey)
                self.__log.error(message)
                raise RuntimeError(message)
            
            recordTypeNode = recordSequenceNode.getRecordType()
            
            referenceKey = referencePath
            referenceTypeNode = recordTypeNode.getReference(referenceKey)
            
            if not referenceTypeNode:
                message = "Cannot resolve referenced record reference for `%s` (non-existing record field `%s`)" % (fqName, referenceKey)
                self.__log.error(message)
                raise RuntimeError(message)
            
            resolvedFieldRefArgumentNode = ResolvedReferenceRefArgumentNode()
            resolvedFieldRefArgumentNode.setAttribute('key', unresolvedReferenceRefArgumentNode.getAttribute("key"))
            resolvedFieldRefArgumentNode.setAttribute('ref', fqName)
            resolvedFieldRefArgumentNode.setAttribute('type', referenceTypeNode.getAttribute("type"))
            resolvedFieldRefArgumentNode.setRecordTypeRef(recordTypeNode)
            resolvedFieldRefArgumentNode.setRecordReferenceRef(recordTypeNode.getReference(referenceKey))
            parent.setArgument(resolvedFieldRefArgumentNode)
    
    def __resolveFunctionRefArguments(self):
        nodeFilter = DepthFirstNodeFilter(filterType=UnresolvedFunctionRefArgumentNode)
        for unresolvedFunctionRefArgumentNode in nodeFilter.getAll(self.__astRoot):
            
            parent = unresolvedFunctionRefArgumentNode.getParent()
            fqName = unresolvedFunctionRefArgumentNode.getAttribute("ref")
            
            functionNode = self.__astRoot.getSpecification().getFunctions().getFunction(fqName)

            if not functionNode:
                message = "Cannot resolve function reference for function `%s`" % (fqName)
                self.__log.error(message)
                raise RuntimeError(message)

            # construct and read the catalog node
            resolvedFunctionRefArgumentNode = ResolvedFunctionRefArgumentNode()
            resolvedFunctionRefArgumentNode.setAttribute('key', unresolvedFunctionRefArgumentNode.getAttribute("key"))
            resolvedFunctionRefArgumentNode.setAttribute('ref', fqName)
            resolvedFunctionRefArgumentNode.setAttribute('type', functionNode.getAttribute("type"))
            resolvedFunctionRefArgumentNode.setAttribute('concrete_type', functionNode.getAttribute("concrete_type"))
            resolvedFunctionRefArgumentNode.setFunctionRef(functionNode)
            parent.setArgument(resolvedFunctionRefArgumentNode)
    
    def __setFieldSetterReferences(self):
        # set reverse references from the record fields to their corresponding setters
        nodeFilter = DepthFirstNodeFilter(filterType=FieldSetterNode)
        for setterNode in nodeFilter.getAll(self.__astRoot):
            field = setterNode.getArgument("field").getFieldRef()
            field.setSetter(setterNode)
            
    def __readSpecification(self, astContext, xmlContext):
        # basic tree areas
        self.__readParameters(astContext, xmlContext)
        self.__readFunctions(astContext, xmlContext)
        self.__readEnumSets(astContext, xmlContext)
        # record related tree areas
        self.__readRecordSequences(astContext, xmlContext)
        
    def __readParameters(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = AbstractReader._createXPathContext(xmlContext)
        
        # read the parameters for this astContext
        for element in xPathContext.xpathEval(".//m:parameters/m:parameter"):
            # add the parameter to the astContext
            astContext.getParameters().setParameter(element.prop("key"), element.content)
        
    def __readFunctions(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = AbstractReader._createXPathContext(xmlContext)

        # attach FunctionNode for each function in the XML document
        for element in xPathContext.xpathEval(".//m:functions/m:function"):
            astContext.getFunctions().setFunction(self.__functionFactory(element))

    def __readEnumSets(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = AbstractReader._createXPathContext(xmlContext)

        # attach EnumSetNode for each function in the XML document
        for element in xPathContext.xpathEval(".//m:enum_sets/m:enum_set"):
            self.__readEnumSet(astContext, element)
        
    def __readEnumSet(self, astContext, xmlContext):
        enumSetNode = EnumSetNode(key=xmlContext.prop("key"))
        ArgumentReader.readArguments(xmlContext, enumSetNode)
        astContext.getEnumSets().setSet(enumSetNode)
            
    def __readRecordSequences(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = AbstractReader._createXPathContext(xmlContext)
        
        # attach RecordSequenceNode for each function in the XML document
        for element in xPathContext.xpathEval(".//m:record_sequences/m:*"):
            recordSequenceType = element.get_name()
        
            if (recordSequenceType == "random_sequence"):
                self.__readRandomSequence(astContext, element)
            else:
                raise RuntimeError('Invalid record sequence type `%s`' % (recordSequenceType))
            
    def __readRandomSequence(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = AbstractReader._createXPathContext(xmlContext)
        
        recordSequenceNode = RandomSequenceNode(key=xmlContext.prop("key"))
        
        # read record type (mandatory)
        recordTypeXMLNode = xPathContext.xpathEval("./m:record_type")
        self.__readRecordType(recordSequenceNode, recordTypeXMLNode.pop())
        # read output format (optional)
        outputFormatterXMLNode = xPathContext.xpathEval("./m:output_format")
        self.__readOutputFormatter(recordSequenceNode, outputFormatterXMLNode.pop() if len(outputFormatterXMLNode) > 0 else None)
        # read setter chain (optional)
        setterChainXMLNode = xPathContext.xpathEval("./m:setter_chain")
        self.__readSetterChain(recordSequenceNode, setterChainXMLNode.pop() if len(setterChainXMLNode) > 0 else None)
        # read cardinality estimator (mandatory)
        cardinalityEstimatorXMLNode = xPathContext.xpathEval("./m:cardinality_estimator")
        self.__readCardinalityEstimator(recordSequenceNode, cardinalityEstimatorXMLNode.pop())
        # read generator tasks (optional)
        sequenceIteratorXMLNode = xPathContext.xpathEval("./m:sequence_iterator")
        self.__readSequenceIterator(recordSequenceNode, sequenceIteratorXMLNode.pop() if len(sequenceIteratorXMLNode) > 0 else None)
        
        astContext.getRecordSequences().setRecordSequence(recordSequenceNode)
        
    def __readRecordType(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = AbstractReader._createXPathContext(xmlContext)
        
        recordTypeNode = RecordTypeNode(key=astContext.getAttribute("key"))
        
        # add implicit fields
        # add `gen_id` field
        i = 0
        recordFieldNode = RecordFieldNode(name="gen_id", type="I64u", implicit=True)
        recordFieldNode.setOrderKey(i)
        recordTypeNode.setField(recordFieldNode)
        
        i = 1
        for element in xPathContext.xpathEval("./m:field"):
            fieldType = element.prop("type")
            if (element.prop("derived") is None):
                fieldIsDerived = False
            else:
                fieldIsDerived = bool(element.prop("derived"))
            
            if fieldType == "Enum":
                recordFieldNode = RecordEnumFieldNode(name=element.prop("name"), type=element.prop("type"), implicit=False, derived=fieldIsDerived, enumref=element.prop("enumref"))
                enumSetNode = self.__astRoot.getSpecification().getEnumSets().getSet(recordFieldNode.getAttribute('enumref'))
                
                if enumSetNode is None:
                    message = "Cannot resolve enum set reference for enum set `%s`" % (recordFieldNode.getAttribute('enumref'))
                    self.__log.error(message)
                    raise RuntimeError(message)
                
                recordFieldNode.setEnumSetRef(enumSetNode)
                recordTypeNode.setEnumField(recordFieldNode)
            else:
                recordFieldNode = RecordFieldNode(name=element.prop("name"), type=element.prop("type"), implicit=False, derived=fieldIsDerived)
                recordTypeNode.setField(recordFieldNode)
            
            recordFieldNode.setOrderKey(i)
            
            i = i+1
        
        i = 0
        for element in xPathContext.xpathEval("./m:reference"):
            recordReferenceNode = UnresolvedRecordReferenceNode(name=element.prop("name"), type=element.prop("type"))
            recordReferenceNode.setOrderKey(i)
            
            recordTypeNode.setReference(recordReferenceNode)
            i = i+1

        astContext.setRecordType(recordTypeNode)
                    
    def __readSetterChain(self, astContext, xmlContext):
        # create and attach the AST node
        setterChainNode = SetterChainNode()
        astContext.setSetterChain(setterChainNode)
        
        # sanity check (XML element is not mandatory)
        if (xmlContext == None):
            return
        
        # derive xPath context from the given xmlContext node
        xPathContext = AbstractReader._createXPathContext(xmlContext)
        
        i = 0
        for setter in xPathContext.xpathEval("./m:setter"):
            setterNode = self.__setterFactory(setter)
            setterNode.setOrderKey(i) # TODO: derive order from dependency graph

            setterChainNode.setSetter(setterNode)
            i = i+1
        
        # enumerate type alias values for all runtime components in this chain
        # ValueProvider nodes 
        i = 1
        nodeFilter = DepthFirstNodeFilter(filterType=AbstractValueProviderNode)
        for node in nodeFilter.getAll(setterChainNode):
            node.setAttribute("type_alias", "ValueProvider%02dType" % (i))
            i = i+1
        # RangeProvider nodes
        i = 1
        nodeFilter = DepthFirstNodeFilter(filterType=AbstractRangeProviderNode)
        for node in nodeFilter.getAll(setterChainNode):
            node.setAttribute("type_alias", "RangeProvider%02dType" % (i))
            i = i+1
        # ReferenceProvider nodes
        i = 1
        nodeFilter = DepthFirstNodeFilter(filterType=AbstractReferenceProviderNode)
        for node in nodeFilter.getAll(setterChainNode):
            node.setAttribute("type_alias", "ReferenceProvider%02dType" % (i))
            i = i+1
        # PredicateProvider nodes
        i = 1
        nodeFilter = DepthFirstNodeFilter(filterType=EqualityPredicateProviderNode)
        for node in nodeFilter.getAll(setterChainNode):
            node.setAttribute("type_alias", "PredicateProvider%02dType" % (i))
            i = i+1
        # PredicateBinder nodes
        i = 1
        nodeFilter = DepthFirstNodeFilter(filterType=EqualityPredicateFieldBinderNode)
        for node in nodeFilter.getAll(setterChainNode):
            node.setAttribute("type_alias", "PredicateBinder%02dType" % (i))
            i = i+1
        
        # set component variable names for all runtime components in this chain
        nodeFilter = DepthFirstNodeFilter(filterType=AbstractRuntimeComponentNode)
        for node in nodeFilter.getAll(setterChainNode):
            typeAlias = node.getAttribute("type_alias")
            node.setAttribute("var_name", "_%s%s" % (typeAlias[0].lower(), typeAlias[1:-4]))
        
    def __readCardinalityEstimator(self, astContext, xmlContext):
        # read element from the corresponding factory
        astContext.setCardinalityEstimator(self.__cardinalityEstimatorFactory(xmlContext))
        
    def __readSequenceIterator(self, astContext, xmlContext):
        # sanity check (XML element is not mandatory)
        if (xmlContext == None):
            return
        # read element from the corresponding factory
        astContext.setSequenceIterator(self.__sequenceIteratorFactory(xmlContext))
        
    def __readOutputFormatter(self, astContext, xmlContext):
        # sanity check (XML element is not mandatory)
        if (xmlContext == None):
            return
        # read element from the corresponding factory
        astContext.setOutputFormatter(self.__outputFormatterFactory(xmlContext))
        
    def __cardinalityEstimatorFactory(self, cardinalityEstimatorXMLNode):
        cardinalityEstimatorType = cardinalityEstimatorXMLNode.prop("type")
        
        # factory logic: create cardinality estimator object
        cardinalityEstimator = None
        if (cardinalityEstimatorType == "linear_scale_estimator"):
            cardinalityEstimator = LinearScaleEstimatorNode()
        else:
            raise RuntimeError('Invalid cardinality estimator type `%s`' % (cardinalityEstimatorType))
        
        # Append arguments
        ArgumentReader.readArguments(cardinalityEstimatorXMLNode, cardinalityEstimator)
        
        return cardinalityEstimator
        
    def __sequenceIteratorFactory(self, sequenceIteratorXMLNode):
        sequenceIteratorType = sequenceIteratorXMLNode.prop("type")
        
        # factory logic: create sequence iterator object
        sequenceIterator = None
        if (sequenceIteratorType == "partitioned_iterator"):
            sequenceIterator = PartitionedSequenceIteratorNode()
        else:
            raise RuntimeError('Invalid generator task type `%s`' % (sequenceIteratorType))
        
        # append arguments
        ArgumentReader.readArguments(sequenceIteratorXMLNode, sequenceIterator)
        
        return sequenceIterator
        
    def __outputFormatterFactory(self, outputFormatterXMLNode):
        outputFormatterType = outputFormatterXMLNode.prop("type")
        
        # factory logic: create output formatter object
        outputFormatter = None
        if (outputFormatterType == "csv"):
            outputFormatter = CsvOutputFormatterNode()
        elif (outputFormatterType == "empty"):
            outputFormatter = EmptyOutputFormatterNode()
        else:
            raise RuntimeError('Invalid output formatter type `%s`' % (outputFormatterType))
        
        # append arguments
        ArgumentReader.readArguments(outputFormatterXMLNode, outputFormatter)
        
        return outputFormatter
        
    def __functionFactory(self, functionXMLNode):
        
        # TODO: second and third component should be mandatory 
        functionMatch = re.match(r"([a-z\_]+)(\[([a-zA-Z0-9\_]+)(;([a-zA-Z0-9\_,]+))?\])?", functionXMLNode.prop("type"))
        # check if type is syntactically correct
        if (functionMatch is None):
            raise RuntimeError('Invalid function type `%s`' % (functionXMLNode.prop("type")))
        # grab the function type
        functionType = functionMatch.group(1)

        # factory logic
        functionNode = None
        if (functionType == "normal_probability"):
            functionNode = NormalProbabilityFunctionNode(key=functionXMLNode.prop("key"))
        elif (functionType == "pareto_probability"):
            functionNode = ParetoProbabilityFunctionNode(key=functionXMLNode.prop("key"))
        elif (functionType == "uniform_probability"):
            functionNode = UniformProbabilityFunctionNode(key=functionXMLNode.prop("key"), domain_type=functionMatch.group(3))
        elif (functionType == "conditional_combined_probability"):
            functionNode = ConditionalCombinedProbabilityFunctionNode(key=functionXMLNode.prop("key"), domain_type1=functionMatch.group(3), domain_type2=functionMatch.group(5))
        elif (functionMatch != "combined_probability"):
            functionNode = CombinedProbabilityFunctionNode(key=functionXMLNode.prop("key"), domain_type=functionMatch.group(3))
        else:
            raise RuntimeError('Invalid function type `%s`' % (functionType))
        
        # append arguments
        ArgumentReader.readArguments(functionXMLNode, functionNode)
        
        return functionNode
        
    def __setterFactory(self, setterXMLNode):
        t = setterXMLNode.prop("type")
        
        # factory logic
        setterNode = None
        if t == "field_setter":
            setterNode = FieldSetterNode(key=setterXMLNode.prop("key"), type=setterXMLNode.prop("type"), type_alias="%sType" % StringTransformer.us2ccAll(setterXMLNode.prop("key")))
        elif t == "reference_setter":
            setterNode = ReferenceSetterNode(key=setterXMLNode.prop("key"), type=setterXMLNode.prop("type"), type_alias="%sType" % StringTransformer.us2ccAll(setterXMLNode.prop("key")))
        else:
            raise RuntimeError('Unsupported setter type `%s`' % (t))
        
        # append arguments
        ArgumentReader.readArguments(setterXMLNode, setterNode)
        
        return setterNode