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

#ifndef STATICSETREFERENCEHYDRATOR_H_
#define STATICSETREFERENCEHYDRATOR_H_

#include "hydrator/RandomRecordHydrator.h"
#include "math/random/RandomStream.h"
#include "math/probability/CustomDiscreteProbability.h"

#include <vector>

using namespace std;
using namespace Poco;

namespace Myriad {

template<class RecordType, typename RefType> class StaticSetReferenceHydrator : public RecordHydrator<RecordType>
{
public:

	typedef const ID& (RecordType::*RefGenIDGetter)() const;
	typedef void (RecordType::*RefSetter)(const RefType&);

	StaticSetReferenceHydrator(RefSetter setter, RefGenIDGetter getter, const vector<AutoPtr<RefType> >& referenceSet) :
		RecordHydrator<RecordType>(),
		_referenceSetter(setter),
		_referenceGenIDGetter(getter),
		_referenceSet(referenceSet)
	{
	}

	void operator()(AutoPtr<RecordType> recordPtr) const
	{
		if (RecordHydrator<RecordType>::_enabled)
		{
			AutoPtr<RefType> reference  = _referenceSet[(recordPtr->*_referenceGenIDGetter)()];

			(recordPtr->*_referenceSetter)(*reference);
		}
	}

private:

	RefSetter _referenceSetter;

	RefGenIDGetter _referenceGenIDGetter;

	const vector<AutoPtr<RefType> >& _referenceSet;
};


} // namespace Myriad

#endif /* STATICSETREFERENCEHYDRATOR_H_ */
