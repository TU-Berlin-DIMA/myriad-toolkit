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

#ifndef OBJECTBUILDER_H_
#define OBJECTBUILDER_H_

#include "core/types.h"
#include "core/exceptions.h"
#include "math/probability/Probability.h"

#include <Poco/Any.h>
#include <Poco/DynamicAny.h>

#include <map>
#include <string>

using namespace std;
using namespace Poco;

namespace Myriad {

class ObjectBuilder
{
public:

	template<class T> ObjectBuilder& addParameter(string name, T value)
	{
		params[name] = Any(value);
		return *this;
	}

	ObjectBuilder& clear()
	{
		params.clear();
		return *this;
	}

	template<class T> T* createObject()
	{
		map<string, Any>::const_iterator it = params.find("name");
		if (it != params.end())
		{
			string name = AnyCast<string>(it->second);
			params.erase("name");
			return new T(name, params);
		}
		else
		{
			throw ConfigException("Missing required parameter `name`");
		}
	}

	template<class T> T* create(const string& name)
	{
		return new T(name, params);
	}

	template<class T> T* create()
	{
		return new T(params);
	}

private:

	map<string, Any> params;
};

} // namespace Myriad

#endif /* OBJECTBUILDER_H_ */
