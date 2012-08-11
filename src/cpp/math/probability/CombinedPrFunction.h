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

#include <Poco/Any.h>
#include <Poco/Exception.h>
#include <Poco/String.h>
#include <Poco/RegularExpression.h>

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
		UnivariatePrFunction<T>(""), _numberOfValues(0), _numberOfBuckets(0), _EPSILON(0.000001)
	{
	}

	CombinedPrFunction(const string& path) :
		UnivariatePrFunction<T>(""), _numberOfValues(0), _numberOfBuckets(0), _EPSILON(0.000001)
	{
		initialize(path);
	}

	CombinedPrFunction(istream& in) :
		UnivariatePrFunction<T>(""), _numberOfValues(0), _numberOfBuckets(0), _EPSILON(0.000001)
	{
		initialize(in);
	}

	CombinedPrFunction(const string& name, const string& path) :
		UnivariatePrFunction<T>(name), _numberOfValues(0), _numberOfBuckets(0), _EPSILON(0.000001)
	{
		initialize(path);
	}

	CombinedPrFunction(const string& name, istream& in) :
		UnivariatePrFunction<T>(name), _numberOfValues(0), _numberOfBuckets(0), _EPSILON(0.000001)
	{
		initialize(in);
	}

	CombinedPrFunction(map<string, Any>& params) :
		UnivariatePrFunction<T>(""), _numberOfValues(0), _numberOfBuckets(0), _EPSILON(0.000001)
	{
		initialize(AnyCast<string>(params["path"]));
	}

	CombinedPrFunction(const string& name, map<string, Any>& params) :
		UnivariatePrFunction<T>(name), _numberOfValues(0), _numberOfBuckets(0), _EPSILON(0.000001)
	{
		initialize(AnyCast<string>(params["path"]));
	}

	virtual ~CombinedPrFunction()
	{
		reset();
	}

	void initialize(const string& path);

	void initialize(istream& path);

	void initialize(istream& in, I16u& currentLineNumber);

	size_t numberOfBuckets() const;

	T min() const;

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

	static RegularExpression headerLine1Format;
	static RegularExpression headerLine2Format;
	static RegularExpression headerLine3Format;
	static RegularExpression valueLineFormat;
	static RegularExpression bucketLineFormat;

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

    Decimal _EPSILON;
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// static template members
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename T> RegularExpression CombinedPrFunction<T>::headerLine1Format("\\W*@numberofexactvals\\W*=\\W*([+]?[0-9]+)\\W*(#(.+))?");
template<typename T> RegularExpression CombinedPrFunction<T>::headerLine2Format("\\W*@numberofbins\\W*=\\W*([+]?[0-9]+)\\W*(#(.+))?");
template<typename T> RegularExpression CombinedPrFunction<T>::headerLine3Format("\\W*@nullprobability\\W*=\\W*([+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\W*(#(.+))?");
template<typename T> RegularExpression CombinedPrFunction<T>::valueLineFormat( "\\W*p\\(X\\)\\W*=\\W*([+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\W+for\\W+X\\W*=\\W*\\{\\W*(.+)\\W*\\}\\W*(#(.+))?");
template<typename T> RegularExpression CombinedPrFunction<T>::bucketLineFormat("\\W*p\\(X\\)\\W*=\\W*([+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\W+for\\W+X\\W*=\\W*\\{\\W*x\\W+in\\W+\\[\\W*(.+)\\W*,\\W*(.+)\\W*\\)\\W*\\}\\W*(#(.+))?");

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// private member function templates
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

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

template<typename T> inline size_t CombinedPrFunction<T>::findIndex(const Decimal y) const
{
	// we assert that the value x is in the [_min, _max] range
	int min = 0;
	int max = _numberOfValues + _numberOfBuckets - 1;
	int mid = 0;

	// protect against invalid y
	if (y >= 1.0)
	{
	    return max;
	}

	// continue searching while [min, max] is not empty
	while (max >= min)
	{
		// calculate the midpoint for roughly equal partition //
		mid = (min + max) / 2;

		// determine which subarray to search
		if (_cumulativeProbabilites[mid] <  y - _EPSILON)
		{
			// change min index to search upper subarray
			min = mid + 1;
		}
		else if (_cumulativeProbabilites[mid] > y + _EPSILON)
		{
			// change max index to search lower subarray
			max = mid - 1;
		}
		else
		{
		    // key found at index mid
		    // for all but the last position, increment the index by one to
		    // compensate for the fact that buckets are defined as Y < y rather than Y <= y
            return (static_cast<size_t>(mid) == _numberOfValues + _numberOfBuckets - 1) ? mid : mid+1;
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
    catch(Poco::Exception& e)
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

template<typename T> void CombinedPrFunction<T>::initialize(istream& in)
{
	I16u currentLineNumber = 1;
	initialize(in, currentLineNumber);
}

/**
 * Load the distribution data from the given input stream. For each entry add a
 * [min, max) interval of domain values to a lookup table.
 */
template<typename T> void CombinedPrFunction<T>::initialize(istream& in, I16u& currentLineNumber)
{
	enum READ_STATE { NOE, NOB, NPR, VLN, BLN, FIN, END };

	// reset old state
	reset();

	// reader variables
	READ_STATE currentState = NOE; // current reader machine state
	string currentLine; // the current line
	I16u currentItemIndex = 0; // current item index
	RegularExpression::MatchVec posVec; // a posVec for all regex matches

	// reader finite state machine
	while (currentState != END)
	{
		// the special FIN stage contains only final initialization constructs
		// and does not a currentLine
		if (currentState == FIN)
		{
			_min = std::min(_buckets[0].min(), _values[0]);
			_max = std::max(_buckets[_numberOfBuckets-1].max(), static_cast<T>(_values[_numberOfBuckets-1]+1));

			currentState = END;
			continue;
		}

		// read next line
		getline(in, currentLine);

		// trim whitespace
		trimInPlace(currentLine);

		// check if this line is empty or contains a single comment
		if (currentLine.empty() || currentLine.at(0) == '#')
		{
			currentLineNumber++;
			continue; // skip this line
		}

		if (currentState == NOE)
		{
			if (!in.good() || !headerLine1Format.match(currentLine, 0, posVec))
			{
				throw DataException(format("line %hu: Bad header line `%s`, should be: '@numberofexactvals = ' + x", currentLineNumber, currentLine));
			}

			I32 numberOfValues = atoi(currentLine.substr(posVec[1].offset, posVec[1].length).c_str());

			if (numberOfValues <= 0 && numberOfValues > 65536)
			{
				throw DataException("Invalid number of exact values '" + toString(numberOfValues) +  "'");
			}

			_numberOfValues = numberOfValues;
			_values = new T[numberOfValues];
			_valueProbabilities = new Decimal[numberOfValues];

			currentState = NOB;
		}
		else if (currentState == NOB)
		{
			if (!in.good() || !headerLine2Format.match(currentLine, 0, posVec))
			{
				throw DataException(format("line %hu: Bad header line `%s`, should be: '@numberofbins = ' + x", currentLineNumber, currentLine));
			}

			I32 numberOfBuckets = atoi(currentLine.substr(posVec[1].offset, posVec[1].length).c_str());

			if (numberOfBuckets <= 0 && numberOfBuckets > 65536)
			{
				throw DataException("Invalid number of buckets '" + toString(numberOfBuckets) +  "'");
			}

			_numberOfBuckets = numberOfBuckets;
			_buckets = new Interval<T>[numberOfBuckets];
			_bucketProbabilities = new Decimal[numberOfBuckets];

			_cumulativeProbabilites = new Decimal[_numberOfValues+_numberOfBuckets];

			currentState = NPR;
		}
		else if (currentState == NPR)
		{
			if (!in.good() || !headerLine3Format.match(currentLine, 0, posVec))
			{
				throw DataException(format("line %hu: Bad header line `%s`, should be: '@nullprobability = ' + x", currentLineNumber, currentLine));
			}

			_notNullProbability = 1.0 - atof(currentLine.substr(posVec[1].offset, posVec[1].length).c_str());

			currentItemIndex = 0;
			currentState = (_numberOfValues > 0) ? VLN : BLN;
		}
		else if (currentState == VLN)
		{
			if (!in.good() || !valueLineFormat.match(currentLine, 0, posVec))
			{
				throw DataException(format("line %hu: Bad value probability line `%s`, should be: 'p(X) = ' + p_x + ' for X = {' + x + ') }'", currentLineNumber, currentLine));
			}

			Decimal probability = fromString<Decimal>(currentLine.substr(posVec[1].offset, posVec[1].length));
			T value = fromString<T>(currentLine.substr(posVec[3].offset, posVec[3].length));

			_values[currentItemIndex] = value;
			_valueProbabilities[currentItemIndex] = probability;
			_valueProbability += probability;
			_cumulativeProbabilites[currentItemIndex] = _valueProbability;

			currentItemIndex++;

			if (currentItemIndex >= _numberOfValues)
			{
				currentState = (_numberOfBuckets > 0) ? BLN : FIN;
				currentItemIndex = 0;
			}
		}
		else if (currentState == BLN)
		{
			if (!in.good() || !bucketLineFormat.match(currentLine, 0, posVec))
			{
				throw DataException(format("line %hu: Bad bucket probability line `%s`, should be: 'p(X) = ' + p_x + ' for X = { x \\in [' + x_min + ', ' + x_max + ') }'", currentLineNumber, currentLine));
			}

			Decimal probability = fromString<Decimal>(currentLine.substr(posVec[1].offset, posVec[1].length));
			T min = fromString<T>(currentLine.substr(posVec[3].offset, posVec[3].length));
			T max = fromString<T>(currentLine.substr(posVec[4].offset, posVec[4].length));

			_buckets[currentItemIndex].set(min, max);
			_bucketProbabilities[currentItemIndex] = probability;
			_bucketProbability += probability;
			_cumulativeProbabilites[currentItemIndex+_numberOfValues] = _valueProbability + _bucketProbability;

			currentItemIndex++;

			if (currentItemIndex >= _numberOfBuckets)
			{
				currentState = FIN;
				currentItemIndex = 0;
			}
		}

		currentLineNumber++;
	}

	// protect against unexpected reader state
	if (currentState != END)
	{
		throw RuntimeException("Unexpected state in CombinedPrFunction reader at line " + currentLineNumber);
	}

	// check if extra normalization is required
	if (std::abs(_valueProbability + _bucketProbability - _notNullProbability) >= 0.00001)
	{
		normalize();
	}
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

		throw RuntimeException(AbstractFunction::name() + ": unknown pdf(x) for x = " + toString<T>(x));
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
			const Interval<T>& b = _buckets[i-_numberOfValues];
			Decimal cdfBefore = i > 0 ? _cumulativeProbabilites[i-1] : 0;

			Decimal z = ((y - cdfBefore) / _bucketProbabilities[i-_numberOfValues]) * b.length();

			T x = static_cast<T>(b.min() + static_cast<I32u>(z));

			if ((z - static_cast<I32u>(z) >= 0.999999))
			{
				x++;
			}

			// FIXME: a quick and dirty hack to protect against out of range behavior, needs rewrite
			if (x >= b.max())
			{
				x--;
			}

			return x;
		}
	}
	else
	{
		return nullValue<T>();
	}
}

} // namespace Myriad

#endif /* COMBINEDPRFUNCTION_H_ */
