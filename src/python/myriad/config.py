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
 
Created on Mar 2, 2011

@author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
'''

import time, datetime
import libxml2
import util.properties

NAMESPACE = "http://www.dima.tu-berlin.de/myriad/frontend"

class Exception:
    pass

class UnknownConfigObjectException(Exception):
    objectType = None
    objectName = None
    
    def __init__(self, objectType, objectName):
        self.objectType = objectType
        self.objectName = objectName
        
    def __str__(self):
        return "unknown configuration object of type <%s> with id='%s'" % (self.objectType, self.objectName)


class Environment(object):
    '''
    classdocs
    '''
    
    dgenPath = None
    master = None
    slaves = None
    params = None
    
    def __init__(self, dgenPath, master, slaves, params):
        '''
        Constructor
        '''
        self.id = id
        self.dgenPath = dgenPath
        self.master = master
        self.slaves = slaves
        self.params = params

class Master(object):
    '''
    classdocs
    '''
    
    name = None
    coorServerPort = None
    
    def __init__(self, name):
        '''
        Constructor
        '''
        self.name = name
        self.coorServerPort = 42070


class Host(object):
    '''
    classdocs
    '''
    
    name = None
    outputBase = None
    
    def __init__(self, name, outputBase):
        '''
        Constructor
        '''
        self.name = name
        self.outputBase = outputBase


class Test(object):
    '''
    classdocs
    '''
    
    id = None
    repeat = None
    vars = None
    params = None
    
    def __init__(self, id, repeat, vars, params):
        '''
        Constructor
        '''
        self.id = id
        self.repeat = repeat
        self.vars = vars
        self.params = params
    

class Node(object):
    '''
    classdocs
    '''
    
    id = None
    host = None
    dgenPath = None
    dgenName = None
    outputBase = None
    nodeConfig = None
    
    def __init__(self, id, host, dgenPath, dgenName, outputBase, nodeConfig):
        '''
        Constructor
        '''
        self.id = id
        self.host = host
        self.dgenPath = dgenPath
        self.dgenName = dgenName
        self.outputBase = outputBase
        self.nodeConfig = nodeConfig


class Config(object):
    '''
    Container for dgen / cloud environments
    '''
    cloudEnvs = {}
    dgenName = None
    nodeConfig = None
    dgenConfigs = {}
    testConfigs = {}
    qgenConfigs = {}

    def __init__(self, dgenName, nodeConfig, doc):
        '''
        Constructor
        '''
        
        self.dgenName = dgenName
        self.nodeConfig = nodeConfig
        
        context = doc.xpathNewContext()
        context.xpathRegisterNs("m", NAMESPACE)
        
        # read cloud environments
        for node in context.xpathEval("//m:cloud_environments/m:cloud_env"):
            environment = {} 
            
            context.setContextNode(node)
            # dgen path
            environment["dgenPath"] = node.prop("dgen")
            # master
            environment["master"] = {}
            environment["master"]["name"] = context.xpathEval("./m:master/m:host/@name").pop().content
            # slaves
            environment["slaves"] = [attr.content for attr in context.xpathEval("./m:slaves/m:host/@name")]
            # params
            environment["params"] = {}
            context.setContextNode(node)
            for param in context.xpathEval("./m:parameters/m:parameter"):
                environment["params"][param.prop("name")] = param.prop("value")
        
            self.cloudEnvs[node.prop("id")] = environment
            
        # read dgen configurations
        for node in context.xpathEval("//m:dgen_configurations/m:dgen_config"):
            config = {}
            
            context.setContextNode(node)
            for param in context.xpathEval("./*"):
                if (param.get_name() == "nodes_per_host"):
                    config["nodesPerHost"] = int(param.content)
                if (param.get_name() == "output_base"):
                    config["outputBase"] = str(param.content)

            config["environment"] = self.cloudEnvs[node.prop("cloud")]
            config["nodesTotal"] = config["nodesPerHost"] * len(config["environment"]["slaves"])
            
            self.dgenConfigs[node.prop("id")] = config;
            
        # read test configurations
        for node in context.xpathEval("//m:test-configurations/m:test"):
            vars = {}
            params = {}
            
            context.setContextNode(node)
            for var in context.xpathEval("./m:variables/m:variable"):
                vars[str(var.prop("name"))] = { 
                    "steps": int(var.prop("steps")), 
                    "sel-min": float(var.prop("sel-min")), 
                    "sel-max": float(var.prop("sel-max")) 
                }
                
            for param in context.xpathEval("./m:parameters/m:parameter"):
                params[param.prop("name")] = param.prop("value")
                
            self.testConfigs[str(node.prop("id"))] = {
                'id': str(node.prop("id")),
                'repeat': 1, 
                'vars': vars,
                'params': params,
            }
            
        # read qgen configurations
        for node in context.xpathEval("//m:qgen-configurations/m:qgen-config"):
            suite = []
            
            context.setContextNode(node)
            for testRef in context.xpathEval("./m:test-ref"):
                params = {}
                vars = {}
            
                context.setContextNode(testRef)
                
                for var in context.xpathEval("./m:variables/m:variable"):
                    vars[str(var.prop("name"))] = { 
                        "steps": int(var.prop("steps")), 
                        "sel-min": float(var.prop("sel-min")), 
                        "sel-max": float(var.prop("sel-max")) 
                    }

                for param in context.xpathEval("./m:parameters/m:parameter"):
                    params[param.prop("name")] = param.prop("value")
                
                suite.append(self.mergeTests({
                    'id': str(testRef.prop("ref")), 
                    'repeat': int(testRef.prop("repeat")), 
                    'vars': vars,
                    'params': params
                }))

            self.qgenConfigs[node.prop("id")] = suite;

        # release resources
        context.xpathFreeContext()
        doc.freeDoc()


    def cloud(self, cloudName):
        '''
        Get the cloud environment identified by the given name
        '''
        
        if (not self.cloudEnvs.has_key(cloudName)):
            raise UnknownConfigObjectException("cloud-env", cloudName)
        
        cloudEnv = self.cloudEnvs[cloudName]
        
        return Environment(cloudEnv["dgenPath"], cloudEnv["master"], cloudEnv["slaves"], cloudEnv["params"])

    def cloudForDgenConfig(self, configName):
        '''
        Get the cloud environment referenced by specific dgen config 
        '''
        
        if (not self.dgenConfigs.has_key(configName)):
            raise UnknownConfigObjectException("dgen-config", configName)
        
        config = self.dgenConfigs[configName]
        cloudEnv = config["environment"]

        return Environment(cloudEnv["dgenPath"], cloudEnv["master"], cloudEnv["slaves"], cloudEnv["params"])

    def master(self, configName):
        '''
        Get the host list for a specific dgen config
        '''
        
        if (not self.dgenConfigs.has_key(configName)):
            raise UnknownConfigObjectException("dgen-config", configName)
        
        config = self.dgenConfigs[configName]
        cloudEnv = config["environment"]
        
        return Master(cloudEnv["master"]["name"]) 

    def nodes(self, configName):
        '''
        Get the node list for a specific dgen config
        '''
        
        if (not self.dgenConfigs.has_key(configName)):
            raise UnknownConfigObjectException("dgen-config", configName)
        
        config = self.dgenConfigs[configName]
        cloudEnv = config["environment"]
        m = config["nodesPerHost"]
        
        return [Node(i, cloudEnv["slaves"][int(i / m)], cloudEnv["dgenPath"], self.dgenName, config["outputBase"], self.nodeConfig) for i in xrange(0, config["nodesTotal"])] 

    def slaves(self, configName):
        '''
        Get the host list for a specific dgen config
        '''
        
        if (not self.dgenConfigs.has_key(configName)):
            raise UnknownConfigObjectException("dgen-config", configName)
        
        config = self.dgenConfigs[configName]
        cloudEnv = config["environment"]
        
        return [Host(name, config["outputBase"]) for name in cloudEnv["slaves"]] 
        
    def tests(self, suiteName):
        '''
        Get the list of tests associated with a specific tests suite
        '''
        
        if (not self.qgenConfigs.has_key(suiteName)):
            raise UnknownConfigObjectException("qgen-config", suiteName)
        
        return [Test(t['id'], t['repeat'], t['vars'], t['params']) for t in self.qgenConfigs[suiteName]]
    
    def mergeTests(self, testRef):
        test = self.testConfigs[testRef['id']]
        
        testRef['vars'] = dict(test['vars'].items() + testRef['vars'].items())
        testRef['params'] = dict(test['params'].items() + testRef['params'].items())

        return testRef

class SizingConfig(object):
    '''
    TODO: this is model specific an probably not required for the next version of the frontend script
    '''
    
    sf = None
    cardinalities = None

    def __init__(self, p, sf):
        
        self.sf = sf
        self.cardinalities = {}
        
        timeFormat = "%Y-%m-%d"
        min = datetime.datetime.fromtimestamp(time.mktime(time.strptime(p.getProperty("sizing.date.min-date"), timeFormat)))
        max = datetime.datetime.fromtimestamp(time.mktime(time.strptime(p.getProperty("sizing.date.max-date"), timeFormat)))
        diff = max - min
        
        self.cardinalities['customer'] = int(sf * int(p.getProperty("sizing.customer.base-cardinality"))) 
        self.cardinalities['date'] = int(diff.days)
        self.cardinalities['market_region'] = int(p.getProperty("sizing.market_region.cardinality")) 
        self.cardinalities['product'] = int(sf * int(p.getProperty("sizing.product.base-cardinality"))) 
        self.cardinalities['product_class'] = int(p.getProperty("sizing.product_class.cardinality")) 
        self.cardinalities['product_offer'] = int(sf * int(p.getProperty("sizing.product_offer.base-cardinality"))) 
        self.cardinalities['reseller'] = int(sf * int(p.getProperty("sizing.reseller.base-cardinality"))) 
        self.cardinalities['time'] = int(p.getProperty("sizing.time.cardinality")) 
        self.cardinalities['address'] = self.cardinalities['customer'] + self.cardinalities['reseller']
        
    def cardinality(self, name):
        return self.cardinalities[name]

def readConfig(dgenName, nodeConfig, configPath):
    print configPath
    return Config(dgenName, nodeConfig, libxml2.parseFile(configPath))

def readDGenConfig(configPath):
    f = None
    try:
        f = open(configPath)
        properties = util.properties.Properties()
        properties.load(f)
        f.close()

        return properties
    except:
        if f != None:
            f.close()
        raise

def readSizingConfig(configPath, sf = 1.0):
    return SizingConfig(readDGenConfig(configPath), sf)