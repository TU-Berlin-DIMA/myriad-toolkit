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
 * @author: Alexander Alexandrov <alexander.s.alexandrov@campus.tu-berlin.de>
 */

#ifndef GENERATOROUTPUTSTREAM_H_
#define GENERATOROUTPUTSTREAM_H_

#include "record/Record.h"


#include <Poco/FileStream.h>

using namespace Poco;

namespace Myriad {

class MyriadOutputStream: public FileOutputStream
{
};

class GeneratorOutputCollector
{
public:

	GeneratorOutputCollector() : _path("")
	{
	}

	GeneratorOutputCollector(const GeneratorOutputCollector& o) : _path(o._path), _mode(o._mode)
	{
		if (_path != "")
		{
			_out.open(_path, _mode);
		}
	}

	GeneratorOutputCollector& operator =(const GeneratorOutputCollector& o)
	{
		_path = o._path;
		_mode = o._mode;

		if (_path != "")
		{
			_out.open(_path, _mode);
		}

		return *this;
	}

	void open(const std::string& path, std::ios::openmode mode)
	{
		if (_path == "")
		{
			_path = path;
			_mode = mode;
			_out.open(_path, _mode);
		}
		else
		{
			throw LogicException(format("Can't open already opened FileOutputStream %s", path));
		}
	}

	void close()
	{
		if (_path != "")
		{
			_path = "";
			_out.close();
		}
	}

	template<typename RecordType> void emit(const RecordType& record);

private:

	std::string _path;

	std::ios::openmode _mode;

public:

	MyriadOutputStream _out;
};

template<typename RecordType> inline void GeneratorOutputCollector::emit(const RecordType& record)
{
	_out << record;
}

} // namespace Myriad

#endif /* GENERATOROUTPUTSTREAM_H_ */
