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

#ifndef CONDITIONALQHISTOGRAMPRFUNCTION_H_
#define CONDITIONALQHISTOGRAMPRFUNCTION_H_

#include "core/types.h"
#include "math/Function.h"
#include "math/probability/QHistogramPrFunction.h"

#include <Poco/Exception.h>
#include <Poco/Any.h>

#include <string>
#include <iostream>
#include <fstream>

using namespace std;
using namespace Poco;

namespace Myriad
{

class ConditionalQHistogramPrFunction: public BivariatePrFunction<I64u, I64u>
{
public:

	ConditionalQHistogramPrFunction(const string& path) :
		BivariatePrFunction<I64u, I64u>(""), _x2Pr(NULL), _x1Pr(NULL)
	{
		initialize(path);
	}

	ConditionalQHistogramPrFunction(const string& name, const string& path) :
		BivariatePrFunction<I64u, I64u>(name), _x2Pr(NULL), _x1Pr(NULL)
	{
		initialize(path);
	}

	ConditionalQHistogramPrFunction(map<string, Any>& params) :
		BivariatePrFunction<I64u, I64u>(""), _x2Pr(NULL), _x1Pr(NULL)
	{
		initialize(AnyCast<string>(params["path"]));
	}

	ConditionalQHistogramPrFunction(const string& name, map<string, Any>& params) :
		BivariatePrFunction<I64u, I64u>(name), _x2Pr(NULL), _x1Pr(NULL)
	{
		initialize(AnyCast<string>(params["path"]));
	}

	virtual ~ConditionalQHistogramPrFunction()
	{
		reset();
	}

	void initialize(const string& path);

	Decimal operator()(const I64u x1, const I64u x2) const;

	Decimal pdf(I64u x1, I64u x2) const;

	Decimal cdf(I64u x1, I64u x2) const;

	I64u invcdf(Decimal y, I64u x2) const;

	I64u sample(Decimal r, I64u x2) const;

private:

	void reset()
	{
		if (_x2Pr != NULL)
		{
			delete _x2Pr;
		}

		if (_x1Pr != NULL)
		{
			delete[] _x1Pr;
		}
	}

	QHistogramPrFunction* _x2Pr;
	QHistogramPrFunction* _x1Pr;
};

inline Decimal ConditionalQHistogramPrFunction::operator()(const I64u x1, const I64u x2) const
{
	return cdf(x1, x2);
}

inline I64u ConditionalQHistogramPrFunction::sample(Decimal r, I64u x2) const
{
	return invcdf(r, x2);
}

} // namespace Myriad

#endif /* CONDITIONALQHISTOGRAMPRFUNCTION_H_ */
