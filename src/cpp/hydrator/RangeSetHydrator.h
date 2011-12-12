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

#ifndef RANGESETHYDRATOR_H_
#define RANGESETHYDRATOR_H_

#include "hydrator/RandomRecordHydrator.h"
#include "math/random/RandomStream.h"
#include "math/probability/CustomDiscreteProbability.h"

#include <vector>

using namespace std;
using namespace Poco;

namespace Myriad {

template<class RecordType, typename T> class RangeSetHydrator : public RandomRecordHydrator<RecordType>
{
public:

	typedef void (RecordType::*ValueSetter)(const T&);

	RangeSetHydrator(RandomStream& random, ValueSetter setter, const vector<T>& range) :
		RandomRecordHydrator<RecordType>(random),
		_setter(setter),
		_range(range),
		_rangeCardinality(range.size()),
		_uniform(true),
		_probability(CustomDiscreteProbability("rangesethydrator.dummy"))
	{
	}

	RangeSetHydrator(RandomStream& random, ValueSetter setter, const vector<T>& range, const CustomDiscreteProbability& probability) :
		RandomRecordHydrator<RecordType>(random),
		_setter(setter),
		_range(range),
		_rangeCardinality(range.size()),
		_uniform(false),
		_probability(probability)
	{
	}

	void operator()(AutoPtr<RecordType> recordPtr) const
	{
		RandomStream& random = const_cast<RangeSetHydrator<RecordType, T>*>(this)->_random;

		if (!RecordHydrator<RecordType>::_enabled)
		{
			// TODO: consume a single random number
		}
		else
		{
			if (_uniform)
			{
				(recordPtr->*_setter)(_range[random(0, _rangeCardinality-1)]);
			}
			else
			{
				(recordPtr->*_setter)(_range[_probability.sample(random())]);
			}
		}
	}

private:

	ValueSetter _setter;

	const vector<T>& _range;

	const typename vector<T>::size_type _rangeCardinality;

	const bool _uniform;

	const CustomDiscreteProbability& _probability;
};


} // namespace Myriad

#endif /* RANGESETHYDRATOR_H_ */
