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
 */

#ifndef CONSTRANGEPROVIDER_H_
#define CONSTRANGEPROVIDER_H_

#include "runtime/provider/range/AbstractRangeProvider.h"

using namespace Poco;

namespace Myriad {
/**
 * @addtogroup runtime_provider_range
 * @{*/

/**
 * Range provider for constant ranges.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<typename RangeType, class CxtRecordType>
class ConstRangeProvider: public AbstractRangeProvider<RangeType, CxtRecordType>
{
public:

    /**
     * Constructor.
     *
     * @param constRange The constant range to be returned always by this
     *        provider.
     */
    ConstRangeProvider(const Interval<RangeType>& constRange) :
        AbstractRangeProvider<RangeType, CxtRecordType>(0),
        _constRange(constRange)
    {
    }

    /**
     * Constructor.
     *
     * @param min The left bound of the range to be returned always by this
     *        provider.
     * @param max The right bound of the range to be returned always by this
     *        provider.
     */
    ConstRangeProvider(RangeType min, RangeType max) :
        AbstractRangeProvider<RangeType, CxtRecordType>(0),
        _constRange(min, max)
    {
    }

    /**
     * Destructor.
     */
    virtual ~ConstRangeProvider()
    {
    }

    /**
     * Functor method. Provides a constant interval of \p RangeType values
     * independent on given \p CxtRecordType object.
     *
     * @param cxtRecordPtr A context record for this range provider.
     */
    virtual const Interval<RangeType> operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr)
    {
        return _constRange;
    }

private:

    const Interval<RangeType> _constRange;
};

/** @}*/// add to runtime_provider_range group
} // namespace Myriad

#endif /* CONSTRANGEPROVIDER_H_ */
