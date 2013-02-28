'''
Copyright 2010-2013 DIMA Research Group, TU Berlin

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
        
    @staticmethod
    def _createXPathContext(xmlNode):
        context = xmlNode.get_doc().xpathNewContext()
        context.xpathRegisterNs("m", AbstractReader.__NAMESPACE)
        context.setContextNode(xmlNode)
        return context


#
# Argument Reader
#

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
    
    @staticmethod
    def createReader(argDescriptor):
        # set default values in the descriptor
        argDescriptor['optional'] = argDescriptor.has_key('default') or argDescriptor.get('optional', False)
        
        # argument reader factory logic
        m = AstUtils.matchArgumentType(argDescriptor['type'])
        if (m):
            if m.group(1) is not None:
                argType = m.group(3)
                encType = m.group(2)
            else:
                argType = m.group(4)
                encType = None
        else:
            raise RuntimeError("Unknown argument reader type `%s`" % (argDescriptor['type']))
        
        argReader = None
        for cls in AstUtils.iterSubClasses(SingleArgumentReader):
            try:
                cls.parseType(argType)
                argReader = cls(descriptor=argDescriptor)
                break
            except ParseTypeError:
                pass
        
        if argReader is None:
            raise RuntimeError("Unknown argument reader type `%s`" % (argType))
        
        if (encType is None):
            return argReader
        elif (encType == "collection"):
            return CollectionArgumentReader(descriptor=argDescriptor,childReader=argReader)
    
    @staticmethod
    def readArguments(argsContainerXMLNode, argsContainerNode):
        for argDescriptor in argsContainerNode.getXMLArguments():
            argReader = ArgumentReader.createReader(argDescriptor)
            argReader.read(argsContainerXMLNode, argsContainerNode)


class SingleArgumentReader(ArgumentReader):
    
    # the type pattern for this class
    __type_pattern = re.compile('^$')
    
    def __init__(self, *args, **kwargs):
        super(SingleArgumentReader, self).__init__(*args, **kwargs)
    
    def read(self, argContainerXMLNode, argsContainerNode):
        # create XML context and grap argument XML node
        childContext = AbstractReader._createXPathContext(argContainerXMLNode)
        argXMLNodes = childContext.xpathEval("./m:argument[@key='%s']" % self._descriptor['key'])
        
        # check if argument exists
        if len(argXMLNodes) < 1:
            # check if argument has default value 
            if self._descriptor.has_key('default') and AstUtils.matchLiteralType(self._descriptor['type']):
                argXMLNode = libxml2.newNode('argument')
                argXMLNode.setProp('key', self._descriptor['key'])
                argXMLNode.setProp('type', self._descriptor['type'])
                argXMLNode.setProp('value', self._descriptor['default'])
                argXMLNodes = [ argXMLNode ]
            # check if argument is optional
            elif not self._descriptor['optional']:
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
    
    @classmethod
    def parseType(cls, type):
        raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-2])


class CollectionArgumentReader(ArgumentReader):
    
    __childReader = None
    
    def __init__(self, *args, **kwargs):
        super(CollectionArgumentReader, self).__init__(*args, **kwargs)
        self.__childReader = kwargs['childReader']
        self._descriptor = self.__childReader._descriptor
    
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
        argsContainerNode.setArgument(ArgumentCollectionNode(key=self._descriptor['key'],collection=[ self.__childReader.parse(argXMLNode, argsContainerNode) for argXMLNode in argXMLNodes ]))


class LiteralArgumentReader(SingleArgumentReader):
    
    # the type pattern for this class
    __type_pattern = re.compile('^(%s)$' % '|'.join(AstUtils.literalTypes()))
    
    def __init__(self, *args, **kwargs):
        super(LiteralArgumentReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        argValue = argXMLNode.prop("value")

        if not argKey:
            raise RuntimeError("Missing attribute `key` for argument `%s`" % (argXMLNode))

        if not argValue:
            raise RuntimeError("Missing attribute `value` for argument `%s`" % (argKey))

        if not AstUtils.matchLiteralType(argType):
            raise RuntimeError("Unexpected argument type `%s` for argument `%s` (expected a literal argument type)" % (argType, argKey))
            
        return LiteralArgumentNode(key=argKey, type=argType, value=argValue)
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if type and m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-2])


class FieldRefArgumentReader(SingleArgumentReader):
    
    # the type pattern for this class
    __type_pattern = re.compile('^field_ref$')
    
    def __init__(self, *args, **kwargs):
        super(FieldRefArgumentReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        argRef = argXMLNode.prop("ref")

        if not argKey:
            raise RuntimeError("Missing attribute `key` for argument `%s`" % (argXMLNode))

        if not argRef:
            raise RuntimeError("Missing required attribute `ref` for `field_ref` argument `%s`" % (argKey))
        
        if argType != 'field_ref':
            raise RuntimeError("Unexpected argument type `%s` for argument `%s` (expected `field_ref`)" % (argType, argKey))
        
        # TODO: check format {record_key}.{field_key}
        
        return UnresolvedFieldRefArgumentNode(key=argKey, ref=argRef)
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if type and m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-2])


class ReferenceRefArgumentReader(SingleArgumentReader):
    
    # the type pattern for this class
    __type_pattern = re.compile('^reference_ref$')
    
    def __init__(self, *args, **kwargs):
        super(ReferenceRefArgumentReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        argRef = argXMLNode.prop("ref")

        if not argKey:
            raise RuntimeError("Missing attribute `key` for argument `%s`" % (argXMLNode))

        if not argRef:
            raise RuntimeError("Missing required attribute `ref` for `reference_ref` argument `%s`" % (argKey))
        
        if argType != 'reference_ref':
            raise RuntimeError("Unexpected argument type `%s` for argument `%s` (expected `reference_ref`)" % (argType, argKey))
        
        # TODO: check format {record_key}.{field_key}
        
        return UnresolvedReferenceRefArgumentNode(key=argKey, ref=argRef)
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if type and m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-2])


class FunctionRefArgumentReader(SingleArgumentReader):
    
    # the type pattern for this class
    __type_pattern = re.compile('^function_ref$')
    
    def __init__(self, *args, **kwargs):
        super(FunctionRefArgumentReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        argRef = argXMLNode.prop("ref")
        
        if not argKey:
            raise RuntimeError("Missing required attribute `key` for argument `%s`" % (argXMLNode))

        if not argRef:
            raise RuntimeError("Missing required attribute `ref` for `function_ref` argument `%s`" % (argKey))
        
        if argType != 'function_ref':
            raise RuntimeError("Unexpected argument type `%s` for argument `%s` (expected `function_ref`)" % (argType, argKey))

        return UnresolvedFunctionRefArgumentNode(key=argKey, ref=argRef)
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if type and m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-2])


class ValueProviderArgumentReader(SingleArgumentReader):
    
    # the type pattern for this class
    __type_pattern = re.compile('^value_provider$')
    
    def __init__(self, *args, **kwargs):
        super(ValueProviderArgumentReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        valueProviderNode = None
        
        for cls in AstUtils.iterSubClasses(AbstractValueProviderNode):
            try:
                args, kwargs = cls.parseType(argType)
                kwargs.update(key=argKey)
                valueProviderNode = cls(*args, **kwargs)
                break
            except ParseTypeError:
                pass
        
        if valueProviderNode is None:    
            raise RuntimeError("Unexpected value provider type `%s` for value provider `%s` (expected one of `(callback|clustered|const|context_field|random)_value_provider[T]`)" % (argType, argKey))
        
        # recursively read value provider arguments
        ArgumentReader.readArguments(argXMLNode, valueProviderNode)
        
        return valueProviderNode
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if type and m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-2])


class RangeProviderArgumentReader(SingleArgumentReader):
    
    # the type pattern for this class
    __type_pattern = re.compile('^range_provider$')
    
    def __init__(self, *args, **kwargs):
        super(RangeProviderArgumentReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        rangeProviderNode = None
        
        for cls in AstUtils.iterSubClasses(AbstractRangeProviderNode):
            try:
                args, kwargs = cls.parseType(argType)
                kwargs.update(key=argKey)
                rangeProviderNode = cls(*args, **kwargs)
                break
            except ParseTypeError:
                pass
        
        if rangeProviderNode is None:    
            raise RuntimeError('Unexpected range provider type `%s` for range provider `%s` (expected one of `(const|context_field)_range_provider[T]`)' % (argType, argKey))
        
        # recursively read range provider arguments
        ArgumentReader.readArguments(argXMLNode, rangeProviderNode)
        
        return rangeProviderNode
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if type and m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-2])


class ReferenceProviderArgumentReader(SingleArgumentReader):
    
    # the type pattern for this class
    __type_pattern = re.compile('^reference_provider$')
    
    def __init__(self, *args, **kwargs):
        super(ReferenceProviderArgumentReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        referenceProviderNode = None
        
        for cls in AstUtils.iterSubClasses(AbstractReferenceProviderNode):
            try:
                args, kwargs = cls.parseType(argType)
                kwargs.update(key=argKey)
                referenceProviderNode = cls(*args, **kwargs)
                break
            except ParseTypeError:
                pass
        
        if referenceProviderNode is None:    
            raise RuntimeError('Unexpected reference provider type `%s` for reference provider `%s` (expected one of `(clustered|random)_reference_provider`)' % (argType, argKey))
        
        # recursively read range provider arguments
        ArgumentReader.readArguments(argXMLNode, referenceProviderNode)
        
        return referenceProviderNode
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if type and m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-2])


class EqualityPredicateProviderReader(SingleArgumentReader):
    
    # the type pattern for this class
    __type_pattern = re.compile('^equality_predicate_provider$')
    
    def __init__(self, *args, **kwargs):
        super(EqualityPredicateProviderReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        predicateProviderNode = None
        
        for cls in AstUtils.iterSubClasses(AbstractPredicateProviderNode):
            try:
                args, kwargs = cls.parseType(argType)
                kwargs.update(key=argKey)
                predicateProviderNode = cls(*args, **kwargs)
                break
            except ParseTypeError:
                pass
        
        if predicateProviderNode is None:    
            raise RuntimeError('Unexpected predicate provider type `%s` for predicate provider `%s` (expected `equality_predicate_provider`)' % (argType, argKey))
        
        # recursively read value provider arguments
        ArgumentReader.readArguments(argXMLNode, predicateProviderNode)
        
        return predicateProviderNode
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if type and m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-2])


class BinderReader(SingleArgumentReader):
    
    # the type pattern for this class
    __type_pattern = re.compile('^binder$')
    
    def __init__(self, *args, **kwargs):
        super(BinderReader, self).__init__(*args, **kwargs)
    
    def parse(self, argXMLNode, argsContainerNode):
        argType = argXMLNode.prop("type")
        argKey = argXMLNode.prop("key")
        binderNode = None
        
        for cls in AstUtils.iterSubClasses(AbstractFieldBinderNode):
            try:
                args, kwargs = cls.parseType(argType)
                kwargs.update(key=argKey)
                binderNode = cls(*args, **kwargs)
                break
            except ParseTypeError:
                pass
        
        if binderNode is None:
            raise RuntimeError('Unexpected binder type `%s` for binder `%s` (expected `predicate_value_binder`)' % (argType, argKey))
        
        # recursively read value provider arguments
        ArgumentReader.readArguments(argXMLNode, binderNode)
        
        return binderNode
    
    @classmethod
    def parseType(cls, type):
        m = cls.__type_pattern.match(type)
        if type and m is not None:
            return [], {}
        else:
            raise ParseTypeError("Bad %s type" % str(cls).split('.').pop()[0:-2])

#
# Argument Reader
#


class PrototypeSpecificationReader(object):
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
        super(PrototypeSpecificationReader, self).__init__()
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
        # initialize recordSequenceNode
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
            
            enumPattern = re.compile('Enum(\[\d+\])?')
            if enumPattern.match(fieldType):
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
        t = cardinalityEstimatorXMLNode.prop("type")
        cardinalityEstimatorNode = None
        
        for cls in AstUtils.iterSubClasses(AbstractCardinalityEstimatorNode):
            try:
                args, kwargs = cls.parseType(t)
                cardinalityEstimatorNode = cls(*args, **kwargs)
                break
            except ParseTypeError:
                pass
        
        if cardinalityEstimatorNode is None:    
            raise RuntimeError('Unexpected cardinality estimator type `%s` (expected one of `(linear_scale_estimator|const_estimator)`)' % (t))
        
        # recursively read range provider arguments
        ArgumentReader.readArguments(cardinalityEstimatorXMLNode, cardinalityEstimatorNode)
        
        return cardinalityEstimatorNode
        
    def __sequenceIteratorFactory(self, sequenceIteratorXMLNode):
        t = sequenceIteratorXMLNode.prop("type")
        sequenceIteratorNode = None
        
        for cls in AstUtils.iterSubClasses(AbstractSequenceIteratorNode):
            try:
                args, kwargs = cls.parseType(t)
                sequenceIteratorNode = cls(*args, **kwargs)
                break
            except ParseTypeError:
                pass
        
        if sequenceIteratorNode is None:    
            raise RuntimeError('Unexpected sequence iterator type `%s` (expected `partitioned_iterator`)' % (t))
        
        # recursively read range provider arguments
        ArgumentReader.readArguments(sequenceIteratorXMLNode, sequenceIteratorNode)
        
        return sequenceIteratorNode
        
    def __outputFormatterFactory(self, outputFormatterXMLNode):
        t = outputFormatterXMLNode.prop("type")
        outputFormatter = None
        
        for cls in AstUtils.iterSubClasses(AbstractOutputFormatterNode):
            try:
                args, kwargs = cls.parseType(t)
                outputFormatter = cls(*args, **kwargs)
                break
            except ParseTypeError:
                pass
        
        if outputFormatter is None:    
            raise RuntimeError('Unexpected output formatter type `%s` (expected one of `(csv|empty)`)' % (t))
        
        # recursively read range provider arguments
        ArgumentReader.readArguments(outputFormatterXMLNode, outputFormatter)
        
        return outputFormatter
        
    def __functionFactory(self, functionXMLNode):
        t = functionXMLNode.prop("type")
        k = functionXMLNode.prop("key")
        functionNode = None
        
        for cls in AstUtils.iterSubClasses(AbstractFunctionNode):
            try:
                args, kwargs = cls.parseType(t)
                kwargs.update(key=k)
                functionNode = cls(*args, **kwargs)
                break
            except ParseTypeError:
                pass
        
        if functionNode is None:    
            raise RuntimeError('Unexpected function type `%s` for function `%s`' % (t, k))
        
        # recursively read range provider arguments
        ArgumentReader.readArguments(functionXMLNode, functionNode)
        
        return functionNode
        
    def __setterFactory(self, setterXMLNode):
        t = setterXMLNode.prop("type")
        k = setterXMLNode.prop("key")
        setterNode = None
        
        for cls in AstUtils.iterSubClasses(AbstractSetterNode):
            try:
                args, kwargs = cls.parseType(t)
                kwargs.update(key=k)
                kwargs.update(type_alias="%sType" % StringTransformer.us2ccAll(k))
                setterNode = cls(*args, **kwargs)
                break
            except ParseTypeError:
                pass
        
        if setterNode is None:    
            raise RuntimeError('Unexpected setter type `%s` for setter `%s` (expected one of `(field|reference)_setter`)' % (t, k))
        
        # recursively read range provider arguments
        ArgumentReader.readArguments(setterXMLNode, setterNode)
        
        return setterNode
