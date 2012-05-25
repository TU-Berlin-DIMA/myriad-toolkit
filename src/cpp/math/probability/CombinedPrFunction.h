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

#ifndef COMBINEDPRFUNCTION_H_
#define COMBINEDPRFUNCTION_H_

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

class CombinedPrFunction: public UnivariatePrFunction<I64u>
{
public:

	CombinedPrFunction() :
		UnivariatePrFunction<I64u>(""), _numberOfBuckets(0)
	{
	}

	CombinedPrFunction(const string& path) :
		UnivariatePrFunction<I64u>(""), _numberOfBuckets(0)
	{
		initialize(path);
	}

	CombinedPrFunction(ifstream& in) :
		UnivariatePrFunction<I64u>(""), _numberOfBuckets(0)
	{
		initialize(in);
	}

	CombinedPrFunction(const string& name, const string& path) :
		UnivariatePrFunction<I64u>(name), _numberOfBuckets(0)
	{
		initialize(path);
	}

	CombinedPrFunction(const string& name, ifstream& in) :
		UnivariatePrFunction<I64u>(name), _numberOfBuckets(0)
	{
		initialize(in);
	}

	CombinedPrFunction(map<string, Any>& params) :
		UnivariatePrFunction<I64u>(""), _numberOfBuckets(0)
	{
		initialize(AnyCast<string>(params["path"]));
	}

	CombinedPrFunction(const string& name, map<string, Any>& params) :
		UnivariatePrFunction<I64u>(name), _numberOfBuckets(0)
	{
		initialize(AnyCast<string>(params["path"]));
	}

	virtual ~CombinedPrFunction()
	{
		reset();
	}

	void initialize(const string& path);

	void initialize(ifstream& path);

	size_t numberOfBuckets() const;

	I64u min()const;

	I64u max() const;

	size_t findIndex(const Decimal y) const;

	size_t findValue(const I64u x) const;

	size_t findBucket(const I64u x) const;

	Decimal operator()(const I64u x) const;

	Decimal pdf(I64u x) const;

	Decimal cdf(I64u x) const;

	I64u invcdf(Decimal x) const;

	I64u sample(Decimal random) const;

private:

	void reset()
	{
		_notNullProbability = 0.0;

		if (_numberOfValues > 0 || _numberOfBuckets > 0)
		{
			delete[] _cdfMap;
		}

		if (_numberOfValues > 0)
		{
			_numberOfValues = 0;
			_valueProbability = 0.0;
			delete[] _values;
			delete[] _valueProbabilities;
		}

		if (_numberOfBuckets > 0)
		{
			_numberOfBuckets = 0;
			_bucketProbability = 0.0;
			delete[] _bucketRanges;
			delete[] _bucketProbabilities;
		}
	}

	Decimal _notNullProbability;

	// TODO: replace with an interval
	I64u _min;
	I64u _max;

	size_t _numberOfValues;
	I64u* _values;
	Decimal _valueProbability;
	Decimal* _valueProbabilities;

	size_t _numberOfBuckets;
	Interval<I64u>* _bucketRanges;
	Decimal _bucketProbability;
	Decimal* _bucketProbabilities;

	Decimal* _cdfMap;
};

inline size_t CombinedPrFunction::numberOfBuckets() const
{
	return _numberOfBuckets;
}

inline I64u CombinedPrFunction::min()const
{
	return _min;
}

inline I64u CombinedPrFunction::max() const
{
	return _max;
}

inline size_t CombinedPrFunction::findIndex(const Decimal y) const
{
	// we assert that the value x is in the [_min, _max] range
	int min = 0;
	int max = _numberOfValues + _numberOfBuckets - 1;
	int mid = 0;

	int origMax = _numberOfValues + _numberOfBuckets - 1;

	// continue searching while [min, max] is not empty
	while (max >= min)
	{
		// calculate the midpoint for roughly equal partition //
		mid = (min + max) / 2;

		// determine which subarray to search
		if (mid == origMax && _cdfMap[mid] <= y)
		{
			return mid;
		}
		else if (_cdfMap[mid+1] <=  y)
		{
			// change min index to search upper subarray
			min = mid + 1;
		}
		else if (_cdfMap[mid] > y)
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

inline size_t CombinedPrFunction::findValue(const I64u x) const
{
	// we assert that the value x is in the [_min, _max] range
	int min = 0;
	int max = _numberOfValues - 1;
	int mid = 0;

	// continue searching while [min, max] is not empty
	while (max >= min)
	{
		// calculate the midpoint for roughly equal partition //
		mid = (min + max) / 2;

		// determine which subarray to search
		if (_values[mid] <  x)
		{
			// change min index to search upper subarray
			min = mid + 1;
		}
		else if (_values[mid] > x)
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
	return nullValue<size_t>();
}

inline size_t CombinedPrFunction::findBucket(const I64u x) const
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
		if (_bucketRanges[mid].max() <=  x)
		{
			// change min index to search upper subarray
			min = mid + 1;
		}
		else if (_bucketRanges[mid].min() > x)
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

inline Decimal CombinedPrFunction::operator()(const I64u x) const
{
	return cdf(x);
}

inline I64u CombinedPrFunction::sample(Decimal random) const
{
	return invcdf(random);
}

} // namespace Myriad

#endif /* COMBINEDPRFUNCTION_H_ */
