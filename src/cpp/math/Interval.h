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

#ifndef INTERVAL_H_
#define INTERVAL_H_

#include <iostream>

namespace Myriad {

// forward declarations
template<typename Domain> class Interval;
template<typename Domain> bool operator<(const Interval<Domain>& a, const Interval<Domain>& b);
template<typename Domain> bool operator==(const Interval<Domain>& a, const Interval<Domain>& b);
template<typename Domain> bool operator!=(const Interval<Domain>& a, const Interval<Domain>& b);
template<typename Domain> std::ostream& operator<<(std::ostream& stream, const Myriad::Interval<Domain>& interval);

template<class Domain> class Interval
{
public:
	Interval()
	{
	}

	Interval(const Domain& x, const Domain& y) :
		_min(x), _max(y)
	{
	}

	const Domain min() const
	{
		return _min;
	}

	const Domain max() const
	{
		return _max;
	}

	const Domain length() const
	{
		return _max-_min;
	}

	const bool contains(Domain x) const
	{
		return _min <= x && x < _max;
	}

	friend std::ostream& operator<< <Domain>(std::ostream& stream, const Interval<Domain>& interval);

	friend bool operator< <Domain>(Interval<Domain> const& l, Interval<Domain> const& r);

	friend bool operator== <Domain>(Interval<Domain> const& l, Interval<Domain> const& r);

	friend bool operator!= <Domain>(Interval<Domain> const& l, Interval<Domain> const& r);

private:
	Domain _min, _max;
};

template<typename Domain> inline std::ostream& operator<<(std::ostream& stream, const Interval<Domain>& interval)
{
	stream << "[" << interval._min << ", " << interval._max << ")";
	return stream;
}

template<typename Domain> inline bool operator< (Interval<Domain> const& l, Interval<Domain> const& r)
{
	if (l._min - r._min != 0)
	{
		return l._min < r._min;
	}
	else
	{
		return l._max < r._max;
	}
}

template<typename Domain> inline bool operator== (Interval<Domain> const& l, Interval<Domain> const& r)
{
	return l._min == r._min && l._max == r._max;
}

template<typename Domain> inline bool operator!= (Interval<Domain> const& l, Interval<Domain> const& r)
{
	return l._min != r._min || l._max != r._max;
}

} // namespace Myriad

#endif /* INTERVAL_H_ */
