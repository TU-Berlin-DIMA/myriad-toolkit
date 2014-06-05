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

#ifndef HASHRANDOMSTREAM_H_
#define HASHRANDOMSTREAM_H_

#include <cmath>
#include <limits>

#include "math/random/EICG.h"
#include "math/random/RNG.h"

using namespace std;

namespace Myriad {
/**
 * @addtogroup math_random
 * @{*/

// forward declarations
class HashRandomStream;

/**
 * Traits specialization for the HashRandomStream random stream.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<> struct prng_traits<HashRandomStream>
{
    typedef RandomSeed<UInt64, 1> seed_type; //!< The concrete seed type
};

/**
 * A hash random stream implementation of a HierarchicalRNG.
 *
 * This is a port of the RNG used by Rabl et al. in the Parallel Data
 * Generator Framework (PDGF).
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class HashRandomStream: public HierarchicalRNG
{
public:

	/**
	 * An alias of the seed type for this RNG implementation.
	 */
    typedef prng_traits<HashRandomStream>::seed_type Seed;

    /**
     * Substream offset.
     */
    static const Seed OFFSET_SUBSTREAM;
    /**
     * Chunk offset.
     */
    static const Seed OFFSET_CHUNK;
    /**
     * Offset element.
     */
    static const Seed OFFSET_ELEMENT;
    /**
     * Computational constant.
     */
    static const double D_2_POW_NEG_64;

    /**
     * Default constructor.
     */
    HashRandomStream(const string name = "anonymous") :
        _name(name)
    {
        initialize();
    }

    /**
     * Copy constructor.
     */
    HashRandomStream(const HashRandomStream& o, const string name = "anonymous") :
        _name(name),
        _masterS(o._masterS),
        _substreamS(o._substreamS),
        _chunkS(o._chunkS),
        _elementS(o._elementS)
    {
        initialize();
    }

    /**
     * Destructor.
     */
    ~HashRandomStream()
    {
    }

    /**
     * Assignment operator.
     */
    HashRandomStream& operator =(const HashRandomStream& o)
    {
        if (this != &o) // protect against invalid self-assignment
        {
	        _masterS = o._masterS;
	        _substreamS = o._substreamS;
	        _chunkS = o._chunkS;
	        _elementS = o._elementS;
	        _currentHash = computeHash();
        }

        return *this;
    }

    /**
     * Adjust the random stream to the given seed.
     */
    void seed(Seed masterSeed)
    {
        _masterS = _substreamS = _chunkS = _elementS = masterSeed;
        _currentHash = computeHash();
    }

    /**
     * Get the current element seed.
     */
    const Seed& seed() const
    {
        return _elementS;
    }

    /**
     * Dump the current RNG state to the <tt>std::cout</tt> stream.
     */
    void dumpState()
    {
        std::cout << "element   : [ " << _elementS.v[0]   << " ]" << std::endl;
        std::cout << "chunk     : [ " << _chunkS.v[0]     << " ]" << std::endl;
        std::cout << "substream : [ " << _substreamS.v[0] << " ]" << std::endl;
        std::cout << "master:     [ " << _masterS.v[0]    << " ]" << std::endl;
    }

    // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
    // RNG interface
    // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

    /**
     * @see RNG::next()
     */
    double next()
    {
        double next = _currentHash * D_2_POW_NEG_64;

        appendToSeed(_elementS, _elementS, OFFSET_ELEMENT);
        _currentHash = computeHash();

        return next;
    }

    /**
     * @see RNG::at()
     */
    double at(UInt64 pos)
    {
        appendToSeed(_elementS, _chunkS, OFFSET_ELEMENT, pos);
        _currentHash = computeHash();

        return next();
    }

    /**
     * @see RNG::skip()
     */
    void skip(UInt64 pos)
    {
        appendToSeed(_elementS, _elementS, OFFSET_ELEMENT, pos);
        _currentHash = computeHash();
    }

    // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
    // HierarchicalRNG interface
    // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

    /**
     * @see HierarchicalRNG::nextSubstream()
     */
    HashRandomStream& nextSubstream()
    {
        appendToSeed(_substreamS, _substreamS, OFFSET_SUBSTREAM);
        _elementS = _chunkS = _substreamS;
        _currentHash = computeHash();

        return *this;
    }

    /**
     * @see HierarchicalRNG::resetSubstream()
     */
    HashRandomStream& resetSubstream()
    {
        _elementS = _chunkS = _substreamS;
        _currentHash = computeHash();

        return *this;
    }

    /**
     * @see HierarchicalRNG::nextChunk()
     */
    HashRandomStream& nextChunk()
    {
        appendToSeed(_chunkS, _chunkS, OFFSET_CHUNK);
        _elementS = _chunkS;
        _currentHash = computeHash();

        return *this;
    }

    /**
     * @see HierarchicalRNG::resetChunk()
     */
    HashRandomStream& resetChunk()
    {
        _elementS = _chunkS;
        _currentHash = computeHash();

        return *this;
    }

    /**
     * @see HierarchicalRNG::atChunk()
     */
    HashRandomStream& atChunk(UInt64 pos)
    {
        appendToSeed(_chunkS, _substreamS, OFFSET_CHUNK, pos);
        _elementS = _chunkS;
        _currentHash = computeHash();

        return *this;
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
     * Seed representing the first position of the current substream RNG.
     */
    Seed _substreamS;
    /**
     * Seed representing the first position of the current substream RNG.
     */
    Seed _chunkS;
    /**
     * Current seed for this RNG.
     */
    Seed _elementS;
    /**
     * Hash for the current master seed.
     */
    UInt64 _currentHash;

    /**
     * Common object initialization logic called from all constructors.
     */
    void initialize();
    /**
     * Computes the hash function for the current position (seed).
     */
    UInt64 computeHash();
    /**
     * Increments the seed.
     */
    void adjustSeed();
    /**
     * Computes y = o + x on the seed.
     */
    void appendToSeed(Seed& y, const Seed& o, const Seed& x);
    /**
     * Computes y = o + f*x on the seed.
     */
    void appendToSeed(Seed& y, const Seed& o, const Seed& x, const Int64 f);
};

inline void HashRandomStream::initialize()
{
    _currentHash = computeHash();
}

inline UInt64 HashRandomStream::computeHash()
{
    UInt64 x = _elementS.v[0];

    x = 3935559000370003845LL * x + 2691343689449507681LL;
    x = x ^ (x >> 21);
    x = x ^ (x << 37);
    x = x ^ (x >> 4);
    x = 4768777513237032717LL * x;
    x = x ^ (x << 20);
    x = x ^ (x >> 41);
    x = x ^ (x << 5);

    return x;
}

inline void HashRandomStream::adjustSeed()
{
    _elementS.v[0] += 1;
}

inline void HashRandomStream::appendToSeed(Seed& y, const Seed& o, const Seed& x)
{
    y.v[0] = o.v[0] + x.v[0];
}

inline void HashRandomStream::appendToSeed(Seed& y, const Seed& o, const Seed& x, const Int64 f)
{
    y.v[0] = o.v[0] + x.v[0] * f;
}

/** @}*/// add to math_random group
} // namespace Myriad

#endif /* HASHRANDOMSTREAM_H_ */
