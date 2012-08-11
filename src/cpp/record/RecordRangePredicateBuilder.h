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

#ifndef RECORDRANGEPREDICATEBUILDER_H_
#define RECORDRANGEPREDICATEBUILDER_H_

#include <Poco/AutoPtr.h>

using namespace Poco;

namespace Myriad
{

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// forward declarations
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType>
class ValueProvider;

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// generic range predicate builder template
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class RecordType, class CxtRecordType, I16u fid1 = 0, I16u fid2 = 0, I16u fid3 = 0, I16u fid4 = 0, I16u fid5 = 0>
class RecordRangePredicateBuilder
{
public:

    // range predicate type
    typedef typename RecordTraits<RecordType>::RangePredicateType RecordRangePredicateType;
    // field types
    typedef typename RecordFieldTraits<fid1, RecordType>::FieldType Field1Type;
    typedef typename RecordFieldTraits<fid2, RecordType>::FieldType Field2Type;
    typedef typename RecordFieldTraits<fid3, RecordType>::FieldType Field3Type;
    typedef typename RecordFieldTraits<fid4, RecordType>::FieldType Field4Type;
    typedef typename RecordFieldTraits<fid5, RecordType>::FieldType Field5Type;
    // setter types
    typedef typename RecordFieldTraits<fid1, RecordType>::RangeSetterShortType FieldSetter1Type;
    typedef typename RecordFieldTraits<fid2, RecordType>::RangeSetterShortType FieldSetter2Type;
    typedef typename RecordFieldTraits<fid3, RecordType>::RangeSetterShortType FieldSetter3Type;
    typedef typename RecordFieldTraits<fid4, RecordType>::RangeSetterShortType FieldSetter4Type;
    typedef typename RecordFieldTraits<fid5, RecordType>::RangeSetterShortType FieldSetter5Type;
    // value provider types
    typedef ValueProvider<Field1Type, CxtRecordType>* ValueProvider1Type;
    typedef ValueProvider<Field2Type, CxtRecordType>* ValueProvider2Type;
    typedef ValueProvider<Field3Type, CxtRecordType>* ValueProvider3Type;
    typedef ValueProvider<Field4Type, CxtRecordType>* ValueProvider4Type;
    typedef ValueProvider<Field5Type, CxtRecordType>* ValueProvider5Type;

    RecordRangePredicateBuilder(ValueProvider1Type valueProvider1 = NULL, ValueProvider2Type valueProvider2 = NULL, ValueProvider3Type valueProvider3 = NULL, ValueProvider4Type valueProvider4 = NULL, ValueProvider5Type valueProvider5 = NULL) :
        _valueProvider1(valueProvider1),
        _valueProvider2(valueProvider2),
        _valueProvider3(valueProvider3),
        _valueProvider4(valueProvider4),
        _valueProvider5(valueProvider5)
    {
        if (fid1 > 0)
        {
            _fieldSetter1 = RecordFieldTraits<fid1, RecordType>::rangeSetterShort();

            if (_valueProvider1 == NULL)
            {
                throw RuntimeException("Value provider for field #1 is missing");
            }
        }
        if (fid2 > 0)
        {
            _fieldSetter2 = RecordFieldTraits<fid2, RecordType>::rangeSetterShort();

            if (_valueProvider1 == NULL)
            {
                throw RuntimeException("Value provider for field #2 is missing");
            }
        }
        if (fid3 > 0)
        {
            _fieldSetter3 = RecordFieldTraits<fid3, RecordType>::rangeSetterShort();

            if (_valueProvider1 == NULL)
            {
                throw RuntimeException("Value provider for field #3 is missing");
            }
        }
        if (fid4 > 0)
        {
            _fieldSetter4 = RecordFieldTraits<fid4, RecordType>::rangeSetterShort();

            if (_valueProvider1 == NULL)
            {
                throw RuntimeException("Value provider for field #4 is missing");
            }
        }
        if (fid5 > 0)
        {
            _fieldSetter5 = RecordFieldTraits<fid5, RecordType>::rangeSetterShort();
            if (_valueProvider1 == NULL)
            {
                throw RuntimeException("Value provider for field #5 is missing");
            }
        }
    }

    ~RecordRangePredicateBuilder()
    {
        delete _valueProvider1;
        delete _valueProvider2;
        delete _valueProvider3;
        delete _valueProvider4;
        delete _valueProvider5;
    }

    /**
     * Object hydrating function (external PRNG).
     */
    void operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr, RecordRangePredicateType& predicate, RandomStream& random)
    {
        if (fid1 > 0)
        {
            (predicate.*_fieldSetter1)((*_valueProvider1)(cxtRecordPtr, random));
        }
        if (fid2 > 0)
        {
            (predicate.*_fieldSetter2)((*_valueProvider2)(cxtRecordPtr, random));
        }
        if (fid3 > 0)
        {
            (predicate.*_fieldSetter3)((*_valueProvider3)(cxtRecordPtr, random));
        }
        if (fid4 > 0)
        {
            (predicate.*_fieldSetter4)((*_valueProvider4)(cxtRecordPtr, random));
        }
        if (fid5 > 0)
        {
            (predicate.*_fieldSetter5)((*_valueProvider5)(cxtRecordPtr, random));
        }
    }

private:

    FieldSetter1Type _fieldSetter1;
    FieldSetter2Type _fieldSetter2;
    FieldSetter3Type _fieldSetter3;
    FieldSetter4Type _fieldSetter4;
    FieldSetter5Type _fieldSetter5;

    ValueProvider1Type _valueProvider1;
    ValueProvider2Type _valueProvider2;
    ValueProvider3Type _valueProvider3;
    ValueProvider4Type _valueProvider4;
    ValueProvider5Type _valueProvider5;
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider from context field member
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType, I16u fid>
class CxtRecordFieldValueProvider : public ValueProvider<ValueType, CxtRecordType>
{
public:

    typedef typename RecordFieldTraits<fid, CxtRecordType>::FieldGetterType FieldGetterType;

    CxtRecordFieldValueProvider() :
        _getter(RecordFieldTraits<fid, CxtRecordType>::getter())
    {
    }

    virtual ~CxtRecordFieldValueProvider()
    {
    }

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        return (cxtRecordPtr->*_getter)();
    }

    virtual I16u arity() const
    {
        return 0;
    }

private:

    FieldGetterType _getter;
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider from a probability function
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType, class PrFunctionType>
class ProbabilityValueProvider : public ValueProvider<ValueType, CxtRecordType>
{
public:

    ProbabilityValueProvider(PrFunctionType& probability) :
        _probability(probability)
    {
    }

    virtual ~ProbabilityValueProvider()
    {
    }

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        return _probability.sample(random());
    }

    virtual I16u arity() const
    {
        return 1;
    }

private:

    PrFunctionType& _probability;
};

} // namespace Myriad

#endif /* RECORDRANGEPREDICATEBUILDER_H_ */
