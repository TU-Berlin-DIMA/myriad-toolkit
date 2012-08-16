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
import sys
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
    
    _descriptor = {}
    _log = logging.getLogger("argument.reader")

    def __init__(self, *args, **kwargs):
        super(ArgumentReader, self).__init__(*args, **kwargs)
        self._descriptor = kwargs["descriptor"]
    
    def read(self, argContainerXMLNode, argsContainerNode):
        raise RuntimeError("Called abstract method ArgumentReader.read()")
    
    def parse(self, argXMLNode, argsContainerNode):
        raise RuntimeError("Called abstract method ArgumentReader.parse()")
    
    def createReader(argKey, argDescriptor):
        argType = argDescriptor.get('type', None)
        argOptional = argDescriptor.get('optional', False)
        argDescriptorUpdated = {'key': argKey, 'type': argType, 'optional': argOptional}
        
        if (argType == "literal"):
            return LiteralArgumentReader(descriptor=argDescriptorUpdated)
        elif (argType == "field_ref"):
            return FieldRefArgumentReader(descriptor=argDescriptorUpdated)
        elif (argType == "function_ref"):
            return FunctionRefArgumentReader(descriptor=argDescriptorUpdated)
        elif (argType == "value_provider"):
            return ValueProviderArgumentReader(descriptor=argDescriptorUpdated)
        else:
            message = "Unknown argument reader type `%s`" % (argType)
            ArgumentReader._log.error(message)
            raise RuntimeError(message)
        
    def readArguments(argsContainerXMLNode, argsContainerNode):
        for argKey, argDescriptor in argsContainerNode.getXMLArguments().iteritems():
            argReader = ArgumentReader.createReader(argKey, argDescriptor)
            argsContainerNode.setArgument(argReader.read(argsContainerXMLNode, argsContainerNode))
        
    # static methods
    createReader = staticmethod(createReader)
    readArguments = staticmethod(readArguments)


class SingleArgumentReader(ArgumentReader):
    
    def __init__(self, *args, **kwargs):
        super(SingleArgumentReader, self).__init__(*args, **kwargs)
    
    def read(self, argContainerXMLNode, argsContainerNode):
        # create XML context and grap argument XML node
        childContext = AbstractReader._createXPathContext(argContainerXMLNode)
        argXMLNode = childContext.xpathEval("./m:argument[@key='%s']" % self._descriptor['key'])
        
        # check if argument exists
        if len(argXMLNode) < 1:
            # argument does not exist, check if argument is optional
            if not self._descriptor['optional']:
                # argument is not optional 
                message = "Cannot find required argument `%s` in container `%s` of type `%s`" % (self._descriptor['key'], argsContainerNode.getAttribute("key"), argsContainerNode.getAttribute("type"))
                ArgumentReader._log.error(message)
                raise RuntimeError(message)
            else:
                # argument is optional
                return
        elif len(argXMLNode) > 1:
            # argument is not unique
            message = "Argument `%s` is not unique in container `%s` of type `%s`" % (self._descriptor['key'], argsContainerNode.getAttribute("key"), argsContainerNode.getAttribute("type"))
            ArgumentReader._log.error(message)
            raise RuntimeError(message)

        # argument exists, grap the 
        argXMLNode = argXMLNode.pop()
        
        return self.parse(argXMLNode, argsContainerNode)


class ListArgumentReader(ArgumentReader):

    def __init__(self, *args, **kwargs):
        super(ListArgumentReader, self).__init__(*args, **kwargs)
    
    def read(self, argContainerXMLNode, argsContainerNode):
        raise RuntimeError("Called abstract method ListArgumentReader.read()")


class LiteralArgumentReader(SingleArgumentReader):
    
    def __init__(self, *args, **kwargs):
        super(LiteralArgumentReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        argValue = argXMLNode.prop("value")
        
        return LiteralArgumentNode(key=argKey, type=argType, value=argValue)
        
#        argumentRef = argXMLNode.prop("ref")
#        if enclosingRecordType:
#            enclosingRecordType = enclosingRecordType.strip(": ") + ":"
#        else:
#            enclosingRecordType = ""
#        
#        if (argumentType == "field_ref"):
#            argumentRef = argumentXMLNode.prop("ref")
#            if not argumentRef:
#                message = "Missing required attribute `ref` for field argument `%s`" % (argumentKey)
#                self.__log.error(message)
#                raise RuntimeError(message)
#            
#            if argumentRef.find(":") == -1:
#                argumentRef = "%s%s" % (enclosingRecordType, argumentRef)
#                
#            return UnresolvedFieldRefArgumentNode(key=argumentKey, ref=argumentRef)
#
#        if (argumentType == "function_ref"):
#            argumentRef = argumentXMLNode.prop("ref")
#            if not argumentRef:
#                message = "Missing required attribute `ref` for field argument `%s`" % (argumentKey)
#                self.__log.error(message)
#                raise RuntimeError(message)
#            return UnresolvedFunctionRefArgumentNode(key=argumentKey, ref=argumentRef)
#        
#        # FIXME: this is (probably) obsolete, remove this code
#        if (argumentType == "hydrator_ref"):
#            if not argumentRef:
#                message = "Missing required attribute `ref` for field argument `%s`" % (argumentKey)
#                self.__log.error(message)
#                raise RuntimeError(message)
#            return UnresolvedHydratorRefArgumentNode(key=argumentKey, ref="%s%s" % (enclosingRecordType, argumentRef))
#        
#        # FIXME: this is (probably) obsolete, remove this code
#        if (argumentType == "string_set_ref"):
#            if not argumentRef:
#                message = "Missing required attribute `ref` for field argument `%s`" % (argumentKey)
#                self.__log.error(message)
#                raise RuntimeError(message)
#            return StringSetRefArgumentNode(key=argumentKey, ref=argumentRef)
#        
#        else:
#            if not argumentValue:
#                message = "Missing required attribute `value` for field argument `%s`" % (argumentKey)
#                self.__log.error(message)
#                raise RuntimeError(message)
#            return LiteralArgumentNode(key=argumentKey, type=argumentType, value=argumentValue)

class FieldRefArgumentReader(ArgumentReader):
    
    def __init__(self, *args, **kwargs):
        super(FieldRefArgumentReader, self).__init__(*args, **kwargs)


class FunctionRefArgumentReader(ArgumentReader):
    
    def __init__(self, *args, **kwargs):
        super(FunctionRefArgumentReader, self).__init__(*args, **kwargs)


class ValueProviderArgumentReader(ArgumentReader):
    
    def __init__(self, *args, **kwargs):
        super(ValueProviderArgumentReader, self).__init__(*args, **kwargs)


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
        self.__log = logging.getLogger("ast.reader")
    
    def read(self):
        # assemble the initial AST 
        self.__astRoot = RootNode()
        self.__astRoot.getSpecification().setAttribute("generatorName", self.__args.dgen_name)
        self.__astRoot.getSpecification().setAttribute("generatorNameSpace", self.__args.dgen_ns)
        self.__astRoot.getSpecification().setAttribute("path", self.__args.prototype_path)
        
        # load the XML
        self.__log.info("Reading XML specification from: `%s`." % (self.__args.prototype_path))
    
        # open the model specification XML
        xmlDoc = libxml2.parseFile(self.__astRoot.getSpecification().getAttribute("path"))
        # construct the remainder of the AST
        self.__readSpecification(self.__astRoot.getSpecification(), xmlDoc)
        
        # resolve argument refs
        self.__resolveRecordReferenceNodes()
        self.__resolveFieldRefArguments()
        self.__resolveFunctionRefArguments()
        self.__resolveHydratorRefArguments()
        
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

            if fqName.find(":") > -1:
                recordKey = fqName[:fqName.find(":")]
                fieldPath = fqName[fqName.find(":")+1:]
            else:
                message = "Cannot resolve field reference for field `%s`" % (fqName)
                self.__log.error(message)
                raise RuntimeError(message)
            
            recordSequenceNode = self.__astRoot.getSpecification().getRecordSequences().getRecordSequence(recordKey)
            
            if not recordSequenceNode:
                message = "Cannot resolve field reference for field `%s` (unexisting record type `%s`)" % (fqName, recordKey)
                self.__log.error(message)
                raise RuntimeError(message)
            
            recordTypeNode = recordSequenceNode.getRecordType()
            
            if fieldPath.find(".") > -1:
                fieldKey = fieldPath[fieldPath.find(".")+1:]
                
                referenceKey = fieldPath[:fieldPath.find(".")]
                recordReferenceNode = recordTypeNode.getReference(referenceKey)
            
                if not recordReferenceNode:
                    message = "Cannot resolve named record reference `%s` for record type `%s`" % (referenceKey, recordKey)
                    self.__log.error(message)
                    raise RuntimeError(message)
                
                fieldTypeNode = recordReferenceNode.getRecordTypeRef().getField(fieldKey)
                
                if not fieldTypeNode:
                    message = "Cannot resolve field reference for field `%s` (unexisting record field `%s`)" % (fqName, fieldKey)
                    self.__log.error(message)
                    raise RuntimeError(message)
                
                resolvedFieldRefArgumentNode = ResolvedReferencedFieldRefArgumentNode()
                resolvedFieldRefArgumentNode.setAttribute('key', unresolvedFieldRefArgumentNode.getAttribute("key"))
                resolvedFieldRefArgumentNode.setAttribute('ref', fqName)
                resolvedFieldRefArgumentNode.setAttribute('type', fieldTypeNode.getAttribute("type"))
                resolvedFieldRefArgumentNode.setRecordTypeRef(recordTypeNode)
                resolvedFieldRefArgumentNode.setRecordReferenceRef(recordReferenceNode)
                resolvedFieldRefArgumentNode.setFieldRef(fieldTypeNode)
                parent.setArgument(resolvedFieldRefArgumentNode)
                
            else:
                fieldKey = fieldPath
                fieldTypeNode = recordTypeNode.getField(fieldKey)
                
                if not fieldTypeNode:
                    fieldTypeNode = recordTypeNode.getReference(fieldKey)
                
                if not fieldTypeNode:
                    message = "Cannot resolve field reference for field `%s` (unexisting record field `%s`)" % (fqName, fieldKey)
                    self.__log.error(message)
                    raise RuntimeError(message)
                
                if isinstance(fieldTypeNode, RecordFieldNode):
                    resolvedFieldRefArgumentNode = ResolvedDirectFieldRefArgumentNode()
                    resolvedFieldRefArgumentNode.setAttribute('key', unresolvedFieldRefArgumentNode.getAttribute("key"))
                    resolvedFieldRefArgumentNode.setAttribute('ref', fqName)
                    resolvedFieldRefArgumentNode.setAttribute('type', fieldTypeNode.getAttribute("type"))
                    resolvedFieldRefArgumentNode.setRecordTypeRef(recordTypeNode)
                    resolvedFieldRefArgumentNode.setFieldRef(fieldTypeNode)
                    parent.setArgument(resolvedFieldRefArgumentNode)
                elif isinstance(fieldTypeNode, RecordReferenceNode):
                    resolvedFieldRefArgumentNode = ResolvedRecordReferenceRefArgumentNode()
                    resolvedFieldRefArgumentNode.setAttribute('key', unresolvedFieldRefArgumentNode.getAttribute("key"))
                    resolvedFieldRefArgumentNode.setAttribute('ref', fqName)
                    resolvedFieldRefArgumentNode.setAttribute('type', fieldTypeNode.getAttribute("type"))
                    resolvedFieldRefArgumentNode.setRecordTypeRef(recordTypeNode)
                    resolvedFieldRefArgumentNode.setRecordReferenceRef(recordTypeNode.getReference(fieldKey))
                    parent.setArgument(resolvedFieldRefArgumentNode)
                else:
                    message = "Unexpected field type"
                    self.__log.error(message)
                    raise RuntimeError(message)
    
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
    
    def __resolveHydratorRefArguments(self):
        nodeFilter = DepthFirstNodeFilter(filterType=UnresolvedHydratorRefArgumentNode)
        for unresolvedHydratorRefArgumentNode in nodeFilter.getAll(self.__astRoot):
            parent = unresolvedHydratorRefArgumentNode.getParent()
            fqName = unresolvedHydratorRefArgumentNode.getAttribute("ref")

            if fqName.find(":") > -1:
                sequenceKey = fqName[:fqName.find(":")]
                hydratorKey = fqName[fqName.find(":")+1:]
            else:
                message = "Cannot resolve hydrator reference for hydrator `%s`" % (fqName)
                self.__log.error(message)
                raise RuntimeError(message)

            hydratorContainer = self.__astRoot.getSpecification().getRecordSequences().getRecordSequence(sequenceKey).getHydrators()
            if not hydratorContainer.hasHydrator(hydratorKey):
                message = "Cannot resolve hydrator reference for hydrator `%s`" % (fqName)
                self.__log.error(message)
                raise RuntimeError(message)

            hydratorNode = hydratorContainer.getHydrator(hydratorKey)
            
            resolvedHydratorRefArgumentNode = ResolvedHydratorRefArgumentNode()
            resolvedHydratorRefArgumentNode.setAttribute('key', unresolvedHydratorRefArgumentNode.getAttribute("key"))
            resolvedHydratorRefArgumentNode.setAttribute('ref', fqName)
            resolvedHydratorRefArgumentNode.setAttribute('type', hydratorNode.getAttribute("type_alias"))
            resolvedHydratorRefArgumentNode.setHydratorRef(hydratorNode)
            parent.setArgument(resolvedHydratorRefArgumentNode)
        
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
        # read hydrators (optional)
        hydratorsXMLNode = xPathContext.xpathEval("./m:hydrators")
        self.__readHydrators(recordSequenceNode, hydratorsXMLNode.pop() if len(hydratorsXMLNode) > 0 else None)
        # read hydration plan (optional)
        hydrationPlanXMLNode = xPathContext.xpathEval("./m:hydration_plan")
        self.__readHydrationPlan(recordSequenceNode, hydrationPlanXMLNode.pop() if len(hydrationPlanXMLNode) > 0 else None)
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
        
        i = 0
        for element in xPathContext.xpathEval("./m:field"):
            fieldType = element.prop("type")
            
            if fieldType == "Enum":
                recordFieldNode = RecordEnumFieldNode(name=element.prop("name"), type=element.prop("type"), enumref=element.prop("enumref"))
                enumSetNode = self.__astRoot.getSpecification().getEnumSets().getSet(recordFieldNode.getAttribute('enumref'))
                
                if enumSetNode is None:
                    message = "Cannot resolve enum set reference for enum set `%s`" % (recordFieldNode.getAttribute('enumref'))
                    self.__log.error(message)
                    raise RuntimeError(message)
                
                recordFieldNode.setEnumSetRef(enumSetNode)
                recordTypeNode.setEnumField(recordFieldNode)
            else:
                recordFieldNode = RecordFieldNode(name=element.prop("name"), type=element.prop("type"))
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
        
    def __readCardinalityEstimator(self, astContext, xmlContext):
        cardinalityEstimatorNode = self.__cardinalityEstimatorFactory(xmlContext)
        ArgumentReader.readArguments(xmlContext, cardinalityEstimatorNode)
        astContext.setCardinalityEstimator(cardinalityEstimatorNode)
        
    def __readHydrators(self, astContext, xmlContext):
        # create and attach the AST node
        hydratorsNode = HydratorsNode()
        astContext.setHydrators(hydratorsNode)
        
        # sanity check (XML element is not mandatory)
        if (xmlContext == None):
            return
        
        # derive xPath context from the given xmlContext node
        xPathContext = AbstractReader._createXPathContext(xmlContext)
        
        i = 0
        for hydrator in xPathContext.xpathEval("./m:hydrator"):
            hydratorNode = self.__hydratorFactory(hydrator, i)
            hydratorNode.setOrderKey(i)
            
            childContext = AbstractReader._createXPathContext(hydrator)
            for argument in childContext.xpathEval("./m:argument"):
                hydratorNode.setArgument(self.__argumentFactory(argument, astContext.getRecordType().getAttribute("key")))

            hydratorsNode.setHydrator(hydratorNode)
            i = i+1
            
    def __readHydrationPlan(self, astContext, xmlContext):
        # create and attach the AST node
        hydrationPlanNode = HydrationPlanNode()
        astContext.setHydrationPlan(hydrationPlanNode)
        
        # sanity check (XML element is not mandatory)
        if (xmlContext == None):
            return
        
        # derive xPath context from the given xmlContext node
        xPathContext = AbstractReader._createXPathContext(xmlContext)
        
        for hydratorRef in xPathContext.xpathEval("./m:hydrator_ref"):
            if not astContext.getHydrators().hasHydrator(hydratorRef.prop("ref")):
                message = "Cannot resolve hydrator reference for hydrator `%s`" % (hydratorRef.prop("ref"))
                self.__log.error(message)
                raise RuntimeError(message)
                
            hydrationPlanNode.addHydrator(astContext.getHydrators().getHydrator(hydratorRef.prop("ref")))
                    
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
        
    def __readSequenceIterator(self, astContext, xmlContext):
        # sanity check (XML element is not mandatory)
        if (xmlContext == None):
            return
        
        # derive xPath context from the given xmlContext node
        xPathContext = AbstractReader._createXPathContext(xmlContext)
        
        sequenceIteratorNode = self.__sequenceIteratorFactory(xmlContext)
        
        for child in xPathContext.xpathEval("./m:argument"):
            sequenceIteratorNode.setArgument(self.__argumentFactory(child))

        astContext.setSequenceIterator(sequenceIteratorNode)
        
    def __cardinalityEstimatorFactory(self, cardinalityEstimatorXMLNode):
        cardinalityEstimatorType = cardinalityEstimatorXMLNode.prop("type")
        
        if (cardinalityEstimatorType == "linear_scale_estimator"):
            return LinearScaleEstimatorNode()

        raise RuntimeError('Invalid cardinality estimator type `%s`' % (cardinalityEstimatorType))
        
    def __sequenceIteratorFactory(self, sequenceIteratorXMLNode):
        sequenceIteratorType = sequenceIteratorXMLNode.prop("type")
        
        if (sequenceIteratorType == "partitioned_iterator"):
            return PartitionedSequenceIteratorNode()

        raise RuntimeError('Invalid generator task type `%s`' % (sequenceIteratorType))
        
    def __functionFactory(self, functionXMLNode):
        
        # @todo: second and third component should be mandatory 
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
            functionNode = UniformProbabilityFunctionNode(key=functionXMLNode.prop("key"))
        elif (functionType == "conditional_combined_probability"):
            functionNode = ConditionalCombinedProbabilityFunctionNode(key=functionXMLNode.prop("key"), domainType1=functionMatch.group(3), domainType2=functionMatch.group(5))
        elif (functionMatch != "combined_probability"):
            functionNode = CombinedProbabilityFunctionNode(key=functionXMLNode.prop("key"), domainType=functionMatch.group(3))
        else:
            raise RuntimeError('Invalid function type `%s`' % (functionType))
        
        # append arguments
        ArgumentReader.readArguments(functionXMLNode, functionNode)
        
        return functionNode
    
    def __hydratorFactory(self, hydratorXMLNode, i):
        t = hydratorXMLNode.prop("type")
        
        if t == "clustered_reference_hydrator":
            return ClusteredReferenceHydratorNode(key=hydratorXMLNode.prop("key"), type=hydratorXMLNode.prop("type"), type_alias="H%02d" % (i))
        if t == "conditional_randomized_hydrator":
            return ConditionalRandomizedHydratorNode(key=hydratorXMLNode.prop("key"), type=hydratorXMLNode.prop("type"), type_alias="H%02d" % (i))
        if t == "const_hydrator":
            return ConstValueHydratorNode(key=hydratorXMLNode.prop("key"), type=hydratorXMLNode.prop("type"), type_alias="H%02d" % (i))
        if t == "referenced_record_hydrator":
            return ReferencedRecordHydratorNode(key=hydratorXMLNode.prop("key"), type=hydratorXMLNode.prop("type"), type_alias="H%02d" % (i))
        if t == "reference_hydrator":
            return ReferenceHydratorNode(key=hydratorXMLNode.prop("key"), type=hydratorXMLNode.prop("type"), type_alias="H%02d" % (i))
        if t == "simple_clustered_hydrator":
            return SimpleClusteredHydratorNode(key=hydratorXMLNode.prop("key"), type=hydratorXMLNode.prop("type"), type_alias="H%02d" % (i))
        if t == "simple_randomized_hydrator":
            return SimpleRandomizedHydratorNode(key=hydratorXMLNode.prop("key"), type=hydratorXMLNode.prop("type"), type_alias="H%02d" % (i))

        raise RuntimeError('Unsupported hydrator type `%s`' % (t))

    def __setterFactory(self, setterXMLNode):
        t = setterXMLNode.prop("type")
        
        if t == "field_setter":
            return FieldSetterNode(key=setterXMLNode.prop("key"), type=setterXMLNode.prop("type"), type_alias=StringTransformer.us2ccAll("%sType" % setterXMLNode.prop("key")))
        if t == "reference_setter":
            return ReferenceSetterNode(key=setterXMLNode.prop("key"), type=setterXMLNode.prop("type"), type_alias=StringTransformer.us2ccAll("%sType" % setterXMLNode.prop("key")))

        raise RuntimeError('Unsupported setter type `%s`' % (t))

    def __argumentFactory(self, argumentXMLNode, enclosingRecordType = None):
        argumentType = argumentXMLNode.prop("type")
        argumentKey = argumentXMLNode.prop("key")
        argumentRef = argumentXMLNode.prop("ref")
        argumentValue = argumentXMLNode.prop("value")
        
        if enclosingRecordType:
            enclosingRecordType = enclosingRecordType.strip(": ") + ":"
        else:
            enclosingRecordType = ""
        
        if (argumentType == "field_ref"):
            argumentRef = argumentXMLNode.prop("ref")
            if not argumentRef:
                message = "Missing required attribute `ref` for field argument `%s`" % (argumentKey)
                self.__log.error(message)
                raise RuntimeError(message)
            
            if argumentRef.find(":") == -1:
                argumentRef = "%s%s" % (enclosingRecordType, argumentRef)
                
            return UnresolvedFieldRefArgumentNode(key=argumentKey, ref=argumentRef)

        if (argumentType == "function_ref"):
            argumentRef = argumentXMLNode.prop("ref")
            if not argumentRef:
                message = "Missing required attribute `ref` for field argument `%s`" % (argumentKey)
                self.__log.error(message)
                raise RuntimeError(message)
            return UnresolvedFunctionRefArgumentNode(key=argumentKey, ref=argumentRef)
        
        # FIXME: this is (probably) obsolete, remove this code
        if (argumentType == "hydrator_ref"):
            if not argumentRef:
                message = "Missing required attribute `ref` for field argument `%s`" % (argumentKey)
                self.__log.error(message)
                raise RuntimeError(message)
            return UnresolvedHydratorRefArgumentNode(key=argumentKey, ref="%s%s" % (enclosingRecordType, argumentRef))
        
        # FIXME: this is (probably) obsolete, remove this code
        if (argumentType == "string_set_ref"):
            if not argumentRef:
                message = "Missing required attribute `ref` for field argument `%s`" % (argumentKey)
                self.__log.error(message)
                raise RuntimeError(message)
            return StringSetRefArgumentNode(key=argumentKey, ref=argumentRef)
        
        else:
            if not argumentValue:
                message = "Missing required attribute `value` for field argument `%s`" % (argumentKey)
                self.__log.error(message)
                raise RuntimeError(message)
            return LiteralArgumentNode(key=argumentKey, type=argumentType, value=argumentValue)