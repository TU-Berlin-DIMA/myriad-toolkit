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

#ifndef RANGEPROVIDER_H_
#define RANGEPROVIDER_H_

#include "core/types.h"
#include "math/random/RandomStream.h"

#include <Poco/AutoPtr.h>

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// generic range provider template
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename RangeType, class CxtRecordType>
class RangeProvider
{
public:

    RangeProvider(const I16u arity) :
    	_arity(arity)
    {
    }

    virtual ~RangeProvider()
    {
    }

    I16u arity() const
    {
    	return _arity;
    }

    virtual const Interval<RangeType> operator()(const AutoPtr<CxtRecordType>& ctxRecordPtr, RandomStream& random) = 0;

private:

    const I16u _arity;
};

} // namespace Myriad

#endif /* RANGEPROVIDER_H_ */
