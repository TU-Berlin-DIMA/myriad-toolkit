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

#ifndef ABSTRACTOUTPUTCOLLECTOR_H_
#define ABSTRACTOUTPUTCOLLECTOR_H_

#include "config/GeneratorConfig.h"
#include "record/Record.h"

using namespace Poco;

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
class AbstractOutputCollector
{
public:

    /**
     * Constructor.
     */
	AbstractOutputCollector(const String& generatorName, const GeneratorConfig& config) :
		_config(config)
	{
	}

	/**
	 * Copy constructor.
	 */
	AbstractOutputCollector(const AbstractOutputCollector& o) :
        _config(o._config)
	{
	}

    /**
     * Copy constructor.
     */
	virtual ~AbstractOutputCollector()
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
	 * Write an output header.
	 */
	virtual void writeHeader() = 0;

	/**
     * Write an output footer.
	 */
	virtual void writeFooter() = 0;

	/**
	 * Collect and write out a single \p RecordType instance.
	 */
	virtual void collect(const RecordType& record) = 0;

protected:

	/**
	 * A reference to the generator config.
	 */
	const GeneratorConfig& _config;
};

/** @}*/// add to io group
} // namespace Myriad

#endif /* ABSTRACTOUTPUTCOLLECTOR_H_ */
