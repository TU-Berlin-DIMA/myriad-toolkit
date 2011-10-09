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

#ifndef STATICSETGENERATOR_H_
#define STATICSETGENERATOR_H_

#include "generator/RecordGenerator.h"

#include <fstream>
#include <string>
#include <vector>
#include <Poco/AutoPtr.h>
#include <Poco/BasicEvent.h>
#include <Poco/PriorityEvent.h>
#include <Poco/Path.h>

using namespace Poco;

namespace Myriad {

template<class RecordType> class StaticSetGenerator: public RecordGenerator
{
public:

	/**
	 * Constructor.
	 *
	 * @param notificationCenter
	 * @return
	 */
	StaticSetGenerator(const string& name, GeneratorConfig& config, NotificationCenter& notificationCenter);

	/**
	 * Initialization method.
	 *
	 * @param appConfig
	 */
	virtual void initialize();

	/**
	 * Stage setup method.
	 *
	 * @param stage
	 * @param pool
	 */
	virtual void prepare(Stage stage, const GeneratorPool& pool);

	/**
	 * Stage finalization method.
	 *
	 * @param stage
	 */
	virtual void cleanup(Stage stage);

	/**
	 * Release method.
	 */
	virtual void release();

	/**
	 * Events
	 */
	PriorityEvent<AutoPtr<RecordType> > newRecord;

protected:

	/**
	 * Virtual destructor.
	 *
	 * @return
	 */
	virtual ~StaticSetGenerator();
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// associated StageTasks
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

/**
 * Iterates over a static set and (optionally) outputs it to the provided
 * OutputStream. This is typically the central task in the lifetime of a
 * StaticSetGenerator.
 */
template<class RecordType> class StaticSetDumpTask: public StageTask<RecordType>
{
public:

	typedef typename RecordTraits<RecordType>::GeneratorType GeneratorType;

	StaticSetDumpTask(StaticSetGenerator<RecordType>& generator, const vector<AutoPtr<RecordType> >& records, bool dryRun = false) :
		StageTask<RecordType> (generator.name() + "::dump_records", dryRun), _generator(generator), _records(records)
	{
		Path outputPath(generator.config().getString("generator." + generator.name() + ".output-file", generator.name() + string(".tbl")));
		outputPath.makeAbsolute(generator.config().getString("application.output-dir"));

		StageTask<RecordType>::initialize(outputPath);
	}

	bool runnable()
	{
		return true;
	}

	void run();

protected:

	StaticSetGenerator<RecordType>& _generator;

	const vector<AutoPtr<RecordType> >& _records;
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// template method definitions
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class RecordType> StaticSetGenerator<RecordType>::StaticSetGenerator(const string& name, GeneratorConfig& config, NotificationCenter& notificationCenter) :
	RecordGenerator(name, config, notificationCenter)
{
}

template<class RecordType> StaticSetGenerator<RecordType>::~StaticSetGenerator()
{
}


/**
 * Basic initialization logic for all static set generators.
 */
template<class RecordType> void StaticSetGenerator<RecordType>::initialize()
{
	if (_logger.debug())
	{
		_logger.debug(format("Initializing static set generator `%s`", name()));
	}
}

/**
 * Releases resources allocated by the static set generator.
 */
template<class RecordType> void StaticSetGenerator<RecordType>::release()
{
	if (_logger.debug())
	{
		_logger.debug(format("Releasing static set generator `%s`", name()));
	}

	delete this;
}

/**
 * Execute stage preparation logic common for all StaticSetGenerators.
 *
 * @param stage
 * @param pool
 */
template<class RecordType> void StaticSetGenerator<RecordType>::prepare(Stage stage, const GeneratorPool& pool)
{
	if (_logger.debug())
	{
		_logger.debug(format("Preparing static set generator `%s` for stage `%s`", name(), stage.name()));
	}
}

/**
 * Execute stage cleanup logic common for all StaticSetGenerators. This includes
 * clearing all registered tasks and event handlers.
 *
 * @param stage
 */
template<class RecordType> void StaticSetGenerator<RecordType>::cleanup(Stage stage)
{
	if (_logger.debug())
	{
		_logger.debug(format("Cleaning up static set generator `%s` after stage `%s`", name(), stage.name()));
	}

	// clear all registered tasks
	clearTasks();
	// clear all registered event handlers
	newRecord.clear();
}

template<class RecordType> void StaticSetDumpTask<RecordType>::run()
{
	Logger& logger = StageTask<RecordType>::_logger;
	GeneratorOutputCollector& out = StageTask<RecordType>::_out;

	if (logger.debug())
	{
		logger.debug(format("Running stage task `%s`", StageTask<RecordType>::name()));
	}

	ID current = 0;
	I32u progressCounter = 0;

	for (typename vector<AutoPtr<RecordType> >::const_iterator it = _records.begin(); it != _records.end(); ++it)
	{
		AutoPtr<RecordType> recordPtr = *it;
		_generator.newRecord.notify(this, recordPtr);

		if (!StageTask<RecordType>::_dryRun)
		{
			out.emit(*recordPtr);
		}

		++current;

		if(++progressCounter >= 100)
		{
			progressCounter = 0;
			StageTask<RecordType>::_progress = current / static_cast<Decimal>(_records.size());
		}
	}

	StageTask<RecordType>::_progress = 1.0;

	if (logger.debug())
	{
		logger.debug(format("Finishing stage task `%s`", StageTask<RecordType>::name()));
	}
}

} // namespace Myriad

#endif /* STATICSETGENERATOR_H_ */
