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

#ifndef INTERVALMAP_H_
#define INTERVALMAP_H_

#include "math/Function.h"
#include "math/Interval.h"

using namespace std;
using namespace Poco;

namespace Myriad {

class IntervalMapTest;

/**
 * This is a template specialization for discrete maps with interval domain types.
 *
 */
template<class Domain, class Range> class IntervalMap: public UnaryFunction<Domain, Range>
{
public:

	// friend classes
	friend class IntervalMapTest;

	IntervalMap(const string& name) :
		UnaryFunction<Domain, Range> (name), _lastAccessedDomainValue(NULL), _lastAccessedRangeValue(NULL)
	{
	}

	IntervalMap(const string& name, map<string, Any>& params) :
		UnaryFunction<Domain, Range> (name), _lastAccessedDomainValue(NULL), _lastAccessedRangeValue(NULL)
	{
	}

	~IntervalMap()
	{
	}

	Range operator()(const Domain x) const;

	void define(const Interval<Domain> x, Range value);

	I32u domainCardinality() const;

	const Interval<Domain>& lastAccessedDomainValue() const;

	const Range& lastAccessedRangeValue() const;

protected:

	typedef map<const Interval<Domain> , Range> Mapping;

	Mapping _mapping;

	Interval<Domain>* _lastAccessedDomainValue;

	Range* _lastAccessedRangeValue;
};

template<class Domain, class Range> inline Range IntervalMap<Domain, Range>::operator()(const Domain x) const
{
	if (_lastAccessedDomainValue != NULL && _lastAccessedDomainValue->contains(x))
	{
		return *_lastAccessedRangeValue;
	}
	else
	{
		// TODO: some special structure can be used to optimize this operation
		for (typename Mapping::const_iterator it = _mapping.begin(); it != _mapping.end(); ++it)
		{
			if (it->first.contains(x))
			{
				const_cast<IntervalMap<Domain, Range>*>(this)->_lastAccessedDomainValue = const_cast<Interval<Domain>* >(&it->first);
				const_cast<IntervalMap<Domain, Range>*>(this)->_lastAccessedRangeValue = const_cast<Range* >(&it->second);

				return it->second;
			}
		}

		throw Poco::LogicException(format("Undefined discrete point %lu in discrete map %s", x, UnaryFunction<Domain, Range>::name()));
	}
}

template<class Domain, class Range> inline void IntervalMap<Domain, Range>::define(const Interval<Domain> x, Range value)
{
	_mapping[x] = value;
}

template<class Domain, class Range> inline I32u IntervalMap<Domain, Range>::domainCardinality() const
{
	return _mapping.size();
}

template<class Domain, class Range> inline const Interval<Domain>& IntervalMap<Domain, Range>::lastAccessedDomainValue() const
{
	return *_lastAccessedDomainValue;
}

template<class Domain, class Range> inline const Range& IntervalMap<Domain, Range>::lastAccessedRangeValue() const
{
	return *_lastAccessedRangeValue;
}

} // namespace Myriad

#endif /* INTERVALMAP_H_ */
