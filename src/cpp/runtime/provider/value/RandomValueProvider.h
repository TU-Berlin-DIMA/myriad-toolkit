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

#ifndef RANDOMVALUEPROVIDER_H_
#define RANDOMVALUEPROVIDER_H_

#include "runtime/provider/value/AbstractValueProvider.h"

using namespace Poco;

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider for random values (conditioned variant)
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType, class PrFunctionType, I16u condFieldID>
class RandomValueProvider: public AbstractValueProvider<ValueType, CxtRecordType>
{
public:

    typedef typename RecordFieldTraits<condFieldID, CxtRecordType>::FieldType CxtRecordFieldType;
    typedef typename RecordFieldTraits<condFieldID, CxtRecordType>::FieldGetterType CxtRecordFieldGetterType;

    RandomValueProvider(const PrFunctionType& prFunction) :
        AbstractValueProvider<ValueType, CxtRecordType>(1, false),
        _prFunction(prFunction),
        _fieldGetter(RecordFieldTraits<condFieldID, CxtRecordType>::getter())
    {
    }

    virtual ~RandomValueProvider()
    {
    }

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        return static_cast<ValueType>(_prFunction.sample(random(), (cxtRecordPtr->*_fieldGetter)()));
    }

private:

    const PrFunctionType& _prFunction;

    const CxtRecordFieldGetterType _fieldGetter;
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider for random values (unconditioned specialization)
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType, class PrFunctionType>
class RandomValueProvider<ValueType, CxtRecordType, PrFunctionType, 0> : public AbstractValueProvider<ValueType, CxtRecordType>
{
public:

    RandomValueProvider(const PrFunctionType& prFunction) :
        AbstractValueProvider<ValueType, CxtRecordType>(1, false),
        _prFunction(prFunction)
    {
    }

    virtual ~RandomValueProvider()
    {
    }

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        return static_cast<ValueType>(_prFunction.sample(random()));
    }

private:

    const PrFunctionType& _prFunction;
};

} // namespace Myriad

#endif /* RANDOMVALUEPROVIDER_H_ */
