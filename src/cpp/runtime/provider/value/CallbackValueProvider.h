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

#ifndef CALLBACKVALUEPROVIDER_H_
#define CALLBACKVALUEPROVIDER_H_

#include "runtime/provider/value/AbstractValueProvider.h"

using namespace Poco;

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider using a custom user callback
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType, class CallbackType>
class CallbackValueProvider : public AbstractValueProvider<ValueType, CxtRecordType>
{
public:

    typedef ValueType (CallbackType::*CallbackMethodType)(const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random);

    CallbackValueProvider(CallbackType& callbackObject, CallbackMethodType callbackMethod, const I16u arity) :
        AbstractValueProvider<ValueType, CxtRecordType>(arity, false),
        _callbackObject(callbackObject),
        _callbackMethod(callbackMethod)
    {
    }

    virtual ~CallbackValueProvider()
    {
    }

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        return (_callbackObject.*_callbackMethod)(cxtRecordPtr, random);
    }

private:

    CallbackType& _callbackObject;
    CallbackMethodType _callbackMethod;
};

} // namespace Myriad

#endif /* CALLBACKVALUEPROVIDER_H_ */
