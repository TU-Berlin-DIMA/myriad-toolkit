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

#ifndef INTERVAL_H_
#define INTERVAL_H_

#include <iostream>

namespace Myriad {
/**
 * @addtogroup math
 * @{*/

/**
 * An template for a semi-open interval <tt>[min, max)</tt> of a generic
 * \p Domain type.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class Domain>
class Interval
{
public:

    /**
     * Constructor.
     *
     * Initializes the \p min and \p max parameters with the default
     * \p DomainType values.
     */
	Interval()
	{
	}

    /**
     * Constructor.
     *
     * Initializes the \p min and \p max parameters with the provided
     * \p DomainType values.
     *
     * @param min The left bound of the interval (inclusive).
     * @param max The right bound of the interval (exclusive).
     */
	Interval(const Domain& min, const Domain& max) :
		_min(min), _max(max)
	{
	}

	/**
	 * Update the bounds of this interval.
     *
     * @param min The new left bound of the interval (inclusive).
     * @param max The new right bound of the interval (exclusive).
	 */
	inline void set(const Domain& min, const Domain& max)
	{
		_min = min;
		_max = max;
	}

	/**
	 * Get the left bound (inclusive) of this interval.
	 *
     * @return The interval left bound.
	 */
	inline const Domain min() const
	{
		return _min;
	}

    /**
     * Get the right bound (exclusive) of this interval.
     *
     * @return The interval right bound.
     */
	inline const Domain max() const
	{
		return _max;
	}

    /**
     * Get the length of this interval, computed as <tt>max() - min()</tt>.
     *
     * @return The interval length.
     */
	inline const double length() const
	{
		return _max-_min;
	}

    /**
     * Check if the interval contains the given point \p x.
     *
     * @return True, if <tt>min() >= x && max() < x</tt>.
     */
	inline const bool contains(Domain x) const
	{
		return _min <= x && x < _max;
	}

    /**
     * Intersect the current and the \p other intervals and update the current
     * \p min and \p max values to the intersected range.
     *
     * @param other The other against which we intersect the current one.
     */
	inline void intersect(const Interval<Domain>& other)
	{
	    if (other._min < _max && _min < other._max)
	    {
	        // intervals intersect, adjust end points
            _min = (other._min > _min) ? other._min : _min;
            _max = (other._max < _max) ? other._max : _max;
	    }
	    else
        {
	        // do not intersect, empty interval
            _min = _max;
        }
	}

	/**
	 * Interval comparison operator.
	 *
	 * Compares two intervals lexicographically by <tt>(min, max)</tt>.
	 */
	inline bool operator < (const Interval<Domain>& r) const
	{
	    if (_min - r._min != 0)
	    {
	        return _min < r._min;
	    }
	    else
	    {
	        return _max < r._max;
	    }
	}

    /**
     * Equality comparison operator.
     *
     * Two intervals are equall iff their \p min and \p max values are equal.
     */
	inline bool operator == (const Interval<Domain>& r)
	{
	    return _min == r._min && _max == r._max;
	}

    /**
     * Equality comparison operator.
     *
     * Two intervals are not equall iff their \p min or \p max values are not
     * equal.
     */
	inline bool operator != (const Interval<Domain>& r)
	{
	    return _min != r._min || _max != r._max;
	}

private:

	/**
	 * The left bound of the interval (inclusive).
	 */
	Domain _min;

    /**
     * The right bound of the interval (exclusive).
     */
	Domain _max;
};

/**
 * A 'write to ostream' specialization for the Interval template.
 */
template<typename Domain> inline std::ostream& operator<<(std::ostream& stream, const Interval<Domain>& interval)
{
	stream << "[" << interval.min() << ", " << interval.max() << ")";
	return stream;
}

/** @}*/// add to math group
} // namespace Myriad

#endif /* INTERVAL_H_ */
