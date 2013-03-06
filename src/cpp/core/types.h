/*
 * Copyright 2010-2013 DIMA Research Group, TU Berlin
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

#ifndef TYPES_H_
#define TYPES_H_

#include "core/types/MyriadDate.h"
#include "core/types/MyriadEnumSet.h"

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

typedef bool Bool;          //!< Date, alias of \c bool
typedef char Char;          //!< A character type (alias of \c char)
typedef MyriadDate Date;    //!< Date, alias of \c MyriadDate
typedef double Decimal;     //!< Decimal number, alias of \c double
typedef std::size_t Enum;   //!< A enumerated sequence type (alias of \c std::size_t)
typedef Poco::Int16 I16;    //!< Unsigned 16-bit integer
typedef Poco::Int32 I32;    //!< Unsigned 32-bit integer
typedef Poco::Int64 I64;    //!< Unsigned 64-bit integer
typedef Poco::UInt16 I16u;  //!< Signed 16-bit integer
typedef Poco::UInt32 I32u;  //!< Signed 32-bit integer
typedef Poco::UInt64 I64u;  //!< Signed 64-bit integer
typedef std::string String; //!< String type, alias of \c std::string
//TODO: remove this type, use I64u instead
typedef Poco::UInt64 ID;    //!< Generic ID type, alias of \c I64u

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
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
// TODO: rename to MethodTypeTraits
// TODO: move to Record.h
template<class RecordType, class T> struct MethodTraits
{
    // getter / setter signatures in record types
    typedef const T& (RecordType::*RefGetter)() const; //!< Get by reference signature.
    typedef void (RecordType::*RefSetter)(const T&); //!< Set by reference signature.
    typedef const T (RecordType::*ValGetter)() const; //!< Get by value signature.
    typedef void (RecordType::*ValSetter)(const T); //!< Set by value signature.
    // getter / setter signatures in record range predicate types
    typedef const Interval<T>& (RecordType::*RangeGetter)() const; //!< Range getter signature.
    typedef void (RecordType::*RangeSetterLong)(T, T); //!< Range setter signature (long version).
    typedef void (RecordType::*RangeSetterShort)(T); //!< Range setter signature (short version).
};

//@}

////////////////////////////////////////////////////////////////////////////////
/// @name NULL Values
////////////////////////////////////////////////////////////////////////////////
//@{

/**
 * A structure containing explicit null type constants.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
struct NullValue
{
    static const I16  SHORT; //!< NULL constant for the I16 type.
    static const I32  INTEGER; //!< NULL constant for the I32 type.
    static const I64  BIGINTEGER; //!< NULL constant for the I64 type.
    static const I16u USHORT; //!< NULL constant for the I16u type.
    static const I32u UINTEGER; //!< NULL constant for the I32u type.
    static const I64u UBIGINTEGER; //!< NULL constant for the I64u type.
    static const Decimal DECIMAL; //!< NULL constant for the Decimal type.
    static const Char CHAR; //!< NULL constant for the Char type.
    static const Date DATE; //!< NULL constant for the Date type.
    static const String STRING; //!< NULL constant for the String type.
};

/**
 * A function template for retrieveing type-specific \c NULL values.
 *
 * The default implementation throws a \c Poco::RuntimeException.
 */
template<typename T> inline const T& nullValue()
{
    throw Poco::RuntimeException("Unsupported null value for this type");
}

/**
 * Returns the \c NULL value representation of the \c I16 type.
 */
template<> inline const I16& nullValue<I16>()
{
    return NullValue::SHORT;
}

/**
 * Returns the \c NULL value representation of the \c I32 type.
 */
template<> inline const I32& nullValue<I32>()
{
    return NullValue::INTEGER;
}

/**
 * Returns the \c NULL value representation of the \c I64 type.
 */
template<> inline const I64& nullValue<I64>()
{
    return NullValue::BIGINTEGER;
}

/**
 * Returns the \c NULL value representation of the \c I16u type.
 */
template<> inline const I16u& nullValue<I16u>()
{
    return NullValue::USHORT;
}

/**
 * Returns the \c NULL value representation of the \c I32u type.
 */
template<> inline const I32u& nullValue<I32u>()
{
    return NullValue::UINTEGER;
}

/**
 * Returns the \c NULL value representation of the \c I64u type.
 */
template<> inline const I64u& nullValue<I64u>()
{
    return NullValue::UBIGINTEGER;
}

/**
 * Returns the \c NULL value representation of the \c Decimal type.
 */
template<> inline const Decimal& nullValue<Decimal>()
{
    return NullValue::DECIMAL;
}

/**
 * Returns the \c NULL value representation of the \c Char type.
 */
template<> inline const Char& nullValue<Char>()
{
    return NullValue::CHAR;
}

/**
 * Returns the \c NULL value representation of the \c Date type.
 */
template<> inline const Date& nullValue<Date>()
{
    return NullValue::DATE;
}

/**
 * Returns the \c NULL value representation of the \c I16u type.
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
 * A template class with two static methods \c min() and \c max() for retrieving
 * type-specific numeric limits.
 *
 * The default implementation returns the <tt>std::numeric_limits</tt> \c min()
 * and \c max() values for \p T.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<typename T> class numericLimits
{
public:

    /**
     * The minimum possible value for the type \p T.
     */
    static T min()
    {
        return std::numeric_limits<T>::min();
    }

    /**
     * The maximum possible value for the type \p T.
     */
    static T max()
    {
        return std::numeric_limits<T>::max();
    }
};

/**
 * Template specialization for the Myriad-specicif Date type.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<> class numericLimits<Date>
{
public:

    /**
     * The minimum possible value for a Date.
     */
    static Date min()
    {
        return Date(DateTime(1, 1, 1));
    }

    /**
     * The maximum possible value for a Date.
     */
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
 * Transforms a \p T value to as a string using a std::stringstream instance.
 */
template<class T> inline std::string toString(const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

/**
 * Reads a \p T value from a given string using a \c std::stringstream instance.
 */
template<class T> inline T fromString(const std::string& s)
{
    std::stringstream ss(s);
    T t;
    ss >> t;
    return t;
}

template<> inline char fromString<char>(const std::string& s)
{
    if (s.length() < 1) {
        throw RuntimeException("Cannot read character from empty string");
    } else {
        return s[0];
    }
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
 * Write out to the given \c std::ostream either the value of \p t or, if \p t
 * is \c NULL, the string literal 'NULL'. The \p quoted parameter is ignored.
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

/**
 * A specialization of the \p write function that also respects the \p quoted
 * parameter.
 */
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

/**
 * Write out to the given \c std::ostream either the value vector \p t.
 * The \p quoted parameter is passed to the write() call of each vector element.
 */
template<class T> inline void write(std::ostream& stream, const vector<T>& t, bool quoted = true)
{
    stream << "[";
    typename vector<T>::const_iterator it = t.begin();
    while (it != t.end())
    {
        write(stream, (*it), quoted);
        it++;

        if  (it != t.end())
        {
            stream << ", ";
        }
    }
    stream << "]";
}

/**
 * Write out to the given \c std::ostream character vector \p t as a character
 * sequence. If \p quoted parameter is <tt>True</tt>, adds enclosing quotes to
 * the character sequence.
 */
template<> inline void write(std::ostream& stream, const vector<Char>& t, bool quoted)
{
    if (quoted)
    {
        stream << '"';
    }

    vector<Char>::const_iterator it = t.begin();
    while (it != t.end())
    {
        stream << (*it);
        it++;
    }

    if (quoted)
    {
        stream << '"';
    }
}

//@}

/** @}*/// add to core group
} // namespace Myriad

// explicitly enable direct use of the Myriad types in the common namespace
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

#endif /* TYPES_H_ */
