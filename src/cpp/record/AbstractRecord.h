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

#include "core/types.h"
#include "io/AbstractOutputCollector.h"
#include "config/EnumSetPool.h"

#include <Poco/Any.h>
#include <Poco/AutoPtr.h>
#include <Poco/DynamicAny.h>
#include <Poco/RefCountedObject.h>

using namespace std;
using namespace Poco;

using Myriad::Bool;
using Myriad::Char;
using Myriad::Date;
using Myriad::Decimal;
using Myriad::Enum;
using Myriad::I16;
using Myriad::I32;
using Myriad::I64;
using Myriad::I16u;
using Myriad::I32u;
using Myriad::I64u;
using Myriad::String;

// forward declarations of auxiliary complex types
template<typename T> class Interval;;

#ifndef ABSTRACTRECORD_H_
#define ABSTRACTRECORD_H_

namespace Myriad {
/**
 * @addtogroup record
 * @{*/

// forward declarations
class AbstractSequenceGenerator;
template<class RecordType>
class SetterChain;
template<class RecordType>
class RecordFactory;
template<class RecordType>
class RecordMeta;
template<class RecordType>
class RecordRangePredicate;


/**
 * A base traits template for all record types.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class RecordType>
struct RecordTraits
{
    typedef RecordMeta<RecordType> MetaType; //!< The concrete record meta type.
    typedef AbstractSequenceGenerator GeneratorType; //!< The concrete sequence generator type.
    typedef SetterChain<RecordType> SetterChainType; //!< The concrete setter chain type.
    typedef RecordFactory<RecordType> FactoryType; //!< The concrete record factory type.
    typedef RecordRangePredicate<RecordType> RangePredicateType; //!< The concrete range predicate type.

    /**
     * An enum of the record fields (zero is always <tt>UNKNOWN</tt>, one is <tt>GEN_ID</tt>).
     */
    enum Field { UNKNOWN, GEN_ID };
};


/**
 * An abstract base for all record types.
 *
 * All records have a common field \p genID that contains the position of
 * the record instance in the generated record sequence.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class AbstractRecord: public Poco::RefCountedObject
{
public:

    /**
     * Set the \p genID field.
     *
     * @param v The new \p genID.
     */
    void genID(const I64u v);
    /**
     * @return The value of the \p genID field.
     */
    I64u genID() const;

    /**
     * Set the \p genID field by reference.
     *
     * @param v The new \p genID.
     */
    void genIDRef(const I64u& v);
    /**
     * @return A reference to the \p genID field.
     */
    const I64u& genIDRef() const;

private:

    ID    _gen_id;
};

inline void AbstractRecord::genID(const I64u v)
{
    _gen_id = v;
}

inline I64u AbstractRecord::genID() const
{
    return _gen_id;
}

inline void AbstractRecord::genIDRef(const I64u& v)
{
    _gen_id = v;
}

inline const I64u& AbstractRecord::genIDRef() const
{
    return _gen_id;
}


/**
 * Default record factory implementation.
 *
 * A factory for a specific record maintains a central instance of the
 * associated record meta type and injects it into all constructed record
 * instances.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class RecordType>
class RecordFactory
{
public:

    /**
     * An alias of the associated record meta type for the \p RecordType.
     */
    typedef typename RecordTraits<RecordType>::MetaType RecordMetaType;

    /**
     * Factory constructor.
     */
    RecordFactory(RecordMetaType meta)
        : _meta(meta)
    {
    }

    /**
     * Object factory function.
     *
     * @return A new \p RecordType instance.
     */
    AutoPtr<RecordType> operator()() const;

    /**
     * Object factory function.
     *
     * @param genID The \p genID of the created record.
     * @return A new \p RecordType instance.
     */
    AutoPtr<RecordType> operator()(const I64u& genID) const;

private:

    const RecordMetaType _meta;
};

template<class RecordType>
inline AutoPtr<RecordType> RecordFactory<RecordType>::operator()() const
{
    return new RecordType(_meta);
}

template<class RecordType>
inline AutoPtr<RecordType> RecordFactory<RecordType>::operator()(const I64u& genID) const
{
    AutoPtr<RecordType> record(new RecordType(_meta));
    record->genID(genID);
    return record;
}


/**
 * A base record meta template for a parameter \p RecordType.
 *
 * RecordMeta instances are used to provide meta required for the construction
 * of a record in a central place. The most common type of such information are
 * references to the specific enumerated sets required for this \p RecordType.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class RecordType>
class RecordMeta
{
public:

    /**
     * Cardinality only constructor.
     *
     * @param cardinality The cardinality of the sequence generated for this
     *        \p RecordType.
     */
    RecordMeta(const I64u cardinality = 0) :
        _cardinality(cardinality) // FIXME: mandatory cardinality
    {
    }

    /**
     * Default constructor.
     *
     * @param enumSets The pool of enumerated sets required for the
     *        generation of this \p RecordType.
     * @param cardinality The cardinality of the sequence generated for this
     *        \p RecordType.
     */
    RecordMeta(const EnumSetPool& enumSets, const I64u cardinality = 0) :
        _cardinality(cardinality) // FIXME: mandatory cardinality
    {
    }

    /**
     * Get the cardinality of the described \p RecordType.
     *
     * @return The cardinality of the described \p RecordType.
     */
    const I64u& cardinality() const
    {
        return _cardinality;
    }

private:

    const I64u _cardinality;
};


/**
 * Unspecialized traits object for introspection of record fields.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template <I16u fid, class RecordType>
struct RecordFieldTraits
{
    typedef I64u FieldType; //!< The field type.
    typedef typename MethodTraits<RecordType, FieldType>::RefGetter FieldGetterType; //!< The field getter method type.
    typedef typename MethodTraits<RecordType, FieldType>::RefSetter FieldSetterType; //!< The field setter method type.

    /**
     * Return a pointer to the field setter method.
     *
     * The default implementation throws a Poco::RuntimeException.
     */
    static FieldSetterType setter()
    {
        throw RuntimeException("Trying to access record field setter for unknown field");
    }

    /**
     * Return a pointer to the field getter method.
     *
     * The default implementation throws a Poco::RuntimeException.
     */
    static FieldGetterType getter()
    {
        throw RuntimeException("Trying to access record field getter for unknown field");
    }
};


/**
 * Specialized traits object for introspection of the \p genID record field.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template <class RecordType>
struct RecordFieldTraits<1, RecordType>
{
    typedef I64u FieldType; //!< The \p genID field type.
    typedef typename MethodTraits<AbstractRecord, FieldType>::RefGetter FieldGetterType; //!< The \p genID field getter method type.
    typedef typename MethodTraits<AbstractRecord, FieldType>::RefSetter FieldSetterType; //!< The \p genID field setter method type.

    /**
     * Return a pointer to the \p genID setter method.
     */
    static FieldSetterType setter()
    {
        return static_cast<FieldSetterType>(&AbstractRecord::genIDRef);
    }

    /**
     * Return a pointer to the \p genID getter method.
     */
    static FieldGetterType getter()
    {
        return static_cast<FieldGetterType>(&AbstractRecord::genIDRef);
    }
};

/** @}*/// add to record group
} // namespace Myriad

#endif /* ABSTRACTRECORD_H_ */
