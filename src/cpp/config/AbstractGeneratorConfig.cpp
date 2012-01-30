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

	// get the path of the master config xml file
	Path configPath(getString("common.config.master-file", getString("application.baseName")+".xml"));
	configPath.makeAbsolute(getString("application.config-dir"));

	_logger.information("Loading generator configuration from " + configPath.toString());
	loadXMLConfig(configPath);

	// initialize the master random stream
	_masterPRNG.seed(RandomStream::Seed(getString("common.master.seed", "0,0,0,0,0,0")));
}

void AbstractGeneratorConfig::loadXMLConfig(const Path& path)
{
	DOMParser parser;
	AutoPtr<Document> doc = parser.parse(path.toString());
	doc->normalize();

	configureParameters(doc);
	configureFunctions(doc);
	configurePartitioning(doc);
	configureSets(doc);
}

void AbstractGeneratorConfig::configureParameters(const AutoPtr<Document>& doc)
{
	Element* parametersEl = static_cast<Element*> (doc->getElementsByTagName("parameters")->item(0));

	AutoPtr<NodeList> parameters = parametersEl->getElementsByTagName("parameter");
	for (unsigned long int j = 0; j < parameters->length(); j++)
	{
		Element* p = static_cast<Element*> (parameters->item(j));
		setString("generator." + p->getAttribute("name"), p->innerText());
	}
}

void AbstractGeneratorConfig::configureFunctions(const AutoPtr<Document>& doc)
{
	ObjectBuilder builder;

	Element* functionsEl = static_cast<Element*> (doc->getElementsByTagName("functions")->item(0));
	AutoPtr<NodeList> functions = functionsEl->getElementsByTagName("function");
	for (unsigned long int i = 0; i < functions->length(); i++)
	{
		Element* f = static_cast<Element*> (functions->item(i));
		// read name and type
		String key = f->getAttribute("key");
		String type = f->getAttribute("type");

		// read function parameters
		AutoPtr<NodeList> parameters = f->getElementsByTagName("argument");
		for (unsigned long int j = 0; j < parameters->length(); j++)
		{
			Element* p = static_cast<Element*> (parameters->item(j));
			builder.addParameter(p->getAttribute("key"), fromString<Decimal>(p->innerText()));
		}

		// create function of the specified type
		if (type == "custom_discrete_probability")
		{
			// read function parameters
			AutoPtr<NodeList> probabilities = f->getElementsByTagName("probability");
			for (unsigned long int j = 0; j < probabilities->length(); j++)
			{
				Element* p = static_cast<Element*> (probabilities->item(j));
				builder.addParameter(p->getAttribute("argument"), fromString<Decimal>(p->getAttribute("value")));
			}

			addFunction(builder.create<CustomDiscreteProbability> (key));
		}
		else if (type == "interval_map")
		{
			addFunction(builder.create<IntervalMap<ID, ID> > (key));
		}
		else if (type == "id_range_map")
		{
			addFunction(builder.create<DiscreteMap<ID, Interval<ID> > > (key));
		}
		else if (type == "pareto_probability")
		{
			addFunction(builder.create<ParetoPrFunction> (key));
		}
		else if (type == "normal_probability")
		{
			addFunction(builder.create<NormalPrFunction> (key));
		}
		else if (type == "bounded_normal")
		{
			addFunction(builder.create<BoundedNormalPrFunction> (key));
		}
		else
		{
			throw FeatureConfigurationException(format("Unsupported function type '%s' for function", type, key));
		}

		builder.clear();
	}
}

void AbstractGeneratorConfig::configurePartitioning(const AutoPtr<Document>& doc)
{
	NodeList* partitioningElements = doc->getElementsByTagName("partitioning");

	if (partitioningElements->length() == 0)
	{
		return;
	}

	Element* partitioningEl = static_cast<Element*> (partitioningElements->item(0));

	AutoPtr<NodeList> partitionConfigs = partitioningEl->getElementsByTagName("partition");
	for (unsigned long int i = 0; i < partitionConfigs->length(); i++)
	{
		Element* f = static_cast<Element*> (partitionConfigs->item(i));

		// read name and type
		String type = f->getAttribute("type");
		String method = f->getAttribute("method");

		if (method == "fixed")
		{
			// TODO: determine cardinality from the loaded object set size
			setString("partitioning." + type + ".cardinality", f->getChildElement("cardinality")->innerText());
			computeFixedPartitioning(type);
		}
		else if (method == "simple")
		{
			setString("partitioning." + type + ".base-cardinality", f->getChildElement("base-cardinality")->innerText());
			computeSimplePartitioning(type);
		}
		else if (method == "mirrored")
		{
			setString("partitioning." + type + ".master", f->getChildElement("master")->innerText());
			computeMirroredPartitioning(type);
		}
		else if (method == "period-bound")
		{
			setString("partitioning." + type + ".period-min", resolveValue(f->getChildElement("period-min")->innerText()));
			setString("partitioning." + type + ".period-max", resolveValue(f->getChildElement("period-max")->innerText()));
			setString("partitioning." + type + ".items-per-second", resolveValue(f->getChildElement("items-per-second")->innerText()));
			computePeriodBoundPartitioning(type);
		}
		else if (method == "nested")
		{
			setString("partitioning." + type + ".parent", f->getChildElement("parent")->innerText());
			setString("partitioning." + type + ".items-per-parent", f->getChildElement("items-per-parent")->innerText());
			computeNestedPartitioning(type);
		}
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

void AbstractGeneratorConfig::computeSimplePartitioning(const string& key)
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

void AbstractGeneratorConfig::computeNestedBlockPartitioning(const string& key)
{
	// TODO: this partitioning method is functional but currently not supported. revision needed.

	string parentKey = getString("partitioning." + key + ".parent");
	string blockName = getString("partitioning." + key + ".block.name");
	string blockDistribution = getString("partitioning." + key + ".block.distribution");
	I32u blockSize = getInt("partitioning." + key + ".block.size.x");
	I32u blockSum = getInt("partitioning." + key + ".block.size.y");

	// get cardinality and partitioning parameters for parent type
	ID parentCardinality = fromString<ID> (getString("generator." + parentKey + ".sequence.cardinality"));
	ID parentGenIDBegin = fromString<ID> (getString("generator." + parentKey + ".partition.begin"));
	ID parentGenIDEnd = fromString<ID> (getString("generator." + parentKey + ".partition.end"));

	AnalyticPrFunction<Decimal, Decimal>& distribution = func<AnalyticPrFunction<Decimal, Decimal> >(blockDistribution);
	DiscreteDistribution<AnalyticPrFunction<Decimal, Decimal> > d(distribution, blockSize, blockSum, 1);

	// configure cardinality and partitioning parameters for nested type
	ID cardinality = (parentCardinality / blockSize) * blockSum + d.cdf(parentCardinality % blockSize) - d(parentCardinality % blockSize);
	ID genIDBegin = (parentGenIDBegin / blockSize) * blockSum + d.cdf(parentGenIDBegin % blockSize) - d(parentGenIDBegin % blockSize);
	ID genIDEnd = (parentGenIDEnd / blockSize) * blockSum + d.cdf(parentGenIDEnd % blockSize) - d(parentGenIDEnd % blockSize);

	addFunction(d.pdfMap(blockName + "_pdf"));
	addFunction(d.cdfMap(blockName + "_cdf"));

	setString("generator." + key + ".sequence.cardinality", toString(cardinality));
	setString("generator." + key + ".partition.begin", toString(genIDBegin));
	setString("generator." + key + ".partition.end", toString(genIDEnd));

	_logger.information(format("Configuring `%s` nested block sizing, `%s` has [%Lu, %Lu) of total %Lu", key, parentKey, parentGenIDBegin, parentGenIDEnd, parentCardinality));
	_logger.information(format("Configuring `%s` nested block sizing, %Lu full blocks, %Lu items before begin", key, parentGenIDBegin / blockSize, parentGenIDBegin % blockSize));
	_logger.information(format("Configuring `%s` nested block sizing, %Lu full blocks, %Lu items before end", key, parentGenIDEnd / blockSize, parentGenIDEnd % blockSize));
	_logger.information(format("Configuring `%s` nested block sizing: %Lu [%Lu, %Lu) of total %Lu", key, genIDEnd - genIDBegin, genIDBegin, genIDEnd, cardinality));
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
