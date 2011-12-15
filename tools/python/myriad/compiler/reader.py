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
        self.__log.info("Reading XML specification from: `%s`." % (self.__args.model_spec_path))
        
        # assemble the initial AST 
        self.__astRoot = RootNode()
        self.__astRoot.getSpecification().setAttribute("generatorName", self.__args.dgen_name)
        self.__astRoot.getSpecification().setAttribute("generatorNameSpace", self.__args.dgen_ns)
        self.__astRoot.getSpecification().setAttribute("xmlLocation", self.__args.model_spec_path)
        
        # load the XML
        xmlDoc = libxml2.parseFile(self.__astRoot.getSpecification().getAttribute("xmlLocation"))
        
        # construct the remainder of the AST
        self.__readSpecification(self.__astRoot.getSpecification(), xmlDoc)
        
        # return the final version AST
        return self.__astRoot
    
    def __createXPathContext(self, xmlNode):
        context = xmlNode.get_doc().xpathNewContext()
        context.xpathRegisterNs("m", self.__NAMESPACE)
        context.setContextNode(xmlNode)
        return context

    def __readSpecification(self, astContext, xmlContext):
        self.__readImports(astContext, xmlContext)
        self.__readParameters(astContext, xmlContext)
        self.__readFunctions(astContext, xmlContext)
        
    def __readImports(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = self.__createXPathContext(xmlContext)
        # get the directory containing the parsed specification XML document
        baseDir = os.path.dirname(astContext.getAttribute("xmlLocation"))
        
        # attach UnresolvedImportNode for each import in the XML document
        for element in xPathContext.xpathEval("//m:generator_specification/m:imports/m:import"):
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
        for element in xPathContext.xpathEval("//m:generator_specification/m:parameters/m:parameter"):
            # add the parameter to the astContext
            astContext.getParameters().setParameter(element.prop("key"), element.prop("key"))
        
    def __readFunctions(self, astContext, xmlContext):
        # derive xPath context from the given xmlContext node
        xPathContext = self.__createXPathContext(xmlContext)

        # attach FunctionNode for each function in the XML document
        for element in xPathContext.xpathEval("//m:generator_specification/m:functions/m:function"):
            function = self.__functionFactory(element)
            
            childContext = self.__createXPathContext(element)
            for child in childContext.xpathEval("m:argument"):
                function.setArgument(self.__argumentFactory(child))

            astContext.getFunctions().setFunction(function)

    def __functionFactory(self, functionXMLNode):
        functionType = functionXMLNode.prop("type")
        
        functionNode = None
        
        if (functionType == "pareto_probability"):
            functionNode = ParetoProbabilityFunctionNode()
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
        elif (argumentType == "value"):
            argumentNode = LiteralArgumentNode(key=argumentXMLNode.prop("key"), value=argumentXMLNode.content)
        else:
            argumentNode = LiteralArgumentNode(key=argumentXMLNode.prop("key"), value=argumentXMLNode.content)
        
        return argumentNode
