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

#ifndef CLUSTERINTERVALHYDRATOR_H_
#define CLUSTERINTERVALHYDRATOR_H_

#include "hydrator/RecordHydrator.h"
#include "math/random/RandomStream.h"
#include "math/probability/CustomDiscreteProbability.h"

#include <vector>

using namespace std;
using namespace Poco;

namespace Myriad {

template<class RecordType, typename T> class ClusterIntervalHydrator : public RecordHydrator<RecordType>
{
public:

	typedef const T& (RecordType::*ValueGetter)() const;

	typedef void (RecordType::*IntervalSetter)(const Interval<I64u>&);

	ClusterIntervalHydrator(IntervalSetter intervalSetter, ValueGetter valueGetter, ID enumCardinality, ID seqCardinality) :
		RecordHydrator<RecordType>(),
		_intervalSetter(intervalSetter),
		_valueGetter(valueGetter),
		_uniform(true),
		_probability(CustomDiscreteProbability("rangesethydrator.dummy")),
		_intervalMap(new DiscreteMap<T, Interval<I64u> >("rangesethydrator.anonymous"))
	{
		Decimal totalProbability = 0;
		ID min, max = 0;
		for (unsigned int i = 0; i < enumCardinality; i++)
		{
			totalProbability = (i+1)/static_cast<Decimal>(enumCardinality);

			min = max;
			max = static_cast<ID> (totalProbability * seqCardinality + 0.5);

			_intervalMap->define(static_cast<T>(i), Interval<ID> (min, max));
		}
	}

	ClusterIntervalHydrator(IntervalSetter intervalSetter, ValueGetter valueGetter, const CustomDiscreteProbability& probability, ID seqCardinality) :
		_intervalSetter(intervalSetter),
		_valueGetter(valueGetter),
		_uniform(false),
		_probability(probability),
		_intervalMap(new DiscreteMap<T, Interval<I64u> >("rangesethydrator.anonymous"))
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

			_intervalMap->define(static_cast<T>(it->first), Interval<ID> (min, max));

			++it;
		}
	}

	~ClusterIntervalHydrator()
	{
		delete _intervalMap;
	}

	void operator()(AutoPtr<RecordType> recordPtr) const;

private:

	IntervalSetter _intervalSetter;

	ValueGetter _valueGetter;

	const bool _uniform;

	const CustomDiscreteProbability& _probability;

	DiscreteMap<T, Interval<I64u> >* _intervalMap;
};

template<class RecordType, typename T> inline void ClusterIntervalHydrator<RecordType, T>::operator()(AutoPtr<RecordType> recordPtr) const
{
	if (RecordHydrator<RecordType>::_enabled)
	{
		const T& v = (recordPtr->*_valueGetter)();
		(recordPtr->*_intervalSetter)((*_intervalMap)(v));
	}
}

} // namespace Myriad

#endif /* CLUSTERINTERVALHYDRATOR_H_ */
