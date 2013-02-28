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

#ifndef RANDOMSEED_H_
#define RANDOMSEED_H_

#include <Poco/Exception.h>
#include <cstdlib>
#include <string>
#include <sstream>

using namespace std;
using namespace Poco;

namespace Myriad {
/**
 * @addtogroup math_random
 * @{*/

// forward declarations;
template<class T> T readComponent(const string& s);
template<> int readComponent<int>(const string& s);
template<> unsigned int readComponent<unsigned int>(const string& s);
template<> unsigned long int readComponent<unsigned long int>(const string& s);

/**
 * A generic template for RNG seeds.
 *
 * This is a wrapper around a fixed vector of length \p x of some integer type
 * \p T.
 */
template<class T, unsigned int x> class RandomSeed
{
public:

	/**
	 * Default constructor. Initializes all seed components to zero.
	 */
    RandomSeed()
    {
        for (unsigned int i = 0; i < x; i++)
        {
	        v[i] = 0;
        }
    }

	/**
	 * Vector initialization constructor.
	 *
	 * @param s A vector of initialization values for this seed.
	 */
    RandomSeed(T s[])
    {
        for (unsigned int i = 0; i < x; i++)
        {
	        v[i] = s[i];
        }
    }

	/**
	 * String initialization constructor.
	 *
	 * Initializes the components from a string of the form "{c1},...,{cn}".
	 *
	 * @param seed A comma separated contatenation of the string component
	 *             values.
	 */
    RandomSeed(string seed)
    {
        seed.append(",");

        for (unsigned int i = 0; i < x; i++)
        {
	        v[i] = 0;
        }

        unsigned int i = 0;
        size_t l = 0, r;
        while (i < x && string::npos != (r = seed.find(',', l)))
        {
	        v[i] = readComponent<T> (seed.substr(l, r - l));

	        l = r + 1;
	        i++;
        }

        if (i < x)
        {
	        throw RuntimeException("Not enough random seed components found");
        }
    }

	/**
	 * Copy constructor.
	 *
	 * @param other A comma separated contatenation of the string component
	 *              values.
	 */
    RandomSeed(const RandomSeed<T, x>& other)
    {
        for (unsigned int i = 0; i < x; i++)
        {
	        v[i] = 0;
	        v[i] = other.v[i];
        }
    }

	/**
	 * Assignment operator.
	 */
    RandomSeed<T, x>& operator =(const RandomSeed<T, x>& other)
    {
        if (this != &other) // protect against invalid self-assignment
        {
	        for (unsigned int i = 0; i < x; i++)
	        {
		        v[i] = other.v[i];
	        }
        }

        return *this;
    }

	/**
	 * Value equality operator.
	 */
    bool operator ==(const RandomSeed<T, x>& other) const
    {
        for (unsigned int i = 0; i < x; i++)
        {
	        if (v[i] != other.v[i])
	        {
		        return false;
	        }
        }

        return true;
    }

    /**
     * Reset all components to zero.
     */
    void reset()
    {
        for (unsigned int i = 0; i < x; i++)
        {
	        v[i] = 0;
        }
    }

    /**
     * Output a comma separated string representation of all string components.
     */
    const std::string toString() const
    {
        std::stringstream ss;

        for (unsigned int i = 0; i < x-1; i++)
        {
	        ss << v[i] << ",";
        }
        ss << v[x-1];

        return ss.str();
    }

    /**
     * The value for the seed.
     */
    T v[x];

};

/**
 * A generic method template for reading seed components of type \p T.
 *
 * The default implementation throws a Poco::RuntimeException.
 */
template<class T> inline T readComponent(const string& s)
{
    throw RuntimeException("Read component not supported");
}

/**
 * A template specialization for reading <tt>int</tt> seed components.
 */
template<> inline int readComponent<int>(const string& s)
{
    return atoi(s.c_str());
}

/**
 * A template specialization for reading <tt>unsigned int</tt>  seed components.
 */
template<> inline unsigned int readComponent<unsigned int>(const string& s)
{
    return strtoul(s.c_str(), 0, 0);
}

/**
 * A template specialization for reading <tt>unsigned long int</tt>  seed
 * components.
 */
template<> inline unsigned long int readComponent<unsigned long int>(const string& s)
{
    return strtoul(s.c_str(), 0, 0);
}

/** @}*/// add to math_random group
} // namespace Myriad

#endif /* RANDOMSEED_H_ */
