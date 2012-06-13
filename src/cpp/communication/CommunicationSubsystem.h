/*
 * Copyright 2010-2011 DIMA Research Group, TU Berlin
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
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
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

class CommunicationSubsystem: public Util::Subsystem
{
public:

	CommunicationSubsystem(NotificationCenter& notificationCenter) :
		_notificationCenter(notificationCenter), _state(), _progressMonitor("progress_monitor"), _heartbeatClient("heartbeat_client"), _initialized(false), _logger(Logger::get("communication.driver")), _ui(Logger::get("ui"))
	{
	}

	~CommunicationSubsystem()
	{
	}

	void start();

	void onStageStart(StartStage* notification);

	void onSatusChange(ChangeStatus* status);

protected:

	const char* name() const
	{
		return "Communication Subsystem";
	}

	void initialize(Application&);

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

} // namespace Myriad

#endif /* COMMUNICATIONSUBSYSTEM_H_ */
