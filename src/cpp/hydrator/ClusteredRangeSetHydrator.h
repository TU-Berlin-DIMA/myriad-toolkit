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

#ifndef CLUSTEREDRANGESETHYDRATOR_H_
#define CLUSTEREDRANGESETHYDRATOR_H_

#include "hydrator/RecordHydrator.h"
#include "math/random/RandomStream.h"
#include "math/probability/CustomDiscreteProbability.h"

#include <vector>

using namespace std;
using namespace Poco;

namespace Myriad {

template<class RecordType, typename T> class ClusteredRangeSetHydrator : public RecordHydrator<RecordType>
{
public:

	typedef void (RecordType::*ValueSetter)(const T&);

	typedef void (RecordType::*IntervalSetter)(const Interval<ID>&);

	ClusteredRangeSetHydrator(ValueSetter valueSetter, IntervalSetter intervalSetter, const vector<T>& range, ID seqCardinality) :
		RecordHydrator<RecordType>(),
		_valueSetter(valueSetter),
		_intervalSetter(intervalSetter),
		_range(range),
		_uniform(true),
		_probability(CustomDiscreteProbability("rangesethydrator.dummy")),
		_intervalMap(new IntervalMap<ID, T>("rangesethydrator.anonymous")),
		_inverseMap(new DiscreteMap<T, Interval<ID> >("rangesethydrator.anonymous"))
	{
		Decimal totalProbability = 0;
		ID min, max = 0;
		ID enumCardinality = range.size();
		for (unsigned int i = 0; i < enumCardinality; i++)
		{
			totalProbability = (i+1)/static_cast<Decimal>(enumCardinality);

			min = max;
			max = static_cast<ID> (totalProbability * seqCardinality + 0.5);

			_intervalMap->define(Interval<ID> (min, max), _range[i]);
			_inverseMap->define(_range[i], Interval<ID> (min, max));
		}
	}

	ClusteredRangeSetHydrator(ValueSetter valueSetter, IntervalSetter intervalSetter, const vector<T>& range, const CustomDiscreteProbability& probability, ID seqCardinality) :
		_valueSetter(valueSetter),
		_intervalSetter(intervalSetter),
		_range(range),
		_uniform(false),
		_probability(probability),
		_intervalMap(new IntervalMap<ID, T>("rangesethydrator.anonymous")),
		_inverseMap(new DiscreteMap<T, Interval<ID> >("rangesethydrator.anonymous"))
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

			_intervalMap->define(Interval<ID> (min, max), _range[it->first]);
			_inverseMap->define(_range[it->first], Interval<ID> (min, max));

			++it;
		}
	}

	~ClusteredRangeSetHydrator()
	{
		delete _intervalMap;
		delete _inverseMap;
	}

	void operator()(AutoPtr<RecordType> recordPtr) const;

	Interval<ID> sampleInterval(RandomStream& random) const;

	Interval<ID> fetchInterval(const T& v) const;

private:

	ValueSetter _valueSetter;

	IntervalSetter _intervalSetter;

	const vector<T>& _range;

	const bool _uniform;

	const CustomDiscreteProbability& _probability;

	IntervalMap<ID, T>* _intervalMap;

	DiscreteMap<T, Interval<ID> >* _inverseMap;
};

template<class RecordType, typename T> inline void ClusteredRangeSetHydrator<RecordType, T>::operator()(AutoPtr<RecordType> recordPtr) const
{
	if (RecordHydrator<RecordType>::_enabled)
	{
		(recordPtr->*_valueSetter)(static_cast<T>((*_intervalMap)(recordPtr->genID())));
		(recordPtr->*_intervalSetter)(_intervalMap->lastAccessedDomainValue());
	}
}

template<class RecordType, typename T> inline Interval<ID> ClusteredRangeSetHydrator<RecordType, T>::sampleInterval(RandomStream& random) const
{
	return (*_inverseMap)(_range[random(0, _range.size()-1)]);
}

template<class RecordType, typename T> inline Interval<ID> ClusteredRangeSetHydrator<RecordType, T>::fetchInterval(const T& v) const
{
	return (*_inverseMap)(v);
}

} // namespace Myriad

#endif /* CLUSTEREDRANGESETHYDRATOR_H_ */
