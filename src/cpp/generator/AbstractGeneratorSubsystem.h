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

#ifndef ABSTRACTGENERATORSUBSYSTEM_H_
#define ABSTRACTGENERATORSUBSYSTEM_H_

#include "generator/AbstractSequenceGenerator.h"
#include "generator/GeneratorPool.h"

#include <string>
#include <Poco/NotificationCenter.h>
#include "Poco/ThreadPool.h"
#include <Poco/Util/Application.h>
#include <Poco/Util/Subsystem.h>

using namespace std;
using namespace Poco;
using namespace Poco::Util;

namespace Myriad {
/**
 * @addtogroup generator
 * @{*/

/**
 * An implementation an abstract subsystem that controls the data generation
 * program flow.
 *
 * The AbstractGeneratorSubsystem is implemented as a concrete
 * GeneratorSubsystem derived class by all data generator applications.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class AbstractGeneratorSubsystem: public Subsystem
{
    friend class GeneratorErrorHandler;
    friend class ThreadExecutor;

public:

    /**
     * Constructor.
     */
    AbstractGeneratorSubsystem(NotificationCenter& notificationCenter, const vector<bool>& executeStages) :
        _notificationCenter(notificationCenter),
        _executeStages(executeStages),
        _config(_generatorPool),
        _threadPool("StageTaskPool", 4, 32),
        _initialized(false),
        _logger(Logger::get("generator.driver")),
        _ui(Logger::get("ui"))
    {
    }

    /**
     * Destructor.
     */
    virtual ~AbstractGeneratorSubsystem()
    {
    }

    /**
     * Runs the GeneratorSubsystem. This method contains the main loop for the
     * data generator logic at the application level.
     *
     * The loop iterates through all GeneratorStage instances.
     * At each GeneratorStage, the AbstractSequenceGenerator::prepare() method
     * is used to poll all registered AbstractSequenceGenerator in order to
     * obtaion a set of runnable StageTask instances (currently only
     * PartitionedSequenceIterator tasks are supported).
     * ThreadExecutor instances from a shared pool are then used to concurrently
     * execute the collected runnable StageTasks.
     * At the end of each stage the AbstractSequenceGenerator::cleanup()
     * methods of all registered generators are invoked.
     *
     * Two ChangeStatus notifications with node values NodeState::ALIVE
     * and NodeState::Ready are issued correspondingly before and after the
     * main GeneratorStage iteration loop. In addition, a StartStage
     * notification is issued before entering the lifecycle of each stage.
     */
    void start();

protected:

    /**
     * Returns a constant subsystem name "Generator Subsystem".
     */
    const char* name() const
    {
        return "Generator Subsystem";
    }

    /**
     * Common initialization logic.
     */
    void initialize(Application&);

    /**
     * Common uninitialization logic.
     */
    void uninitialize();

    /**
     * Virtual method for registering generators.
     */
    virtual void registerGenerators() = 0;

    /**
     * Adds a single generator of the given \p T type to the generator pool.
     *
     * @param name The name of the added generator.
     */
    template<class T> void registerGenerator(const string& name)
    {
        _generatorPool.set(new T(name, _config, _notificationCenter));
    }

private:
    /**
     * Prepares all generators for the next stage.
     *
     * @param stage
     * @return the number of runnable tasks for this stage
     */
    unsigned short prepareStage(AbstractSequenceGenerator::Stage stage);

    /**
     * Cleans up all generators upon execution of the current stage.
     *
     * @param stage
     */
    void cleanupStage(AbstractSequenceGenerator::Stage stage);

    /**
     * A reference to the application wide notification center.
     */
    NotificationCenter& _notificationCenter;

    /**
     * A bitmap of stages that need to be executed.
     */
    const vector<bool>& _executeStages;

    /**
     * A pool for the registered generators.
     */
    GeneratorPool _generatorPool;

    /**
     * An application wide generator config instance.
     */
    GeneratorConfig _config;

    /**
     * A pool for the runnable generator task threads.
     */
    ThreadPool _threadPool;

    /**
     * A flag indicating that the initialize() method has already been invoked.
     */
    bool _initialized;

protected:

    /**
     * Generator subsystem logger.
     */
    Logger& _logger;

    /**
     * User interface (normally stdout) logger.
     */
    Logger& _ui;
};

/** @}*/// add to generator group
} // namespace Myriad

#endif /* ABSTRACTGENERATORSUBSYSTEM_H_ */
