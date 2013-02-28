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

#ifndef FRONTEND_H_
#define FRONTEND_H_

#include <Poco/Util/Application.h>
#include <vector>
#include <string>

using namespace std;
using namespace Poco;
using namespace Poco::Util;

namespace Myriad {
/**
 * @addtogroup core
 * @{*/

/**
 * A command line interface (CLI) frontend for the generated data generators.
 *
 * The Frontend object is instantiated and executed in the main function of
 * all Myriad based data generators.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class Frontend: public Application
{
public:

    /**
     * Default constructor.
     */
    Frontend();

    /**
     * Handles the occurence of a '--help' CLI parameter.
     *
     * @param name The name of the processed parameter (always `help`).
     * @param value The value of the processed parameter (always empty).
     */
    void handleHelp(const std::string& name, const std::string& value);

    /**
     * Handles the occurence of a '--version' CLI parameter.
     *
     * @param name The name of the processed parameter (always `version`).
     * @param value The value of the processed parameter (always empty).
     */
    void handleVersion(const std::string& name, const std::string& value);

    /**
     * Handles the occurence of a '-x<value>' CLI parameter.
     *
     * Marks the generator stage provided by value to be executed. Per default,
     * if no '-x' parameters are supplied to the Frontend, all generator
     * stages will be executed.
     *
     * @param name The name of the processed parameter (always
     *             `execute-stages`).
     * @param value The name of the stage to be executed.
     */
    void handleExecuteStage(const std::string& name, const std::string& value);

    /**
     * Handles the '-o<output-type>' CLI parameter.
     *
     * If the parameter is 'file', sets the 'application.output-type' parameter
     * to 'file.'.
     *
     * If the parameter is 'socket(port)', sets the 'application.output-base'
     * parameter to 'socket' and the 'application.output-port' parameter to the
     * given port.
     *
     * @param name The name of the processed parameter (always `output-type`).
     * @param value The processed value (`file` or `socket(port)`).
     */
    void handleOutputType(const std::string& name, const std::string& value);

protected:

    /**
     * Initializes the application environment.
     *
     * Initializes the two subsystems comprising the application - the
     * GeneratorSubsystem and the CommunicationSubsystem. If an exception is
     * caught during the initialization lifecycle of the Application, a
     * new  <tt>ChangeStatus(NodeState::ABORTED)</tt> notification is issued.
     */
    void initialize(Application& self);

    /**
     * Registers the set of opptions supported by the CLI Frontend.
     *
     * @param options A reference to the OptionSet used by the application.
     */
    void defineOptions(OptionSet& options);

    /**
     * Runs the data generator.
     *
     * Starts the registered GeneratorSubsystem and CommunicationSubsystem
     * subsystem instances.
     */
    int main(const std::vector<std::string>& args);

private:

    vector<bool> _executeStages;

    bool _metaInfoRequested;

    bool _excCaught;

    Logger& _ui;
};

/** @}*/// add to core group
} // Myriad namespace

#endif /* FRONTEND_H_ */
