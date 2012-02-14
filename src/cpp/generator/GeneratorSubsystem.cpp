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

#include "communication/Notifications.h"
#include "generator/GeneratorSubsystem.h"

#include <functional>
#include <Poco/ErrorHandler.h>
#include <Poco/Format.h>
#include <Poco/Stopwatch.h>

using namespace std;
using namespace Poco;

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// helper function objects
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

/**
 * Function object handling RecordGenerator thread creation for ready generators.
 */
class ThreadExecutor: public unary_function<void, RecordGenerator*>
{
public:
	ThreadExecutor(GeneratorSubsystem& caller) :
		caller(caller)
	{
	}

	void operator()(RecordGenerator* generator)
	{
		RecordGenerator::TaskPtrList::const_iterator it;
		for (it = generator->executors().begin(); it != generator->executors().end(); ++it)
		{
			AbstractStageTask* task = (*it);
			if (task->runnable())
			{
				caller._threadPool.start(*task);
			}
		}
	}

private:
	GeneratorSubsystem& caller;
};

/**
 * An error handler to be registered with all generator threads spawned in the
 * main loop.
 */
class GeneratorErrorHandler: public ErrorHandler
{
public:
	GeneratorErrorHandler(GeneratorSubsystem& caller) :
		_caller(caller), _invoked(false)
	{
	}

	void exception(const Exception& exc)
	{
		_invoked = true;
		_caller._logger.error(format("Exception caught in generator thread: %s", exc.displayText()));
	}

	void exception(const std::exception& exc)
	{
		_invoked = true;
		_caller._logger.error(format("Exception caught in generator thread: %s", exc.what()));
	}

	void exception()
	{
		_invoked = true;
		_caller._logger.error("Exception caught in generator thread");
	}

	void checkSanity()
	{
		if (!_invoked)
		{
			return; // handler was not invoked, so we're ok
		}

		if (_caller._logger.debug())
		{
			_caller._logger.debug("Cleaning up resources...");
		}

		// TODO: put basic resource cleaning of the generator subsystem here...

		throw Poco::RuntimeException("Exception caught in generator thread");
	}

private:
	GeneratorSubsystem& _caller;
	bool _invoked;
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// method implementations
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

void GeneratorSubsystem::initialize(Application& app)
{
	if (_initialized)
	{
		return;
	}

	try
	{
		// initialize the GeneratorConfig instance
		_config.initialize(app.config());

		_ui.information(format("Scaling factor is %s", toString(_config.scalingFactor())));
		_ui.information(format("Job output will be written in %s", _config.getString("application.job-dir")));

		// register all generators in the generator pool
		registerGenerators();
		list<RecordGenerator*>& generators = _generatorPool.getAll();

		// initialize all registered generators
		for(list<RecordGenerator*>::iterator it = generators.begin(); it != generators.end(); ++it)
		{
			(*it)->initialize();
		}

		_ui.information(format("Starting generation for node %hu from %hu", _config.chunkID(), _config.numberOfChunks()));
	}
	catch(const Exception& exc)
	{
		_logger.error(format("Exception caught in generator subsystem: %s", exc.displayText()));
		exc.rethrow();
	}
	catch(const std::exception& exc)
	{
		_logger.error(format("Exception caught in generator subsystem: %s", string(exc.what())));
		throw exc;
	}
	catch(...)
	{
		_logger.error("Unknown exception caught in generator subsystem");
		throw;
	}

	_initialized = true;
}

void GeneratorSubsystem::uninitialize()
{
	if (!_initialized)
	{
		return;
	}

	// TODO: clear the _config instance

	_initialized = false;
}

void GeneratorSubsystem::prepareStage(RecordGenerator::Stage stage)
{
	list<RecordGenerator*>& generators = _generatorPool.getAll();

	if (_logger.debug())
	{
		_logger.debug(format("Preparing stage `%s`", stage.name()));
	}

	unsigned short runnableCount = 0;
	for (RecordGenerator::PtrList::iterator it = generators.begin(); it != generators.end(); ++it)
	{
		(*it)->prepare(stage, _generatorPool);
		runnableCount += (*it)->runnableTasksCount();
	}

	// increase the size of the thread pool if needed
	if (_threadPool.capacity() < runnableCount)
	{
		_threadPool.addCapacity(runnableCount - _threadPool.capacity());
	}
}

void GeneratorSubsystem::cleanupStage(RecordGenerator::Stage stage)
{
	list<RecordGenerator*>& generators = _generatorPool.getAll();
	for (RecordGenerator::PtrList::iterator it = generators.begin(); it != generators.end(); ++it)
	{
		(*it)->cleanup(stage);
	}
}

void GeneratorSubsystem::start()
{
	Stopwatch totalTimer, stageTimer;

	double sf = _config.getDouble("application.scaling-factor");
	_logger.information(format("Generating tables with scaling factor %.03f", sf));

	GeneratorErrorHandler handler(*this);
	ErrorHandler* oldHandler = ErrorHandler::set(&handler);

	try
	{
		// mark node as alive
		_notificationCenter.postNotification(new ChangeStatus(NodeState::ALIVE));

		// start total timer
		totalTimer.start();

		list<RecordGenerator*>& generators = _generatorPool.getAll();
		for (RecordGenerator::StageList::const_iterator it = RecordGenerator::STAGES.begin(); it != RecordGenerator::STAGES.end(); ++it)
		{
			if (!_executeStages[it->id()])
			{
				_logger.information(format("Skipping stage `%s`", it->name()));
				continue;
			}

			_logger.information(format("Entering stage `%s`", it->name()));

			// start stage timer
			stageTimer.restart();

			// announce next stage
			_notificationCenter.postNotification(new StartStage(it->id()));

			// prepare generators for the next stage
			prepareStage(*it);

			// spawn separate threads for each RUNNABLE task
			ThreadExecutor execute(*this);
			for_each(generators.begin(), generators.end(), execute);

			// wait for all threads to finish
			_threadPool.joinAll();

			// cleanup generators upon stage execution
			cleanupStage(*it);

			// make sure that all threads exited correctly
			handler.checkSanity();

			// stop the stage timer
			stageTimer.stop();

			_logger.information(format("Stage `%s` completed in %d seconds, moving to the next stage", it->name(), stageTimer.elapsedSeconds()));
		}

		// stop the total timer
		totalTimer.stop();

		// reset error handler
		ErrorHandler::set(oldHandler);

		// mark node as ready
		_notificationCenter.postNotification(new ChangeStatus(NodeState::READY));
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

} // namespace Myriad
