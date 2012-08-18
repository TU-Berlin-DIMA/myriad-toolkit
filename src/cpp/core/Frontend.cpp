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

#include "core/constants.h"
#include "core/Frontend.h"
#include "communication/CommunicationSubsystem.h"
#include "communication/Notifications.h"
#include "generator/GeneratorSubsystem.h"
#include "generator/AbstractSequenceGenerator.h"

#include <string>
#include <iostream>
#include <Poco/Exception.h>
#include <Poco/Format.h>
#include <Poco/Util/HelpFormatter.h>

using std::string;
using Poco::format;
using Poco::Exception;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;

namespace Myriad {

Frontend::Frontend() :
	_executeStages(AbstractSequenceGenerator::STAGES.size(), false),
	_metaInfoRequested(false),
	_excCaught(false),
	_ui(Logger::get("ui"))
{
}

void Frontend::initialize(Application& self)
{
	if (!_metaInfoRequested)
	{
		_ui.information(format("%s (Version %s)", Constant::APP_NAME, Constant::APP_VERSION));
	}

	if (!config().hasProperty("application.config-dir"))
	{
		config().setString("application.config-dir", "../config");
	}

	Path configDir(config().getString("application.config-dir"));
	if (!configDir.isAbsolute())
	{
		configDir.makeAbsolute(config().getString("application.dir"));
		config().setString("application.config-dir", configDir.toString());
	}

	loadConfiguration(config().getString("application.config-dir") + "/" + config().getString("application.baseName") + ".properties");

	// if the custom-stages flag is not set, enable all stages
	if (!config().getBool("application.custom-stages", false))
	{
		for (unsigned int stage = 0; stage < _executeStages.size(); stage++)
		{
			_executeStages[stage] = true;
		}
	}

	NotificationCenter& notificationCenter = NotificationCenter::defaultCenter();

	if (!_metaInfoRequested)
	{
		addSubsystem(new GeneratorSubsystem(notificationCenter, _executeStages));
		addSubsystem(new CommunicationSubsystem(notificationCenter));
	}

	try
	{
		Application::initialize(self);
	}
	catch (const Exception& exc)
	{
		notificationCenter.postNotification(new ChangeStatus(NodeState::ABORTED));
		_ui.fatal(format("Exception caught while initializing application: %s", exc.displayText()));
		_excCaught = true;
	}
	catch (const exception& exc)
	{
		notificationCenter.postNotification(new ChangeStatus(NodeState::ABORTED));
		_ui.fatal(format("Exception caught while initializing application: %s", string(exc.what())));
		_excCaught = true;
	}
	catch (...)
	{
		notificationCenter.postNotification(new ChangeStatus(NodeState::ABORTED));
		_ui.fatal("Exception caught in while generating data");
		_excCaught = true;
	}
}

void Frontend::defineOptions(OptionSet& options)
{
	Application::defineOptions(options);

	options.addOption(Option("help", "h", "display help information")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<Frontend> (this, &Frontend::handleHelp)));

	options.addOption(Option("version", "v", "display version information")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<Frontend> (this, &Frontend::handleVersion)));

	options.addOption(Option("scaling-factor", "s", "scaling factor (s=1 generates 1GB)")
			.required(false)
			.repeatable(false)
			.argument("<double>")
			.binding("application.scaling-factor"));

	options.addOption(Option("dataset-id", "m", "ID of the generated Myriad dataset")
			.required(false)
			.repeatable(false)
			.argument("<string>")
			.binding("application.job-id"));

	options.addOption(Option("node-id", "i", "node ID (i.e. partition number) of the current generating node")
			.required(false)
			.repeatable(false)
			.argument("<int>")
			.binding("application.node-id"));

	options.addOption(Option("node-count", "N", "total node count (i.e. total number of partitions)")
			.required(false)
			.repeatable(false)
			.argument("<int>")
			.binding("application.node-count"));

	options.addOption(Option("config-dir", "c", "default directory where all config files are stored")
			.required(false)
			.repeatable(false)
			.argument("<path>")
			.binding("application.config-dir"));

	options.addOption(Option("node-config", "n", "the default XML node config files (residing in the config directory)")
			.required(false)
			.repeatable(false)
			.argument("<filename>")
			.binding("application.node-config"));

	options.addOption(Option("output-base", "o", "base path for writing the output")
			.required(false)
			.repeatable(false)
			.argument("<path>")
			.binding("application.output-base"));

	options.addOption(Option("coordinator-host", "H", "coordinator server hostname")
			.required(false)
			.repeatable(false)
			.argument("<hostname>")
			.binding("application.coordinator.host"));

	options.addOption(Option("coordinator-port", "P", "coordinator server port")
			.required(false)
			.repeatable(false)
			.argument("<port>")
			.binding("application.coordinator.port"));

	options.addOption(Option("execute-stages", "x", "specify a specifc stage to be executed")
			.required(false)
			.repeatable(true)
			.argument("<stagename>")
			.callback(OptionCallback<Frontend> (this, &Frontend::handleExecuteStage)));
}

void Frontend::handleExecuteStage(const string& name, const string& stage)
{
	for (AbstractSequenceGenerator::StageList::const_iterator it = AbstractSequenceGenerator::STAGES.begin(); it != AbstractSequenceGenerator::STAGES.end(); ++it)
	{
		if (stage == it->name())
		{
			_executeStages[it->id()] = true;
			config().setBool("application.custom-stages", true);
			break;
		}
	}
}

void Frontend::handleHelp(const string& name, const string& value)
{
	_metaInfoRequested = true;

	HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage("OPTIONS");
	helpFormatter.setHeader("\nAvailable Options:");

	helpFormatter.format(std::cout);

	stopOptionsProcessing();
}

void Frontend::handleVersion(const string& name, const string& value)
{
	_metaInfoRequested = true;

	std::cout << Constant::APP_VERSION << std::endl;

	stopOptionsProcessing();
}

int Frontend::main(const std::vector<std::string>& args)
{
	if (_metaInfoRequested)
	{
		return Application::EXIT_OK;
	}

	if (_excCaught)
	{
		return Application::EXIT_SOFTWARE;
	}

	NotificationCenter& notificationCenter = NotificationCenter::defaultCenter();

	try
	{
		CommunicationSubsystem& communicationSubsystem = getSubsystem<CommunicationSubsystem> ();
		communicationSubsystem.start();

		GeneratorSubsystem& generatorSubsystem = getSubsystem<GeneratorSubsystem> ();
		generatorSubsystem.start();
	}
	catch (const Exception& exc)
	{
		notificationCenter.postNotification(new ChangeStatus(NodeState::ABORTED));
		_ui.fatal(format("Exception caught while generating data: %s", exc.displayText()));
		return Application::EXIT_SOFTWARE;
	}
	catch (const exception& exc)
	{
		notificationCenter.postNotification(new ChangeStatus(NodeState::ABORTED));
		_ui.fatal(format("Exception caught while generating data: %s", string(exc.what())));
		return Application::EXIT_SOFTWARE;
	}
	catch (...)
	{
		notificationCenter.postNotification(new ChangeStatus(NodeState::ABORTED));
		_ui.fatal("Exception caught in while generating data");
		return Application::EXIT_SOFTWARE;
	}

	return Application::EXIT_OK;
}

} // Myriad namespace
