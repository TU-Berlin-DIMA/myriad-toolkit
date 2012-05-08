'''
This file is licensed unter the PSF License. For more information about the 
license, please refer to

    http://docs.python.org/license.html

@author: alane@sourceforge.net.

The original algorithm for this component was provided by Xavier Defrang.
http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/81330. 
'''

import re
import UserDict

# Original algorithm by Xavier Defrang.
# http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/81330
# This implementation by alane@sourceforge.net.

class TextSub(UserDict.UserDict):
    def __init__(self, dict = None):
        self.re = None
        self.regex = None
        UserDict.UserDict.__init__(self, dict)
        
    def compile(self):
        if len(self.data) > 0:
            tmp = "(%s)" % "|".join(map(re.escape, map(self.formatKey, self.data.keys())))
            if self.re != tmp:
                self.re = tmp
                self.regex = re.compile(self.re)

    def __call__(self, match):
        return self.data[match.string[match.start()+1:match.end()-1]]

    def sub(self, s):
        if len(self.data) == 0:
            return s
        return self.regex.sub(self, s)

    def formatKey(self, key):
        return "{%s}" %(key)
    
def processTemplate(inPath, outPath, params):
    writeString(outPath, params.sub(readString(inPath)))

def writeString(path, s):
    f = open(path, 'w')
    f.write(s)
    
def readString(path):
    f = open(path, 'r')
    return "".join(f.readlines())
    
def readFirstLine(path):
    f = open(path, 'r')
    return f.readline()

class StringTransformer(object):
    '''
    classdocs
    '''
    
    _cc2us_pattern1 = re.compile('(.)([A-Za-z]+)')
    _cc2us_pattern2 = re.compile('([a-z0-9])([A-Z])')
    _complex_type_pattern = re.compile('(vector|Interval)\[(\w+)\]')
    
    def uc(s):
        return s.upper()
    
    def lc(s):
        return s.lower()
    
    def cc2us(s):
        return StringTransformer._cc2us_pattern2.sub(r'\1_\2', self._cc2us_pattern1.sub(r'\1_\2', s)).lower()
    
    def us2cc(s):
        def camelcase(): 
            yield str.lower
            while True:
                yield str.capitalize
    
        c = camelcase()
        return "".join(c.next()(x) if x else '_' for x in s.split("_"))
    
    def us2ccAll(s):
        return StringTransformer.ucFirst(StringTransformer.us2cc(s))
    
    def ucFirst(s):
        return "%s%s" % (s[0].capitalize(), s[1:])
    
    def sourceType(s):
        r = StringTransformer._complex_type_pattern.match(s)
        if r:
            return "%s<%s>" % (r.group(1), r.group(2))
        else:
            return s
        
    def isVectorType(s):
        r = StringTransformer._complex_type_pattern.match(s)
        if r:
            return r.group(1) == 'vector'
        else:
            return False
        
        r = StringTransformer._complex_type_pattern.match(s)
        
    def coreType(s):
        r = StringTransformer._complex_type_pattern.match(s)
        if r:
            return r.group(2)
        else:
            return s
    
    # static methods
    uc = staticmethod(uc)
    lc = staticmethod(lc)
    cc2us = staticmethod(cc2us)
    us2cc = staticmethod(us2cc)
    us2ccAll = staticmethod(us2ccAll)
    ucFirst = staticmethod(ucFirst)
    sourceType = staticmethod(sourceType)
    isVectorType = staticmethod(isVectorType)
    coreType = staticmethod(coreType)