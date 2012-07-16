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
#include <fstream>

using namespace std;
using namespace Poco;

namespace Myriad
{

template<typename T> class CombinedPrFunction: public UnivariatePrFunction<T>
{
public:

	CombinedPrFunction() :
		UnivariatePrFunction<T>(""), _numberOfValues(0), _numberOfBuckets(0)
	{
	}

	CombinedPrFunction(const string& path) :
		UnivariatePrFunction<T>(""), _numberOfValues(0), _numberOfBuckets(0)
	{
		initialize(path);
	}

	CombinedPrFunction(istream& in) :
		UnivariatePrFunction<T>(""), _numberOfValues(0), _numberOfBuckets(0)
	{
		initialize(in);
	}

	CombinedPrFunction(const string& name, const string& path) :
		UnivariatePrFunction<T>(name), _numberOfValues(0), _numberOfBuckets(0)
	{
		initialize(path);
	}

	CombinedPrFunction(const string& name, istream& in) :
		UnivariatePrFunction<T>(name), _numberOfValues(0), _numberOfBuckets(0)
	{
		initialize(in);
	}

	CombinedPrFunction(map<string, Any>& params) :
		UnivariatePrFunction<T>(""), _numberOfValues(0), _numberOfBuckets(0)
	{
		initialize(AnyCast<string>(params["path"]));
	}

	CombinedPrFunction(const string& name, map<string, Any>& params) :
		UnivariatePrFunction<T>(name), _numberOfValues(0), _numberOfBuckets(0)
	{
		initialize(AnyCast<string>(params["path"]));
	}

	virtual ~CombinedPrFunction()
	{
		reset();
	}

	void initialize(const string& path);

	void initialize(istream& path);

	size_t numberOfBuckets() const;

	T min()const;

	T max() const;

	Decimal operator()(const T x) const;

	Decimal pdf(T x) const;

	Decimal cdf(T x) const;

	T invcdf(Decimal x) const;

	T sample(Decimal random) const;

private:

	void reset();

	void normalize();

	size_t findIndex(const Decimal y) const;

	size_t findValue(const T x, bool exact = true) const;

	size_t findBucket(const T x, bool exact = true) const;

	Decimal _notNullProbability;

	// TODO: replace with an interval
	T _min;
	T _max;

	size_t _numberOfValues;
	T* _values;
	Decimal _valueProbability;
	Decimal* _valueProbabilities;

	size_t _numberOfBuckets;
	Interval<T>* _buckets;
	Decimal _bucketProbability;
	Decimal* _bucketProbabilities;

	Decimal* _cumulativeProbabilites;
};

template<typename T> void CombinedPrFunction<T>::reset()
{
	_notNullProbability = 0.0;
	_valueProbability = 0.0;
	_bucketProbability = 0.0;

	if (_numberOfValues > 0 || _numberOfBuckets > 0)
	{
		delete[] _cumulativeProbabilites;
	}

	if (_numberOfValues > 0)
	{
		_numberOfValues = 0;
		delete[] _values;
		delete[] _valueProbabilities;
	}

	if (_numberOfBuckets > 0)
	{
		_numberOfBuckets = 0;
		delete[] _buckets;
		delete[] _bucketProbabilities;
	}
}

template<typename T> void CombinedPrFunction<T>::normalize()
{
	Decimal normalizationFactor = 1.0 / static_cast<Decimal>(_valueProbability + _bucketProbability + (1.0 - _notNullProbability));

	_valueProbability = 0;
	_bucketProbability = 0;

	for (size_t i = 0; i < _numberOfValues; i++)
	{
		Decimal probability = _valueProbabilities[i] * normalizationFactor;

		_valueProbabilities[i] = probability;
		_valueProbability += probability;
		_cumulativeProbabilites[i] = _valueProbability;
	}

	for (size_t i = 0; i < _numberOfBuckets; i++)
	{
		Decimal probability = _bucketProbabilities[i] * normalizationFactor;

		_bucketProbabilities[i] = probability;
		_bucketProbability += probability;
		_cumulativeProbabilites[i+_numberOfValues] = _valueProbability + _bucketProbability;
	}

	_notNullProbability = _valueProbability + _bucketProbability;
}

template<typename T> inline size_t CombinedPrFunction<T>::numberOfBuckets() const
{
	return _numberOfBuckets;
}

template<typename T> inline T CombinedPrFunction<T>::min()const
{
	return _min;
}

template<typename T> inline T CombinedPrFunction<T>::max() const
{
	return _max;
}

template<typename T> inline size_t CombinedPrFunction<T>::findIndex(const Decimal y) const
{
	// we assert that the value x is in the [_min, _max] range
	int min = 0;
	int max = _numberOfValues + _numberOfBuckets - 1;
	int mid = 0;

	// continue searching while [min, max] is not empty
	while (max >= min)
	{
		// calculate the midpoint for roughly equal partition //
		mid = (min + max) / 2;

		// determine which subarray to search
		if (_cumulativeProbabilites[mid] <  y)
		{
			// change min index to search upper subarray
			min = mid + 1;
		}
		else if (_cumulativeProbabilites[mid] > y)
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

	if (static_cast<size_t>(mid) < _numberOfValues + _numberOfBuckets - 1 && _cumulativeProbabilites[mid] < y)
	{
		return mid+1;
	}
	else
	{
		return mid;
	}
}

template<typename T> inline size_t CombinedPrFunction<T>::findValue(const T x, bool exact) const
{
	// we assert that the value x is in the [_min, _max] range
	int min = 0;
	int max = _numberOfValues - 1;
	int mid = 0;

	// continue searching while [min, max] is not empty
	while (max >= min)
	{
		// calculate the midpoint for roughly equal partition
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

	// at this point, mid is always equal to min and points to the first record higher than x
	// what we actually want the index of the first value less than or equal to x
	if (!exact && mid > 0)
	{
		if (static_cast<size_t>(mid) < _numberOfValues-2 && _values[mid+1] < x)
		{
			return mid+1;
		}
		else if (_values[mid] < x)
		{
			return mid;
		}
		else
		{
			return mid-1;
		}
	}
	else
	{
		return nullValue<size_t>();
	}
}

template<typename T> inline size_t CombinedPrFunction<T>::findBucket(const T x, bool exact) const
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

	// at this point, mid is always equal to min and points to the first bucket where higher than x
	// what we actually want the index of the first bucket less than but not containing x
	if (!exact && mid > 0)
	{
		if (static_cast<size_t>(mid) < _numberOfBuckets-2 && _buckets[mid+1].max() <= x)
		{
			return mid+1;
		}
		else if (_buckets[mid].max() <= x)
		{
			return mid;
		}
		else
		{
			return mid-1;
		}
	}
	else
	{
		return nullValue<size_t>();
	}
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// inline member function templates
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename T> inline Decimal CombinedPrFunction<T>::operator()(const T x) const
{
	return cdf(x);
}

template<typename T> inline T CombinedPrFunction<T>::sample(Decimal random) const
{
	return invcdf(random);
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// member function templates
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

/**
 * Load a Q-histogram from the given path.
 */
template<typename T> void CombinedPrFunction<T>::initialize(const string& path)
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
    catch(Exception& e)
    {
        in.close();
        throw e;
    }
    catch(exception& e)
    {
        in.close();
        throw e;
    }
    catch(...)
    {
        in.close();
        throw;
    }
}

/**
 * Load a Q-histogram from the given input stream. For each entry add a
 * [min, max) interval of domain values to a lookup table.
 */
template<typename T> void CombinedPrFunction<T>::initialize(istream& in)
{
	// reset old state
	reset();

	string line, binMin, binMax;

	// read first line
	getline(in, line);
	if (!in.good() || line.substr(0, 20) != "# numberofexactvals:")
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
	if (!in.good() || line.substr(0, 15) != "# numberofbins:")
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
	if (!in.good() || line.substr(0, 18) != "# nullprobability:")
	{
		throw DataException("Unexpected file header (line 3)");
	}
	Decimal nullProbability = atof(line.substr(18).c_str());

	_notNullProbability = 1.0 - nullProbability;

	_numberOfValues = numberOfValues;
	_values = new T[numberOfValues];
	_valueProbabilities = new Decimal[numberOfValues];

	_numberOfBuckets = numberOfBuckets;
	_buckets = new Interval<T>[numberOfBuckets];
	_bucketProbabilities = new Decimal[numberOfBuckets];

	_cumulativeProbabilites = new Decimal[numberOfValues+numberOfBuckets];

	for (I16u i = 0; i < numberOfValues; i++)
	{
		if (!in.good())
		{
			throw DataException("Bad line for bin #" + toString(i));
		}

		getline(in, line);

		size_t tab1 = line.find_first_of('\t');
        size_t lend = line.find_last_of('#');

        if (lend == string::npos)
        {
            lend = line.length();
        }

		Decimal probability = fromString<Decimal>(line.substr(0, tab1));
		T value = fromString<T>(line.substr(tab1+1, lend-tab1-1));

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

		size_t tab1 = line.find_first_of('\t');
		size_t tab2 = line.find_last_of('\t');
        size_t lend = line.find_last_of('#');

        if (lend == string::npos)
        {
            lend = line.length();
        }

		Decimal probability = fromString<Decimal>(line.substr(0, tab1));
		T min = fromString<T>(line.substr(tab1+1, tab2-tab1-1));
		T max = fromString<T>(line.substr(tab2+1, lend-tab2-1));

		_buckets[i].set(min, max);
		_bucketProbabilities[i] = probability;
		_bucketProbability += probability;
		_cumulativeProbabilites[i+numberOfValues] = _valueProbability + _bucketProbability;
	}

	_min = std::min(_buckets[0].min(), _values[0]);
	_max = std::max(_buckets[_numberOfBuckets-1].max(), _values[_numberOfBuckets-1]+1);

	if (std::abs(_valueProbability + _bucketProbability - _notNullProbability) >= 0.00001)
	{
		normalize();
	}
}

template<typename T> Decimal CombinedPrFunction<T>::pdf(T x) const
{
	if (x == nullValue<T>())
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

		throw RuntimeException("Unknown pdf(x) for x = " + toString<T>(x));
	}
}

template<typename T> Decimal CombinedPrFunction<T>::cdf(T x) const
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
			const Interval<T>& b = _buckets[i];

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

				cdf += _bucketProbabilities[i] * ((1 + (x - b.min()))/static_cast<Decimal>(b.length()));
			}
		}

		return cdf;
	}
}

template<typename T> T CombinedPrFunction<T>::invcdf(Decimal y) const
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
			const Interval<T> b = _buckets[i-_numberOfValues];
			Decimal cdfBefore = i > 0 ? _cumulativeProbabilites[i-1] : 0;
			return static_cast<T>(b.min() + ((y - cdfBefore) / _bucketProbabilities[i-_numberOfValues]) * b.length());
		}
	}
	else
	{
		return nullValue<T>();
	}
}

} // namespace Myriad

#endif /* COMBINEDPRFUNCTION_H_ */
