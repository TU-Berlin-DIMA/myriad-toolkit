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

#ifndef ENUMSETPOOL_H_
#define ENUMSETPOOL_H_

#include "core/exceptions.h"
#include "core/types/MyriadEnumSet.h"

#include <string>
#include <map>
#include <Poco/AutoPtr.h>
#include <Poco/AutoReleasePool.h>
#include <Poco/Exception.h>
#include <Poco/Logger.h>
#include <Poco/Path.h>
#include <Poco/Util/LayeredConfiguration.h>

using namespace std;
using namespace Poco;
using namespace Poco::Util;

namespace Myriad {
/**
 * @addtogroup config
 * @{*/

/**
 * A container for dynamically allocated EnumPool objects.
 *
 * All enums are registered with an autorelease pool that takes care of
 * reseource cleanup when the EnumPool object expires.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class EnumSetPool
{
public:

    /**
     * Default constructor.
     */
    EnumSetPool()
    {
    }

    /**
     * Add a new enum set pointer to the pool.
     *
     * @param enumPtr a pointer to the enum set to be added.
     * @throw ConfigException if the enum set is already registered
     */
    void add(MyriadEnumSet* enumSetPtr)
    {
        map<string, MyriadEnumSet*>::const_iterator it = _enumSets.find(enumSetPtr->name());

        if (it != _enumSets.end())
        {
	        throw ConfigException("Trying to add an already undefined enum set " + enumSetPtr->name());
        }

        _autoreleasePool.add(enumSetPtr);
        _enumSets[enumSetPtr->name()] = enumSetPtr;
    }

    /**
     * Retrieve the enum set identified by the given \p name.
     *
     * @return a reference to a \p MyriadEnum enum set identified by the
     *         given \p name
     * @throw ConfigException if the enum set identified by the given \name is
     *                        not registered or if it cannot be cast to the
     *                        provided \p FunctionType
     */
    const MyriadEnumSet& get(const string& name) const
    {
        map<string, MyriadEnumSet*>::const_iterator it = _enumSets.find(name);

        if (it == _enumSets.end())
        {
	        throw ConfigException("Trying to access undefined enum set " + name);
        }

        return *(it->second);
    }

private:

    /**
     * A container for the registered enum sets.
     */
    map<string, MyriadEnumSet*> _enumSets;

    /**
     * An auto-release pool for the registered enum sets.
     */
    AutoReleasePool<MyriadEnumSet> _autoreleasePool;
};

/** @}*/// add to core group
}  // namespace Myriad

#endif /* ENUMSETPOOL_H_ */
