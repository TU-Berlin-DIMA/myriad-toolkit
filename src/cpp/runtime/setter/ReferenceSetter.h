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

#ifndef REFERENCESETTER_H_
#define REFERENCESETTER_H_

#include "runtime/setter/AbstractSetter.h"

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// setter for scalar record fields
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class RecordType, I16u fid, class ReferenceProviderType>
class ReferenceSetter: public AbstractSetter<RecordType, fid>
{
public:

    typedef typename RecordFieldTraits<fid, RecordType>::FieldType ReferenceType;
    typedef typename RecordFieldTraits<fid, RecordType>::FieldSetterType ReferenceSetterType;
    typedef typename RecordFieldTraits<fid, RecordType>::FieldGetterType ReferenceGetterType;

    ReferenceSetter(ReferenceProviderType& referenceProvider) :
        AbstractSetter<RecordType, fid>(referenceProvider.arity(), referenceProvider.invertible()),
        _referenceSetter(RecordFieldTraits<fid, RecordType>::setter()),
        _referenceGetter(RecordFieldTraits<fid, RecordType>::getter()),
        _referenceProvider(referenceProvider)
    {
    }

    virtual ~ReferenceSetter()
    {
    }

    virtual Interval<I64u> valueRange(const AutoPtr<RecordType>& cxtRecordPtr)
    {
        return _referenceProvider.referenceRange((cxtRecordPtr->*_referenceGetter)()->genID(), cxtRecordPtr);
    }

    virtual const void operator()(AutoPtr<RecordType>& cxtRecordPtr, RandomStream& random)
    {
        (cxtRecordPtr->*_referenceSetter)(static_cast<const AutoPtr<ReferenceType>&>(_referenceProvider(cxtRecordPtr, random)));
    }

private:

    const ReferenceSetterType _referenceSetter;
    const ReferenceGetterType _referenceGetter;
    ReferenceProviderType& _referenceProvider;
};

} // namespace Myriad

#endif /* REFERENCESETTER_H_ */
