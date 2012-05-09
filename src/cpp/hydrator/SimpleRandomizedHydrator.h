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

#ifndef SIMPLERANDOMIZEDHYDRATOR_H_
#define SIMPLERANDOMIZEDHYDRATOR_H_

#include "hydrator/RandomRecordHydrator.h"
#include "math/random/RandomStream.h"

namespace Myriad {

template<class RecordType, typename T, typename P> class SimpleRandomizedHydrator : public RandomRecordHydrator<RecordType>
{
public:

	typedef void (RecordType::*ValueSetter)(const T&);

	SimpleRandomizedHydrator(RandomStream& random, ValueSetter setter, const P& probability) :
		RandomRecordHydrator<RecordType>(random),
		_setter(setter),
		_probability(probability)
	{
	}

	void operator()(AutoPtr<RecordType> recordPtr) const
	{
		RandomStream& random = const_cast<SimpleRandomizedHydrator<RecordType, T, P>*>(this)->_random;

		if (!RecordHydrator<RecordType>::_enabled)
		{
			random();
		}
		else
		{
			(recordPtr->*_setter)(static_cast<T>(_probability.sample(random())));
		}
	}

private:

	ValueSetter _setter;

	const P& _probability;
};


} // namespace Myriad

#endif /* SIMPLERANDOMIZEDHYDRATOR_H_ */
