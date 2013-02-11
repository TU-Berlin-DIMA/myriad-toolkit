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
#include <Poco/StreamCopier.h>

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
     * Opens an output stream in a file given by the value of the given
     * \p outputPath parameter.
     */
    LocalFileOutputCollector(const Path& outputPath, const String& collectorName) :
        AbstractOutputCollector<RecordType>(outputPath, collectorName),
        _outputPath(outputPath),
        _isOpen(false),
        _logger(Logger::get(collectorName))
    {
    }

    /**
     * Copy constructor.
     */
    LocalFileOutputCollector(const LocalFileOutputCollector& o) :
        AbstractOutputCollector<RecordType>(o),
        _outputPath(o._outputPath),
        _isOpen(false),
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
            _logger.debug(format("Opening local file for output path `%s`", _outputPath.toString()));

	        // make sure that the output-dir exists
	        File outputDir(_outputPath.parent());
            outputDir.createDirectories();

	        _outputStream.open(_outputPath.toString(), std::ios::trunc | std::ios::binary);
	        AbstractOutputCollector<RecordType>::writeHeader(_outputStream);
	        _isOpen = true;
        }
        else
        {
	        throw LogicException(format("Can't open already opened local file at `%s`", _outputPath.toString()));
        }
    }

    /**
     * Closes the internal FileOutputStream instance.
     */
    void close()
    {
        if (_isOpen)
        {
	        _logger.debug(format("Closing local file for output `%s`", _outputPath.toString()));

	        AbstractOutputCollector<RecordType>::writeFooter(_outputStream);
	        flush();
	        _outputStream.close();
        }
    }

    /**
     * Flushes the internal FileOutputStream instance and resets the flush counter.
     */
    void flush()
    {
        if (_isOpen)
        {
            _outputStream.flush();
        }
    }

    /**
     * Output collection method.
     */
    void collect(const RecordType& record)
    {
        LocalFileOutputCollector<RecordType>::serialize(_outputStream, record);
    }

private:

    /**
     * The path of the underlying OutputStream.
     */
    const Path _outputPath;

    /**
     * The underlying output stream.
     */
    FileOutputStream _outputStream;

    /**
     * A boolean flag indicating that the underlying \p _outputFile is open.
     */
    bool _isOpen;

    /**
     * Logger instance.
     */
    Logger& _logger;
};

/** @}*/// add to io group
} // namespace Myriad

#endif /* LOCALFILEOUTPUTCOLLECTOR_H_ */
