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

#ifndef ABSTRACTOUTPUTCOLLECTOR_H_
#define ABSTRACTOUTPUTCOLLECTOR_H_

#include "config/GeneratorConfig.h"
#include "record/Record.h"

using namespace Poco;

namespace Myriad
{

template<typename RecordType> class AbstractOutputCollector
{
public:

	AbstractOutputCollector(const String& generatorName, const GeneratorConfig& config)
	{
	}

	AbstractOutputCollector(const AbstractOutputCollector& o)
	{
	}

	virtual ~AbstractOutputCollector()
	{
	}

	/**
	 * Open the underlying stream.
	 */
	virtual void open() = 0;

	/**
	 * Close the underlying stream.
	 */
	virtual void close() = 0;

	/**
	 * Output header method.
	 */
	virtual void writeHeader() = 0;

	/**
	 * Output footer method.
	 */
	virtual void writeFooter() = 0;

	/**
	 * Output collection method.
	 */
	void collect(const RecordType& record)
	{
		throw new NotImplementedException("");
	}
};

} // namespace Myriad

#endif /* ABSTRACTOUTPUTCOLLECTOR_H_ */
