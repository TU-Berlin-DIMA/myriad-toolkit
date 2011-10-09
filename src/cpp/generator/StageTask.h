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
 * @author: Alexander Alexandrov <alexander.s.alexandrov@campus.tu-berlin.de>
 */

#ifndef STAGEEXECUTOR_H_
#define STAGETASK_H_

#include "communication/Notifications.h"
#include "io/GeneratorOutputStream.h"
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

	StageTask(const string& name, bool dryRun = false) :
		AbstractStageTask(name), _dryRun(dryRun), _logger(Logger::get("task."+name)), _initialized(false)
	{
	}

	~StageTask()
	{
		if (_initialized && !_dryRun)
		{
			_logger.debug(format("Closing output file %s", _outputPath));
			_out.close();
		}
	}

protected:

	void initialize(Path& outputPath, std::ios::openmode mode = std::ios::trunc | std::ios::binary)
	{
		if (!_initialized)
		{
			_outputPath = outputPath.toString();

			if (!_dryRun)
			{
				_logger.debug(format("Opening output file %s", _outputPath));
				_out.open(_outputPath, mode);
			}
		}

		_initialized = true;
	}

	/**
	 * A flag indicating whether writing the output is required.
	 */
	bool _dryRun;

	/**
	 * An output stream used for writing the task output data.
	 */
	GeneratorOutputCollector _out;

	/**
	 * Logger instance.
	 */
	Logger& _logger;

private:

	std::string _outputPath;

	bool _initialized;
};

} // namespace Myriad

#endif /* STAGETASK_H_ */
