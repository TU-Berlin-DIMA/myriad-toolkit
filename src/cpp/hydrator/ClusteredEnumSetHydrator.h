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

#ifndef CLUSTEREDENUMSETHYDRATOR_H_
#define CLUSTEREDENUMSETHYDRATOR_H_

#include "hydrator/RecordHydrator.h"
#include "math/random/RandomStream.h"
#include "math/probability/CustomDiscreteProbability.h"

#include <vector>

using namespace std;
using namespace Poco;

namespace Myriad {

template<class RecordType, typename T> class ClusteredEnumSetHydrator : public RecordHydrator<RecordType>
{
public:

	typedef void (RecordType::*ValueSetter)(const T&);

	typedef void (RecordType::*IntervalSetter)(const Interval<ID>&);

	ClusteredEnumSetHydrator(ValueSetter valueSetter, IntervalSetter intervalSetter, ID enumCardinality, ID seqCardinality) :
		RecordHydrator<RecordType>(),
		_valueSetter(valueSetter),
		_intervalSetter(intervalSetter),
		_uniform(true),
		_probability(CustomDiscreteProbability("rangesethydrator.dummy")),
		_intervalMap(new IntervalMap<ID, T>("rangesethydrator.anonymous"))
	{
		Decimal totalProbability = 0;
		ID min, max = 0;
		for (unsigned int i = 0; i < enumCardinality; i++)
		{
			totalProbability = (i+1)/static_cast<Decimal>(enumCardinality);

			min = max;
			max = static_cast<ID> (totalProbability * seqCardinality + 0.5);

			_intervalMap->define(Interval<ID> (min, max), static_cast<T>(i));
		}
	}

	ClusteredEnumSetHydrator(ValueSetter valueSetter, IntervalSetter intervalSetter, const CustomDiscreteProbability& probability, ID seqCardinality) :
		_valueSetter(valueSetter),
		_intervalSetter(intervalSetter),
		_uniform(false),
		_probability(probability),
		_intervalMap(new IntervalMap<ID, T>("rangesethydrator.anonymous"))
	{
		map<ID, Decimal>::const_iterator it = _probability.mapping().begin();
		map<ID, Decimal>::const_iterator end = _probability.mapping().end();

		Decimal totalProbability = 0;
		ID min, max = 0;
		while (it != end)
		{
			totalProbability += it->second;

			min = max;
			max = static_cast<ID> (totalProbability * seqCardinality + 0.5);

			_intervalMap->define(Interval<ID> (min, max), static_cast<T>(it->first));

			++it;
		}
	}

	~ClusteredEnumSetHydrator()
	{
		delete _intervalMap;
	}

	void operator()(AutoPtr<RecordType> recordPtr) const;

private:

	ValueSetter _valueSetter;

	IntervalSetter _intervalSetter;

	const bool _uniform;

	const CustomDiscreteProbability& _probability;

	IntervalMap<ID, T>* _intervalMap;
};

template<class RecordType, typename T> inline void ClusteredEnumSetHydrator<RecordType, T>::operator()(AutoPtr<RecordType> recordPtr) const
{
	if (RecordHydrator<RecordType>::_enabled)
	{
		(recordPtr->*_valueSetter)(static_cast<T>((*_intervalMap)(recordPtr->genID())));
		(recordPtr->*_intervalSetter)(_intervalMap->lastAccessedDomainValue());
	}
}

} // namespace Myriad

#endif /* CLUSTEREDENUMSETHYDRATOR_H_ */
