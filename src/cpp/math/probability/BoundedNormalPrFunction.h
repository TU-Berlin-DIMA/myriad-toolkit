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
 */

#ifndef BOUNDEDNORMALPRFUNCTION_H_
#define BOUNDEDNORMALPRFUNCTION_H_

#include "math/probability/NormalPrFunction.h"

using namespace Poco;

namespace Myriad {
/**
 * @addtogroup math_probability
 * @{*/

/**
 * A bounded normal probability function variant.
 *
 * This variant abuses the theoretical properties of the normal probability, but
 * ensures that all random values lie within the (-3*stddev:+3*stddev) bounds,
 * which contains more than 99% of the probability mass.
 *
 * \b ATTENTION: Using this variant of the normal pr. function is not suitable,
 * if you the data generator program to produce outliers!!!
 *
 * @deprecated
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class BoundedNormalPrFunction: public NormalPrFunction
{
public:

	/**
	 * Explicit anonymous parameter constructor.
	 *
	 * @param mean The \p mean of the normal distribution.
	 * @param stddev The <tt>standard deviation</tt> of the mean distribution.
	 * @param xMin The left bound of the function range.
	 * @param xMax The right bound of the function range.
	 */
	BoundedNormalPrFunction(Decimal mean, Decimal stddev, Decimal xMin, Decimal xMax) :
		NormalPrFunction(mean, stddev), _xMin(xMin), _xMax(xMax)
	{
		initialize();
	}

	/**
	 * Explicit named parameter constructor.
	 *
	 * @param name The name of this probability function instance.
	 * @param mean The \p mean of the normal distribution.
	 * @param stddev The <tt>standard deviation</tt> of the mean distribution.
	 * @param xMin The left bound of the function range.
	 * @param xMax The right bound of the function range.
	 */
	BoundedNormalPrFunction(const string& name, Decimal mean, Decimal stddev, Decimal xMin, Decimal xMax) :
		NormalPrFunction(name, mean, stddev), _xMin(xMin), _xMax(xMax)
	{
		initialize();
	}

	/**
	 * Anonymous ObjectBuilder constructor.
	 *
	 * @param params An array containing the required function parameters.
	 */
	BoundedNormalPrFunction(map<string, Any>& params) :
		NormalPrFunction(params)
	{
		_xMin = AnyCast<Decimal>(params["xMin"]);
		_xMax = AnyCast<Decimal>(params["xMax"]);

		initialize();
	}

	/**
	 * Named ObjectBuilder constructor.
	 *
	 * @param name The name of this probability function instance.
	 * @param params An array containing the required function parameters.
	 */
	BoundedNormalPrFunction(const string& name, map<string, Any>& params) :
		NormalPrFunction(name, params)
	{
		_xMin = AnyCast<Decimal>(params["xMin"]);
		_xMax = AnyCast<Decimal>(params["xMax"]);

		initialize();
	}

	/**
	 * \see UnivariatePrFunction::sample()
	 */
	Decimal sample(Decimal random) const;

private:

	/**
	 * Common initialization logic.
	 */
	void initialize()
	{
		_yMin = cdf(_xMin);
		_yMax = cdf(_xMax);
		_yFactor = _yMax - _yMin;
	}

	Decimal _xMin;
	Decimal _xMax;
	Decimal _yMin;
	Decimal _yMax;
	Decimal _yFactor;
};

inline Decimal BoundedNormalPrFunction::sample(Decimal random) const
{
	return invcdf(_yMin + _yFactor * random);
}

/** @}*/// add to math group
} // namespace Myriad

#endif /* BOUNDEDNORMALPRFUNCTION_H_ */
