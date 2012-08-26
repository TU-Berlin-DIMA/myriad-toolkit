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

    // expose some application.* parameters unter generator.ENV.*
    setString("generator.ENV.config-dir", getString("application.config-dir"));
    setString("generator.ENV.output-dir", getString("application.output-dir"));

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

    _logger.information("Loading generator configuration");

    configurePartitioning();
    configureFunctions();
    configureSets();

    if (hasProperty("common.master.seed"))
    {
        // initialize the master random stream (otherwise use the default master seed)
        _masterPRNG.seed(RandomStream::Seed(getString("common.master.seed")));
    }
}

void AbstractGeneratorConfig::bindEnumSet(const string& key, Path path)
{
    // compute the output path
    path.makeAbsolute(getString("application.config-dir"));

    if (_logger.debug())
    {
        _logger.debug(format("Loading enum set from file `%s`", path.toString()));
    }

    if (!path.isFile())
    {
        throw ConfigException(format("Cannot find file at `%s`", path.toString()));
    }

    File file(path);

    if (!file.canRead())
    {
        throw ConfigException(format("Cannot read from file at `%s`", path.toString()));
    }

    ifstream in(file.path().c_str());

    if (!in.is_open())
    {
        throw ConfigException(format("Cannot open file at `%s`", path.toString()));
    }

    string line;

    try
    {
        vector<String>& set = _enumSets[key];

        // read first line
        getline(in, line);

        if (!in.good() || line.substr(0, 17) != "@numberofvalues =")
        {
	        throw DataException("Unexpected file header");
        }

        I32 numberOfEntries = atoi(line.substr(17).c_str());

        set.resize(numberOfEntries);

        for (I16u i = 0; i < numberOfEntries; i++)
        {
	        if (!in.good())
	        {
		        throw DataException("Bad line for bin #" + toString(i));
	        }

	        getline(in, line);

	        string::size_type t = line.find_first_of('\t');
	        string::size_type c = line.find_first_of('#');
	        string value;

	        if (t == string::npos)
	        {
                throw DataException("Bad line for bin #" + toString(i) + ", expected format '{i}\t{value}[# {comment}]");
	        }

	        if (c != string::npos)
	        {
		        value = line.substr(t+1, c-t-1);
		        trim(value);
	        }
	        else
	        {
		        value = line.substr(t+1);
		        trim(value);
	        }

            set[i] = value;
        }

        in.close();
    }
    catch(Exception& e)
    {
        in.close();
        throw e;
    }
    catch(exception& e)
    {
        in.close();
        throw e;
    }
    catch(...)
    {
        in.close();
        throw;
    }
}

void AbstractGeneratorConfig::computeFixedPartitioning(const string& key)
{
    ID cardinality = fromString<ID> (getString("partitioning." + key + ".cardinality"));

    ID genIDBegin, genIDEnd;

    // chunk 0 is responsible for dumping all data
    if (nodeID() == 0)
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

    I64u genIDBegin = static_cast<ID> ((chunkSize * nodeID()) + 0.5);
    I64u genIDEnd = static_cast<ID> ((chunkSize * (nodeID() + 1) + 0.5));

    setString("generator." + key + ".sequence.base_cardinality", getString("partitioning." + key + ".base-cardinality"));
    setString("generator." + key + ".sequence.cardinality", toString(cardinality));
    setString("generator." + key + ".partition.begin", toString(genIDBegin));
    setString("generator." + key + ".partition.end", toString(genIDEnd));
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
