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

#ifndef RANDOMSEED_H_
#define RANDOMSEED_H_

#include <Poco/Exception.h>
#include <cstdlib>
#include <string>
#include <sstream>

using namespace std;
using namespace Poco;

// forward declarations;
template<class T> T readComponent(const string& s);
template<> int readComponent(const string& s);
template<> unsigned int readComponent(const string& s);
template<> unsigned long int readComponent(const string& s);

namespace Myriad {

/**
 * A random seed template. Basically, a random seed is a wrapper around a
 * fixed length vector of some integer type. The number of integers and the
 * length of the vector are RNG implemtation dependant.
 */
template<class T, unsigned int x> class RandomSeed
{
public:

	RandomSeed()
	{
		for (unsigned int i = 0; i < x; i++)
		{
			v[i] = 0;
		}
	}

	RandomSeed(T s[])
	{
		for (unsigned int i = 0; i < x; i++)
		{
			v[i] = s[i];
		}
	}

	RandomSeed(string seed)
	{
		seed.append(",");

		unsigned int i = 0;
		size_t l = 0, r;

		while (i < x && string::npos != (r = seed.find(',', l)))
		{
			v[i] = readComponent<T> (seed.substr(l, r - l));

			l = r + 1;
			i++;
		}
	}

	RandomSeed(const RandomSeed<T, x>& other)
	{
		for (unsigned int i = 0; i < x; i++)
		{
			v[i] = 0;
			v[i] = other.v[i];
		}
	}

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

	void reset()
	{
		for (unsigned int i = 0; i < x; i++)
		{
			v[i] = 0;
		}
	}

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

	T v[x];

private:

};

} // namespace Myriad

template<class T> inline T readComponent(const string& s)
{
	throw RuntimeException("Read component not supported");
}

template<> inline int readComponent(const string& s)
{
	return atoi(s.c_str());
}

template<> inline unsigned int readComponent(const string& s)
{
	return strtoul(s.c_str(), 0, 0);
}

template<> inline unsigned long int readComponent(const string& s)
{
	return strtoul(s.c_str(), 0, 0);
}

#endif /* RANDOMSEED_H_ */
