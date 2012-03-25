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

#ifndef NATIVERANDOMSTREAM_H_
#define NATIVERANDOMSTREAM_H_

#include <cstdlib>
#include <cmath>
#include <limits>

#include "math/random/EICG.h"
#include "math/random/RNG.h"

using namespace std;

namespace Myriad {

class NativeRandomStream;

template<> struct prng_traits<NativeRandomStream>
{
	typedef RandomSeed<UInt32, 1> seed_type;
};

class NativeRandomStream: public HierarchicalRNG
{
public:

	typedef prng_traits<NativeRandomStream>::seed_type Seed;

	/**
	 * Computational constant.
	 */
	static const double D_2_POW_NEG_64;

	NativeRandomStream(const string name = "anonymous") :
		_name(name)
	{
		initialize();
		srand(_masterS.v[0]);
	}

	NativeRandomStream(const NativeRandomStream& o, const string name = "anonymous") :
		_name(name),
		_masterS(o._masterS)
	{
		initialize();
		srand(_masterS.v[0]);
	}

	~NativeRandomStream()
	{
	}

	NativeRandomStream& operator =(const NativeRandomStream& o)
	{
		if (this != &o) // protect against invalid self-assignment
		{
			_masterS = o._masterS;
		}

		srand(_masterS.v[0]);

		return *this;
	}

	// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
	// RandomStream: TODO: add to NativeRandomStream interface
	// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

	double operator()()
	{
		return next();
	}

	template<class T> T operator()(T min, T max)
	{
		return min + static_cast<T> ((max - min + 1.0) * next());
	}

	// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
	// NativeRandomStream interface
	// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

	void seed(Seed masterSeed)
	{
		_masterS = masterSeed;
		srand(masterSeed.v[0]);
	}

	const Seed& seed() const
	{
		return _masterS;
	}

	double next()
	{
		return rand()/RAND_MAX;
	}

	double at(UInt64 pos)
	{
		throw Poco::NotImplementedException("at() is not supported by the NativeRandomStream");
	}

	// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
	// HierarchicalRNG interface
	// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

	NativeRandomStream& nextSubstream()
	{
		// ignore method call
		return *this;
	}

	NativeRandomStream& resetSubstream()
	{
		// ignore method call
		return *this;
	}

	NativeRandomStream& nextChunk()
	{
		// ignore method call
		return *this;
	}

	NativeRandomStream& resetChunk()
	{
		throw Poco::NotImplementedException("resetChunk() is not supported by the NativeRandomStream");
	}

	NativeRandomStream& atChunk(UInt64 pos)
	{
		// ignore method call
		return *this;
	}

	void dumpState()
	{
		std::cout << "master:     [ " << _masterS.v[0]    << " ]" << std::endl;
	}

private:

	/**
	 * The generator name
	 */
	const string _name;
	/**
	 * Seed representing the initial position on the RNG cycle.
	 */
	Seed _masterS;

	/**
	 * Common object initialization logic called from all constructors.
	 */
	void initialize();
};

inline void NativeRandomStream::initialize()
{
}

} // namespace Myriad

#endif /* NATIVERANDOMSTREAM_H_ */
