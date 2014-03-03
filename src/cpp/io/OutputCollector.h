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

#ifndef OUTPUTCOLLECTOR_H_
#define OUTPUTCOLLECTOR_H_

#include "io/AbstractOutputCollector.h"
#include "io/LocalFileOutputCollector.h"
#include "io/SocketStreamOutputCollector.h"
#include "io/VoidOutputCollector.h"

#include <Poco/AutoPtr.h>

namespace Myriad {
/**
 * @addtogroup io
 * @{*/

/**
 * OutputCollector factory.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class RecordType> struct OutputCollector
{
    /**
     * Factory method.
     *
     * Constructs appropriate output collector based on given \p collectorType.
     *
     * @return The constructed <tt>AbstractOutputCollector<RecordType></tt>
     *         subclass instance.
     */
    static Poco::AutoPtr< AbstractOutputCollector<RecordType> > factory(const String& collectorType, const I16u collectorPort, const Poco::Path& outputPath, const String& collectorName)
    {
        // local file
        if (collectorType == "file")
        {
            return new LocalFileOutputCollector<RecordType>(outputPath, collectorName);
        }
        // socket stream
        if (collectorType == "socket")
        {
            return new SocketStreamOutputCollector<RecordType>(outputPath, collectorPort, collectorName);
        }
        // void stream
        if (collectorType == "void")
        {
            return new VoidOutputCollector<RecordType>(outputPath, collectorName);
        }
        // unknown output collector type
        throw RuntimeException("Cannot construct output collector of type `" + collectorType + "`");
    }
};

/** @}*/// add to io group
} // namespace Myriad

#endif /* OUTPUTCOLLECTOR_H_ */
