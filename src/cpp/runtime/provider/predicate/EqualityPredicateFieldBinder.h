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
 */

#ifndef EQUALITYPREDICATEFIELDBINDER_H_
#define EQUALITYPREDICATEFIELDBINDER_H_

#include "runtime/provider/predicate/AbstractFieldBinder.h"

namespace Myriad {
/**
 * @addtogroup runtime_provider_predicate
 * @{*/

/**
 * Generic equality predicate field value binder template.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class RecordType, I16u fid, class CxtRecordType, class ValueProviderType>
class EqualityPredicateFieldBinder : public AbstractFieldBinder<RecordType, CxtRecordType>
{
public:

    /**
     * An alias of the equality predicate type for the concrete \p RecordType
     * template parameter.
     */
    typedef EqualityPredicate<RecordType> EqualityPredicateType;

    /**
     * Constructor.
     *
     * @param valueProvider A value provider that works on context records of
     *        the specified \p CxtRecordType to provide the value to be bound.
     */
    EqualityPredicateFieldBinder(ValueProviderType& valueProvider) :
        AbstractFieldBinder<RecordType, CxtRecordType>(valueProvider.arity()),
        _valueProvider(valueProvider)
    {
    }

    /**
     * Destructor.
     */
    virtual ~EqualityPredicateFieldBinder()
    {
    }

    /**
     * Use the \p valueProvider constructor parameter to obtain a value from the
     * current \p cxtRecordPtr that will be bound to the given \p predicate.
     *
     * @param predicate The predicate to be bound to a value.
     * @param cxtRecordPtr A context record that provides the value to be bound.
     * @param random The random stream associated with the \p RecordType random
     *        sequence.
     */
    virtual void operator()(EqualityPredicateType& predicate, AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        predicate.template fieldValue<fid>(_valueProvider(cxtRecordPtr, random));
    }

private:

    ValueProviderType& _valueProvider;
};

/** @}*/// add to runtime_provider_predicate group
}  // namespace Myriad

#endif /* EQUALITYPREDICATEFIELDBINDER_H_ */
