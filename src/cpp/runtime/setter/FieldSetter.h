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
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */

#ifndef FIELDSETTER_H_
#define FIELDSETTER_H_

#include "runtime/setter/AbstractSetter.h"

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// setter for scalar record fields
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class RecordType, I16u fid, class ValueProviderType>
class FieldSetter: public AbstractSetter<RecordType>
{
public:

    typedef EqualityPredicate<RecordType> EqualityPredicateType;
    typedef typename RecordFieldTraits<fid, RecordType>::FieldType RecordFieldType;
    typedef typename RecordFieldTraits<fid, RecordType>::FieldSetterType RecordFieldSetterType;
    typedef typename RecordFieldTraits<fid, RecordType>::FieldGetterType RecordFieldGetterType;

    FieldSetter(ValueProviderType& valueProvider) :
        AbstractSetter<RecordType>(0, valueProvider.invertible()),
        _fieldSetter(RecordFieldTraits<fid, RecordType>::setter()),
        _fieldGetter(RecordFieldTraits<fid, RecordType>::getter()),
        _valueProvider(valueProvider)
    {
    }

    virtual ~FieldSetter()
    {
    }

    virtual Interval<I64u> valueRange(const AutoPtr<RecordType>& cxtRecordPtr)
    {
        return _valueProvider.valueRange((cxtRecordPtr->*_fieldGetter)(), cxtRecordPtr);
    }

    virtual void filterRange(const EqualityPredicateType& predicate, Interval<I64u>& currentRange)
    {
        if (predicate.bound(fid))
        {
            currentRange.intersect(valueRange(predicate.valueHolder()));
        }
    }

    virtual const void operator()(AutoPtr<RecordType>& cxtRecordPtr, RandomStream& random)
    {
        (cxtRecordPtr->*_fieldSetter)(static_cast<RecordFieldType>(_valueProvider(cxtRecordPtr, random)));
    }

private:

    const RecordFieldSetterType _fieldSetter;
    const RecordFieldGetterType _fieldGetter;
    ValueProviderType& _valueProvider;
};

} // namespace Myriad

#endif /* FIELDSETTER_H_ */
