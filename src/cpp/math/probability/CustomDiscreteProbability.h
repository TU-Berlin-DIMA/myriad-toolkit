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

#ifndef CUSTOMDISCRETEPROBABILITY_H_
#define CUSTOMDISCRETEPROBABILITY_H_

#include "math/probability/DiscreteProbability.h"
#include "math/DiscreteMap.h"
#include "math/IntervalMap.h"

#include <string>

using namespace std;
using namespace Poco;

namespace Myriad {

class CustomDiscreteProbability: public DiscreteProbability
{
public:

	CustomDiscreteProbability(const string& name) :
		DiscreteProbability(name), _map(name + "-map")
	{
	}

	CustomDiscreteProbability(const string& name, map<string, Any>& params) :
		DiscreteProbability(name), _map(name + "-map")
	{
		for(map<string, Any>::const_iterator it = params.begin(); it != params.end(); ++it)
		{
			define(fromString<ID>(it->first), AnyCast<Decimal>(it->second));
		}
	}

	Decimal at(const ID x) const;

	Decimal operator()(const ID x) const;

	ID sample(Decimal randomValue) const;

	void define(const ID x, Decimal value);

	I32u domainCardinality() const;

	const map<const ID, Decimal>& mapping() const;

private:

	DiscreteMap<ID, Decimal> _map;
};

inline Decimal CustomDiscreteProbability::at(const ID x) const
{
	return _map(x);
}

inline Decimal CustomDiscreteProbability::operator()(const ID x) const
{
	return at(x);
}

inline ID CustomDiscreteProbability::sample(Decimal random) const
{
	Decimal x = 0.0;
	for (map<ID, Decimal>::const_iterator it = _map._mapping.begin(); it != _map._mapping.end(); ++it)
	{
		x += it->second;

		if (x >= random)
		{
			return it->first;
		}
	}

	throw Poco::LogicException(format("Can't sample value in from probability %s with random uniform input %f", _name, random));
}

inline void CustomDiscreteProbability::define(const ID x, Decimal probability)
{
	_map.define(x, probability);
}

inline I32u CustomDiscreteProbability::domainCardinality() const
{
	return _map.domainCardinality();
}

inline const map<const ID, Decimal>& CustomDiscreteProbability::mapping() const
{
	return _map.mapping();
}

} // namespace Myriad


#endif /* CUSTOMDISCRETEPROBABILITY_H_ */
