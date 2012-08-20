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

#ifndef ABSTRACTSEQUENCEGENERATOR_H_
#define ABSTRACTSEQUENCEGENERATOR_H_

#include "communication/Notifications.h"
#include "config/GeneratorConfig.h"
#include "generator/StageTask.h"
#include "math/random/RandomStream.h"

#include <list>
#include <iostream>
#include <string>
#include <Poco/AutoReleasePool.h>
#include <Poco/AutoPtr.h>
#include <Poco/Delegate.h>
#include <Poco/Event.h>
#include <Poco/Format.h>
#include <Poco/Logger.h>
#include <Poco/NotificationCenter.h>
#include <Poco/Notification.h>
#include <Poco/Observer.h>
#include <Poco/Util/AbstractConfiguration.h>

using namespace std;
using namespace Poco;
using namespace Poco::Util;

namespace Myriad {
/**
 * @addtogroup generator
 * @{*/

// forward declarations
class GeneratorPool;

/**
 * A class to representing a generic generator stage.
 *
 * A stage is represented by a unique ID used by the system and a name. Each
 * data generator application registers a set of relevant GeneratorStages by
 * assigning a value to the static AbstractSequenceGenerator::STAGES vector.
 *
 */
class GeneratorStage
{
public:

	/**
	 * Constructs a stage with the next available \p ID and the given \p name.
	 */
	GeneratorStage(const String& name) :
		_id(NEXT_STAGE_ID++),
		_name(name)
	{
	}

	/**
	 * Stage ID getter.
	 */
	const I32u& id() const
	{
		return _id;
	}

	/**
	 * Stage name getter.
	 */
	const String& name() const
	{
		return _name;
	}

private:

	/**
	 * A static constant holding the next available STAGE_ID.
	 *
	 * Initially always 0.
	 */
	static I32u NEXT_STAGE_ID;

	/**
	 * The stage ID.
	 */
	const I32u _id;

	/**
	 * The stage name.
	 */
	const string _name;
};

/**
 * An implementation of an abstract PRDG sequence generator.
 *
 * This is used as a common base for all record specific sequence generators.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class AbstractSequenceGenerator
{
public:

	typedef GeneratorStage Stage; //<! Alias of GeneratorStage
	typedef list<AbstractSequenceGenerator*> PtrList; //<! A list of AbstractSequenceGenerator pointers
	typedef list<Stage> StageList; //<!- A list of GeneratorStage objects
	typedef list<AbstractStageTask*> TaskPtrList; //<!-- A list of AbstractStageTask pointers
	typedef AutoReleasePool<AbstractStageTask> AbstractStageTaskPool; //<!-- An autorelease pool for AbstractStageTask objects

	/**
	 * A static list of GeneratorStage objects relevant to the data generator
	 * application.
	 *
	 * The value of this list is application dependent, and is typically
	 * supplied by in the \c generator/base/BaseGeneratorSubsystem.cpp by the
	 * application C++ sources.
	 */
	static const StageList STAGES;

	/**
	 * Constructor.
	 */
	AbstractSequenceGenerator(const string& name, GeneratorConfig& config, NotificationCenter& notificationCenter) :
		_name(name),
		_config(config),
		_notificationCenter(notificationCenter),
		_runnableTasksCount(0),
		_logger(Logger::get("generator." + name))
	{
	}

	/**
	 * Initialization method.
	 *
	 * @param appConfig
	 */
	virtual void initialize() = 0;

	/**
	 * The prepare method is called once prior to the execution of each
	 * GeneratorStage. Its functions will typically include
	 *
	 *  - Reset the subsequence state
	 *  - Add AbstractStageTask instances to be executed in the next \p stage
	 *
	 * @param stage the current GeneratorStage
	 * @param pool the pool of generators registered with the application
	 */
	virtual void prepare(Stage stage, const GeneratorPool& pool) = 0;

	/**
	 * The cleanup method is called once after the execution of each
	 * generation phase. Its functions will typically include
	 *
	 *  - Uninitialize all registered AbstractStageTask instances
	 *  - Cleanup other stage related dependencies
	 *
	 * @param stage the current GeneratorStage
	 */
	virtual void cleanup(Stage stage) = 0;

	/**
	 * Release method (invoked on delete).
	 */
	virtual void release() = 0; //TODO: this can probably be implemented directly in the object destructors

	/**
	 * The name of the sequence generator.
	 */
	const string& name() const
	{
		return _name;
	}

	/**
	 * A reference to the global application configuration object.
	 */
	GeneratorConfig& config()
	{
		return _config;
	}

	/**
	 * A list of stage tasks registered for the current GeneratorStage.
	 */
	const TaskPtrList& stageTasks()
	{
		return _stageTasks;
	}

	/**
	 * Returns the current number of runnable registered GeneratorStage tasks.
	 */
	const unsigned short runnableTasksCount()
	{
		return _runnableTasksCount;
	}

protected:

	/**
	 * Virtual destructor for abstract base class.
	 */
	virtual ~AbstractSequenceGenerator()
	{
	}

	/**
	 * Registers a task with the current execution stage. This method
	 * is typically called by subclasses in the concrete prepareStage()
	 * implementations.
	 *
	 * @param task
	 */
	void registerTask(AbstractStageTask* task)
	{
		_stageTasks.push_back(task);
		_taskPool.add(task);

		if (task->runnable())
		{
			_runnableTasksCount++;
		}

		_notificationCenter.addObserver(Observer<AbstractStageTask, UpdateProgress> (*task, &AbstractStageTask::reportProgress));
	}

	/**
	 * Clears all registered tasks. This method must be called in the
	 * cleanup() implementation of all subclasses.
	 */
	void clearTasks()
	{
		for (TaskPtrList::iterator it = _stageTasks.begin(); it != _stageTasks.end(); ++it)
		{
			_notificationCenter.removeObserver(Observer<AbstractStageTask, UpdateProgress> (**it, &AbstractStageTask::reportProgress));
		}

		_stageTasks.clear();
		_taskPool.release();
		_runnableTasksCount = 0;
	}

	/**
	 * The name of the generator.
	 */
	const string _name;

	/**
	 * A reference to the global generator configuration.
	 */
	GeneratorConfig& _config;

	/**
	 * A reference to the generator subsystem notification center.
	 */
	NotificationCenter& _notificationCenter;

	/**
	 * A list of GeneratorStageExecutor executors refreshed at the beginning of each stage.
	 */
	TaskPtrList _stageTasks;

	/**
	 * An autorelease pool for the created executors. This list is cleared at
	 * the end of each stage (which means that the life of an executors spans
	 * only one stage).
	 */
	AbstractStageTaskPool _taskPool;

	/**
	 * A counter for the number of runnable executors currently present in the executor pool.
	 */
	unsigned short _runnableTasksCount;

	/**
	 * Logger instance.
	 */
	Logger& _logger;

private:

	/**
	 * Private copy constructor (abstract base class).
	 */
	AbstractSequenceGenerator(const AbstractSequenceGenerator&);

	/**
	 * Private assignment operator (abstract base class).
	 */
	AbstractSequenceGenerator& operator =(const AbstractSequenceGenerator&);
};

/** @}*/// add to generator group
} // namespace Myriad

#endif /* ABSTRACTSEQUENCEGENERATOR_H_ */
