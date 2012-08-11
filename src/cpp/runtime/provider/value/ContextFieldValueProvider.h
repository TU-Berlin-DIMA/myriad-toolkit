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

#ifndef CONTEXTFIELDVALUEPROVIDER_H_
#define CONTEXTFIELDVALUEPROVIDER_H_

#include "runtime/provider/value/ValueProvider.h"

using namespace Poco;

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider for constant values
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType, I16u fid>
class ContextFieldValueProvider: public ValueProvider<ValueType, CxtRecordType>
{
public:

	typedef typename RecordFieldTraits<fid, CxtRecordType>::FieldType CxtRecordFieldType;
	typedef typename RecordFieldTraits<fid, CxtRecordType>::FieldGetterType CxtRecordFieldGetterType;

    ContextFieldValueProvider() :
        ValueProvider<ValueType, CxtRecordType>(0, false),
        _fieldGetter(RecordFieldTraits<fid, CxtRecordType>::getter())
    {
    }

    virtual ~ContextFieldValueProvider()
    {
    }

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        return (cxtRecordPtr->*_fieldGetter)();
    }

private:

    const CxtRecordFieldGetterType _fieldGetter;
};

} // namespace Myriad

#endif /* CONTEXTFIELDVALUEPROVIDER_H_ */
