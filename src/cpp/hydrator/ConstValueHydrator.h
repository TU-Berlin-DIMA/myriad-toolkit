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

#ifndef CONSTVALUEHYDRATOR_H_
#define CONSTVALUEHYDRATOR_H_

#include "hydrator/RecordHydrator.h"
#include "math/random/RandomStream.h"
#include "math/probability/CustomDiscreteProbability.h"

#include <vector>

using namespace std;
using namespace Poco;

namespace Myriad {

template<class RecordType, typename T> class ConstValueHydrator: public RecordHydrator<RecordType>
{
public:

	typedef void (RecordType::*ValueSetter)(const T&);

	ConstValueHydrator(ValueSetter setter, const T& constValue) :
		_setter(setter),
		_constValue(constValue)
	{
	}

	void operator()(AutoPtr<RecordType> recordPtr) const
	{
		if (RecordHydrator<RecordType>::_enabled)
		{
			(recordPtr->*_setter)(_constValue);
		}
	}

private:

	ValueSetter _setter;

	const T _constValue;
};

} // namespace Myriad

#endif /* CONSTVALUEHYDRATOR_H_ */
