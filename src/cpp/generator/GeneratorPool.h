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

#ifndef GENERATORPOOL_H_
#define GENERATORPOOL_H_

#include "generator/AbstractSequenceGenerator.h"

#include <list>
#include <Poco/AutoReleasePool.h>

using namespace std;
using namespace Poco;

namespace Myriad {
/**
 * @addtogroup generator
 * @{*/

/**
 * A container for dynamically allocated AbstractSequenceGenerator objects.
 *
 * AbstractSequenceGenerator instances registered with a GeneratorPool are
 * managed by this pool and are deleted when the pool itsel is deleted. Exactly
 * one \p generator is allowed to be added for each concrete \p GeneratorType.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class GeneratorPool
{
public:

    /**
     * Constructor.
     */
    GeneratorPool()
    {
    }

    /**
     * Add a new generator pointer to the pool.
     *
     * @param generator A pointer to the generator to be added.
     * @throw ConfigException if a generator of the provided \p FunctionType is
     *                        already registered with this pool
     */
    template<class GeneratorType> void set(GeneratorType* generator)
    {
        for (list<AbstractSequenceGenerator*>::const_iterator it = generators.begin(); it != generators.end(); ++it)
        {
	        const AbstractSequenceGenerator* pSS(*it);
	        const GeneratorType* pC = dynamic_cast<const GeneratorType*>(pSS);
	        if (pC) throw LogicException("A generator of the given concrete type is already registered with this GeneratorPool.");
        }

        generators.push_back(generator);
        autoReleasePool.add(generator);
    }

    /**
     * Get a type-safe version of an already registered generator identified by
     * the given name.
     *
     * @return A reference of the \p generator of the given \p GeneratorType.
     * @throw ConfigException if the function identified by the given
     *                        \p GeneratorType is not registered
     */
    template<class GeneratorType> GeneratorType& get() const
    {
        for (list<AbstractSequenceGenerator*>::const_iterator it = generators.begin(); it != generators.end(); ++it)
        {
	        const AbstractSequenceGenerator* pSS(*it);
	        const GeneratorType* pC = dynamic_cast<const GeneratorType*>(pSS);
	        if (pC) return *const_cast<GeneratorType*>(pC);
        }

        throw LogicException("Trying to access unsupported generator type");
    }


    /**
     * Returns a reference to a list containing all generators.
     */
    list<AbstractSequenceGenerator*>& getAll()
    {
        return generators;
    }

private:

    /**
     * An autorelease pool for all registered generators.
     */
    AutoReleasePool<AbstractSequenceGenerator> autoReleasePool;

    /**
     * A list of registered generators.
     */
    list<AbstractSequenceGenerator*> generators;
};

/** @}*/// add to generator group
} // namespace Myriad

#endif /* GENERATORPOOL_H_ */
