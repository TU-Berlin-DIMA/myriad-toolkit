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

#ifndef COMPOUNDEICG_H_
#define COMPOUNDEICG_H_

#include <cmath>

#include "math/random/EICG.h"
#include "math/random/RNG.h"

using namespace std;

namespace Myriad {
/**
 * @addtogroup math_random
 * @{*/

// forward declarations
class CompoundEICG;

/**
 * Traits specialization for the CompoundEICG random stream.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<> struct prng_traits<CompoundEICG>
{
    typedef RandomSeed<bool, 6> seed_bitmap_type; //!< Boolean seed map
    typedef RandomSeed<UInt32, 6> seed_type; //!< The concrete seed type
};

/**
 * A compound linear combination of size extended inversive congruential
 * generators (EICG).
 *
 */
class CompoundEICG: public HierarchicalRNG
{
public:

    /**
     * An alias of the seed type for this RNG implementation.
     */
    typedef prng_traits<CompoundEICG>::seed_type Seed;

    /**
     * A vector of period lengths for each seed component.
     */
    static const Seed PERIOD;
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
     * Default constructor.
     */
    CompoundEICG(const string name = "anonymous") : _name(name)
    {
        initialize();
    }

    /**
     * Anonymous copy constructor.
     */
    CompoundEICG(const CompoundEICG& o, const string name = "anonymous") :
        _name(name), _masterS(o._masterS), _substreamS(o._substreamS), _chunkS(o._chunkS), _elementS(o._elementS)
    {
        initialize();
    }

    /**
     * Destructor.
     */
    ~CompoundEICG()
    {
        for (unsigned int i = 0; i < 6; i++)
        {
	        delete _eicg[i];
        }
    }

    /**
     * Assignment operator.
     */
    CompoundEICG& operator =(const CompoundEICG& o)
    {
        if (this != &o) // protect against invalid self-assignment
        {
	        _masterS = o._masterS;
	        _substreamS = o._substreamS;
	        _chunkS = o._chunkS;
	        _elementS = o._elementS;
	        _currentSum = updateResults();
        }

        return *this;
    }

    /**
     * Adjust the random stream to the given seed.
     */
    void seed(Seed masterSeed)
    {
        _masterS = _substreamS = _chunkS = _elementS = masterSeed;
        _currentSum = updateResults();
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
        std::cout << "element   : [ ";
        for (unsigned int i = 0; i < 5; i++)
        {
	        std::cout << _elementS.v[i] << ", ";
        }
        std::cout << _elementS.v[5] << " ]" << std::endl;

        std::cout << "chunk     : [ ";
        for (unsigned int i = 0; i < 5; i++)
        {
	        std::cout << _chunkS.v[i] << ", ";
        }
        std::cout << _chunkS.v[5] << " ]" << std::endl;

        std::cout << "substream : [ ";
        for (unsigned int i = 0; i < 5; i++)
        {
	        std::cout << _substreamS.v[i] << ", ";
        }
        std::cout << _substreamS.v[5] << " ]" << std::endl;

        std::cout << "master:     [ ";
        for (unsigned int i = 0; i < 5; i++)
        {
	        std::cout << _masterS.v[i] << ", ";
        }
        std::cout << _masterS.v[5] << " ]" << std::endl;
    }

    // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
    // CompoundEICG interface
    // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

    /**
     * See RNG::next()
     */
    double next()
    {
        double fpart, ipart;
        fpart = modf(_currentSum, &ipart);

        appendToSeed(_elementS, _elementS, OFFSET_ELEMENT);
        _currentSum = updateResults();

        return fpart;
    }

    /**
     * See RNG::at()
     */
    double at(UInt64 pos)
    {
        appendToSeed(_elementS, _chunkS, OFFSET_ELEMENT, pos);
        _currentSum = updateResults();

        return next();
    }

    /**
     * @see RNG::skip()
     */
    void skip(UInt64 pos)
    {
        appendToSeed(_elementS, _elementS, OFFSET_ELEMENT, pos);
        _currentSum = updateResults();
    }

    // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
    // HierarchicalRNG interface
    // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

    /**
     * See CompoundEICG::nextSubstream()
     */
    CompoundEICG& nextSubstream()
    {
        appendToSeed(_substreamS, _substreamS, OFFSET_SUBSTREAM);
        _elementS = _chunkS = _substreamS;
        _currentSum = updateResults();

        return *this;
    }

    /**
     * See CompoundEICG::resetSubstream()
     */
    CompoundEICG& resetSubstream()
    {
        _elementS = _chunkS = _substreamS;
        _currentSum = updateResults();

        return *this;
    }

    /**
     * See CompoundEICG::nextChunk()
     */
    CompoundEICG& nextChunk()
    {
        appendToSeed(_chunkS, _chunkS, OFFSET_CHUNK);
        _elementS = _chunkS;
        _currentSum = updateResults();

        return *this;
    }

    /**
     * See CompoundEICG::resetChunk()
     */
    CompoundEICG& resetChunk()
    {
        _elementS = _chunkS;
        _currentSum = updateResults();

        return *this;
    }

    /**
     * See CompoundEICG::atChunk()
     */
    CompoundEICG& atChunk(UInt64 pos)
    {
        appendToSeed(_chunkS, _substreamS, OFFSET_CHUNK, pos);
        _elementS = _chunkS;
        _currentSum = updateResults();

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
     * A vector of EICG components.
     */
    EICG* _eicg[6];
    /**
     * A vector of current results.
     */
    double _currentResults[6];
    /**
     * Sum of the EICG components for the current seed values.
     */
    double _currentSum;

    /**
     * Common object initialization logic called from all constructors.
     */
    void initialize();
    /**
     * Updates the current results vector and returns the results sum.
     */
    double updateResults();
    /**
     * Increments seed (+1 operation in the residue class product ring).
     */
    void adjustSeed();
    /**
     * Computes y = a + x in the residue class product ring.
     */
    void appendToSeed(Seed& y, const Seed& o, const Seed& x);
    /**
     * Computes y = o + f*x in the residue class product ring.
     */
    void appendToSeed(Seed& y, const Seed& o, const Seed& x, const Int64 f);
    /**
     * Private helper method for multiplying 64 bit integers modulo p.
     */
    Int64 mult64(Int64 x, Int64 y, Int32 p) const;
};

inline void CompoundEICG::initialize()
{
    for (unsigned int i = 0; i < 6; i++)
    {
        _eicg[i] = new EICG(PERIOD.v[i], 32, 0, 0, _elementS.v[i]);
        _currentResults[i] = 0.0;
    }

    _currentSum = updateResults();
}

inline double CompoundEICG::updateResults()
{
    double s = 0.0;

    for (unsigned int i = 0; i < 6; i++)
    {
        _currentResults[i] = _eicg[i]->at(_elementS.v[i]);
        s += _currentResults[i];
    }

    return s;
}

inline void CompoundEICG::adjustSeed()
{
    for (unsigned int i = 0; i < 6; i++)
    {
        _elementS.v[i] = (_elementS.v[i] + 1) % PERIOD.v[i];
    }
}

inline void CompoundEICG::appendToSeed(Seed& y, const Seed& o, const Seed& x)
{
    for (unsigned int i = 0; i < 6; i++)
    {
        y.v[i] = ((o.v[i] + x.v[i]) % PERIOD.v[i]);
    }
}

inline void CompoundEICG::appendToSeed(Seed& y, const Seed& o, const Seed& x, const Int64 f)
{
    for (unsigned int i = 0; i < 6; i++)
    {
        y.v[i] = ((o.v[i] + mult64(x.v[i], f, PERIOD.v[i])) % PERIOD.v[i]);
    }
}

inline Int64 CompoundEICG::mult64(Int64 x, Int64 y, Int32 p) const
{
    Int64 q, r, z;

    q = p / x;
    r = p % x;

    z = x * (y % q) - r * (y / q);
    while (z < 0)
    {
        z += p;
    }

    return z;
}

/** @}*/// add to math_random group
} // namespace Myriad

#endif /* COMPOUNDEICG_H_ */
