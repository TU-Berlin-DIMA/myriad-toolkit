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

#include "core/types.h"
#include "math/probability/BoundedParetoPrFunction.h"
#include "math/probability/CombinedPrFunction.h"
#include "math/probability/NormalPrFunction.h"
#include "math/probability/ParetoPrFunction.h"

#include <algorithm>

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// Bounded Pareto probability
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

Decimal BoundedParetoPrFunction::pdf(I64u x) const
{
	if (x < xMin || x > xMax)
	{
		return 0;
	}
	else
	{
		return (alpha * xMinAlpha * pow(x, -alpha - 1)) / B;
	}
}

Decimal BoundedParetoPrFunction::cdf(I64u x) const
{
	if (x < xMin)
	{
		return 0;
	}
	else if (x > xMax)
	{
		return 1;
	}
	else
	{
		return (1 - xMinAlpha * pow(x, -alpha)) / B;
	}
}

I64u BoundedParetoPrFunction::invpdf(Decimal y) const
{
	return pow((y * B) / (alpha * xMinAlpha), 1 / (-alpha - 1));
}

I64u BoundedParetoPrFunction::invcdf(Decimal y) const
{
	return pow(-(y*xMaxAlpha - y*xMinAlpha - xMaxAlpha)/(xMinAlpha*xMaxAlpha), -1/alpha);
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// Normal probability
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

Decimal NormalPrFunction::pdf(Decimal x) const
{
	return _A * exp(-(x - _mean) * (x - _mean) / (2 * _var));
}

Decimal NormalPrFunction::cdf(Decimal x) const
{
	return 0.5 * (1 + erf((x - _mean) / sqrt(2 * _var)));
}

Decimal NormalPrFunction::invpdf(Decimal y) const
{
	return sqrt(-log(y * (1 / _A) * 2 * _var)) + y; //TODO: this code is not tested
}

Decimal NormalPrFunction::invcdf(Decimal y) const
{
	return sqrt(2) * _stddev * inverf(2 * y - 1) + _mean;
}

Interval<Decimal> NormalPrFunction::threshold(Decimal yMin) const
{
	Decimal x = invpdf(yMin);
	return Interval<Decimal> (2*_mean - x, x);
}

Decimal NormalPrFunction::mean() const
{
	return _mean;
}

Decimal NormalPrFunction::stddev() const
{
	return _stddev;
}

Decimal NormalPrFunction::erf(const Decimal x) const
{
	Decimal P, Q, xx, axx, y;

	xx = x * x;
	axx = _a * xx;

	P = 4 / M_PI + axx;
	Q = 1 + axx;

	y = sqrt(1 - exp(-xx * (P / Q)));

	return (x < 0) ? -y : +y;
}

Decimal NormalPrFunction::inverf(const Decimal y) const
{
	Decimal ln, twopia, x;

	ln = log(1 - y * y);
	twopia = 2 / (M_PI * _a);
	x = 0;

	x -= twopia;
	x -= ln / 2;
	x += sqrt((twopia + ln / 2) * (twopia + ln / 2) - (1 / _a) * ln);
	x = sqrt(x);

	return (y < 0) ? -x : +x;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// Pareto probability
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

Decimal ParetoPrFunction::pdf(Decimal x) const
{
	if (x < xMin)
	{
		return 0;
	}
	else
	{
		return (alpha * xMinAlpha) / pow(x, alpha + 1);
	}
}

Decimal ParetoPrFunction::cdf(Decimal x) const
{
	if (x < xMin)
	{
		return 0;
	}
	else
	{
		return 1 - pow(xMin / x, alpha);
	}
}

Decimal ParetoPrFunction::invpdf(Decimal y) const
{
	return pow((alpha * xMinAlpha) / y, 1 / (alpha + 1));
}

Decimal ParetoPrFunction::invcdf(Decimal y) const
{
	return xMin / pow((1 - y), (1 / alpha));
}

} // namespace Myriad
