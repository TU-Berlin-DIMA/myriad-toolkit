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

#ifndef CONSTRANGEPROVIDER_H_
#define CONSTRANGEPROVIDER_H_

#include "runtime/provider/range/RangeProvider.h"

using namespace Poco;

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// range provider for constant ranges
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename RangeType, class CxtRecordType>
class ConstRangeProvider: public RangeProvider<RangeType, CxtRecordType>
{
public:

    ConstRangeProvider(const Interval<RangeType>& constRange) :
        RangeProvider<RangeType, CxtRecordType>(0),
        _constRange(constRange)
    {
    }

    ConstRangeProvider(const RangeType& min, const RangeType& max) :
        RangeProvider<RangeType, CxtRecordType>(0),
        _constRange(min, max)
    {
    }

    virtual ~ConstRangeProvider()
    {
    }

    virtual const Interval<RangeType> operator()(const AutoPtr<CxtRecordType>& ctxRecordPtr, RandomStream& random)
    {
        return _constRange;
    }

private:

    const Interval<RangeType> _constRange;
};

} // namespace Myriad

#endif /* CONSTRANGEPROVIDER_H_ */
