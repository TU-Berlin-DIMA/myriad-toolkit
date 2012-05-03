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

class QHistogramPrFunction: public AnalyticPrFunction<I64u>
{
public:

	QHistogramPrFunction(const string& path) :
			AnalyticPrFunction<I64u>("")
	{
		initialize(path);
	}

	QHistogramPrFunction(const string& name, const string& path) :
			AnalyticPrFunction<I64u>(name), _binsLength(0)
	{
		initialize(path);
	}

	QHistogramPrFunction(map<string, Any>& params) :
			AnalyticPrFunction<I64u>(""), _binsLength(0)
	{
		initialize(AnyCast<string>(params["path"]));
	}

	QHistogramPrFunction(const string& name, map<string, Any>& params) :
			AnalyticPrFunction<I64u>(name), _binsLength(0)
	{
		initialize(AnyCast<string>(params["path"]));
	}

	virtual ~QHistogramPrFunction()
	{
		reset();
	}

	Decimal operator()(const I64u x) const;

	Decimal pdf(I64u x) const;

	Decimal cdf(I64u x) const;

	I64u invcdf(Decimal x) const;

	I64u sample(Decimal random) const;

	Interval<I64u> threshold(Decimal yMin) const;

private:

	/**
	 * Load a Q-histogram from the given path. For each entry add a [min, max)
	 * interval of domain values to a lookup table.
	 */
	void initialize(const string& path)
	{
		// reset old state
		reset();

		string line, binMin, binMax;

		ifstream myfile(path.c_str());

		if (!myfile.is_open())
		{
			throw OpenFileException("Unexpected file header for file `" + path +  "`");
		}

		try
		{
			// read first line
			getline(myfile, line);

			if (line.substr(0, 15) != "# numberofbins:")
			{
				throw DataException("Unexpected file header for file `" + path +  "`");
			}

			I32 numberOfLines = atoi(line.substr(15).c_str());

			_bins = new Interval<I64u>[numberOfLines];
			_binsLength = numberOfLines;
			_binProbability = 1.0/numberOfLines;

			if (numberOfLines <= 0 && numberOfLines > 65536)
			{
				throw DataException("Invalid number of lines `" + toString(numberOfLines) +  "`");
			}

			for (I16u i = 0; i < numberOfLines; i++)
			{
				if (!myfile.good())
				{
					throw DataException("Bad line for bin #" + toString(i));
				}

				getline(myfile, line);

				size_t firsttab = line.find_first_of('\t');

				I64u min = fromString<I64u>(line.substr(0, firsttab));
				I64u max = fromString<I64u>(line.substr(firsttab));

				_bins[i].set(min, max);

				if (i > 0)
				{
					if (_bins[i].min() != _bins[i-1].max())
					{
						throw DataException("Bad line for bin #" + toString(i));
					}
				}
			}

			_min = _bins[0].min();
			_max = _bins[_binsLength-1].max();

			myfile.close();
		}
		catch(exception& e)
		{
			myfile.close();
			throw e;
		}
	}

	void reset()
	{
		if (_binsLength > 0)
		{
			delete[] _bins;
			_binsLength = 0;
			_binProbability = 1.0;
		}
	}

	const size_t findBucket(const I64u x) const;

	Interval<I64u>* _bins;
	I64u _min;
	I64u _max;
	size_t _binsLength;
	Decimal _binProbability;
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

inline const size_t QHistogramPrFunction::findBucket(const I64u x) const
{
	// we assert that the value x is in the [_min, _max] range
	int min = 0;
	int max = _binsLength - 1;
	int mid = 0;

	// continue searching while [min, max] is not empty
	while (max >= min)
	{
		// calculate the midpoint for roughly equal partition //
		mid = (min + max) / 2;

		// determine which subarray to search
		if (_bins[mid].max() <=  x)
		{
			// change min index to search upper subarray
			min = mid + 1;
		}
		else if (_bins[mid].min() > x)
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

} // namespace Myriad

#endif /* QHISTOGRAMPRFUNCTION_H_ */
