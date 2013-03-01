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

#ifndef STAGETASK_H_
#define STAGETASK_H_

#include "core/types.h"
#include "communication/Notifications.h"
#include "io/OutputCollector.h"
#include "record/AbstractRecord.h"

#include <Poco/Logger.h>
#include <Poco/Path.h>
#include <Poco/Runnable.h>
#include <Poco/RefCountedObject.h>

#include <string>

using namespace Poco;
using namespace std;

namespace Myriad {
/**
 * @addtogroup generator
 * @{*/

/**
 * Abstract base for all stage tasks.
 *
 * An stage task is an object that spans a sequence of generation calls for a
 * particular \p RecordType. Stage tasks can be runnable (i.e. they can run
 * in a separate thread) or not (in which case most probably they will be
 * attached to a runnable stage task for another \p RecordType, for instance in
 * order to implement nested record generation patterns.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class AbstractStageTask: public Runnable, public RefCountedObject
{
public:

    /**
     * Constructor.
     */
    AbstractStageTask(const string& name) :
        _taskName(name), _progress(0)
    {
    }

    /**
     * The name of this AbstractStageTask.
     */
    const string& name() const
    {
        return _taskName;
    }

    /**
     * Indicates whether the task is runnable.
     *
     * Runnable tasks must provide a valid implementation of the run() method.
     */
    virtual bool runnable()
    {
        return false;
    }

    /**
     * A virtual method to be implemented by all runnable tasks.
     */
    virtual void run()
    {
    }

    /**
     * Updates the task progress in the given UpdateProgress \p notification.
     */
    void reportProgress(const AutoPtr<UpdateProgress>& notification)
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

/**
 * A common template class for all record generators that produce pseudo-random
 * record sequences.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class RecordType>
class StageTask: public AbstractStageTask
{
public:

    /**
     * The RandomSequenceGenerator type associated with the given \p RecordType.
     */
    typedef typename RecordTraits<RecordType>::GeneratorType SequenceGeneratorType;
    /**
     * The \p AbstractOutputCollector template specialization for this \p RecordType.
     */
    typedef AbstractOutputCollector<RecordType> AbstractOutputCollectorType;

    /**
     * Constructor.
     *
     * Opens the internal output collector.
     *
     * @param name The name of this StageTask.
     * @param generatorName The name of the enclosing generator.
     * @param config A reference to the global generator configuration.
     * @param dryRun A boolean flag indicating whether the output collector
     *               should be used or not (i.e. whether it is a dry run).
     */
    StageTask(const string& taskName, const string& generatorName, const GeneratorConfig& config, bool dryRun = false) :
        AbstractStageTask(taskName),
        _out(OutputCollector<RecordType>::factory(config.outputType(), config.outputPort(), config.outputPath(generatorName), "task." + taskName + ".collector")),
        _dryRun(dryRun),
        _logger(Logger::get("task." + taskName))
    {
        if (!_dryRun)
        {
	        _out->open();
        }
    }

    /**
     * Destructor.
     *
     * Closes the internal output collector.
     */
    ~StageTask()
    {
        if (!_dryRun)
        {
	        _out->close();
        }
    }

protected:

    /**
     * An output stream used for writing the task output data.
     */
    AutoPtr<AbstractOutputCollectorType> _out;

    /**
     * A flag indicating whether writing the output is required.
     */
    bool _dryRun;

    /**
     * Logger instance.
     */
    Logger& _logger;
};

/** @}*/// add to generator group
} // namespace Myriad

#endif /* STAGETASK_H_ */
