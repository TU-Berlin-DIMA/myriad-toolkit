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

#ifndef EQUALITYPREDICATE_H_
#define EQUALITYPREDICATE_H_

#include <Poco/AutoPtr.h>

using namespace Poco;

namespace Myriad {

template<class RecordType>
class EqualityPredicate
{
public:

    typedef typename RecordTraits<RecordType>::FactoryType RecordFactoryType;

    EqualityPredicate(RecordFactoryType& recordFactory) :
        _valueHolder(recordFactory()),
        _boundValues(0) // FIXME: this restricts the max number of fields per record type to 64
    {
    }

    /**
     * Resets all bound values in this predicate.
     */
    void reset()
    {
        _boundValues = static_cast<I64u>(0);
    }

    /**
     * Check if a predicate field (i.e. a field of the RecordType) is bound.
     */
    bool bound(I16u fid) const
    {
        return (_boundValues >> fid) & static_cast<I64u>(1);
    }

    /**
     * Get a constant pointer to the valueHolder.
     */
    const AutoPtr<RecordType>& valueHolder() const
    {
        return _valueHolder;
    }

    /**
     * Generic template method - binds a value to a particular record field.
     *
     * @throws RuntimeException if the fiels is already bound to another value.
     */
    template<I16u fid> void fieldValue(const typename RecordFieldTraits<fid, RecordType>::FieldType& value)
    {
        // sanity check - field should not be already bound
        if (bound(fid))
        {
            throw RuntimeException("Trying to bind abinding values");
        }

        // grab the corresponding field setter
        typename RecordFieldTraits<fid, RecordType>::FieldSetterType setter = RecordFieldTraits<fid, RecordType>::setter();

        // bind the field value
        (_valueHolder->*setter)(value);

        // mark field as bound
        markBound(fid);
    }

    /**
     * Generic template method - retrieves a predicate field value.
     *
     * @throws
     */
    template<I16u fid> typename RecordFieldTraits<fid, RecordType>::FieldType fieldValue() const
    {
        // grab the corresponding field getter
        typename RecordFieldTraits<fid, RecordType>::FieldGetterType getter = RecordFieldTraits<fid, RecordType>::getter();

        // bind the field value
        return (_valueHolder->*getter)();
    }

private:

    /**
     * Marks predicate field as bound.
     */
    void markBound(I16u fid)
    {
        _boundValues |= (static_cast<I64u>(1) << fid);
    }

    AutoPtr<RecordType> _valueHolder;

    I64u _boundValues;
};

}  // namespace Myriad

#endif /* EQUALITYPREDICATE_H_ */
