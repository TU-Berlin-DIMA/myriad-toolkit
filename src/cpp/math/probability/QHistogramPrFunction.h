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

#include <Poco/Exception.h>
#include <Poco/Any.h>

#include <string>
#include <iostream>
#include <fstream>

using namespace std;
using namespace Poco;

namespace Myriad
{

class QHistogramPrFunction: public UnivariatePrFunction<I64u>
{
public:

	QHistogramPrFunction() :
		UnivariatePrFunction<I64u>(""), _numberOfBuckets(0)
	{
	}

	QHistogramPrFunction(const string& path) :
		UnivariatePrFunction<I64u>(""), _numberOfBuckets(0)
	{
		initialize(path);
	}

	QHistogramPrFunction(ifstream& in) :
		UnivariatePrFunction<I64u>(""), _numberOfBuckets(0)
	{
		initialize(in);
	}

	QHistogramPrFunction(const string& name, const string& path) :
		UnivariatePrFunction<I64u>(name), _numberOfBuckets(0)
	{
		initialize(path);
	}

	QHistogramPrFunction(const string& name, ifstream& in) :
		UnivariatePrFunction<I64u>(name), _numberOfBuckets(0)
	{
		initialize(in);
	}

	QHistogramPrFunction(map<string, Any>& params) :
		UnivariatePrFunction<I64u>(""), _numberOfBuckets(0)
	{
		initialize(AnyCast<string>(params["path"]));
	}

	QHistogramPrFunction(const string& name, map<string, Any>& params) :
		UnivariatePrFunction<I64u>(name), _numberOfBuckets(0)
	{
		initialize(AnyCast<string>(params["path"]));
	}

	virtual ~QHistogramPrFunction()
	{
		reset();
	}

	void initialize(const string& path);

	void initialize(ifstream& path);

	size_t numberOfBuckets() const;

	I64u min()const;

	I64u max() const;

	size_t findBucket(const I64u x) const;

	Decimal operator()(const I64u x) const;

	Decimal pdf(I64u x) const;

	Decimal cdf(I64u x) const;

	I64u invcdf(Decimal x) const;

	I64u sample(Decimal random) const;

private:

	void reset()
	{
		if (_numberOfBuckets > 0)
		{
			_numberOfBuckets = 0;
			_bucketProbability = 1.0;
			delete[] _buckets;
		}
	}

	Interval<I64u>* _buckets;
	I64u _min;
	I64u _max;
	size_t _numberOfBuckets;
	Decimal _bucketProbability;
};

inline size_t QHistogramPrFunction::numberOfBuckets() const
{
	return _numberOfBuckets;
}

inline I64u QHistogramPrFunction::min()const
{
	return _min;
}

inline I64u QHistogramPrFunction::max() const
{
	return _max;
}

inline size_t QHistogramPrFunction::findBucket(const I64u x) const
{
	// we assert that the value x is in the [_min, _max] range
	int min = 0;
	int max = _numberOfBuckets - 1;
	int mid = 0;

	// continue searching while [min, max] is not empty
	while (max >= min)
	{
		// calculate the midpoint for roughly equal partition //
		mid = (min + max) / 2;

		// determine which subarray to search
		if (_buckets[mid].max() <=  x)
		{
			// change min index to search upper subarray
			min = mid + 1;
		}
		else if (_buckets[mid].min() > x)
		{
			// change max index to search lower subarray
			max = mid - 1;
		}
		else
		{
			// key found at index mid
			return mid;
		}
	}

	// key not found
	return mid;
}

inline Decimal QHistogramPrFunction::operator()(const I64u x) const
{
	return cdf(x);
}

inline I64u QHistogramPrFunction::sample(Decimal random) const
{
	return invcdf(random);
}

} // namespace Myriad

#endif /* QHISTOGRAMPRFUNCTION_H_ */
