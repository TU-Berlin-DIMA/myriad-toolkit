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
 * @author: Alexander Alexandrov <alexander.s.alexandrov@campus.tu-berlin.de>
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
#include <sstream>
#include <string>

#include <Poco/Types.h>

namespace Myriad {

typedef Poco::Int16 I16;
typedef Poco::Int32 I32;
typedef Poco::Int64 I64;
typedef Poco::UInt16 I16u;
typedef Poco::UInt32 I32u;
typedef Poco::UInt64 I64u;

typedef Poco::UInt64 ID;
typedef double Decimal;
typedef std::string Ch; // deprecated
typedef std::string VCh; // deprecated
typedef std::string String;

template<class T> inline std::string toString(const T& t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}

template<class T> T fromString(const std::string& s)
{
	std::stringstream ss(s);
	T t;
	ss >> t;
	return t;
}

template<typename T> T toEnum(int v);

} // namespace Myriad

#endif /* TYPES_H_ */
