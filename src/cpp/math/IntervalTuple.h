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

#ifndef INTERVALTUPLE_H_
#define INTERVALTUPLE_H_

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
template<class MyriadTuple>
class IntervalTuple // ((low_T1, low_T2), (up_T1, up_T2), prob_interval)
{
public:

    /**
     * Constructor.
     *
     * Initializes the \p min and \p max parameters with the default
     * \p DomainType values.
     */
    IntervalTuple()
    {
    }

    /**
     * Constructor.
     *
     * Initializes the \p min and \p max parameters with the provided
     * \p DomainType values.
     *
     * @param min The left bounds of the interval (inclusive).
     * @param max The right bounds of the interval (exclusive).
     */
    IntervalTuple(const MyriadTuple& min, const MyriadTuple& max) :
        _min(min), _max(max)
    {
    }

    /**
     * Update the bounds of this interval.
     *
     * @param min The new left bound of the interval (inclusive).
     * @param max The new right bound of the interval (exclusive).
     */
    inline void set(const MyriadTuple& min, const MyriadTuple& max)
    {
        _min = min;
        _max = max;
    }

    /**
     * Get the left bound (inclusive) of this interval.
     *
     * @return The interval left bound.
     */
    inline const MyriadTuple min() const
    {
        return _min;
    }

    /**
     * Get the right bound (exclusive) of this interval.
     *
     * @return The interval right bound.
     */
    inline const MyriadTuple max() const
    {
        return _max;
    }

    /**
     * Get the cardinality of all dimensions of this interval, computed as <tt>prod(max_i - min_i)</tt>.
     *
     * @return The interval length.
     */
    inline const double length() const
    {
    	double gamma = 1;
    	for (size_t i = 0; i < _max.getDim(); ++i)
    		gamma *= _max.elementAt(i)-_min.elementAt(i);
        return gamma;
    }

    /**
     * Get the cardinality of dim d of this interval, computed as <tt>max_d - min_d</tt>.
     *
     * @param Dimension index.
     * @return The interval length.
     */
    inline const double length(size_t d) const
    {
        return _max.elementAt(d)-_min.elementAt(d);
    }

    /**
     * Check if the interval contains the given point \p x.
     *
     * @return True, if <tt>min_i >= x && max_i < x for i=1..dim</tt>.
     */
    inline const bool contains(MyriadTuple x) const
    {
    	bool isContained = false;
    	for (size_t i = 0; i < x.getDim(); ++i)
    		isContained &= _min.elementAt(i) <= x && x < _max.elementAt(i);
        return isContained;
    }

    /**
     * Intersect the current and the \p other intervals and update the current
     * \p min and \p max values to the intersected range.
     * TODO
     * @param other The other against which we intersect the current one.
     */
    inline void intersect(const IntervalTuple<MyriadTuple>& other)
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
    inline bool operator < (const IntervalTuple<MyriadTuple>& r) const
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
    inline bool operator == (const IntervalTuple<MyriadTuple>& r) const
    {
    	bool isEqual = true;
        for (size_t i = 0; i < r.getDim(); ++i)
        	isEqual &= _min.elementAt(i) == r._min.elementAt(i) && _max.elementAt(i) == r._max.elementAt(i);
        return isEqual;
    }

    /**
     * Equality comparison operator.
     *
     * Two intervals are not equall iff their \p min or \p max values are not
     * equal.
     */
    inline bool operator != (const IntervalTuple<MyriadTuple>& r) const
    {
    	bool isUnequal = true;
    	for (size_t i = 0; i < r.getDim(); ++i)
    		isUnequal &=_min != r._min || _max != r._max;
    	return isUnequal;
    }

private:

    /**
     * The left bound of the interval (inclusive).
     */
    MyriadTuple _min;

    /**
     * The right bound of the interval (exclusive).
     */
    MyriadTuple _max;
};

/**
 * A 'write to ostream' specialization for the Interval template.
 */
template<typename MyriadTuple> inline std::ostream& operator<<(std::ostream& stream, const IntervalTuple<MyriadTuple>& interval)
{
    stream << "[" << interval.min() << ", " << interval.max() << ")";
    return stream;
}

/** @}*/// add to math group
} // namespace Myriad

#endif /* INTERVALTUPLE_H_ */
