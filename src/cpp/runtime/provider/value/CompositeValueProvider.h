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

#ifndef COMPOSITEVALUEPROVIDER_H_
#define COMPOSITEVALUEPROVIDER_H_

#include "runtime/provider/value/AbstractValueProvider.h"

using namespace Poco;

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider for clustered values
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType, class PrFunctionType, class RangeProviderType>
class CompositeValueProvider: public AbstractValueProvider<ValueType, CxtRecordType>
{
public:

    CompositeValueProvider(const PrFunctionType& prFunction, RangeProviderType& rangeProvider) :
        AbstractValueProvider<ValueType, CxtRecordType>(rangeProvider.arity()+1, true), // TODO: is this always invertible?
        _prFunction(prFunction),
        _rangeProvider(rangeProvider)
    {
    }

    virtual ~CompositeValueProvider()
    {
    }

    virtual Interval<I64u> valueRange(const ValueType& value, const AutoPtr<CxtRecordType>& cxtRecordPtr)
    {
    	throw RuntimeException("valueRange not supported");
    }

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        return static_cast<ValueType>(_prFunction.sample(cxtRecordPtr->genID(), _rangeProvider(cxtRecordPtr).length()));
    }

private:

    const PrFunctionType& _prFunction;

    RangeProviderType& _rangeProvider;
};

} // namespace Myriad

#endif /* COMPOSITEVALUEPROVIDER_H_ */
