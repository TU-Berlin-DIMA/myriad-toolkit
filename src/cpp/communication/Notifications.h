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

#ifndef PROGRESSUPDATE_H_
#define PROGRESSUPDATE_H_

#include "core/types.h"

#include <Poco/Notification.h>
#include <string>

using namespace std;
using namespace Poco;

namespace Myriad {

class NodeState
{
public:

	enum Status
	{
		PREPARE, ALIVE, READY, ABORTED
	};

	NodeState()
	{
		reset(0, 1);
	}

	void reset(I32u id, I16u numberOfStages)
	{
		status = PREPARE;
		chunkID = id;
		stage = 0;
		progressPerStage = 1 / static_cast<Decimal> (numberOfStages);
		taskCount = 0;
		taskProgress = 0;
	}

	Decimal progress() const
	{
		if (taskCount > 0)
		{
			return (stage + taskProgress / taskCount) * progressPerStage;
		}
		else
		{
			return stage * progressPerStage;
		}
	}

	Status status;
	I32u chunkID;
	I16u stage;
	Decimal progressPerStage;
	Decimal taskCount;
	Decimal taskProgress;
};

class ChangeStatus: public Notification
{
public:

	ChangeStatus(const NodeState::Status status) :
		status(status)
	{
	}

	const NodeState::Status status;
};

class StartStage: public Notification
{
public:

	StartStage(I16u stageID) :
		m_stage_id(stageID)
	{
	}

	I16u m_stage_id;
};

class UpdateProgress: public Notification
{
public:

	UpdateProgress(NodeState& state) :
		state(state)
	{
		state.taskCount = 0;
		state.taskProgress = 0;
	}

	NodeState& state;
};

class Heartbeat: public Notification
{
public:

	Heartbeat(const NodeState& state) :
		state(state)
	{
	}

	const NodeState state;
};

} // namespace Myriad

#endif /* NOTIFICATIONS_H_ */
