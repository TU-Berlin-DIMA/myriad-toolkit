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

#ifndef AGGREGATECHILDFIELDVALUEPROVIDER_H_
#define AGGREGATECHILDFIELDVALUEPROVIDER_H_

namespace Myriad {

template<typename ValueType, class CxtRecordType, class RefRecordType, I16u aggFID>
class AggregateChildFieldValueProvider : public AbstractValueProvider<ValueType, CxtRecordType>
{
public:

    typedef typename RecordFieldTraits<aggFID, RefRecordType>::FieldGetterType AggFieldGetterType;
    typedef RandomSequenceInspector<RefRecordType> RefRecordSetType;

    AggregateChildFieldValueProvider(RefRecordSetType referenceSequence) :
        AbstractValueProvider<ValueType, CxtRecordType>(0, false),
        _referenceSequence(referenceSequence),
        _aggFieldGetter(RecordFieldTraits<aggFID, RefRecordType>::getter())
    {

    }

    virtual ~AggregateChildFieldValueProvider()
    {
    }

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        return 0.43;
    }

private:

    RefRecordSetType _referenceSequence;
    AggFieldGetterType _aggFieldGetter;
};

}  // namespace Myriad

#endif /* AGGREGATECHILDFIELDVALUEPROVIDER_H_ */
