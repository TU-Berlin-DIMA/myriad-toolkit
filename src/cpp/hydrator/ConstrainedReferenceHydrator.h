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

#ifndef CONSTRAINEDREFERENCEHYDRATOR_H_
#define CONSTRAINEDREFERENCEHYDRATOR_H_

#include "hydrator/RandomRecordHydrator.h"
#include "math/random/RandomStream.h"

using namespace std;
using namespace Poco;

namespace Myriad {

template<class RecordType, class RefRecordType, typename P> class ConstrainedReferenceHydrator: public RandomRecordHydrator<RecordType>
{
public:

	typedef void (RecordType::*ValueSetter)(const AutoPtr<RefRecordType>&);
	typedef const Interval<I64u>& (RecordType::*SamplingRangeGetter)() const;

	ConstrainedReferenceHydrator(RandomStream& random, ValueSetter setter, SamplingRangeGetter rangeGetter, RandomSetGenerator<RefRecordType>& refGenerator, P& probability) :
		RandomRecordHydrator<RecordType>(random),
		_delta(0.0001),
		_totalProbability(1.0 - 2*_delta),
		_xOffset(probability.invcdf(_delta)),
		_setter(setter),
		_rangeGetter(rangeGetter),
		_referenceInspector(refGenerator.inspector()),
		_probability(probability),
		_logger(Logger::get("hydrator.constrained_reference"))
	{
		Interval<Decimal> threshold(_probability.invcdf(_delta), _probability.invcdf(1.0 - _delta));
		_normalizationFactor = 1.0 / threshold.length();
	}

	void operator()(AutoPtr<RecordType> recordPtr) const;

private:

	double _delta;
	double _totalProbability;
	double _xOffset;

	ValueSetter _setter;

	SamplingRangeGetter _rangeGetter;

	RandomSetInspector<RefRecordType> _referenceInspector;

	P& _probability;

	double _normalizationFactor;

	Logger& _logger;
};

template<class RecordType, class RefRecordType, typename P> inline void ConstrainedReferenceHydrator<RecordType, RefRecordType, P>::operator()(AutoPtr<RecordType> recordPtr) const
{
	RandomStream& random = const_cast<ConstrainedReferenceHydrator<RecordType, RefRecordType, P>*>(this)->_random;

	if (RecordHydrator<RecordType>::_enabled)
	{
		const Interval<ID>& referencesRange = (recordPtr->*_rangeGetter)();
		const ID length = referencesRange.length();

		I64u referenceGenID = referencesRange.min() + (static_cast<I64u> (length * _normalizationFactor * (_probability.sample(_delta + _totalProbability * random()) - _xOffset)));

//		_logger.information(format("range is [%Lu, %Lu), len = %Lu, sample is %Lu", referencesRange.min(), referencesRange.max(), length, referenceGenID));

		if (referenceGenID < referencesRange.min())
		{
			_logger.warning(format("range sampling underflow %Lu < %Lu", referenceGenID, referencesRange.min()));
			referenceGenID = referencesRange.min();
		}

		if (referenceGenID >= referencesRange.max())
		{
			_logger.warning(format("range sampling overflow %Lu >= %Lu", referenceGenID, referencesRange.max()));
			referenceGenID = referencesRange.max()-1;
		}

		(recordPtr->*_setter)(const_cast<RandomSetInspector<RefRecordType>&>(_referenceInspector).at(referenceGenID));
	}
	else
	{
		// TODO: consume a single random number
	}
}

} // namespace Myriad

#endif /* CONSTRAINEDSELFREFERENCEHYDRATOR_H_ */
