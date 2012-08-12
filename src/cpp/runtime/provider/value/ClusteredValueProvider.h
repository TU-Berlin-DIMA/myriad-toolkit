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

#ifndef CLUSTEREDVALUEPROVIDER_H_
#define CLUSTEREDVALUEPROVIDER_H_

#include "runtime/provider/value/AbstractValueProvider.h"

using namespace Poco;

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider for clustered values
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType, class PrFunctionType, class RangeProviderType>
class ClusteredValueProvider: public AbstractValueProvider<ValueType, CxtRecordType>
{
public:

    ClusteredValueProvider(const PrFunctionType& prFunction, RangeProviderType& rangeProvider) :
        AbstractValueProvider<ValueType, CxtRecordType>(rangeProvider.arity()+1, true), // TODO: is this always invertible?
        _prFunction(prFunction),
        _rangeProvider(rangeProvider)
    {
    }

    virtual ~ClusteredValueProvider()
    {
    }

    virtual Interval<I64u> valueRange(const ValueType& value, const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        Interval<I64u> currentRange = _rangeProvider(cxtRecordPtr, random);
        Decimal currentRangeLength = currentRange.length();

        Decimal cdf = _prFunction.cdf(value);
        Decimal pdf = _prFunction.pdf(value);

        return Interval<I64u>(currentRange.min() + (cdf-pdf) * currentRangeLength + 0.5, currentRange.min() + cdf * currentRangeLength + 0.5);
    }

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        Interval<I64u> currentRange = _rangeProvider(cxtRecordPtr, random);
        I64u currentRangeLength = currentRange.length();
        Decimal currentRangeLengthDecimal = currentRange.length();

        return static_cast<ValueType>(_prFunction.sample(((cxtRecordPtr->genID() - currentRange.min()) % currentRangeLength)/currentRangeLengthDecimal));
    }

private:

    const PrFunctionType& _prFunction;

    RangeProviderType& _rangeProvider;
};

} // namespace Myriad

#endif /* CLUSTEREDVALUEPROVIDER_H_ */
