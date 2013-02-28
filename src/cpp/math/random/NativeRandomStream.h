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

#ifndef NATIVERANDOMSTREAM_H_
#define NATIVERANDOMSTREAM_H_

#include <cstdlib>
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
class NativeRandomStream;

/**
 * Traits specialization for the NativeRandomStream random stream.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<> struct prng_traits<NativeRandomStream>
{
    typedef RandomSeed<UInt32, 1> seed_type; //!< The concrete seed type
};

/**
 * A wrapper around the C library rand() function.
 *
 * This component is included only for benchmarking purposes and is not
 * compatible with the HierarchicalRNG interface.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class NativeRandomStream: public HierarchicalRNG
{
public:

    /**
     * An alias of the seed type for this RNG implementation.
     */
    typedef prng_traits<NativeRandomStream>::seed_type Seed;

    /**
     * Default constructor.
     */
    NativeRandomStream(const string name = "anonymous") :
        _name(name)
    {
        initialize();
        srand(_masterS.v[0]);
    }

    /**
     * Copy constructor.
     */
    NativeRandomStream(const NativeRandomStream& o, const string name = "anonymous") :
        _name(name),
        _masterS(o._masterS)
    {
        initialize();
        srand(_masterS.v[0]);
    }

    /**
     * Destructor.
     */
    ~NativeRandomStream()
    {
    }

    /**
     * Assignment operator.
     */
    NativeRandomStream& operator =(const NativeRandomStream& o)
    {
        if (this != &o) // protect against invalid self-assignment
        {
	        _masterS = o._masterS;
        }

        srand(_masterS.v[0]);

        return *this;
    }

    /**
     * Adjust the random stream to the given seed.
     */
    void seed(Seed masterSeed)
    {
        _masterS = masterSeed;
        srand(masterSeed.v[0]);
    }

    /**
     * Get the current element seed.
     */
    const Seed& seed() const
    {
        return _masterS;
    }

    /**
     * Dump the current RNG state to the <tt>std::cout</tt> stream.
     */
    void dumpState()
    {
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
        return rand()/RAND_MAX;
    }

    /**
     * @see RNG::at()
     */
    double at(UInt64 pos)
    {
        throw Poco::NotImplementedException("at() is not supported by the NativeRandomStream");
    }

    // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
    // HierarchicalRNG interface
    // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

    /**
     * @see HierarchicalRNG::nextSubstream()
     */
    NativeRandomStream& nextSubstream()
    {
        // ignore method call
        return *this;
    }

    /**
     * @see HierarchicalRNG::resetSubstream()
     */
    NativeRandomStream& resetSubstream()
    {
        // ignore method call
        return *this;
    }

    /**
     * @see HierarchicalRNG::nextChunk()
     */
    NativeRandomStream& nextChunk()
    {
        // ignore method call
        return *this;
    }

    /**
     * @see HierarchicalRNG::resetChunk()
     */
    NativeRandomStream& resetChunk()
    {
        throw Poco::NotImplementedException("resetChunk() is not supported by the NativeRandomStream");
    }

    /**
     * @see HierarchicalRNG::atChunk()
     */
    NativeRandomStream& atChunk(UInt64 pos)
    {
        // ignore method call
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
     * Common object initialization logic called from all constructors.
     */
    void initialize()
    {
    }
};

/** @}*/// add to math_random group
} // namespace Myriad

#endif /* NATIVERANDOMSTREAM_H_ */
