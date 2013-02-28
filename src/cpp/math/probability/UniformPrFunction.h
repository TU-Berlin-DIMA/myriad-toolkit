/*
 * Copyright 2010-2013 DIMA Research Group, TU Berlin
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
/**
 * @addtogroup math_probability
 * @{*/

/**
 * An uniform probability function implementation.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<typename T>
class UniformPrFunction: public UnivariatePrFunction<T>
{
public:

    /**
     * Explicit anonymous parameter constructor.
     *
     * @param xMin The minimal \p X value (inclusive) for this distribution.
     * @param xMax The The minimal \p X value (exclusive) for this distribution.
     */
    UniformPrFunction(T xMin = 0, T xMax = 1) :
        UnivariatePrFunction<T> (""),
        _xMin(xMin),
        _xMax(xMax),
        _size(_xMax - _xMin),
        _xPDF(1.0 / _size)
    {
    }

    /**
     * Explicit anonymous parameter constructor.
     *
     * @param name The name of this probability function instance.
     * @param xMin The minimal \p X value (inclusive) for this distribution.
     * @param xMax The The minimal \p X value (exclusive) for this distribution.
     */
    UniformPrFunction(const string& name, T xMin = 0, T xMax = 1) :
        UnivariatePrFunction<T> (name),
        _xMin(xMin),
        _xMax(xMax),
        _size(_xMax - _xMin),
        _xPDF(1.0 / _size)
    {
    }

    /**
     * Anonymous ObjectBuilder constructor.
     *
     * @param params An array containing the required function parameters.
     */
    UniformPrFunction(map<string, Any>& params) :
        UnivariatePrFunction<T> ("")
    {
        _xMin = AnyCast<T>(params["xMin"]);
        _xMax = AnyCast<T>(params["xMax"]);
        _size = _xMax - _xMin;
        _xPDF = 1.0 / _size;
    }

    /**
     * Named ObjectBuilder Constructor.
     *
     * @param name The name of this probability function instance.
     * @param params An array containing the required function parameters.
     */
    UniformPrFunction(const string& name, map<string, Any>& params) :
        UnivariatePrFunction<T> (name)
    {
        _xMin = AnyCast<T>(params["xMin"]);
        _xMax = AnyCast<T>(params["xMax"]);
        _size = _xMax - _xMin;
        _xPDF = 1.0 / _size;
    }

    /**
     * @see UnivariatePrFunction::operator()
     */
    Decimal operator()(const T x) const;

    /**
     * @see UnivariatePrFunction::pdf()
     */
    Decimal pdf(T x) const;

    /**
     * @see UnivariatePrFunction::cdf()
     */
    Decimal cdf(T x) const;

    /**
     * @see UnivariatePrFunction::invcdf()
     */
    T invcdf(Decimal x) const;

    /**
     * @see UnivariatePrFunction::sample()
     */
    T sample(Decimal random) const;

    /**
     * FIXME: this is suspicious.
     */
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

/** @}*/// add to math_probability group
} // namespace Myriad

#endif /* UNIFORMPRFUNCTION_H_ */
