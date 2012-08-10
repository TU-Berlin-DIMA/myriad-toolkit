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

#include "runtime/provider/value/ValueProvider.h"

using namespace Poco;

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider for constant values
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType>
class ConstValueProvider: public ValueProvider<ValueType, CxtRecordType>
{
public:

    ConstValueProvider(const ValueType& constValue) :
        ValueProvider<ValueType, CxtRecordType>(0),
        _constValue(constValue)
    {
    }

    virtual ~ConstValueProvider()
    {
    }

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& ctxRecordPtr, RandomStream& random)
    {
        return _constValue;
    }

private:

    const ValueType _constValue;
};

} // namespace Myriad

#endif /* CONSTVALUEPROVIDER_H_ */
