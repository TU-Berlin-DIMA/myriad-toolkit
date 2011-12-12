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

#ifndef FUNCTIONPOOL_H_
#define FUNCTIONPOOL_H_

#include "core/exceptions.h"
#include "math/Function.h"

#include <string>
#include <map>
#include <Poco/AutoPtr.h>
#include <Poco/AutoReleasePool.h>
#include <Poco/Exception.h>
#include <Poco/Logger.h>
#include <Poco/Path.h>
#include <Poco/Util/LayeredConfiguration.h>
#include <Poco/SAX/XMLReader.h>

using namespace std;
using namespace Poco;
using namespace Poco::Util;

namespace Myriad {

class FunctionPool
{
public:

	void addFunction(AbstractFunction* functionPtr);
	AbstractFunction* function(const string& name);
	template<class C> C& func(const string& name) const;

private:

	/**
	 * A set of probability functions to be used by the configurator.
	 */
	map<string, AbstractFunction*> functions;

	/**
	 * An auto-release pool for the probabilities.
	 */
	AutoReleasePool<AbstractFunction> autoreleasePool;
};

template<class C> C& FunctionPool::func(const string& name) const
{

	map<string, AbstractFunction*>::const_iterator it = functions.find(name);
	if (it != functions.end())
	{
		const AbstractFunction* pSS(it->second);
		const C* pC = dynamic_cast<const C*>(pSS);
		if (pC) return *const_cast<C*>(pC);
	}

	throw ConfigException("Trying to access undefined function " + name);
}

}  // namespace Myriad

#endif /* FUNCTIONPOOL_H_ */
