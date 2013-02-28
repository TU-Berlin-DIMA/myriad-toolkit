/*
 * Copyright 2010-2013 DIMA Research Group, TU Berlin
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

#ifndef RANDOMSEQUENCEGENERATOR_H_
#define RANDOMSEQUENCEGENERATOR_H_

#include "generator/InvalidRecordException.h"
#include "generator/AbstractSequenceGenerator.h"
#include "runtime/setter/SetterChain.h"

#include <Poco/AutoPtr.h>
#include <Poco/BasicEvent.h>
#include <Poco/PriorityEvent.h>
#include <Poco/PriorityDelegate.h>
#include <string>
#include <vector>

using namespace Poco;

namespace Myriad {
/**
 * @addtogroup generator
 * @{*/

// forward declarations
template<class RecordType>
class RandomSequenceInspector;

/**
 * A common template class for all record generators that produce pseudo-random
 * record sequences.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class RecordType>
class RandomSequenceGenerator: public AbstractSequenceGenerator
{
public:

    /**
     * The RecordMeta type associated with the given \p RecordType.
     */
    typedef typename RecordTraits<RecordType>::MetaType RecordMetaType;
    /**
     * The RecordFactory type associated with the given \p RecordType.
     */
    typedef typename RecordTraits<RecordType>::FactoryType RecordFactoryType;
    /**
     * The SetterChain type associated with the given \p RecordType.
     */
    typedef typename RecordTraits<RecordType>::SetterChainType RecordSetterChainType;

    /**
     * Constructor.
     */
    RandomSequenceGenerator(const string& name, GeneratorConfig& config, NotificationCenter& notificationCenter) :
        AbstractSequenceGenerator(name, config, notificationCenter), _random(name)
    {
    }

    /**
     * Basic initialization logic for all random set generators.
     *
     * This method initializes a generator specific RandomStream substream as a
     * copy of the current GeneratorConfig::masterPRNG() and then advances the
     * position of the master PRNG.
     *
     * @see AbstractSequenceGenerator::initialize()
     */
    virtual void initialize()
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

        string s1 = format("substream cardinality is %Lu", _config.cardinality(name()));
        string s2 = format("generating segment [%Lu, %Lu) of %Lu", _config.genIDBegin(name()), _config.genIDEnd(name()), _config.cardinality(name()));
        _logger.information(format("Configured sizing for generator `%s` # %05hu of %05hu: %s, %s", name(), _config.nodeID(), _config.numberOfChunks(), s1, s2));
        _logger.information(format("Substream seed for the generator `%s` set to [%s]", name(), _random.seed().toString()));
    }

    /**
     * Releases resources allocated by the static set generator.
     */
    virtual void release()
    {
        if (_logger.debug())
        {
	        _logger.debug(format("Releasing random set generator `%s`", name()));
        }

        delete this;
    }

    /**
     * Execute common preparation logic common to all generator stages.
     *
     * @param stage The current GgeneratorStage.
     * @param pool A reference to the applicaiton GeneratorPool.
     */
    virtual void prepare(Stage stage, const GeneratorPool& pool)
    {
        if (_logger.debug())
        {
	        _logger.debug(format("Preparing random set generator `%s` for stage `%s`", name(), stage.name()));
        }
    }

    /**
     * Execute stage cleanup logic common to all generator stages.
     *
     * @param stage The current GeneratorStage.
     */
    virtual void cleanup(Stage stage)
    {
        if (_logger.debug())
        {
	        _logger.debug(format("Cleaning up random set generator `%s` after stage `%s`", name(), stage.name()));
        }

        // clear all registered tasks
        clearTasks();
        // reset the random generator
        _random.resetSubstream();
    }

    /**
     * Returns a reference to the underlying RandomStream. If the configure()
     * method of the RandomSequenceGenerator has been called, this method is
     * guaranteed to return a RandomStream instance that points to the first
     * position of the associated PRNG substream.
     *
     * @return A reference to the local RandomStream instance.
     */
    const RandomStream& random() const
    {
        return _random;
    }

    /**
     * Returns the cardinality of the record sequence produced by this generator.
     *
     * @return The sequence cardinality for this generator.
     */
    const I64u cardinality() const
    {
        return _config.cardinality(name());
    }

    /**
     * Constructs and returns a new record factory for the given \p RecordType.
     */
    virtual const RecordFactoryType recordFactory()
    {
        return RecordFactoryType(RecordMetaType(_config.enumSets()));
    }

    /**
     * Creates and returns an random access inspector for this record sequence.
     */
    RandomSequenceInspector<RecordType> inspector()
    {
        return RandomSequenceInspector<RecordType> (*this);
    }

    /**
     * Creates a new setter chain which consumes records from the provided
     * RandomStream reference.
     *
     * @param opMode OperationMode for the SetterChain (sequential or random).
     * @param random A reference to the RandomStream consumed by the chain.
     */
    virtual RecordSetterChainType setterChain(BaseSetterChain::OperationMode opMode, RandomStream& random) = 0;

protected:

    /**
     * Private destructor (no static allocation).
     */
    virtual ~RandomSequenceGenerator()
    {
    }

    /**
     * RandomStream generator for this generator.
     */
    RandomStream _random;
};


/**
 * A random-access wraper for a random record sequence.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class RecordType>
class RandomSequenceInspector
{
public:

    /**
     * The RandomSequenceGenerator type associated with the given \p RecordType.
     */
    typedef typename RecordTraits<RecordType>::GeneratorType SequenceGeneratorType;
    /**
     * The SetterChain type associated with the given \p RecordType.
     */
    typedef typename RecordTraits<RecordType>::SetterChainType RecordSetterChainType;
    /**
     * The RecordFactory type associated with the given \p RecordType.
     */
    typedef typename RecordTraits<RecordType>::FactoryType RecordFactoryType;

    /**
     * Constructor.
     *
     * @param generator A reference to the enclosing RecordSetterChainType.
     */
    RandomSequenceInspector(RandomSequenceGenerator<RecordType>& generator) :
        _generator(generator),
        _recordFactory(_generator.recordFactory()),
        _random(_generator.random()),
        _setterChain(_generator.setterChain(BaseSetterChain::RANDOM, _random)),
        _logger(Logger::get("inspector."+generator.name()))
    {
    }

    /**
     * Copy constructor.
     */
    RandomSequenceInspector(const RandomSequenceInspector& other) :
        _generator(other._generator),
        _recordFactory(_generator.recordFactory()),
        _random(_generator.random()),
        _setterChain(_generator.setterChain(BaseSetterChain::RANDOM, _random)),
        _logger(other._logger)
    {
    }

    /**
     * Instantiates and returns the \p RecordType at the given \p genID
     * position in the random sequence.
     *
     * @throws InvalidRecordException If the record at sequence position
     *         \p genID is not valid, i.e. if it is not defined.
     */
    const AutoPtr<RecordType> at(const I64u genID) const
    {
        AutoPtr<RecordType> recordPtr = _recordFactory();
        recordPtr->genID(genID);

        _setterChain(recordPtr);

        return recordPtr;
    }

    /**
     * Filter the range of valid \p genID values from an equality predicate.
     */
    Interval<I64u> filter(const EqualityPredicate<RecordType>& predicate)
    {
        return _setterChain.filter(predicate);
    }

    /**
     * Get the cardinality of the accessed record sequence.
     */
    const I64u cardinality() const
    {
        return _generator.cardinality();
    }

private:

    /**
     * A reference to the parent generator (needed by the copy constructor).
     */
    RandomSequenceGenerator<RecordType>& _generator;

    /**
     * A record factory instance.
     */
    RecordFactoryType _recordFactory;

    /**
     * A copy of the generator's random stream.
     */
    RandomStream _random;

    /**
     * The SetterChain for this record type.
     */
    RecordSetterChainType _setterChain;

    /**
     * Logger instance.
     */
    Logger& _logger;
};


/**
 * Partitioned iterator task.
 *
 * Iterates over a subsequence of records allocated to a particular partition.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class RecordType>
class PartitionedSequenceIteratorTask: public StageTask<RecordType>
{
public:

    /**
     * The RandomSequenceGenerator type associated with the given \p RecordType.
     */
    typedef typename RecordTraits<RecordType>::GeneratorType SequenceGeneratorType;
    /**
     * The SetterChain type associated with the given \p RecordType.
     */
    typedef typename RecordTraits<RecordType>::SetterChainType RecordSetterChainType;
    /**
     * The RecordFactory type associated with the given \p RecordType.
     */
    typedef typename RecordTraits<RecordType>::FactoryType RecordFactoryType;


    /**
     * Constructor.
     */
    PartitionedSequenceIteratorTask(RandomSequenceGenerator<RecordType>& generator, const GeneratorConfig& config, bool dryRun = false) :
        StageTask<RecordType> (generator.name() + "::generate_records", generator.name(), config, dryRun),
        _generator(generator),
        _recordFactory(_generator.recordFactory()),
        _random(generator.random()),
        _logger(Logger::get("task.random.default."+generator.name()))
    {
    }

    /**
     * Indicates that this StageTask is runnable.
     *
     * @return Always \p true.
     */
    bool runnable()
    {
        return true;
    }

    /**
     * The run() of this runnable task.
     *
     * This method simply iterates over the allocated genID subsequence and
     * instantiates and writes out the \p RecordType instance at each position.
     */
    void run()
    {
        if (_logger.debug())
        {
	        _logger.debug(format("Running stage task `%s`", this->name()));
        }

        I32u progressCounter = 0;

        ID first = _generator.config().genIDBegin(_generator.name());
        ID current = _generator.config().genIDBegin(_generator.name());
        ID last = _generator.config().genIDEnd(_generator.name());

        _random.atChunk(current);

        RecordSetterChainType setterChain = _generator.setterChain(BaseSetterChain::SEQUENTIAL, _random);
        while (current < last)
        {
	        AutoPtr<RecordType> recordPtr = _recordFactory();
	        recordPtr->genID(current);

	        try
	        {
		        setterChain(recordPtr);
	        }
	        catch(const InvalidRecordException& e)
	        {
		        current = e.nextValidGenID();
		        _random.atChunk(current);

	            if(progressCounter + e.invalidRangeSize() >= 1000)
	            {
	                progressCounter = 0;
	                this->_progress = (current - first) / static_cast<Decimal>(last - first);
	            }

		        continue;
	        }

	        if (!this->_dryRun)
	        {
		        this->_out->collect(*recordPtr);
	        }

	        ++current;
	        _random.nextChunk();

	        if(++progressCounter >= 1000)
	        {
		        progressCounter = 0;
		        this->_progress = (current - first) / static_cast<Decimal>(last - first);
	        }
        }

        // flush the output collector contents
        this->_out->flush();

        this->_progress = 1.0;

        if (_logger.debug())
        {
	        _logger.debug(format("Finishing stage task `%s`", this->name()));
        }
    }

protected:

    /**
     * A reference to the generator creating the stage.
     */
    RandomSequenceGenerator<RecordType>& _generator;

    /**
     * A record factory instance.
     */
    RecordFactoryType _recordFactory;

    /**
     * A copy of the generator's random stream.
     */
    RandomStream _random;

    /**
     * Logger instance.
     */
    Logger& _logger;
};

/** @}*/// add to generator group
} // namespace Myriad

#endif /* RANDOMSEQUENCEGENERATOR_H_ */
