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
/**
 * @addtogroup config
 * @{*/

/**
 * A generic builder for instances of a template class \p T.
 *
 * Instances of this class can be used to accumulate parameters of arbitrary
 * types with the ObjectBuilder::addParameter template method and then to
 * instantiate an object by passing the collected parameters to the appropriate
 * constructor.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class ObjectBuilder
{
public:

    /**
     * Default constructor.
     */
    ObjectBuilder()
    {
    }

    /**
     * Add a parameter \p value identified by the given \p name to the current
     * collection.
     */
    template<class T> ObjectBuilder& addParameter(String name, T value)
    {
        params[name] = Any(value);
        return *this;
    }

    /**
     * Clear the current parameter collection.
     */
    ObjectBuilder& clear()
    {
        params.clear();
        return *this;
    }

    /**
     * Create a new named instante of the type \p T.
     *
     * This method assumes a \p name parameter is provided it the current
     * parameter map, removes it and invokes the
     * <tt>T(string& name, map<string, Any> params)</tt> constructor.
     */
    template<class T> T* createObject()
    {
        map<String, Any>::const_iterator it = params.find("name");
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

    /**
     * Create a new instante of the type \p T with the given \p name used as
     * first parameter in a <tt>T(name, params)</tt> constructor.
     */
    template<class T> T* create(const String& name)
    {
        return new T(name, params);
    }

    /**
     * Create a new instante of the type \p T.
     */
    template<class T> T* create()
    {
        return new T(params);
    }

private:

    map<String, Any> params;
};

/** @}*/// add to core group
} // namespace Myriad

#endif /* OBJECTBUILDER_H_ */
