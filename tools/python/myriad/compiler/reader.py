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

import os
import sys
import logging
import libxml2
from myriad.compiler.ast import * #@UnusedWildImport


class XMLReader(object):
    '''
    Reads the initial AST from an XML file.
    '''
    
    __NAMESPACE = "http://www.dima.tu-berlin.de/myriad/prototype"
    
    __args = None
    __log = None
    
    __astRoot = None
    
    def __init__(self, args):
        '''
        Constructor
        '''
        self.__args = args
        self.__log = logging.getLogger("ast.reader")
    
    def read(self):
        # assemble the initial AST 
        self.__astRoot = RootNode()
        self.__astRoot.getSpecification().setAttribute("generatorName", self.__args.dgen_name)
        self.__astRoot.getSpecification().setAttribute("generatorNameSpace", self.__args.dgen_ns)
        self.__astRoot.getSpecification().setAttribute("path", self.__args.prototype_path)
        
        # load the XML
        try:
            self.__log.info("Reading XML specification from: `%s`." % (self.__args.prototype_path))
        
            # open the model specification XML
            xmlDoc = libxml2.parseFile(self.__astRoot.getSpecification().getAttribute("path"))
            # construct the remainder of the AST
            self.__readSpecification(self.__astRoot.getSpecification(), xmlDoc)
            
            # resolve argument refs
            self.__resolveFieldRefArguments()
            self.__resolveFunctionRefArguments()
            self.__resolveHydratorRefArguments()
            
        except:
            e = sys.exc_info()[1]
            print "Unexpected error: %s" % (e)
            raise
        
        # return the final version AST
        return self.__astRoot
    
    def __resolveFieldRefArguments(self):
        nodeFilter = DepthFirstNodeFilter(filterType=UnresolvedFieldRefArgumentNode)
        for unresolvedFieldRefArgumentNode in nodeFilter.getAll(self.__astRoot):
            parent = unresolvedFieldRefArgumentNode.getParent()
            fqName = unresolvedFieldRefArgumentNode.getAttribute("ref")

            if fqName.find(":") > -1:
                recordKey = fqName[:fqName.find(":")]
                fieldKey = fqName[fqName.find(":")+1:]
            else:
                message = "Cannot resolve field reference for field `%s`" % (fqName)
                self.__log.error(message)
                raise RuntimeError(message)
            
            recordTypeNode = self.__astRoot.getSpecification().getRecordSequences().getRecordSequence(recordKey).getRecordType()
            fieldTypeNode = recordTypeNode.getField(fieldKey)
            
            resolvedFieldRefArgumentNode = ResolvedFieldRefArgumentNode()
            resolvedFieldRefArgumentNode.setAttribute('key', unresolvedFieldRefArgumentNode.getAttribute("key"))
            resolvedFieldRefArgumentNode.setAttribute('ref', fqName)
            resolvedFieldRefArgumentNode.setAttribute('type', fieldTypeNode.getAttribute("type"))
            resolvedFieldRefArgumentNode.setRecordTypeRef(recordTypeNode)
            resolvedFieldRefArgumentNode.setFieldRef(fieldTypeNode)
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
        
    def __createXPathContext(self, xmlNode):
        context = xmlNode.get_doc().xpathNewContext()
        context.xpathRegisterNs("m", self.__NAMESPACE)
        context.setContextNode(xmlNode)
        return context

    def __readSpecification(self, astContext, xmlContext):
        # basic tree areas
        self.__readParameters(astContext, xmlContext)
        self.__readFunctions(astContext, xmlContext)
        self.__readEnumSets(astContext, xmlContext)
        self.__readStringSets(astContext, xmlContext)
        # record related tree areas
        self.__readRecordSequences(astContext, xmlContext)
        
    def __readParameters(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = self.__createXPathContext(xmlContext)
        
        # read the parameters for this astContext
        for element in xPathContext.xpathEval(".//m:parameters/m:parameter"):
            # add the parameter to the astContext
            astContext.getParameters().setParameter(element.prop("key"), element.prop("key"))
        
    def __readFunctions(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = self.__createXPathContext(xmlContext)

        # attach FunctionNode for each function in the XML document
        for element in xPathContext.xpathEval(".//m:functions/m:function"):
            functionNode = self.__functionFactory(element)
            
            childContext = self.__createXPathContext(element)
            for child in childContext.xpathEval("./m:argument"):
                functionNode.setArgument(self.__argumentFactory(child))

            astContext.getFunctions().setFunction(functionNode)

    def __readEnumSets(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = self.__createXPathContext(xmlContext)

        # attach FunctionNode for each function in the XML document
        for element in xPathContext.xpathEval(".//m:enum_sets/m:enum_set"):
            enumSet = EnumSetNode(key=element.prop("key"))
            
            childContext = self.__createXPathContext(element)
            for child in childContext.xpathEval("./m:item"):
                enumSet.addItem(SetItemNode(value=child.prop("value")))
            
            astContext.getEnumSets().setSet(enumSet)

    def __readStringSets(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = self.__createXPathContext(xmlContext)

        # attach FunctionNode for each function in the XML document
        for element in xPathContext.xpathEval(".//m:string_sets/m:string_set"):
            enumSet = StringSetNode(key=element.prop("key"))
            
            childContext = self.__createXPathContext(element)
            for child in childContext.xpathEval("./m:item"):
                enumSet.addItem(SetItemNode(value=child.prop("value")))
            
            astContext.getStringSets().setSet(enumSet)
            
    def __readRecordSequences(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = self.__createXPathContext(xmlContext)
        
        # attach FunctionNode for each function in the XML document
        for element in xPathContext.xpathEval(".//m:record_sequences/m:*"):
            recordSequenceType = element.get_name()
        
            if (recordSequenceType == "random_sequence"):
                self.__readRandomSequence(astContext, element)
            else:
                raise RuntimeError('Invalid record sequence type `%s`' % (recordSequenceType))
            
    def __readRandomSequence(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = self.__createXPathContext(xmlContext)
        
        recordSequenceNode = RandomSequenceNode(key=xmlContext.prop("key"))
        
        # read record type (mandatory)
        self.__readRecordType(recordSequenceNode, xPathContext.xpathEval("./m:record_type").pop())
        # read cardinality estimator (mandatory)
        self.__readCardinalityEstimator(recordSequenceNode, xPathContext.xpathEval("./m:cardinality_estimator").pop())
        # read hydrators (optional)
        hydratorsXMLNode = xPathContext.xpathEval("./m:hydrators")
        self.__readHydrators(recordSequenceNode, hydratorsXMLNode.pop() if len(hydratorsXMLNode) > 0 else None)
        # read hydration plan (optional)
        hydrationPlanXMLNode = xPathContext.xpathEval("./m:hydration_plan")
        self.__readHydrationPlan(recordSequenceNode, hydrationPlanXMLNode.pop() if len(hydrationPlanXMLNode) > 0 else None)
        # read generator tasks (optional)
        generatorTasksXMLNode = xPathContext.xpathEval("./m:generator_tasks")
        self.__readGeneratorTasks(recordSequenceNode, generatorTasksXMLNode.pop() if len(generatorTasksXMLNode) > 0 else None)
        
        astContext.getRecordSequences().setRecordSequence(recordSequenceNode)
        
    def __readRecordType(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = self.__createXPathContext(xmlContext)
        
        recordTypeNode = RecordTypeNode(key=astContext.getAttribute("key"))
        
        for element in xPathContext.xpathEval("./m:field"):
            recordTypeNode.setField(RecordFieldNode(name=element.prop("name"), type=element.prop("type")))

        astContext.setRecordType(recordTypeNode)
        
    def __readCardinalityEstimator(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = self.__createXPathContext(xmlContext)
        
        cardinalityEstimatorNode = CardinalityEstimatorNode(type=xmlContext.prop("type"))
        
        for child in xPathContext.xpathEval("./m:argument"):
            cardinalityEstimatorNode.setArgument(self.__argumentFactory(child))

        astContext.setCardinalityEstimator(cardinalityEstimatorNode)
        
    def __readHydrators(self, astContext, xmlContext):
        # create and attach the AST node
        hydratorsNode = HydratorsNode()
        astContext.setHydrators(hydratorsNode)
        
        # sanity check (XML element is not mandatory)
        if (xmlContext == None):
            return
        
        # derive xPath context from the given xmlContext node
        xPathContext = self.__createXPathContext(xmlContext)
        
        i = 0
        for hydrator in xPathContext.xpathEval("./m:hydrator"):
            hydratorNode = self.__hydratorFactory(hydrator, i)
            hydratorNode.setOrderKey(i)
            
            childContext = self.__createXPathContext(hydrator)
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
        xPathContext = self.__createXPathContext(xmlContext)
        
        for hydratorRef in xPathContext.xpathEval("./m:hydrator_ref"):
            if not astContext.getHydrators().hasHydrator(hydratorRef.prop("ref")):
                message = "Cannot resolve hydrator reference for hydrator `%s`" % (hydratorRef.prop("ref"))
                self.__log.error(message)
                raise RuntimeError(message)
                
            hydrationPlanNode.addHydrator(astContext.getHydrators().getHydrator(hydratorRef.prop("ref")))
        
    def __readGeneratorTasks(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = self.__createXPathContext(xmlContext)
        
        generatorTasksNode = GeneratorTasksNode()
        
        for element in xPathContext.xpathEval("./m:generator_task"):
            generatorTasksNode.setTask(GeneratorTaskNode(key=element.prop("key"), type=element.prop("type")))
        
        astContext.setGeneratorTasks(generatorTasksNode)
        
    def __functionFactory(self, functionXMLNode):
        functionType = functionXMLNode.prop("type")
        
        functionNode = None
        
        if (functionType == "pareto_probability"):
            return ParetoProbabilityFunctionNode(key=functionXMLNode.prop("key"))
        if (functionType == "normal_probability"):
            return NormalProbabilityFunctionNode(key=functionXMLNode.prop("key"))
        if (functionType == "custom_discrete_probability"):
            return CustomDiscreteProbabilityFunctionNode(key=functionXMLNode.prop("key"))

        raise RuntimeError('Invalid function type `%s`' % (functionType))
    
    def __hydratorFactory(self, hydratorXMLNode, i):
        t = hydratorXMLNode.prop("type")
        
        if t == "clustered_sequence_hydrator":
            return ClusteredEnumSetHydratorNode(key=hydratorXMLNode.prop("key"), type=hydratorXMLNode.prop("type"), type_alias="H%02d" % (i))
        if t == "conditional_hydrator":
            return ConditionalHydratorNode(key=hydratorXMLNode.prop("key"), type=hydratorXMLNode.prop("type"), type_alias="H%02d" % (i))
        if t == "const_hydrator":
            return ConstValueHydratorNode(key=hydratorXMLNode.prop("key"), type=hydratorXMLNode.prop("type"), type_alias="H%02d" % (i))
        if t == "enum_set_hydrator":
            return EnumSetHydratorNode(key=hydratorXMLNode.prop("key"), type=hydratorXMLNode.prop("type"), type_alias="H%02d" % (i))
        if t == "randomized_sequence_hydrator":
            return MultiplicativeGroupHydratorNode(key=hydratorXMLNode.prop("key"), type=hydratorXMLNode.prop("type"), type_alias="H%02d" % (i))
        if t == "range_set_hydrator":
            return RangeSetHydratorNode(key=hydratorXMLNode.prop("key"), type=hydratorXMLNode.prop("type"), type_alias="H%02d" % (i))
        
        raise RuntimeError('Unsupported hydrator type `%s`' % (t))

    def __argumentFactory(self, argumentXMLNode, enclosingRecordType = None):
        argumentType = argumentXMLNode.prop("type")
        argumentKey = argumentXMLNode.prop("key")
        argumentRef = argumentXMLNode.prop("ref")
        
        if enclosingRecordType:
            enclosingRecordType = enclosingRecordType.strip(": ") + ":"
        else:
            enclosingRecordType = ""
        
        if (argumentType == "field_ref"):
            argumentRef = argumentXMLNode.prop("ref")
            if not argumentRef:
                message = "Missing required attribute ref for field argument `%s`" % (argumentKey)
                self.__log.error(message)
                raise RuntimeError(message)
            return UnresolvedFieldRefArgumentNode(key=argumentKey, ref="%s%s" % (enclosingRecordType, argumentRef))

        if (argumentType == "function_ref"):
            argumentRef = argumentXMLNode.prop("ref")
            if not argumentRef:
                message = "Missing required attribute ref for field argument `%s`" % (argumentKey)
                self.__log.error(message)
                raise RuntimeError(message)
            return UnresolvedFunctionRefArgumentNode(key=argumentKey, ref=argumentRef)
        
        if (argumentType == "hydrator_ref"):
            if not argumentRef:
                message = "Missing required attribute ref for field argument `%s`" % (argumentKey)
                self.__log.error(message)
                raise RuntimeError(message)
            return UnresolvedHydratorRefArgumentNode(key=argumentKey, ref="%s%s" % (enclosingRecordType, argumentRef))
        
        if (argumentType == "string_set_ref"):
            if not argumentRef:
                message = "Missing required attribute ref for field argument `%s`" % (argumentKey)
                self.__log.error(message)
                raise RuntimeError(message)
            return StringSetRefArgumentNode(key=argumentKey, ref=argumentRef)
        
        else:
            return LiteralArgumentNode(key=argumentKey, type=argumentType, value=argumentXMLNode.prop("value"))