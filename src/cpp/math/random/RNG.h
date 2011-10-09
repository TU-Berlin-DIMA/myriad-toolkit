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
 * @author: Alexander Alexandrov <alexander.s.alexandrov@campus.tu-berlin.de>
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
 * Basic traits implementation for RNG implementations.
 */
template<class RNG> struct prng_traits
{
	typedef RandomSeed<Int64, 3> seed_type;
};

/**
 * A base interface for all random number generators.
 */
class RNG
{
public:

	/**
	 * Return the next random double from the RNG stream. This method must be
	 * provided by all implementations.
	 */
	virtual double next() = 0;

	/**
	 * Compute the random unified double at the specified position. This method
	 * is optional. For hierarchical RNGs, the position is relative to the
	 * current RNG chunk. Throws Poco::RangeException if the provided index
	 * position is out of the available range. If the RNG doesn't support
	 * random skips, the method should throw a Poco::NotImplementedException.
	 */
	virtual double at(UInt64 i) = 0;
};

/**
 * A base interface for hierarchical random number generators. A hierarchical
 * RNG must support splitting the main random number stream into smaller
 * substreams as well as splitting each substream into even smaller
 * subsubstreams (chunks).
 */
class HierarchicalRNG: public RNG
{
public:

	/**
	 * Move the RNG position to the beginning of the next substream and
	 * return a reference to this RNG object with the adjusted position.
	 * Throws a Poco::RangeException if all substreams of the RNG have already
	 * been consumed.
	 */
	virtual HierarchicalRNG& nextSubstream() = 0;

	/**
	 * Move the RNG position to the beginning of the current substream
	 * and return a reference to this RNG object with the adjusted position.
	 *
	 * @return
	 */
	virtual HierarchicalRNG& resetSubstream() = 0;

	/**
	 * Move the current substream position to the beginning of the next chunk
	 * and return a reference to this RNG object with the adjusted position.
	 * Throws a Poco::RangeException if all chunks of the current substream
	 * have already been consumed.
	 */
	virtual HierarchicalRNG& nextChunk() = 0;

	/**
	 * Move the RNG position to the beginning of the current chunk and
	 * return a reference to this RNG object with the adjusted position.
	 *
	 * @return
	 */
	virtual HierarchicalRNG& resetChunk() = 0;

	/**
	 * Move the current substream position to the specified index. This method
	 * is optional. Throws Poco::RangeException if the provided index position
	 * is out of the available range. If the hierarchical RNG doesn't support
	 * random skips, the method should throw a Poco::NotImplementedException.
	 */
	virtual double atChunk(UInt64 i) = 0;
};

}  // namespace Myriad

#endif /* RNG_H_ */
