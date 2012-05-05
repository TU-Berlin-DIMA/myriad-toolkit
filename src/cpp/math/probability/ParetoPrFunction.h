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

#ifndef PARETOPRFUNCTION_H_
#define PARETOPRFUNCTION_H_

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

class ParetoPrFunction: public UnivariatePrFunction<Decimal>
{
public:

	ParetoPrFunction(Decimal xMin = 1, Decimal alpha = 1) :
		UnivariatePrFunction<Decimal> (""), xMin(xMin), alpha(alpha), xMinAlpha(pow(xMin, alpha))
	{
	}

	ParetoPrFunction(const string& name, Decimal xMin = 1, Decimal alpha = 1) :
		UnivariatePrFunction<Decimal> (name), xMin(xMin), alpha(alpha), xMinAlpha(pow(xMin, alpha))
	{
	}

	ParetoPrFunction(map<string, Any>& params) :
		UnivariatePrFunction<Decimal> ("")
	{
		xMin = AnyCast<Decimal>(params["xMin"]);
		alpha = AnyCast<Decimal>(params["alpha"]);
		xMinAlpha = pow(xMin, alpha);
	}

	ParetoPrFunction(const string& name, map<string, Any>& params) :
		UnivariatePrFunction<Decimal> (name)
	{
		xMin = AnyCast<Decimal>(params["xMin"]);
		alpha = AnyCast<Decimal>(params["alpha"]);
		xMinAlpha = pow(xMin, alpha);
	}

	Decimal operator()(const Decimal x) const;

	Decimal pdf(Decimal x) const;

	Decimal cdf(Decimal x) const;

	Decimal invcdf(Decimal x) const;

	Decimal invpdf(Decimal x) const;

	Decimal sample(Decimal random) const;

	Interval<Decimal> threshold(Decimal yMin) const;

private:

	// parameters
	Decimal xMin;
	Decimal alpha;

	// common used terms
	Decimal xMinAlpha;
};

inline Decimal ParetoPrFunction::operator()(const Decimal x) const
{
	return cdf(x);
}

inline Decimal ParetoPrFunction::sample(Decimal random) const
{
	return invcdf(random);
}

inline Interval<Decimal> ParetoPrFunction::threshold(Decimal yMin) const
{
	return Interval<Decimal>(xMin, invpdf(yMin));
}

} // namespace Myriad


#endif /* PARETOPRFUNCTION_H_ */
