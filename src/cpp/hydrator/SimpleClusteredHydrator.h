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

#ifndef SIMPLECLUSTEREDHYDRATOR_H_
#define SIMPLECLUSTEREDHYDRATOR_H_

#include "hydrator/RecordHydrator.h"

namespace Myriad {

template<class RecordType, typename T, typename P> class SimpleClusteredHydrator : public InvertibleHydrator<RecordType, T>
{
public:

	typedef void (RecordType::*ValueSetter)(const T&);

	SimpleClusteredHydrator(ValueSetter valueSetter, const P& probability, const I64u sequenceCardinality) :
		InvertibleHydrator<RecordType, T>(valueSetter, 0),
		_valueSetter(valueSetter),
		_probability(probability),
		_sequenceCardinality(sequenceCardinality),
		_sequenceCardinalityDecimal(sequenceCardinality)
	{
	}

	void operator()(AutoPtr<RecordType> recordPtr) const
	{
        (recordPtr->*_valueSetter)(static_cast<T>(_probability.sample((recordPtr->genID() % _sequenceCardinality)/_sequenceCardinalityDecimal)));
	}

	const Interval<I64u> operator()(const T& x) const
	{
		Decimal cdf = _probability.cdf(x);
		Decimal pdf = _probability.pdf(x);

		return Interval<I64u>((cdf-pdf) * _sequenceCardinalityDecimal + 0.5, cdf * _sequenceCardinalityDecimal + 0.5);
	}

private:

	ValueSetter _valueSetter;
	const P& _probability;
	const I64u _sequenceCardinality;
	const Decimal _sequenceCardinalityDecimal;
};

} // namespace Myriad

#endif /* SIMPLECLUSTEREDHYDRATOR_H_ */
