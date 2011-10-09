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

#ifndef DISCRETEMAP_H_
#define DISCRETEMAP_H_

#include "math/Function.h"
#include "math/Interval.h"

#include <map>

using namespace std;
using namespace Poco;

namespace Myriad {

class CustomDiscreteProbability;

/**
 * Basic template for all discrete maps.
 *
 */
template<class Domain, class Range> class DiscreteMap: public UnaryFunction<Domain, Range>
{
public:

	friend class CustomDiscreteProbability;

	DiscreteMap(const string& name) :
		UnaryFunction<Domain, Range> (name)
	{
	}

	DiscreteMap(const string& name, map<string, Any>& params) :
		UnaryFunction<Domain, Range> (name)
	{
	}

	~DiscreteMap()
	{
	}

	Range operator()(const Domain x) const;

	void define(const Domain x, Range value);

	I32u domainCardinality() const;

	const map<const Domain, Range>& mapping() const;

protected:

	map<const Domain, Range> _mapping;
};

template<class Domain, class Range> inline Range DiscreteMap<Domain, Range>::operator()(const Domain x) const
{
	typename map<const Domain, Range>::const_iterator it = _mapping.find(x);
	if (it != _mapping.end())
	{
		return it->second;
	}
	else
	{
		throw Poco::LogicException(format("Undefined discrete point %s in discrete map `%s`", toString(x), UnaryFunction<Domain, Range>::name()));
	}
}

template<class Domain, class Range> inline void DiscreteMap<Domain, Range>::define(const Domain x, Range value)
{
	_mapping[x] = value;
}

template<class Domain, class Range> inline I32u DiscreteMap<Domain, Range>::domainCardinality() const
{
	return _mapping.size();
}

template<class Domain, class Range> inline const map<const Domain, Range>& DiscreteMap<Domain, Range>::mapping() const
{
	return _mapping;
}

} // namespace Myriad

#endif /* DISCRETEMAP_H_ */
