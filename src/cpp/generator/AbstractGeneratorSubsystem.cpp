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

#include "communication/Notifications.h"
#include "generator/AbstractGeneratorSubsystem.h"

#include <functional>
#include <Poco/ErrorHandler.h>
#include <Poco/Format.h>
#include <Poco/Stopwatch.h>

using namespace std;
using namespace Poco;

namespace Myriad {
/**
 * @addtogroup generator
 * @{*/

////////////////////////////////////////////////////////////////////////////////
/// @name Helper Function Objects
////////////////////////////////////////////////////////////////////////////////
//@{

/**
 * Function object handling AbstractSequenceGenerator thread creation for ready generators.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class ThreadExecutor: public unary_function<void, AbstractSequenceGenerator*>
{
public:

    /**
     * Constructor.
     */
    ThreadExecutor(AbstractGeneratorSubsystem& caller) :
        _caller(caller)
    {
    }

    /**
     * Functor operator.
     *
     * Iterates through all runnable AbstractStageTask objects currently
     * registered with the \p generator and executes them using one of the
     * threads of a fix-sized thread pool.
     */
    void operator()(AbstractSequenceGenerator* generator)
    {
        AbstractSequenceGenerator::TaskPtrList::const_iterator it;
        for (it = generator->stageTasks().begin(); it != generator->stageTasks().end(); ++it)
        {
	        AbstractStageTask* task = (*it);
	        if (task->runnable())
	        {
		        _caller._threadPool.start(*task, task->name());
	        }
        }
    }

private:

    /**
     * A reference to the enclosing AbstractGeneratorSubsystem.
     */
    AbstractGeneratorSubsystem& _caller;
};

/**
 * An error handler for the runnable AbstractStageTask instances.
 *
 * This error handler implements a shutdown logic for the GeneratorSubsystem
 * and is registered with all AbstractStageTask threads spawned in the main
 * loop of the GeneratorSubsystem. All exception handlers merely mark the
 * handler as invoked, which triggers an execution of the checkSanity() method
 * invoked by the GeneratorSubsystem at the end of each GeneratorStage cycle.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class GeneratorErrorHandler: public ErrorHandler
{
public:

    /**
     * Constructor.
     */
    GeneratorErrorHandler(AbstractGeneratorSubsystem& caller) :
        _invoked(false),
        _caller(caller)
    {
    }

    /**
     * Exception handler for platform specific exceptions.
     */
    void exception(const Exception& exc)
    {
        _invoked = true;
        _caller._logger.error(format("Exception caught in generator thread: %s", exc.displayText()));
    }

    /**
     * Exception handler for STL exceptions.
     */
    void exception(const std::exception& exc)
    {
        _invoked = true;
        _caller._logger.error(format("Exception caught in generator thread: %s", exc.what()));
    }

    /**
     * Exception handler for all STL exceptions.
     */
    void exception()
    {
        _invoked = true;
        _caller._logger.error("Exception caught in generator thread");
    }

    /**
     * A sanity check procedure invoked by the GeneratorSubsystem at the end of
     * each GeneratorStage cycle.
     *
     * Can be used to implement coordinated cleanup logic to be executed when
     * the generation process fails at the subsystem level.
     *
     * @return True, if the handler was not invoked.
     */
    bool checkSanity()
    {
        if (!_invoked)
        {
	        return true; // handler was not invoked, so we're ok
        }

        if (_caller._logger.debug())
        {
	        _caller._logger.debug("Cleaning up resources...");
        }

        // basic resource cleaning of the generator subsystem can be done here

        return false;
    }

private:

    /**
     * A flag indicating that the error handler has been invoked.
     */
    bool _invoked;

    /**
     * A reference to the enclosing AbstractGeneratorSubsystem.
     */
    AbstractGeneratorSubsystem& _caller;
};

//@}

////////////////////////////////////////////////////////////////////////////////
/// @name Method Implementations
////////////////////////////////////////////////////////////////////////////////
//@{

void AbstractGeneratorSubsystem::initialize(Application& app)
{
    if (_initialized)
    {
        return;
    }

    try
    {
        // initialize the GeneratorConfig instance
        _config.initialize(app.config());

        // scaling factor should always be greater or equal than 1.0
        if (_config.scalingFactor() < 1.0)
        {
	        throw LogicException("Scaling factor can not be less than 1.0");
        }

        _ui.information(format("Scaling factor is %s", toString(_config.scalingFactor())));
        _ui.information(format("Job output will be written in %s", _config.getString("application.job-dir")));

        // register all generators in the generator pool
        registerGenerators();
        list<AbstractSequenceGenerator*>& generators = _generatorPool.getAll();

        // initialize all registered generators
        for(list<AbstractSequenceGenerator*>::iterator it = generators.begin(); it != generators.end(); ++it)
        {
	        (*it)->initialize();
        }

        _ui.information(format("Starting generation for node %hu from %hu", _config.nodeID(), _config.numberOfChunks()));
    }
    catch(const Exception& e)
    {
        _logger.error(format("Exception caught in generator subsystem: %s", e.displayText()));
        e.rethrow();
    }
    catch(const std::exception& e)
    {
        _logger.error(format("Exception caught in generator subsystem: %s", string(e.what())));
        throw e;
    }
    catch(...)
    {
        _logger.error("Unknown exception caught in generator subsystem");
        throw;
    }

    _initialized = true;
}

void AbstractGeneratorSubsystem::uninitialize()
{
    if (!_initialized)
    {
        return;
    }

    // TODO: clear the _config instance

    _initialized = false;
}

unsigned short AbstractGeneratorSubsystem::prepareStage(AbstractSequenceGenerator::Stage stage)
{
    list<AbstractSequenceGenerator*>& generators = _generatorPool.getAll();

    if (_logger.debug())
    {
        _logger.debug(format("Preparing stage `%s`", stage.name()));
    }

    unsigned short runnableCount = 0;
    for (AbstractSequenceGenerator::PtrList::iterator it = generators.begin(); it != generators.end(); ++it)
    {
        (*it)->prepare(stage, _generatorPool);
        runnableCount += (*it)->runnableTasksCount();
    }

    // increase the size of the thread pool if needed
    if (_threadPool.capacity() < runnableCount)
    {
        _threadPool.addCapacity(runnableCount - _threadPool.capacity());
    }

    return runnableCount;
}

void AbstractGeneratorSubsystem::cleanupStage(AbstractSequenceGenerator::Stage stage)
{
    list<AbstractSequenceGenerator*>& generators = _generatorPool.getAll();
    for (AbstractSequenceGenerator::PtrList::iterator it = generators.begin(); it != generators.end(); ++it)
    {
        (*it)->cleanup(stage);
    }
}

void AbstractGeneratorSubsystem::start()
{
    Stopwatch totalTimer, stageTimer;

    double sf = _config.getDouble("application.scaling-factor");
    _logger.information(format("Generating tables with scaling factor %.03f", sf));

    GeneratorErrorHandler handler(*this);
    ErrorHandler* oldHandler = ErrorHandler::set(&handler);

    try
    {
        // mark node as alive
        _notificationCenter.postNotification(new ChangeNodeState(NodeState::ALIVE));

        // start total timer
        totalTimer.start();

        list<AbstractSequenceGenerator*>& generators = _generatorPool.getAll();
        for (AbstractSequenceGenerator::StageList::const_iterator it = AbstractSequenceGenerator::STAGES.begin(); it != AbstractSequenceGenerator::STAGES.end(); ++it)
        {
	        if (!_executeStages[it->id()])
	        {
		        _logger.debug(format("Skipping stage `%s`", it->name()));
		        continue;
	        }

	        // start stage timer
	        stageTimer.restart();

	        // announce next stage
	        _notificationCenter.postNotification(new StartStage(it->id()));

	        // prepare generators for the next stage
	        unsigned short runnableCount = prepareStage(*it);

	        if (runnableCount == 0)
	        {
		        continue;
	        }

	        _logger.information(format("Entering stage `%s`", it->name()));

	        // spawn separate threads for each RUNNABLE task
	        ThreadExecutor execute(*this);
	        for_each(generators.begin(), generators.end(), execute);

	        // wait for all threads to finish
	        _threadPool.joinAll();

	        // cleanup generators upon stage execution
	        cleanupStage(*it);

	        // make sure that all threads exited correctly
	        if (!handler.checkSanity())
	        {
	            throw Poco::RuntimeException(format("Error at stage `%s`. Aborting the generation process.", it->name()));
	        }

	        // stop the stage timer
	        stageTimer.stop();

	        _logger.information(format("Stage `%s` completed in %d seconds", it->name(), stageTimer.elapsedSeconds()));
        }

        // stop the total timer
        totalTimer.stop();

        // reset error handler
        ErrorHandler::set(oldHandler);

        // mark node as ready
        _notificationCenter.postNotification(new ChangeNodeState(NodeState::READY));
    }
    catch(const Exception& exc)
    {
        _logger.error(format("Exception caught in generator subsystem: %s", exc.displayText()));
        ErrorHandler::set(oldHandler);
        exc.rethrow();
    }
    catch(const std::exception& exc)
    {
        _logger.error(format("Exception caught in generator subsystem: %s", string(exc.what())));
        ErrorHandler::set(oldHandler);
        throw exc;
    }
    catch(...)
    {
        _logger.error("Unknown exception caught in generator subsystem");
        ErrorHandler::set(oldHandler);
        throw;
    }

    _logger.information(format("Generation process completed in %d seconds", totalTimer.elapsedSeconds()));
}

//@}

/** @}*/// add to generator group
} // namespace Myriad
