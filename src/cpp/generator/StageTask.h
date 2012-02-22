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

#ifndef STAGEEXECUTOR_H_
#define STAGETASK_H_

#include "communication/Notifications.h"
#include "io/OutputCollector.h"
#include "record/Record.h"

#include <Poco/Logger.h>
#include <Poco/Path.h>
#include <Poco/Runnable.h>
#include <Poco/RefCountedObject.h>

#include <string>

using namespace Poco;
using namespace std;

namespace Myriad {

/**
 * Abstract base for all stage task implementations. A stage task is an object
 * spanning over a sequence of single record generation calls.
 *
 * TODO: add doc
 *
 *   * setup internal state prior to generation begin
 *   * register notification handlers to other generators specific for the next stage
 *   * register events to other generators specific to the next stage
 */
class AbstractStageTask: public Runnable, public RefCountedObject
{
public:

	AbstractStageTask(const string& name) :
		_taskName(name), _progress(0)
	{
	}

	const string& name() const
	{
		return _taskName;
	}

	/**
	 * Indicates whether the task is runnable. Runnable tasks must
	 * provide a valid implementation of the virtual runnable method.
	 *
	 * @return
	 */
	virtual bool runnable()
	{
		return false;
	}

	/**
	 * A virtual method to be implemented by all tasks which want to be executed
	 * in a separate thread.
	 *
	 * @return
	 */
	virtual void run()
	{
	}


	void reportProgress(UpdateProgress* notification)
	{
		notification->state.taskCount++;
		notification->state.taskProgress += _progress;
	}

protected:

	/**
	 * Name of the task.
	 */
	string _taskName;


	/**
	 * Current progress.
	 */
	Decimal _progress;
};

template<class RecordType> class StageTask: public AbstractStageTask
{
public:

	typedef typename RecordTraits<RecordType>::GeneratorType GeneratorType;
	typedef typename OutputCollector<RecordType>::CollectorType CollectorType;

	StageTask(const string& name, const string& generatorName, const GeneratorConfig& config, bool dryRun = false) :
		AbstractStageTask(name), _out(generatorName, config), _dryRun(dryRun), _logger(Logger::get("task."+name))
	{
		if (!_dryRun)
		{
			_out.open();
		}
	}

	~StageTask()
	{
		if (!_dryRun)
		{
			_out.close();
		}
	}

protected:

	/**
	 * An output stream used for writing the task output data.
	 */
	CollectorType _out;

	/**
	 * A flag indicating whether writing the output is required.
	 */
	bool _dryRun;

	/**
	 * Logger instance.
	 */
	Logger& _logger;
};

} // namespace Myriad

#endif /* STAGETASK_H_ */
