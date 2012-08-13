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

#ifndef EQUALITYPREDICATEFIELDBINDER_H_
#define EQUALITYPREDICATEFIELDBINDER_H_

#include "runtime/provider/predicate/AbstractFieldBinder.h"

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// generic equality predicate field value binder template
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class RecordType, I16u fid, class CxtRecordType, class ValueProviderType>
class EqualityPredicateFieldBinder : public AbstractFieldBinder<RecordType, CxtRecordType>
{
public:

    typedef EqualityPredicate<RecordType> EqualityPredicateType;

    EqualityPredicateFieldBinder(ValueProviderType& valueProvider) :
        AbstractFieldBinder<RecordType, CxtRecordType>(valueProvider.arity()),
        _valueProvider(valueProvider)
    {
    }

    virtual ~EqualityPredicateFieldBinder()
    {
    }

    virtual void operator()(EqualityPredicateType& predicate, AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        predicate.template fieldValue<fid>(_valueProvider(cxtRecordPtr, random));
    }

private:

    ValueProviderType& _valueProvider;
};

}  // namespace Myriad

#endif /* EQUALITYPREDICATEFIELDBINDER_H_ */
