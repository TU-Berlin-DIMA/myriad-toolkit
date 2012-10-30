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

#ifndef RNG_H_
#define RNG_H_

#include "math/random/RandomSeed.h"

#include <Poco/Types.h>
#include <Poco/Exception.h>

using namespace std;
using namespace Poco;

namespace Myriad {
/**
 * @addtogroup math_random
 * @{*/

/**
 * Basic traits type for RNG implementations.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class RNG> struct prng_traits
{
    typedef RandomSeed<Int64, 3> seed_type; //!< The concrete seed type
};

/**
 * A base interface for all random number generators.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class RNG
{
public:

	/**
	 * Virtual destructor.
	 */
    virtual ~RNG()
    {
    }

    /**
     * Empty functor operator. Alias of RNG::next().
     *
     * @return The next double from this random stream.
     */
    double operator()()
    {
        return next();
    }

    /**
     * Range functor operator.
     *
     * @return An arithmetic type \p T instance in the <tt>[min,max]</tt> range.
     */
    template<class T> T operator()(T min, T max)
    {
//        return min + static_cast<T> ((max - min + 1.0) * next());
        // FIXME: adapt dependant components to the new operator() semantics
        return min + static_cast<T> ((max - min) * next());
    }

    /**
     * Return the next random double from the RNG stream.
     *
     * This method must be provided by all implementations.
     */
    virtual double next() = 0;

    /**
     * Compute the random unified double at the specified position.
     *
     * This method is optional. For hierarchical RNGs, the position is relative
     * to the current chunk seed.
     *
     * @throws Poco::RangeException If the provided index position is out of
     *         the available range.
     * @throws Poco::NotImplementedException If the RNG doesn't support random
     *         skips.
     *
     * FIXME exception throwing is not consistently implemented in the concrete components.
     */
    virtual double at(UInt64 i) = 0;

    /**
     * Skip the given amount of positions on the RNG cycle.
     *
     * This method is optional. For hierarchical RNGs, the position is relative
     * to the current element seed.
     *
     * @throws Poco::RangeException If the provided index position is out of
     *         the available range.
     * @throws Poco::NotImplementedException If the RNG doesn't support random
     *         skips.
     *
     * FIXME exception throwing is not consistently implemented in the concrete components.
     */
    virtual void skip(UInt64 pos) = 0;
};

/**
 * A base interface for a two-level hierarchical random number generators.
 *
 * A hierarchical RNG must support splitting the main random number stream
 * into smaller substreams as well as splitting each substream into
 * even smaller subsubstreams (chunks).
 */
class HierarchicalRNG: public RNG
{
public:

    /**
     * Move the RNG position to the beginning of the next substream and
     * return a reference to this RNG object with the adjusted position.
     *
     * @return A reference to this object.
     * @throws Poco::RangeException If all substreams of the RNG have already
     *         been consumed.
     */
    virtual HierarchicalRNG& nextSubstream() = 0;

    /**
     * Move the RNG position to the beginning of the current substream
     * and return a reference to this RNG object with the adjusted position.
     *
     * @return A reference to this object.
     */
    virtual HierarchicalRNG& resetSubstream() = 0;

    /**
     * Move the current substream position to the beginning of the next chunk
     * and return a reference to this RNG object with the adjusted position.
     *
     * @return A reference to this object.
     * @throws Poco::RangeException If all chunks of the current substream
     *         have already been consumed.
     */
    virtual HierarchicalRNG& nextChunk() = 0;

    /**
     * Move the RNG position to the beginning of the current chunk and
     * return a reference to this RNG object with the adjusted position.
     *
     * @return A reference to this object.
     */
    virtual HierarchicalRNG& resetChunk() = 0;

    /**
     * Move the current substream position to the specified index.
     *
     * This method is optional.
     *
     * @throws Poco::RangeException If the provided index position is out of
     *         the available range.
     * @throws Poco::NotImplementedException If the hierarchical RNG doesn't
     *         support random skips, the method should throw a .
     */
    virtual HierarchicalRNG& atChunk(UInt64 i) = 0;
};

/** @}*/// add to math_random group
}  // namespace Myriad

#endif /* RNG_H_ */
