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

import logging

class ASTReader():
    '''
    classdocs
    '''
    
    __path = None
    __log = None

    def __init__(self, *args, **kwargs):
        '''
        Constructor
        '''
        
        self.__path = kwargs['path']
        self.__log = logging.getLogger("ast.reader")
    
    def read(self):
        self.__log.info("Reading AST from location: %s" % (self.__path))
