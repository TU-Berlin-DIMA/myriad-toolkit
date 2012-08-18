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
/**
 * @addtogroup core
 * @{*/

////////////////////////////////////////////////////////////////////////////////
/// @name Type Aliases
////////////////////////////////////////////////////////////////////////////////
//@{

typedef Poco::Int16 I16;    //!< Unsigned 16-bit integer
typedef Poco::Int32 I32;    //!< Unsigned 32-bit integer
typedef Poco::Int64 I64;    //!< Unsigned 64-bit integer
typedef Poco::UInt16 I16u;  //!< Signed 16-bit integer
typedef Poco::UInt32 I32u;  //!< Signed 32-bit integer
typedef Poco::UInt64 I64u;  //!< Signed 64-bit integer
typedef std::size_t Enum;   //!< A enumerated sequence type (alias of std::size_t)
typedef double Decimal;     //!< Decimal number, alias of double
typedef MyriadDate Date;    //!< Date, alias of MyriadDate
//TODO: remove this type, use I64u instead
typedef Poco::UInt64 ID;    //!< Generic ID type, alias of I64u
typedef std::string String; //!< String type, alias of std::string

// forward declarations of auxiliary complex types
template<typename T> class Interval;

//@}

////////////////////////////////////////////////////////////////////////////////
/// @name Traits Structures
////////////////////////////////////////////////////////////////////////////////
//@{

/**
 * A traits object for reflective inspection of Record methods.
 *
 * TODO: rename to MethodTypeTraits
 * TODO: move to Record.h
 */
template<class RecordType, class T> struct MethodTraits
{
    // getter / setter signatures in record types
    typedef const T& (RecordType::*Getter)() const;
    typedef void (RecordType::*Setter)(const T&);
    // getter / setter signatures in record range predicate types
    typedef const Interval<T>& (RecordType::*RangeGetter)() const;
    typedef void (RecordType::*RangeSetterLong)(T, T);
    typedef void (RecordType::*RangeSetterShort)(T);
};

//@}

////////////////////////////////////////////////////////////////////////////////
/// @name Type Metainformation
////////////////////////////////////////////////////////////////////////////////
//@{

/**
 * A structure containing explicit null type constants.
 */
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

/**
 * A function template for retrieveing type-specific NULL values.
 *
 * The default implementation throws a Poco::RuntimeException.
 */
template<typename T> inline const T& nullValue()
{
    throw Poco::RuntimeException("Unsupported null value for this type");
}

/**
 * Returns the NULL value representation of the I16 type.
 */
template<> inline const I16& nullValue<I16>()
{
    return NullValue::SHORT;
}

/**
 * Returns the NULL value representation of the I32 type.
 */
template<> inline const I32& nullValue<I32>()
{
    return NullValue::INTEGER;
}

/**
 * Returns the NULL value representation of the I64 type.
 */
template<> inline const I64& nullValue<I64>()
{
    return NullValue::BIGINTEGER;
}

/**
 * Returns the NULL value representation of the I16u type.
 */
template<> inline const I16u& nullValue<I16u>()
{
    return NullValue::USHORT;
}

/**
 * Returns the NULL value representation of the I32u type.
 */
template<> inline const I32u& nullValue<I32u>()
{
    return NullValue::UINTEGER;
}

/**
 * Returns the NULL value representation of the I64u type.
 */
template<> inline const I64u& nullValue<I64u>()
{
    return NullValue::UBIGINTEGER;
}

/**
 * Returns the NULL value representation of the Decimal type.
 */
template<> inline const Decimal& nullValue<Decimal>()
{
    return NullValue::DECIMAL;
}

/**
 * Returns the NULL value representation of the Date type.
 */
template<> inline const Date& nullValue<Date>()
{
    return NullValue::DATE;
}

/**
 * Returns the NULL value representation of the I16u type.
 */
template<> inline const String& nullValue<String>()
{
    return NullValue::STRING;
}

//@}

////////////////////////////////////////////////////////////////////////////////
/// @name Numeric Limits
////////////////////////////////////////////////////////////////////////////////
//@{

/**
 * A template class with two static methods min() and max() for retrieving
 * type-specific numeric limits.
 *
 * The default implementation returns the std::numeric_limits min() and
 * max() values for T.
 */
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

/**
 * Template specialization for the Myriad-specicif Date type.
 */
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

//@}

////////////////////////////////////////////////////////////////////////////////
/// @name String Helpers
////////////////////////////////////////////////////////////////////////////////
//@{

/**
 * Transforms a T value to as a string using a \ref std::stringstream instance.
 */
template<class T> inline std::string toString(const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

/**
 * Reads a T value from a given string using a \ref std::stringstream instance.
 */
template<class T> inline T fromString(const std::string& s)
{
    std::stringstream ss(s);
    T t;
    ss >> t;
    return t;
}

/**
 * Trims the left-hand side of a string.
 */
static inline std::string& ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

/**
 * Trims the right-hand side of a string.
 */
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

//@}

////////////////////////////////////////////////////////////////////////////////
/// @name Record Field Serialization Helpers
////////////////////////////////////////////////////////////////////////////////
//@{

/**
 * Write out to the given \ref stream either the value of t or, if t is NULL,
 * the literal 'NULL'.
 */
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

//@}

/** @}*/
} // namespace Myriad

#endif /* TYPES_H_ */
