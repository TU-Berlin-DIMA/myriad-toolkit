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

#ifndef FIELDSETTER_H_
#define FIELDSETTER_H_

#include "runtime/setter/AbstractSetter.h"

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// range provider for constant ranges
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class RecordType, I16u fid, class ValueProviderType>
class FieldSetter: public AbstractSetter<RecordType, fid>
{
public:

	typedef typename RecordFieldTraits<fid, RecordType>::FieldType RecordFieldType;
	typedef typename RecordFieldTraits<fid, RecordType>::FieldSetterType RecordFieldSetterType;
	typedef typename RecordFieldTraits<fid, RecordType>::FieldGetterType RecordFieldGetterType;

    FieldSetter(ValueProviderType& valueProvider) :
    	AbstractSetter<RecordType, fid>(0, valueProvider.invertible()),
    	_fieldSetter(RecordFieldTraits<fid, RecordType>::setter()),
    	_fieldGetter(RecordFieldTraits<fid, RecordType>::getter()),
        _valueProvider(valueProvider)
    {
    }

    virtual ~FieldSetter()
    {
    }

    virtual Interval<I64u> fieldValueRange(const AutoPtr<RecordType>& ctxRecordPtr, RandomStream& random)
	{
    	return _valueProvider.fieldValueRange((ctxRecordPtr->*_fieldGetter)(), ctxRecordPtr, random);
	}

    virtual const void operator()(AutoPtr<RecordType>& ctxRecordPtr, RandomStream& random)
    {
    	(ctxRecordPtr->*_fieldSetter)(static_cast<RecordFieldType>(_valueProvider(ctxRecordPtr, random)));
    }

private:

    const RecordFieldSetterType _fieldSetter;
    const RecordFieldGetterType _fieldGetter;
    ValueProviderType& _valueProvider;
};

} // namespace Myriad

#endif /* CONTEXTFIELDRANGEPROVIDER_H_ */
