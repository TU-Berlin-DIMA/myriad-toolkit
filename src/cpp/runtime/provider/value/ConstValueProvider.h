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

#ifndef CONSTVALUEPROVIDER_H_
#define CONSTVALUEPROVIDER_H_

#include "runtime/provider/value/AbstractValueProvider.h"

using namespace Poco;

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider for constant values
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType>
class ConstValueProvider: public AbstractValueProvider<ValueType, CxtRecordType>
{
public:

    ConstValueProvider(const ValueType& constValue) :
    	AbstractValueProvider<ValueType, CxtRecordType>(0, true),
        _constValue(constValue)
    {
    }

    virtual ~ConstValueProvider()
    {
    }

    virtual Interval<I64u> valueRange(const ValueType& value, const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
	{
    	if (_constValue == value)
    	{
			// value range is the whole cxtRecordPtr.genID() interval
			return Interval<I64u>(0, cxtRecordPtr->meta().cardinality());
    	}
    	else
    	{
			// value range is zero
			return Interval<I64u>(0, 0);
    	}
	}

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        return _constValue;
    }

private:

    const ValueType _constValue;
};

} // namespace Myriad

#endif /* CONSTVALUEPROVIDER_H_ */
