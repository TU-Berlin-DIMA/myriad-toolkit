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

#ifndef CLUSTEREDSELFREFERENCEHYDRATOR_H_
#define CLUSTEREDSELFREFERENCEHYDRATOR_H_

#include "hydrator/RandomRecordHydrator.h"
#include "math/random/RandomStream.h"

using namespace std;
using namespace Poco;

namespace Myriad {

template<class RecordType, class RefKeyHydrator, typename P> class ClusteredSelfReferenceHydrator: public RandomRecordHydrator<RecordType>
{
public:

	typedef void (RecordType::*ValueSetter)(const ID&);
	typedef const I32u& (RecordType::*CardinalityGetter)() const;
	typedef const Interval<ID>& (RecordType::*SamplingRangeGetter)() const;

	ClusteredSelfReferenceHydrator(RandomStream& random, ValueSetter setter, CardinalityGetter cardinalityGetter, SamplingRangeGetter rangeGetter, RefKeyHydrator& hydrateKey, P& probability) :
		RandomRecordHydrator<RecordType>(random),
		_setter(setter),
		_cardinalityGetter(cardinalityGetter),
		_rangeGetter(rangeGetter),
		_hydrateKey(hydrateKey),
		_probability(probability)
	{
		Interval<Decimal> treshold = _probability.threshold(0.98);
		_normalizationFactor = 1 / (treshold.max() - treshold.min());
	}

	void operator()(AutoPtr<RecordType> recordPtr) const;

private:

	ValueSetter _setter;

	CardinalityGetter _cardinalityGetter;

	SamplingRangeGetter _rangeGetter;

	RefKeyHydrator& _hydrateKey;

	P& _probability;

	double _normalizationFactor;
};

template<class RecordType, class RefKeyHydrator, typename P> inline void ClusteredSelfReferenceHydrator<RecordType, RefKeyHydrator, P>::operator()(AutoPtr<RecordType> recordPtr) const
{
	RandomStream& random = const_cast<ClusteredSelfReferenceHydrator<RecordType, RefKeyHydrator, P>*>(this)->_random;

	if (RecordHydrator<RecordType>::_enabled)
	{
		const I32u referencesCardinality = (recordPtr->*_cardinalityGetter)();

		const Interval<ID>& referencesRange = (recordPtr->*_rangeGetter)();
		const ID length = referencesRange.length();

		for (I32u i = 0; i < referencesCardinality; i++)
		{
			ID referenceGenID = referencesRange.min() + (static_cast<ID> (length * _normalizationFactor * _probability.sample(random())) % length);
			(recordPtr->*_setter)(_hydrateKey(referenceGenID));
		}
	}
	else
	{
		// TODO: consume a single random number
	}
}

} // namespace Myriad

#endif /* CLUSTEREDSELFREFERENCEHYDRATOR_H_ */
