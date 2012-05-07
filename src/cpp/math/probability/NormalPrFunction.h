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

#ifndef NORMALPRFUNCTION_H_
#define NORMALPRFUNCTION_H_

#include "core/types.h"
#include "math/Function.h"
#include "math/DiscreteMap.h"
#include "math/IntervalMap.h"

#include <Poco/Any.h>

#include <string>
#include <map>
#include <cmath>

using namespace std;
using namespace Poco;

namespace Myriad {

class NormalPrFunction: public UnivariatePrFunction<Decimal>
{
public:

	NormalPrFunction(Decimal mean = 0, Decimal stddev = 1) :
		UnivariatePrFunction<Decimal> (""), _mean(mean), _stddev(stddev)
	{
		initialize();
	}

	NormalPrFunction(const string& name, Decimal mean = 0, Decimal stddev = 1) :
		UnivariatePrFunction<Decimal> (name), _mean(mean), _stddev(stddev)
	{
		initialize();
	}

	NormalPrFunction(map<string, Any>& params) :
		UnivariatePrFunction<Decimal> ("")
	{
		_mean = AnyCast<Decimal>(params["mean"]);
		_stddev = AnyCast<Decimal>(params["stddev"]);

		initialize();
	}

	NormalPrFunction(const string& name, map<string, Any>& params) :
		UnivariatePrFunction<Decimal> (name)
	{
		_mean = AnyCast<Decimal>(params["mean"]);
		_stddev = AnyCast<Decimal>(params["stddev"]);

		initialize();
	}

	Decimal operator()(const Decimal x) const;

	Decimal pdf(Decimal x) const;

	Decimal cdf(Decimal x) const;

	Decimal invpdf(Decimal y) const;

	Decimal invcdf(Decimal y) const;

	Decimal sample(Decimal random) const;

	Interval<Decimal> threshold(Decimal yMin) const;

	Decimal mean() const;

	Decimal stddev() const;

private:

	/**
	 * Common initialization logic.
	 */
	void initialize()
	{
		_var = _stddev * _stddev;
		_A = 1 / sqrt(2 * M_PI * _var);

		_a = 0.14;
	}

	Decimal erf(const Decimal x) const;

	Decimal inverf(const Decimal y) const;

	// parameters
	Decimal _mean;
	Decimal _stddev;

	// common used terms
	Decimal _var;
	Decimal _A;
	Decimal _a;
};

inline Decimal NormalPrFunction::operator()(const Decimal x) const
{
	return cdf(x);
}

inline Decimal NormalPrFunction::sample(Decimal random) const
{
	return invcdf(random);
}

} // namespace Myriad

#endif /* NORMALPRFUNCTION_H_ */
