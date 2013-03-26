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
 
Created on Mar 7, 2011

@author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
'''
import sys, tempfile, optparse
import config
from myriad.util import stringutil, sysutil

class Importer(object):
    '''
    classdocs
    '''
    basePath = None
    systemName = None
    config = None
    parser = None
    logBase = None

    sf = None
    configDir = None
    datasetID = None
    configName = None
    execute = None
    
    cloud = None
    
    commonParams = {}
    
    smallRelations = [ "customer_demographics", "date", "market_region", "product_class", "time" ];
    largeRelations = [ "address", "customer", "lineitem", "order", "order_return", "product", "product_offer", "reseller", "shipment" ];
    
    log = None
    
    VERSION = "0.3.0"

    def __init__(self, basePath, systemName, argv):
        '''
        Constructor
        '''
        self.basePath = basePath
        self.systemName = systemName
        self.initialize(argv)
        
    def initialize(self, argv):
        parser = optparse.OptionParser(usage="%prog <target-system> [options] <dgen-config>", version=self.VERSION)
        self.configureParser(parser)
        self.parser = parser

        args, remainder = parser.parse_args(argv)
        if (len(remainder) != 1):
            self.error(None, True)
            raise
        
        self.log = sysutil.createLogger("myriad.importer")
        
        try:
            # process command line arguments
            self.setup(args, remainder)
                    
            if (self.logBase == None):
                # create log dir
                self.logBase = tempfile.mkdtemp("", "myriad-import-%s_" % (self.datasetID))
            
            # make sure that logBase directories exist
            sysutil.checkDir(self.logBase)
            
            # register file handler to the logger
            sysutil.registerFileHandler(self.log, "%s/myriad-importer.log" % (self.logBase))
            
        except config.UnknownConfigObjectException, e:
            self.error(str(e), True)
            raise e
        except:
            e = sys.exc_info()[1]
            self.error("unexpected error: %s" % (str(e)))
            raise
        
    def configureParser(self, parser):
        parser.add_option("-s", dest="sf", type="float", default=1.0, 
                          help="scaling factor of the imported dataset")
        parser.add_option("-m", dest="dataset", type="str", default="default-dataset", 
                          help="ID of the Myriad dataset to import")
        parser.add_option("--config-dir", dest="config_dir", type="str", default="%s/config" % (self.basePath), 
                          help="path to the myriad config folder")
        parser.add_option("--log-dir", dest="log_dir", type="str", default=None, 
                          help="base directory for import output logging")
        parser.add_option("--cleanup", dest="execute", action="store_const", const="cleanup", default="import",
                          help="cleanup previously imported data")
        
    def setup(self, args, remainder):
        self.sf = args.sf
        self.configDir = args.config_dir
        self.datasetID = args.dataset
        self.configName = remainder.pop()
        self.logBase = args.log_dir
        self.execute = args.execute
        
        # load myriad config 
        self.config = config.readConfig("%s/myriad-config.xml" % (self.configDir))
        # load cloud config
        self.cloud = self.config.cloudForDgenConfig(self.configName)
        
        # extend common parameters dictionary
        self.commonParams.update({ 'dataset.id': self.datasetID })  

    def getParameters(self):
        params = stringutil.TextSub(dict(self.cloud.params.items() + self.commonParams.items()))
        params.compile()
        return params
        
    def error(self, message = None, withUsage = False):
        if (withUsage):
            self.parser.print_usage(sys.stderr)
        if (message != None):
            self.log.error("%s: error: %s", self.parser.get_prog_name(), message)

    def cleanup(self):
        pass