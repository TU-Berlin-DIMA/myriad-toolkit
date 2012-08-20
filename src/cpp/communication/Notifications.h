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
 */

#ifndef PROGRESSUPDATE_H_
#define PROGRESSUPDATE_H_

#include "core/types.h"

#include <Poco/Notification.h>
#include <string>

using namespace std;
using namespace Poco;

namespace Myriad {
/**
 * @addtogroup communication
 * @{*/

/**
 * A class representing the state of a data generator node. The state of the
 * internal state machine identified by the value of the \p currentState
 * public member.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class NodeState
{
public:

	/**
	 * An enumerated type for the possible NodeState states.
	 */
	enum State
	{
		PREPARE, ALIVE, READY, ABORTED
	};

	/**
	 * Resets the NodeState with default <tt>(nodeID, numberOfStages)</tt>
	 * values of (0, 1).
	 */
	NodeState()
	{
		reset(0, 1);
	}

	/**
	 * Reset the NodeState as follows:
	 *
	 *  - \p currentState to \p NodeState::PREPARE;
	 *  - \p currentGeneratorStageID to 0;
	 *  - \p taskCound and taskProgress to 0;
	 *  - \p progressPerStage to 1/\p numberOfStages;
	 */
	void reset(I32u nodeID, I16u numberOfStages)
	{
		this->currentState = PREPARE;
		this->nodeID = nodeID;
		this->currentGeneratorStageID = 0;
		this->progressPerStage = 1 / static_cast<Decimal> (numberOfStages);
		this->taskCount = 0;
		this->taskProgress = 0;
	}

	/**
	 * Return the overall progess of the generation process. The progress is
	 * mass is evenly distributed across all generator stages and all concurrent
	 * tasks at each stage.
	 *
	 * @see GeneratorStage
	 */
	Decimal progress() const
	{
		if (taskCount > 0)
		{
			return (currentGeneratorStageID + taskProgress / taskCount) * progressPerStage;
		}
		else
		{
			return currentGeneratorStageID * progressPerStage;
		}
	}

	/**
	 * The current node state.
	 */
	State currentState;
	/*
	 * The ID of the currently processed node.
	 */
	I32u nodeID;
	/*
	 * The ID of the current GeneratorStage.
	 */
	I16u currentGeneratorStageID;
	/**
	 * The amount of total progress completed at each stage.
	 */
	Decimal progressPerStage;
	/**
	 * The total amout of GeneratorTasks associated in this application.
	 */
	Decimal taskCount;
	/**
	 * The progress of the currently running GeneratorTask.
	 */
	Decimal taskProgress;
};


/**
 * A notification issued when the NodeState::State value changes.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class ChangeNodeState: public Notification
{
public:

	/**
	 * Constructor.
	 */
	ChangeNodeState(const NodeState::State newNodeState) :
		newNodeState(newNodeState)
	{
	}

	/**
	 * The new node state.
	 */
	const NodeState::State newNodeState;
};


/**
 * A notification issued when a new GeneratorStage begins.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class StartStage: public Notification
{
public:

	/**
	 * Constructor.
	 */
	StartStage(I16u stageID) :
		stageID(stageID)
	{
	}

	/**
	 * The ID of the GeneratorStage that was started.
	 */
	I16u stageID;
};


/**
 * A notification issued periodically to update the NodeStage::taskProgress and
 * NodeState::taskCount values.
 *
 * Runnable StageTask instances report their progress by subscribing to this
 * notification type. After each update, the thread creates a new Heartbeat
 * notification.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
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


/**
 * A Notification implementation for periodic node heartbeats.
 *
 * Heartbeat notifications are used to report the node state to an orchestrated
 * coordination service when the data generator runs in distributed mode.
 */
class Heartbeat: public Notification
{
public:

	/**
	 * Constructor.
	 */
	Heartbeat(const NodeState& state) :
		state(state)
	{
	}

	/**
	 * The current state of this node.
	 */
	const NodeState state;
};

/** @}*/// add to communication group
} // namespace Myriad

#endif /* NOTIFICATIONS_H_ */
