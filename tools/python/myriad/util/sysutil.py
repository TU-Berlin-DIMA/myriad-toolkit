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
 
Created on May 3, 2011

@author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
'''

import os.path, sys, logging

def createLogger(name):
    handler = logging.StreamHandler(sys.stderr)
    handler.setFormatter(logging.Formatter("%(message)s", "%y-%m-%d %H:%M:%S"))
    
    logger = logging.getLogger(name)
    logger.setLevel(logging.INFO)
    logger.addHandler(handler)
    
    return logger

def getExistingLogger(name):
    return logging.getLogger(name)

def registerFileHandler(logger, path):
    handler = logging.FileHandler(path)
    handler.setFormatter(logging.Formatter("[%(asctime)s] %(levelname)s %(message)s", "%y-%m-%d %H:%M:%S"))

    logger.addHandler(handler)
    
def checkDir(path):
    if (not os.path.isdir(path)):
        os.makedirs(path)