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

#ifndef NESTEDCLUSTEREDENUMSETHYDRATOR_H_
#define NESTEDCLUSTEREDENUMSETHYDRATOR_H_

#include "hydrator/RecordHydrator.h"
#include "math/random/RandomStream.h"
#include "math/probability/CustomDiscreteProbability.h"

#include <vector>

using namespace std;
using namespace Poco;

namespace Myriad {

template<class RecordType, typename T> class NestedClusteredEnumSetHydrator : public RecordHydrator<RecordType>
{
public:

	typedef void (RecordType::*ValueSetter)(const T&);
	typedef void (RecordType::*IntervalSetter)(const Interval<ID>&);
	typedef const Interval<ID>& (RecordType::*IntervalGetter)() const;

	NestedClusteredEnumSetHydrator(ValueSetter valueSetter, IntervalSetter intervalSetter, IntervalSetter parentIntervalGetter, ID enumCardinality) :
		RecordHydrator<RecordType>(),
		_valueSetter(valueSetter),
		_intervalSetter(intervalSetter),
		_parentIntervalGetter(parentIntervalGetter),
		_uniform(true),
		_cardinality(enumCardinality),
		_probability(CustomDiscreteProbability("rangesethydrator.dummy")),
		_nestedIntervalMap(new IntervalMap<ID, T>("rangesethydrator.anonymous")),
		_currentParentInterval(0, 0)
	{
	}

	NestedClusteredEnumSetHydrator(ValueSetter valueSetter, IntervalSetter intervalSetter, IntervalGetter parentIntervalGetter, const CustomDiscreteProbability& probability) :
		_valueSetter(valueSetter),
		_intervalSetter(intervalSetter),
		_parentIntervalGetter(parentIntervalGetter),
		_uniform(false),
		_cardinality(probability.domainCardinality()),
		_probability(probability),
		_nestedIntervalMap(new IntervalMap<ID, T>("rangesethydrator.anonymous")),
		_currentParentInterval(0, 0)
	{
	}

	~NestedClusteredEnumSetHydrator()
	{
		delete _nestedIntervalMap;
	}

	void operator()(AutoPtr<RecordType> recordPtr) const;

private:

	void updateNestedIntervalMap()
	{
		ID offset = _currentParentInterval.min();
		ID min, max = offset;
		ID parentSeqCardinality = _currentParentInterval.length();
		Decimal totalProbability = 0;

		if (_uniform)
		{
			for (unsigned int i = 0; i < _cardinality; i++)
			{
				totalProbability = (i+1)/static_cast<Decimal>(_cardinality);

				min = max;
				max = offset + static_cast<ID> (totalProbability * parentSeqCardinality + 0.5);

				_nestedIntervalMap->define(Interval<ID> (min, max), static_cast<T>(i));
			}
		}
		else
		{
			map<ID, Decimal>::const_iterator end = _probability.mapping().end();
			for (map<ID, Decimal>::const_iterator it = _probability.mapping().begin(); it != end; ++it)
			{
				totalProbability += it->second;

				min = max;
				max = offset + static_cast<ID> (totalProbability * parentSeqCardinality + 0.5);

				_nestedIntervalMap->define(Interval<ID> (min, max), static_cast<T>(it->first));
			}
		}
	}

	ValueSetter _valueSetter;

	IntervalSetter _intervalSetter;

	IntervalGetter _parentIntervalGetter;

	const bool _uniform;

	I32u _cardinality;

	const CustomDiscreteProbability& _probability;

	IntervalMap<ID, T>* _nestedIntervalMap;

	Interval<ID> _currentParentInterval;
};

template<class RecordType, typename T> inline void NestedClusteredEnumSetHydrator<RecordType, T>::operator()(AutoPtr<RecordType> recordPtr) const
{
	if (RecordHydrator<RecordType>::_enabled)
	{
		const Interval<ID>& parentInterval = (recordPtr->*_parentIntervalGetter)();

		if (parentInterval != _currentParentInterval)
		{
			const_cast<NestedClusteredEnumSetHydrator<RecordType, T>*>(this)->_currentParentInterval = parentInterval;
			const_cast<NestedClusteredEnumSetHydrator<RecordType, T>*>(this)->updateNestedIntervalMap();
		}

		(recordPtr->*_valueSetter)(static_cast<T>((*_nestedIntervalMap)(recordPtr->genID())));
		(recordPtr->*_intervalSetter)(_nestedIntervalMap->lastAccessedDomainValue());
	}
}

} // namespace Myriad

#endif /* NESTEDCLUSTEREDENUMSETHYDRATOR_H_ */
