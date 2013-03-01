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

#ifndef COMMUNICATIONSUBSYSTEM_H_
#define COMMUNICATIONSUBSYSTEM_H_

#include "communication/Notifications.h"

#include <string>
#include <Poco/NotificationCenter.h>
#include <Poco/NotificationQueue.h>
#include <Poco/Thread.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Util/Application.h>
#include <Poco/Util/Subsystem.h>

using namespace std;
using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

namespace Myriad {
/**
 * @addtogroup communication
 * @{*/

/**
 * The CommunicationSubsystem implements an application sybsystem which
 * coordinates and executes the lifecycle of all data generation components.
 * This is the core of all controller-related data generation-related logic.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class CommunicationSubsystem: public Util::Subsystem
{
public:

    /**
     * Constructor.
     */
    CommunicationSubsystem(NotificationCenter& notificationCenter) :
        _notificationCenter(notificationCenter), _state(), _progressMonitor("progress_monitor"), _heartbeatClient("heartbeat_client"), _initialized(false), _logger(Logger::get("communication.driver")), _ui(Logger::get("ui"))
    {
    }

    /**
     * Destructor.
     */
    ~CommunicationSubsystem()
    {
    }

    /**
     * Starts the CommunicationSubsystem instance.
     */
    void start();

    /**
     * StartStage listener.
     *
     * Updates the \p current_stage_id of the internal \p NodeState instance
     * with new stage_id provided by the StartStage notification.
     *
     */
    void onStageStart(const AutoPtr<StartStage>& notification);


    /**
     * ChangeStatus listener.
     *
     * Handles a ChangeNodeState notification, which is issued by the
     * GeneratorSubsystem between each pair of stages.
     * If the notification \p stage status is different than the \p state.status
     * member the \p _state.status is updated and a new Heartbeat notification
     * is sent.
     */
    void onSatusChange(const AutoPtr<ChangeNodeState>& status);

protected:

    /**
     * Returns a constant subsystem name "Communication Subsystem".
     */
    const char* name() const
    {
        return "Communication Subsystem";
    }

    /**
     * Initializes the subsystem.
     *
     * Resets the internal NodeState and attaches the Notification listeners.
     */
    void initialize(Application&);

    /**
     * Sets the node status of the internal _state
     */
    void uninitialize();

private:

    NotificationCenter& _notificationCenter;
    NotificationQueue  _notificationQueue;

    NodeState _state;
    Thread _progressMonitor;
    Thread _heartbeatClient;
    HTTPClientSession _httpSession;
    I16u _badRequestCounter;

    bool _initialized;

    LayeredConfiguration* _config;
    Logger& _logger;
    Logger& _ui;
};

/** @}*/// add to communication group
} // namespace Myriad

#endif /* COMMUNICATIONSUBSYSTEM_H_ */
