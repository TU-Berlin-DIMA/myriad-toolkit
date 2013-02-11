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

#ifndef EQUALITYPREDICATEPROVIDER_H_
#define EQUALITYPREDICATEPROVIDER_H_

#include "runtime/provider/predicate/EqualityPredicateFieldBinder.h"

using namespace Poco;

namespace Myriad {
/**
 * @addtogroup runtime_provider_predicate
 * @{*/

/**
 * A provider for equality predicates.
 *
 * This implementation uses between one and four field binders, which are
 * applied in turn to bind the corresponding fields to the equality predicate
 * associated with the \p RecordType template type.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class RecordType, class CxtRecordType>
class EqualityPredicateProvider
{
public:

    /**
     * The type of the abstract field binder for the current \p RecordType,
     * \p CxtRecordType pair.
     */
    typedef AbstractFieldBinder<RecordType, CxtRecordType> AbstractBinderType;
    /**
     * An alias of the factory type associated with the current \p RecordType.
     */
    typedef typename RecordTraits<RecordType>::FactoryType RecordFactoryType;
    /**
     * An alias of the equality predicate for the current \p RecordType.
     */
    typedef EqualityPredicate<RecordType> EqualityPredicateType;

    /**
     * Constructor.
     *
     * @param recordFactory A recordFactory instance required by the
     *        EqualityPredicateType constructor.
     * @param b1 A field binder to be applied when constructing the predicate.
     */
    EqualityPredicateProvider(const RecordFactoryType& recordFactory, AbstractBinderType& b1) :
        _predicate(recordFactory),
        _bindersSize(1),
        _binders(new AbstractBinderType*[_bindersSize])
    {
        _binders[0] = &b1;
    }

    /**
     * Constructor.
     *
     * @param recordFactory A recordFactory instance required by the
     *        EqualityPredicateType constructor.
     * @param b1 A field binder to be applied when constructing the predicate.
     * @param b2 A field binder to be applied when constructing the predicate.
     */
    EqualityPredicateProvider(const RecordFactoryType& recordFactory, AbstractBinderType& b1, AbstractBinderType& b2) :
        _predicate(recordFactory),
        _bindersSize(2),
        _binders(new AbstractBinderType*[_bindersSize])
    {
        _binders[0] = &b1;
        _binders[1] = &b2;
    }

    /**
     * Constructor.
     *
     * @param recordFactory A recordFactory instance required by the
     *        EqualityPredicateType constructor.
     * @param b1 A field binder to be applied when constructing the predicate.
     * @param b2 A field binder to be applied when constructing the predicate.
     * @param b3 A field binder to be applied when constructing the predicate.
     */
    EqualityPredicateProvider(const RecordFactoryType& recordFactory, AbstractBinderType& b1, AbstractBinderType& b2, AbstractBinderType& b3) :
        _predicate(recordFactory),
        _bindersSize(3),
        _binders(new AbstractBinderType*[_bindersSize])
    {
        _binders[0] = &b1;
        _binders[1] = &b2;
        _binders[2] = &b3;
    }

    /**
     * Constructor.
     *
     * @param recordFactory A recordFactory instance required by the
     *        EqualityPredicateType constructor.
     * @param b1 A field binder to be applied when constructing the predicate.
     * @param b2 A field binder to be applied when constructing the predicate.
     * @param b3 A field binder to be applied when constructing the predicate.
     * @param b4 A field binder to be applied when constructing the predicate.
     */
    EqualityPredicateProvider(const RecordFactoryType& recordFactory, AbstractBinderType& b1, AbstractBinderType& b2, AbstractBinderType& b3, AbstractBinderType& b4) :
        _predicate(recordFactory),
        _bindersSize(4),
        _binders(new AbstractBinderType*[_bindersSize])
    {
        _binders[0] = &b1;
        _binders[1] = &b2;
        _binders[2] = &b3;
        _binders[3] = &b4;
    }

    /**
     * Copy constructor.
     */
    EqualityPredicateProvider(const EqualityPredicateProvider& other) :
        _predicate(other._predicate),
        _bindersSize(other._bindersSize),
        _binders(new AbstractBinderType*[_bindersSize])
    {
        for (size_t i = 0; i < _bindersSize; i++)
        {
            _binders[i] = other._binders[i];
        }
    }

    /**
     * Destructor.
     */
    ~EqualityPredicateProvider()
    {
        delete[] _binders;
        _binders = NULL;
        _bindersSize = 0;
    }

    /**
     * Assignment operator.
     */
    EqualityPredicateProvider& operator=(const EqualityPredicateProvider& rhs)
    {
        delete[] _binders;

        _predicate(rhs._predicate);
        _bindersSize(rhs._bindersSize);
        _binders(new AbstractBinderType*[_bindersSize]);

        for (size_t i = 0; i < _bindersSize; i++)
        {
            _binders[i] = rhs._binders[i];
        }

        _predicate.reset();

        return *this;
    }

    /**
     * Functor method. Applies all binders using the given \p cxtRecordPtr and
     * \p random stream and returns the bound EqualityPredicateType.
     *
     * @param cxtRecordPtr A context record that provides the value to be bound.
     * @param random The random stream associated with the \p RecordType random
     *        sequence.
     * @return The bound predicate.
     */
    const EqualityPredicateType& operator()(AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        _predicate.reset();

        for (size_t i = 0; i < _bindersSize; i++)
        {
            (*_binders[i])(_predicate, cxtRecordPtr, random);
        }

        return _predicate;
    }

private:

    EqualityPredicateType _predicate;

    size_t _bindersSize;

    AbstractBinderType** _binders;
};

/** @}*/// add to runtime_provider_predicate group
}  // namespace Myriad

#endif /* EQUALITYPREDICATEPROVIDER_H_ */
