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

#ifndef CONTEXTFIELDVALUEPROVIDER_H_
#define CONTEXTFIELDVALUEPROVIDER_H_

#include "runtime/provider/value/AbstractValueProvider.h"

using namespace Poco;

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider for context field values (2-hop field)
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType, I16u rid1, I16u rid2, I16u fid>
class ContextFieldValueProvider: public AbstractValueProvider<ValueType, CxtRecordType>
{
public:

    typedef typename RecordFieldTraits<rid1, CxtRecordType>::FieldType RID1Type;
    typedef typename RecordFieldTraits<rid1, CxtRecordType>::FieldGetterType RID1GetterType;

    typedef typename RecordFieldTraits<rid2, RID1Type>::FieldType RID2Type;
    typedef typename RecordFieldTraits<rid2, RID1Type>::FieldGetterType RID2GetterType;

    typedef typename RecordFieldTraits<fid, RID2Type>::FieldType FIDType;
    typedef typename RecordFieldTraits<fid, RID2Type>::FieldGetterType FIDGetterType;

    ContextFieldValueProvider() :
        AbstractValueProvider<ValueType, CxtRecordType>(0, false),
        _rid1Getter(RecordFieldTraits<rid1, CxtRecordType>::getter()),
        _rid2Getter(RecordFieldTraits<rid2, RID1Type>::getter()),
        _fidGetter(RecordFieldTraits<fid, RID2Type>::getter())
    {
    }

    virtual ~ContextFieldValueProvider()
    {
    }

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        return (((((cxtRecordPtr->*_rid1Getter)())->*_rid2Getter)())->*_fidGetter)();
    }

private:

    const RID1GetterType _rid1Getter;
    const RID2GetterType _rid2Getter;
    const FIDGetterType _fidGetter;
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider for context field values (1-hop field)
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType, I16u rid1, I16u fid>
class ContextFieldValueProvider<ValueType, CxtRecordType, rid1, fid, 0>: public AbstractValueProvider<ValueType, CxtRecordType>
{
public:

    typedef typename RecordFieldTraits<rid1, CxtRecordType>::FieldType RID1Type;
    typedef typename RecordFieldTraits<rid1, CxtRecordType>::FieldGetterType RID1GetterType;

    typedef typename RecordFieldTraits<fid, RID1Type>::FieldType FIDType;
    typedef typename RecordFieldTraits<fid, RID1Type>::FieldGetterType FIDGetterType;

    ContextFieldValueProvider() :
        AbstractValueProvider<ValueType, CxtRecordType>(0, false),
        _rid1Getter(RecordFieldTraits<rid1, CxtRecordType>::getter()),
        _fidGetter(RecordFieldTraits<fid, RID1Type>::getter())
    {
    }

    virtual ~ContextFieldValueProvider()
    {
    }

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        return (((cxtRecordPtr->*_rid1Getter)())->*_fidGetter)();
    }

private:

    const RID1GetterType _rid1Getter;
    const FIDGetterType _fidGetter;
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// value provider for context field values (direct field)
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename ValueType, class CxtRecordType, I16u fid>
class ContextFieldValueProvider<ValueType, CxtRecordType, fid, 0, 0>: public AbstractValueProvider<ValueType, CxtRecordType>
{
public:

    typedef typename RecordFieldTraits<fid, CxtRecordType>::FieldType CxtRecordFieldType;
    typedef typename RecordFieldTraits<fid, CxtRecordType>::FieldGetterType CxtRecordFieldGetterType;

    ContextFieldValueProvider() :
        AbstractValueProvider<ValueType, CxtRecordType>(0, false),
        _fieldGetter(RecordFieldTraits<fid, CxtRecordType>::getter())
    {
    }

    virtual ~ContextFieldValueProvider()
    {
    }

    virtual const ValueType operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random)
    {
        return (cxtRecordPtr->*_fieldGetter)();
    }

private:

    const CxtRecordFieldGetterType _fieldGetter;
};

} // namespace Myriad

#endif /* CONTEXTFIELDVALUEPROVIDER_H_ */
