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

#ifndef UNIFORMPRFUNCTION_H_
#define UNIFORMPRFUNCTION_H_

#include "core/types.h"
#include "math/Function.h"

#include <Poco/Any.h>

#include <string>
#include <map>
#include <cmath>

using namespace std;
using namespace Poco;

namespace Myriad {

template<typename T>
class UniformPrFunction: public UnivariatePrFunction<T>
{
public:

	UniformPrFunction(T xMin = 0, T xMax = 1) :
		UnivariatePrFunction<T> (""),
		_xMin(xMin),
		_xMax(xMax),
		_size(_xMax - _xMin),
		_xPDF(1.0 / _size)
	{
	}

	UniformPrFunction(const string& name, T xMin = 0, T xMax = 1) :
		UnivariatePrFunction<T> (name),
		_xMin(xMin),
		_xMax(xMax),
		_size(_xMax - _xMin),
		_xPDF(1.0 / _size)
	{
	}

	UniformPrFunction(map<string, Any>& params) :
		UnivariatePrFunction<T> ("")
	{
		_xMin = AnyCast<T>(params["xMin"]);
		_xMax = AnyCast<T>(params["xMax"]);
		_size = _xMax - _xMin;
		_xPDF = 1.0 / _size;
	}

	UniformPrFunction(const string& name, map<string, Any>& params) :
		UnivariatePrFunction<T> (name)
	{
		_xMin = AnyCast<T>(params["xMin"]);
		_xMax = AnyCast<T>(params["xMax"]);
		_size = _xMax - _xMin;
		_xPDF = 1.0 / _size;
	}

	Decimal operator()(const T x) const;

	Decimal pdf(T x) const;

	Decimal cdf(T x) const;

	T invcdf(Decimal x) const;

	T sample(Decimal random) const;

	Interval<T> threshold(Decimal yMin) const;

private:

	// parameters
	T _xMin;
	T _xMax;

	// common used terms
	Decimal _size;
	Decimal _xPDF;
};

template<typename T>
inline Decimal UniformPrFunction<T>::operator()(const T x) const
{
	return cdf(x);
}

template<typename T>
Decimal UniformPrFunction<T>::pdf(T x) const
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

template<typename T>
Decimal UniformPrFunction<T>::cdf(T x) const
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

template<typename T>
T UniformPrFunction<T>::invcdf(Decimal y) const
{
    return _xMin + y * _size;
}

template<typename T>
inline T UniformPrFunction<T>::sample(Decimal random) const
{
	return invcdf(random);
}

template<typename T>
inline Interval<T> UniformPrFunction<T>::threshold(Decimal yMin) const
{
	return Interval<Decimal>(_xMin, _xMax);
}

} // namespace Myriad


#endif /* UNIFORMPRFUNCTION_H_ */
