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

#ifndef MULTIPLICATIVEGROUPHYDRATOR_H_
#define MULTIPLICATIVEGROUPHYDRATOR_H_

#include "hydrator/RecordHydrator.h"
#include "math/random/RandomStream.h"
#include "math/random/SurrogateKeyGenerator.h"
#include "math/probability/CustomDiscreteProbability.h"

#include <vector>
#include <iterator>

using namespace std;
using namespace Poco;

namespace Myriad {

/**
 * A hydrator component for for multiplicative integer groups (typically used
 * as a randomization key for clustered generation sequences).
 */
template<class RecordType> class MultiplicativeGroupHydrator : public RecordHydrator<RecordType>
{
public:

	typedef void (RecordType::*ValueSetter)(const ID&);

	MultiplicativeGroupHydrator(ValueSetter valueSetter, ID cardinality, ID beginID, ID endID) :
		_valueSetter(valueSetter),
		_G(cardinality),
		_iterG(_G[beginID]),
		_beginID(beginID),
		_currentID(beginID),
		_endID(beginID)
	{
	}

	void operator()(AutoPtr<RecordType> recordPtr) const
	{
		if (RecordHydrator<RecordType>::_enabled)
		{
			(recordPtr->*_valueSetter)(static_cast<ID>(const_cast<MultiplicativeGroupHydrator<RecordType>*>(this)->_iterG++));
			const_cast<MultiplicativeGroupHydrator<RecordType>*>(this)->_currentID++;
		}
	}

	ID operator()(const ID& genID) const
	{
		return _G(genID);
	}

private:

	ValueSetter _valueSetter;

	SurrogateKeyGenerator _G;
	SurrogateKeyGenerator::iterator _iterG;

	ID _beginID;
	ID _currentID;
	ID _endID;
};

} // namespace Myriad

#endif /* MULTIPLICATIVEGROUPHYDRATOR_H_ */
