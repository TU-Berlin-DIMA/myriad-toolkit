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

from myriad.compiler.visitor import AbstractVisitor

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
            # print node with attributes
            print "%s+ %s" % (self.__indentPrefix * self.__indent, node.__class__.__name__)
        else:
            # print node without attributes
            print "%s+ %s {" % (self.__indentPrefix * self.__indent, node.__class__.__name__)
            for (k, v) in node.allAttributes().iteritems():
                print "%s'%s': '%s'," % (self.__indentPrefix * (self.__indent + 3), k, v)
            print "%s}" % (self.__indentPrefix * (self.__indent + 2))
        self._increaseIndent()

    def _postVisitAbstractNode(self, node):
        self._decreaseIndent()
        
#    def _preVisitSetItemNode(self, node):
#        pass
#    
#    def _postVisitSetItemNode(self, node):
#        pass
    
    def _increaseIndent(self):
        self.__indent = self.__indent + 1
        
    def _decreaseIndent(self):
        self.__indent = self.__indent - 1
