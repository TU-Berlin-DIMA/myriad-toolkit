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

#ifndef QHISTOGRAMPRFUNCTION_H_
#define QHISTOGRAMPRFUNCTION_H_

#include "core/types.h"
#include "math/Function.h"

#include <Poco/Any.h>

#include <string>
#include <map>
#include <cmath>

using namespace std;
using namespace Poco;

namespace Myriad {

class QHistogramPrFunction: public AnalyticPrFunction<I64u>
{
public:

	QHistogramPrFunction(const string& path) :
		AnalyticPrFunction<I64u> ("")
	{
		initialize(path);
	}

	QHistogramPrFunction(const string& name, const string& path) :
		AnalyticPrFunction<I64u> (name)
	{
		initialize(path);
	}

	QHistogramPrFunction(map<string, Any>& params) :
		AnalyticPrFunction<I64u> ("")
	{
		initialize(AnyCast<string>(params["path"]));
	}

	QHistogramPrFunction(const string& name, map<string, Any>& params) :
		AnalyticPrFunction<I64u> (name)
	{
		initialize(AnyCast<string>(params["path"]));
	}

	Decimal operator()(const I64u x) const;

	Decimal pdf(I64u x) const;

	Decimal cdf(I64u x) const;

	I64u invcdf(Decimal x) const;

	I64u sample(Decimal random) const;

	Interval<I64u> threshold(Decimal yMin) const;

private:

	void initialize(const string& path)
	{
		// read Q-hist from path
		// for each q-entry add a [min, max) pair of domain values to a lookup table
	}
};

inline Decimal QHistogramPrFunction::operator()(const I64u x) const
{
	return cdf(x);
}

inline I64u QHistogramPrFunction::sample(Decimal random) const
{
	return invcdf(random);
}

inline Interval<I64u> QHistogramPrFunction::threshold(Decimal yMin) const
{
	return Interval<I64u>(0, 0); //FIXME
}

} // namespace Myriad


#endif /* QHISTOGRAMPRFUNCTION_H_ */
