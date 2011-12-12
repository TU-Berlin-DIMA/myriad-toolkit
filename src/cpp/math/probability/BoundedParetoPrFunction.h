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

#ifndef BOUNDEDPARETOPRFUNCTION_H_
#define BOUNDEDPARETOPRFUNCTION_H_

#include "core/types.h"
#include "math/Function.h"
#include "math/DiscreteMap.h"
#include "math/IntervalMap.h"

#include <Poco/DynamicAny.h>

#include <string>
#include <map>
#include <cmath>

using namespace std;
using namespace Poco;

namespace Myriad {

class BoundedParetoPrFunction: public UnaryFunction<ID, Decimal>
{
public:

	BoundedParetoPrFunction(Decimal xMin = 1, Decimal xMax = 100, Decimal alpha = 1) :
		UnaryFunction<ID, Decimal> (""), xMin(xMin), xMax(xMax), alpha(alpha)
	{
		initialize();
	}

	BoundedParetoPrFunction(const string& name, Decimal xMin = 1, Decimal xMax = 100, Decimal alpha = 1) :
		UnaryFunction<ID, Decimal> (name), xMin(xMin), xMax(xMax), alpha(alpha)
	{
		initialize();
	}

	BoundedParetoPrFunction(map<string, DynamicAny> params) :
		UnaryFunction<ID, Decimal> ("")
	{
		xMin = params["xMin"].convert<Decimal> ();
		xMax = params["xMax"].convert<Decimal> ();
		alpha = params["alpha"].convert<Decimal> ();

		initialize();
	}

	BoundedParetoPrFunction(const string& name, map<string, DynamicAny> params) :
		UnaryFunction<ID, Decimal> (name)
	{
		xMin = params["xMin"].convert<Decimal> ();
		xMax = params["xMax"].convert<Decimal> ();
		alpha = params["alpha"].convert<Decimal> ();

		initialize();
	}

	Decimal operator()(const ID x) const;

	Decimal pdf(Decimal x) const;

	Decimal cdf(Decimal x) const;

	Decimal invpdf(Decimal x) const;

	Decimal invcdf(Decimal x) const;

	Decimal sample(Decimal random) const;

private:

	void initialize()
	{
		xMinAlpha = pow(xMin, alpha);
		xMaxAlpha = pow(xMax, alpha);
		B = 1 - xMinAlpha/xMaxAlpha;
	}

	// parameters
	Decimal xMin;
	Decimal xMax;
	Decimal alpha;

	// common used terms
	Decimal xMinAlpha;
	Decimal xMaxAlpha;
	Decimal B;
};

inline Decimal BoundedParetoPrFunction::operator()(const ID x) const
{
	return cdf(x);
}

inline Decimal BoundedParetoPrFunction::sample(Decimal random) const
{
	return invcdf(random);
}

} // namespace Myriad


#endif /* BOUNDEDPARETTOPRFUNCTION_H_ */
