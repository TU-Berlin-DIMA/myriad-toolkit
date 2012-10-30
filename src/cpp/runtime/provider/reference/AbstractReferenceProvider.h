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

#ifndef ABSTRACTREFERENCEPROVIDER_H_
#define ABSTRACTREFERENCEPROVIDER_H_

#include "core/types.h"
#include "math/random/RandomStream.h"

#include <Poco/AutoPtr.h>

namespace Myriad {
/**
 * @addtogroup runtime_provider_reference
 * @{*/

/**
 * Generic reference provider template.
 *
 * This is a common base for all runtime components that provide referenced
 * records.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<typename RefRecordType, class CxtRecordType>
class AbstractReferenceProvider
{
public:

    /**
     * Constructor.
     *
     * @param arity An integer specifying how many random seed are consumed on
     *        each invocation of the provider.
     * @param invertible A boolean flag indicating whether the provider is
     *        invertible.
     */
    AbstractReferenceProvider(const I16u arity, bool invertible) :
        _arity(arity),
        _invertible(invertible)
    {
    }

    /**
     * Destructor.
     */
    virtual ~AbstractReferenceProvider()
    {
    }

    /**
     * Get the random stream arity of this binder.
     */
    I16u arity() const
    {
        return _arity;
    }

    /**
     * Returns \p true if the provider is invertible.
     */
    bool invertible() const
    {
        return _invertible;
    }

    /**
     * If the provider is invertible, this method returns the range of genID
     * values which are associated with the referenced.
     *
     * @param refRecordID The genID of the referenced record.
     * @param cxtRecordPtr The current context record. Please note that this is
     *        not necessary the record with contains the reference.
     */
    virtual Interval<I64u> referenceRange(const I64u& refRecordID, const AutoPtr<CxtRecordType>& cxtRecordPtr)
    {
        if (_invertible)
        {
            throw RuntimeException("Trying to access missing referenceRange method implementation in an invertible ReferenceProvider");
        }
        else
        {
            throw RuntimeException("Trying to access referenceRange method of non-invertible ReferenceProvider");
        }
    }

    /**
     * Functor method. Provides an object of type \p RefRecordType based on the
     * given \p CxtRecordType object and the \p random stream associated with
     * the setter chain enclosing this component.
     *
     * @param cxtRecordPtr A context record for this range provider.
     * @param random The random stream associated with the \p CxtRecordType
     *        sequence.
     */
    virtual const AutoPtr<RefRecordType>& operator()(AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random) = 0;

private:

    const I16u _arity;

    const bool _invertible;
};

/** @}*/// add to runtime_provider_reference group
} // namespace Myriad

#endif /* ABSTRACTREFERENCEPROVIDER_H_ */
