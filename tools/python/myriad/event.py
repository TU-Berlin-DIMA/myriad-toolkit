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

@author: Alexander Alexandrov <alexander.s.alexandrov@campus.tu-berlin.de>
'''

class UndefinedEventException(Exception):
    
    def __init__(self):
        super(UndefinedEventException, self).__init__()

class Event(object):
    '''
    Sample Event class found on
    
    http://stackoverflow.com/questions/1092531/event-system-in-python
    '''

    def __init__(self):
        self.__handlers = []

    def registerHandler(self, handler):
        self.__handlers.append(handler)
        return self

    def unregisterHandler(self, handler):
        self.__handlers.remove(handler)
        return self

    def fire(self, *args, **keywargs):
        for handler in self.__handlers:
            handler(*args, **keywargs)

    def clearObjectHandlers(self, inObject):
        for theHandler in self.__handlers:
            if theHandler.im_self == inObject:
                self.unregisterHandler(theHandler)