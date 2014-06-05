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

};

// all ValueTypes permitted except MyriadTuple
template<typename ValueType1, typename ValueType2>
class MyriadTuple: public MyriadAbstractTuple
{
public:
	MyriadTuple(){}

	MyriadTuple(ValueType1 value1, ValueType2 value2)
	{
		this->first = value1;
		this->second = value2;
	}

	ValueType1 getFirst(){
		return first;
	}

	ValueType2 getSecond()
	{
		return second;
	}
private:
	ValueType1 first;
	ValueType2 second;

};

// all ValueTypes permitted except MyriadTuple
template<typename ValueType1, typename ValueType2, typename ValueType3>
class MyriadTriple: public MyriadAbstractTuple
{
public:

	MyriadTriple(){}

	MyriadTriple(ValueType1 value1, ValueType2 value2, ValueType3 value3)
	{
		this->first = value1;
		this->second = value2;
		this->third = value3;
	}
	ValueType1 getFirst(){
		return first;
	}

	ValueType2 getSecond()
	{
		return second;
	}

	ValueType3 getThird()
	{
		return third;
	}

private:

	//MyriadTriple(const MyriadTuple& mt){}
	ValueType1 first;
	ValueType2 second;
	ValueType3 third;
};

//@}

/** @}*/// add to core group
} // namespace Myriad

#endif /* MYRIADABSTRACTTUPLE_H_ */

