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

#include "config/AbstractGeneratorConfig.h"
#include "core/exceptions.h"
#include "generator/GeneratorPool.h"

#include <Poco/File.h>
#include <Poco/FormattingChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/SimpleFileChannel.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/Util/MapConfiguration.h>

using namespace Poco;
using namespace Poco::Util;
using namespace Poco::XML;

namespace Myriad {

void AbstractGeneratorConfig::initialize(AbstractConfiguration& appConfig)
{
	_logger.information("Loading generator configuration");

	// add the application config as a read-only layer
	this->addWriteable(&appConfig, 0, true);

	// set partitioning and sizing parameters
	if (!hasProperty("application.scaling-factor"))
	{
		setDouble("application.scaling-factor", getDouble("common.defaults.scaling-factor", 1.0));
	}

	setInt("common.partitioning.number-of-chunks", getInt("application.node-count", 1));
	setInt("common.partitioning.chunks-id", getInt("application.node-id", 0));

	// configure directory structure
	setString("application.job-dir", format("%s/%s", getString("application.output-base"), getString("application.job-id")));
	setString("application.output-dir", format("%s/node%03d", getString("application.job-dir"), getInt("common.partitioning.chunks-id")));
	setString("application.log-path", format("%s/log/node%03d.log", getString("application.job-dir"), getInt("common.partitioning.chunks-id")));

	// make sure that the job, log and output paths exist

	// job-dir
	File jobDir(getString("application.job-dir"));
	jobDir.createDirectories();

	// log-path
	File logPath(getString("application.log-path"));
	if (logPath.exists())
	{
		logPath.remove(true);
	}
	File logDir(Path(getString("application.log-path")).parent());
	logDir.createDirectories();

	// configure log channels
	FormattingChannel* logChannel = new FormattingChannel(new PatternFormatter("%t"), new SimpleFileChannel());
	logChannel->setProperty("path", getString("application.log-path"));
	Logger::root().setChannel(logChannel);
	_logger.setChannel(logChannel);

	configurePartitioning();
	configureFunctions();
	configureSets();

	if (hasProperty("common.master.seed"))
	{
		// initialize the master random stream (otherwise use the default master seed)
		_masterPRNG.seed(RandomStream::Seed(getString("common.master.seed")));
	}
}

void AbstractGeneratorConfig::bindStringSet(const AutoPtr<Document>& doc, const string& id, vector<string>& set)
{
	Element* containerEl = doc->getElementById(id, "key");
	if (containerEl == NULL || containerEl->tagName() != "string_set")
	{
		throw ConfigException(format("No <string-set> element found for `%s`", id));
	}

	AutoPtr<NodeList> strings = containerEl->getElementsByTagName("item");

	set.resize(strings->length());
	for (unsigned long int i = 0; i < strings->length(); i++)
	{
		Element* s = static_cast<Element*> (strings->item(i));
		set[i] = s->getAttribute("value");

		AutoPtr<NodeList> probabilities = s->getElementsByTagName("probability");
		for (unsigned long int j = 0; j < probabilities->length(); j++)
		{
			setProbability(i, static_cast<Element*> (probabilities->item(j)));
		}
	}
}

void AbstractGeneratorConfig::setProbability(const ID x, const Element* probability)
{
	CustomDiscreteProbability& f = func<CustomDiscreteProbability>(probability->getAttribute("function"));
	f.define(x, fromString<Decimal>(probability->getAttribute("value")));
}

void AbstractGeneratorConfig::computeFixedPartitioning(const string& key)
{
	ID cardinality = fromString<ID> (getString("partitioning." + key + ".cardinality"));

	ID genIDBegin, genIDEnd;

	// chunk 0 is responsible for dumping all data
	if (chunkID() == 0)
	{
		genIDBegin = 0;
		genIDEnd = cardinality;
	}
	else
	{
		genIDBegin = cardinality;
		genIDEnd = cardinality;
	}

	setString("generator." + key + ".sequence.cardinality", toString(cardinality));
	setString("generator." + key + ".partition.begin", toString(genIDBegin));
	setString("generator." + key + ".partition.end", toString(genIDEnd));
}

void AbstractGeneratorConfig::computeLinearScalePartitioning(const string& key)
{
	I64u cardinality = static_cast<I64u>(scalingFactor() * getInt("partitioning." + key + ".base-cardinality"));
	double chunkSize = cardinality / static_cast<double> (numberOfChunks());

	I64u genIDBegin = static_cast<ID> ((chunkSize * chunkID()) + 0.5);
	I64u genIDEnd = static_cast<ID> ((chunkSize * (chunkID() + 1) + 0.5));

	setString("generator." + key + ".sequence.cardinality", toString(cardinality));
	setString("generator." + key + ".partition.begin", toString(genIDBegin));
	setString("generator." + key + ".partition.end", toString(genIDEnd));
}

void AbstractGeneratorConfig::computeMirroredPartitioning(const string& key)
{
	string masterKey = getString("partitioning." + key + ".master");

	I64u cardinality = fromString<I64u> (getString("generator." + masterKey + ".sequence.cardinality"));
	I64u genIDBegin = fromString<I64u> (getString("generator." + masterKey + ".partition.begin"));
	I64u genIDEnd = fromString<I64u> (getString("generator." + masterKey + ".partition.end"));

	setString("generator." + key + ".sequence.cardinality", toString(cardinality));
	setString("generator." + key + ".partition.begin", toString(genIDBegin));
	setString("generator." + key + ".partition.end", toString(genIDEnd));
}

void AbstractGeneratorConfig::computePeriodBoundPartitioning(const string& key)
{
	// start and end date
	int minDateTimeTzd, maxDateTimeTzd;
	DateTime minDateTime, maxDateTime;
	DateTimeParser::parse("%Y-%m-%d %H:%M:%S", getString("partitioning." + key + ".period-min"), minDateTime, minDateTimeTzd);
	DateTimeParser::parse("%Y-%m-%d %H:%M:%S", getString("partitioning." + key + ".period-max"), maxDateTime, maxDateTimeTzd);
	I64u itemsPerSecond = fromString<I64u>(getString("partitioning." + key + ".items-per-second"));

	Timespan span = maxDateTime - minDateTime;

	I64u cardinality = static_cast<I64u>(scalingFactor() * itemsPerSecond * span.totalSeconds());
	double chunkSize = cardinality / static_cast<double> (numberOfChunks());

	I64u genIDBegin = static_cast<ID> ((chunkSize * chunkID()) + 0.5);
	I64u genIDEnd = static_cast<ID> ((chunkSize * (chunkID() + 1) + 0.5));

	setString("generator." + key + ".sequence.cardinality", toString(cardinality));
	setString("generator." + key + ".partition.begin", toString(genIDBegin));
	setString("generator." + key + ".partition.end", toString(genIDEnd));
}

void AbstractGeneratorConfig::computeNestedPartitioning(const string& key)
{
	string parentKey = getString("partitioning." + key + ".parent");
	I32u itemsPerParent = getInt("partitioning." + key + ".items-per-parent");

	I64u cardinality = fromString<I64u> (getString("generator." + parentKey + ".sequence.cardinality"));
	I64u genIDBegin = fromString<I64u> (getString("generator." + parentKey + ".partition.begin"));
	I64u genIDEnd = fromString<I64u> (getString("generator." + parentKey + ".partition.end"));

	setString("generator." + key + ".sequence.cardinality", toString(cardinality * itemsPerParent));
	setString("generator." + key + ".partition.begin", toString(genIDBegin * itemsPerParent));
	setString("generator." + key + ".partition.end", toString(genIDEnd * itemsPerParent));
}

const string AbstractGeneratorConfig::resolveValue(const string& value)
{
	RegularExpression::MatchVec posVec;
	if (_pattern_param_ref.match(value, 0, posVec))
	{
		string key = value.substr(posVec[1].offset, posVec[1].length);
		return getString("generator." + key);
	}
	else
	{
		return value;
	}
}

} // namespace Myriad
