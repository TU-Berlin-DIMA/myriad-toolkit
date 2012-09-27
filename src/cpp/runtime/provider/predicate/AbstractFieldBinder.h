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

#ifndef ABSTRACTFIELDBINDER_H_
#define ABSTRACTFIELDBINDER_H_

using namespace Poco;

namespace Myriad {
/**
 * @addtogroup runtime_provider_predicate
 * @{*/

/**
 * Abstract field value binder for equality predicates.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class RecordType, class CxtRecordType>
class AbstractFieldBinder
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
     * @param arity An integer specifying how many random seed are consumed on
     *        each invocation of the binder operator.
     */
    AbstractFieldBinder(const I16u arity) :
        _arity(arity)
    {
    }

    /**
     * Destructor.
     */
    virtual ~AbstractFieldBinder()
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
     * Bind a value to the provided \p predicate.
     *
     * @param predicate The predicate to be bound to a value.
     * @param cxtRecordPtr A context record that provides the value to be bound.
     * @param random The random stream associated with the \p RecordType random
     *        sequence.
     */
    virtual void operator()(EqualityPredicateType& predicate, AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random) = 0;

private:

    const I16u _arity;
};

/** @}*/// add to runtime_predicate group
}  // namespace Myriad

#endif /* ABSTRACTFIELDBINDER_H_ */
