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

#ifndef ABSTRACTOUTPUTCOLLECTOR_H_
#define ABSTRACTOUTPUTCOLLECTOR_H_

#include "core/types.h"

#include <Poco/Path.h>
#include <Poco/RefCountedObject.h>

#include <sstream>

namespace Myriad {
/**
 * @addtogroup io
 * @{*/

/**
 * An abstract output collector template.
 *
 * This is an abstract base for all output collectors. The
 * AbstractOutputCollector interface provides means to open and close an
 * output stream and write (collect) records into it. Concrete output collectors
 * are typically used by the GeneratorTask implementations to write out the
 * generated data.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<typename RecordType>
class AbstractOutputCollector : public Poco::RefCountedObject
{
public:

    /**
     * Constructor.
     */
    AbstractOutputCollector(const String& collectorName) :
        _collectorName(collectorName)
    {
    }

    /**
     * Copy constructor.
     */
    AbstractOutputCollector(const AbstractOutputCollector& o) :
        _collectorName(o._collectorName)
    {
    }

    /**
     * Copy constructor.
     */
    virtual ~AbstractOutputCollector()
    {
    }

    /**
     * Writes out an output specific header.
     *
     * @param out A reference to the <tt>std::ostream</tt> on which the output
     *        will be written.
     */
    virtual void writeHeader(std::ostream& out)
    {
    }

    /**
     * Writes out an output specific footer.
     *
     * @param out A reference to the <tt>std::ostream</tt> on which the output
     *        will be written.
     */
    virtual void writeFooter(std::ostream& out)
    {
    }

    /**
     * Open the underlying output stream.
     */
    virtual void open() = 0;

    /**
     * Flush and close the underlying output stream.
     */
    virtual void close() = 0;

    /**
     * Flushes the underlying output stream.
     */
    virtual void flush() = 0;

    /**
     * Collect and write out a single \p RecordType instance.
     */
    virtual void collect(const RecordType& record) = 0;

    /**
     * Record serialization method.
     *
     * Writes the generated \p record into the given \p outputBuffer.
     *
     * @param out A reference to the <tt>std::ostream</tt> on which the object
     *        should be serialized.
     * @param record The record to be serialized on the output buffer.
     */
    static void serialize(std::ostream& out, const RecordType& record)
    {
        out << "abstract record #" << record.genID() << "\n";
    }

private:

    const String& _collectorName;
};

/** @}*/// add to io group
} // namespace Myriad

#endif /* ABSTRACTOUTPUTCOLLECTOR_H_ */
