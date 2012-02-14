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
