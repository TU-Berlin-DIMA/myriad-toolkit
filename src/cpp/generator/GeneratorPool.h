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

#ifndef GENERATORPOOL_H_
#define GENERATORPOOL_H_

#include "generator/RecordGenerator.h"

#include <list>
#include <Poco/AutoReleasePool.h>

using namespace std;
using namespace Poco;

namespace Myriad {

class GeneratorPool
{
public:

	GeneratorPool()
	{
	}

	template<class C> void set(C* generator);

	template<class C> C& get() const;

	list<RecordGenerator*>& getAll();

private:

	AutoReleasePool<RecordGenerator> autoReleasePool;
	list<RecordGenerator*> generators;
};

template<class C> void GeneratorPool::set(C* generator)
{
	generators.push_back(generator);
	autoReleasePool.add(generator);
}

template<class C> C& GeneratorPool::get() const
{
	for (list<RecordGenerator*>::const_iterator it = generators.begin(); it != generators.end(); ++it)
	{
		const RecordGenerator* pSS(*it);
		const C* pC = dynamic_cast<const C*>(pSS);
		if (pC) return *const_cast<C*>(pC);
	}

	throw LogicException("Trying to access unsupported generator type");
}

inline list<RecordGenerator*>& GeneratorPool::getAll()
{
	return generators;
}

} // namespace Myriad

#endif /* GENERATORPOOL_H_ */
