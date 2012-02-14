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

#ifndef CONDITIONALHYDRATOR_H_
#define CONDITIONALHYDRATOR_H_

#include "hydrator/RecordHydrator.h"
#include "math/random/RandomStream.h"
#include "math/probability/CustomDiscreteProbability.h"

#include <vector>

using namespace std;
using namespace Poco;

namespace Myriad {

template<class RecordType, typename T, typename H1, typename H2> class ConditionalHydrator: public RecordHydrator<RecordType>
{
public:

	typedef const T& (RecordType::*ValueGetter)() const;

	ConditionalHydrator(ValueGetter getter, const T& compareValue, const H1& hydratorTrue, const H2& hydratorFalse) :
		_getter(getter),
		_compareValue(compareValue),
		_hydratorTrue(hydratorTrue),
		_hydratorFalse(hydratorFalse)
	{
	}

	void operator()(AutoPtr<RecordType> recordPtr) const
	{
		if ((recordPtr->*_getter)() == _compareValue)
		{
			_hydratorTrue(recordPtr);
		}
		else
		{
			_hydratorFalse(recordPtr);
		}
	}

private:

	ValueGetter _getter;

	const T _compareValue;

	const H1& _hydratorTrue;

	const H2& _hydratorFalse;
};

} // namespace Myriad

#endif /* CONDITIONALHYDRATOR_H_ */
