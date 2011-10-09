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

#ifndef VECTORMAP_H_
#define VECTORMAP_H_

#include "math/Function.h"
#include "math/Interval.h"

#include <vector>

using namespace std;
using namespace Poco;

namespace Myriad {

/**
 * A map wrapped around a vector.
 *
 */
template<class Range> class VectorMap: public UnaryFunction<I32u, Range>
{
public:

	VectorMap(const string& name, I32u size = 1024) :
		UnaryFunction<I32u, Range> (name), _size(size), _mapping(_size)
	{
	}

	VectorMap(const string& name, map<string, Any>& params) :
		UnaryFunction<I32u, Range> (name), _size(AnyCast<I32u>(params["size"])), _mapping(_size)
	{
	}

	~VectorMap()
	{
	}

	Range operator()(const I32u x) const;

	void define(const I32u x, Range value);

	const vector<Range>& mapping() const;

protected:

	I32u _size;
	vector<Range> _mapping;
};

template<class Range> inline Range VectorMap<Range>::operator()(const I32u x) const
{
	if (x < _size)
	{
		return _mapping[x];
	}
	else
	{
		throw Poco::LogicException(format("Undefined discrete point %s in discrete map `%s`", toString(x), UnaryFunction<I32u, Range>::name()));
	}
}

template<class Range> inline void VectorMap<Range>::define(const I32u x, Range value)
{
	_mapping[x] = value;
}

template<class Range> inline const vector<Range>& VectorMap<Range>::mapping() const
{
	return _mapping;
}

} // namespace Myriad


#endif /* VECTORMAP_H_ */
