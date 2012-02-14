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
    
    __NAMESPACE = "http://www.dima.tu-berlin.de/myriad/specification"
    
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
        self.__astRoot.getSpecification().setAttribute("path", self.__args.model_spec_path)
        
        # load the XML
        try:
            self.__log.info("Reading XML specification from: `%s`." % (self.__args.model_spec_path))
        
            # open the model specification XML
            xmlDoc = libxml2.parseFile(self.__astRoot.getSpecification().getAttribute("path"))
            # construct the remainder of the AST
            self.__readSpecification(self.__astRoot.getSpecification(), xmlDoc)
            
            # resolve imports
            importsNode = self.__astRoot.getImports()
            unresolvedImports = importsNode.getUnresolvedImports()
            while unresolvedImports:
                for unresolvedImportNode in unresolvedImports:
                    path = unresolvedImportNode.getAttribute('path')
                    self.__log.info("Resolving XML import `%s`." % (path))
                    
                    # open the catalog XML
                    xmlDoc = libxml2.parseFile(path)
                    # construct and read the catalog node
                    resolvedImportNode = ResolvedImportNode(path=path)
                    self.__readCatalogImport(resolvedImportNode, xmlDoc)
                    
                    importsNode.addImport(resolvedImportNode)
                unresolvedImports = importsNode.getUnresolvedImports()
            
        except:
            e = sys.exc_info()[1]
            print "Unexpected error: %s" % (e)
            raise
        
        # return the final version AST
        return self.__astRoot
    
    def __createXPathContext(self, xmlNode):
        context = xmlNode.get_doc().xpathNewContext()
        context.xpathRegisterNs("m", self.__NAMESPACE)
        context.setContextNode(xmlNode)
        return context

    def __readSpecification(self, astContext, xmlContext):
        # basic tree areas
        self.__readImports(astContext, xmlContext)
        self.__readParameters(astContext, xmlContext)
        self.__readFunctions(astContext, xmlContext)
        self.__readEnumSets(astContext, xmlContext)
        self.__readStringSets(astContext, xmlContext)
        # record related tree areas
        self.__readRecordSequences(astContext, xmlContext)
        
    def __readCatalogImport(self, astContext, xmlContext):
        # basic tree areas
        self.__readImports(astContext, xmlContext)
        self.__readParameters(astContext, xmlContext)
        self.__readFunctions(astContext, xmlContext)
        self.__readEnumSets(astContext, xmlContext)
        self.__readStringSets(astContext, xmlContext)
        
    def __readImports(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = self.__createXPathContext(xmlContext)
        # get the directory containing the parsed specification XML document
        baseDir = os.path.dirname(astContext.getAttribute("path"))
        
        # attach UnresolvedImportNode for each import in the XML document
        for element in xPathContext.xpathEval(".//m:imports/m:import"):
            # get the namespace for the current import
            namespace = element.prop("namespace")
            # get the path for the current import
            path = element.prop("path")
            # use XML location dir to prefix relative paths
            if (not os.path.isabs(path)):
                path = os.path.realpath("%s/%s" % (baseDir, path))

            # register the namespace with this astContext
            astContext.getNamespaces().setNamespace(namespace, path)
                            
            # attach UnresolvedImportNode
            self.__astRoot.getImports().addImport(UnresolvedImportNode(path=path))
        
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
        
        self.__readRecordType(recordSequenceNode, xPathContext.xpathEval("./m:record_type").pop())
        self.__readCardinalityEstimator(recordSequenceNode, xPathContext.xpathEval("./m:cardinality_estimator").pop())
        self.__readHydrators(recordSequenceNode, xPathContext.xpathEval("./m:hydrators").pop())
        self.__readHydrationPlan(recordSequenceNode, xPathContext.xpathEval("./m:hydration_plan").pop())
        self.__readGeneratorTasks(recordSequenceNode, xPathContext.xpathEval("./m:generator_tasks").pop())
        
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
        # derive xPath context from the given xmlContext node
        xPathContext = self.__createXPathContext(xmlContext)
        
        hydratorsNode = HydratorsNode()
        
        for hydrator in xPathContext.xpathEval("./m:hydrator"):
            hydratorNode = HydratorNode(key=hydrator.prop("key"), type=hydrator.prop("type"))
            
            childContext = self.__createXPathContext(hydrator)
            for argument in childContext.xpathEval("./m:argument"):
                hydratorNode.setArgument(self.__argumentFactory(argument))

            hydratorsNode.setHydrator(hydratorNode)
                
        astContext.setHydrators(hydratorsNode)
        
    def __readHydrationPlan(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = self.__createXPathContext(xmlContext)
        
        hydrationPlanNode = HydrationPlanNode()
        
        for hydratorRef in xPathContext.xpathEval("./m:hydrator_ref"):
            hydrationPlanNode.addHydrator(astContext.getHydrators().getHydrator(hydratorRef.prop("ref")))
        
        astContext.setHydrationPlan(hydrationPlanNode)
        
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
            functionNode = ParetoProbabilityFunctionNode(key=functionXMLNode.prop("key"))
        elif (functionType == "normal_probability"):
            functionNode = NormalProbabilityFunctionNode(key=functionXMLNode.prop("key"))
        elif (functionType == "custom_discrete_probability"):
            functionNode = CustomDiscreteProbabilityFunctionNode(key=functionXMLNode.prop("key"))
        else:
            raise RuntimeError('Invalid function type `%s`' % (functionType))
        
        return functionNode

    def __argumentFactory(self, argumentXMLNode):
        argumentType = argumentXMLNode.prop("type")
        
        argumentNode = None
        
        if (argumentType == "field_ref"):
            argumentNode = FieldRefArgumentNode(key=argumentXMLNode.prop("key"), ref=argumentXMLNode.prop("ref"))
        elif (argumentType == "function_ref"):
            argumentNode = FunctionRefArgumentNode(key=argumentXMLNode.prop("key"), ref=argumentXMLNode.prop("ref"))
        else:
            argumentNode = LiteralArgumentNode(key=argumentXMLNode.prop("key"), value=argumentXMLNode.prop("value"))
        
        return argumentNode