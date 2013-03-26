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
 
Created on Mar 2, 2011

@author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
'''

import os, sys, tempfile, optparse, datetime, time
import BaseHTTPServer
import config
from urlparse import urlparse
from SocketServer import ThreadingMixIn
from threading import Thread, Lock, RLock
from myriad.util import sysutil, timeutil
import httplib

class DGen(object):
    '''
    classdocs
    '''
    basePath = None
    dgenName = None
    nodeName = None
    
    config = None
    dgenConfig = None
    parser = None
    logBase = None

    cleanup = None
    sf = None
    configPath = None
    nodeConfig = None
    datasetID = None
    executeStages = None
    configName = None
    serverAddress = None
    
    dgenMaster = None
    dgenNodes = None
    
    started = None
    finished = None
    
    log = None
    
    VERSION = "0.3.0"

    def __init__(self, basePath, dgenName, argv):
        '''
        Constructor
        '''
        
        self.basePath = basePath
        self.dgenName = dgenName
        self.initialize(argv)
        self.dgenNodes = []
    
    def initialize(self, argv):
        
        parser = optparse.OptionParser(usage="%prog [options] <dgen-config>", version=self.VERSION)
        parser.add_option("-s", dest="sf", type="float", default=1.0,
                          help="scaling factor (sf=1 generates 1GB data)")
        parser.add_option("-m", dest="dataset_id", type="str", default="default-dataset",
                          help="ID of the generated Myriad dataset")
        parser.add_option("-x", dest="execute_stages", action="append", type="str", default=[],
                          help="Specify a specific stage to execute")
        parser.add_option("-n", dest="node_config", type="str", default="%s-node.xml" % (self.dgenName),
                          help="name of the node config file (should reside in the config dir)")
        parser.add_option("--config-dir", dest="config_path", type="str", default="%s/config" % (self.basePath),
                          help="path to the myriad config folder (TODO)")
        parser.add_option("--log-dir", dest="log_dir", type="str", default=None, 
                          help="base directory for output logging")
        parser.add_option("--cleanup", dest="cleanup", action="store_true",
                          help="remove output from previously generated job")
            
        self.parser = parser

        args, remainder = parser.parse_args(argv)
        if (len(remainder) != 1):
            self.error(None, True)
            raise
        
        self.log = sysutil.createLogger("myriad.dgen")

        try:
            self.cleanup = args.cleanup
            self.sf = args.sf
            self.datasetID = args.dataset_id
            self.configPath = args.config_path
            self.nodeConfig = args.node_config
            self.logBase = args.log_dir
            self.executeStages = args.execute_stages
            self.configName = remainder.pop()
            
            # load myriad config 
            self.config = config.readConfig(self.dgenName, self.nodeConfig, "%s/%s-frontend.xml" % (self.configPath, self.dgenName))
            # load sizing config
            self.dgenConfig = config.readDGenConfig("%s/%s-node.properties" % (self.configPath, self.dgenName))
            
            DGenNode.MAX_ATTEMPTS = int(self.dgenConfig.getProperty("coordinator.node.max.attempts", DGenNode.MAX_ATTEMPTS))
            DGenNode.DEAD_TIMEOUT = datetime.timedelta(0, 0, 0, int(self.dgenConfig.getProperty("coordinator.node.dead.timeout", DGenNode.DEAD_TIMEOUT.seconds*1000)))
            NodeMonitor.POLL_INTERVAL = int(self.dgenConfig.getProperty("coordinator.node.monitor.interval", NodeMonitor.POLL_INTERVAL*1000))/1000.0
            
            if (self.logBase == None):
                # create log dir
                self.logBase = tempfile.mkdtemp("", "%s-frontend-%s_" % (self.dgenName, self.datasetID))
            
            # make sure that logBase directories exist
            sysutil.checkDir(self.logBase)
            
            # register file handler to the logger
            sysutil.registerFileHandler(self.log, "%s/%s-frontend.log" % (self.logBase, self.dgenName))

        except:
            e = sys.exc_info()[1]
            self.error("unexpected error: %s" % (str(e)), True)
            raise
            
    def run(self):
        '''
        Srart the distributed generation process using the specified dgen configName.
        '''
        
        self.started = datetime.datetime.now()
        
        server = None
        monitor = None
        
        try:
            if (self.cleanup):
                slaves = self.config.slaves(self.configName)
            
                self.log.info("~" * 55)
                self.log.info("Myriad Parallel Data Generator (Version %s)", self.VERSION)
                self.log.info("~" * 55)
                self.log.info("cleaning configuration `%s`", self.configName)
                
                for h in slaves:
                    DGenHost(h).clean(self)
                
            else:
                master = self.config.master(self.configName)
                nodes = self.config.nodes(self.configName)
            
                self.log.info("~" * 55)
                self.log.info("Myriad Parallel Data Generator (Version %s)", self.VERSION)
                self.log.info("~" * 55)
                self.log.info("running configuration `%s` with scaling factor %.3f", self.configName, self.sf)
                
                self.dgenMaster = master
                self.dgenNodes = [ DGenNode(n) for n in nodes ]
                
                self.log.info("starting heartbeat server on address `%s:%d`", self.dgenMaster.name, self.dgenMaster.coorServerPort)    
                server = HeartbeatServer(self.datasetID, self.dgenNodes, ('0.0.0.0', self.dgenMaster.coorServerPort))
                
                # start node monitor
                self.log.info("starting node monitor thread")
                monitor = NodeMonitor(self, server)
                monitor.start()

                # start server loop
                serverThread = Thread(target=server.serveLoop) 
                serverThread.start()
                
                self.log.info("starting %d generator nodes", len(self.dgenNodes))
                self.startNodes()
                
                # wait for server thread to finish (timeout and loop needed for KeyboardInterrupt)
                while(serverThread.isAlive()):
                    serverThread.join(3.0)

                # wait for monitor thread
                monitor.join()
                
                if (monitor.exception):
                    self.log.error("interrupting generation process after failure in node %d ", monitor.exception.id)
                    raise monitor.exception
                
                # abort all running nodes
                self.abortAllNodes()
                            
                self.finished = datetime.datetime.now()
                self.log.info("generator process finished in %s seconds", timeutil.formatTime(self.finished - self.started))
                    
        except KeyboardInterrupt:
            self.log.warning("execution interrupted by user")
            if (monitor != None):
                monitor.shutdown()
            self.abortAllNodes()
            raise
        except NodeFailureException, e:
            self.abortAllNodes()
            if (monitor != None):
                monitor.shutdown()
            self.error(str(e), False)
            raise
        except config.UnknownConfigObjectException, e:
            self.abortAllNodes()
            self.error(str(e), False)
            raise
        except:
            e = sys.exc_info()[1]
            if (monitor != None):
                monitor.shutdown()
            self.error(str(e), False)
            raise
            self.abortAllNodes()

    def startNodes(self):
        for node in self.dgenNodes:
            node.start(self, len(self.dgenNodes))

    def abortAllNodes(self):
        for node in self.dgenNodes:
            node.abort(self, len(self.dgenNodes))

    def error(self, message=None, withUsage = False):
        if (withUsage):
            self.parser.print_usage(sys.stderr)
        if (message != None):
            self.log.error("%s: error: %s", self.parser.get_prog_name(), message)


class NodeMonitor(Thread):

    POLL_INTERVAL = 5.0
    
    dgen = None
    server = None
    exception = None
    isShutdown = False
    
    log = None

    def __init__(self, dgen, server):
        Thread.__init__(self)
        self.dgen = dgen
        self.server = server
        self.isShutdown = False
        
        self.log = sysutil.getExistingLogger("myriad.dgen")

    def run(self):
        
        while (not self.isShutdown):
            
            time.sleep(3.0)
            
            self.server.nonReadyLock.acquire()
            
            try:
                
                if (self.server.nonReady == 0):
                    self.isShutdown = True
        
                for node in self.server.nodes:
                    if (node.isDead()):
                        self.log.warning("restarting dead node #%d", node.id)
                        node.restart(self.dgen, len(self.server.nodes))
            
            except NodeFailureException, e:
                self.isShutdown = True
                self.exception = e
            
            self.server.nonReadyLock.release()

        self.server.stopServeLoop()
        
    def shutdown(self):
        self.isShutdown = True

class HeartbeatServer(ThreadingMixIn, BaseHTTPServer.HTTPServer):

    datasetID = None  
    nodes = []
    nonReady = None
    nonReadyLock = None
    isShutdown = False

    def __init__(self, datasetID, nodes, address):
        BaseHTTPServer.HTTPServer.__init__(self, address, RequestHandler)
        self.datasetID = datasetID
        self.nodes = nodes
        self.nonReady = len(nodes)
        self.nonReadyLock = Lock()
        self.isShutdown = False
        
    def serveLoop(self):
        while (not self.isShutdown):
            self.handle_request()
            
    def stopServeLoop(self):
        self.isShutdown = True
        self.makeSentinelRequest()

    def makeSentinelRequest(self):
        try:
            conn = httplib.HTTPConnection(self.server_address[0], self.server_address[1])
            conn.request("GET", "/sentinel")
            conn.getresponse()
            conn.close()
        except:
            pass


class RequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    
    GET_REFRESH = 8000
    
    def do_HEAD(self):
        self.send_response(200)
        self.end_headers()
        
        result = urlparse(self.path, '', False)
        params = dict([part.split('=') for part in result[4].split('&')])

        status = int(params['status'])
        
        if (status >= DGenNode.INITIALIZING and status <= DGenNode.ABORTED): # valid values for heartbeats
        
            node = self.server.nodes[int(params['id'])]
            
            node.lock.acquire()

            if (status == DGenNode.READY):
                self.server.nonReadyLock.acquire()
            
            node.lastBeat = datetime.datetime.now()

            if (node.status != status and node.status < status):
                log = sysutil.getExistingLogger("myriad.dgen")
                log.info("node %05d: %s -> %s", node.id, DGenNode.STATUS_STRING[node.status], DGenNode.STATUS_STRING[status])
                node.status = status
            
            if (node.status == DGenNode.ACTIVE):
                node.progress = float(params['progress'])
                
            elif (node.status == DGenNode.READY):
                node.progress = 1.0
                node.finished = datetime.datetime.now()
                self.server.nonReady -= 1
                
            elif (node.status == DGenNode.ABORTED or node.status == DGenNode.FAILED):
                pass

            else:
                log = sysutil.getExistingLogger("myriad.dgen")
                log.error("unknown status %d for node %d", node.status, node.id)
                
            if (status == DGenNode.READY):
                self.server.nonReadyLock.release()

            node.lock.release()

    def do_GET(self):
        
        if (self.path == '/sentinel'): 
            self.send_response(200)
            self.end_headers()
            return
        
        elif (self.path != '/'): 
            self.send_response(404)
            self.end_headers()
            return
        
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        
        print >> self.wfile, "<html>"
        print >> self.wfile, "<head>"
        print >> self.wfile, "  <title>Myriad Parallel Data Generator &raquo; % s</title>" % (self.server.datasetID)
        print >> self.wfile, "  <script type='text/JavaScript'>"
        print >> self.wfile, "  <!--"
        print >> self.wfile, "  function timedRefresh(timeoutPeriod) {"
        print >> self.wfile, "      setTimeout('location.reload(true);', timeoutPeriod);"
        print >> self.wfile, "  }"
        print >> self.wfile, "  //   -->"
        print >> self.wfile, "  </script>"
        print >> self.wfile, "</head>"
        print >> self.wfile, "<body style='margin: 0; padding: 2ex 2em; font-size: 14px;' onload='javascript:timedRefresh(%d);'>" % (self.GET_REFRESH)
        print >> self.wfile, "<div id='header' style='text-align: center;'>"
        print >> self.wfile, "  <h1 style='color: #333; font-size: 2em; margin: 0 0 0.5ex 0; padding: 0;'>Myriad Parallel Data Generator</h1>"
        print >> self.wfile, "  <h2 style='color: #333; font-size: 1.5em; margin: 0 0 3ex 0; padding: 0;'>Job coordinator for dataset &raquo;%s&laquo; </h2>" % (self.server.datasetID)
        print >> self.wfile, "</div>"
        print >> self.wfile, "<table style='width: 100%; border: 1px solid #999;' cellspacing='5' cellpadding='0'>"
        print >> self.wfile, "<thead>"
        print >> self.wfile, "<tr>"
        print >> self.wfile, "  <td style='width: 10%; background: #454545; color: #fafafa; padding: 0.5ex'>Node #</td>"
        print >> self.wfile, "  <td style='width: 20%; background: #454545; color: #fafafa; padding: 0.5ex'>Hostname</td>"
        print >> self.wfile, "  <td style='width: 35%; background: #454545; color: #fafafa; padding: 0.5ex'>Progress</td>"
        print >> self.wfile, "  <td style='width: 15%; background: #454545; color: #fafafa; padding: 0.5ex'>Status</td>"
        print >> self.wfile, "  <td style='width: 10%; background: #454545; color: #fafafa; padding: 0.5ex'>Attempt #</td>"
        print >> self.wfile, "  <td style='width: 10%; background: #454545; color: #fafafa; padding: 0.5ex'>Time</td>"
        print >> self.wfile, "</tr>"
        print >> self.wfile, "</thead>"
        print >> self.wfile, "<tbody>"
        for n in self.server.nodes:
            print >> self.wfile, "<tr>"
            print >> self.wfile, "  <td style='background: #fafafa; color: #454545; padding: 0.5ex'>%05d</td>" % (n.id)
            print >> self.wfile, "  <td style='background: #fafafa; color: #454545; padding: 0.5ex'>%s</td>" % (n.host)
            print >> self.wfile, "  <td style='background: #fafafa; color: #454545; padding: 0.5ex'><span style='float: left; width: 15%%;'>%02d%%</span><span style='float: left; width: %d%%; border-left: 1px solid #666; background: #666; color: #666; overflow: hidden;'>&raquo;</span></td>" % (100 * n.progress, 80 * n.progress)
            print >> self.wfile, "  <td style='background: #fafafa; color: #454545; padding: 0.5ex'>%s</td>" % (DGenNode.STATUS_STRING[n.status])
            print >> self.wfile, "  <td style='background: #fafafa; color: #454545; padding: 0.5ex'>%d</td>" % (n.attempt)
            print >> self.wfile, "  <td style='background: #fafafa; color: #454545; padding: 0.5ex'>%s</td>" % (timeutil.formatTime(n.lastBeat - n.started))
            print >> self.wfile, "</tr>"
        print >> self.wfile, "</tbody>"
        print >> self.wfile, "</table>"
        print >> self.wfile, "<body>"
        print >> self.wfile, "</html>"
    
    def log_request(self, code='-', size='-'):
        '''
        Disable request logging for the communication server
        '''
        pass

class DGenHost(config.Host):
    '''
    classdocs
    '''
    name = None
    outputBase = None
    

    def __init__(self, envNode):
        '''
        Constructor
        '''
        self.name = envNode.name
        self.outputBase = envNode.outputBase 
    
    def clean(self, dgen):
        log = sysutil.getExistingLogger("myriad.dgen")
        log.info("clearing host `%s`", self.name)
        os.system("ssh %s 'rm -Rf %s/%s'" % (self.name, self.outputBase, dgen.datasetID))


class DGenNode(config.Node):
    '''
    classdocs
    '''
    id = None
    host = None
    dgenPath = None
    dgenName = None
    outputBase = None
    
    lock = None
    
    attempt = None
    progress = None
    status = None
    started = None
    finished = None
    lastBeat = None
    
    NEW = -1
    INITIALIZING = 0
    ACTIVE = 1
    READY = 2
    ABORTED = 3
    FAILED = 4
    
    DEAD_TIMEOUT = datetime.timedelta(0, 30)
    MAX_ATTEMPTS = 3
    
    STATUS_STRING = {
       -1: "NEW",
        0: "INITIALIZING",
        1: "ACTIVE",
        2: "READY",
        3: "ABORTED",
        4: "FAILED",
    }

    def __init__(self, envNode):
        '''
        Constructor
        '''
        self.id = envNode.id
        self.host = envNode.host
        self.dgenPath = envNode.dgenPath
        self.dgenName = envNode.dgenName
        self.outputBase = envNode.outputBase
        self.nodeConfig = envNode.nodeConfig
        
        self.attempt = 0
        self.resetState()

        
        self.lock = RLock()
    
    def start(self, dgen, nodesTotal):
        self.lock.acquire();
        
        os.system("ssh -f %s '%s/bin/%s-node -s%.3f -m%s -i%d -N%d -H%s -P%d -o%s -n%s %s > /dev/null 2> /dev/null &'" % (self.host, self.dgenPath, self.dgenName, dgen.sf, dgen.datasetID, self.id, nodesTotal, dgen.dgenMaster.name, dgen.dgenMaster.coorServerPort, self.outputBase, self.nodeConfig, ' '.join(map(lambda s: '-x%s' % s, dgen.executeStages))))

        self.attempt += 1
        self.resetState()

        self.lock.release()
    
    def restart(self, dgen, nodesTotal):
        self.lock.acquire();
        
        if (self.attempt < DGenNode.MAX_ATTEMPTS):
            os.system("ssh -f %s '%s/bin/%s-kill %d %s > /dev/null 2> /dev/null'" % (self.host, self.dgenPath, self.dgenName, self.id, dgen.datasetID))
            os.system("ssh -f %s '%s/bin/%s-node -s%.3f -m%s -i%d -N%d -H%s -P%d -o%s -n%s %s > /dev/null 2> /dev/null &'" % (self.host, self.dgenPath, self.dgenName, dgen.sf, dgen.datasetID, self.id, nodesTotal, dgen.dgenMaster.name, dgen.dgenMaster.coorServerPort, self.outputBase, self.nodeConfig, ' '.join(map(lambda s: '-x%s' % s, dgen.executeStages))))

            self.attempt += 1
            self.resetState()
            
            self.lock.release()
        
        else:
            self.status = DGenNode.FAILED
            self.lock.release()
            raise NodeFailureException(self.id)
        
    def abort(self, dgen, nodesTotal):
        self.lock.acquire()
        
        if (self.status < DGenNode.READY):
            log = sysutil.getExistingLogger("myriad.dgen")
            log.info("aborting node #%03d" % (self.id))
            os.system("ssh -f %s '%s/bin/%s-kill %d %s > /dev/null 2> /dev/null'" % (self.host, self.dgenPath, self.dgenName, self.id, dgen.datasetID))
            self.status = DGenNode.FAILED

        self.lock.release()
            
    def isDead(self):
        self.lock.acquire()
            
        if (self.status == DGenNode.FAILED):
            self.lock.release()
            raise NodeFailureException(self.id)
        
        if (self.status == DGenNode.READY):
            self.lock.release()
            return False
            
        elif (self.status == DGenNode.ABORTED):
            self.lock.release()
            return True
        
        else:
            diff = datetime.datetime.now() - self.lastBeat
            self.lock.release()
            return diff > DGenNode.DEAD_TIMEOUT

    def resetState(self):
        self.progress = 0.0
        self.status = DGenNode.NEW
        self.started = datetime.datetime.now()
        self.lastBeat = datetime.datetime.now()


class NodeFailureException(RuntimeError):
    
    id = None
    
    def __init__(self, id):
        self.id = id
        
    def __str__(self):
        return "node %d failed" % (self.id)