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

    typedef FileOutputStream StreamType; //!< The type of the underlying stream.

    /**
     * Constructor.
     *
     * Opens an output stream in a file named as the value of the
     * <tt>"generator.{generatorName}.output-file"</tt> config property. The
     * output folder where the file is stored is given by the value of the
     * <tt>application.output-dir</tt> config parameter.
     */
    LocalFileOutputCollector(const String& generatorName, const GeneratorConfig& config) :
        AbstractOutputCollector<RecordType>(generatorName, config),
        _isOpen(false),
        _logger(Logger::get("collector." + generatorName))
    {
        // make sure that the output-dir exists
        File outputDir(config.getString("application.output-dir"));
        outputDir.createDirectories();

        // compute the output path
        Path path(config.getString(format("generator.%s.output-file", generatorName), generatorName));
        path.makeAbsolute(config.getString("application.output-dir"));

        _path = path.toString();
    }

    /**
     * Copy constructor.
     */
    LocalFileOutputCollector(const LocalFileOutputCollector& o) :
        AbstractOutputCollector<RecordType>(o),
        _path(o._path),
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
     * Closes the internal StreamType instance if opened.
     */
    virtual ~LocalFileOutputCollector()
    {
        close();
    }

    /**
     * Opens the internal StreamType instance.
     */
    void open()
    {
        if (!_isOpen)
        {
	        _logger.debug(format("Opening local file output `%s`", _path));

	        _out.open(_path, std::ios::trunc | std::ios::binary);
	        writeHeader();
	        _isOpen = true;
        }
        else
        {
	        throw LogicException(format("Can't open already opened FileOutputStream %s", _path));
        }
    }

    /**
     * Closes the internal StreamType instance.
     */
    void close()
    {
        if (_isOpen)
        {
	        _logger.debug(format("Closing local file output `%s`", _path));
	        writeFooter();
	        _out.close();
        }
    }

    /**
     * Flushes the internal StreamType instance.
     */
    void flush()
    {
        if (_isOpen)
        {
            _out.flush();
        }
    }

    /**
     * Writes out an output specific header.
     */
    void writeHeader()
    {
    }

    /**
     * Writes out an output specific footer.
     */
    void writeFooter()
    {
    }

    /**
     * Output collection method.
     */
    void collect(const RecordType& record)
    {
        LocalFileOutputCollector<RecordType>::serialize(_out, record);
    }

    /**
     * Output collection method.
     */
    static void serialize(StreamType& out, const RecordType& record)
    {
        out << "abstract record #" << record.genID() << "\n";
    }

private:

    /**
     * The path of the underlying OutputStream.
     */
    std::string _path;

    /**
     * The underlying output stream.
     */
    StreamType _out;

    /**
     * A boolean flag indicating that the underlying output stream is open.
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
