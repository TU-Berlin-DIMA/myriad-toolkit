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

#ifndef DETERMINISTICSETGENERATOR_H_
#define DETERMINISTICSETGENERATOR_H_

#include "generator/RecordGenerator.h"

#include <string>
#include <vector>
#include <Poco/AutoPtr.h>
#include <Poco/BasicEvent.h>
#include <Poco/Path.h>

using namespace Poco;

namespace Myriad {

template<class RecordType> class DeterministicSetGenerator: public RecordGenerator
{
public:

	/**
	 * Constructor.
	 *
	 * @param notificationCenter
	 * @return
	 */
	DeterministicSetGenerator(const string& name, GeneratorConfig& config, NotificationCenter& notificationCenter);

	/**
	 * Initialization method.
	 *
	 * @param appConfig
	 */
	virtual void initialize();

	/**
	 * Release method.
	 */
	virtual void release();

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
	 * Object generating function.
	 */
	AutoPtr<RecordType> operator()() const;

	/**
	 * Events
	 */
	BasicEvent<AutoPtr<RecordType> > newRecord;

protected:

	/**
	 * Virtual destructor.
	 *
	 * @return
	 */
	virtual ~DeterministicSetGenerator();
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// associated StageTasks
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

/**
 * Iterates over a static set and (optionally) outputs it to the provided
 * OutputStream. This is typically the central task in the lifetime of a
 * StaticSetGenerator.
 */
template<class RecordType> class DeterministicSetDumpTask: public StageTask<RecordType>
{
public:

	typedef typename RecordTraits<RecordType>::GeneratorType GeneratorType;

	DeterministicSetDumpTask(DeterministicSetGenerator<RecordType>& generator, const GeneratorConfig& config, bool dryRun = false) :
		StageTask<RecordType> (generator.name() + "::dump_records", generator.name(), config, dryRun), _generator(generator)
	{
	}

	bool runnable()
	{
		return true;
	}

	void run();

protected:

	DeterministicSetGenerator<RecordType>& _generator;
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// template method definitions
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class RecordType> DeterministicSetGenerator<RecordType>::DeterministicSetGenerator(const string& name, GeneratorConfig& config, NotificationCenter& notificationCenter) :
	RecordGenerator(name, config, notificationCenter)
{
}

template<class RecordType> DeterministicSetGenerator<RecordType>::~DeterministicSetGenerator()
{
}

/**
 * Basic initialization logic for all random set generators.
 */
template<class RecordType> void DeterministicSetGenerator<RecordType>::initialize()
{
	if (_logger.debug())
	{
		_logger.debug(format("Initializing deterministic set generator `%s`", name()));
	}
}

/**
 * Releases resources allocated by the static set generator.
 */
template<class RecordType> void DeterministicSetGenerator<RecordType>::release()
{
	if (_logger.debug())
	{
		_logger.debug(format("Releasing deterministic set generator `%s`", name()));
	}

	delete this;
}

/**
 * Execute stage preparation logic common for all DeterministicSetGenerators.
 *
 * @param stage
 * @param pool
 */
template<class RecordType> void DeterministicSetGenerator<RecordType>::prepare(Stage stage, const GeneratorPool& pool)
{
	if (_logger.debug())
	{
		_logger.debug(format("Preparing deterministic set generator `%s` for stage `%s`", name(), stage.name()));
	}
}

/**
 * Execute stage cleanup logic common for all DeterministicSetGenerators. This includes
 * clearing all registered tasks and event handlers.
 *
 * @param stage
 */
template<class RecordType> void DeterministicSetGenerator<RecordType>::cleanup(Stage stage)
{
	if (_logger.debug())
	{
		_logger.debug(format("Cleaning up deterministic set generator `%s` after stage `%s`", name(), stage.name()));
	}

	// clear all registered tasks
	clearTasks();
	// clear all registered event handlers
	newRecord.clear();
}

template<class RecordType> AutoPtr<RecordType> DeterministicSetGenerator<RecordType>::operator()() const
{
	AutoPtr<RecordType> recordPtr(new RecordType());

	return recordPtr;
}

template<class RecordType> void DeterministicSetDumpTask<RecordType>::run()
{
	Logger& logger = StageTask<RecordType>::_logger;
	GeneratorOutputCollector& out = StageTask<RecordType>::_out;

	if (logger.debug())
	{
		logger.debug(format("Running stage task `%s`", StageTask<RecordType>::name()));
	}

	I32u progressCounter = 0;

	ID first = _generator.config().genIDBegin(_generator.name());
	ID current = _generator.config().genIDBegin(_generator.name());
	ID last = _generator.config().genIDEnd(_generator.name());

	while (current < last)
	{
		AutoPtr<RecordType> recordPtr = _generator();
		recordPtr->genID(current);

		_generator.newRecord.notify(this, recordPtr);

		if (!StageTask<RecordType>::_dryRun)
		{
			out.collect(*recordPtr);
		}

		++current;

		if(++progressCounter >= 100)
		{
			progressCounter = 0;
			StageTask<RecordType>::_progress = (current - first) / static_cast<Decimal>(last - first);
		}
	}

	StageTask<RecordType>::_progress = 1.0;

	if (logger.debug())
	{
		logger.debug(format("Finishing stage task `%s`", StageTask<RecordType>::name()));
	}
}

} // namespace Myriad

#endif /* DETERMINISTICSETGENERATOR_H_ */
