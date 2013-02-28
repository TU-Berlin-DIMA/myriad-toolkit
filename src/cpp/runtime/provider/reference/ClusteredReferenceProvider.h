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

#ifndef CLUSTEREDREFERENCEPROVIDER_H_
#define CLUSTEREDREFERENCEPROVIDER_H_

#include "generator/RandomSequenceGenerator.h"
#include "generator/InvalidRecordException.h"
#include "runtime/provider/reference/AbstractReferenceProvider.h"
#include "runtime/provider/value/ContextFieldValueProvider.h"
#include "runtime/provider/value/ConstValueProvider.h"

using namespace Poco;

namespace Myriad {
/**
 * @addtogroup runtime_provider_reference
 * @{*/

/**
 * Reference provider for clustered references.
 *
 * This component provides records of type \p RefRecordType to records of
 * type \p CxtRecordType in a clustered manner. For each record of the
 * \p RefRecordType sequence, the provider reserves a block of exactly \p
 * maxChildren corresponding records in the \p CxtRecordType sequence. For each
 * block, the reference provider will use the \p ChildrenCountValueProviderType
 * to obtain the concrete number of children for the corresponding \p
 * RefRecordType reference (this number is guaranteed to be no more than \p
 * maxChildren), and set the reference.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<typename RefRecordType, class CxtRecordType, class ChildrenCountValueProviderType, I16u posFieldID = 0>
class ClusteredReferenceProvider: public AbstractReferenceProvider<RefRecordType, CxtRecordType>
{
public:

    /**
     * The type of the \p CxtRecordType setter for the position field in the.
     */
    typedef typename RecordFieldTraits<posFieldID, CxtRecordType>::FieldSetterType PosFieldSetterType;
    /**
     * The type of the \p CxtRecordType getter for the \p maxChildren parameter.
     */
    typedef ConstValueProvider<I32u, CxtRecordType> MaxChildrenValueProviderType;
    /**
     * The type of the sequence inspector for the referenced \p RefRecordType
     * sequence .
     */
    typedef RandomSequenceInspector<RefRecordType> RefRecordSetType;

    /**
     * Constructor.
     *
     * @param maxChildrenProvider Value provider for the \p maxChildren
     *        parameter.
     * @param childrenCountProvider Value provider for the number of children
     *        associated with a particular \p RefRecordType parent.
     * @param referenceSequence The virtual sequence of the \p RefRecordType.
     */
    ClusteredReferenceProvider(MaxChildrenValueProviderType& maxChildrenProvider, ChildrenCountValueProviderType& childrenCountProvider, RefRecordSetType referenceSequence) :
        AbstractReferenceProvider<RefRecordType, CxtRecordType>(0, true),
        _maxChildrenValue(nullValue<I32u>()),
        _maxChildrenProvider(maxChildrenProvider),
        _childrenCountProvider(childrenCountProvider),
        _referenceSequence(referenceSequence),
        _posFieldSetter(posFieldID != 0 ? RecordFieldTraits<posFieldID, CxtRecordType>::setter() : NULL)
    {
        // make sure that the _childrenCountProvider does not consume random records
        if (_childrenCountProvider.arity() > 0)
        {
            throw RuntimeException("Cannot use childrenCountProviders with arity() > 0 in a ClusteredReferenceProvider");
        }
    }

    /**
     * Destructor.
     */
    virtual ~ClusteredReferenceProvider()
    {
    }

    /**
     * @see AbstractReferenceProvider::referenceRange()
     */
    virtual Interval<I64u> referenceRange(const I64u& refRecordID, const AutoPtr<CxtRecordType>& cxtRecordPtr)
    {
        // lazy initialize max nested per parent
        if (_maxChildrenValue == nullValue<I32u>())
        {
            // TODO: passing a mock random object here is really not pretty
            RandomStream random;
            _maxChildrenValue = _maxChildrenProvider(cxtRecordPtr, random);
        }

        // TODO: return the exact range (i.e. the range without the black IDs)
        return Interval<I64u>(refRecordID * _maxChildrenValue, (refRecordID + 1)* _maxChildrenValue);
    }

    /**
     * @see AbstractReferenceProvider::operator()
     *
     * @throws InvalidRecordException If the given \p cxtRecordPtr belongs to
     *         the invalid range of a \p CxtRecordType block, i.e. to the right
     *         part of the block that is not associated with any parent.
     */
    virtual const AutoPtr<RefRecordType>& operator()(AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        // lazy initialize max nested per parent
        if (_maxChildrenValue == nullValue<I32u>())
        {
            _maxChildrenValue = _maxChildrenProvider(cxtRecordPtr, random);
        }

        I64u nestedRecordGenID = cxtRecordPtr->genID();
        I64u parentRecordGenID = nestedRecordGenID/_maxChildrenValue;

        if (_reference.isNull() || _reference->genID() != parentRecordGenID)
        {
            try
            {
                _reference = _referenceSequence.at(parentRecordGenID);
            }
            catch(const InvalidRecordException& e)
            {
                // throw adapted InvalidRecordException for the child sequence with custom nextValidGenID
                throw InvalidRecordException(nestedRecordGenID, _maxChildrenValue, nestedRecordGenID % _maxChildrenValue, e.nextValidGenID() * _maxChildrenValue);
            }
        }

        // FIXME: the random argument here is wrong
        // Currently this does not influence the execution order as we only
        // permit _childrenCountProvider instances with zero arity.
        // Nevertheless, a cleaner solution will not hurt here.
        I32u nestedCount = _childrenCountProvider(_reference, random);

        if (nestedRecordGenID % _maxChildrenValue < nestedCount)
        {
            if (posFieldID != 0)
            {
                (cxtRecordPtr->*_posFieldSetter)(static_cast<I32u>(nestedRecordGenID-(parentRecordGenID*_maxChildrenValue)));
            }

            return _reference;
        }
        else
        {
            throw InvalidRecordException(nestedRecordGenID, _maxChildrenValue, nestedCount);
        }
    }

private:

    AutoPtr<RefRecordType> _reference;

    I32u _maxChildrenValue;

    MaxChildrenValueProviderType& _maxChildrenProvider;

    ChildrenCountValueProviderType& _childrenCountProvider;

    RefRecordSetType _referenceSequence;

    PosFieldSetterType _posFieldSetter;
};

/** @}*/// add to runtime_provider_reference group
} // namespace Myriad

#endif /* CLUSTEREDREFERENCEPROVIDER_H_ */
