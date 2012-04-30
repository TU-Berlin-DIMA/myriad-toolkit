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

#ifndef UNIFORMPRFUNCTION_H_
#define UNIFORMPRFUNCTION_H_

#include "core/types.h"
#include "math/Function.h"

#include <Poco/Any.h>

#include <string>
#include <map>
#include <cmath>

using namespace std;
using namespace Poco;

namespace Myriad {

class UniformPrFunction: public AnalyticPrFunction<Decimal>
{
public:

	UniformPrFunction(Decimal xMin = 0, Decimal xMax = 1) :
		AnalyticPrFunction<Decimal> (""), _xMin(xMin), _xMax(xMax), _size(_xMax - _xMin), _xPDF(1.0 / _size)
	{
	}

	UniformPrFunction(const string& name, Decimal xMin = 0, Decimal xMax = 1) :
		AnalyticPrFunction<Decimal> (name), _xMin(xMin), _xMax(xMax), _size(_xMax - _xMin), _xPDF(1.0 / _size)
	{
	}

	UniformPrFunction(map<string, Any>& params) :
		AnalyticPrFunction<Decimal> ("")
	{
		_xMin = AnyCast<Decimal>(params["xMin"]);
		_xMax = AnyCast<Decimal>(params["xMax"]);
		_size = _xMax - _xMin;
		_xPDF = 1.0 / _size;
	}

	UniformPrFunction(const string& name, map<string, Any>& params) :
		AnalyticPrFunction<Decimal> (name)
	{
		_xMin = AnyCast<Decimal>(params["xMin"]);
		_xMax = AnyCast<Decimal>(params["xMax"]);
		_size = _xMax - _xMin;
		_xPDF = 1.0 / _size;
	}

	Decimal operator()(const Decimal x) const;

	Decimal pdf(Decimal x) const;

	Decimal cdf(Decimal x) const;

	Decimal invcdf(Decimal x) const;

	Decimal sample(Decimal random) const;

	Interval<Decimal> threshold(Decimal yMin) const;

private:

	// parameters
	Decimal _xMin;
	Decimal _xMax;

	// common used terms
	Decimal _size;
	Decimal _xPDF;
};

inline Decimal UniformPrFunction::operator()(const Decimal x) const
{
	return cdf(x);
}

inline Decimal UniformPrFunction::sample(Decimal random) const
{
	return invcdf(random);
}

inline Interval<Decimal> UniformPrFunction::threshold(Decimal yMin) const
{
	return Interval<Decimal>(_xMin, _xMax);
}

} // namespace Myriad


#endif /* UNIFORMPRFUNCTION_H_ */
