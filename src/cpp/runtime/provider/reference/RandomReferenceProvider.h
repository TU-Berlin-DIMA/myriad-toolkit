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
/**
 * @addtogroup runtime_provider_reference
 * @{*/

/**
 * Reference provider for random references.
 *
 * This component binds an equality predicate from the provided \p CxtRecordType
 * instance, and uses it to obtain a range of \p RefRecordType \p genIDs
 * fulfilling the predicate. One of the identified \p genIDs is picked uniformly
 * at random as the referenced \p RefRecordType parent for the given
 * \p CxtRecordType.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
//
template<class RefRecordType, class CxtRecordType>
class RandomReferenceProvider: public AbstractReferenceProvider<RefRecordType, CxtRecordType>
{
public:

    /**
     * The type of the equality predicate associated with the \p RefRecordType.
     */
    typedef EqualityPredicate<RefRecordType> EqualityPredicateType;
    /**
     * The type of the \p RefRecordType equality predicate provider with bound
     * values obtained from the given \p CxtRecordType.
     */
    typedef EqualityPredicateProvider<RefRecordType, CxtRecordType> EqualityPredicateProviderType;
    /**
     * The type of the sequence inspector for the referenced \p RefRecordType
     * sequence .
     */
    typedef RandomSequenceInspector<RefRecordType> RefRecordSetType;

    /**
     * Constructor.
     *
     * @param equalityPredicateProvider The provider used to obtain valid
     *        selection predicates from the given \p CxtRecordType instances.
     * @param referenceSequence The virtual sequence of the \p RefRecordType.
     */
    RandomReferenceProvider(EqualityPredicateProvider<RefRecordType, CxtRecordType>& equalityPredicateProvider, RandomSequenceInspector<RefRecordType> referenceSequence) :
        AbstractReferenceProvider<RefRecordType, CxtRecordType>(1, false),
        _equalityPredicateProvider(equalityPredicateProvider),
        _referenceSequence(referenceSequence)
    {
    }

    /**
     * Destructor.
     */
    virtual ~RandomReferenceProvider()
    {
    }


    /**
     * @see AbstractReferenceProvider::operator()
     */
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
            // protect against InvalidRecordExceptions
            I16u x = 0;
            while(true)
            {
                try
                {
                    if (x > 1)
                    {
                        throw RuntimeException(format("Subsequent child sequences of effective length zero detected at position %Lu", genID));
                    }

                    _reference = _referenceSequence.at(genID);
                    break;
                }
                catch(const InvalidRecordException& e)
                {
                    // use modValidGenID
                    genID = e.prevValidGenIDMin() + genID % e.prevValidGenIDSize();
                    x++;
                }
            }
        }

        // return the selected reference
        return _reference;
    }

private:

    EqualityPredicateProviderType _equalityPredicateProvider;

    RefRecordSetType _referenceSequence;

    AutoPtr<RefRecordType> _reference;
};

/** @}*/// add to runtime_provider_reference group
} // namespace Myriad

#endif /* RANDOMREFERENCEPROVIDER_H_ */
