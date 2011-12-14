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

import os
import logging
import libxml2


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
    __imports = None
    
    
    def __init__(self, *args, **kwargs):
        super(RootNode, self).__init__(*args, **kwargs)
        self.__specification = SpecificationNode()
        self.__imports = ImportsNode()
    
    def accept(self, visitor):
        visitor.preVisit(self)
        self.__specification.accept(visitor)
        self.__imports.accept(visitor)
        visitor.postVisit(self)

    def getSpecification(self):
        return self.__specification

    def getImports(self):
        return self.__imports


class ImportsNode(AbstractNode):
    '''
    classdocs
    '''
    
    __imports = {}
    
    def __init__(self, *args, **kwargs):
        super(ImportsNode, self).__init__(*args, **kwargs)
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in self.__imports.itervalues():
            node.accept(visitor)
        visitor.postVisit(self)
        
    def addImport(self, node):
        self.__imports[node.getAttribute('path')] = node
        

class UnresolvedImportNode(AbstractNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(UnresolvedImportNode, self).__init__(*args, **kwargs)


class SpecificationNode(AbstractNode):
    '''
    classdocs
    '''

    __namespaces = None
    __parameters = None
    __functions = None
    
    def __init__(self, *args, **kwargs):
        super(SpecificationNode, self).__init__(*args, **kwargs)
        self.__namespaces = ValidNamespacesNode()
        self.__parameters = ParametersNode()
        self.__functions = FunctionsNode()
    
    def accept(self, visitor):
        visitor.preVisit(self)
        self.__namespaces.accept(visitor)
        self.__parameters.accept(visitor)
        self.__functions.accept(visitor)
        visitor.postVisit(self)
        
    def getNamespaces(self):
        return self.__namespaces
        
    def getParameters(self):
        return self.__parameters
    
    def getFunctions(self):
        return self.__functions


class ValidNamespacesNode(AbstractNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(ValidNamespacesNode, self).__init__(*args, **kwargs)
        
    def setNamespace(self, namespace, path):
        self.setAttribute(namespace, path)
        
    def getNamespace(self, namespace):
        return self.getAttribute(namespace)
    
    def hasNamespace(self, namespace):
        return self.hasAttribute(namespace)


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


class FunctionsNode(AbstractNode):
    '''
    classdocs
    '''
    
    __functions = {}
    
    def __init__(self, *args, **kwargs):
        super(FunctionsNode, self).__init__(*args, **kwargs)
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in self.__functions.itervalues():
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
    
    _arguments = {}
    
    def __init__(self, *args, **kwargs):
        super(FunctionNode, self).__init__(*args, **kwargs)
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in self._arguments.itervalues():
            node.accept(visitor)
        visitor.postVisit(self)
        
    def setArgument(self, node):
        self._arguments[node.getAttribute('key')] = node
    
    def getArgument(self, key):
        return self._arguments.get(key)
    
class ParetoProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(ParetoProbabilityFunctionNode, self).__init__(*args, **kwargs)
    
class ArgumentNode(AbstractNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(ArgumentNode, self).__init__(*args, **kwargs)
        
class LiteralArgumentNode(ArgumentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(LiteralArgumentNode, self).__init__(*args, **kwargs)
        
class FieldRefArgumentNode(ArgumentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(FieldRefArgumentNode, self).__init__(*args, **kwargs)
        
class FunctionRefArgumentNode(ArgumentNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(FunctionRefArgumentNode, self).__init__(*args, **kwargs)
    

class AbstractVisitor(object):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        pass
    
    def traverse(self, node):
        node.accept(self)
    
    def preVisit(self, node):
        preVisitMethod = None
        
        for cls in node.__class__.__mro__:
            methodName = '_preVisit' + cls.__name__
            preVisitMethod = getattr(self, methodName, None)
            if preVisitMethod:
                break

        if not preVisitMethod:
            preVisitMethod = self._preVisitGeneric
            
        return preVisitMethod(node)
    
    def postVisit(self, node):
        postVisitMethod = None
        
        for cls in node.__class__.__mro__:
            methodName = '_postVisit' + cls.__name__
            postVisitMethod = getattr(self, methodName, None)
            if postVisitMethod:
                break

        if not postVisitMethod:
            postVisitMethod = self._postVisitGeneric
            
        return postVisitMethod(node)
    
    def _preVisitGeneric(self, node):
        pass

    def _postVisitGeneric(self, node):
        pass


class PrintVisitor(AbstractVisitor):
    '''
    classdocs
    '''
    
    __indent = 0
    __indentPrefix = "  "
    
    def __init__(self, *args, **kwargs):
        super(PrintVisitor, self).__init__(*args, **kwargs)
    
    def traverse(self, node):
        print "~" * 160
        node.accept(self)
        print "~" * 160
    
    def _preVisitAbstractNode(self, node):
        if (len(node.allAttributes()) == 0):
            print "%s+ %s" % (self.__indentPrefix * self.__indent, node.__class__.__name__)
        else:
            print "%s+ %s {" % (self.__indentPrefix * self.__indent, node.__class__.__name__)
            for (k, v) in node.allAttributes().iteritems():
                print "%s'%s': '%s'," % (self.__indentPrefix * (self.__indent + 3), k, v)
            print "%s}" % (self.__indentPrefix * (self.__indent + 2))
        self._increaseIndent()

    def _postVisitAbstractNode(self, node):
        self._decreaseIndent()
    
    def _increaseIndent(self):
        self.__indent = self.__indent + 1
        
    def _decreaseIndent(self):
        self.__indent = self.__indent - 1


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
#                function.setArgument(ArgumentNode(key=child.prop("key"), type=child.prop("type")))
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