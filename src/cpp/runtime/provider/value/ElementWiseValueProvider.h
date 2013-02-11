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

#ifndef ELEMENTWISEVALUEPROVIDER_H_
#define ELEMENTWISEVALUEPROVIDER_H_

#include "runtime/provider/value/AbstractValueProvider.h"

using namespace Poco;

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider for random values (conditioned variant)
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType, size_t N>
class ElementWiseValueProvider: public AbstractValueProvider<vector<ValueType>, CxtRecordType>
{
public:

    ElementWiseValueProvider(AbstractValueProvider<ValueType, CxtRecordType>& elementValueProvider) :
        AbstractValueProvider<vector<ValueType>, CxtRecordType>(N, false), // TODO: maybe in some cases can be invertible
        _buffer(N),
        _elementValueProvider(elementValueProvider)
    {
    }

    virtual ~ElementWiseValueProvider()
    {
    }

    virtual const vector<ValueType> operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        for (size_t i = 0; i < N; i++)
        {
            _buffer[i] = _elementValueProvider(cxtRecordPtr, random);
        }

        return _buffer;
    }

private:

    vector<ValueType> _buffer;

    AbstractValueProvider<ValueType, CxtRecordType>& _elementValueProvider;
};

} // namespace Myriad

#endif /* ELEMENTWISEVALUEPROVIDER_H_ */
