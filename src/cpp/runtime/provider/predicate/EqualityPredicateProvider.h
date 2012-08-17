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

#ifndef EQUALITYPREDICATEPROVIDER_H_
#define EQUALITYPREDICATEPROVIDER_H_

#include "runtime/provider/predicate/EqualityPredicateFieldBinder.h"

using namespace Poco;

namespace Myriad {

template<class RecordType, class CxtRecordType>
class EqualityPredicateProvider
{
public:

    typedef AbstractFieldBinder<RecordType, CxtRecordType> AbstractBinderType;
    typedef typename RecordTraits<RecordType>::FactoryType RecordFactoryType;
    typedef EqualityPredicate<RecordType> EqualityPredicateType;

    EqualityPredicateProvider(const RecordFactoryType& recordFactory, AbstractBinderType& b1) :
        _predicate(recordFactory),
        _bindersSize(1),
        _binders(new AbstractBinderType*[_bindersSize])
    {
        _binders[0] = &b1;
    }

    EqualityPredicateProvider(const RecordFactoryType& recordFactory, AbstractBinderType& b1, AbstractBinderType& b2) :
        _predicate(recordFactory),
        _bindersSize(2),
        _binders(new AbstractBinderType*[_bindersSize])
    {
        _binders[0] = &b1;
        _binders[1] = &b2;
    }

    EqualityPredicateProvider(const RecordFactoryType& recordFactory, AbstractBinderType& b1, AbstractBinderType& b2, AbstractBinderType& b3) :
        _predicate(recordFactory),
        _bindersSize(3),
        _binders(new AbstractBinderType*[_bindersSize])
    {
        _binders[0] = &b1;
        _binders[1] = &b2;
        _binders[2] = &b3;
    }

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

    ~EqualityPredicateProvider()
    {
        delete[] _binders;
        _binders = NULL;
        _bindersSize = 0;
    }

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

}  // namespace Myriad

#endif /* EQUALITYPREDICATEPROVIDER_H_ */
