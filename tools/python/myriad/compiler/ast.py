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
        print "~" * 80
        node.accept(self)
        print "~" * 80
    
    def _preVisitAbstractNode(self, node):
        print '%s%s %s' % (self.__indentPrefix * self.__indent, node.__class__.__name__, node.allAttributes())
        self._increaseIndent()

    def _preVisitValidNamespacesNode(self, node):
        print '%s%s %s' % (self.__indentPrefix * self.__indent, node.__class__.__name__, node.allAttributes())
        self._increaseIndent()

    def _postVisitAbstractNode(self, node):
        self._decreaseIndent()
    
    def _increaseIndent(self):
        self.__indent = self.__indent + 1
        
    def _decreaseIndent(self):
        self.__indent = self.__indent - 1


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
    
    def accept(self, visitor):
        visitor.preVisit(self)
        self.__specification.accept(visitor)
        self.__imports.accept(visitor)
        visitor.postVisit(self)
        
    def setSpecification(self, node):
        self.__specification = node

    def getSpecification(self):
        return self.__specification
        
    def setImports(self, node):
        self.__imports = node

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
    
    def __init__(self, *args, **kwargs):
        self.__namespaces = ValidNamespacesNode()
        super(SpecificationNode, self).__init__(*args, **kwargs)
    
    def accept(self, visitor):
        visitor.preVisit(self)
        self.__namespaces.accept(visitor)
        visitor.postVisit(self)
        
    def setNamespaces(self, node):
        self.__namespaces = node

    def getNamespaces(self):
        return self.__namespaces


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


class XMLReader(object):
    '''
    Reads the initial AST from an XML file.
    '''
    
    __NAMESPACE = "http://www.dima.tu-berlin.de/myriad/specification"
    
    __args = None
    __log = None
    
    __xmlRoot = None
    __xPathContext = None

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
        self.__astRoot.setSpecification(SpecificationNode(generatorName=self.__args.dgen_name, generatorNameSpace=self.__args.dgen_ns, location=self.__args.model_spec_path))
        self.__astRoot.setImports(ImportsNode())
        
        # load the XML
        self.__xmlRoot = libxml2.parseFile(self.__args.model_spec_path)

        # construct the initial xPath context and register the Myriad namespace 
        self.__xPathContext = self.__xmlRoot.xpathNewContext()
        self.__xPathContext.xpathRegisterNs("m", self.__NAMESPACE)
        
        # construct the remainder of the AST
        self.__readSpecification()
        
        # return the final version AST
        return self.__astRoot

    def __readSpecification(self):
        
        # get the directory containing the parsed specification XML document
        baseDir = os.path.dirname(self.__astRoot.getSpecification().getAttribute("location"))
        
        # attach UnresolvedImportNode for each import in the XML document
        for importElement in self.__xPathContext.xpathEval("//m:generator_specification/m:imports/m:import"):
            # get the namespace for the current import
            namespace = importElement.prop("namespace")
            # get the path for the current import
            path = importElement.prop("path")
            # use XML location dir to prefix relative paths
            if (not os.path.isabs(path)):
                path = os.path.realpath("%s/%s" % (baseDir, path))

            # register the namespace with this
            self.__astRoot.getSpecification().getNamespaces().setNamespace(namespace, path)
                            
            # attach UnresolvedImportNode
            self.__astRoot.getImports().addImport(UnresolvedImportNode(path=path))
        
    def __readImports(self):
        pass
        
    def __readTypes(self):
        pass
