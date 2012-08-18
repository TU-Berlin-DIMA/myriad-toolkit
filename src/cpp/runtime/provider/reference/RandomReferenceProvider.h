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

#ifndef RANDOMREFERENCEPROVIDER_H_
#define RANDOMREFERENCEPROVIDER_H_

#include "generator/RandomSequenceGenerator.h"
#include "runtime/provider/reference/AbstractReferenceProvider.h"

using namespace Poco;

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// reference provider for clustered references
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class RefRecordType, class CxtRecordType>
class RandomReferenceProvider: public AbstractReferenceProvider<RefRecordType, CxtRecordType>
{
public:

    typedef EqualityPredicate<RefRecordType> EqualityPredicateType;
    typedef EqualityPredicateProvider<RefRecordType, CxtRecordType> EqualityPredicateProviderType;
    typedef RandomSetInspector<RefRecordType> RefRecordSetType;

    RandomReferenceProvider(EqualityPredicateProvider<RefRecordType, CxtRecordType>& equalityPredicateProvider, RandomSetInspector<RefRecordType> referenceSequence) :
        AbstractReferenceProvider<RefRecordType, CxtRecordType>(1, false),
        _equalityPredicateProvider(equalityPredicateProvider),
        _referenceSequence(referenceSequence)
    {
    }

    virtual ~RandomReferenceProvider()
    {
    }

    virtual const AutoPtr<RefRecordType>& operator()(AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        // create EqualityPredicate
        const EqualityPredicateType& predicate = _equalityPredicateProvider(cxtRecordPtr, random);

        // apply predicate filter to get the corresponding record genIDs range
        Interval<I64u> genIDRange = _referenceSequence.filter(predicate);

        // pick a random genID from the valid range
        I64u genID = random(genIDRange.min(), genIDRange.max());

        // lazy-instantiate the corresponding reference
        if (_reference.isNull() || _reference->genID() != genID)
        {
            _reference = _referenceSequence.at(genID);
        }

        // return the selected reference
        return _reference;
    }

private:

    EqualityPredicateProviderType _equalityPredicateProvider;

    RefRecordSetType _referenceSequence;

    AutoPtr<RefRecordType> _reference;
};

} // namespace Myriad

#endif /* RANDOMREFERENCEPROVIDER_H_ */
