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

#ifndef TYPES_H_
#define TYPES_H_

#include "types/MyriadDate.h"

#include <Poco/Exception.h>
#include <Poco/Types.h>

#include <stdint.h>
#include <algorithm>
#include <cctype>
#include <functional>
#include <limits>
#include <locale>
#include <sstream>
#include <string>

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// type aliasees
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

typedef Poco::Int16 I16;
typedef Poco::Int32 I32;
typedef Poco::Int64 I64;
typedef Poco::UInt16 I16u;
typedef Poco::UInt32 I32u;
typedef Poco::UInt64 I64u;
typedef std::size_t Enum;

typedef Poco::UInt64 ID;
typedef double Decimal;
typedef MyriadDate Date;
typedef std::string String;

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// explicit null type constants
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

struct NullValue
{
    static const I16  SHORT;
    static const I32  INTEGER;
    static const I64  BIGINTEGER;
    static const I16u USHORT;
    static const I32u UINTEGER;
    static const I64u UBIGINTEGER;
    static const Decimal DECIMAL;
    static const Date DATE;
    static const String STRING;
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// method type traits
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

// TODO: rename to MethodTypeTraits
template<class RecordType, class T> struct MethodTraits
{
    typedef const T& (RecordType::*Getter)() const;
    typedef void (RecordType::*Setter)(const T&);
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// null value template
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename T> inline const T& nullValue()
{
    throw Poco::RuntimeException("Unsupported null value for this type");
}

template<> inline const I16& nullValue<I16>()
{
    return NullValue::SHORT;
}

template<> inline const I32& nullValue<I32>()
{
    return NullValue::INTEGER;
}

template<> inline const I64& nullValue<I64>()
{
    return NullValue::BIGINTEGER;
}

template<> inline const I16u& nullValue<I16u>()
{
    return NullValue::USHORT;
}

template<> inline const I32u& nullValue<I32u>()
{
    return NullValue::UINTEGER;
}

template<> inline const I64u& nullValue<I64u>()
{
    return NullValue::UBIGINTEGER;
}

template<> inline const Decimal& nullValue<Decimal>()
{
    return NullValue::DECIMAL;
}

template<> inline const Date& nullValue<Date>()
{
    return NullValue::DATE;
}

template<> inline const String& nullValue<String>()
{
    return NullValue::STRING;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// numericLimits
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename T> class numericLimits
{
public:
    static T min()
    {
        return std::numeric_limits<T>::min();
    }

    static T max()
    {
        return std::numeric_limits<T>::max();
    }
};

template<> class numericLimits<Date>
{
public:
    static Date min()
    {
        return Date(DateTime(1, 1, 1));
    }

    static Date max()
    {
        return Date(DateTime(9999, 12, 31));
    }
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// string conversion templates
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class T> inline std::string toString(const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

template<class T> inline T fromString(const std::string& s)
{
    std::stringstream ss(s);
    T t;
    ss >> t;
    return t;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// serialization helpers
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class T> inline void write(std::ostream& stream, const T& t, bool quoted = true)
{
    if (t != nullValue<T>())
    {
        stream << t;
    }
    else
    {
        stream << "NULL";
    }
}

template<> inline void write<String>(std::ostream& stream, const String& t, bool quoted)
{
    if (t != nullValue<String>())
    {
        if (quoted)
        {
            stream << '"' << t << '"';
        }
        else
        {
            stream << t;
        }
    }
    else
    {
        stream << "NULL";
    }
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// string trim helpers
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

// trim from start
static inline std::string& ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string& rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string& trim(std::string& s)
{
    return ltrim(rtrim(s));
}


} // namespace Myriad

#endif /* TYPES_H_ */
