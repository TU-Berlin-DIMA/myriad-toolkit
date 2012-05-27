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
#include "math/probability/ConditionalQHistogramPrFunction.h"
#include "math/probability/CustomDiscreteProbability.h"
#include "math/probability/NormalPrFunction.h"
#include "math/probability/ParetoPrFunction.h"
#include "math/probability/QHistogramPrFunction.h"
#include "math/probability/UniformPrFunction.h"

#include <algorithm>

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// Bounded Pareto probability
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

Decimal BoundedParetoPrFunction::pdf(Decimal x) const
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

Decimal BoundedParetoPrFunction::cdf(Decimal x) const
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

Decimal BoundedParetoPrFunction::invpdf(Decimal y) const
{
	return pow((y * B) / (alpha * xMinAlpha), 1 / (-alpha - 1));
}

Decimal BoundedParetoPrFunction::invcdf(Decimal y) const
{
	return pow(-(y*xMaxAlpha - y*xMinAlpha - xMaxAlpha)/(xMinAlpha*xMaxAlpha), -1/alpha);
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// CombinedPrFunction probability
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

/**
 * Load a Q-histogram from the given path.
 */
void CombinedPrFunction::initialize(const string& path)
{
	ifstream in(path.c_str());

	if (!in.is_open())
	{
		throw OpenFileException("Unexpected file header for file `" + path +  "`");
	}

	try
	{
		initialize(in);

		in.close();
	}
	catch(exception& e)
	{
		in.close();
		throw e;
	}
}

/**
 * Load a Q-histogram from the given input stream. For each entry add a
 * [min, max) interval of domain values to a lookup table.
 */
void CombinedPrFunction::initialize(ifstream& in)
{
	// reset old state
	reset();

	string line, binMin, binMax;

	// read first line
	getline(in, line);
	if (line.substr(0, 20) != "# numberofexactvals:")
	{
		throw DataException("Unexpected file header (line 1)");
	}
	I32 numberOfValues = atoi(line.substr(20).c_str());

	if (numberOfValues <= 0 && numberOfValues > 65536)
	{
		throw DataException("Invalid number of exact values `" + toString(numberOfValues) +  "`");
	}

	// read second line
	getline(in, line);
	if (line.substr(0, 15) != "# numberofbins:")
	{
		throw DataException("Unexpected file header (line 2)");
	}
	I32 numberOfBuckets = atoi(line.substr(15).c_str());

	if (numberOfBuckets <= 0 && numberOfBuckets > 65536)
	{
		throw DataException("Invalid number of lines `" + toString(numberOfBuckets) +  "`");
	}

	// read third line
	getline(in, line);
	if (line.substr(0, 18) != "# nullprobability:")
	{
		throw DataException("Unexpected file header (line 3)");
	}
	Decimal nullProbability = atof(line.substr(18).c_str());

	_notNullProbability = 1.0 - nullProbability;


	_numberOfValues = numberOfValues;
	_values = new I64u[numberOfValues];
	_valueProbabilities = new Decimal[numberOfValues];

	_numberOfBuckets = numberOfBuckets;
	_buckets = new Interval<I64u>[numberOfBuckets];
	_bucketProbabilities = new Decimal[numberOfBuckets];

	_cumulativeProbabilites = new Decimal[numberOfValues+numberOfBuckets];

	for (I16u i = 0; i < numberOfBuckets; i++)
	{
		if (!in.good())
		{
			throw DataException("Bad line for bin #" + toString(i));
		}

		getline(in, line);

		size_t firsttab = line.find_first_of('\t');

		Decimal probability = fromString<Decimal>(line.substr(0, firsttab));
		I64u value = fromString<I64u>(line.substr(firsttab));

		_values[i] = value;
		_valueProbabilities[i] = probability;
		_valueProbability += probability;
		_cumulativeProbabilites[i] = _valueProbability;
	}

	for (I16u i = 0; i < numberOfBuckets; i++)
	{
		if (!in.good())
		{
			throw DataException("Bad line for bin #" + toString(i));
		}

		getline(in, line);

		size_t firsttab = line.find_first_of('\t');
		size_t secondtab = line.find_last_of('\t');

		Decimal probability = fromString<Decimal>(line.substr(0, firsttab));
		I64u min = fromString<I64u>(line.substr(firsttab, secondtab));
		I64u max = fromString<I64u>(line.substr(secondtab));

		_buckets[i].set(min, max);
		_bucketProbabilities[i] = probability;
		_bucketProbability += probability;
		_cumulativeProbabilites[i+numberOfValues] = _valueProbability + _bucketProbability;
	}

	_min = std::min(_buckets[0].min(), _values[0]);
	_max = std::max(_buckets[_numberOfBuckets-1].max(), _values[_numberOfBuckets-1]+1);

	if (std::abs(_valueProbability + _bucketProbability - _notNullProbability) >= 0.00001)
	{
		throw LogicException("Probabilities don't sum up to 1");
	}
}

Decimal CombinedPrFunction::pdf(I64u x) const
{
	if (x == nullValue<I64u>())
	{
		return 1.0-_notNullProbability;
	}
	else if (x < _min || x >= _max)
	{
		return 0.0;
	}
	else
	{
		size_t i;

		if (nullValue<size_t>() != (i = findValue(x)))
		{
			return _valueProbabilities[i];
		}

		if (nullValue<size_t>() != (i = findBucket(x)))
		{
			return _bucketProbabilities[i] * (1.0 / static_cast<Decimal>(_buckets[i].length()));
		}

		throw RuntimeException("Unknown pdf(x) for x = " + x);
	}
}

Decimal CombinedPrFunction::cdf(I64u x) const
{
	if (x < _min)
	{
		return 0.0;
	}
	else if (x >= _max)
	{
		return _notNullProbability;
	}
	else
	{
		Decimal cdf = 0.0;

		size_t i;

		if (nullValue<size_t>() != (i = findValue(x, false)))
		{
			cdf += _cumulativeProbabilites[i];
		}

		if (nullValue<size_t>() != (i = findBucket(x, false)))
		{
			const Interval<I64u>& b = _buckets[i];

			if (!b.contains(x))
			{
				cdf += _cumulativeProbabilites[_numberOfValues + i] - _valueProbability;
			}
			else
			{
				if (i > 0)
				{
					cdf += _cumulativeProbabilites[_numberOfValues + i - 1] - _valueProbability;
				}

				cdf += _bucketProbabilities[i] * ((1 + x - b.min())/static_cast<Decimal>(b.length()));
			}
		}

		return cdf;
	}
}

I64u CombinedPrFunction::invcdf(Decimal y) const
{
	if (y < _notNullProbability)
	{
		size_t i = findIndex(y);

		if (i < _numberOfValues)
		{
			return _values[i];
		}
		else
		{
			const Interval<I64u> b = _buckets[i-_numberOfValues];
			Decimal cdfBefore = i > 0 ? _cumulativeProbabilites[i-1] : 0;
			return static_cast<I64u>(b.min() + ((y - cdfBefore) / _bucketProbabilities[i-_numberOfValues]) * b.length());
		}
	}
	else
	{
		return nullValue<I64u>();
	}
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

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// QHistogram probability
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

/**
 * Load a Q-histogram from the given path.
 */
void QHistogramPrFunction::initialize(const string& path)
{
	ifstream in(path.c_str());

	if (!in.is_open())
	{
		throw OpenFileException(path);
	}

	try
	{
		initialize(in);

		in.close();
	}
	catch(exception& e)
	{
		in.close();
		throw e;
	}
}

/**
 * Load a Q-histogram from the given input stream. For each entry add a
 * [min, max) interval of domain values to a lookup table.
 */
void QHistogramPrFunction::initialize(ifstream& in)
{
	// reset old state
	reset();

	string line, binMin, binMax;

	// read first line
	getline(in, line);

	if (line.substr(0, 15) != "# numberofbins:")
	{
		throw DataException("Unexpected file header");
	}

	I32 numberOfLines = atoi(line.substr(15).c_str());

	_buckets = new Interval<I64u>[numberOfLines];
	_numberOfBuckets = numberOfLines;
	_bucketProbability = 1.0/numberOfLines;

	if (numberOfLines <= 0 && numberOfLines > 65536)
	{
		throw LogicException("Invalid number of lines `" + toString(numberOfLines) +  "`");
	}

	for (I16u i = 0; i < numberOfLines; i++)
	{
		if (!in.good())
		{
			throw LogicException("Bad line for bin #" + toString(i));
		}

		getline(in, line);

		size_t firsttab = line.find_first_of('\t');

		I64u min = fromString<I64u>(line.substr(0, firsttab));
		I64u max = fromString<I64u>(line.substr(firsttab));

		_buckets[i].set(min, max);

		if (i > 0)
		{
			if (_buckets[i].min() != _buckets[i-1].max())
			{
				throw LogicException("Bad line for bin #" + toString(i));
			}
		}
	}

	_min = _buckets[0].min();
	_max = _buckets[_numberOfBuckets-1].max();
}

Decimal QHistogramPrFunction::pdf(I64u x) const
{
	if (x < _min || x >= _max)
	{
		return 0.0;
	}
	else
	{
		return _bucketProbability * (1.0 / static_cast<Decimal>(_buckets[findBucket(x)].length()));
	}
}

Decimal QHistogramPrFunction::cdf(I64u x) const
{
	if (x < _min)
	{
		return 0.0;
	}
	else if (x >= _max)
	{
		return 1.0;
	}
	else
	{
		size_t i = findBucket(x);
		Interval<I64u>& b = _buckets[i];

		return _bucketProbability * i + _bucketProbability * ((x - b.min()) / static_cast<Decimal>(b.length()));
	}
}

I64u QHistogramPrFunction::invcdf(Decimal y) const
{
	// locate the bin
	size_t i = static_cast<size_t>(y / _bucketProbability);
	return static_cast<I64u>(_buckets[i].min() + (y - i * _bucketProbability) * _numberOfBuckets * _buckets[i].length());
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// ConditionalQHistogram probability
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

/**
 * Load a Q-histogram from the given path. For each entry add a [min, max)
 * interval of domain values to a lookup table.
 */
void ConditionalQHistogramPrFunction::initialize(const string& path)
{
	ifstream in(path.c_str());

	if (!in.is_open())
	{
		throw OpenFileException("Cannot open file `" + path +  "`");
	}

	try
	{
		reset();

		_x2Pr = new QHistogramPrFunction(in);

		_x1Pr = new QHistogramPrFunction[_x2Pr->numberOfBuckets()];

		for (size_t i = 0; i < _x2Pr->numberOfBuckets(); i++)
		{
			_x1Pr[i].initialize(in);
		}

		in.close();
	}
	catch(exception& e)
	{
		in.close();
		throw e;
	}
}

Decimal ConditionalQHistogramPrFunction::pdf(I64u x1, I64u x2) const
{
	if (x2 < _x2Pr->min() || x2 >= _x2Pr->max())
	{
		return 0.0;
	}
	else
	{
		return _x1Pr[_x2Pr->findBucket(x2)].pdf(x1);
	}
}

Decimal ConditionalQHistogramPrFunction::cdf(I64u x1, I64u x2) const
{
	if (x2 < _x2Pr->min() || x2 >= _x2Pr->max())
	{
		return 0.0;
	}
	else
	{
		return _x1Pr[_x2Pr->findBucket(x2)].cdf(x1);
	}
}

I64u ConditionalQHistogramPrFunction::invcdf(Decimal y, I64u x2) const
{
	return _x1Pr[_x2Pr->findBucket(x2)].invcdf(y);
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// Uniform probability
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

Decimal UniformPrFunction::pdf(Decimal x) const
{
	if (x < _xMin || x >= _xMax)
	{
		return 0;
	}
	else
	{
		return _xPDF;
	}
}

Decimal UniformPrFunction::cdf(Decimal x) const
{
	if (x < _xMin)
	{
		return 0;
	}
	else if (x >= _xMax)
	{
		return 1;
	}
	else
	{
		return (x - _xMin) * _xPDF;
	}
}

Decimal UniformPrFunction::invcdf(Decimal y) const
{
	return _xMin + y * _size;
}

} // namespace Myriad
