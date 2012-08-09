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

#ifndef REFERENCEDRECORDHYDRATOR_H_
#define REFERENCEDRECORDHYDRATOR_H_

#include "hydrator/RandomRecordHydrator.h"
#include "math/random/RandomStream.h"

namespace Myriad {

template<class RecordType, class RefRecordType, class T, class P> class ReferencedRecordHydrator : public RandomRecordHydrator<RecordType>
{
public:

	typedef void (RecordType::*RefRecordSetter)(const AutoPtr<RefRecordType>&);
	typedef void (RefRecordType::*PivotFieldSetter)(const T&);

	ReferencedRecordHydrator(RandomStream& random, RefRecordSetter referenceSetter, PivotFieldSetter pivotSetter, RandomSetInspector<RefRecordType> referenceSet, const P& probability) :
		RandomRecordHydrator<RecordType>(random, 2),
		_referenceSetter(referenceSetter),
		_referenceSet(referenceSet),
		_invertibleHydrator(_referenceSet.template invertableHydrator<T>(pivotSetter)),
		_probability(probability)
	{
	}

	void operator()(AutoPtr<RecordType> recordPtr) const
	{
		RandomStream& random = const_cast<ReferencedRecordHydrator<RecordType, RefRecordType, T, P>*>(this)->_random;

        // get the interval from the referenced PRDG sequence where [T = t]
        Interval<I64u> tRange = _invertibleHydrator(static_cast<T>(_probability.sample(random())));

        // hydrate a record with a random genID from the tRange interval
        const AutoPtr<RefRecordType> refRecordPtr = _referenceSet.at(random(tRange.min(), tRange.max()-1));

        (recordPtr->*_referenceSetter)(refRecordPtr);
	}

private:

	RefRecordSetter _referenceSetter;

	RandomSetInspector<RefRecordType> _referenceSet;

	const InvertibleHydrator<RefRecordType, T>& _invertibleHydrator;

	const P& _probability;
};

}  // namespace Myriad


#endif /* REFERENCEDRECORDHYDRATOR_H_ */
