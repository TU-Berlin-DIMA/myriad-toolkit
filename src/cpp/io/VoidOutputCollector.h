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

#ifndef VOIDOUTPUTCOLLECTOR_H_
#define VOIDOUTPUTCOLLECTOR_H_

#include "io/AbstractOutputCollector.h"

#include <Poco/Buffer.h>
#include <Poco/Logger.h>
#include <Poco/NullStream.h>

namespace Myriad {
/**
 * @addtogroup io
 * @{*/

/**
 * A VoidOutputCollector subclass that merely consumes the output.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<typename RecordType>
class VoidOutputCollector: public AbstractOutputCollector<RecordType>
{
public:

    /**
     * Constructor.
     *
     * Opens a null output stream.
     */
    VoidOutputCollector(const Path& outputPath, const String& collectorName) :
        AbstractOutputCollector<RecordType>(collectorName),
        _outputPath(outputPath),
        _isOpen(false),
        _logger(Logger::get(collectorName))
    {
    }

    /**
     * Copy constructor.
     */
    VoidOutputCollector(const VoidOutputCollector& o) :
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
    virtual ~VoidOutputCollector()
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
            _logger.debug(format("Opening null output stream for output path `%s`", _outputPath.toString()));

            AbstractOutputCollector<RecordType>::writeHeader(_outputStream);
            _isOpen = true;
        }
        else
        {
            throw LogicException(format("Can't open null output stream for output path `%s`", _outputPath.toString()));
        }
    }

    /**
     * Closes the internal NullOutputStream instance.
     */
    void close()
    {
        if (_isOpen)
        {
            _logger.debug(format("Closing null output stream for output path `%s`", _outputPath.toString()));

            AbstractOutputCollector<RecordType>::writeFooter(_outputStream);
            flush();
        }
    }

    /**
     * Flushes the internal NullOutputStream instance and resets the flush counter.
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
        VoidOutputCollector<RecordType>::serialize(_outputStream, record);
    }

private:

    /**
     * The path for this collector (untied to the underlying output stream).
     */
    const Path _outputPath;

    /**
     * The underlying output stream.
     */
    NullOutputStream _outputStream;

    /**
     * A boolean flag indicating that the underlying \p _outputStream is open.
     */
    bool _isOpen;

    /**
     * Logger instance.
     */
    Logger& _logger;
};

/** @}*/// add to io group
} // namespace Myriad

#endif /* VOIDOUTPUTCOLLECTOR_H_ */
