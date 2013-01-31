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

#ifndef LOCALFILEOUTPUTCOLLECTOR_H_
#define LOCALFILEOUTPUTCOLLECTOR_H_

#include "io/AbstractOutputCollector.h"

#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/Logger.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Path.h>

using namespace Poco;

namespace Myriad {
/**
 * @addtogroup io
 * @{*/

/**
 * An AbstractOutputCollector subclass that writes the output into the local
 * file system.
 *
 * This output collector works with local filesystem output streams and is the
 * default one for all data generator applications.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<typename RecordType>
class LocalFileOutputCollector: public AbstractOutputCollector<RecordType>
{
public:

    /**
     * Constructor.
     *
     * Opens an output stream in a file named as the value of the
     * <tt>"generator.{generatorName}.output-file"</tt> config property. The
     * output folder where the file is stored is given by the value of the
     * <tt>application.output-dir</tt> config parameter.
     */
    LocalFileOutputCollector(const String& generatorName, const GeneratorConfig& config) :
        AbstractOutputCollector<RecordType>(),
        _isOpen(false),
        _flushRate(1000),
        _flushCounter(0),
        _logger(Logger::get("collector." + generatorName))
    {
        // make sure that the output-dir exists
        File outputDir(config.getString("application.output-dir"));
        outputDir.createDirectories();

        // compute the output path
        Path path(config.getString(format("generator.%s.output-file", generatorName), generatorName));
        path.makeAbsolute(config.getString("application.output-dir"));

        _outputPath = path.toString();
    }

    /**
     * Constructor.
     *
     * Opens an output stream in a file given by the value of the given
     * \p outputPath parameter.
     */
    LocalFileOutputCollector(const Path& outputPath, const String& collectorName) :
        AbstractOutputCollector<RecordType>(),
    	_isOpen(false),
        _flushRate(1000),
        _flushCounter(0),
        _logger(Logger::get(collectorName))
    {
        _outputPath = outputPath.toString();
    }

    /**
     * Copy constructor.
     */
    LocalFileOutputCollector(const LocalFileOutputCollector& o) :
        AbstractOutputCollector<RecordType>(o),
        _outputPath(o._outputPath),
        _isOpen(false),
        _flushRate(o._flushRate),
        _flushCounter(o._flushCounter),
        _logger(Logger::get(o._logger.name()))
    {
        if (o._isOpen)
        {
	        open();
        }
    }

    /**
     * Destructor.
     *
     * Closes the internal FileOutputStream instance if opened.
     */
    virtual ~LocalFileOutputCollector()
    {
        close();
    }

    /**
     * Opens the internal FileOutputStream instance.
     */
    void open()
    {
        if (!_isOpen)
        {
	        _logger.debug(format("Opening local file output `%s`", _outputPath));

	        _outputFile.open(_outputPath, std::ios::trunc | std::ios::binary);
	        this->writeHeader();
	        _isOpen = true;

	        this->_outputBuffer.str("");
			this->_outputBuffer.clear();
        }
        else
        {
	        throw LogicException(format("Can't open already opened FileOutputStream %s", _outputPath));
        }
    }

    /**
     * Closes the internal FileOutputStream instance.
     */
    void close()
    {
        if (_isOpen)
        {
	        _logger.debug(format("Closing local file output `%s`", _outputPath));
	        this->writeFooter();
	        _outputFile.close();
        }
    }

    /**
     * Flushes the internal FileOutputStream instance and resets the flush counter.
     */
    void flush()
    {
    	// TODO: copy everything from the output buffer
    	StreamCopier::copyStream(this->_outputBuffer, _outputFile, 8192);

        if (_isOpen)
        {
            _outputFile.flush();
        }

        this->_outputBuffer.str("");
        this->_outputBuffer.clear();
    	_flushCounter = 0;
    }

    /**
     * Output collection method.
     */
    void collect(const RecordType& record)
    {
    	std::ostream::pos_type p2, p1 = this->_outputBuffer.tellp();
        if (p1 == -1)
        {
        	throw RuntimeException("Failed to read before-position from output stream");
        }

        LocalFileOutputCollector<RecordType>::serialize(this->_outputBuffer, record);

        p2 = this->_outputBuffer.tellp();
        if (p2 == -1)
        {
        	throw RuntimeException("Failed to read after-position from output stream");
        }

        _flushCounter++;
        if (_flushCounter >= _flushRate)
        {
        	flush();
        }
    }

private:

    /**
     * The path of the underlying OutputStream.
     */
    std::string _outputPath;

    /**
     * The underlying output stream.
     */
    FileOutputStream _outputFile;

    /**
     * A boolean flag indicating that the underlying \p _outputFile is open.
     */
    bool _isOpen;

    /**
     * The rate (in records) with which the \p _outputBuffer is flushed to the
     * \p outputFile.
     */
    I16u _flushRate;

    /**
     * A counter of the records written into the \p _outputBuffer since the last
     * flush() call.
     */
    I16u _flushCounter;

    /**
     * Logger instance.
     */
    Logger& _logger;
};

/** @}*/// add to io group
} // namespace Myriad

#endif /* LOCALFILEOUTPUTCOLLECTOR_H_ */
