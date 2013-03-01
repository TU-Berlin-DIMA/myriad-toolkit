/*
 * Copyright 2010-2013 DIMA Research Group, TU Berlin
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#include "communication/CommunicationSubsystem.h"
#include "generator/AbstractSequenceGenerator.h"

#include <Poco/Runnable.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>

using namespace std;
using namespace Poco;

namespace Myriad {
/**
 * @addtogroup communication
 * @{*/

////////////////////////////////////////////////////////////////////////////////
/// @name Helper Classes
////////////////////////////////////////////////////////////////////////////////
//@{

/**
 * Implements a thread for progress monitoring.
 *
 * The thread periodically posts UpdateProgress notifications to the
 * NotificationCenter to poll all running StageTask instances for their
 * progress. After each update, the thread also shares a new Heartbeat via a
 * shared NotificationQueue.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class ProgressMonitor: public Runnable
{
public:

    /**
     * Constructor.
     */
    ProgressMonitor(NotificationCenter& notificationCenter, NotificationQueue& notificationQueue, NodeState& state, I32u interval) :
        _notificationCenter(notificationCenter), _notificationQueue(notificationQueue), _state(state), _interval(interval), _logger(Logger::get("progress.monitor")), _ui(Logger::get("ui"))
    {
    }

    /**
     * Destructor.
     */
    virtual ~ProgressMonitor()
    {
        _ui.information("Stopping progress monitor");
    }

    /**
     * Implements the thread loop.
     */
    void run();

private:

    NotificationCenter& _notificationCenter;
    NotificationQueue& _notificationQueue;

    NodeState& _state;
    I32u _interval;

    Logger& _logger;
    Logger& _ui;
};

/**
 * An heartbeat client thread.
 *
 * The thread consumes heartbeat notifications and forwards them via HTTP to a
 * central coordination service.
 */
class HeartbeatClient: public Runnable
{
public:

    /**
     * Constructor.
     *
     * The \p notificationCenter and \p notificationQueue parameters are used
     * for shared communication, and the \p host and \p port for connection
     * with the coordination service.
     */
    HeartbeatClient(NotificationCenter& notificationCenter, NotificationQueue& notificationQueue, const String& host, I16u port) :
        _notificationCenter(notificationCenter),
        _notificationQueue(notificationQueue),
        _serverHost(host),
        _serverPort(port),
        _logger(Logger::get("heartbeat.client")),
        _hb(Logger::get("heartbeat.logger")),
        _ui(Logger::get("ui"))
    {
    }

    /**
     * Constructor.
     */
    virtual ~HeartbeatClient()
    {
        _ui.information("Stopping heartbeat client");
    }

    /**
     * Implements the thread loop.
     */
    void run();

private:

    NotificationCenter& _notificationCenter;
    NotificationQueue& _notificationQueue;

    String _serverHost;
    I16u _serverPort;

    Logger& _logger;
    Logger& _hb;
    Logger& _ui;
};

/**
 * A specific notification subtype used by the CommunicationSubsystem to stop
 * the heartbeat client when the subsystem on shutdown.
 */
class StopHeartbeatClient : public Notification
{
};

//@}

////////////////////////////////////////////////////////////////////////////////
/// @name Method Implementations
////////////////////////////////////////////////////////////////////////////////
//@{

void CommunicationSubsystem::initialize(Application& app)
{
    if (_initialized)
    {
        return;
    }

    try
    {
        // set config and reset node status state
        _config = &(app.config());
        _state.reset(_config->getInt("common.partitioning.chunks-id"), AbstractSequenceGenerator::STAGES.size());

        // attach observers
        _notificationCenter.addObserver(NObserver<CommunicationSubsystem, StartStage> (*this, &CommunicationSubsystem::onStageStart));
        _notificationCenter.addObserver(NObserver<CommunicationSubsystem, ChangeNodeState> (*this, &CommunicationSubsystem::onSatusChange));
    }
    catch (const Exception& exc)
    {
        _logger.error(format("Exception caught in communication subsystem: %s", exc.displayText()));
        exc.rethrow();
    }
    catch (const std::exception& exc)
    {
        _logger.error(format("Exception caught in communication subsystem: %s", string(exc.what())));
        throw exc;
    }
    catch (...)
    {
        _logger.error("Unknown exception caught in communication subsystem");
        throw;
    }

    _initialized = true;
}

void CommunicationSubsystem::uninitialize()
{
    if (!_initialized)
    {
        return;
    }

    if (_state.currentState < NodeState::ABORTED)
    {
        // if no error status was set until now, assume evertything went fine
        _state.currentState = NodeState::READY;
    }

    // close update thread and unset config
    _progressMonitor.join();
    _heartbeatClient.join();
    _config = NULL;

    // remove attached observers
    _notificationCenter.removeObserver(NObserver<CommunicationSubsystem, StartStage> (*this, &CommunicationSubsystem::onStageStart));
    _notificationCenter.removeObserver(NObserver<CommunicationSubsystem, ChangeNodeState> (*this, &CommunicationSubsystem::onSatusChange));

    _initialized = false;
}

void CommunicationSubsystem::start()
{
    try
    {
        ProgressMonitor* monitor = new ProgressMonitor(_notificationCenter, _notificationQueue, _state, _config->getInt("coordinator.progress.update.interval", 3000));
        HeartbeatClient* client = new HeartbeatClient(_notificationCenter, _notificationQueue, _config->getString("application.coordinator.host", "localhost"), _config->getInt("application.coordinator.port", 42070));

        _progressMonitor.start(*monitor);
        _heartbeatClient.start(*client);
    }
    catch (const Exception& exc)
    {
        _logger.error(format("Exception caught in communication subsystem: %s", exc.displayText()));
        exc.rethrow();
    }
    catch (const std::exception& exc)
    {
        _logger.error(format("Exception caught in communication subsystem: %s", string(exc.what())));
        throw exc;
    }
    catch (...)
    {
        _logger.error("Unknown exception caught in communication subsystem");
        throw;
    }
}

void CommunicationSubsystem::onStageStart(const AutoPtr<StartStage>& notification)
{
    _state.currentGeneratorStageID = notification->stageID;
}

void CommunicationSubsystem::onSatusChange(const AutoPtr<ChangeNodeState>& notification)
{
    if (_state.currentState < notification->newNodeState)
    {
        _ui.information(format("Changing node status %u -> %u", static_cast<unsigned int>(_state.currentState), static_cast<unsigned int>(notification->newNodeState)));
        _state.currentState = notification->newNodeState;

        _notificationQueue.enqueueNotification(new Heartbeat(_state));

        if (_state.currentState >= NodeState::READY)
        {
	        _notificationQueue.enqueueNotification(new StopHeartbeatClient());
        }
    }
}

void ProgressMonitor::run()
{
    while (_state.currentState < NodeState::READY)
    {
        if (_state.currentState == NodeState::ALIVE)
        {
	        _notificationCenter.postNotification(new UpdateProgress(_state));
	        _notificationQueue.enqueueNotification(new Heartbeat(_state));
        }

        Thread::current()->sleep(_interval);
    }

    delete this;
}

void HeartbeatClient::run()
{
    // FIXME: maintain client session as a private member
    HTTPClientSession session(_serverHost, _serverPort);
    // bad request counter
    I16u badRequestCounter = 0;
    // request
    HTTPRequest request("HTTP/1.1");
    request.setMethod("HEAD");
    // response
    HTTPResponse response;

    AutoPtr<Notification> notification = _notificationQueue.waitDequeueNotification();

    while (true)
    {
        if (NULL != dynamic_cast<StopHeartbeatClient*> (notification.get()))
        {
	        break;
        }

        Heartbeat* heartbeat = dynamic_cast<Heartbeat*> (notification.get());
        if (NULL != heartbeat)
        {
	        string uri = format("/heartbeat?id=%u&status=%u&stage=%u&progress=%f", heartbeat->state.nodeID, static_cast<unsigned int> (heartbeat->state.currentState), static_cast<unsigned int> (heartbeat->state.currentGeneratorStageID), heartbeat->state.progress());
	        string msg = format("heartbeat: { id: %u, status: %u, stage: %u, progress: %f }", heartbeat->state.nodeID, static_cast<unsigned int> (heartbeat->state.currentState), static_cast<unsigned int> (heartbeat->state.currentGeneratorStageID), heartbeat->state.progress());

	        _hb.information(format("task count    %f", heartbeat->state.taskCount));
	        _hb.information(format("task progress %f", heartbeat->state.taskProgress));

	        _hb.information(msg);

	        if (badRequestCounter < 20)
	        {
		        try
		        {
			        request.setURI(uri);
			        session.sendRequest(request);
			        session.receiveResponse(response);
		        }
		        catch (const Exception& exc)
		        {
			        _logger.warning(format("Error in heartbeat request #%hu: %s", ++badRequestCounter, exc.displayText()));

			        if (badRequestCounter == 20)
			        {
				        _logger.warning(format("Discontinuing communication with coordination service at `%s:%hu`", session.getHost(), session.getPort()));
			        }
		        }
	        }
        }

        notification = _notificationQueue.waitDequeueNotification(4000);
    }

    delete this;
}

//@}

/** @}*/// add to communication group
} // namespace Myriad
