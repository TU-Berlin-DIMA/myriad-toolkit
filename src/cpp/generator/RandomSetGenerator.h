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

#ifndef RANDOMSETGENERATOR_H_
#define RANDOMSETGENERATOR_H_

#include "generator/RecordGenerator.h"
#include "hydrator/HydratorChain.h"

#include <Poco/AutoPtr.h>
#include <Poco/BasicEvent.h>
#include <Poco/PriorityEvent.h>
#include <Poco/PriorityDelegate.h>
#include <string>
#include <vector>

using namespace Poco;

namespace Myriad {

// forward declarations
template<class RecordType> class RandomSetInspector;
template<class RecordType> class RandomSetDefaultGeneratingTask;

template<class RecordType> class RandomSetGenerator: public RecordGenerator
{
public:

	typedef typename RecordTraits<RecordType>::HydratorChainType HydratorChainType;

	/**
	 * Constructor.
	 */
	RandomSetGenerator(const string& name, GeneratorConfig& config, NotificationCenter& notificationCenter);

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
	 * Returns a reference to the underlying RandomStream. If the configure()
	 * method of the generator has been called, this method is guaranteed to
	 * return a configured RandomStream instance.
	 *
	 * @return a reference to the local random stream instance
	 */
	RandomStream& random()
	{
		return _random;
	}

	/**
	 * Creates a new hydrator chain which works on the provided RandomStream
	 * reference.
	 */
	virtual HydratorChainType hydratorChain(BaseHydratorChain::OperationMode opMode, RandomStream& random) = 0;

	/**
	 * Creates a new inspector.
	 */
	RandomSetInspector<RecordType> inspector()
	{
		return RandomSetInspector<RecordType> (*this);
	}

	/**
	 * Events
	 */
	PriorityEvent<AutoPtr<RecordType> > newRecord;

protected:

	virtual ~RandomSetGenerator();

	/**
	 * RandomStream generator for this generator.
	 */
	RandomStream _random;
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// inspector inspector
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class RecordType> class RandomSetInspector
{
public:

	typedef typename RecordTraits<RecordType>::GeneratorType GeneratorType;
	typedef typename RecordTraits<RecordType>::HydratorChainType HydratorChainType;

	RandomSetInspector(RandomSetGenerator<RecordType>& generator) :
		_random(generator.random()),
		_recordPtr(new RecordType()),
		_hydrate(generator.hydratorChain(BaseHydratorChain::RANDOM, _random)),
		_logger(Logger::get("inspector."+generator.name()))
	{
	}

	const AutoPtr<RecordType> at(const I64u genID);

private:

	/**
	 * A copy of the generator's random stream.
	 */
	RandomStream _random;

	/**
	 * A record instance used for inspection.
	 */
	AutoPtr<RecordType> _recordPtr;

	/**
	 * Hydrator for the generated records.
	 */
	HydratorChainType _hydrate;

	/**
	 * Logger instance.
	 */
	Logger& _logger;
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// associated stage tasks
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

/**
 * Default random set task implementation.
 */
template<class RecordType> class RandomSetDefaultGeneratingTask: public StageTask<RecordType>
{
public:

	typedef typename RecordTraits<RecordType>::GeneratorType GeneratorType;
	typedef typename RecordTraits<RecordType>::HydratorChainType HydratorChainType;

	RandomSetDefaultGeneratingTask(RandomSetGenerator<RecordType>& generator, const GeneratorConfig& config, bool dryRun = false) :
		StageTask<RecordType> (generator.name() + "::generate_records", generator.name(), config, dryRun), _generator(generator), _random(generator.random()), _logger(Logger::get("task.random.default."+generator.name()))
	{
	}

	bool runnable()
	{
		return true;
	}

	void run();

protected:

	/**
	 * A reference to the generator creating the stage.
	 */
	RandomSetGenerator<RecordType>& _generator;

	/**
	 * A copy of the generator's random stream.
	 */
	RandomStream _random;

	/**
	 * Logger instance.
	 */
	Logger& _logger;
};

/**
 * Default random set task implementation.
 */
template<class RecordType, class ProbabilityType> class RandomSetTimeSpanGeneratingTask: public StageTask<RecordType>
{
public:

	typedef typename RecordTraits<RecordType>::GeneratorType GeneratorType;
	typedef typename RecordTraits<RecordType>::HydratorChainType HydratorChainType;

	typedef void (RecordType::*DateTimeSetter)(const DateTime&);

	RandomSetTimeSpanGeneratingTask(RandomSetGenerator<RecordType>& generator, const GeneratorConfig& config, DateTimeSetter dateTimeSetter, bool dryRun = false) :
		StageTask<RecordType> (generator.name() + "::generate_records", generator.name(), config, dryRun),
		_generator(generator),
		_random(generator.random()),
		_dateTimeSetter(dateTimeSetter),
		_probability(config.func<ProbabilityType>(config.getString("generator." + generator.name() + ".timespan.pattern.probability"))),
		_logger(Logger::get("task.random.timespan."+generator.name()))
	{
	}

	bool runnable()
	{
		return true;
	}

	void run();

protected:

	/**
	 * A reference to the generator creating the stage.
	 */
	RandomSetGenerator<RecordType>& _generator;

	/**
	 * A copy of the generator's random stream.
	 */
	RandomStream _random;

	/**
	 * The setter for the DateTime field of the generated objects.
	 */
	DateTimeSetter _dateTimeSetter;

	/**
	 * Probability function describing the frequency of occurrence inside a single period.
	 */
	ProbabilityType& _probability;

	/**
	 * Logger instance.
	 */
	Logger& _logger;
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// method definitions
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class RecordType> RandomSetGenerator<RecordType>::RandomSetGenerator(const string& name, GeneratorConfig& config, NotificationCenter& notificationCenter) :
	RecordGenerator(name, config, notificationCenter), _random(name)
{
}

template<class RecordType> RandomSetGenerator<RecordType>::~RandomSetGenerator()
{
}

/**
 * Basic initialization logic for all random set generators.
 */
template<class ConfigType> void RandomSetGenerator<ConfigType>::initialize()
{
	if (_logger.debug())
	{
		_logger.debug(format("Initializing random set generator `%s`", name()));
	}

	_random = _config.masterPRNG();

	// advance the position of the master PRNG to the next substream
	// this ensures that no two generators will share the same substream in
	// the initialization of the generator subsystem
	_config.masterPRNG().nextSubstream();

	_logger.information(format("Configured sizing for generator `%s` # %05hu of %05hu: generating segment [%Lu, %Lu) of %Lu", name(), _config.chunkID(), _config.numberOfChunks(), _config.genIDBegin(name()), _config.genIDEnd(name()), _config.cardinality(name())));
	_logger.information(format("Substream seed for the generator `%s` set to [%s]", name(), _random.seed().toString()));
}

/**
 * Releases resources allocated by the static set generator.
 */
template<class RecordType> void RandomSetGenerator<RecordType>::release()
{
	if (_logger.debug())
	{
		_logger.debug(format("Releasing random set generator `%s`", name()));
	}

	delete this;
}

/**
 * Execute stage preparation logic common for all RandomSetGenerators.
 *
 * @param stage
 * @param pool
 */
template<class RecordType> void RandomSetGenerator<RecordType>::prepare(Stage stage, const GeneratorPool& pool)
{
	if (_logger.debug())
	{
		_logger.debug(format("Preparing random set generator `%s` for stage `%s`", name(), stage.name()));
	}
}

/**
 * Execute stage cleanup logic common for all RandomSetGenerators. This includes
 * clearing all registered tasks and event handlers.
 *
 * @param stage
 */
template<class RecordType> void RandomSetGenerator<RecordType>::cleanup(Stage stage)
{
	if (_logger.debug())
	{
		_logger.debug(format("Cleaning up random set generator `%s` after stage `%s`", name(), stage.name()));
	}

	// clear all registered tasks
	clearTasks();
	// reset the random generator
	_random.resetSubstream();
	// clear all listeners attached to the newRecord event
	newRecord.clear();
}

template<class RecordType> inline AutoPtr<RecordType> RandomSetGenerator<RecordType>::operator()() const
{
	AutoPtr<RecordType> recordPtr(new RecordType());

	return recordPtr;
}

template<class RecordType> inline const AutoPtr<RecordType> RandomSetInspector<RecordType>::at(const I64u genID)
{
	_random.atChunk(genID);

	_recordPtr->genID(genID);
	_hydrate(_recordPtr);

	return _recordPtr;
}

template<class RecordType> void RandomSetDefaultGeneratingTask<RecordType>::run()
{
	Logger& logger = StageTask<RecordType>::_logger;
	OutputCollector& out = StageTask<RecordType>::_out;

	if (logger.debug())
	{
		logger.debug(format("Running stage task `%s`", StageTask<RecordType>::name()));
	}

	I32u progressCounter = 0;

	ID first = _generator.config().genIDBegin(_generator.name());
	ID current = _generator.config().genIDBegin(_generator.name());
	ID last = _generator.config().genIDEnd(_generator.name());

	_random.atChunk(current);

	HydratorChainType hydrate = _generator.hydratorChain(BaseHydratorChain::SEQUENTIAL, _random);
	while (current < last)
	{
		AutoPtr<RecordType> recordPtr = _generator();
		recordPtr->genID(current);

		hydrate(recordPtr);

		_generator.newRecord.notify(&_generator, recordPtr);

		if (!StageTask<RecordType>::_dryRun)
		{
			out.collect(*recordPtr);
		}

		++current;
		_random.nextChunk();

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

template<class RecordType, class ProbabilityType> void RandomSetTimeSpanGeneratingTask<RecordType, ProbabilityType>::run()
{
	Logger& logger = StageTask<RecordType>::_logger;
	OutputCollector& out = StageTask<RecordType>::_out;

	if (logger.debug())
	{
		logger.debug(format("Running stage task `%s`", StageTask<RecordType>::name()));
	}

	/*
	 * read input parameters for this iterator task
	 */

	ID first = _generator.config().genIDBegin(_generator.name());
	ID current = _generator.config().genIDBegin(_generator.name());
	ID last = _generator.config().genIDEnd(_generator.name());

	// start and end date
	int minDateTimeTzd, maxDateTimeTzd;
	DateTime minDateTime, maxDateTime;
	DateTimeParser::parse("%Y-%m-%d %H:%M:%S", _generator.config().getString("generator." + _generator.name() + ".timespan.min-date"), minDateTime, minDateTimeTzd);
	DateTimeParser::parse("%Y-%m-%d %H:%M:%S", _generator.config().getString("generator." + _generator.name() + ".timespan.max-date"), maxDateTime, maxDateTimeTzd);
	I32u period = _generator.config().getInt("generator." + _generator.name() + ".timespan.pattern.period");

	double delta = 0.0001;
	double totalProbability = 1.0 - 2*delta;

	/*
	 * compute derived parameters
	 */

	// how many seconds do we have in our timespan
	Timespan timespan = maxDateTime - minDateTime;
	// how many periods fit into the specified timespan
	double numberOfPeriods = timespan.totalSeconds() / (double) period;
	// get the number of generated records per period
	double recordsPerPeriod = last / numberOfPeriods;
	// get the conversion factor between a record in a period an a point on the x-axis of the period probability
	Interval<Decimal> xRange(_probability.invcdf(delta), _probability.invcdf(1.0 - delta));
	double xAxisRatio = xRange.length() / period;

	/*
	 * initialize current period related parameters
	 */

	// find the period of the current record
	I32u currentPeriod = first / recordsPerPeriod;
	// find the ID of the first and last records for the current period
	I32u currentPeriodFirst = recordsPerPeriod * currentPeriod;
	I32u currentPeriodLast  = recordsPerPeriod * (currentPeriod + 1);
	double yAxisRatio = totalProbability / (currentPeriodLast - currentPeriodFirst);

	/*
	 * generate the assigned record substream
	 */

	I32u progressCounter = 0;

	_random.atChunk(current);

	HydratorChainType hydrate = _generator.hydratorChain(BaseHydratorChain::SEQUENTIAL, _random);
	while (current < last)
	{
		if (current >= currentPeriodLast)
		{
			// entering new period, update relevant entries
			currentPeriod++;
			// find the ID of the first and last records for the current period
			currentPeriodFirst = recordsPerPeriod * currentPeriod;
			currentPeriodLast  = recordsPerPeriod * (currentPeriod + 1);
			yAxisRatio = totalProbability / (currentPeriodLast - currentPeriodFirst);
		}

		// get the DateTime associated with the current record
		I32u currentPeriodXPosition = (_probability.invcdf((current - currentPeriodFirst) * yAxisRatio + delta) - xRange.min()) / xAxisRatio;
		DateTime currentDateTime = minDateTime + Timespan(period * currentPeriod + currentPeriodXPosition, 0);

		AutoPtr<RecordType> recordPtr = _generator();
		recordPtr->genID(current);

		(recordPtr->*_dateTimeSetter)(currentDateTime);

		// hydrate random values
		hydrate(recordPtr);

		_generator.newRecord.notify(&_generator, recordPtr);

		if (!StageTask<RecordType>::_dryRun)
		{
			out.collect(*recordPtr);
		}

		++current;
		_random.nextChunk();

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

#endif /* RANDOMSETGENERATOR_H_ */
