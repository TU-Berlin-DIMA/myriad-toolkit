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

#ifndef MYRIADABSTRACTTUPLE_H_
#define MYRIADABSTRACTTUPLE_H_


#include <iostream>

using namespace std;
using namespace Poco;

namespace Myriad {
/**
 * @addtogroup core
 * @{*/


/**
 * A class representing a generic tuple.
 *
 *
 *
 */
class MyriadAbstractTuple
{
public:
    /**
     * Default constructor.
     */
    MyriadAbstractTuple(){}
    virtual ~MyriadAbstractTuple(){};
    virtual size_t getDim() = 0;
   // virtual bool operator == (const MyriadAbstractTuple& t) const = 0;

};

// all ValueTypes permitted except MyriadTuple
template<typename ValueType1, typename ValueType2>
class MyriadTuple: public MyriadAbstractTuple
{
public:
//	typename ValueType1  VType1;
//	typename ValueType2  VType2;

	MyriadTuple(): _dim(0){}

	MyriadTuple(ValueType1 value1, ValueType2 value2): _dim(2)
	{
		this->_first = value1;
		this->_second = value2;
	}
	// copy constructor
	MyriadTuple(const MyriadTuple& t):
		_first(t._first),
		_second(t._second)
	{}

	virtual ~MyriadTuple(){};

	template<typename ValueType>
	ValueType elementAt(size_t index){
		switch (index) {
		case 1: return getFirst();
		case 2: return getSecond();
		}
	}

	size_t getDim(){
		return _dim;
	}
	ValueType1 getFirst(){
		return _first;
	}

	ValueType2 getSecond()
	{
		return _second;
	}

	bool operator == (const MyriadTuple& t) const
	{
		return (this->_first == t._first && this->_second == t._second);
	}


private:

	ValueType1 _first;
	ValueType2 _second;
	size_t _dim;
};

// all ValueTypes permitted except MyriadTuple
template<typename ValueType1, typename ValueType2, typename ValueType3>
class MyriadTriple: public MyriadAbstractTuple
{
public:

	MyriadTriple(): _dim(3){}

	MyriadTriple(ValueType1 value1, ValueType2 value2, ValueType3 value3) : _dim(3)
	{
		this->_first = value1;
		this->_second = value2;
		this->_third = value3;
	}

	virtual ~MyriadTriple(){};

	ValueType1 getFirst(){
		return _first;
	}

	ValueType2 getSecond()
	{
		return _second;
	}

	ValueType3 getThird()
	{
		return _third;
	}

	size_t getDim(){
		return _dim;
	}

	template<typename ValueType>
	ValueType elementAt(size_t index){
		switch (index) {
		case 1: return getFirst();
		case 2: return getSecond();
		case 3: return getThird();
		// TODO: default: raise Exception
		}
	}

	bool operator == (const MyriadTriple& t) const
	{
		return (this->_first == t._first && this->_second == t._second && this->_third == t._third);
	}

private:

	//MyriadTriple(const MyriadTuple& mt){}
	ValueType1 _first;
	ValueType2 _second;
	ValueType3 _third;
	size_t _dim;
};

//@}

/** @}*/// add to core group
} // namespace Myriad

#endif /* MYRIADABSTRACTTUPLE_H_ */

