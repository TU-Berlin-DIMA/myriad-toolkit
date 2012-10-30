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

#ifndef MULTIPLICATIVEGROUP_H_
#define MULTIPLICATIVEGROUP_H_

#include "core/types.h"
#include "math/util.h"

#include <Poco/Logger.h>
#include <Poco/Format.h>

#include <string>
#include <map>

using namespace Poco;
using namespace std;

namespace Myriad {
/**
 * @addtogroup math_algebra
 * @{*/

/**
 * An implementation of a multiplicative group modulo some prime number \p N.
 *
 * Multiplicative groups can be used to generate a premutated dense sequences of
 * numbers with a given cardinality.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 * @deprecated
 */
class MultiplicativeGroup
{
public:

    /**
     * An element of the multiplicative group with value semantics.
     *
     * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
     */
    class Entry
    {
    public:

        /**
         * Constructor.
         */
        Entry(I64u g = 1, I64u m = 1, I64u N = 1) :
            x(modexp(g, m, N)), g(g), m(m), N(N)
        {
        }

        /**
         * Converts an element to its string representation.
         */
        operator string()
        {
            return format("< x = %Lu, g = %Lu, m = %Lu, N = %Lu >", x, g, m, N);
        }

        /**
         * Converts an element to a 64-bit unsigned integer.
         */
        operator I64u();

        /**
         * Pre-increment operator.
         */
        Entry& operator ++();

        /**
         * Post-increment operator.
         */
        Entry operator ++(int);

        /**
         * Equality comparison operator (against another Entry).
         */
        bool operator ==(const Entry& other) const;

        /**
         * Equality comparison operator (against a 64-bit unsigned integer).
         */
        bool operator ==(const I64u& other) const;

        /**
         * Inequality comparison operator (against another Entry).
         */
        bool operator !=(const Entry& other) const;

        /**
         * Equality comparison operator (against a 64-bit unsigned integer).
         */
        bool operator !=(const I64u& other) const;

    private:

        I64u x;
        I64u g;
        I64u m;
        I64u N;
    };

    /**
     * An alias for iterator type.
     */
    typedef Entry iterator;
    /**
     * An alias for the map of known generators.
     */
    typedef map<I64u, I64u> GeneratorMap;

    /**
     * Default constructor.
     */
    MultiplicativeGroup(string name = "anonymoys") :
        _name(name), _logger(Logger::get("skgen."+name))
    {
        N = 1;
        g = 0;
    }

    /**
     * Default constructor.
     */
    MultiplicativeGroup(const MultiplicativeGroup& other) :
        _logger(Logger::get("skgen."+other._name))
    {
        _name = other._name;
        N = other.N;
        g = other.g;
    }

    /**
     * Cardinality constructor.
     *
     * @param cardinality The minimal number of elements in the group.
     * @param name The name of this multiplicative group instance.
     */
    MultiplicativeGroup(I64u cardinality, string name = "anonymoys") :
        _name(name), _logger(Logger::get("skgen."+name))
    {
        configure(cardinality);
    }

    /**
     * Converts the group to its string representation.
     */
    operator string()
    {
        return format("< N = %Lu, g = %Lu >", N, g);
    }

    /**
     * Returns the name of this object.
     */
    const string& name()
    {
        return _name;
    }

    /**
     * Configure a group to have a minimum ov \p cardinality elements.
     */
    void configure(I64u cardinality);

    /**
     * Functor operator.
     *
     * Returns the integer representation for the group entry for position \p m.
     *
     * @param m The index of the group element to be returned.
     */
    I64u operator ()(const I64u m) const;

    /**
     * Functor operator.
     *
     * Returns the group entry for position \p m.
     *
     * @param m The index of the group element to be returned.
     */
    Entry operator [](const I64u m);

private:

    static GeneratorMap GENERATORS;

    string _name;

    I64u g;
    I64u N;

    Logger& _logger;
};

inline I64u MultiplicativeGroup::operator ()(const I64u m) const
{
    return modexp(g, m, N);
}

inline MultiplicativeGroup::Entry MultiplicativeGroup::operator [](const I64u m)
{
    return Entry(g, m, N);
}

inline MultiplicativeGroup::Entry::operator I64u()
{
    return x;
}

inline MultiplicativeGroup::Entry& MultiplicativeGroup::Entry::operator ++()
{
    m++;
    x = (x * g) % N;

    return *this;
}

inline MultiplicativeGroup::Entry MultiplicativeGroup::Entry::operator ++(int)
{
    Entry temp = *this;
    ++*this;
    return temp;
}

inline bool MultiplicativeGroup::Entry::operator ==(const MultiplicativeGroup::Entry& o) const
{
    return x == o.x && g == o.g && m == o.m && N == o.N;
}

inline bool MultiplicativeGroup::Entry::operator ==(const I64u& o) const
{
    return x == o;
}

inline bool MultiplicativeGroup::Entry::operator !=(const MultiplicativeGroup::Entry& o) const
{
    return x != o.x || g != o.g || m != o.m || N != o.N;
}

inline bool MultiplicativeGroup::Entry::operator !=(const I64u& o) const
{
    return x != o;
}

/** @}*/// add to math_algebra group
} // namespace Myriad

#endif /* MULTIPLICATIVEGROUP_H_ */
