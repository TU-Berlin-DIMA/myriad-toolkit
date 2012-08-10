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

#include "runtime/provider/value/ValueProvider.h"
#include "runtime/provider/value/ConstValueProvider.h"

using namespace Poco;

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider for clustered values
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType, class PrFunctionType, class RangeProviderType>
class ClusteredValueProvider: public ValueProvider<ValueType, CxtRecordType>
{
public:

    ClusteredValueProvider(RangeProviderType& valueProvider) :
        ValueProvider<ValueType, CxtRecordType>(0, false)
    {
    }

    virtual ~ClusteredValueProvider()
    {
    }

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& ctxRecordPtr, RandomStream& random)
    {
        throw LogicException("Unsupported RangeProviderType in ClusteredValueProvider");
    }
};

//~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider for clustered values (const cardinality specialization)
//~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType, class PrFunctionType>
class ClusteredValueProvider<ValueType, CxtRecordType, PrFunctionType, ConstRangeProvider<I64u, CxtRecordType> >: public ValueProvider<ValueType, CxtRecordType>
{
public:

    ClusteredValueProvider(ConstValueProvider<I64u, CxtRecordType>& valueProvider) :
        ValueProvider<ValueType, CxtRecordType>(0, false), // TODO: invertibility may be possible in some cases
        _rangeProvider(valueProvider)
    {
    }

    virtual ~ClusteredValueProvider()
    {
    }

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& ctxRecordPtr, RandomStream& random)
    {
    	return _rangeProvider(ctxRecordPtr, random);
    }

private:

    ConstRangeProvider<I64u, CxtRecordType>& _rangeProvider;
};

//~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider for clustered values (const cardinality specialization)
//~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

//template<typename ValueType, class CxtRecordType, class PrFunctionType>
//class ClusteredValueProvider<ValueType, CxtRecordType, PrFunctionType, ConstValueProvider<I64u, CxtRecordType> >: public ValueProvider<ValueType, CxtRecordType>
//{
//public:
//
//    ClusteredValueProvider(ConstValueProvider<I64u, CxtRecordType>& valueProvider) :
//        ValueProvider<ValueType, CxtRecordType>(0),
//        _valueProvider(valueProvider)
//    {
//    }
//
//    virtual ~ClusteredValueProvider()
//    {
//    }
//
//    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& ctxRecordPtr, RandomStream& random)
//    {
//    	return _valueProvider(ctxRecordPtr, random);
//    }
//
//private:
//
//    ConstValueProvider<I64u, CxtRecordType>& _valueProvider;
//};

} // namespace Myriad

#endif /* CLUSTEREDVALUEPROVIDER_H_ */
