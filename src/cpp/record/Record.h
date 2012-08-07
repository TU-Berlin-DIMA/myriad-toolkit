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

#include "core/types.h"

#include <map>
#include <string>
#include <Poco/Any.h>
#include <Poco/AutoPtr.h>
#include <Poco/DynamicAny.h>
#include <Poco/RefCountedObject.h>

using namespace std;
using namespace Poco;

#ifndef RECORD_H_
#define RECORD_H_

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// forward declarations
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

class RecordGenerator;
template<class RecordType>
class HydratorChain;
template<class RecordType>
class RecordFactory;
template<class RecordType>
class RecordMeta;
template<class RecordType>
class RecordRangePredicate;

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// record traits
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class RecordType>
struct RecordTraits
{
	typedef RecordMeta<RecordType> MetaType;
	typedef RecordGenerator GeneratorType;
	typedef HydratorChain<RecordType> HydratorChainType;
	typedef RecordFactory<RecordType> FactoryType;
	typedef RecordRangePredicate<RecordType> RangePredicateType;

	enum Field { UNKNOWN, GEN_ID };
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// record type
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

class Record: public Poco::RefCountedObject
{
public:

	I64u genID() const;
	void genID(const I64u v);

	const I64u& genIDRef() const;
    void genIDRef(const I64u& v);

private:

	ID	meta_genid;
};

inline I64u Record::genID() const
{
	return meta_genid;
}

inline void Record::genID(const I64u v)
{
	meta_genid = v;
}

inline const I64u& Record::genIDRef() const
{
	return meta_genid;
}

inline void Record::genIDRef(const I64u& v)
{
	meta_genid = v;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// record factory
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class RecordType>
class RecordFactory
{
public:

	typedef typename RecordTraits<RecordType>::MetaType RecordMetaType;

	RecordFactory(RecordMetaType meta)
		: _meta(meta)
	{
	}

	/**
	 * Object generating function.
	 */
	AutoPtr<RecordType> operator()() const;

private:

	const RecordMetaType _meta;
};

template<class RecordType>
inline AutoPtr<RecordType> RecordFactory<RecordType>::operator()() const
{
	return new RecordType(_meta);
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// record meta
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class RecordType>
class RecordMeta
{
public:

	RecordMeta()
	{
	}

	RecordMeta(const map<string, vector<string> >& enumSets)
	{
	}
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// record field inspection structures
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

/**
 * Unspecialized traits object for introspection on record fields.
 */
template <I16u fid, class RecordType>
struct RecordFieldTraits
{
	typedef I64u FieldType;
	typedef typename MethodTraits<RecordType, FieldType>::Getter GetterType;
	typedef typename MethodTraits<RecordType, FieldType>::Setter SetterType;

	static const char* name;

	static SetterType setter()
	{
		throw RuntimeException("Trying to access setter for unknown field");
	}

	static GetterType getter()
	{
		throw RuntimeException("Trying to access getter for unknown field");
	}
};

template <I16u fid, class RecordType>
const char* RecordFieldTraits<fid, RecordType>::name = "unknown_field";

/**
 * Unspecialized traits object for introspection on record fields.
 */
template <class RecordType>
struct RecordFieldTraits<1, RecordType>
{
	typedef I64u FieldType;
	typedef typename MethodTraits<Record, FieldType>::Getter GetterType;
	typedef typename MethodTraits<Record, FieldType>::Setter SetterType;

	static const char* name;

	static SetterType setter()
	{
		return static_cast<SetterType>(&Record::genIDRef);
	}

	static GetterType getter()
	{
		return static_cast<GetterType>(&Record::genIDRef);
	}
};

template <class RecordType>
const char* RecordFieldTraits<1, RecordType>::name = "gen_id";

} // namespace Myriad

#endif /* RECORD_H_ */
