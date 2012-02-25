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

#ifndef RECORDGENERATOR_H_
#define RECORDGENERATOR_H_

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

class GeneratorPool;

class GeneratorStage
{
public:

	GeneratorStage(const String& name) :
		_id(NEXT_STAGE_ID++),
		_name(name)
	{
	}

	const I32u& id() const
	{
		return _id;
	}

	const String& name() const
	{
		return _name;
	}

private:

	static I32u NEXT_STAGE_ID;

	const I32u _id;
	const string _name;
};

class RecordGenerator
{
public:

	typedef GeneratorStage Stage;
	typedef list<RecordGenerator*> PtrList;
	typedef list<Stage> StageList;
	typedef list<AbstractStageTask*> TaskPtrList;
	typedef AutoReleasePool<AbstractStageTask> ExecutorPool;

	static const StageList STAGES;

	RecordGenerator(const string& name, GeneratorConfig& config, NotificationCenter& notificationCenter) :
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
	 * generation phase. Its functions will typically include
	 *
	 *   * register stage executors
	 *
	 * @param stage
	 * @param pool
	 */
	virtual void prepare(Stage stage, const GeneratorPool& pool) = 0;

	/**
	 * The cleanup method is called once after the execution of each
	 * generation phase. Its functions will typically include
	 *
	 *   * clear all delegates from the generator's own event objects
	 *   * uninitialize all registered stage executors
	 *   * cleanup other stage related dependencies
	 *
	 * @param stage
	 */
	virtual void cleanup(Stage stage) = 0;

	/**
	 * Release method (invoked on delete)
	 */
	virtual void release() = 0;

	/**
	 * The name of the Generator.
	 */
	const string& name()
	{
		return _name;
	}

	/**
	 * Global configuration reference getter.
	 */
	GeneratorConfig& config()
	{
		return _config;
	}

	/**
	 * The name of the Generator.
	 */
	const TaskPtrList& executors()
	{
		return _stageTasks;
	}

	/**
	 * Returns the current number of runnable executors present in the taskPool.
	 */
	const unsigned short runnableTasksCount()
	{
		return _runnableTasksCount;
	}

protected:
	/**
	 * Virtual destructor for abstract base class.
	 */
	virtual ~RecordGenerator()
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
	ExecutorPool _taskPool;

	/**
	 * A counter for the number of runnable executors currently present in the executor pool.
	 */
	unsigned short _runnableTasksCount;

	/**
	 * Logger instance.
	 */
	Logger& _logger;

private:

	RecordGenerator(const RecordGenerator&);
	RecordGenerator& operator =(const RecordGenerator&);
};

} // namespace Myriad

#endif /* RECORDGENERATOR_H_ */
