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

#ifndef CONDITIONALRANDOMIZEDHYDRATOR_H_
#define CONDITIONALRANDOMIZEDHYDRATOR_H_

#include "hydrator/RandomRecordHydrator.h"
#include "reflection/getter/ValueGetter.h"

using namespace std;
using namespace Poco;

namespace Myriad {

template<class RecordType, typename T, typename C, typename P> class ConditionalRandomizedHydrator : public RandomRecordHydrator<RecordType>
{
public:

	typedef void (RecordType::*ValueSetter)(const T&);

	ConditionalRandomizedHydrator(RandomStream& random, ValueSetter setter, ValueGetter<RecordType, C>* getter, const P& probability) :
		RandomRecordHydrator<RecordType>(random, 1),
		_setter(setter),
		_getter(getter),
		_probability(probability)
	{
	}

	virtual ~ConditionalRandomizedHydrator()
	{
		delete _getter;
	}

	void operator()(AutoPtr<RecordType> recordPtr) const
	{
		RandomStream& random = const_cast<ConditionalRandomizedHydrator<RecordType, T, C, P>*>(this)->_random;
		const C& condition = (*_getter)(recordPtr);

        (recordPtr->*_setter)(static_cast<T>(_probability.sample(random(), condition)));
	}

private:

	ValueSetter _setter;

	ValueGetter<RecordType, C>* _getter;

	const P& _probability;
};


} // namespace Myriad

#endif /* CONDITIONALRANDOMIZEDHYDRATOR_H_ */
