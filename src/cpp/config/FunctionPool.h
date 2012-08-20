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
/**
 * @addtogroup config
 * @{*/

/**
 * A container for dynamically allocated AbstractFunction objects.
 *
 * All functions are registered with an autorelease pool that takes care of
 * reseource cleanup when the FunctionPool object expires.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class FunctionPool
{
public:

	/**
	 * Default constructor.
	 */
	FunctionPool()
	{
	}

	/**
	 * Add a new function pointer to the pool.
	 *
	 * @param functionPtr a pointer to the function to be added.
	 * @throw ConfigException if the function is already registered
	 */
	void add(AbstractFunction* functionPtr)
	{
		map<string, AbstractFunction*>::const_iterator it = functions.find(functionPtr->name());

		if (it != functions.end())
		{
			throw ConfigException("Trying to add an already undefined function " + functionPtr->name());
		}

		autoreleasePool.add(functionPtr);
		functions[functionPtr->name()] = functionPtr;
	}

	/**
	 * Get a type-safe version of an already registered function identified by
	 * the given \p name.
	 *
	 * @return a reference to a \p FunctionType function identified by the
	 *         given \p name
	 * @throw ConfigException if the function identified by the given \name is
	 *                        not registered or if it cannot be cast to the
	 *                        provided \p FunctionType
	 */
	template<class FunctionType> FunctionType& get(const string& name) const
	{
		map<string, AbstractFunction*>::const_iterator it = functions.find(name);

		if (it == functions.end())
		{
			throw ConfigException("Trying to access undefined function " + name);
		}

		const AbstractFunction* abstractFunction(it->second);
		const FunctionType* castFunction = dynamic_cast<const FunctionType*>(abstractFunction);

		if (castFunction)
		{
			return *const_cast<FunctionType*>(castFunction);
		}
		else
		{
			throw ConfigException("Cannot cast function "  + name + " to the supplied template type ");
		}
	}

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

/** @}*/// add to core group
}  // namespace Myriad

#endif /* FUNCTIONPOOL_H_ */
