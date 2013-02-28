/*
 * Copyright 2010-2013 DIMA Research Group, TU Berlin
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

#ifndef ABSTRACTRANGEPROVIDER_H_
#define ABSTRACTRANGEPROVIDER_H_

#include "core/types.h"

#include <Poco/AutoPtr.h>

namespace Myriad {
/**
 * @addtogroup runtime_provider_range
 * @{*/

/**
 * Generic range provider template.
 *
 * This is a common base for all runtime components that provide value ranges.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<typename RangeType, class CxtRecordType>
class AbstractRangeProvider
{
public:

    /**
     * Constructor.
     *
     * @param arity An integer specifying how many random seed are consumed on
     *        each invocation of the provider.
     */
    AbstractRangeProvider(const I16u arity) :
        _arity(arity)
    {
    }

    /**
     * Destructor.
     */
    virtual ~AbstractRangeProvider()
    {
    }

    /**
     * Get the random stream arity of this binder.
     */
    I16u arity() const
    {
        return _arity;
    }

    /**
     * Functor method. Provides an interval of \p RangeType values based on the
     * given \p CxtRecordType object.
     *
     * @param cxtRecordPtr A context record for this range provider.
     */
    virtual const Interval<RangeType> operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr) = 0;

private:

    const I16u _arity;
};

/** @}*/// add to runtime_provider_range group
} // namespace Myriad

#endif /* ABSTRACTRANGEPROVIDER_H_ */
