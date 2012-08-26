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

#ifndef SURROGATEKEYGENERATOR_H_
#define SURROGATEKEYGENERATOR_H_

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
 * The ID generator is used to generate a permutated dense sequence of numbers
 * with a given cardinality N.
 *
 * TODO: rename to MultiplicativeGroup
 *
 */
class SurrogateKeyGenerator
{
public:

    class Entry;
    typedef Entry iterator;
    typedef map<I64u, I64u> GeneratorMap;

    SurrogateKeyGenerator(string name = "anonymoys") :
        _name(name), _logger(Logger::get("skgen."+name))
    {
        N = 1;
        g = 0;
    }

    SurrogateKeyGenerator(const SurrogateKeyGenerator& other) :
        _logger(Logger::get("skgen."+other._name))
    {
        _name = other._name;
        N = other.N;
        g = other.g;
    }

    SurrogateKeyGenerator(I64u cardinality, string name = "anonymoys") :
        _name(name), _logger(Logger::get("skgen."+name))
    {
        configure(cardinality);
    }

    operator string()
    {
        return format("< N = %Lu, g = %Lu >", N, g);
    }

    const string& name()
    {
        return _name;
    }

    void configure(I64u cardinality);

    I64u operator ()(const I64u m) const;

    Entry operator [](const I64u m);

    class Entry
    {
    public:

        Entry(I64u g = 1, I64u m = 1, I64u N = 1) :
	        x(modexp(g, m, N)), g(g), m(m), N(N)
        {
        }

        operator string()
        {
	        return format("< x = %Lu, g = %Lu, m = %Lu, N = %Lu >", x, g, m, N);
        }

        operator I64u();

        Entry& operator ++();

        Entry operator ++(int);

        bool operator ==(const Entry& other) const;

        bool operator ==(const I64u& other) const;

        bool operator !=(const Entry& other) const;

        bool operator !=(const I64u& other) const;

    private:

        I64u x;
        I64u g;
        I64u m;
        I64u N;
    };

private:

    static GeneratorMap GENERATORS;

    string _name;

    I64u g;
    I64u N;

    Logger& _logger;
};

inline I64u SurrogateKeyGenerator::operator ()(const I64u m) const
{
    return modexp(g, m, N);
}

inline SurrogateKeyGenerator::Entry SurrogateKeyGenerator::operator [](const I64u m)
{
    return Entry(g, m, N);
}

inline SurrogateKeyGenerator::Entry::operator I64u()
{
    return x;
}

inline SurrogateKeyGenerator::Entry& SurrogateKeyGenerator::Entry::operator ++()
{
    m++;
    x = (x * g) % N;

    return *this;
}

inline SurrogateKeyGenerator::Entry SurrogateKeyGenerator::Entry::operator ++(int)
{
    Entry temp = *this;
    ++*this;
    return temp;
}

inline bool SurrogateKeyGenerator::Entry::operator ==(const SurrogateKeyGenerator::Entry& o) const
{
    return x == o.x && g == o.g && m == o.m && N == o.N;
}

inline bool SurrogateKeyGenerator::Entry::operator ==(const I64u& o) const
{
    return x == o;
}

inline bool SurrogateKeyGenerator::Entry::operator !=(const SurrogateKeyGenerator::Entry& o) const
{
    return x != o.x || g != o.g || m != o.m || N != o.N;
}

inline bool SurrogateKeyGenerator::Entry::operator !=(const I64u& o) const
{
    return x != o;
}

// TODO: rename to MultiplicativeGroup (for now as typedef)
typedef SurrogateKeyGenerator MultiplicativeGroup;

} // namespace Myriad

#endif /* SURROGATEKEYGENERATOR_H_ */
