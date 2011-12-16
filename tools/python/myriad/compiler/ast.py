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
        self.__imports = {}
    
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
    __enumSets = None
    __stringSets = None
    
    def __init__(self, *args, **kwargs):
        super(SpecificationNode, self).__init__(*args, **kwargs)
        self.__namespaces = ValidNamespacesNode()
        self.__parameters = ParametersNode()
        self.__functions = FunctionsNode()
        self.__enumSets = EnumSetsNode()
        self.__stringSets = StringSetsNode()
    
    def accept(self, visitor):
        visitor.preVisit(self)
        self.__namespaces.accept(visitor)
        self.__parameters.accept(visitor)
        self.__functions.accept(visitor)
        self.__enumSets.accept(visitor)
        self.__stringSets.accept(visitor)
        visitor.postVisit(self)
        
    def getNamespaces(self):
        return self.__namespaces
        
    def getParameters(self):
        return self.__parameters
    
    def getFunctions(self):
        return self.__functions
    
    def getEnumSets(self):
        return self.__enumSets
    
    def getStringSets(self):
        return self.__stringSets


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
        self._arguments = {}
    
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

    
class NormalProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(NormalProbabilityFunctionNode, self).__init__(*args, **kwargs)
    

class CustomDiscreteProbabilityFunctionNode(FunctionNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(CustomDiscreteProbabilityFunctionNode, self).__init__(*args, **kwargs)
    

#
# Enum and String Sets
# 

class SetsNode(AbstractNode):
    '''
    classdocs
    '''
    
    __sets = {}
    
    def __init__(self, *args, **kwargs):
        super(SetsNode, self).__init__(*args, **kwargs)
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


class EnumSetsNode(SetsNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(EnumSetsNode, self).__init__(*args, **kwargs)


class StringSetsNode(SetsNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(StringSetsNode, self).__init__(*args, **kwargs)
    
    
class SetNode(AbstractNode):
    '''
    classdocs
    '''
    
    _items = []
    
    def __init__(self, *args, **kwargs):
        super(SetNode, self).__init__(*args, **kwargs)
        self._items = []
    
    def accept(self, visitor):
        visitor.preVisit(self)
        for node in self._items:
            node.accept(visitor)
        visitor.postVisit(self)
        
    def addItem(self, node):
        self._items.append(node)


class EnumSetNode(SetNode):
    '''
    classdocs
    '''

    def __init__(self, *args, **kwargs):
        super(EnumSetNode, self).__init__(*args, **kwargs)


class StringSetNode(SetNode):
    '''
    classdocs
    '''

    def __init__(self, *args, **kwargs):
        super(StringSetNode, self).__init__(*args, **kwargs)


class SetItemNode(AbstractNode):
    '''
    classdocs
    '''
    
    def __init__(self, *args, **kwargs):
        super(SetItemNode, self).__init__(*args, **kwargs)


#
# Arguments
# 
    
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