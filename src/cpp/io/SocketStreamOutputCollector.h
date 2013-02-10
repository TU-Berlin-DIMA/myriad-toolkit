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

#ifndef SOCKETSTREAMOUTPUTCOLLECTOR_H_
#define SOCKETSTREAMOUTPUTCOLLECTOR_H_

#include "io/AbstractOutputCollector.h"

#include <Poco/DigestEngine.h>
#include <Poco/Logger.h>
#include <Poco/MD5Engine.h>
#include <Poco/NumberFormatter.h>
#include <Poco/StreamCopier.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>

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
class SocketStreamOutputCollector: public AbstractOutputCollector<RecordType>
{
public:

    /**
     * Constructor.
     *
     * Opens an output stream for the given \p outputPath in a localhost output
     * socket specified by the given \p outputPort.
     */
    SocketStreamOutputCollector(const Path& outputPath, const I16u outputPort, const String& collectorName) :
        AbstractOutputCollector<RecordType>(outputPath, collectorName),
        _outputPath(outputPath),
        _outputSocket(Poco::Net::IPAddress::IPv4),
        _outputPort(outputPort),
    	_isOpen(false),
        _flushRate(1000),
        _flushCounter(0),
        _logger(Logger::get(collectorName))
    {
    }

    /**
     * Copy constructor.
     */
    SocketStreamOutputCollector(const SocketStreamOutputCollector& o) :
        AbstractOutputCollector<RecordType>(o),
        _outputPath(o._outputPath),
        _outputSocket(o._outputSocket),
        _outputPort(o._outputPort),
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
    virtual ~SocketStreamOutputCollector()
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
            _logger.debug(format("Opening socket for output path `%s` at port %hu", _outputPath.toString(), _outputPort));

            try
            {
                Poco::Net::StreamSocket socket(Poco::Net::IPAddress::IPv4);
                _outputSocket.connect(Poco::Net::SocketAddress("localhost", _outputPort));
            }
            catch(const Poco::Exception& e)
            {
                throw RuntimeException(format("Could not connect to socket at address localhost:%hu", _outputPort));
            }

	        this->writeHeader();
	        _isOpen = true;

	        this->_outputBuffer.str("");
			this->_outputBuffer.clear();
        }
        else
        {
	        throw LogicException(format("Can't open already opened socket for output `%s`", _outputPath.toString()));
        }
    }

    /**
     * Closes the internal FileOutputStream instance.
     */
    void close()
    {
        if (_isOpen)
        {
	        _logger.debug(format("Closing socket stream for output `%s`", _outputPath.toString()));

	        this->writeFooter();
	        _outputSocket.close();
        }
    }

    /**
     * Flushes the internal FileOutputStream instance and resets the flush counter.
     */
    void flush()
    {
        Poco::Net::SocketStream socketStream(_outputSocket);
    	StreamCopier::copyStream(this->_outputBuffer, socketStream, 8192);

        if (_isOpen)
        {
            socketStream.flush();
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

        SocketStreamOutputCollector<RecordType>::serialize(this->_outputBuffer, record);

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

//    static I16u socketPortFromOutputPath(const Path& path)
//    {
//        Poco::MD5Engine md5;
//        md5.update(path.toString());
//        const Poco::DigestEngine::Digest& digest = md5.digest();
//
//        I64 hashSuffix = ((0x000000FF & static_cast<I64>(digest.at(digest.size()-4))) << 48) |
//                         ((0x000000FF & static_cast<I64>(digest.at(digest.size()-3))) << 32) |
//                         ((0x000000FF & static_cast<I64>(digest.at(digest.size()-2))) << 16) |
//                         ((0x000000FF & static_cast<I64>(digest.at(digest.size()-1))) << 0 );
//
//        return static_cast<I16u>(42100 + ((hashSuffix % 900 < 0) ? (1000 - (hashSuffix % 900)) : (hashSuffix % 900)));
//    }

private:

    /**
     * The path of the underlying OutputStream.
     */
    const Path _outputPath;

    /**
     * The underlying output stream.
     */
    Poco::Net::StreamSocket _outputSocket;

    /**
     * The connection port for the stream socket.
     */
    const I16u _outputPort;

    /**
     * A boolean flag indicating that the underlying \p _outputSocket is open.
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

#endif /* SOCKETSTREAMOUTPUTCOLLECTOR_H_ */
